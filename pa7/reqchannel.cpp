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

const int MAX_MESSAGE = 255;

void* server_thread_function(void* arg) {
    char buf [MAX_MESSAGE];
    int my_fd = *((int*) arg); // dereference the void pointer
    char* dt = "data";
    while(1){
        recv (my_fd, buf, sizeof (buf), 0);
        //cout << "server: received msg: " << buf << endl;
    
        if(buf[0] == '!') { //!
            //pscout << "closing socket " << my_fd << endl;
            if(buf[1] == '!'){ //!!
                cout << "exitting" << endl;
                exit(0); // exit entire server, must join all client threads first so possib ERROR fixed if you want by pthread_cont_t and poling on server side
            }
            close(my_fd);
            break;
        } else {
            // send
        		if (buf[0] == 'd' && buf[1] == 'a' && buf[2] == 't' && buf[3] == 'a') { // data cause I'm lazy
							usleep(1000 + (rand() % 5000));
							string trach = to_string(rand() % 100);
							const char *msg = trach.c_str();
							if (send(my_fd, msg, trach.size()+1, 0) == -1) {
                perror("send");
            	} 
						} else {
							const char *msg = "unknown stuff";
							cerr << msg << " <- you've screwed up because you didn't send data: " << buf << endl;
							if (send(my_fd, msg, strlen(msg)+1, 0) == -1) {
                perror("send");
            	} 
						}
        }
    }

    pthread_exit(NULL);
    return (void*)NULL;
}

////////////////////////////////////////////////// NETWORK FUNCs

NetworkRequestChannel::NetworkRequestChannel(const string _name, const Side _side, char* _server_address, char* _server_port) : RequestChannel(_name, _side) {
	server_port = _server_port;

	if (_side == RequestChannel::SERVER_SIDE) { // server

    //pthread_mutex_t server_shutdown = PTHREAD_MUTEX_INITIALIZER; // used for server shutdown if you want
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, server_port, &hints, &serv)) != 0) {
        cerr  << "getaddrinfo: " << gai_strerror(rv) << endl;
        exit(1);
    }
		if ((my_fd = socket(serv->ai_family, serv->ai_socktype, serv->ai_protocol)) == -1) {
	        perror("server: socket");
					exit(1);
	    }
	    if (::bind(my_fd, serv->ai_addr, serv->ai_addrlen) == -1) {
			close(my_fd);
			perror("server: bind");
			exit(1);
		}
    freeaddrinfo(serv); // all done with this structure
    if (listen(my_fd, 300) == -1) {
        perror("listen");
        exit(1);
    }
		
	} else { // client
		
		server_address = _server_address;
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		int status;
		//getaddrinfo("www.example.com", "3490", &hints, &res);
		if ((status = getaddrinfo(server_address, server_port, &hints, &serv)) != 0) {
	        cerr << "getaddrinfo: " << gai_strerror(status) << endl;
	        exit(1);
	    }

		// make a socket:
		my_fd = socket(serv->ai_family, serv->ai_socktype, serv->ai_protocol);
		if (my_fd < 0)
		{
			perror ("Cannot create scoket");	
			exit(1);
		}

		// connect!
		if (connect(my_fd, serv->ai_addr, serv->ai_addrlen)<0)
		{
			perror ("Cannot Connect");
			exit(1);
		}
		//cout << "Successfully connected to " << server_address << endl;
		}
}

NetworkRequestChannel::~NetworkRequestChannel() {
	//if (my_side == SERVER_SIDE) {
	//	close(my_fd);
	//}
}



int NetworkRequestChannel::handle_process_loop() {
	int new_fd = 0;
	vector<int> new_fds;
  vector<pthread_t> server_threads;
  int ret_val;
	//cout << "server: waiting for connections..." << endl;
	while(1) {  
        // main accept() loop
        sin_size = sizeof their_addr;
        if(new_fd == 0){
            new_fd = accept(my_fd, (struct sockaddr *)&their_addr, &sin_size);
            if (new_fd == -1) {
                perror("accept");
                continue;
            } else {
                new_fds.push_back(int(new_fd)); // to ensure deep copy
                server_threads.push_back(pthread_t(0)); // to ensure deep copy
                ret_val = pthread_create(&(server_threads.back()), NULL, &server_thread_function, (void*)&(new_fds.back())); // might be a problem using same temp vals
                if(ret_val != 0) {
                    cerr << "Seems like creating thread for " << new_fd << " failed " << server_threads.back() << endl;
                    exit(1);
                }
                ret_val = pthread_detach(server_threads.back());
                if(ret_val != 0) {
                    cerr << "Seems like detaching thread for " << new_fd << " failed " << server_threads.back() << endl;
                    exit(1);
                }
                //cout << "server: got connection at: " << new_fd << endl;
            }
        }
        new_fd = 0;
        //inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
  }
  close(my_fd);
  return 0;

}

string NetworkRequestChannel::cread() {

	char buf [MAX_MESSAGE];
	if (recv(my_fd, buf, MAX_MESSAGE, 0) <= 0) {
		execl("rmv", (char*) NULL);
		EXITONERROR ("cread");
	}
	string s = std::string(buf);
	return s;

}

int NetworkRequestChannel::cwrite(string msg) {

	if (msg.size() > MAX_MESSAGE) {
		execl("rmv", (char*) NULL);
		return -2;
		EXITONERROR ("cwrite");
	}
	char buf [MAX_MESSAGE];
	sprintf (buf, msg.c_str());

	if (send(my_fd, buf, msg.size()+1, 0) < 0) { // msg.size() + 1 to include the NULL byte
		cerr << buf << endl;
		execl("rmv", (char*) NULL);
		return -1;
		EXITONERROR ("cwrite");
	}

	return 0;
}


