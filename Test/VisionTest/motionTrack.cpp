#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

//default camera at 0
int main(int argc, char **argv) {
	VideoCapture cap;

	//change to 0 when on Pascal
	if (!cap.open(1)) {
		cout << "Error detecting camera" << endl;
		return -1;
	}

	// loop to capture and analyze frames
	while (true) {
		Mat frame, gray;
		cap.read(frame);
		if (frame.empty()) {
			break;
		}
		cvtColor(frame, gray, CV_BGR2GRAY);
		GaussianBlur(gray, gray, Size(9, 9), 2, 2);
		vector<Vec3f> circles;
		imshow("gray frame", gray);

		HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 1, 30, 200, 50, 0, 0);
		for (int i = 0; i < circles.size(); i++) {
			Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
			int radius = cvRound(circles[i][2]);
			circle(frame, center, 3, Scalar(0,255,0), 3, 8, 0);
			circle(frame, center, radius, Scalar(0,0,255), 3, 8, 0);
			cout << "Center: " << center << "\nRadius: " << radius << endl;
		}
		// imshow not supported on intel boards
		// comment out when not on Pascal
		imshow("current frame", frame);
		// exits loop on "esc" press
		if (waitKey(30) >= 0) {
			break;
		}
	}

	cap.release();

	return 0;
}