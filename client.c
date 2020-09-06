//////////////////////////////////////////////////////////////////////
// File Name : client.c                                             //
// Date      : 2018/4/27                                            //
// Os        : Ubuntu 16.04 LTS 64bits                              //
// Author    : CHoi Yeon Hee                                        //
// Student ID: 2016722035                                           //
// -----------------------------------------------------------------//
// Title : System Programming Assignment #2-1                       //
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFSIZE 1024 // buffer size is 1024
#define PORTNO 38039 // server's port number to connect is 38039


/////////////////////////////////////////////////////////////////////
//Function name : main                                             //
//input : variable                                                 //
//output : int                                                     //
//Purpose : Echo Client                                            //
/////////////////////////////////////////////////////////////////////

int main()
{
	int socket_fd, len; // socket_fd = nonnegative descriptor, length of recieve text from user
	struct sockaddr_in server_addr; // server address struct variable
	char haddr[] = "127.0.0.1"; // local ip address : server's ip address exactly
	char buf[BUFFSIZE]; // buffer which size is 1024

	if((socket_fd = socket(PF_INET, SOCK_STREAM,0)) <0) // if socket error
	{ 
		printf("can't create socket.\n"); // print erorr phrase
		return -1; // exist with '-1' return value
	}

	// initialize server's struct variable
	bzero(buf, sizeof(buf)); // initialize buffer by 0
	bzero((char*)&server_addr, sizeof(server_addr)); // initialize struct by 0
	server_addr.sin_family = AF_INET; // server address's family is AF_INET
	server_addr.sin_addr.s_addr = inet_addr(haddr); // server's address initialize
	server_addr.sin_port = htons(PORTNO); // server's port number initialize
	// connect
	if(connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) 
	{ // if connect error
		printf("can't connect.\n"); // print error sentence
		return -1; // exist with '-1' return value
	}

	write(STDOUT_FILENO, "input URL> ",11); // write at stdout
	while ( (len = read(STDIN_FILENO, buf, sizeof(buf))) > 0)  // while if input exist
	{  
		if (!strncmp(buf, "bye", 3)) break; // if buf is "bye", disconnect

		if(write(socket_fd, buf, strlen(buf)) > 0 )  // write recieve text to server
		{ 
		 	if((len = read(socket_fd, buf, sizeof(buf))) > 0 ) // read from server 
			{
				write(STDOUT_FILENO, buf, len); // write recieve the text to stdout
				bzero(buf, sizeof(buf)); // initialize buf by 0
			}
		}
		write(STDOUT_FILENO, "input URL> ",11); // write to stdout
	}
	// if user enter "bye" into command
	close(socket_fd); // socket close or connection exist
	return 0; // exist normally
}


