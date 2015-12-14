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

using namespace cv;
using namespace std;

// struct BoundedBuffer {
//     vector<vector<string> > buffer;
//     int capacity;

//     int front;
//     int rear;
//     int count;

//     mutex lock;

//     condition_variable not_full;
//     condition_variable not_empty;

//     BoundedBuffer(int capacity);

//     ~BoundedBuffer();

//     void deposit(vector<string> data);

//     vector<string> fetch();
// };

struct BoundedBuffer {
    vector<vector<string> > buffer;
    int capacity;

    int front;
    int rear;
    int count;

    mutex lock;

    condition_variable not_full;
    condition_variable not_empty;

    BoundedBuffer(int capacity) : capacity(capacity), front(0), rear(0), count(0) {
        buffer.resize(capacity);
    }

    ~BoundedBuffer(){
    	buffer.clear();
    }

    void deposit(vector<string> data){
        unique_lock<mutex> l(lock);
        not_full.wait(l, [this](){return count != capacity; });

        buffer[rear] = data;
        rear = (rear + 1) % capacity;
        ++count;

        not_empty.notify_one();
    }

    vector<string> fetch(){
        unique_lock<mutex> l(lock);

        not_empty.wait(l, [this](){return count != 0; });

        vector<string> result = buffer[front];
        front = (front + 1) % capacity;
        --count;

        not_full.notify_one();

        return result;
    }
};

extern BoundedBuffer bBuffer;

#endif
