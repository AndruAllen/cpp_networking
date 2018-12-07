#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <errno.h>
#include <pthread.h>

using namespace std;

void* request_thread_function(void* arg) {
  /*
    Fill in this function.

    The loop body should require only a single line of code.
    The loop conditions should be somewhat intuitive.

    In both thread functions, the arg parameter
    will be used to pass parameters to the function.
    One of the parameters for the request thread
    function MUST be the name of the "patient" for whom
    the data requests are being pushed: you MAY NOT
    create 3 copies of this function, one for each "patient".
   */
    cout << "here" << endl;

    pthread_exit(NULL);
    return (void*)NULL;
}

int main(){
  vector<int> vals;

  /*
  int temp = 0;
  for(int i = 0; i < 10; i++){
    temp = i + 1 - 7 * i;
    vals.push_back(int(temp));
  }

  for(int i = 0; i < 10; i++){
    cout << &vals.at(i) << " : " << vals.at(i) << endl;
  }


  vector<pthread_t> joe;
  void* temp_crap;

  joe.push_back(pthread_t(0));
  cout << &joe.at(0) << " " << (*(&joe.at(0))) << endl;
  pthread_create(&joe.back(), NULL, &request_thread_function, temp_crap); // might be a problem using same temp vals

  cout << pthread_detach(joe.at(0)) << endl;

  joe.push_back(pthread_t(-911));
  cout << &joe.at(1) << " " << (joe.at(1)) << " " << (long(joe.back()) == -911) << endl;
  pthread_create(&joe.back(), NULL, &request_thread_function, temp_crap); // might be a problem using same temp vals

  cout << pthread_detach(joe.at(1)) << endl;
  */
  const char* junk = "8000";
  execl("dataserver", "dataserver", junk, (char*) NULL);

  
  exit(0);
  return 0;
}