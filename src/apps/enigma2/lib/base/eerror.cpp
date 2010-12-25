#include <lib/base/eerror.h>
#include <lib/base/elock.h>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include <string>

#ifdef MEMLEAK_CHECK
AllocList *allocList;
pthread_mutex_t memLock =
	PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

void DumpUnfreed()
{
	AllocList::iterator i;
	unsigned int totalSize = 0;

	if(!allocList)
		return;

	FILE *f = fopen("/var/tmp/enigma2_mem.out", "w");
	size_t len = 1024;
	char *buffer = (char*)malloc(1024);
	for(i = allocList->begin(); i != allocList->end(); i++)
	{
		unsigned int tmp;
		fprintf(f, "%s\tLINE %d\tADDRESS %p\t%d unfreed\ttype %d (btcount %d)\n",
			i->second.file, i->second.line, (void*)i->second.address, i->second.size, i->second.type, i->second.btcount);
		totalSize += i->second.size;

		char **bt_string = backtrace_symbols( i->second.backtrace, i->second.btcount );
		for ( tmp=0; tmp < i->second.btcount; tmp++ )
		{
			if ( bt_string[tmp] )
			{
				char *beg = strchr(bt_string[tmp], '(');
				if ( beg )
				{
					std::string tmp1(beg+1);
					int pos1=tmp1.find('+'), pos2=tmp1.find(')');
					if ( pos1 != std::string::npos && pos2 != std::string::npos )
					{
						std::string tmp2(tmp1.substr(pos1,(pos2-pos1)));
						tmp1.erase(pos1);
						if (tmp1.length())
						{
							int state;
							abi::__cxa_demangle(tmp1.c_str(), buffer, &len, &state);
							if (!state)
								fprintf(f, "%d %s%s\n", tmp, buffer,tmp2.c_str());
							else
								fprintf(f, "%d %s\n", tmp, bt_string[tmp]);
						}
					}
				}
				else
					fprintf(f, "%d %s\n", tmp, bt_string[tmp]);
			}
		}
		free(bt_string);
		if (i->second.btcount)
			fprintf(f, "\n");
	}
	free(buffer);

	fprintf(f, "-----------------------------------------------------------\n");
	fprintf(f, "Total Unfreed: %d bytes\n", totalSize);
	fflush(f);
};
#endif

Signal2<void, int, const std::string&> logOutput;
int logOutputConsole=1;

static pthread_mutex_t DebugLock =
	PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP;

extern void bsodFatal(const char *component);

void eFatal(const char* fmt, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, 1024, fmt, ap);
	va_end(ap);
	{
		singleLock s(DebugLock);
		logOutput(lvlFatal, "FATAL: " + std::string(buf) + "\n");
		fprintf(stderr, "FATAL: %s\n",buf );
	}
	bsodFatal("enigma2");
}

#ifdef DEBUG

#define DEBUG_WITH_PID

#ifdef DEBUG_WITH_PID
#include <sys/types.h>
#include <sys/syscall.h>
#endif /// DEBUG_WITH_PID

void eDebug(const char* fmt, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, 1024, fmt, ap);
	va_end(ap);
	singleLock s(DebugLock);
	logOutput(lvlDebug, std::string(buf) + "\n");
	if (logOutputConsole)
#ifdef DEBUG_WITH_PID
	{

		pid_t pid = getpid();
		pid_t tid = (pid_t) syscall (SYS_gettid);
		fprintf(stderr, "[pr=%d,th=%d] %s\n",pid,tid,buf);
	}
#else ///DEBUG_WITH_PID
		fprintf(stderr, "%s\n", buf);
#endif ///DEBUG_WITH_PID
}

void eDebugNoNewLine(const char* fmt, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, 1024, fmt, ap);
	va_end(ap);
	singleLock s(DebugLock);
	logOutput(lvlDebug, buf);
	if (logOutputConsole)
		fprintf(stderr, "%s", buf);
}

void eWarning(const char* fmt, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, 1024, fmt, ap);
	va_end(ap);
	singleLock s(DebugLock);
	logOutput(lvlWarning, std::string(buf) + "\n");
	if (logOutputConsole)
// #ifdef QBOXHD
// 	{
// 		pid_t pid = getpid();
// 		pid_t tid = (pid_t) syscall (SYS_gettid);
// 		fprintf(stderr, "[pr=%d,th=%d] %s\n",pid,tid,buf);
// 	}
// #else
		fprintf(stderr, "%s\n", buf);
// #endif
}


#ifdef MAX_LOG

static struct timeval timeVal0;

void setTime0(void)
{
  gettimeofday(&timeVal0, NULL);
}

char *get_timestamp(char *buffP)
{
  struct timeval timeVal;
  struct tm tmSec;
  struct tm *timeP = &tmSec;
  time_t time_sec;
  int delta=1;

  if (buffP == NULL)
  {
    printf("get_time_str: buffP=NULL");
    return NULL;
  }

  time_sec = time(NULL);

  gettimeofday(&timeVal, NULL);
  timeP = (struct tm *) localtime_r(&time_sec,&tmSec);

  if(delta)
    sprintf(buffP,"%02d:%02d:%02d - sec=%ld usec=%ld",
            timeP->tm_hour,timeP->tm_min,timeP->tm_sec,
            timeVal.tv_sec-timeVal0.tv_sec,
            (timeVal.tv_usec<timeVal0.tv_usec)?1000000+timeVal.tv_usec-timeVal0.tv_usec:timeVal.tv_usec-timeVal0.tv_usec);
  else
    sprintf(buffP,"%02d:%02d:%02d - sec=%ld usec=%ld",
            timeP->tm_hour,timeP->tm_min,timeP->tm_sec,
            timeVal.tv_sec, timeVal.tv_usec);

  return buffP;
}


// void printTs(const char *msg){};
void printTs_(const char *msg,const char *file,int line,const char *function)
{
  char ubuf[2*TIME_STR_LEN+1]="";
  get_timestamp(ubuf);
  eDebug("%s:%d::%s - time=%s -  %s\n",file,line,function,ubuf,msg);
}

#endif ///MAX_LOG

#endif // DEBUG

void ePythonOutput(const char *string)
{
#ifdef DEBUG
	singleLock s(DebugLock);
	logOutput(lvlWarning, string);
	if (logOutputConsole)
		fwrite(string, 1, strlen(string), stderr);
#endif
}

void eWriteCrashdump()
{
		/* implement me */
}
