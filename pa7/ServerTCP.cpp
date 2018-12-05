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

int server (char* port)
{
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    new_fd = 0;
    struct addrinfo hints, *serv;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, port, &hints, &serv)) != 0) {
        cerr  << "getaddrinfo: " << gai_strerror(rv) << endl;
        return -1;
    }
	if ((sockfd = socket(serv->ai_family, serv->ai_socktype, serv->ai_protocol)) == -1) {
        perror("server: socket");
		return -1;
    }
    if (bind(sockfd, serv->ai_addr, serv->ai_addrlen) == -1) {
		close(sockfd);
		perror("server: bind");
		return -1;
	}
    freeaddrinfo(serv); // all done with this structure

    if (listen(sockfd, 20) == -1) {
        perror("listen");
        exit(1);
    }
	vector<int> new_fds;
	cout << "server: waiting for connections..." << endl;
    char buf [1024];
	while(1) 
	{  
        // main accept() loop
        sin_size = sizeof their_addr;
        if(new_fd == 0){
            new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
            new_fds.push_back(int(new_fd));
            cout << "server: got connection at: " << new_fd << endl;
        }
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        new_fd = 0;
        //inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		recv (new_fd, buf, sizeof (buf), 0);
		cout << "server: received msg: " << buf << endl;
		
        if(buf[0] == '!'){ //!
            cout << "closing socket" << endl;
            close(new_fd);
            new_fd = 0;
            if(buf[1] == '!'){ //!!
                cout << "exitting" << endl;
                break;
            }
        } else {
		  // send 
		  char *msg = "Hello to you";
          dprintf (1, msg); // std output which should be cout?
          cout << " <- was sent" << endl;
          if (send(new_fd, msg, strlen(msg)+1, 0) == -1) {
            perror("send");
          }	
        }
    }

    return 0;
}

int main (int ac, char ** av)
{
    if (ac < 2){
        cout << "Usage: ./server <port no>" << endl;
        exit (-1);
    }
	server (av [1]);	
}
