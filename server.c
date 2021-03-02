/*
A simple server for the chat

*/

#include <stdio.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdlib.h>

#define PORT 5555 // some port for the server 

#define FAILED_SOCKET 1
#define FAILED_BIND 2


int main()
{
	int sockfd = socket (AF_INET,SOCK_STREAM,0); //creating socket for IPv4 protocol , TCP conection  
	
	if(sockfd < 0)
	{
		perror("socket failed");
		exit(FAILED_SOCKET);
	}
	
	struct sockaddr_in server_address;

	server_address.sin_family = AF_INET; // IPv4 
    server_address.sin_addr.s_addr = INADDR_ANY; // set for localhost 
    server_address.sin_port = htons(PORT); // a random port 
	
	int server_bind = bind (sockfd , (struct sockaddr *) &server_address, sizeof(server_address));
	// bind the socket to the adress and port
	if(server_bind < 0) 
	{
		perror("bind failed");
		exit(FAILED_BIND);
	}
	
	printf("Hello World !\n");
	
	return 0;
}