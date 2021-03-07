/*
A simple server for the chat

*/

#include <stdio.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#define PORT 5555 // some port for the server 
#define backlog 3

#define FAILED_SOCKET 1
#define FAILED_BIND 2
#define FAILED_LISTEN 3
#define FAILED_ACCEPT 4
#define FAILED_RECV 5
#define FAILED_WRITE 7

#define max_users 10// max nr of users in chat 
#define MESSAGE_LEN 1024
#define username_len 20

typedef struct{
	struct sockaddr_in address;
	int sockfd;
	int uid;
	char name[username_len];
} client; // structure of a client

client *array[max_users];
pthread_mutex_t lock;

/* Add clients to queue */
void queue_add(client *cl)
{
	
	pthread_mutex_lock(&lock);

	for(int i=0; i < max_users; i++)
	{
		if(array[i] ==  NULL)
		{
			array[i] = cl;
				break;
		}
	}

	pthread_mutex_unlock(&lock);
}


void queue_remove(int uid) // remove clients from queue
{
	
}

void send_message(char *s) //send messages to clients
{
	pthread_mutex_lock(&lock);
	
	for(int i=0;i< max_users ; i++)
	{
		if(array[i])
		{
			//printf("%s\n",s);
			
			if( send(array[i] -> sockfd , s , strlen(s) , 0) < 0)
			{
				perror("error send");
				exit(FAILED_WRITE);
			}
			
		}
	}
	
	pthread_mutex_unlock(&lock);
}

void add_nullchar (char* arr, int length) { //adds  \0 at the end of the string 
 
	int i=0;
	  
	while(arr[i]!='\n' && i<length)
	{
		i++;
	}
  
	arr[i]='\0';
  
}

void *client_routine(void *arg)
{
	char buff[MESSAGE_LEN];
	bool leave_flag=false;
	client *client_user = (client *)arg;
	
	/*  Check the username password etc  */
	
	
	/*****************************************/
	
	bzero(buff,MESSAGE_LEN);
	
	while(1)
	{
		if (leave_flag == true) 
		{
			break;
		}
		
		int msg_received=recv(client_user -> sockfd,buff,MESSAGE_LEN,0);
		
		add_nullchar(buff,MESSAGE_LEN);
		//printf("%s\n",buff);
		if(msg_received > 0)
		{	
			send_message(buff);
			add_nullchar(buff,MESSAGE_LEN);
			printf("%s\n",buff);
			
		}
		else if (msg_received == 0)
		{
			printf("Client has left");
			leave_flag=true;
		}
		else
		{
			perror("client recv ");
			exit(FAILED_RECV);
		}
	
	}
	queue_remove(client_user->uid);
	free(client_user);
	pthread_detach(pthread_self());
	
	return NULL;
}

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

	struct sockaddr_in client_address;
	pthread_t thread_id;
	int uid=1; // user id
	
	while(1)
	//wait for a connection from the client
	{
		new_socket_fd = accept(server_socket_fd, (struct sockaddr *) &client_address, (socklen_t *) &adresslen);
		
		if(new_socket_fd < 0)
		{	
			perror("accept failed");			
			exit(FAILED_ACCEPT); // client - server connection is the problem 
		}
		
		client *client_user = (client *) malloc (sizeof (client));
		
		client_user->address = client_address;
		client_user->sockfd = new_socket_fd;
		client_user->uid = uid;
		uid=uid + 1;
		
		queue_add(client_user);
		pthread_create(&thread_id,NULL,&client_routine,(void *)client_user);

	}
	
	
	printf("Server End\n");
	
	return 0;
}
