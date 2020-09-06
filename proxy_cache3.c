//////////////////////////////////////////////////////////////////////
// File Name : proxy_cache.c                                        //
// Date      : 2018/4/10                                            //
// Os        : Ubuntu 16.04 LTS 64bits                              //
// Author    : CHoi Yeon Hee                                        //
// Student ID: 2016722035                                           //
// -----------------------------------------------------------------//
// Title : System Programming Assignment #1-3 (proxy server)        //
//////////////////////////////////////////////////////////////////////


#include<stdio.h> // printf or scanf, sprintf
#include<string.h> // strcpy()
#include<openssl/sha.h> // SHA1()
#include<sys/types.h> // Dir* type
#include<dirent.h> // struct dirent
#include<sys/stat.h> // stat ex) S_IRWXU
#include<stdlib.h>
#include<unistd.h> // home directory
#include<pwd.h> // home directory
#include<time.h> // time function
#include<fcntl.h> 
#include<sys/wait.h> // wait for state change in a child

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

/////////////////////////////////////////////////////////////////////
//Function name : main                                             //
//input : void                                                     //
//output : void                                                    //
//Purpose : Multiple processing                                    //
/////////////////////////////////////////////////////////////////////

void main(void)
{
	pid_t pid; // process id
	char cmd[15]; // CMD store variable
	int status; // process status store variable
	char input[41]; // input url
	char output[41]; // result to hashed url

	int sub_process = 0; // sub process count
	
	int run_sub = 0; // sub process run time
	int runtime = 0; // program exit time
	int run = run_time(); // store the time that begin the program
	int final_run = 0; // program run time
	int run_final = 0; // time when program is exitted

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


	while (1)
	{
		memset(cmd, '\0', sizeof(cmd)); // memory initailize
		printf("[%d]input CMD> ", getpid()); 
		scanf("%s", cmd); // input cmd

		if (strcmp(cmd, "quit") == 0) // if cmd == quit
		{
			runtime = run_time(); // program exit time
			break;
		}
	
		else if (strcmp(cmd, "connect") == 0) // if cmo == connect
		{
			run_sub = run_time(); // program start time
			sub_process++; // count of sub process
			if ((pid = fork()) < 0) printf("fork error\n");
			else if (pid == 0) // child
			{
				while (1)
				{
					memset(input, '\0', sizeof(input)); // memory initialize
					printf("[%d]input URL> ", getpid());
					scanf("%s", input); // input
					// process exit
					if (strcmp(input, "bye") == 0) // if input == bye
					{
						run_final = run_time(); // time when the program is exitted
						break;
					}
					else // assignment 1-2
					{  /********************************************************************/
						strcpy(output, sha1_hash(input, output)); // convert text url to hashed url 
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
						strcpy(log_miss, "[Miss]");
						strcat(log_miss, input);
						strcat(log_miss, pstime);

						// print when HIT
						strcpy(log_hit, "[Hit]");
						strcat(log_hit, hashed_d);
						strcat(log_hit, "/");
						strcat(log_hit, hashed_f);
						strcat(log_hit, pstime);
						strcat(log_hit, "[Hit]");
						strcat(log_hit, input);
						strcat(log_hit, "\n");
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
									fprintf(File, "%s", log_hit); // fprintf log_hit
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
								fprintf(File, "%s", log_miss); // fprintf log_miss
								fclose(File);
							}

							closedir(pDir); // close directory
						}

						
					} /********************************************************************/
				}

				// write in logfile.txt
				if (!strcmp("bye", input)) // if input is "bye"
				{
					runtime = run_final - run_sub; // caculate program run time
					File = fopen("/home/choi/logfile/logfile.txt", "a"); // file open
					// fprintf sentence in File
					fprintf(File, "%s", terminate);
					fprintf(File, "%d sec. #request hit : ", runtime);
					fprintf(File, "%d, miss : %d\n", hit, miss);
					fclose(File); // close the file
					exit(0); 
				}

			}
			if ((pid = waitpid(pid, &status, 0)) < 0) printf("waitpid error\n");
		}
		
	}

	// if cmd == quit, write in logfile.txt
	if (strcmp(cmd, "quit") == 0)
	{
		final_run = runtime - run; // caculate program time
		File = fopen("/home/choi/logfile/logfile.txt", "a"); // file open
		// fprintf the setence in File
		fprintf(File, "**SERVER** [Terminated] run time: %d sec. #sub process: %d\n", final_run, sub_process); // the number of sub process count
		fclose(File); // close the file
		exit(0); // exit the program
	}

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
	sprintf(ret, "-[%d/%d/%d, %d:%d:%d]\n", year, mon, day, hour, min, sec); // printf the sentence
	
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

	for(i=0; i<sizeof(hashed_160bits); i++)
		sprintf(hashed_hex + i*2, "%02x", hashed_160bits[i]); // return value copy

	strcpy(hashed_url, hashed_hex); // copy hashed_hex to hashed_url

	return hashed_url;
}
