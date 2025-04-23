// Copyright (c) 2012 DotNetAnywhere
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifdef _WIN32
#include <time.h>
#else
#include <sys/time.h>
#endif // _WIN32

#if defined(__PSP__)

// this makes the PSP stuff redirect to the debug onscreen printing
#include <pspdebug.h>

#endif

#include "Compat.h"
#include "Sys.h"

#include "MetaData.h"
#include "Types.h"


#if defined(__PSP__)

#define printf pspDebugScreenPrintf

#endif

void printline(char* buff, int size, int linesize)
{
#if defined(__PSP__)
	int hedge = size % linesize;
	int newsize = size + hedge;
	int pointer = 0;
	char buffer[newsize];

	for (int i = 0; i < newsize; i++)
	{
		if (i % linesize == 0)
		{
			buffer[i] = '\n';
		}
		else
		{
			buffer[i] = buff[pointer++];
		}
	}
#endif

	printf("%s\n", buff);
}

void Crash(char* pMsg, ...)
{
	va_list va;

	printf("\n\n*** CRASH ***\n");

	char buff[255];

	va_start(va, pMsg);

#if defined(__PSP__) || (__APPLE__)
	vsnprintf((char const*)&buff, 255, (const char*)pMsg, va);

	int i = strlen(buff);
	printline(buff, i, 60);
#else
	vprintf(pMsg, va);
#endif

	va_end(va);

	printf("\n\n");

	exit(1);
}

U32 logLevel = 0;

void log_f(U32 level, char* pMsg, ...)
{
	va_list va;

	if (logLevel >= level)
	{
		va_start(va, pMsg);
		vprintf(pMsg, va);
		va_end(va);
	}
#if _DEBUG
	//fflush(stdout);
#endif
}

static char methodName[2048];
char* Sys_GetMethodDesc(tMD_MethodDef* pMethod)
{
	U32 i;

	sprintf(methodName, "%s.%s.%s(", pMethod->pParentType->nameSpace, pMethod->pParentType->name, pMethod->name);
	for (i = METHOD_ISSTATIC(pMethod) ? 0 : 1; i < pMethod->numberOfParameters; i++)
	{
		if (i > (U32)(METHOD_ISSTATIC(pMethod) ? 0 : 1))
		{
			sprintf(strchr(methodName, 0), ",");
		}
		sprintf(strchr(methodName, 0), pMethod->pParams[i].pTypeDef->name);
	}
	sprintf(strchr(methodName, 0), ")");
	return methodName;
}

static U32 mallocForeverSize = 0;
// malloc() some memory that will never need to be resized or freed.
void* mallocForever(U32 size)
{
	if (size > 31457280)
	{
		printf("--- mallocForever: !!!! Trying to allocate 30Mb+: %dMb\n", size / 1024 / 1024);
	}
	mallocForeverSize += size;
	printf("--- mallocForever: TotalSize %d\n", mallocForeverSize);
	log_f(3, "--- mallocForever: TotalSize %d\n", mallocForeverSize);
	return malloc(size);
}

/*
#ifdef _DEBUG
void* mallocTrace(int s, char *pFile, int line) {
	//printf("MALLOC: %s:%d %d\n", pFile, line, s);
#undef malloc
	return malloc(s);
}
#endif
*/

U64 msTime()
{
#ifdef _WIN32
	static LARGE_INTEGER freq = { 0, 0 };
	LARGE_INTEGER time;
	if (freq.QuadPart == 0)
	{
		QueryPerformanceFrequency(&freq);
	}
	QueryPerformanceCounter(&time);
	return (time.QuadPart * 1000) / freq.QuadPart;
#else
	struct timeval tp;
	U64 ms;
	gettimeofday(&tp, NULL);
	ms = tp.tv_sec;
	ms *= 1000;
	ms += ((U64)tp.tv_usec) / ((U64)1000);
	return ms;
#endif
}

#if defined(DIAG_METHOD_CALLS) || defined(DIAG_OPCODE_TIMES) || defined(DIAG_GC) || defined(DIAG_TOTAL_TIME)
U64 microTime()
{
#ifdef _WIN32
	static LARGE_INTEGER freq = { 0, 0 };
	LARGE_INTEGER time;
	if (freq.QuadPart == 0)
	{
		QueryPerformanceFrequency(&freq);
	}
	QueryPerformanceCounter(&time);
	return (time.QuadPart * 1000000) / freq.QuadPart;
#else
	struct timeval tp;
	U64 ms;
	gettimeofday(&tp, NULL);
	ms = tp.tv_sec;
	ms *= 1000000;
	ms += ((U64)tp.tv_usec);
	return ms;
#endif
}
#endif

void SleepMS(U32 ms)
{
#ifdef _WIN32
	Sleep(ms);
#else
	sleep(ms / 1000);
	//usleep((ms % 1000) * 1000);
#endif
}

FILE* pLogger = NULL;

void initLogfile()
{
	if (pLogger == NULL)
	{
		pLogger = fopen("log.txt", "w");
		if (pLogger == NULL)
		{
			Crash("logfile could not be opened");
		}

		fseek(pLogger, 0, SEEK_END); // got to the end of the file

		log_s("--------------------\nstarted\n\n");
	}
}

void closeLogfile()
{
	if (pLogger != NULL)
	{
		log_s("\nended\n--------------------\n");
		fflush(pLogger);
		fclose(pLogger);
	}
}

void log_s(char* pMsg, ...)
{
	if (pLogger == NULL)
	{
		Crash("logfile is not open");
	}

	va_list va;
	va_start(va, pMsg);
	fprintf(pLogger, pMsg, va);
	va_end(va);

	fflush(pLogger);
}