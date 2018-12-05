#include "SafeBuffer.h"
#include <string>
#include <queue>
#include <iostream>
using namespace std;

SafeBuffer::SafeBuffer(const int& n) {
	mut = PTHREAD_MUTEX_INITIALIZER;
	cond_nf = PTHREAD_COND_INITIALIZER;
	cond_ne = PTHREAD_COND_INITIALIZER;
	max_size = n;
}

SafeBuffer::~SafeBuffer() {
	if(pthread_cond_destroy(&cond_nf) != 0){
		cerr << "couldn't destroy cond_nf in SafeBuffer" << endl;
	}
	if(pthread_cond_destroy(&cond_ne) != 0){
		cerr << "couldn't destroy cond_ne in SafeBuffer" << endl;
	}
	if(pthread_mutex_destroy(&mut) != 0){
		cerr << "couldn't destroy mut in SafeBuffer" << endl;
	}
}

int SafeBuffer::size() {
	/*
	Is this function thread-safe???
	Make necessary modifications to make it thread-safe
	*/
	pthread_mutex_lock(&mut);
	int out = q.size();
	pthread_mutex_unlock(&mut);
  return out;
}

void SafeBuffer::push(string str) {
	/*
	Is this function thread-safe???
	Make necessary modifications to make it thread-safe
	*/
	pthread_mutex_lock(&mut);
	while(q.size() >= max_size){ // different from SafeBuffer.size()
		pthread_cond_wait(&cond_nf, &mut);
	}
	q.push(str);
	pthread_cond_signal(&cond_ne);
	pthread_mutex_unlock(&mut);
}

string SafeBuffer::pop() {
	/*
	Is this function thread-safe???
	Make necessary modifications to make it thread-safe
	*/
	pthread_mutex_lock(&mut);
	while(q.size() <= 0){
		pthread_cond_wait(&cond_ne, &mut);
	}
	string s = q.front();
	q.pop();
	pthread_cond_signal(&cond_nf);
	pthread_mutex_unlock(&mut);
	return s;
}
