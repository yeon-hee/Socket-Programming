//////////////////////////////////////////////////////////////////////
// File Name : server.c                                             //
// Date      : 2018/4/27                                            //
// Os        : Ubuntu 16.04 LTS 64bits                              //
// Author    : CHoi Yeon Hee                                        //
// Student ID: 2016722035                                           //
// -----------------------------------------------------------------//
// Title : System Programming Assignment #2-1                       //
//////////////////////////////////////////////////////////////////////

#include <stdio.h> // printf or scanf, sprintf
#include <string.h> // strcpy()
#include <openssl/sha.h> // SHA1()
#include <dirent.h> // struct dirent
#include <sys/stat.h> // stat ex) S_IRWXU
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h> // home directory
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h> // wait for state change in a child
#include <pwd.h> // home directory
#include <fcntl.h> 
#include <time.h> // time function
#include <arpa/inet.h>

#define BUFFSIZE 1024 // define BUFFSIZE is 1024
#define PORTNO 38039 // define PORTNO is 38039

/////////////////////////////////////////////////////////////////////
// Sha1_hash                                                       //
// ================================================================//
// Input : char *input_url -> URL to hash                          //
//         char * hashed_url -> recieve the result value           //
// Output : char *hashed_url -> result value                       //
// Purpose : convert input_url to hashed_url                       //
/////////////////////////////////////////////////////////////////////

char *sha1_hash(char *input_url, char *hashed_url); // sha1_hash function
char* transfer_time(); // time function
int run_time(); // time function

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

static void handler()
{
	pid_t pid; // when fork
	int status; // status
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0); // wait signal
}

/////////////////////////////////////////////////////////////////////
//Function name : main                                             //
//input : variable                                                 //
//output : int                                                     //
//Purpose : Multiple Echo Server                                   //
/////////////////////////////////////////////////////////////////////

int main()
{
	int run = run_time(); // store the time that begin the program
	int run_final = 0; // time when program is exitted
	int runtime = 0; // program run time
	char input[BUFFSIZE]; // input url
	char output[41]; // result to hashed url
	char dir_path[512]; // home path

	char home_path[512]; // path to make dir and file

	char hashed_d[3]; // hashed direcotory name
	char hashed_f[38]; // hashed file name
	char dir_name[20]; // name of new directory
	char file_name[20]; // name of new file

	int confirm = 0; // variable to check that is HIT or MISS
	char *pstime; // time
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
				 /****************************************************************************/

	struct sockaddr_in server_addr, client_addr; // socket address struct variable
	int socket_fd, client_fd; // socket descriptor and client descriptor
	int len, len_out; //client's address length, length of recieve text from client
	int state;
	char *str;
	char buf[BUFFSIZE]; // make buffer which size is 1024 
	pid_t pid;

	if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) // if socket make error
	{
		printf("Server: Can't open stream socket\n"); // if socket make error
		return 0; // exist
	}

	bzero((char *)&server_addr, sizeof(server_addr)); // address struct initialize
	server_addr.sin_family = AF_INET; // server's family is AF_INET
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");//server's address initialize
	server_addr.sin_port = htons(PORTNO); // server's port is network addess type 
	
										  // bind
	if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{ // if bind error
		printf("Server : Can't bind local address\n"); // print error phase
		close(socket_fd);
		return 0;
	}

	listen(socket_fd, 5); // listen, queue size is 5
	signal(SIGCHLD, (void *)handler);

	while (1)
	{
		bzero((char*)&client_addr, sizeof(client_addr));
		len = sizeof(client_addr); // a variable which is length of client address
		client_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &len); // accept client's connection

		if (client_fd < 0) // if client's accept error
		{
			printf("Server: accept failed	%d\n", getpid()); // print error phase
			close(socket_fd);
			return 0;
		}

		printf("[%s : %d] client was connected. \n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
		// client_addr.sin_addr.s_addr = client's ip address , client_addr.sin_port = client's port number
		pid = fork();

		if (pid == -1)
		{
			close(client_fd);
			close(socket_fd);
			continue;
		}

		if (pid == 0)
		{
			while ((len_out = read(client_fd, buf, BUFFSIZE)) > 0) // if exist readable buf
			{
				confirm = 0; // variable that check miss or hit
				strcpy(input, strtok(buf, "\n")); // read buf before enter

				if (!strncmp(buf, "bye", 3)) // if buf is "bye"
				{
					break;
				}

				else
				{

					strcpy(output, sha1_hash(buf, output)); // convert text url to hashed url 
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


					pstime = transfer_time(); // time
											  // print when MISS
					strcpy(log_miss, pstime);

					// print when HIT
					strcpy(log_hit, pstime);

					free(pstime); // time

					strcpy(terminate, "[Terminated] run time: ");

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
						fprintf(File, "[MISS] ServerPID : %d | %s", getpid(), buf); // fprintf log_miss
						fprintf(File, "%s", log_miss); // fprintf log_miss
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
								fprintf(File, "[HIT] ServerPID : %d | %s/%s", getpid(), hashed_d, hashed_f); // fprintf log_hit
								fprintf(File, "%s", log_hit);
								fprintf(File, "[HIT]%s\n", buf);
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
							fprintf(File, "[MISS] ServerPID : %d | %s", getpid(), buf); // fprintf log_miss
							fprintf(File, "%s", log_miss); // fprintf log_miss
							fclose(File);
						}

						closedir(pDir); // close directory
					}

					if (confirm == 0) write(client_fd, "MISS\n", 5); // write miss signal 
					else write(client_fd, "HIT\n", 4); // write hit signal

				} // else
			} // while

			run_final = run_time(); // time when the program is exitted
			runtime = run_final - run;
			strcpy(terminate, "[Terminated] ServerPID :");
			File = fopen("/home/choi/logfile/logfile.txt", "a"); // file open
			fprintf(File, "%s %d | run time : %d sec. #request hit : %d, miss : %d\n", terminate, getpid(), runtime, hit, miss);
			fclose(File); // file close

			printf("[%s : %d] client was disconnected.\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port); // print the sentence
			close(client_fd);
			exit(0); // exit the process
		}
		close(client_fd); // disconnected about client
	}
	close(socket_fd); // close server's socket

	return 0;
}


/////////////////////////////////////////////////////////////////////
// transfer_time                                                   //
// ================================================================//
// Output : char * ret -> local time                               //
// Purpose : get time                                              //
/////////////////////////////////////////////////////////////////////


char* transfer_time() // time
{
	char *ret = (char *)malloc(40); // memory
	time_t now;
	struct tm *ltp; // struct time
	int year = 0;
	int mon = 0;
	int day = 0;
	int hour = 0;
	int min = 0;
	int sec = 0;

	time(&now);
	ltp = localtime(&now); // local time
	year = ltp->tm_year + 1900; // year
	mon = ltp->tm_mon + 1; // mon
	day = ltp->tm_mday; // day
	hour = ltp->tm_hour;//hour
	min = ltp->tm_min;//min
	sec = ltp->tm_sec;//second
	sprintf(ret, " - [%d/%d/%d, %d:%d:%d]\n", year, mon, day, hour, min, sec); // printf the sentence

	return ret; // return value
}

/////////////////////////////////////////////////////////////////////
// run_time                                                        //
// ================================================================//
// Output : int program -> calculate now time                      //
// Purpose : get to program time                                   //
/////////////////////////////////////////////////////////////////////

int run_time() // calculate the run time
{
	time_t now;
	int program = 0;
	struct tm *ltp; // struct time
	int year = 0;
	int mon = 0;
	int day = 0;
	int hour = 0;
	int min = 0;
	int sec = 0;

	time(&now);
	ltp = localtime(&now); // local time

	year = ltp->tm_year + 1900; // year
	mon = ltp->tm_mon + 1; // mon
	day = ltp->tm_mday; // day
	hour = ltp->tm_hour;//hour
	min = ltp->tm_min;//min
	sec = ltp->tm_sec;//second

	program = mktime(ltp); // count the program executing time

	return program;
}


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

