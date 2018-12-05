/* 
    File: requestchannel.C

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 2012/07/11

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

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

#include "reqchannel.h"

void EXITONERROR (string msg){
	execl("rmv", (char*) NULL);
	perror (msg.c_str());
	exit (-1);
}

////////////////////////////////////////////////// SPECIAL FIFO FUNCs

string FIFOrc::pipe_name(Mode _mode) {
	string pname = "fifo_" + my_name;

	if (my_side == RequestChannel::CLIENT_SIDE) {
		if (_mode == RequestChannel::READ_MODE)
			pname += "1";
		else
			pname += "2";
	}
	else {
	/* SERVER_SIDE */
		if (_mode == READ_MODE)
			pname += "2";
		else
			pname += "1";
	}
	return pname;
}

void FIFOrc::create_pipe(string _pipe_name){
	try{
		if(mkfifo(_pipe_name.c_str(), 0600) < 0){
			cerr << "couldn't make pipe: " << _pipe_name << endl;
		} 
	} catch(...){
		execl("rmv", (char*) NULL);
	}
	//{
	//	EXITONERROR (_pipe_name);
	//}
}

void FIFOrc::open_write_pipe(string _pipe_name) {
	
	//if (my_side == SERVER_SIDE)
		create_pipe (_pipe_name);

	wfd = open(_pipe_name.c_str(), O_WRONLY);
	if (wfd < 0) {
		execl("rmv", (char*) NULL);
		EXITONERROR (_pipe_name);
	}
}

void FIFOrc::open_read_pipe(string _pipe_name) {

	//if (my_side == SERVER_SIDE)
 	try{
		create_pipe (_pipe_name);
		rfd = open(_pipe_name.c_str(), O_RDONLY);
		cout << "here" << endl;
		if (rfd < 0) {
			cerr << "failed to open" << endl;
			execl("rmv", (char*) NULL);
			perror ("");
			exit (0);
		}
	} catch(...){
		cerr << "caught" << endl;
		execl("rmv", (char*) NULL);
	}
}

////////////////////////////////////////////////// STANDARD FIFO FUNCs

FIFOrc::FIFOrc(const string _name, const Side _side) : RequestChannel(_name, _side){
	if (_side == RequestChannel::SERVER_SIDE) {
		open_write_pipe(pipe_name(RequestChannel::WRITE_MODE).c_str());
		open_read_pipe(pipe_name(RequestChannel::READ_MODE).c_str());
	}
	else {
		cout << "read" << endl;
		open_read_pipe(pipe_name(RequestChannel::READ_MODE).c_str());
		cout << "write" << endl;
		open_write_pipe(pipe_name(RequestChannel::WRITE_MODE).c_str());
	}
}

FIFOrc::~FIFOrc() {
	close(wfd);
	close(rfd);
	//if (my_side == SERVER_SIDE) {
		remove(pipe_name(RequestChannel::READ_MODE).c_str());
		remove(pipe_name(RequestChannel::WRITE_MODE).c_str());
	//}
}

const int MAX_MESSAGE = 255;

string FIFOrc::cread() {

	char buf [MAX_MESSAGE];
	if (read(rfd, buf, MAX_MESSAGE) <= 0) {
		execl("rmv", (char*) NULL);
		EXITONERROR ("cread");
	}
	string s = buf;
	return s;

}

int FIFOrc::cwrite(string msg) {

	if (msg.size() > MAX_MESSAGE) {
		execl("rmv", (char*) NULL);
		return -2;
		EXITONERROR ("cwrite");
	}
	if (write(wfd, msg.c_str(), msg.size()+1) < 0) { // msg.size() + 1 to include the NULL byte
		execl("rmv", (char*) NULL);
		return -1;
		EXITONERROR ("cwrite");
	}

	return 0;
}


