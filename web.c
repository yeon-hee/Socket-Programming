//////////////////////////////////////////////////////////////////////
// File Name : web.c                                                //
// Date      : 2018/5/18                                            //
// Os        : Ubuntu 16.04 LTS 64bits                              //
// Author    : CHoi Yeon Hee                                        //
// Student ID: 2016722035                                           //
// -----------------------------------------------------------------//
// Title : Construction Proxy Connection #2-3                       //
//////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <dirent.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h> // waitpid 
#include <stdlib.h> // exit()
#include <signal.h>
#include<time.h> // time function

#define BUFFSIZE 1024 // define BUFFSIZE is 1024
#define PORTNO 38039 // define PORTNO is 38039
char * getIPAddr(char *addr);

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
// my_wait                                                         //
// ================================================================//
// Purpose : wait signal                                           //
/////////////////////////////////////////////////////////////////////

void my_wait(int signo)
{
	pid_t pid;
	int status;
	pid = wait(&status); // wait fuction

	printf("* PID : %d Child End!\n", pid); // wait pid complete
}

/////////////////////////////////////////////////////////////////////
// my_alarm                                                        //
// ================================================================//
// Purpose : transfer alarm to process                             //
/////////////////////////////////////////////////////////////////////
void my_alarm(int signo)
{
	printf("No Response message\n"); // print
	exit(0); // child process exit
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

char * getIPAddr(char* addr)
{
	struct hostent* hent; // host entry struct variable
	char *haddr; // host address pointer variable
	int len = strlen(addr); // host address length

	if ((hent = (struct hostent*)gethostbyname(addr)) != NULL) // get host name
	{
		haddr = inet_ntoa(*((struct in_addr*)hent->h_addr_list[0]));
		// 32 bit big-endian ip address convert to dotted decimal string
	}
	return haddr;
}

/////////////////////////////////////////////////////////////////////////////
//Function name : main                                                     //
//input : variable                                                         //
//output : int                                                             //
//Purpose : Forward HTTP request to Web server and signal handling         //
/////////////////////////////////////////////////////////////////////////////


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
	sprintf(ret, "-[%d/%d/%d, %d:%d:%d]\n", year, mon, day, hour, min, sec); // printf the sentence

	return ret; // return value
}

int main()
{
	int web_fd;  // web brower connect socket dscriptor
	struct sockaddr_in web_addr;
	char web_buf[BUFFSIZE]; // read from web server which is response message
	int response_fd; // response file descriptor
	pid_t pid; // process id
	int status; // process status store variable
	int flag = 0; // for alarm
	char *phost; // to fetch host name from http request message
	char *IPAddr; // to get ip address from host by using getIPAddr function
	char buf[BUFFSIZE]; // make buffer
	char input[BUFFSIZE]; // input url
	char output[41]; // result to hashed url
	char dir_path[512]; // home path
	int final_run = 0;
	char path[BUFFSIZE];

	char tmp[BUFFSIZE] = { 0, }; // buffer
	char buf_tmp[BUFFSIZE] = { 0, }; // buffer
	char method[20] = { 0, };
	char url[BUFFSIZE] = { 0, }; // url
	char arr[BUFFSIZE] = { 0, };
	char * tok = NULL;

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
	int opt;

	struct dirent * pFile; // cache directory entry pointer

	DIR * pDir; // directory pointer
	FILE * fDir; // hashed file pointer

	int sub_process = 0; // sub process count
	char *pstime; // time
	int check = 0;
	int run = run_time(); // store the time that begin the program

	int runtime = 0;
	int a = 0;

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

	opt = 1;
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

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
	len = sizeof(client_addr);
	signal(SIGALRM, my_alarm);
	signal(SIGCHLD, my_wait);

	getHomeDir(home_path); // get home directory

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

	while (1)
	{
		
		//bzero((char*)&client_addr, sizeof(client_addr));
		len = sizeof(client_addr);
		client_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &len);

		if (client_fd < 0) // if client's accept error
		{
			printf("Server: accept failed	%d\n", getpid()); // print error phase
			close(socket_fd);
			return 0;
		}

		pid = fork(); // fork()

		if (pid == -1)
		{
			close(client_fd);
			close(socket_fd);
			continue;
		}
	
		if (pid == 0) // child process
		{
			while ((len_out = read(client_fd, buf, BUFFSIZE)) > 0) // request 
			{
				confirm = 0;

				struct in_addr inet_client_address; // client address
				inet_client_address.s_addr = client_addr.sin_addr.s_addr; // client address

				char response_header[BUFFSIZE] = { 0, }; // response header initialize
				char response_message[BUFFSIZE] = { 0, }; // response message initialize

				memset(response_header, 0, sizeof(response_header)); // initialize response_header
				memset(response_message, 0, sizeof(response_message)); // initialize response_message

				strcpy(tmp, buf); // copy buf into tmp
				phost = strtok(tmp, "/"); // get host name
				phost = strtok(NULL, "/");
				IPAddr = getIPAddr(phost);

				strcpy(buf_tmp, buf);
				tok = strtok(buf_tmp, " "); // Cut in spacing
				strcpy(method, tok); // copy tok into method
				if (strcmp(method, "GET") == 0) // if method = "GET"
				{
					tok = strtok(NULL, " "); // cut in spacing
					strcpy(url, tok); // copy tok into url
				}

				char arr[BUFFSIZE] = { 0, };
				int length = 0;
				length = strlen(url);
				arr[0] = url[length - 4]; // url[length - 4]
				arr[1] = url[length - 3]; // url[length - 3]
				arr[2] = url[length - 2]; // url[length - 2]
				arr[3] = url[length - 1]; // url[length - 1]
			
				sprintf(response_message, // response_message format
					"<h1>RESPONSE</h1><br>"
					"Hello %d:%d<br>", client_addr.sin_addr.s_addr, client_addr.sin_port);
				sprintf(response_header, // response_header format
					"HTTP/1.0 200 OK\r\n"
					"Server:2018 simple web server\r\n"
					"Content-lengh:%lu\r\n"
					"Content-type:text/html\r\n\r\n", strlen(response_message));
				

				strcpy(output, sha1_hash(url, output)); // convert text url to hashed url 
				getHomeDir(dir_path); // home path
				getHomeDir(home_path); // get home directory
				strncpy(hashed_d, output, 3); // new directory name
				hashed_d[3] = '\0';
				strncpy(hashed_f, output + 3, 38); // new file name
				hashed_f[38] = '\0';

				strcpy(dir_name, "logfile"); // directory
				strcpy(file_name, "logfile.txt"); // file

				getHomeDir(path); // home path

				strcat(dir_path, "/cache/"); // cache directory
				strcat(path, "/cache/"); // cache directory
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
					File = fopen(dir_path, "a"); // creat file
					fprintf(File, "%s\n", response_header); // response data
					fprintf(File, "%s\n", response_message); // response data
					fclose(fDir);
					
					if ((strcmp(arr, "html") == 0) || (arr[3] == '/'))
					{
						sub_process++;
						File = fopen("/home/choi/logfile/logfile.txt", "a"); // file open
						fprintf(File, "[MISS] %s %s", url, pstime);
						fclose(File);
					}
					
				}

				else
				{ // read directory
					if (confirm == 0)
					{// MISS
						miss++;
						strcat(dir_path, "/");
						strcat(dir_path, hashed_f); //flie path
						fDir = fopen(dir_path, "a"); // creat file
						File = fopen(dir_path, "a"); // creat file
						fprintf(File, "%s\n", response_header); // response data
						fprintf(File, "%s\n", response_message); // response data
						fclose(fDir);
					}

					for (pFile = readdir(pDir); pFile; pFile = readdir(pDir))
					{
						if (!strcmp(hashed_f, pFile->d_name)) // if exist file
						{
							hit++; // increase number of hit
							confirm = 1; // hit
							File = fopen("/home/choi/logfile/logfile.txt", "a"); // file path
							fprintf(File, "[HIT] %s/%s %s", hashed_d, hashed_f, pstime); // fprintf log_hit
							fprintf(File, "[HIT]%s\n", url);
							fclose(File); // close the file
										  //bzero((char*)&buf, sizeof(buf));
							confirm = 0;
						}
					}
					
					closedir(pDir); // close directory
				}


				if (confirm == 0) // MISS
				{

					while ((web_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
					{
						printf("Server: Can't open stream socket to connect web");
						return 0;
					}
					// initialize web server's address & port number to connect
					bzero((char*)&web_addr, sizeof(web_addr)); // memset
					web_addr.sin_family = AF_INET; // server address's family is AF_INET
					web_addr.sin_addr.s_addr = inet_addr(IPAddr); // server's ip address
					web_addr.sin_port = htons(80); // server's port number 80
												   // connect with web server
					if (connect(web_fd, (struct sockaddr*)&web_addr, sizeof(web_addr)) < 0)
					{
						printf("Can't connect with web server.\n");
						return -1;
					}
					// write to web server from proxy server
					write(web_fd, buf, strlen(buf)); // write request http message to web from proxy
					alarm(10); // alarm 10 set;
					flag = 0; // setting alarm flag

							  // read from web server to proxy
					while (1) // read from web
					{
						if ((len_out = read(web_fd, web_buf, BUFFSIZE)) <= 0) // if no read  - HTTP response
						{
							break;
						}
						// write response message to file
						flag = 1;
						write(client_fd, web_buf, len_out); // transfer web browser
						bzero((char*)&web_buf, sizeof(web_buf));

					}

					if (flag == 0) // if no read any response message
						pause(); // for signal alarm
					else alarm(0); // alarm reset
								   // close file descriptor
					close(web_fd);
					//bzero((char*)&buf, sizeof(buf))
				}
			}
			
			close(client_fd);
			exit(0);
		}
		close(client_fd); // disconnected about client
		
	}
	runtime = run_time();
	final_run = runtime - run; // caculate program time
	File = fopen("/home/choi/logfile/logfile.txt", "a"); // file open
	fprintf(File, "**SERVER** [Terminated] run time: %d sec. #sub process: %d\n", final_run, sub_process); // the number of sub process count
	fclose(File);
	close(socket_fd); // close server's socket
	if (client_fd < 0)
	{
		printf("Server : accept failed.\n");
		return 0;
	}
	
	return 0;
}


