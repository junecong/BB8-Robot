#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <queue>
#include <cctype>
#include <string>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define MAXQUEUESIZE 32

using namespace cv;
using namespace std;

//106 141
//48 165
//118 173
// detect circle and get color bounds
void calibrate(VideoCapture cap, Scalar *lowerBound, Scalar *upperBound, ofstream &file) {

    namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"
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

	Mat imgTmp;

	cap.read(imgTmp);

	Mat imgLines = Mat::zeros(imgTmp.size(), CV_8UC3);

    while (true) {
        Mat imgOriginal;

        bool bSuccess = cap.read(imgOriginal); // read a new frame from video

        if (!bSuccess) {
             cout << "Cannot read a frame from video stream" << endl;
             break;
        }

	    Mat imgHSV;

	    cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

	    Mat imgThresholded;

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

        if (waitKey(30) == 27) {
        	file << iLowH << endl;
        	file << iHighH << endl;
        	file << iLowS << endl;
        	file << iHighS << endl;
        	file << iLowV << endl;
        	file << iHighV << endl;
        	file.close();
            cout << "esc key is pressed by user. Values saved." << endl;
            destroyWindow("Thresholded Image");
            break; 
       }
    }
}

//default camera at 0
int main(int argc, char **argv) {
	VideoCapture cap;
	// fstream file("values.txt", ios::in | ios::out | ios::app);
	ifstream infile;
	ofstream outfile;
	deque <Point2f> points;
	char response[10];

	// Set up camera
	// change to 0 when on Pascal
	if (!cap.open(0)) {
		cout << "Error detecting camera" << endl;
		return -1;
	}

	Scalar lowerBound = Scalar(40,106,100);
	Scalar upperBound = Scalar(120,255,255);

	// Scalar lowerBound = Scalar(29, 86, 6);
	// Scalar upperBound = Scalar(64, 255, 255);
	cout << "Recalibrate? Y or N: ";
	cin >> response;
	tolower(response[0]);

	if (response[0] == 'y') {
		outfile.open("values.txt");
		calibrate(cap, &lowerBound, &upperBound, outfile);
	} else {
		infile.open("values.txt");
		string curr_line;
		int hsvArr[6];
		int i = 0;
		while (getline(infile, curr_line)) {
			hsvArr[i] = atoi(curr_line.c_str());
			i++;
		}
		lowerBound = Scalar(hsvArr[0], hsvArr[2], hsvArr[4]);
		upperBound = Scalar(hsvArr[1], hsvArr[3], hsvArr[5]);
	}


	// loop to capture and analyze frames
	while(1) {
		Mat frame, gray, blur, hsv_frame, mask;
		cap.read(frame);

		if (frame.empty()) {
			break;
		}

		// imshow not supported on intel edison boards
		// comment out when uploading to board
		// imshow("original frame", frame);
		GaussianBlur(frame, blur, Size(11,11), 0, 0);
		cvtColor(blur, hsv_frame, CV_BGR2HSV);
		// imshow("hsv image", hsv_frame);

	    // Create a structuring element
	    int erosion_size = 6;  
        Mat element = getStructuringElement(cv::MORPH_CROSS,
            cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
            cv::Point(erosion_size, erosion_size) );

		inRange(hsv_frame, lowerBound, upperBound, mask);

		// test to see if bitwise_and will work on HSV Mat
		// vector<Mat> test;
		// split(hsv_frame, test);
		// Mat h = test[0];
		// Mat s = test[1];
		// Mat v = test[2];
		// inRange(h, Scalar(lowerBound[0],0,0,0), Scalar(upperBound[0],0,0,0), h);
		// inRange(s, Scalar(lowerBound[1],0,0,0), Scalar(upperBound[1],0,0,0), s);
		// inRange(v, Scalar(lowerBound[2],0,0,0), Scalar(upperBound[2],0,0,0), v);
		// imshow("h", h);
		// imshow("s", s);
		// imshow("v", v);
		// bitwise_and(h, s, h);
		// bitwise_and(h,v, mask);
		// imshow("anded", h);

		imshow("mask1", mask);
		// erode(mask, mask, element, Point(-1,-1), 2);
		// dilate(mask, mask, element, Point(-1,-1), 2);

	    //morphological opening (removes small objects from the foreground)
	    erode(mask, mask, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
	    dilate( mask, mask, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 

	    //morphological closing (removes small holes from the foreground)
	    dilate(mask, mask, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 
	    erode(mask, mask, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

		imshow("mask2", mask);

		// circle test
		cvtColor(blur, gray, CV_BGR2GRAY);
		imshow("gray", gray);
		// play around with HoughCircle parameters to get better circle detection
		GaussianBlur(mask, mask, Size(9,9), 0, 0);
		imshow("blur mask", mask);
		vector<Vec3f> circles;
		HoughCircles(mask, circles, CV_HOUGH_GRADIENT, 2, 15, 200, 80, 0, 0);
		// cout << circles.size() << endl;
		// draw circles
		// for (int i = 0; i < circles.size(); i++) {
		// 	Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		// 	int radius = cvRound(circles[i][2]);
		// 	circle(frame, center, 3, Scalar(0,255,0), 3, 8, 0);
		// 	circle(frame, center, radius, Scalar(0,0,255), 3, 8, 0);
		// 	cout << "Center: " << center << "\nRadius: " << radius << endl;
		// }
		imshow("current frame", frame);

		vector<vector<Point> > contours;
		findContours(mask.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		double largest_area = 0;
		int contour_index = 0;
		Moments m;
		Point2f center;
		float radius;

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
			approxPolyDP(Mat(correctContour), correctContour, 3, true);
			minEnclosingCircle((Mat)correctContour, center, radius);
			m = moments(contours[contour_index], false);

			// for (int i = 0; i < circles.size(); i++) {
			// 	cout << "circle" << endl;
			// 	cout << Point(cvRound(circles[i][0]) , cvRound(circles[i][1])) << endl;
			// 	cout << "center" << endl;
			// 	cout << center << endl;
			// }

			int bias = 10;

			if (radius > 10) {
				if (circles.size() > 0) {
					// for sure this is the object we are looking for
					for (int i = 0; i < circles.size(); i++) {
						Point circleCenter = Point(cvRound(circles[i][0]), cvRound(circles[i][1]));
						if (abs(circleCenter.x - center.x) < bias && abs(circleCenter.y - center.y) < bias) {
							Scalar color = Scalar(40, 70, 200);
							circle(frame, center, 3, Scalar(0,255,0), 3, 8, 0);
							circle(frame, center, (int)radius, color, 2, 8, 0);
						}
					}
				} else {

				}
			}
			// if (radius > 10) {
			// 	Scalar color = Scalar(40, 70, 200);
			// 	circle(frame, center, 3, Scalar(0,255,0), 3, 8, 0);
			// 	circle(frame, center, (int)radius, color, 2, 8, 0);
			// }
		}

		// if (center != Point2f(0,0)) {
		// 	points.push_back(center);
		// }
		// for (int i = 1; i < points.size(); i++) {
		// 	line(frame, points[i - 1], points[i], Scalar(43,231,123), 6);
		// }

		// if(points.size() >= MAXQUEUESIZE) {
		// 	points.pop_front();
		// }

    	imshow("drawing", frame);

		if (waitKey(30) >= 0) {
			break;
		}
	}

	cap.release();

	return 0;
}