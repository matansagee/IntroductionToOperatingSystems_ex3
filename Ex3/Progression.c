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

HANDLE MutexHandleTop       ;
HANDLE MutexHandleArithmetic;
HANDLE MutexHandleGeometric ;
HANDLE MutexHandleDiffrence ;

//---------------------------------------------------------------------------
// Function Decleration
//---------------------------------------------------------------------------

//Create Structs Function
TopStatus* CreateTopStatus();
SeriesData* CreateSeriesDataArray (InputParams *inputParams);
//Create Handlers Function
HANDLE CreateMutexSimple( LPCTSTR MutexName );
HANDLE CreateThreadSimple(  LPTHREAD_START_ROUTINE StartAddress,LPVOID ParameterPtr,LPDWORD ThreadIdPtr );
BOOL CreateProcessSimple(LPTSTR CommandLine, PROCESS_INFORMATION *ProcessInfoPtr);
//Artithmetic Function
float CalcArithmeticProgressionForItemI(float a1,int n, float d);
float CalcGeometricProgressionForItemI(float a1,int n,float q);
float CalaDifferenceProgressionForItemI(float a1,int n, float d, float q);
//Building and Cleaning Threads
void StartCleaningOperation(int N,int seriesNextJob, int subSeqArrayNumber, SubSeqArray *subSeqArray,int subSeqLength,FILE *file);
void StartBuildingOperation(SubSeqArray *subSeqArray, InputParams *inputParams, int startIndexInN, int startIndexInSubSequenceArray, SeriesType type);
//Main Function
int CreateThreads(InputParams *inputParams,FILE **files);
//Thread Function
DWORD ThreadProgresion( LPVOID Argument );
//Mutex Functions
DWORD RequestCleanerInTopStatus(ThreadParams *threadParams);
DWORD UpdateSeriesParams ( ThreadParams *threadParams );
DWORD GetNextJob(BOOL *stop, ThreadParams *threadParams );
DWORD UpdateTopStatus(ThreadParams *threadParams);
//MainLogic Function
SeriesType GetSeriesTypeAndUpdateThreadParams(ThreadParams *threadParams);
//Logic Support function
int WorkOnSeries ( ThreadParams *threadParams );
int UpdateThreadParamsByLogic(ThreadParams *threadParams);

//---------------------------------------------------------------------------
// Function Implentation
//---------------------------------------------------------------------------

TopStatus* CreateTopStatus()
{
	TopStatus *topStatus = (TopStatus*) malloc(sizeof(*topStatus));
	if (topStatus == NULL)
		return NULL;
	topStatus->ArithmeticJobsCounter = 0;
	topStatus->GeometriceticJobsCounter = 0;
	topStatus->DiffrenceJobsCounter = 0;
	topStatus->NumOfWorkersOnArithmetic = 0;
	topStatus->NumOfWorkersOnDiffrence = 0;
	topStatus->NumOfWorkersOnGeometric = 0;
	topStatus->ArithmeticNeedClean = FALSE;
	topStatus->GeometricNeedClean = FALSE;
	topStatus->DiffrenceNeedClean = FALSE;
	topStatus->ArithmeticCleanOnProgress= FALSE;
	topStatus->GeometricCleanOnProgress	= FALSE;
	topStatus->DiffrenceCleanOnProgress	= FALSE;
	return topStatus;
}

float CalcArithmeticProgressionForItemI(float a1,int n, float d)
{
	return (a1+ (float)(n-1)*d);
}

float CalcGeometricProgressionForItemI(float a1,int n,float q)
{
	return a1*pow(q,(float) (n-1));
}

float CalaDifferenceProgressionForItemI(float a1,int n, float d, float q)
{
	return CalcGeometricProgressionForItemI(a1,n,q) - CalcArithmeticProgressionForItemI(a1,n,d);
}
/**
* CreateProcessSimple uses the win32 API to create a process that runs the
* command in 'CommandLine'. it uses the win32 API function CreateProcess()
* using default values for most parameters.
*
* Accepts:
* --------
* CommandLine - a windows generic string containing the command that the new
*               process performs. ( See CreateProcess( documentation for more ).
* ProcessInfoPtr - an output parameter, used to return a PROCESS_INFORMATION
*					structure containing data about the process that was created.
*					( See CreateProcess() documentation for more ).
* Returns:
* --------
* the output of CreateProcess().
*/
BOOL CreateProcessSimple(LPTSTR CommandLine, PROCESS_INFORMATION *ProcessInfoPtr)
{
	STARTUPINFO    startinfo = { sizeof(STARTUPINFO), NULL, 0 }; /* <ISP> here we */
																 /* initialize a "Neutral" STARTUPINFO variable. Supplying this to */
																 /* CreateProcess() means we have no special interest in this parameter. */
																 /* This is equivalent to what we are doing by supplying NULL to most other */
																 /* parameters of CreateProcess(). */

	return CreateProcess(NULL, /*  No module name (use Command line). */
		CommandLine,            /*  Command line. */
		NULL,                    /*  Process handle not inheritable. */
		NULL,                    /*  Thread handle not inheritable. */
		FALSE,                    /*  Set handle inheritance to FALSE. */
		NORMAL_PRIORITY_CLASS,    /*  creation/priority flags. */
		NULL,                    /*  Use parent's environment block. */
		NULL,                    /*  Use parent's starting directory. */
		&startinfo,                /*  Pointer to STARTUPINFO structure. */
		ProcessInfoPtr            /*  Pointer to PROCESS_INFORMATION structure. */
		);
}

//------------------------------------------------------------------------------------
// StartBuildingOperation - in case of "builder" thread - calculate the series by kind,
//                          according to decision that has been taken erlier
//------------------------------------------------------------------------------------


void StartBuildingOperation(SubSeqArray *subSeqArray, InputParams *inputParams, int startIndexInN, int startIndexInSubSequenceArray, SeriesType type)
{
	int i;
	int index;
	int n;

	
	for (i = 0; i < inputParams->JobSize; i++)
	{
		index = startIndexInSubSequenceArray/*inputParams->JobSize*/ + i;
		n = i + startIndexInN+1;
		switch(type)
		{
		case ARITHMETIC :
			subSeqArray[index].Value = CalcArithmeticProgressionForItemI(inputParams->A1, n, inputParams->d);
			printf("a%d of ARITHNETIC is %f\n",n,subSeqArray[index].Value);
			break;
		case GEOMETRIC:
			subSeqArray[index].Value = CalcGeometricProgressionForItemI(inputParams->A1, n, inputParams->q);
			printf("a%d of GEOMETRIC is %f\n",n,subSeqArray[index].Value);
			break;
		case DIFFERENCEPROG:
			subSeqArray[index].Value = CalaDifferenceProgressionForItemI(inputParams->A1, n, inputParams->d, inputParams->q);
			printf("a%d of DIFFRENCE is %f\n",n,subSeqArray[index].Value);
			break;
		}
		subSeqArray[index].ThreadNumber = GetCurrentThreadId();
		GetLocalTime(&subSeqArray[index].lpSystemTime);
	}
}

//----------------------------------------------------------------------------------------
// StartCleaningOperation - "cleaning" thread function - write values from sub_seq to file
//----------------------------------------------------------------------------------------

void StartCleaningOperation(int N,int seriesNextJob, int subSeqArrayNumber, SubSeqArray *subSeqArray,int subSeqLength,FILE *file)
{
	int j = 0;
	int index;
	for (j = 0; j < subSeqLength; j++)
	{
		index = subSeqArrayNumber*subSeqLength + j +1;
		printf("%d - Index #%d = %f, calculated by thread %d @ %02d:%02d:%02d.%03d\n", 
			seriesNextJob,
			index, 
			subSeqArray[j].Value, 
			subSeqArray[j].ThreadNumber, 
			subSeqArray[j].lpSystemTime.wHour,
			subSeqArray[j].lpSystemTime.wMinute,
			subSeqArray[j].lpSystemTime.wSecond,
			subSeqArray[j].lpSystemTime.wMilliseconds);
		fprintf(file,"Index #%d = %f, calculated by thread %d @ %02d:%02d:%02d.%03d\n", 
			index, 
			subSeqArray[j].Value, 
			subSeqArray[j].ThreadNumber, 
			subSeqArray[j].lpSystemTime.wHour,
			subSeqArray[j].lpSystemTime.wMinute,
			subSeqArray[j].lpSystemTime.wSecond,
			subSeqArray[j].lpSystemTime.wMilliseconds);
		if (index == N)
			break;
	}

}


HANDLE CreateThreadSimple(  LPTHREAD_START_ROUTINE StartAddress,LPVOID ParameterPtr,LPDWORD ThreadIdPtr )
{
	return CreateThread(
		NULL,            /*  default security attributes */
		0,                /*  use default stack size */
		StartAddress,    /*  thread function */
		ParameterPtr,    /*  argument to thread function */
		0,                /*  use default creation flags */
		ThreadIdPtr );    /*  returns the thread identifier */
}


HANDLE CreateMutexSimple( LPCTSTR MutexName )
{
	return CreateMutex( 
		NULL,              // default security attributes
		FALSE,             // initially not owned
		MutexName);             
}

//----------------------------------------------------------------------------------------
// RequestCleanerInTopStatus - the function asking to get to highest database TopStatus and
//                             write cleaning request (using mutex)
//----------------------------------------------------------------------------------------

DWORD RequestCleanerInTopStatus(ThreadParams *threadParams)
{
	DWORD WaitRes;
	SeriesType currentJobs;
	__try 
	{
		WaitRes = WaitForSingleObject( MutexHandleTop, INFINITE );
		switch (WaitRes) 
		{
		case WAIT_OBJECT_0: 
			{
				printf("%d is holding mutex %d\n",GetCurrentThreadId(),MutexHandleTop);
				switch(threadParams->SeriesNextJob)
				{
				case ARITHMETIC:		threadParams->TopStatus->ArithmeticNeedClean= TRUE; break;
				case GEOMETRIC:			threadParams->TopStatus->GeometricNeedClean = TRUE; break;
				case DIFFERENCEPROG:	threadParams->TopStatus->DiffrenceNeedClean = TRUE; break;
				}
			} 
			break; 
		case WAIT_ABANDONED: 
			return FALSE; 
		}
	}
	__finally 
	{ 
		if ( !ReleaseMutex(MutexHandleTop)) {
			return ( ISP_MUTEX_RELEASE_FAILED );
		} 
		printf("%d is releasing mutex %d\n",GetCurrentThreadId(),MutexHandleTop);
		return ( ISP_SUCCESS );
	}
}

//----------------------------------------------------------------------------------------
// UpdateThreadParamsByLogic - after job_size job is done - update lower database of the 
//                             series that job is done and if any cleaning requierd
//----------------------------------------------------------------------------------------

int UpdateThreadParamsByLogic(ThreadParams *threadParams)
{
	int subSeqLength = threadParams->InputParams->SubSeqLength;
	int jobSize = threadParams->InputParams->JobSize;
	int N = threadParams->InputParams->N;
	int maxWorkersOnSeries = (subSeqLength / jobSize);

	printf("%d, I'm builder and i finished job %d on series %d\n",GetCurrentThreadId(),threadParams->SeriesData[(int)(threadParams->SeriesNextJob)].NumOfSubJobsDone,threadParams->SeriesNextJob); 
	// update series
	threadParams->SeriesData[(int)(threadParams->SeriesNextJob)].NumOfSubJobsDone++;
	if (threadParams->SeriesData[threadParams->SeriesNextJob].NumOfSubJobsDone == maxWorkersOnSeries)
	{
		RequestCleanerInTopStatus(threadParams);
		threadParams->SeriesData[threadParams->SeriesNextJob].NumOfSubJobsDone = 0;
	}
	//handle N % SubSeqLength != 0
	if ((threadParams->n) + jobSize == N)
	{
		RequestCleanerInTopStatus(threadParams);
		threadParams->SeriesData[threadParams->SeriesNextJob].NumOfSubJobsDone = 0;
	}

}

//----------------------------------------------------------------------------------------
// UpdateSeriesParams - ask for mutex inorder to update parameters of series, then call
//                      UpdateThreadParamsByLogic
//----------------------------------------------------------------------------------------

DWORD UpdateSeriesParams ( ThreadParams *threadParams )
{
	HANDLE handle;
	DWORD waitRes;
	DWORD exitCode;
	switch(threadParams->SeriesNextJob)
	{
	case ARITHMETIC:		handle = MutexHandleArithmetic; break;
	case GEOMETRIC:			handle = MutexHandleGeometric; break;
	case DIFFERENCEPROG:	handle = MutexHandleDiffrence; break;
	}
	__try 
	{
		waitRes = WaitForSingleObject( handle, INFINITE );
		switch (waitRes) 
		{
			// The thread got ownership of the mutex
		case WAIT_OBJECT_0: 
			{
				printf("%d is holding mutex %d\n",GetCurrentThreadId(),threadParams->SeriesNextJob);
				exitCode = UpdateThreadParamsByLogic(threadParams);
				if (exitCode != ISP_SUCCESS)
					return exitCode;
			} 
			break; 
			// The thread got ownership of an abandoned mutex
		case WAIT_ABANDONED: 
			return FALSE; 
		}
	}
	__finally
	{ 
		printf("%d is releasing mutex %d\n",GetCurrentThreadId(),threadParams->SeriesNextJob);
		if ( !ReleaseMutex(handle)) {
			return ( ISP_MUTEX_RELEASE_FAILED );
		} 
	}
	return ISP_SUCCESS;
}

//----------------------------------------------------------------------------------------
// UpdateTopStatus - update highes database after cleaning job (through mutex)
//----------------------------------------------------------------------------------------

DWORD UpdateTopStatus(ThreadParams *threadParams)
{
	DWORD WaitRes;
	SeriesType currentJobs;
	__try 
	{
		WaitRes = WaitForSingleObject( MutexHandleTop, INFINITE );
		switch (WaitRes) 
		{
		case WAIT_OBJECT_0: 
			{
				printf("%d is holding mutex %d\n",GetCurrentThreadId(),MutexHandleTop);
				switch(threadParams->SeriesNextJob)
				{
				case ARITHMETIC:		threadParams->TopStatus->ArithmeticCleanOnProgress= FALSE; break;
				case GEOMETRIC:			threadParams->TopStatus->GeometricCleanOnProgress = FALSE; break;
				case DIFFERENCEPROG:	threadParams->TopStatus->DiffrenceCleanOnProgress = FALSE; break;
				}
			} 
			break; 
		case WAIT_ABANDONED: 
			return FALSE; 
		}
	}
	__finally { 
		if ( !ReleaseMutex(MutexHandleTop)) {
			return ( ISP_MUTEX_RELEASE_FAILED );
		} 

		printf("%d is releasing mutex %d\n",GetCurrentThreadId(),MutexHandleTop);
		return ( ISP_SUCCESS );
	}
}

//----------------------------------------------------------------------------------------
// WorkOnSeries - according to erlier decision (thread type and series) call job function
//----------------------------------------------------------------------------------------

int WorkOnSeries ( ThreadParams *threadParams )
{
	int startIndexInN = threadParams->n;
	int seriesDataInd;
	int startIndexInSubSequenceArray = threadParams->n % threadParams->InputParams->SubSeqLength;
	int subSeqArrayNumber =  threadParams->n / threadParams->InputParams->SubSeqLength;
	switch(threadParams->SeriesNextJob)
	{
	case ARITHMETIC:		seriesDataInd = 0; break;
	case GEOMETRIC:			seriesDataInd = 1; break;
	case DIFFERENCEPROG:	seriesDataInd = 2; break;
	}

	switch(threadParams->WorkerType)
	{
	case BUILDER: 
		{
			StartBuildingOperation(threadParams->SeriesData[seriesDataInd].SubSeqArray, threadParams->InputParams, startIndexInN, startIndexInSubSequenceArray, threadParams->SeriesNextJob);
			UpdateSeriesParams(threadParams);
		}
		break;
	case CLEANER: 
		{
			printf("%d,I'm cleaner of series %d\n",GetCurrentThreadId(), threadParams->SeriesNextJob); 
			StartCleaningOperation(threadParams->InputParams->N,
				threadParams->SeriesNextJob,subSeqArrayNumber, 
				threadParams->SeriesData[seriesDataInd].SubSeqArray,
				threadParams->InputParams->SubSeqLength,
				threadParams->files[seriesDataInd]);
			UpdateTopStatus(threadParams);			
		}
		break;
	}
	return ISP_SUCCESS;
}

//----------------------------------------------------------------------------------------
// GetSeriesTypeAndUpdateThreadParams - the "main maneger" based on highest database - 
//                                      "TopStatus", first priority to send thread if there
//                                      any cleaning requests, second priority - decide on
//                                      which series to work on
//----------------------------------------------------------------------------------------


SeriesType GetSeriesTypeAndUpdateThreadParams(ThreadParams *threadParams)
{
	int N = threadParams->InputParams->N;
	int subSeqLength = threadParams->InputParams->SubSeqLength;
	int jobSize = threadParams->InputParams->JobSize;
	int totalJobsInSeries = N/jobSize;
	int minTotalJobs = totalJobsInSeries;
	int maxWorkersOnSeries = (subSeqLength / jobSize);
	SeriesType candidate = DONOTHING;
	//check if cleaners is required
	if (threadParams->TopStatus->ArithmeticNeedClean)
	{
		threadParams->TopStatus->NumOfWorkersOnArithmetic = 0;
		threadParams->TopStatus->ArithmeticCleanOnProgress = TRUE;
		threadParams->TopStatus->ArithmeticNeedClean = FALSE;

		candidate = ARITHMETIC;
		threadParams->WorkerType = CLEANER;
		return candidate;
	}

	if (threadParams->TopStatus->GeometricNeedClean)
	{
		threadParams->TopStatus->NumOfWorkersOnGeometric = 0;
		threadParams->TopStatus->GeometricCleanOnProgress = TRUE;
		threadParams->TopStatus->GeometricNeedClean = FALSE;
		candidate = GEOMETRIC;
		threadParams->WorkerType = CLEANER;
		return candidate;
	}

	if (threadParams->TopStatus->DiffrenceNeedClean)
	{
		threadParams->TopStatus->NumOfWorkersOnDiffrence = 0;
		threadParams->TopStatus->DiffrenceCleanOnProgress = TRUE;
		threadParams->TopStatus->DiffrenceNeedClean = FALSE;
		candidate = DIFFERENCEPROG;
		threadParams->WorkerType = CLEANER;
		return candidate;
	}
	//assign Builder
	if (threadParams->TopStatus->ArithmeticJobsCounter < minTotalJobs
		&& threadParams->TopStatus->NumOfWorkersOnArithmetic < maxWorkersOnSeries
		&& !threadParams->TopStatus->ArithmeticCleanOnProgress)
	{
		threadParams->n = threadParams->TopStatus->ArithmeticJobsCounter * jobSize;
		threadParams->WorkerType = BUILDER;
		candidate = ARITHMETIC;
		minTotalJobs = threadParams->TopStatus->ArithmeticJobsCounter;
	}
	if (threadParams->TopStatus->GeometriceticJobsCounter < minTotalJobs 
		&& threadParams->TopStatus->NumOfWorkersOnGeometric < maxWorkersOnSeries
		&& !threadParams->TopStatus->GeometricCleanOnProgress)
	{
		threadParams->n = threadParams->TopStatus->GeometriceticJobsCounter * jobSize;
		threadParams->WorkerType = BUILDER;
		candidate = GEOMETRIC;
		minTotalJobs = threadParams->TopStatus->GeometriceticJobsCounter;
	}
	if (threadParams->TopStatus->DiffrenceJobsCounter < minTotalJobs 
		&& threadParams->TopStatus->NumOfWorkersOnDiffrence < maxWorkersOnSeries
		&& !threadParams->TopStatus->DiffrenceCleanOnProgress)
	{
		threadParams->n = threadParams->TopStatus->DiffrenceJobsCounter * jobSize;
		threadParams->WorkerType = BUILDER;
		candidate = DIFFERENCEPROG;
		minTotalJobs = threadParams->TopStatus->DiffrenceJobsCounter;
	}

	switch(candidate)
	{
	case DIFFERENCEPROG : 
		{
			threadParams->n = threadParams->TopStatus->DiffrenceJobsCounter * jobSize;
			threadParams->TopStatus->DiffrenceJobsCounter++;
			threadParams->TopStatus->NumOfWorkersOnDiffrence++;
		}
		break;
	case ARITHMETIC : 
		{
			threadParams->n = threadParams->TopStatus->ArithmeticJobsCounter * jobSize;
			threadParams->TopStatus->ArithmeticJobsCounter++; 
			threadParams->TopStatus->NumOfWorkersOnArithmetic++;
		}
		break;
	case GEOMETRIC : 
		{
			threadParams->n = threadParams->TopStatus->GeometriceticJobsCounter * jobSize;
			threadParams->TopStatus->GeometriceticJobsCounter++; 
			threadParams->TopStatus->NumOfWorkersOnGeometric++; 
		}
		break;
	}

	if ( (threadParams->TopStatus->DiffrenceJobsCounter == totalJobsInSeries)
		 && (threadParams->TopStatus->GeometriceticJobsCounter == totalJobsInSeries)
		 && (threadParams->TopStatus->ArithmeticJobsCounter == totalJobsInSeries)
		 && (candidate == DONOTHING) )
		 candidate = THREADDONE;
		printf("%d got a job on %d, the updated n is %d \n",GetCurrentThreadId(), candidate,threadParams->n); 
	return candidate;


}

//----------------------------------------------------------------------------------------
// GetNextJob - ask for mutex inorder to get to highest database - TopStatus and then 
//                                      call GetSeriesTypeAndUpdateThreadParams
//----------------------------------------------------------------------------------------

DWORD GetNextJob(BOOL *stop, ThreadParams *threadParams )
{
	DWORD WaitRes;
	SeriesType currentJobs;
	__try 
	{
		WaitRes = WaitForSingleObject( MutexHandleTop, INFINITE );
		switch (WaitRes) 
		{
			// The thread got ownership of the mutex
		case WAIT_OBJECT_0: 
			{
				printf("%d is holding mutex %d\n",GetCurrentThreadId(),MutexHandleTop);
				currentJobs = GetSeriesTypeAndUpdateThreadParams(threadParams);
				if (currentJobs == THREADDONE)
				{
					threadParams->SeriesNextJob = currentJobs;
					*stop = TRUE; /* handle last letter, then exit loop */
					break;
				}
			
				threadParams->SeriesNextJob = currentJobs;
			} 
			break; 
			// The thread got ownership of an abandoned mutex
		case WAIT_ABANDONED: 
			return FALSE; 
		}
	}
	__finally { 
		// Release ownership of the mutex object
		if ( !ReleaseMutex(MutexHandleTop)) 
		{
			return ( ISP_MUTEX_RELEASE_FAILED );
			// Handle error.
		} 

		printf("%d is releasing mutex %d\n",GetCurrentThreadId(),MutexHandleTop);
		return ( ISP_SUCCESS );
	}

}

//----------------------------------------------------------------------------------------
// ThreadProgresion - main thread function 
//----------------------------------------------------------------------------------------

DWORD ThreadProgresion( LPVOID Argument )
{
	ThreadParams *threadParams = (ThreadParams*) Argument;
	int localTreadID =  GetCurrentThreadId();
	BOOL stop = FALSE;
	DWORD ExitCode = ISP_SUCCESS;/* if no errors occur, this value */ 

	while (!stop)
	{
		//find job and work
		ExitCode = GetNextJob( &stop, threadParams );
		if ( ExitCode != ISP_SUCCESS )
		{
			return ( ExitCode );
		}
		if (threadParams->SeriesNextJob == THREADDONE)
		{
			return ISP_SUCCESS;
		}
		if (threadParams->SeriesNextJob != DONOTHING)
		{
        	WorkOnSeries(threadParams);
		}
	} // End of while
	//printf("thread %d Job is - %d\n",localTreadID,ExitCode);

	return ExitCode;
}

SeriesData* CreateSeriesDataArray (InputParams *inputParams)
{
	int i;
	SeriesData *seriesDataArray = (SeriesData*) malloc (3*sizeof(*seriesDataArray));
	for (i=0;i<3;i++)
	{
		seriesDataArray[i].NumOfSubJobsDone = 0;
		seriesDataArray[i].SubSeqArray = (SubSeqArray*) malloc(inputParams->SubSeqLength*sizeof(*seriesDataArray[i].SubSeqArray));
		if (seriesDataArray[i].SubSeqArray == NULL)
			exit(1);
		seriesDataArray[i].SubSeqArray->ThreadNumber=0;
		seriesDataArray[i].SubSeqArray->Value=0;
		seriesDataArray[i].SubSeqArray->ValueValid=0;
	}
	return seriesDataArray;
}


int CreateThreads(InputParams *inputParams,FILE **files)
{
	DWORD *threadIDs; /* An array of threadIDs */
	HANDLE *threadHandles; /* An array of thread handles */
	ThreadParams *threadParams;
	SeriesData *seriesDataArray;
	DWORD waitRes; 
	DWORD exitcode;
	int ThreadInd;
	int i;
	TopStatus* topStatus = CreateTopStatus();
	threadHandles = (HANDLE*) malloc (inputParams->NumOfWorkers * sizeof(*threadHandles));
	threadParams = (ThreadParams*) malloc(inputParams->NumOfWorkers * sizeof(*threadParams));
	seriesDataArray = CreateSeriesDataArray (inputParams);
	if (threadParams ==NULL || threadHandles == NULL || topStatus == NULL)
		exit(1);
	__try
	{
		MutexHandleTop        = CreateMutexSimple( MutexNameTop );
		if (MutexHandleTop == NULL) 
		{
			printf("CreateMutex error: %d\n", GetLastError());
		}
		MutexHandleArithmetic = CreateMutexSimple( MutexNameArithmetic );
		if (MutexHandleArithmetic == NULL) 
		{
			printf("CreateMutex error: %d\n", GetLastError());
		}
		MutexHandleGeometric  = CreateMutexSimple( MutexNameGeometric );
		if (MutexHandleGeometric == NULL) 
		{
			printf("CreateMutex error: %d\n", GetLastError());
		}
		MutexHandleDiffrence  = CreateMutexSimple( MutexNameDiffrence );
		if (MutexHandleDiffrence == NULL) 
		{
			printf("CreateMutex error: %d\n", GetLastError());
		}

		for (ThreadInd=0;ThreadInd<inputParams->NumOfWorkers;ThreadInd++)
		{
			threadParams[ThreadInd].TopStatus   = topStatus;
			threadParams[ThreadInd].InputParams	= inputParams;
			threadParams[ThreadInd].SeriesData	= seriesDataArray;
			threadParams[ThreadInd].files		= files;
			threadHandles[ThreadInd] = CreateThreadSimple(
				(LPTHREAD_START_ROUTINE)ThreadProgresion,
				(LPVOID) &(threadParams[ThreadInd]),
				NULL); 

			if ( threadHandles[ThreadInd] == NULL )
			{
				printf("Failed to create Thread. Exiting program.\n");
				break;
			}
		}

		waitRes = WaitForMultipleObjects(
			inputParams->NumOfWorkers,
			threadHandles,
			TRUE,       /* wait until all threads finish */
			INFINITE);

		if ( waitRes == WAIT_FAILED )
		{ 
			printf("Waiting for threads failed. Ending program.\n"); 
			exit(1);
		}
	}
	__finally
	{
		for (i = 0; i < inputParams->NumOfWorkers; i++)
		{
			GetExitCodeThread(threadHandles[i], &exitcode);
			//printf("Thread number %d returned exit code %d\n", i, exitcode);
			CloseHandle(threadHandles[i]);
		}

		CloseHandle(MutexHandleTop);
		CloseHandle(MutexHandleArithmetic);
		CloseHandle(MutexHandleGeometric);
		CloseHandle(MutexHandleDiffrence);
	}
	return 0;
}

