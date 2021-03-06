#include <stdio.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5555 // some port for the server 
#define backlog 3

#define FAILED_SOCKET 1
#define FAILED_BIND 2
#define FAILED_LISTEN 3
#define FAILED_ACCEPT 4
#define FAILED_ADDRESS 5
#define FAILED_CONNECTION 6
#define FAILED_WRITE 7
#define FAILED_READ 8

#define MESSAGE_LEN 1024

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
	server_address.sin_port = htons(PORT); //our defined port

	if(inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr)<=0)  
    { 
        perror("Address not supported :(");
		exit(FAILED_ADDRESS);
    } 

    if (connect(server_socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) 
    { 
        perror("Conection failed :(\n");
		exit(FAILED_CONNECTION);
    } 
    
    printf("Connected to server!\n");
    
    char message[MESSAGE_LEN];
    memset(message, '0', sizeof(memset));
    
    while(1){
    	printf("Write your message:\n");
    	bzero(message, MESSAGE_LEN);
    	
    	fgets(message, MESSAGE_LEN - 1, stdin);
    	
    	//Send message to server
    	//printf("\nSending to SERVER: %s ", message);
    	
    	int n;
    	if((n = send(server_socket_fd, message, strlen(message), 0)) < 0){
    		perror("Error writing to server T_T\n");
    		exit(FAILED_WRITE);
    	}
    	
    	//Read server response
    	bzero(message, MESSAGE_LEN);
    	if((n = recv(server_socket_fd, message, MESSAGE_LEN - 1, 0)) < 0){
    		perror("Error readin from server :C\n");
    		exit(FAILED_READ);	
    	}
    	
    	printf("\nReceived from server: %s", message);
    
    }

	return 0;
}
