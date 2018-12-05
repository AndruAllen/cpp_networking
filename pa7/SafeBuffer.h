#ifndef SafeBuffer_h
#define SafeBuffer_h

#include <iostream>
#include <stdio.h>
#include <queue>
#include <pthread.h>
#include <string>
using namespace std;

class SafeBuffer {
private:
  pthread_cond_t cond_nf, cond_ne;
  pthread_mutex_t mut;

  int max_size;
	queue<string> q;	
public:
  SafeBuffer(const int& n);
	~SafeBuffer();
	int size();
  void push (string str);
  string pop();
};

#endif /* SafeBuffer_ */
