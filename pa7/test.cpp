#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <vector>
using namespace std;

int main(){
  vector<int> vals;

  int temp = 0;
  for(int i = 0; i < 10; i++){
    temp = i + 1 - 7 * i;
    vals.push_back(int(temp));
  }

  for(int i = 0; i < 10; i++){
    cout << &vals.at(i) << " : " << vals.at(i) << endl;
  }

  return 0;
}