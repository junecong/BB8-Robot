#ifndef MOTIONTRACK
#define MOTIONTRACK
#include <stdio.h>
#include <stdlib.h>
#include <queue>
#include <string>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "FSM.h"

#define MAXQUEUESIZE 32

using namespace cv;
using namespace std;

void calibrate(VideoCapture cap, Scalar *lowerBound, Scalar *upperBound, ofstream &file);
void filterImage(Mat *frame, Mat *mask, Scalar lowerBound, Scalar upperBound, vector<Vec3f> circles);
void detectObject(Mat *frame, vector<Vec3f> circles, vector<vector<Point> > contours, Point2f *center, float *radius);
void detectDirection(Mat *frame, deque <Point2f> points, int pt_size, string *direction);
int analyzeVideo();
#endif