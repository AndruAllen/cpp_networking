
#ifndef _reqchannel_H_                   
#define _reqchannel_H_

#include <iostream>
#include <fstream>
#include <exception>
#include <string>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include <csignal>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <errno.h>
#include <pthread.h>

using namespace std;

void EXITONERROR (string msg);

class RequestChannel {

public:

	typedef enum {SERVER_SIDE, CLIENT_SIDE} Side;

	typedef enum {READ_MODE, WRITE_MODE} Mode;

protected:

	string   my_name = "";
	string side_name = "";
	Side     my_side;
	
public:

	/* -- CONSTRUCTOR/DESTRUCTOR */
	RequestChannel(const string _name, const Side _side) :
	my_name(_name), my_side(_side), side_name((_side == RequestChannel::SERVER_SIDE) ? "SERVER" : "CLIENT"){
		//cout << "starting base construction" << endl;
	}
	/* Creates a "local copy" of the channel specified by the given name. 
	 If the channel does not exist, the associated IPC mechanisms are 
	 created. If the channel exists already, this object is associated with the channel.
	 The channel has two ends, which are conveniently called "SERVER_SIDE" and "CLIENT_SIDE".
	 If two processes connect through a channel, one has to connect on the server side 
	 and the other on the client side. Otherwise the results are unpredictable.

	 NOTE: If the creation of the request channel fails (typically happens when too many
	 request channels are being created) and error message is displayed, and the program
	 unceremoniously exits.

	 NOTE: It is easy to open too many request channels in parallel. In most systems,
	 limits on the number of open files per process limit the number of established
	 request channels to 125.
	*/

	virtual ~RequestChannel(){}
	/* Destructor of the local copy of the bus. By default, the Server Side deletes any IPC 
	 mechanisms associated with the channel. */

	virtual string cread() = 0;
	/* Blocking read of data from the channel. Returns a string of characters
	 read from the channel. Returns NULL if read failed. */

	virtual int cwrite(string _msg) = 0;
	/* Write the data to the channel. The function returns the number of characters written
	 to the channel. */

	string name(){ return my_name; }
	/* Returns the name of the request channel. */
};


class NetworkRequestChannel: public RequestChannel{

private:
		/*  The current implementation uses named pipes. */ 
	int my_fd;
	char* server_address;
	char* server_port;

  struct addrinfo hints, *serv;
  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size;
  char s[INET6_ADDRSTRLEN];
  int rv;

	//string pipe_name(Mode _mode);
	//void create_pipe (string _pipe_name);
	//void open_read_pipe(string _pipe_name);
	//void open_write_pipe(string _pipe_name)

public:
	NetworkRequestChannel(const string _name, const Side _side, char* _server_address, char* _server_port);
	~NetworkRequestChannel();

	string cread();
	int cwrite(string _msg);
	int handle_process_loop();

	//////////////////////////////////// special functions
	// could be private
	//string pipe_name(Mode _mode);
	//void create_pipe(string _pipe_name);
	//void open_write_pipe(string _pipe_name);
	//void open_read_pipe(string _pipe_name);

	// should be public
	//int read_fd(){ return my_fd; }
	/* Returns the file descriptor used to read from the channel. */

	//int write_fd(){ return my_fd; }
	/* Returns the file descriptor used to write to the channel. */

};


#endif


