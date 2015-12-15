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

int CalcArithmeticProgressionForItemI(int a1,int n, int d)
{
	return a1+((n+2)-1)*d;
}

double CalcGeometricProgressionForItemI(int a1,int n,int q)
{
	return a1*pow((double) q,(double) (n+2)-1);
}

float CalaDifferenceProgressionForItemI(int a1,int n, int d, int q)
{
	return (float) CalcGeometricProgressionForItemI(a1,n,q) - (float) CalcArithmeticProgressionForItemI(a1,n,d);
}

void CalculateSeries(SubSeqArray *subSeqArray, InputParams *inputParams, int startIndexInN, int startIndexInSubSequenceArray, SeriesType type)
{
	int i;
	for (i = 0; i < inputParams->JobSize; i++)
	{
		if (type == ARITHMETIC)
		{
			subSeqArray[startIndexInSubSequenceArray*inputParams->JobSize + i].Value = (float) CalcArithmeticProgressionForItemI(inputParams->A1, i + startIndexInN, inputParams->d);
			subSeqArray[startIndexInSubSequenceArray*inputParams->JobSize + i].ThreadNumber = GetCurrentThreadId();
			subSeqArray[startIndexInSubSequenceArray*inputParams->JobSize + i].Time = 11;//TODO
		}
		if (type == GEOMETRIC)
		{
			subSeqArray[startIndexInSubSequenceArray*inputParams->JobSize + i].Value = (float) CalcGeometricProgressionForItemI(inputParams->A1, i + startIndexInN, inputParams->q);
			subSeqArray[startIndexInSubSequenceArray*inputParams->JobSize + i].ThreadNumber = GetCurrentThreadId();
			subSeqArray[startIndexInSubSequenceArray*inputParams->JobSize + i].Time = 11;//TODO
		}
		if (type == DIFFERENCEPROG)
		{
			subSeqArray[startIndexInSubSequenceArray*inputParams->JobSize + i].Value = (float) CalaDifferenceProgressionForItemI(inputParams->A1, i + startIndexInN, inputParams->d, inputParams->q);
			subSeqArray[startIndexInSubSequenceArray*inputParams->JobSize + i].ThreadNumber = GetCurrentThreadId();
			subSeqArray[startIndexInSubSequenceArray*inputParams->JobSize + i].Time = 11;//TODO
		}
	}
}

void CalculateSeriess(int numOfWorkers, int iterationNumber, InputParams *inputParams, SubSeqArray *subSeqArray,SeriesType type)
{
	int subSeq = inputParams->SubSeqLength;
	int jobSize = inputParams->JobSize;
	int threadNumber;
	int startIndex = iterationNumber*subSeq;
	int stopIndex = (iterationNumber + 1)*subSeq;
	//spliiting to thread
	for (threadNumber = 0; threadNumber < subSeq/jobSize; threadNumber++)
	{
		CalculateSeries(subSeqArray, inputParams, startIndex + (threadNumber*jobSize),threadNumber,type);
	}
}
void StartCleaningThread(int subSeqArrayNumber, SubSeqArray *subSeqArray,int subSeqLength,FILE *file)
{
	int j = 0;
	int index;
	for (j = 0; j < subSeqLength; j++)
	{
		index = subSeqArrayNumber*subSeqLength + j;
		printf("Index #%d = %d, calculated by thread %d @ %d\n", index, subSeqArray[j].Value, subSeqArray[j].ThreadNumber, subSeqArray[j].Time);
		//fprintf(file,"Index #%d = %d, calculated by thread %d @ %d\n", index, subSeqArray[j].Value, subSeqArray[j].ThreadNumber, subSeqArray[j].Time);
	}

}

void CalculateSeriesByType(int numOfWorkers,InputParams *inputParams, FILE* file,SeriesType type)
{
	int iterationNumber = inputParams->N / inputParams->SubSeqLength;
	int i;
	SubSeqArray *subSeqArray = (SubSeqArray*)malloc(inputParams->SubSeqLength*sizeof(subSeqArray));
	if (subSeqArray == NULL)
		exit(1);

	printf("CalculateSeriesByType %d\n",type);
	for (i = 0; i < iterationNumber; i++)
	{	
		CalculateSeriess(numOfWorkers,i, inputParams, subSeqArray,type);
		StartCleaningThread(i,subSeqArray, inputParams->SubSeqLength,file);
	}
}


void DoCalculations(InputParams *inputParams,FILE **files)
{
	CalculateSeriesByType(inputParams->NumOfWorkers / 2,inputParams, files[0],ARITHMETIC);
	CalculateSeriesByType(inputParams->NumOfWorkers / 2, inputParams, files[1], GEOMETRIC);
	//CalculateSeriesByType(inputParams->NumOfWorkers / 2, inputParams, files[2], DIFFERENCEPROG);
}

HANDLE CreateThreadSimple(  LPTHREAD_START_ROUTINE StartAddress, 
	LPVOID ParameterPtr, 
	LPDWORD ThreadIdPtr )
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

int UpdateThreadParamsByLogic(ThreadParams *threadParams)
{
	int subSeqLength = threadParams->InputParams->SubSeqLength;
	int jobSize = threadParams->InputParams->JobSize;
	int maxWorkersOnSeries = (subSeqLength / jobSize);

	printf("%d, I'm builder and i finished job %d\n",GetCurrentThreadId(),threadParams->SeriesData[(int)(threadParams->SeriesNextJob)].NumOfSubJobsDone); 
	// update series
	threadParams->SeriesData[(int)(threadParams->SeriesNextJob)].NumOfSubJobsDone++;
	if (threadParams->SeriesData[threadParams->SeriesNextJob].NumOfSubJobsDone == maxWorkersOnSeries)
	{
		RequestCleanerInTopStatus(threadParams);
		threadParams->SeriesData[threadParams->SeriesNextJob].NumOfSubJobsDone = 0;
	}

}
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
int UpdateTopStatus(ThreadParams *threadParams)
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
int WorkOnSeries ( ThreadParams *threadParams )
{
	int startIndexInN = threadParams->n;
	int startIndexInSubSequenceArray = threadParams->n % threadParams->InputParams->SubSeqLength;
	int subSeqArrayNumber =  threadParams->n / threadParams->InputParams->SubSeqLength;
	switch(threadParams->WorkerType)
	{
	case BUILDER: 
		{
			CalculateSeries(threadParams->SeriesData->SubSeqArray, threadParams->InputParams, startIndexInN, startIndexInSubSequenceArray, threadParams->SeriesNextJob);
			UpdateSeriesParams(threadParams);
		}
		break;
	case CLEANER: 
		{
			printf("%d,I'm cleaner\n",GetCurrentThreadId()); 
			StartCleaningThread(subSeqArrayNumber, threadParams->SeriesData->SubSeqArray,threadParams->InputParams->SubSeqLength,NULL);
			UpdateTopStatus(threadParams);			
		}
		break;
	}
	return ISP_SUCCESS;
}


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
	return candidate;
}


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
				if (currentJobs == DONOTHING)
				{
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
		WorkOnSeries(threadParams);
	} // End of while
	printf("thread %d Job is - %d\n",localTreadID,ExitCode);

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
		seriesDataArray[i].SubSeqArray->Time=0;
		seriesDataArray[i].SubSeqArray->Value=0;
		seriesDataArray[i].SubSeqArray->ValueValid=0;
	}
	return seriesDataArray;
}


int CreateThreads(InputParams *inputParams)
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

	/* Wait for all threads to end: */
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

	//Sleep(10);

	for (i = 0; i < inputParams->NumOfWorkers; i++)
	{
		GetExitCodeThread(threadHandles[i], &exitcode);
		printf("Thread number %d returned exit code %d\n", i, exitcode);
		CloseHandle(threadHandles[i]);
	}

	CloseHandle(MutexHandleTop);
	CloseHandle(MutexHandleArithmetic);
	CloseHandle(MutexHandleGeometric);
	CloseHandle(MutexHandleDiffrence);

	return 0;
}

