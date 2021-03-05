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
#define backlog 3

#define FAILED_SOCKET 1
#define FAILED_BIND 2
#define FAILED_LISTEN 3
#define FAILED_ACCEPT 4
#define FAILED_CHILD 5

#define MESSAGE_LEN 1024


int main()
{
	int server_socket_fd = socket (AF_INET,SOCK_STREAM,0); //creating socket for IPv4 protocol , TCP conection  
	
	if(server_socket_fd < 0)
	{
		perror("socket failed");
		exit(FAILED_SOCKET);
	}
	
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET; // IPv4 
	server_address.sin_addr.s_addr = INADDR_ANY; // set for localhost 
	server_address.sin_port = htons(PORT); // a random port 
	
	int server_bind = bind (server_socket_fd , (struct sockaddr *) &server_address, sizeof(server_address));
	// bind the socket to the adress and port
	if(server_bind < 0) 
	{
		perror("bind failed");
		exit(FAILED_BIND);
	}
	
	if(listen (server_socket_fd , backlog) < 0)
	{
		perror("listen failed");
		exit(FAILED_LISTEN);
	}
	
	int adresslen = sizeof (server_address) ;
	int new_socket_fd;

	char message[MESSAGE_LEN];

	while(new_socket_fd = accept(server_socket_fd, (struct sockaddr *) &server_address, (socklen_t *) &adresslen))
	//wait for a connection from the client
	{
		if(new_socket_fd < 0)
		{	
			perror("accept failed");			
			exit(FAILED_ACCEPT); // client - server connection is the problem 
		}

		//Concurrent server= we shall use a child process for each user
		int cpid;

		//------------------------------<Child>----------------------
		if( (cpid=fork())==0 )
		{
			printf("Conexiune din partea clientului \n");
			//Child has connected

			close(server_socket_fd);
			//child does not need to listen

			bzero(message, MESSAGE_LEN);

			while(int n = recv(new_socket_fd, message, sizeof(message),0)) > 0 )
			{
				printf("Message from someone: %s", message);
				//print here

				send(new_socket_fd, message, strlen(message), 0);
				//send to all

				bzero(message, MESSAGE_LEN);
			}

			close(new_socket_fd);
			exit(0);
		}
		else
		{
			if(cpid<0)
			{
				perror("Could not create child D: !");			
				exit(FAILED_CHILD);
			}
		}
		//------------------------------</Child>--------------------
	}
	
	
	printf("Server End\n");
	
	return 0;
}