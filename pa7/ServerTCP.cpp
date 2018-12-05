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

void* server_thread_function(void* arg) {
    char buf [1024];
    int my_fd = *((int*) arg); // dereference the void pointer

    while(1){
        recv (my_fd, buf, sizeof (buf), 0);
        cout << "server: received msg: " << buf << endl;
    
        if(buf[0] == '!') { //!
            cout << "closing socket " << my_fd << endl;
            if(buf[1] == '!'){ //!!
                cout << "exitting" << endl;
                exit(0); // exit entire server, must join all client threads first so possib ERROR fixed if you want by pthread_cont_t and poling on server side
            }
            close(my_fd);
            break;
        } else {
            // send 
            char *msg = "Hello to you";
            dprintf (1, msg); // std output which should be cout?
            cout << " <- was sent" << endl;
            if (send(my_fd, msg, strlen(msg)+1, 0) == -1) {
                perror("send");
            } 
        }

    }

    pthread_exit(NULL);
    return (void*)NULL;
}

int server (char* port) {
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    new_fd = 0;
    struct addrinfo hints, *serv;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];
    int rv;
    //pthread_mutex_t server_shutdown = PTHREAD_MUTEX_INITIALIZER; // used for server shutdown if you want

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
    vector<pthread_t> server_threads;
    int ret_val;
	cout << "server: waiting for connections..." << endl;
	while(1) {  
        // main accept() loop
        sin_size = sizeof their_addr;
        if(new_fd == 0){
            new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
            if (new_fd == -1) {
                perror("accept");
                continue;
            } else {
                new_fds.push_back(int(new_fd)); // to ensure deep copy
                server_threads.push_back(pthread_t(0)); // to ensure deep copy
                ret_val = pthread_create(&(server_threads.back()), NULL, &server_thread_function, (void*)&(new_fds.back())); // might be a problem using same temp vals
                if(ret_val != 0){
                    cerr << "Seems like creating thread for " << new_fd << " failed " << server_threads.back() << endl;
                    exit(1);
                }
                ret_val = pthread_detach(server_threads.back());
                if(ret_val != 0){
                    cerr << "Seems like detaching thread for " << new_fd << " failed " << server_threads.back() << endl;
                    exit(1);
                }
                cout << "server: got connection at: " << new_fd << endl;
            }
        }
        new_fd = 0;
        //inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
    }
    close(sockfd);
    return 0;
}

int main (int ac, char ** av)
{
    if (ac < 2){
        cout << "Usage: ./server <port no> - try 127.0.0.1" << endl;
        exit (-1);
    }
	server (av [1]);
}
