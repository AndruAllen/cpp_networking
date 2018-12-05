#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
struct addrinfo* DNSLookup (char * hostname, char* port)
{
	int status;
	struct addrinfo hints;
	struct addrinfo *res;  // will point to the results
	struct addrinfo *p;

	//preparing hints data structure
	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

	// look up the IP address from the name: "www.example.com"
	status = getaddrinfo(hostname, port, &hints, &res);
	if (status != 0)
		return NULL;
	
	return res;	
}

int main (int ac, char ** av)
{
	struct addrinfo * res = DNSLookup (av [1], av [2]);  // lookup using name and port number
	if (res == NULL)
	{
		printf ("ERROR:: Look up failed\n");
		return 0;
	}
	else
	{
		printf ("Look up succeeded\n");		
	}
	int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockfd == -1)
	{
		printf ("ERROR:: cannot create socket\n");
		return 0;		
	}
	else
	{
		printf ("Socket created\n");
	}
	int status = connect(sockfd, res->ai_addr, res->ai_addrlen);
	if (status == -1)
	{
		printf ("ERROR:: cannot connect\n");
		return 0;		
	}
	else
	{
		printf ("Successfully connected to PORT: %s on SERVER: %s\n", av [2], av [1]);
	}
	
	return 0;
}
