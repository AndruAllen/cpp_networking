#include <cassert>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <stdexcept>
#include <sys/ipc.h>

//#include "reqchannel.h"

using namespace std;

//#define FIFO_NAME "test.txt"

int main(){
	cout << "running..." << endl;

  int test = mkfifo("notworking", 0600);
  cout << test << endl;
  int rfd = open("notworking", O_RDWR);

  cout << "opened" << endl;

  unlink("notworking");
  
  //FIFOrc a("a_node", RequestChannel::CLIENT_SIDE);
  //MQrc b("b_node", RequestChannel::CLIENT_SIDE);
  //SHMrc c("c_node", RequestChannel::SERVER_SIDE);
	
  //string temp;
  //cout << "initialized" << endl;
  //cout << a.read_fd() << " " << a.write_fd() << endl;
  //cout << a.cwrite("hellow") << endl;
  //cin >> temp;

//	cout << a.name() << "\n" << a.cread() << endl;

	return 0;
}
