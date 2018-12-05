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
	{
		printf ("Lookup failed \n");	
		return NULL;
	}

	for(p = res;p != NULL; p = p->ai_next) {
		void *addr;
		char *ipver;
		// get the pointer to the address itself,
		// different fields in IPv4 and IPv6:
		if (p->ai_family == AF_INET) { // IPv4
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
			addr = &(ipv4->sin_addr);
			ipver = "IPv4";
		} else { // IPv6
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
			addr = &(ipv6->sin6_addr);
			ipver = "IPv6";
		}
		// convert the IP to a string and print it:
		char ipstr [1024];
		inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
		printf("  %s: %s\n", ipver, ipstr);
	}
	return res;	
}

int main (int ac, char ** av)
{
	DNSLookup (av [1], 0);  // just lookup
	return 0;
}
