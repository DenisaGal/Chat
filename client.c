#include <stdio.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>

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
#define FAILED_SEND 9
#define FAILED_RECV 10
#define FAILED_USERNAME 11
#define FAILED_PASSWORD 12

#define MESSAGE_LEN 1024
#define OUT 50

int server_socket_fd = 0;
char username[20];
char password[21];
bool leave_flag = false;

void leave_chat(int signal){
	leave_flag = true;
}

void to_stdout(){
	printf("%s", "-> ");
  	fflush(stdout);
}

void add_nullchar(char* arr, int length){ //adds  \0 at the end of the string 
	int i=0;
	  
	while(arr[i]!='\n' && i<length){
		i++;
	}  
	arr[i]='\0'; 
}

void send_message_routine(){
	char message[MESSAGE_LEN] = {};
	char buff[MESSAGE_LEN + 25] = {};

	while(1){
		to_stdout();
  		fgets(message, MESSAGE_LEN, stdin);
  		add_nullchar(message, MESSAGE_LEN);

  		if(strcmp(message, "leave chat") == 0)
  			break;
  		else{
  			sprintf(buff, "%s: %s\n\n", username, message);
  			send(server_socket_fd, buff, strlen(buff), 0);
  		}

  		bzero(message, MESSAGE_LEN);
    	bzero(buff, MESSAGE_LEN + 25);
	}

	leave_chat(OUT);
}

void receive_message_routine(){
	char message[MESSAGE_LEN] = {};

	while(1){
		int receive = recv(server_socket_fd, message, MESSAGE_LEN, 0);

		if(receive > 0){
			printf("%s", message);
			to_stdout();
		}
		else if(receive == 0)
			break;

		memset(message, 0, sizeof(message));
	}
}

int main ()
{
	server_socket_fd = socket (AF_INET,SOCK_STREAM,0); //creating socket for IPv4 protocol , TCP conection  
	
	if(server_socket_fd < 0)
	{
		perror("socket failed");
		exit(FAILED_SOCKET);
	}

	//Get username
	printf("Enter your username please UwU: ");
	fgets(username, 20, stdin);
	add_nullchar(username, strlen(username));

	//Check if name fits
	if(strlen(username) > 20 || strlen(username) == 0){
		perror("Username must be less than 20 characters and it can't be NULL!");
		exit(FAILED_USERNAME);
	}
	
	//Get password
	printf("Now enter your password: ");
	fgets(password, 20, stdin);
	add_nullchar(password, strlen(password));

	//Check if password is ok
	if(strlen(password) > 20 || strlen(password) < 6 ){
		perror("Password must be between 6 and 20 characters!");
		exit(FAILED_PASSWORD);
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
        perror("Conection failed :(");
		exit(FAILED_CONNECTION);
    } 
    
    printf("Connected to server!\n");

    send(server_socket_fd, username, strlen(username), 0);
    send(server_socket_fd, password, strlen(password), 0);
    
    char message[MESSAGE_LEN];
    memset(message, '0', sizeof(memset));

    pthread_t send_message;
    pthread_t recieve_message;

	if(pthread_create(&send_message, NULL, (void *) send_message_routine, NULL) != 0){
		perror("Pthread");
		exit(FAILED_SEND);
	}


  	if(pthread_create(&recieve_message, NULL, (void *) receive_message_routine, NULL) != 0){
		perror("Pthread failed ");
		exit(FAILED_RECV);
	}

	while(1){
		if(leave_flag){
			printf("ByeBye\n");
			break;
		}
	}
    
    // while(1){
    // 	printf("Write your message:\n");
    // 	bzero(message, MESSAGE_LEN);
    	
    // 	fgets(message, MESSAGE_LEN - 1, stdin);
    	
    // 	//Send message to server
    // 	//printf("\nSending to SERVER: %s ", message);
    	
    // 	int n;
    // 	if((n = send(server_socket_fd, message, strlen(message), 0)) < 0){
    // 		perror("Error writing to server T_T\n");
    // 		exit(FAILED_WRITE);
    // 	}
    	
    // 	/*//Read server response
    // 	bzero(message, MESSAGE_LEN);
    // 	if((n = recv(server_socket_fd, message, MESSAGE_LEN - 1, 0)) < 0){
    // 		perror("Error reading from server :C\n");
    // 		exit(FAILED_READ);	
    // 	}
    	
    // 	printf("\nReceived from server: %s", message);*/
    
    // }

    close(server_socket_fd);

	return 0;
}
