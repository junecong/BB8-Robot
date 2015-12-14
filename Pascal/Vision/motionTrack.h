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
#include "../Globals/externals.h"
#include "FSM.h"

#define MAXQUEUESIZE 32
#define MAXSIZE 5
#define MAX_OBJ_DIST_BW_FRAMES 10
#define ACTUAL_DIAMETER_IN_CM 16.f

// extern bool debugMode;
// extern bool global_Need_ToExit;

using namespace cv;
using namespace std;

// struct BoundedBuffer {
//     vector<vector<string> > buffer;
//     int capacity;

//     int front;
//     int rear;
//     int count;

//     std::mutex lock;

//     std::condition_variable not_full;
//     std::condition_variable not_empty;

//     BoundedBuffer(int capacity) : capacity(capacity), front(0), rear(0), count(0) {
//         buffer.resize(capacity);
//     }

//     ~BoundedBuffer(){
//     	buffer.clear();
//     }

//     void deposit(vector<string> data){
//         std::unique_lock<std::mutex> l(lock);
//         not_full.wait(l, [this](){return count != capacity; });

//         buffer[rear] = data;
//         rear = (rear + 1) % capacity;
//         ++count;

//         not_empty.notify_one();
//     }

//     vector<string> fetch(){
//         std::unique_lock<std::mutex> l(lock);

//         not_empty.wait(l, [this](){return count != 0; });

//         vector<string> result = buffer[front];
//         front = (front + 1) % capacity;
//         --count;

//         not_full.notify_one();

//         return result;
//     }
// };

// extern BoundedBuffer bBuffer;

void calibrate(VideoCapture cap, Scalar *lowerBound, Scalar *upperBound, ofstream &file);
void filterImage(Mat *frame, Mat *mask, Scalar lowerBound, Scalar upperBound,
				 vector<Vec3f> circles, bool isObject);
void detectObject(Mat *frame, vector<Vec3f> circles, vector<vector<Point> > contours, Point2f *center, Point2f prev_center,
				  float *radius, float prev_radius, bool isObject, bool *isOffscreen, int bias=10, int radialBias=10);
void detectDirection(Mat *frame, deque <Point2f> points, int pt_size, string *direction, int x_bias=10, int y_bias=10);
void userInput(VideoCapture cap, Scalar *lowerBound, Scalar *upperBound, char *fileName);
Point2f getAveragePoint (deque <Point2f> center, float size);
float getAverageRadius (deque <float> radii, int radiiSize);
int analyzeVideo();
#endif
