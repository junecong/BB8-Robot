#include <stdio.h>
#include <stdlib.h>
#include <queue>
#include <string>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "motionTrack.h"
#include "FSM.h"

#define MAXQUEUESIZE 32
#define MAXSIZE 5
#define MAX_OBJ_DIST_BW_FRAMES 10
#define ACTUAL_DIAMETER_IN_CM 16.f

using namespace cv;
using namespace std;

// calibrate HSV values and save in output file for later use
void calibrate(VideoCapture cap, Scalar *lowerBound, Scalar *upperBound, ofstream &file) {

    namedWindow("Control", WINDOW_NORMAL); //create a window called "Control"
    resizeWindow("Control", 300, 100);
	// start calibration
	int iLowH = 0;
	int iHighH = 179;

	int iLowS = 0;
	int iHighS = 255;

	int iLowV = 0;
	int iHighV = 255;

	createTrackbar("LowH", "Control", &iLowH, 179);
	createTrackbar("HighH", "Control", &iHighH, 179);

	createTrackbar("LowS", "Control", &iLowS, 255);
	createTrackbar("HighS", "Control", &iHighS, 255);

	createTrackbar("LowV", "Control", &iLowV, 255);
	createTrackbar("HighV", "Control", &iHighV, 255);

	namedWindow("Thresholded Image", WINDOW_NORMAL);
	resizeWindow("Thresholded Image", 600, 600);

    while (true) {
        Mat imgOriginal;

        bool bSuccess = cap.read(imgOriginal); // read a new frame from video

        if (!bSuccess) {
             cout << "Cannot read a frame from video stream" << endl;
             break;
        }

	    Mat imgHSV, imgThresholded;

	    cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

	    inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image
	      
	    //morphological opening (removes small objects from the foreground)
	    erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
	    dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 

	    //morphological closing (removes small holes from the foreground)
	    dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 
	    erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

	    *lowerBound = Scalar(iLowH, iLowS, iLowV);
	    *upperBound = Scalar(iHighH, iHighS, iHighV);

	    imshow("Thresholded Image", imgThresholded); //show the thresholded image

        if (waitKey(30) >= 0) {
        	file << iLowH << endl;
        	file << iHighH << endl;
        	file << iLowS << endl;
        	file << iHighS << endl;
        	file << iLowV << endl;
        	file << iHighV << endl;
        	file.close();
            cout << "esc key is pressed by user. Values saved." << endl;
            destroyAllWindows();
            break; 
       }
    }
}

void filterImage(Mat *frame, Mat *mask, Scalar lowerBound, Scalar upperBound,
				 vector<Vec3f> circles, bool isObject) {
	Mat gray, blur, hsv_frame;
	// imshow not supported on intel edison boards
	// comment out when uploading to board
	// imshow("original frame", frame);
	GaussianBlur(*frame, blur, Size(11,11), 0, 0);
	cvtColor(blur, hsv_frame, CV_BGR2HSV);
	// imshow("hsv image", hsv_frame);

	inRange(hsv_frame, lowerBound, upperBound, *mask);

	// imshow("mask1", mask);
	// Create a structuring element
    // int erosion_size = 6;  
 	// Mat element = getStructuringElement(cv::MORPH_CROSS,
 	//     cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
 	//     cv::Point(erosion_size, erosion_size) );
	// erode(mask, mask, element, Point(-1,-1), 2);
	// dilate(mask, mask, element, Point(-1,-1), 2);

    //morphological opening (removes small objects from the foreground)
    erode(*mask, *mask, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
    dilate(*mask, *mask, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 

    //morphological closing (removes small holes from the foreground)
    dilate(*mask, *mask, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 
    erode(*mask, *mask, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

	// imshow("mask2", mask);
    if (isObject) {
		cvtColor(blur, gray, CV_BGR2GRAY);
		// imshow("gray", gray);
		GaussianBlur(*mask, *mask, Size(9,9), 0, 0);
		// imshow("blur mask", mask);
		// play around with HoughCircle parameters to get better circle detection
		HoughCircles(*mask, circles, CV_HOUGH_GRADIENT, 2, 15, 200, 80, 0, 0);
	}
}

// play around with radialBias to tune how big the object is to detect
void detectObject(Mat *frame, vector<Vec3f> circles, vector<vector<Point> > contours, Point2f *center, Point2f prev_center,
				  float *radius, float prev_radius, bool isObject, bool *isOffscreen, int bias, int radialBias) {
	double largest_area = 0;
	int contour_index = 0;
	deque <vector<Point> > largest_contours;
	int contours_size;

	// if contours exist
	if (contours.size() > 0) {
		// find largest area and center
		for (int i = 0; i < contours.size(); i++) {
			double area = contourArea(contours[i], false);
			if (area > largest_area) {
				largest_area = area;
				contour_index = i;
				largest_contours.push_back(contours[i]);
				contours_size = largest_contours.size();
				if (contours_size > MAXSIZE){
					largest_contours.pop_front();
				}
			}
		}


		// vector<Point> correctContour = contours[contour_index];
		float min_radius;
		float max_radius;
		float avg_radius;
		float avg_center;
		float dist_center;
		float dist_radius;
		Point2f min_center;
		Point2f max_center;


		for (int i = 0; i < largest_contours.size(); i++){
			vector<Point> correctContour = largest_contours[i];

			// Approximate polygon curve with specified position
			approxPolyDP(Mat(correctContour), correctContour, 3, true);
			minEnclosingCircle((Mat)correctContour, *center, *radius);
			// maxEnclosingCircle((Mat)correctContour, *center, *radius);
			// avg_radius = (*min_radius + *max_radius)/2.f;
			// avg_center = (*min_center + *max_center)/2.f;

			// Filter on movement of the Center
			dist_center = (norm(*center - prev_center) * ACTUAL_DIAMETER_IN_CM) / (2.0 * (*radius));
			dist_radius = fabs((*radius - prev_radius) * ACTUAL_DIAMETER_IN_CM) / (2.0 * (*radius));
			
			if (dist_center < MAX_OBJ_DIST_BW_FRAMES && dist_radius < MAX_OBJ_DIST_BW_FRAMES){
				break;
			}
		}

		if (*radius > radialBias) {
			if (isObject) {
				// might not be object we're looking for
				circle(*frame, *center, 3, Scalar(147, 20, 32), 3, 8, 0);
				circle(*frame, *center, (int)*radius, Scalar(0, 255, 0), 2, 8, 0);
				*isOffscreen = false;
			} else {
				circle(*frame, *center, 3, Scalar(255, 101, 255), 3, 8, 0);
				circle(*frame, *center, (int)*radius, Scalar(255, 0, 0), 2, 8, 0);
			}
		} else {
			if (isObject) {
				if (circles.size() > 0) {
					// for sure this is the object we are looking for
					for (int i = 0; i < circles.size(); i++) {
						Point circleCenter = Point(cvRound(circles[i][0]), cvRound(circles[i][1]));
						if (abs(circleCenter.x - (*center).x) < bias && abs(circleCenter.y - (*center).y) < bias) {
							circle(*frame, *center, 3, Scalar(139, 100, 54), 3, 8, 0);
							circle(*frame, *center, (int)*radius, Scalar(0, 255, 0), 2, 8, 0);
							*isOffscreen = true;
						}
					}
				} else {
					circle(*frame, *center, 3, Scalar(0, 0, 255), 3, 8, 0);
					circle(*frame, *center, (int)*radius, Scalar(0, 0, 255), 2, 8, 0);
					*isOffscreen = true;
				}
			}
		}
	}
}

// play around with bias to get more sensitive readings
void detectDirection(Mat *frame, deque <Point2f> points, int pt_size, string *direction, int x_bias, int y_bias) {
	int dX = 0;
	int dY = 0;
	char dXdY[50] = "";
	string latDirection = "";
	string longDirection = "";

	if (points.size() > 10) {
		// find change in x and y using points from queue
		dX = (points[pt_size - 10]).x - (points[pt_size]).x;
		dY = (points[pt_size - 10]).y - (points[pt_size]).y;
		sprintf(dXdY, "dx: %d dy: %d", dX, dY);
		if (abs(dX) > x_bias) {
			if (dX > 0) {
				latDirection = "West";
			} else {
				latDirection = "East";
			}
		}
		if (abs(dY) > y_bias) {
			if (dY > 0) {
				longDirection = "North";
			} else {
				longDirection = "South";
			}
		}
		if (!longDirection.empty() && !latDirection.empty()) {
			*direction = longDirection + "-" + latDirection;
		} else if (!longDirection.empty()){
			*direction = longDirection;
		} else if (!latDirection.empty()) {
			*direction = latDirection;
		}
	}

	putText(*frame, dXdY, Point(10, 450), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255));
}

float getAverageRadius (deque <float> radii, int radiiSize) {
	float total = 0;
	for (int i = 0; i < radiiSize; i++) {
		total += radii[i];
	}
	return total / radiiSize;
}

Point2f getAveragePoint (deque <Point2f> center, float size) {
	Point2f pointTotal = Point2f(0, 0);
	for (int i = 0; i < size; i++) {
		pointTotal += center[i];
	}

	return Point2f(pointTotal.x/size, pointTotal.y/size);
}

void userInput(VideoCapture cap, Scalar *lowerBound, Scalar *upperBound, char *fileName) {
	ifstream infile;
	ofstream outfile;
	char response[10];

	cout << "Recalibrate for "<< fileName << " ? y or n: ";
	cin >> response;
	response[0] = tolower(response[0]);

	if (response[0] == 'y') {
		outfile.open(fileName);
		calibrate(cap, lowerBound, upperBound, outfile);
	} else {
		infile.open(fileName);
		string curr_line;
		int hsvArr[6];
		int i = 0;
		while (getline(infile, curr_line)) {
			hsvArr[i] = atoi(curr_line.c_str());
			i++;
		}
		if (i > 5) {
			*lowerBound = Scalar(hsvArr[0], hsvArr[2], hsvArr[4]);
			*upperBound = Scalar(hsvArr[1], hsvArr[3], hsvArr[5]);
		}
	}
}

//default camera at 0
// int main(int argc, char **argv) {
int analyzeVideo(string output[]) {
	VideoCapture cap;
	deque <Point2f> objectPoints;
	deque <Point2f> destPoints;
	deque <float> objectRadii;
	deque <float> destRadii;

	bool debugMode = false;

	// detect debug mode
	// if (argc > 1) {
	// 	char *flag = argv[1];
	// 	if (strcmp(flag, "-d") == 0) {
	// 		debugMode = true;
	// 	}
	// }

	// Set up camera
	if (!cap.open(1)) {
		cout << "Error detecting camera1" << endl;
		if (!cap.open(0)) {
			cout << "Error detecting camera0" << endl;
			return -1;
		}
	}

	Scalar lowerBoundObject = Scalar(0, 0, 0);
	Scalar upperBoundObject = Scalar(120, 255, 255);

	Scalar lowerBoundDest = Scalar(0, 0, 0);
	Scalar upperBoundDest = Scalar(120, 255, 255);

	// for calibrating Object
	userInput(cap, &lowerBoundObject, &upperBoundObject, "Object-HSV.txt");
	// for calibrating Destination
	userInput(cap, &lowerBoundDest, &upperBoundDest, "Destination-HSV.txt");

	namedWindow("drawing", WINDOW_NORMAL);
	resizeWindow("drawing", 600, 600);

	Point2f objectCenter;
	Point2f prev_objectCenter = objectCenter;
	Point2f destCenter;
	Point2f prev_destCenter = destCenter;
	float objectRadius;
	float prev_objectRadius;
	float destRadius;
	float prev_destRadius;

	double max_dist = 0;
	double dist = 0;
	int count = 0;

	// loop to capture and analyze frames
	while(1) {
		string direction = "Stationary";
		bool isOffscreen = true;
		Mat frame, mask, destMask;
		cap.read(frame);

		if (frame.empty()) {
			cout << "Empty Frame!" << endl;
			break;
		}

		// creates a mask from HSV values and circle detection for the object
		vector<Vec3f> circles;
		filterImage(&frame, &mask, lowerBoundObject, upperBoundObject, circles, true);

		// creates a mask from HSV values and circle detection for the destination
		vector<Vec3f> destCircles;
		filterImage(&frame, &destMask, lowerBoundDest, upperBoundDest, destCircles, false);

		// finds Contours for the Object
		vector<vector<Point> > contours;
		findContours(mask.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		// finds Contours for the Destination
		vector<vector<Point> > destContours;
		findContours(destMask.clone(), destContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		// detects the object and draws to the frame
		// gives the center and radius of the object
		detectObject(&frame, circles, contours, &objectCenter, prev_objectCenter, &objectRadius, prev_objectRadius, true, &isOffscreen); 
		prev_objectCenter = objectCenter; 
		prev_objectRadius = objectRadius;

		detectObject(&frame, destCircles, destContours, &destCenter, prev_destCenter, &destRadius, prev_destRadius, false, &isOffscreen);
		prev_destCenter = destCenter;
		prev_objectRadius = objectRadius;

		int pt_size = objectPoints.size();
		int obpt_size = destPoints.size();
		int objectRadii_size = objectRadii.size();
		int destRadii_size = destRadii.size();

		// add object center points to queue
		objectPoints.push_back(objectCenter);
		// add destination center points to queue
		destPoints.push_back(destCenter);
		// add radius of object to queue
		objectRadii.push_back(objectRadius);
		// add radius of dest ro queue
		destRadii.push_back(destRadius);

		// draw line to frame from point queue of object movement
		for (int i = 1; i < pt_size; i++) {
			line(frame, objectPoints[i - 1], objectPoints[i], Scalar(43,231,123), 6);
		}

		// detects direction of object movement
		detectDirection(&frame, objectPoints, pt_size, &direction);

		// get averaged center points from object
		Point2f avgCenterPoint = getAveragePoint(objectPoints, pt_size);

		// get averaged center points from destination
		Point2f avgDestPoint = getAveragePoint(destPoints, obpt_size);

		// gets average radius of object
		float avgObjectRadius = getAverageRadius(objectRadii, objectRadii_size);

		// gets average radius of destination object
		float avgDestRadius = getAverageRadius(destRadii, destRadii_size);

		// need to calculate angle and driveDistance
		float angle = 0.0;
		float dx = abs(avgCenterPoint.x - avgDestPoint.x);
		float dy = abs(avgCenterPoint.y - avgDestPoint.y);
		float centerDistance = sqrt(dx*dx + dy*dy);
		float driveDistance = centerDistance - avgObjectRadius - avgDestRadius;

		if (debugMode) {
			cout << endl;
			cout << "distance left: " << driveDistance << endl;
			cout << "offscreen: " << isOffscreen << endl;
			cout << "object point: " << "(" << avgCenterPoint.x << ", " << avgCenterPoint.y << ")" << endl;
			cout << "object radius: " << avgObjectRadius << endl;
			cout << "dest point: " << "(" << avgDestPoint.x << ", " << avgDestPoint.y << ")" << endl;
			cout << "dest radius: " << avgDestRadius << endl;
			cout << "dircetion: " << direction << endl;
			cout << endl;
		}

		MaxwellStatechart(
			driveDistance, 			// distance from object to destination
			isOffscreen, 			// if Object is isOffscreen
			avgCenterPoint.x, 		// x point of Object
			avgCenterPoint.y, 		// y point of Object
			avgObjectRadius, 		// radius of Object
			avgDestPoint.x, 		// x point of Destination
			avgDestPoint.y, 		// y point of Destination
			avgDestRadius,			// radius of destination
			direction,				// direction object is moving
			output					// output 
		);

		if (output[0].compare("turn") == 0){
			cout << "CURRENTLY TURNING" << endl;
		}

		// pop points queue
		if (pt_size >= MAXQUEUESIZE) {
			objectPoints.pop_front();
		}

		if (obpt_size >= MAXQUEUESIZE) {
			destPoints.pop_front();
		}

		// pop objectRadii queue
		if (objectRadii_size >= MAXQUEUESIZE) {
			objectRadii.pop_front();
		}

		// pop destRadii queue
		if (destRadii_size >= MAXQUEUESIZE) {
			destRadii.pop_front();
		}

    	imshow("drawing", frame);

		if (waitKey(30) >= 0) {
			break;
		}
	}

	cap.release();

	return 0;
}
