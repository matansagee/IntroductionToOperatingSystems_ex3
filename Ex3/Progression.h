#define _CRT_SECURE_NO_WARNINGS /* to suppress Visual Studio 2010 compiler warning */

#ifndef __PROGRESSION_H
#define __PROGRESSION_H


#include <stdio.h>
#include <stdlib.h>
#include "windows.h"
#include <math.h>
#include <string.h>
#include <tchar.h>

typedef enum {ARITHMETIC,GEOMETRIC,DIFFERENCEPROG, DONOTHING , THREADDONE} SeriesType;
typedef enum {CLEANER, BUILDER} WorkerType;

typedef char* string; 

typedef enum { 
    ISP_SUCCESS, 
    ISP_FILE_OPEN_FAILED, 
    ISP_FILE_SEEK_FAILED,
    ISP_FILE_READING_FAILED,
    ISP_FTELL_FAILED,
    ISP_MUTEX_OPEN_FAILED,
    ISP_MUTEX_CREATE_FAILED,
    ISP_MUTEX_WAIT_FAILED,
    ISP_MUTEX_ABANDONED,
    ISP_MUTEX_RELEASE_FAILED,
    ISP_ILLEGAL_LETTER_WAS_READ,
} ErrorCode_t;

typedef struct InputParams
{
	int NumOfWorkers;
	int N;
	int JobSize;
	int SubSeqLength;
	string FailurePeriod;
	float A1;
	float d;
	float q;

}InputParams;

typedef struct TopStatus
{
	int ArithmeticJobsCounter;
	int GeometriceticJobsCounter;
	int DiffrenceJobsCounter;
	int NumOfWorkersOnArithmetic;
	int NumOfWorkersOnGeometric;
	int NumOfWorkersOnDiffrence;
	BOOL ArithmeticNeedClean;
	BOOL GeometricNeedClean;
	BOOL DiffrenceNeedClean;
	BOOL ArithmeticCleanOnProgress;
	BOOL GeometricCleanOnProgress;
	BOOL DiffrenceCleanOnProgress;

}TopStatus;

typedef struct SubSeqArray
{
	float Value;
	int ThreadNumber;
	SYSTEMTIME lpSystemTime;
	BOOL ValueValid;
}SubSeqArray;

typedef struct SeriesData
{
	int NumOfSubJobsDone;
	SubSeqArray *SubSeqArray;
}SeriesData;


typedef struct ThreadParams
{
     TopStatus   *TopStatus;
	 InputParams *InputParams;
	 SeriesType   SeriesNextJob;
	 WorkerType  WorkerType;
	 SeriesData *SeriesData;
	 int	n;
	 FILE **files;
}ThreadParams;




void DoCalculations(InputParams *inputParams,FILE **files);
int CreateThreads(InputParams *inputParams,FILE **files);

#endif //PROGRESSION_H
