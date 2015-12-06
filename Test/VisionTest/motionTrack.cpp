#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

// detect circle and get color bounds
void calibrate(VideoCapture cap, Scalar *lowerBound, Scalar *upperBound) {
	Vec3f centerPixel = (0, 0);
	Vec3f outerPixel = (0, 0);
	bool detectCircle = false;

	while (!detectCircle) {
		Mat frame, gray;
		cap.read(frame);
		if (frame.empty()) {
			break;
		}
		// convert to gray
		cvtColor(frame, gray, CV_BGR2GRAY);
		// blur to reduce noise
		GaussianBlur(gray, gray, Size(9, 9), 2, 2);
		vector<Vec3f> circles;
		imshow("gray frame", gray);

		// Hough transform to find circles
		HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 1, gray.rows/8, 200, 100, 0, 0);
		if (circles.size() > 0) {
			detectCircle = true;
			// draw circles
			for (int i = 0; i < circles.size(); i++) {
				Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
				int radius = cvRound(circles[i][2]);
				centerPixel = frame.at<Vec3f>(center.x, center.y);
				cout << (int)centerPixel[0] << " " << (int)centerPixel[1] << " " << (int)centerPixel[2] << endl;
				outerPixel = frame.at<Vec3f>(center.x + radius - 1, center.y + radius - 1);
				circle(frame, center, 3, Scalar(0,255,0), 3, 8, 0);
				circle(frame, center, radius, Scalar(0,0,255), 3, 8, 0);
				cout << "Center: " << center << "\nRadius: " << radius << endl;
			}
			*lowerBound = Scalar((int)centerPixel[0], (int)centerPixel[1], (int)centerPixel[2]);
			*upperBound = Scalar((int)outerPixel[0], (int)outerPixel[1], (int)outerPixel[2]);
			cout << (int)centerPixel[0] <<" " << (int)centerPixel[1] << " " << (int)centerPixel[2] << endl;
			cout << (int)outerPixel[0] <<" " <<(int)outerPixel[1]<< " " << (int)outerPixel[2] << endl;
		}
		// imshow not supported on intel boards
		// comment out when not on Pascal
		imshow("current frame", frame);

		if (waitKey(30) >= 0) {
			break;
		}
	}
}

//default camera at 0
int main(int argc, char **argv) {
	VideoCapture cap;

	Scalar lowerBound = Scalar(29, 86, 6);
	Scalar upperBound = Scalar(64, 255, 255);

	//change to 0 when on Pascal
	if (!cap.open(2)) {
		cout << "Error detecting camera" << endl;
		return -1;
	}

	// calibrate(cap, &lowerBound, &upperBound);

	// loop to capture and analyze frames
	while(1) {
		Mat frame, blur, hsv_frame, mask;
		cap.read(frame);

		if (frame.empty()) {
			break;
		}

		// imshow not supported on intel edison boards
		// comment out when uploading to board
		imshow("original frame", frame);
		GaussianBlur(frame, blur, Size(11,11), 0, 0);
		// imshow("blurred image", blur);
		cvtColor(blur, hsv_frame, CV_BGR2HSV);
		// imshow("hsv image", hsv_frame);


       // Create a structuring element
       int erosion_size = 5;  
       Mat element = getStructuringElement(cv::MORPH_CROSS,
              cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
              cv::Point(erosion_size, erosion_size) );

		inRange(hsv_frame, lowerBound, upperBound, mask);
		imshow("mask1", mask);
		erode(mask, mask, element, Point(-1,-1), 3);
		dilate(mask, mask, element, Point(-1,-1), 2);
		imshow("mask", mask);

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
			// center = Point(m.m10/m.m00, m.m01/m.m00);
			if (radius > 10) {
				Scalar color = Scalar(40, 70, 200);
				circle(frame, center, 3, Scalar(0,255,0), 3, 8, 0);
				circle(frame, center, (int)radius, color, 2, 8, 0);
			}
			// drawContours(frame, contours, contour_index, color, 2, 8);
		}

    	imshow("drawing", frame);

		if (waitKey(30) >= 0) {
			break;
		}
	}

	// loop to capture and analyze frames
	// while (true) {
	// 	Mat frame, gray;
	// 	Vec3f centerPixel = (0, 0);
	// 	cap.read(frame);
	// 	if (frame.empty()) {
	// 		break;
	// 	}
	// 	// convert to gray
	// 	cvtColor(frame, gray, CV_BGR2GRAY);
	// 	// blur to reduce noise
	// 	GaussianBlur(gray, gray, Size(9, 9), 2, 2);
	// 	vector<Vec3f> circles;
	// 	imshow("gray frame", gray);

	// 	// Hough transform to find circles
	// 	HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 1, gray.rows/8, 200, 100, 0, 0);
	// 	// draw circles
	// 	for (int i = 0; i < circles.size(); i++) {
	// 		Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
	// 		int radius = cvRound(circles[i][2]);
	// 		centerPixel = frame.at<Vec3f>(center.x, center.y);
	// 		circle(frame, center, 3, Scalar(0,255,0), 3, 8, 0);
	// 		circle(frame, center, radius, Scalar(0,0,255), 3, 8, 0);
	// 		cout << "Center: " << center << "\nRadius: " << radius << endl;
	// 	}
	// 	// imshow not supported on intel boards
	// 	// comment out when not on Pascal
	// 	imshow("current frame", frame);
	// 	// exits loop on "esc" press
	// 	if (waitKey(30) >= 0) {
	// 		break;
	// 	}
	// }

	cap.release();

	return 0;
}