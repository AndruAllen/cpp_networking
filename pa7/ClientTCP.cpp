#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string>

using namespace std;

int client (char * server_name, char* port)
{
	struct addrinfo hints, *res;
	int sockfd;

	// first, load up address structs with getaddrinfo():

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	int status;
	//getaddrinfo("www.example.com", "3490", &hints, &res);
	if ((status = getaddrinfo(server_name, port, &hints, &res)) != 0) {
        cerr << "getaddrinfo: " << gai_strerror(status) << endl;
        return -1;
    }

	// make a socket:
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockfd < 0)
	{
		perror ("Cannot create scoket");	
		return -1;
	}

	// connect!
	if (connect(sockfd, res->ai_addr, res->ai_addrlen)<0)
	{
		perror ("Cannot Connect");
		return -1;
	}
	cout << "Successfully connected to " << server_name << endl;
	string temp = "";
	char buf [1024];
	while(1){
		cin >> temp;
		cout <<"Now Attempting to send a message "<< server_name << endl;
		if(temp == "!!" || temp == "!") {
			sprintf (buf, temp.c_str());
			send (sockfd, buf, strlen (buf)+1, 0);
		} else{
			const char* stuff = "test data from john here";
			temp = "test data from john here";
			cout << "testing: " << stuff << " == " << temp.c_str() << "?" << endl;
			sprintf (buf, temp.c_str());
			send (sockfd, buf, strlen (buf)+1, 0);
		}
		recv (sockfd, buf, 1024, 0);
		cout << "Received " << buf << " from the server" << endl;
		if(buf[0] == '!'){
			break;
		}
	}
	
	return 0;
}


int main (int ac, char** av)
{
	if (ac < 3){
        cout << "Usage: ./client <server name/IP> <port no>" << endl;
        exit (-1);
    }
	client (av [1], av [2]);
}