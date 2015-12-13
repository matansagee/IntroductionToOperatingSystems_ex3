#define _CRT_SECURE_NO_WARNINGS /* to suppress Visual Studio 2010 compiler warning */

#include "Progression.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <windows.h>
#include <tchar.h>

LPCTSTR MutexNameTop        = _T( "ProgressionMutexTop" );
LPCTSTR MutexNameArithmetic = _T( "ProgressionMutexArithmetic" );
LPCTSTR MutexNameGeometric  = _T( "ProgressionMutexGeometric" );
LPCTSTR MutexNameDiffrence  = _T( "ProgressionMutexDiffrence" );

TopStatus* CreateTopStatus()
{
	TopStatus *topStatus = (TopStatus*) malloc(sizeof(*topStatus));
	if (topStatus == NULL)
		return NULL;
	topStatus->ArithmeticJobStatus = 0;
	topStatus->GeometriceticJobStatus = 0;
	topStatus->DiffrenceJobStatus = 0;
	return topStatus;
}

int CalcArithmeticProgressionForItemI(int a1,int n, int d)
{
	return a1+(n-1)*d;
}

double CalcGeometricProgressionForItemI(int a1,int n,int q)
{
	return a1*pow((double) q,(double) n-1);
}

float CalaDifferenceProgressionForItemI(int a1,int n, int d, int q)
{
	return (float) CalcGeometricProgressionForItemI(a1,n,q) - (float) CalcArithmeticProgressionForItemI(a1,n,d);
}

void StartBuildingThread(SubSeqArray *subSeqArray, InputParams *inputParams, int startIndex, int threadNumber, enum CALCULATION type)
{
	int i;
	for (i = 0; i < inputParams->JobSize; i++)
	{
		if (type == ARITHMETIC)
		{
			subSeqArray[threadNumber*inputParams->JobSize + i].Value = (float) CalcArithmeticProgressionForItemI(inputParams->A1, i + startIndex, inputParams->d);
			subSeqArray[threadNumber*inputParams->JobSize + i].ThreadNumber = 11;//TODO
			subSeqArray[threadNumber*inputParams->JobSize + i].Time = 11;//TODO
		}
		if (type == GEOMETRIC)
		{
			subSeqArray[threadNumber*inputParams->JobSize + i].Value = (float) CalcGeometricProgressionForItemI(inputParams->A1, i + startIndex, inputParams->q);
			subSeqArray[threadNumber*inputParams->JobSize + i].ThreadNumber = 11;//TODO
			subSeqArray[threadNumber*inputParams->JobSize + i].Time = 11;//TODO
		}
		if (type == DIFFERENCEPROG)
		{
			subSeqArray[threadNumber*inputParams->JobSize + i].Value = (float) CalaDifferenceProgressionForItemI(inputParams->A1, i + startIndex, inputParams->d, inputParams->q);
			subSeqArray[threadNumber*inputParams->JobSize + i].ThreadNumber = 11;//TODO
			subSeqArray[threadNumber*inputParams->JobSize + i].Time = 11;//TODO
		}
	}
}

void StartBuildingThreads(int numOfWorkers, int iterationNumber, InputParams *inputParams, SubSeqArray *subSeqArray,enum CALCULATION type)
{
	int subSeq = inputParams->SubSeqLength;
	int jobSize = inputParams->JobSize;
	int threadNumber;
	int startIndex = iterationNumber*subSeq;
	int stopIndex = (iterationNumber + 1)*subSeq;
	//spliiting to thread
	for (threadNumber = 0; threadNumber < subSeq/jobSize; threadNumber++)
	{
		StartBuildingThread(subSeqArray, inputParams, startIndex + (threadNumber*jobSize),threadNumber,type);
	}
}
void StartCleaningThread(int i, SubSeqArray *subSeqArray,int subSeqLength,FILE *file)
{
	int j = 0;
	int index;
	for (j = 0; j < subSeqLength; j++)
	{
		index = i*subSeqLength + j;
		printf("Index #%d = %d, calculated by thread %d @ %d\n", index, subSeqArray[j].Value, subSeqArray[j].ThreadNumber, subSeqArray[j].Time);
		//fprintf(file,"Index #%d = %d, calculated by thread %d @ %d\n", index, subSeqArray[j].Value, subSeqArray[j].ThreadNumber, subSeqArray[j].Time);
	}

}

void CalculateSeriesByType(int numOfWorkers,InputParams *inputParams, FILE* file,enum CALCULATION type)
{
	int iterationNumber = inputParams->N / inputParams->SubSeqLength;
	int i;
	SubSeqArray *subSeqArray = (SubSeqArray*)malloc(inputParams->SubSeqLength*sizeof(subSeqArray));
	if (subSeqArray == NULL)
		exit(1);

	printf("CalculateSeriesByType %d\n",type);
	for (i = 0; i < iterationNumber; i++)
	{	
		StartBuildingThreads(numOfWorkers,i, inputParams, subSeqArray,type);
		StartCleaningThread(i,subSeqArray, inputParams->SubSeqLength,file);
	}
}


void DoCalculations(InputParams *inputParams,FILE **files)
{
	CalculateSeriesByType(inputParams->NumOfWorkers / 2,inputParams, files[0],ARITHMETIC);
	CalculateSeriesByType(inputParams->NumOfWorkers / 2, inputParams, files[1], GEOMETRIC);
	//CalculateSeriesByType(inputParams->NumOfWorkers / 2, inputParams, files[2], DIFFERENCEPROG);
}

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



HANDLE CreateMutexSimple( LPCTSTR MutexName )
{
    return CreateMutex( 
        NULL,              // default security attributes
        FALSE,             // initially not owned
        MutexName);             
}
    
DWORD GetNextJob(BOOL *stop, TopStatus* TopStatusPtr, HANDLE MutexHandle )
{
	BOOL ReleaseRes;
	DWORD WaitRes = WaitForSingleObject( MutexHandle, INFINITE );
	if ( WaitRes != WAIT_OBJECT_0 )
    { 
		if ( WaitRes == WAIT_ABANDONED )
		{
		    printf("Some thread has previously exited without releasing a mutex."
		            " This is not good programming. Please fix the code.\n" );
		    return ( ISP_MUTEX_ABANDONED );
		}
		else
		{
		    return( ISP_MUTEX_WAIT_FAILED );
		}
	}
                
    /* ........Critical Section Start................ */

	if ( TopStatusPtr->ArithmeticJobStatus == 100 || TopStatusPtr->GeometriceticJobStatus == 100 ||TopStatusPtr->DiffrenceJobStatus == 100 ) 
	{
		TopStatusPtr->ArithmeticJobStatus++;
        *stop = TRUE; /* handle last letter, then exit loop */
	}
	else
	{
		TopStatusPtr->ArithmeticJobStatus++;
        *stop = TRUE; /* handle last letter, then exit loop */
	}
	//GetNextProgression ( )

	/* ........Critical Section End.................. */
    
    ReleaseRes = ReleaseMutex( MutexHandle );

    if ( ReleaseRes == FALSE )
        return ( ISP_MUTEX_RELEASE_FAILED );

    return ( ISP_SUCCESS );

}

DWORD ThreadProgresion(ThreadParams* threadParamsPtr)
{
	int localTreadID =  GetCurrentThreadId();
	BOOL stop = FALSE;
	HANDLE MutexHandleTop;
	DWORD ExitCode = ISP_SUCCESS;/* if no errors occur, this value */ 
                                    /* will be returned */

	 MutexHandleTop = OpenMutex( 
        SYNCHRONIZE, /* default value */
        FALSE,       /* default value */
        MutexNameTop ); /* <ISP> This MUST be the EXACT same name as was used when */
                      /* the mutex was created. To save heartache, use a string */
                      /* constants ( as is done here ). */
    
    if ( MutexHandleTop == NULL )
	{
        if ( GetLastError() == ERROR_FILE_NOT_FOUND ){
            printf("A mutex with the requested name does not exit.\n");
		}
        ExitCode = ISP_MUTEX_OPEN_FAILED; 
        return 1;
    }
	while (!stop)
	{
		 //find job and work
		 ExitCode = GetNextJob( &stop, threadParamsPtr->TopStatusPtr, MutexHandleTop );
		 if ( ExitCode != ISP_SUCCESS ) 
		 {
			 return( ExitCode );
		 }
         break;
	}

	printf(" thread %d hello world %d\n",localTreadID,threadParamsPtr->TopStatusPtr->ArithmeticJobStatus);
	return ExitCode;
}




int CreateThreads(InputParams *inputParams)
{
	DWORD *threadIDs; /* An array of threadIDs */
	HANDLE *threadHandles; /* An array of thread handles */
	ThreadParams *threadParamsPtr;
	DWORD exitcode;
    HANDLE MutexHandleTop;
	HANDLE MutexHandleArithmetic;
	HANDLE MutexHandleGeometric;
	HANDLE MutexHandleDiffrence;
	int i;


	TopStatus* topStatus = CreateTopStatus();
	if (topStatus == NULL)
		return 0;

// ----------------------------- 
//   Create Mutexes            
// ----------------------------- 
	threadIDs = (DWORD*) malloc (inputParams->NumOfWorkers * sizeof(threadIDs));
	threadHandles = (HANDLE*) malloc (inputParams->NumOfWorkers * sizeof(threadHandles));
	threadParamsPtr = (ThreadParams*) malloc(inputParams->NumOfWorkers * sizeof(threadParamsPtr));
	if (threadParamsPtr ==NULL )
		exit(1);

	MutexHandleTop        = CreateMutexSimple( MutexNameTop );
	MutexHandleArithmetic = CreateMutexSimple( MutexNameArithmetic );
	MutexHandleGeometric  = CreateMutexSimple( MutexNameGeometric );
	MutexHandleDiffrence  = CreateMutexSimple( MutexNameDiffrence );

	for (i=0;i<inputParams->NumOfWorkers;i++)
	{
		threadParamsPtr[i].MutexHandleTop        = MutexHandleTop;
		threadParamsPtr[i].MutexHandleArithmetic = MutexHandleArithmetic;
		threadParamsPtr[i].MutexHandleGeometric  = MutexHandleGeometric;
		threadParamsPtr[i].MutexHandleDiffrence  = MutexHandleDiffrence;
		threadParamsPtr[i].TopStatusPtr          = topStatus;
		threadParamsPtr[i].InputParams			 = inputParams;
	}

// ----------------------------- 
//   Create Threads            
// ----------------------------- 

	
	if (threadHandles==NULL || threadIDs ==NULL )
		exit(1);
	
	for (i=0;i<inputParams->NumOfWorkers;i++)
	{
		threadHandles[i] = CreateThreadSimple(
		(LPTHREAD_START_ROUTINE) ThreadProgresion,  /*  thread function */
		(LPVOID) &(threadParamsPtr[i]),                            /*  argument to thread function */
		&threadIDs[i]); 
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

