//////////////////////////////////////////////////////////////////////
// File Name : proxy_cache.c                                        //
// Date      : 2018/3/29                                            //
// Os        : Ubuntu 16.04 LTS 64bits                              //
// Author    : CHoi Yeon Hee                                        //
// Student ID: 2016722035                                           //
// -----------------------------------------------------------------//
// Title : System Programming Assignment #1-1 (proxy server)        //
//////////////////////////////////////////////////////////////////////


#include<stdio.h>
#include<string.h> // strcpy()
#include<openssl/sha.h> // SHA1()
#include<sys/types.h> // Dir* type
#include<dirent.h> // struct dirent
#include<sys/stat.h> // stat ex) S_IRWXU
#include<stdlib.h>
#include<unistd.h>
#include<pwd.h>

/////////////////////////////////////////////////////////////////////
// Sha_hash                                                        //
// ================================================================//
// Input : char *input_url -> input url                            //
//         (Input parameter Description)                           //
// Output : char *hashed_url -> hex transformed hashed_url         //
//         (Output parameter Description)                          //
// Purpose : convert input_url to hashed_url                       //
/////////////////////////////////////////////////////////////////////

char *sha1_hash(char *input_url, char *hashed_url); 
int cache(char *output);

char *getHomeDir(char * home) // get home path function
{
	struct passwd * usr_info = getpwuid(getuid());
	strcpy(home, usr_info->pw_dir);

	return home; // home directory path
}

/////////////////////////////////////////////////////////////////////
// GetHomeDir                                                      //
// =============================================================== //
// Output : home -> home directory                                 //
// Purpose : get home directory path                               //
/////////////////////////////////////////////////////////////////////


int main()
{
	char input[20]; // input index
	char output[41];

	while(1)
	{
		printf("input URL> ");
		scanf("%s", input);
		if(strcmp(input, "bye") == 0)
			return 0; // process exit
		strcpy(output, sha1_hash(input,output)); // convert text url to hashed url 

		cache(output);
	}
}

int cache(char * output)
{
	int i=0; // for hashed file name copy
	char hashed_d[4]; // hashed direcotory name
	char hashed_f[40]; // hashed file name

	char path_home[40]; // home path

	char home[41]; 


	char path_d[40]; // hashed directory path
	char path_f[80]; // hashed file path

	struct dirent * pFile; // cache directory entry pointer
	struct dirent * pFile2; // directory entry pointer

	DIR * pDir; // directory pointer
	DIR * fDir; // hashed file pointer
		
	strcpy(path_d, getHomeDir(home)); //store path
	strcpy(path_f, getHomeDir(home)); // store path

	strcat(path_d, "/cache/"); // store home/cache path
	strcat(path_f, "/cache/");

	hashed_d[0] = output[0]; // directory name assignment
	hashed_d[1] = output[1];
	hashed_d[2] = output[2];
	hashed_d[3] = 0;
	
	for(i=0; ;i++) // hashed file name assignment
	{
		hashed_f[i] = output[i+3];
		if(output[i+3]==NULL) break;
	}

	strcat(path_d, hashed_d); // hashed directory path and name
	strcat(path_f, hashed_d); // hashed file path and name
	strcat(path_f, "/");
	strcat(path_f, hashed_f);

	strcpy(path_home, getHomeDir(home)); // home path assignment
	strcat(path_home, "/cache");

	pDir = opendir(path_home); // cache directory open
	if(pDir == NULL)
	{
		umask(000); // make ~/cache directory 
		mkdir(path_home, S_IRWXU | S_IRWXG | S_IRWXO);	
		pDir = opendir(path_home);
	}

	for(pFile = readdir(pDir); pFile; pFile = readdir(pDir))
	{ // directory tour
		if(strcmp(hashed_d, pFile->d_name)==0)
		{ // exist cache directory
			fDir == opendir(path_d); // directory open
			if(fDir == NULL)
			{
				return;
			}
			for(pFile2=readdir(fDir);pFile2;pFile2=readdir(fDir))
			{
				if(strcmp(hashed_f, pFile2->d_name) == 0) // if exist cache file
				{
					return 0; // HIT
				}
			}
			 
			if(creat(path_f, 0777)<0) // make cache file
			{
				printf("file make error\n");
			}

			closedir(fDir);
			closedir(pDir);
			return 1; // MISS
		}
	}

	umask(000); // if no exist directory
	mkdir(path_d, S_IRWXU | S_IRWXG | S_IRWXO); // make directory and hashed file
	creat(path_f, 0777); // make a hashed named file
	closedir(pDir);
	return 1;
}


char *sha1_hash(char *input_url, char *hashed_url)
{
	unsigned char hashed_160bits[20];
	char hashed_hex[41];
	int i;

	SHA1(input_url, strlen(input_url), hashed_160bits); // transfe hashed_160bit to hashed_hex

	for(i=0; i<sizeof(hashed_160bits); i++)
		sprintf(hashed_hex + i*2, "%02x", hashed_160bits[i]); // return value copy

	strcpy(hashed_url, hashed_hex);

	return hashed_url;
}
