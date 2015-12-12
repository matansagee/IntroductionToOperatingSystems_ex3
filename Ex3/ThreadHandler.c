#define _CRT_SECURE_NO_WARNINGS /* to suppress Visual Studio 2010 compiler warning */

#include "ThreadHandler.h"
#include "Progression.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <windows.h>
#include <tchar.h>

HANDLE CreateThreadSimple(LPTHREAD_START_ROUTINE StartAddress,
	LPVOID ParameterPtr,
	LPDWORD ThreadIdPtr)
{
	return CreateThread(
		NULL,            /*  default security attributes */
		0,                /*  use default stack size */
		StartAddress,    /*  thread function */
		ParameterPtr,    /*  argument to thread function */
		0,                /*  use default creation flags */
		ThreadIdPtr);    /*  returns the thread identifier */
}

int ThreadTask()
{
	printf("hello world\n");
}

int CreateThreads(InputParams *inputParams)
{
 	HANDLE *threadHandles = (HANDLE*) malloc (inputParams->NumOfWorkers * sizeof(threadHandles));   /* An array of thread handles */
	DWORD *threadIDs = (DWORD*) malloc (inputParams->NumOfWorkers * sizeof(threadIDs));      /* An array of threadIDs */
	DWORD exitcode;
	int i;
	if (threadHandles==NULL || threadIDs ==NULL )
		exit(1);


	
	for (i=0;i<inputParams->NumOfWorkers;i++)
	{
		threadHandles[0] = CreateThreadSimple(
		(LPTHREAD_START_ROUTINE) ThreadTask,  /*  thread function */
		NULL,                            /*  argument to thread function */
		&threadIDs[0]); 
	}

	/* Wait for all threads to end: */
	WaitForMultipleObjects(
		inputParams->NumOfWorkers,
		threadHandles,
		TRUE,       /* wait until all threads finish */
		INFINITE);

	Sleep(10);

	for (i = 0; i < inputParams->NumOfWorkers; i++)
	{
		GetExitCodeThread(threadHandles[i], &exitcode);
		printf("Thread number %d returned exit code %d\n", i, exitcode);
		CloseHandle(threadHandles[i]);
	}
}