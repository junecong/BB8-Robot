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

void filterImage(Mat *frame, Mat *mask, Scalar lowerBound, Scalar upperBound, vector<Vec3f> circles, bool isObject) {
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

void detectObject(Mat *frame, vector<Vec3f> circles, vector<vector<Point> > contours, Point2f *center, float *radius, bool isObject, bool *offscreen) {
	double largest_area = 0;
	int contour_index = 0;
	// if contours exist
	if (contours.size() > 0) {
		// find largest area and center
		for (int i = 0; i < contours.size(); i++) {
			double area = contourArea(contours[i], false);
			if (area > largest_area) {
				largest_area = area;
				contour_index = i;
			}
		}

		vector<Point> correctContour = contours[contour_index];

		// Approximate polygon curve with specified position
		approxPolyDP(Mat(correctContour), correctContour, 3, true);
		minEnclosingCircle((Mat)correctContour, *center, *radius);

		int bias = 10;

		if (*radius > 10) {
			if (isObject) {
				if (circles.size() > 0) {
					// for sure this is the object we are looking for
					for (int i = 0; i < circles.size(); i++) {
						Point circleCenter = Point(cvRound(circles[i][0]), cvRound(circles[i][1]));
						if (abs(circleCenter.x - (*center).x) < bias && abs(circleCenter.y - (*center).y) < bias) {
							circle(*frame, *center, 3, Scalar(139, 100, 54), 3, 8, 0);
							circle(*frame, *center, (int)*radius, Scalar(0, 255, 0), 2, 8, 0);
						}
					}
				} else {
					// might not be object we're looking for
					circle(*frame, *center, 3, Scalar(147, 20, 32), 3, 8, 0);
					circle(*frame, *center, (int)*radius, Scalar(0, 0, 255), 2, 8, 0);
				}
				*offscreen = false;
			} else {
				circle(*frame, *center, 3, Scalar(255, 101, 255), 3, 8, 0);
				circle(*frame, *center, (int)*radius, Scalar(79, 167, 64), 2, 8, 0);
			}
		} else {
			if (isObject) {
				*offscreen = true;
			}
		}
	}
}

void detectDirection(Mat *frame, deque <Point2f> points, int pt_size, string *direction) {
	int dX = 0;
	int dY = 0;
	int x_bias = 20;
	int y_bias = 20;
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
int main(int argc, char **argv) {
	VideoCapture cap;
	deque <Point2f> points;
	deque <float> radii;

	// Set up camera
	// change to 0 when on Pascal
	if (!cap.open(0)) {
		cout << "Error detecting camera" << endl;
		return -1;
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

	// loop to capture and analyze frames
	while(1) {
		string direction = "Stationary";
		bool offscreen = true;
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

		Moments m;

		// finds Contours for the Destination
		vector<vector<Point> > destContours;
		findContours(destMask.clone(), destContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		// detects the object and draws to the frame
		// gives the center and radius of the object
		Point2f center;
		Point2f prev_center = center; 
		float radius = 0;
		detectObject(&frame, circles, contours, &center, &radius, true, &offscreen); 

		// detects the destination object and draws to the frame
		// gives the center and radius of the object
		Point2f destCenter;
		float destRadius;
		detectObject(&frame, destCircles, destContours, &destCenter, &destRadius, false, &offscreen);

		int pt_size = points.size();
		int radii_size = radii.size();

		// add center points to queue 
		if (center != Point2f(0,0)) {
			points.push_back(center);
		}

		// add radius of object to queue
		radii.push_back(radius);

		// draw line to frame from point queue of object movement
		for (int i = 1; i < pt_size; i++) {
			line(frame, points[i - 1], points[i], Scalar(43,231,123), 6);
		}

		// detects direction of object movement
		detectDirection(&frame, points, pt_size, &direction);

		// detects change in radius of object
		float avgRadius = getAverageRadius(radii, radii_size);
		
		// need to calculate angle and driveDistance
		float angle = 0.0;
		float dx = abs(center.x - destCenter.x);
		float dy = abs(center.y - destCenter.y);
		float driveDistance = sqrt(dx*dx + dy*dy);

		MaxwellStatechart (
			angle, 					// angle
			driveDistance, 			// distance from object to destination
			offscreen, 				// if Object is offscreen
			center.x, 				// x point of Object
			center.y, 				// y point of Object
			avgRadius, 				// radius of Object
			destCenter.x, 			// x point of Destination
			destCenter.y, 			// y point of Destination
			destRadius,				// radius of destination
			direction				// direction object is moving
		);

		// pop points queue
		if (pt_size >= MAXQUEUESIZE) {
			points.pop_front();
		}

		// pop radii queue
		if (radii_size >= MAXQUEUESIZE) {
			radii.pop_front();
		}

    	imshow("drawing", frame);

		if (waitKey(30) >= 0) {
			break;
		}
	}

	cap.release();

	return 0;
}
