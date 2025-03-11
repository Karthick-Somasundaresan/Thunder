#ifndef __MYLOG__
#define __MYLOG__
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
//extern char* FILE_NAME;
//extern void myLog(const char *format, ... );
static void myLog(const char *format, ... )
{
	FILE* pFile;
	char FILE_NAME[128] = {'\0'};
	char finalBuf[2048]= {0};
	char buffer[1024] = {0};
	char xbuffer[30]= {0};
	struct timeval tv;
	time_t curtime;
	gettimeofday(&tv, NULL);
	curtime=tv.tv_sec;
	strftime(xbuffer,sizeof(xbuffer) - 1,"%m-%d-%Y %T.",localtime(&curtime));
	va_list args;
	va_start (args, format);
	vsnprintf (buffer, sizeof(buffer) - 1, format, args);
	va_end (args);
	snprintf(finalBuf,sizeof(finalBuf),"%s%06ld [TID: %lu] %s\n",xbuffer,tv.tv_usec, pthread_self(), buffer);
	int strl = strlen(finalBuf);
	snprintf(FILE_NAME,sizeof(FILE_NAME), "/root/mylog.%d.log", getpid());
	pFile = fopen(FILE_NAME, "a");
	if(pFile)
	{
		fwrite (finalBuf, sizeof(char), strl, pFile);
		fclose(pFile);
	}
}
#define DEBUG_ON
#ifdef DEBUG_ON
#define ENTER myLog("[%s:%d](%s) Enter", __FILE__, __LINE__, __FUNCTION__);
#define EXIT myLog("[%s:%d](%s) Exit", __FILE__, __LINE__, __FUNCTION__);
#define FAIL_EXIT myLog("[%s:%d](%s) Failure Exit", __FILE__, __LINE__, __FUNCTION__);
#define MYTRACE myLog("[%s:%d] (%s)", __FILE__, __LINE__, __FUNCTION__);
#define MYLOG(X,...) myLog("[%s:%d](%s) " X, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);
#else
#define ENTER 
#define EXIT 
#define MYTRACE 
#define MYLOG(X,...) 
#endif
#endif
