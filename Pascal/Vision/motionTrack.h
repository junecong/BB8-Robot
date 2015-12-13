#ifndef MOTIONTRACK_H
#define MOTIONTRACK_H
#include <stdio.h>
#include <stdlib.h>
#include <queue>
#include <string>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "FSM.h"

#define MAXQUEUESIZE 32
#define MAXSIZE 5
#define MAX_OBJ_DIST_BW_FRAMES 10
#define ACTUAL_DIAMETER_IN_CM 16.f

extern mutex msg_mutex;
extern condition_variable no_message;
extern bool messageReady;
extern bool debugMode;

using namespace cv;
using namespace std;

void calibrate(VideoCapture cap, Scalar *lowerBound, Scalar *upperBound, ofstream &file);
void filterImage(Mat *frame, Mat *mask, Scalar lowerBound, Scalar upperBound,
				 vector<Vec3f> circles, bool isObject);
void detectObject(Mat *frame, vector<Vec3f> circles, vector<vector<Point> > contours, Point2f *center, Point2f prev_center,
				  float *radius, float prev_radius, bool isObject, bool *isOffscreen, int bias=10, int radialBias=10);
void detectDirection(Mat *frame, deque <Point2f> points, int pt_size, string *direction, int x_bias=10, int y_bias=10);
void userInput(VideoCapture cap, Scalar *lowerBound, Scalar *upperBound, char *fileName);
Point2f getAveragePoint (deque <Point2f> center, float size);
float getAverageRadius (deque <float> radii, int radiiSize);
int analyzeVideo(string output[]);
#endif
