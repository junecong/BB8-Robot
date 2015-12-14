#ifndef EXTERNALS_H
#define EXTERNALS_H
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
#include <chrono>

extern bool debugMode;
extern bool sendMode;
extern bool localhostMode;

using namespace cv;
using namespace std;

class BoundedBuffer {
    vector<vector<string> > buffer;
    int capacity;
    int front;
    int rear;
    int count;
    mutex lock;
    condition_variable not_full;
    condition_variable not_empty;

public:
    BoundedBuffer(int capacity);
    ~BoundedBuffer();
    void deposit(vector<string> data);
    vector<string> fetch();
};

extern BoundedBuffer bBuffer;

#endif
