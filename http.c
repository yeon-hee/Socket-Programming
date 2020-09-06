//////////////////////////////////////////////////////////////////////
// File Name : http.c                                               //
// Date      : 2018/5/2                                             //
// Os        : Ubuntu 16.04 LTS 64bits                              //
// Author    : CHoi Yeon Hee                                        //
// Student ID: 2016722035                                           //
// -----------------------------------------------------------------//
// Title : Construction Proxy Connection #2-2                       //
//////////////////////////////////////////////////////////////////////


#include <stdio.h> // printf or scanf, sprintf
#include <string.h> // strcpy()
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> // home directory
#include <openssl/sha.h> // SHA1()
#include <dirent.h> // struct dirent
#include <sys/stat.h> // stat ex) S_IRWXU
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h> // wait for state change in a child
#include <pwd.h> // home directory
#include <fcntl.h> 

#define BUFFSIZE 1024 // define BUFFSIZE is 1024
#define PORTNO 39999 // define PORTNO is 39999

/////////////////////////////////////////////////////////////////////
// Sha1_hash                                                       //
// ================================================================//
// Input : char *input_url -> URL to hash                          //
//         char * hashed_url -> recieve the result value           //
// Output : char *hashed_url -> result value                       //
// Purpose : convert input_url to hashed_url                       //
/////////////////////////////////////////////////////////////////////

char *sha1_hash(char *input_url, char *hashed_url)
{
	unsigned char hashed_160bits[20]; // value the hashed 160 bits
	char hashed_hex[41]; // hashed data in hex format
	int i; // used for loop

	SHA1(input_url, strlen(input_url), hashed_160bits); // transfe hashed_160bit to hashed_hex

	for (i = 0; i<sizeof(hashed_160bits); i++)
		sprintf(hashed_hex + i * 2, "%02x", hashed_160bits[i]); // return value copy

	strcpy(hashed_url, hashed_hex); // copy hashed_hex to hashed_url

	return hashed_url;
}

/////////////////////////////////////////////////////////////////////
// GetHomeDir                                                      //
// =============================================================== //
// input : char * home -> receive the result value                 //
// Output : char * home -> get home name                           //
// Purpose : get home directory name                               //
/////////////////////////////////////////////////////////////////////

char *getHomeDir(char * home) // get home path function
{
	struct passwd * usr_info = getpwuid(getuid());
	strcpy(home, usr_info->pw_dir); // copy home directory name

	return home; // home directory path
}

/////////////////////////////////////////////////////////////////////
// handler                                                         //
// =============================================================== //
// input : void                                                    //
// Output : void                                                   //
// Purpose : wait signal function                                  //
/////////////////////////////////////////////////////////////////////

static void handler()
{
	pid_t pid; // when fork
	int status; // status
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0); // wait signal
}

/////////////////////////////////////////////////////////////////////////////
//Function name : main                                                     //
//input : variable                                                         //
//output : int                                                             //
//Purpose : Forward HTTP request to Web server & print the HTTP response   //
/////////////////////////////////////////////////////////////////////////////

int main()
{
	char input[BUFFSIZE]; // input url
	char output[41]; // result to hashed url
	char dir_path[512]; // home path

	char home_path[512]; // path to make dir and file

	char hashed_d[3]; // hashed direcotory name
	char hashed_f[38]; // hashed file name
	char dir_name[20]; // name of new directory
	char file_name[20]; // name of new file

	int confirm = 0; // variable to check that is HIT or MISS
	char log_miss[512]; // print if MISS 
	char log_hit[512]; // print if HIT

	char terminate[512]; // print sentence when program is exitted

	int miss = 0; // count number of miss
	int hit = 0; // count number of hit

	struct dirent * pFile; // cache directory entry pointer

	DIR * pDir; // directory pointer
	FILE * fDir; // hashed file pointer

	DIR * Directory; // variable to open directory 
	FILE * File; // variable to read directory

	struct sockaddr_in server_addr, client_addr; // socket address struct variable
	int socket_fd, client_fd; // socket descriptor and client descriptor
	int len, len_out; //client's address length, length of recieve text from client

	if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) // if socket make error
	{
		printf("Server: Can't open stream socket\n"); // if socket make error
		return 0; // exist
	}

	bzero((char *)&server_addr, sizeof(server_addr)); // address struct initialize
	server_addr.sin_family = AF_INET; // server's family is AF_INET
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);//server's address initialize
	server_addr.sin_port = htons(PORTNO); // server's port is network addess type 

	if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{ // if bind error
		printf("Server : Can't bind local address\n"); // print error phase
		return 0;
	}
	listen(socket_fd, 5); // listen, queue size is 5

	while (1)
	{
		struct in_addr inet_client_address; // client address

		char buf[BUFFSIZE]; // make buffer
		char response_header[BUFFSIZE] = { 0, }; // response header initialize
		char response_message[BUFFSIZE] = { 0, }; // response message initialize

		// initialize
		char tmp[BUFFSIZE] = { 0, }; 
		char method[20] = { 0, };
		char url[BUFFSIZE] = { 0, };

		char * tok = NULL;

		len = sizeof(client_addr); // a variable which is length of client address
		client_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &len); // accept client's connection
		if (client_fd < 0) // if client's accept error
		{
			printf("Server : accept failed\n"); // print error phase
			return 0;
		}

		inet_client_address.s_addr = client_addr.sin_addr.s_addr; // client address

		printf("[%s  :  %d] client was connected\n", inet_ntoa(inet_client_address), client_addr.sin_port);
		read(client_fd, buf, BUFFSIZE); // read client's buf

		strcpy(tmp, buf); // copy buf into tmp
		// print sentence
		puts("=======================================================");
		printf("Request from [%s  :  %d]\n", inet_ntoa(inet_client_address), client_addr.sin_port);
		puts(buf);
		puts("=======================================================");

		tok = strtok(tmp, " "); // Cut in spacing
		strcpy(method, tok); // copy tok into method
		if (strcmp(method, "GET") == 0) // if method = "GET"
		{
			tok = strtok(NULL, " "); // cut in spacing
			strcpy(url, tok); // copy tok into url
		}

		confirm = 0; // check if miss or hit
		strcpy(output, sha1_hash(url, output)); // convert text url to hashed url 
		getHomeDir(dir_path); // home path
		getHomeDir(home_path); // get home directory
		strncpy(hashed_d, output, 3); // new directory name
		hashed_d[3] = '\0';
		strncpy(hashed_f, output + 3, 38); // new file name
		hashed_f[38] = '\0';

		strcpy(dir_name, "logfile"); // directory
		strcpy(file_name, "logfile.txt"); // file


		strcat(dir_path, "/cache/"); // cache directory
		strcat(home_path, "/");


		strcat(home_path, dir_name); // directory path
		Directory = opendir(home_path); // open directory 

		umask(0);
		mkdir(home_path, 0777); // make directory logfile
		strcat(home_path, "/");
		strcat(home_path, file_name); // make file logfile.txt
		File = fopen(home_path, "a");
		fclose(File);

		pDir = opendir(dir_path);
		if (pDir == NULL)
		{
			mkdir(dir_path, 0777); // make cache dir
		}
		else closedir(pDir); // if already exist cachd dir

		strcat(dir_path, hashed_d); // directory path
		pDir = opendir(dir_path); // open directory

		if (pDir == NULL) // MISS - no directory 
		{
			miss++;
			umask(0);
			mkdir(dir_path, 0777); // make dirtory
			strcat(dir_path, "/");
			strcat(dir_path, hashed_f); // file path
			fDir = fopen(dir_path, "a"); // creat file
			fclose(fDir);
			File = fopen("/home/choi/logfile/logfile.txt", "a"); // file open
			fprintf(File, "[MISS] ServerPID : %d | %s\n", getpid(), url);
			fclose(File); // close the file
		}

		else
		{ // read directory
			for (pFile = readdir(pDir); pFile; pFile = readdir(pDir))
			{
				if (!strcmp(hashed_f, pFile->d_name)) // if exist file
				{
					hit++; // increase number of hit
					confirm = 1; // hit
					File = fopen("/home/choi/logfile/logfile.txt", "a"); // file path
					fprintf(File, "[HIT] ServerPID : %d | %s/%s\n", getpid(), hashed_d, hashed_f); // fprintf log_hit
					fprintf(File, "[HIT]%s\n", url);
					fclose(File); // close the file
				}
			}
			if (confirm == 0)
			{// MISS
				miss++;
				strcat(dir_path, "/");
				strcat(dir_path, hashed_f); //flie path
				fDir = fopen(dir_path, "a"); // creat file
				fclose(fDir);
				File = fopen("/home/choi/logfile/logfile.txt", "a");
				fprintf(File, "[MISS] ServerPID : %d | %s\n", getpid(), url); // fprintf log_miss
				fclose(File);
			}
			closedir(pDir); // close directory
		}

		if (confirm == 0) sprintf(response_message, "MISS\n"); // response message if miss
		else sprintf(response_message, "HIT\n"); // response message if hit

		// print response header
		sprintf(response_header,
			"HTTP/1.0 200 OK\r\n"
			"Server:2018 simple web server\r\n"
			"Content-length:%lu\r\n"
			"Content-type:text/html\r\n\r\n", strlen(response_message));

		write(client_fd, response_header, strlen(response_header)); // write reponse header
		write(client_fd, response_message, strlen(response_message)); // write reponse message

		printf("[%s  :  %d] client was disconnected\n", inet_ntoa(inet_client_address), client_addr.sin_port); // print the sentence
		close(client_fd); // disconnected about client
	}
	close(socket_fd); // close server's socket
	return 0;
}

