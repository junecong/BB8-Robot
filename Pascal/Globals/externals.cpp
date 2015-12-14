#include "externals.h"

bool debugMode = false;
bool sendMode = false;
bool localhostMode = false;

BoundedBuffer::BoundedBuffer(int capacity) : capacity(capacity), front(0), rear(0), count(0) {
    buffer.resize(capacity);
}

BoundedBuffer::~BoundedBuffer(){
	buffer.clear();
}

void BoundedBuffer::deposit(vector<string> data){
    unique_lock<mutex> l(lock);
    not_full.wait(l, [this](){return count != capacity; });

    buffer[rear] = data;
    rear = (rear + 1) % capacity;
    ++count;

    not_empty.notify_one();
}

vector<string> BoundedBuffer::fetch(){
    unique_lock<mutex> l(lock);

    not_empty.wait(l, [this](){return count != 0; });

    vector<string> result = buffer[front];
    front = (front + 1) % capacity;
    --count;

    not_full.notify_one();

    return result;
}
