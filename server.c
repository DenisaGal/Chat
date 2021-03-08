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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/md5.h>

#define PORT 5555 // some port for the server 
#define backlog 3

#define FAILED_SOCKET 1
#define FAILED_BIND 2
#define FAILED_LISTEN 3
#define FAILED_ACCEPT 4
#define FAILED_RECV 5
#define FAILED_WRITE 7
#define FILE_ERROR 8

#define max_users 10// max nr of users in chat 
#define MESSAGE_LEN 1024
#define username_len 20
#define pwd_len 35

typedef struct{
	struct sockaddr_in address;
	int sockfd;
	int uid;
	char name[username_len];
} client; // structure of a client

client *array[max_users];
pthread_mutex_t lock;
int file_descriptor;

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
	pthread_mutex_lock(&lock);
	
	for(int i=0; i < max_users; i++){
		if(array[i]){
			if(array[i]->uid == uid)
			{
				array[i] = NULL;
				break;
			}
		}
	}

	pthread_mutex_unlock(&lock);
}

void send_message(char *s,int uid) //send messages to clients
{
	pthread_mutex_lock(&lock);
	
	for(int i=0;i< max_users ; i++)
	{
		if(array[i])
		{
			//printf("%s\n",s);
			
			//if(uid != array[i] -> uid)
			//{
				if( send(array[i] -> sockfd , s , strlen(s) , 0) < 0)
				{
					perror("error send");
					exit(FAILED_WRITE);
				}
			//}
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

char md5hash[33];
char* hash(char pwd[pwd_len]){
	unsigned char digest[MD5_DIGEST_LENGTH];
	MD5_CTX context;
	MD5_Init(&context);
	MD5_Update(&context, pwd, strlen(pwd));
	MD5_Final(digest, &context);
	
	
	for(int i = 0; i < 16; ++i)
		sprintf(&md5hash[i*2], "%02x", (unsigned int)digest[i]);
			
	return md5hash;
}

int username_exists(char username[username_len]){
	FILE *file_stream;
	if((file_stream = fopen("database", "r")) == NULL){
		perror("Opening stream");
		exit(FILE_ERROR);
	}
	
	fseek(file_stream, 0, SEEK_SET);
	
	char *line = NULL;
	size_t len = 0;
	
	while(getline(&line, &len, file_stream) != -1){
		add_nullchar(line, len);
		
		if(strcmp(username, line) == 0)
			return 1;
			
		getline(&line, &len, file_stream); //next line is pwd and we don't need it
	}
	
	fclose(file_stream);
	return 0;
}

int credentials_match(char username[username_len], char password[pwd_len]){
	FILE *file_stream;
	if((file_stream = fopen("database", "r")) == NULL){
		perror("Opening stream");
		exit(FILE_ERROR);
	}
	
	fseek(file_stream, 0, SEEK_SET);
	
	char *line = NULL;
	size_t len = 0;
	
	while(getline(&line, &len, file_stream) != -1){
		add_nullchar(line, len);
		
		if(strcmp(username, line) == 0){
			getline(&line, &len, file_stream); //get pwd
			add_nullchar(line, len);
			
			if(strcmp(password, line) == 0)
				return 1;
			else
				return 0;
		}
	}
	
	fclose(file_stream);
	return 1;
}

int is_already_connected(char username[username_len]){
	for(int i = 0; i < max_users; i++)
		if(array[i])
			if(strcmp(array[i]->name, username) == 0)
				return 1;
		
	return 0;
}

void *client_routine(void *arg)
{
	char buff[MESSAGE_LEN];
	bool leave_flag = false;
	client *client_user = (client *)arg;
	char username[username_len];
	char password[pwd_len];
	int logged_in = 0;
	
	/*  Check the username password etc  */ 
	while(!logged_in){
		bzero(username, username_len);
		bzero(password, pwd_len);
		
		if(recv(client_user ->sockfd, username, username_len, 0) <= 0 ){
			printf("Username fail\n"); 
			leave_flag = true;
		}
		
		if(recv(client_user ->sockfd, password, pwd_len, 0) <= 0 ){
			printf("Password fail\n"); 
			leave_flag = true;
		}
		
		//can't login at the same time with the same user again
		if(is_already_connected(username)){
			send(client_user -> sockfd, "e1" , 2, 0);
			continue;
		}
		
		//daca e user nou il bagam in database
		if(!username_exists(username)){
			logged_in = 1;
			char credentials[100];
			snprintf(credentials, sizeof(credentials), "%s\n%s\n", username, password);
			if(write(file_descriptor, credentials, strlen(credentials)) < 0){
				perror("write");
				exit(FILE_ERROR);	
			}
		}
		else{		
			//cauta username-ul in database si verifica sa fie match cu parola
			if(credentials_match(username, password))
				logged_in = 1;
		}
		
		char log_msg[10];
		memset(log_msg, '\0', 10);
		if(logged_in)
			sprintf(log_msg, "%s", "ok");			
		else
			sprintf(log_msg, "%s", "e2");
		send(client_user -> sockfd, log_msg , strlen(log_msg), 0);
	}
	
	
	//logged in
	strcpy(client_user -> name, username);
	sprintf(buff,"%s has joined the chat\n",client_user -> name);
	printf("%s\n",buff);
	send_message(buff, client_user ->uid);
	
	
	/*****************************************/
	
	bzero(buff,MESSAGE_LEN);
	
	while(1)
	{	
		bzero(buff,MESSAGE_LEN);
	
		if (leave_flag == true) 
		{
			break;
		}
		
		int msg_received=recv(client_user -> sockfd,buff,MESSAGE_LEN,0);
		
		add_nullchar(buff,MESSAGE_LEN);
		
		if(msg_received > 0)
		{	
			send_message(buff,client_user ->uid);
			add_nullchar(buff,MESSAGE_LEN);
			printf("%s\n",buff);
			
		}
		else if (msg_received == 0 || strcmp(buff,"leave chat") == 0 )
		{
			
			printf("%s has left the chat\n",client_user ->name);
			leave_flag=true;
			sprintf(buff,"%s has left the chat",client_user ->name);
			send_message(buff, client_user -> uid );
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
	/*************** DATABASE FILE **************/
	
	if((file_descriptor = open("database", O_WRONLY | O_APPEND | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO)) < 0){
		perror("Opening file");
		exit(FILE_ERROR);
	}
	
	
	/*************** DATABASE FILE **************/
	

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
	close(file_descriptor);
	
	return 0;
}
