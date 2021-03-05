#include <stdio.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define PORT 5555 // some port for the server 
#define backlog 3

#define FAILED_SOCKET 1
#define FAILED_BIND 2
#define FAILED_LISTEN 3
#define FAILED_ACCEPT 4
#define FAILED_ADDRESS 5
#define FAILED_CONNECTION 6

/*
Just an empty file for now 
*/

int main ()
{
	int server_socket_fd = socket (AF_INET,SOCK_STREAM,0); //creating socket for IPv4 protocol , TCP conection  
	
	if(server_socket_fd < 0)
	{
		perror("socket failed");
		exit(FAILED_SOCKET);
	}
	
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET; // IPv4
	server_address.sin_port = htons(PORT); // a random port 

	if(inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr)<=0)  
    { 
        perror("Address not supported :(");
		exit(FAILED_ADDRESS);
    } 

    if (connect(server_socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) 
    { 
        perror("Conection failed :(");
		exit(FAILED_CONNECTION);
    } 

	return 0;
}