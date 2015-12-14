#define _CRT_SECURE_NO_WARNINGS /* to suppress Visual Studio 2010 compiler warning */

#ifndef __PROGRESSION_H
#define __PROGRESSION_H


#include <stdio.h>
#include <stdlib.h>
#include "windows.h"
#include <math.h>
#include <string.h>
#include <tchar.h>

enum CALCULATION {ARITHMETIC,GEOMETRIC,DIFFERENCEPROG};



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
    ISP_ILLEGAL_LETTER_WAS_READ
} ErrorCode_t;

typedef struct InputParams
{
	int NumOfWorkers;
	int N;
	int JobSize;
	int SubSeqLength;
	string FailurePeriod;
	int A1;
	int d;
	int q;

}InputParams;

typedef struct TopStatus
{
	int ArithmeticJobStatus;
	int GeometriceticJobStatus;
	int DiffrenceJobStatus;

}TopStatus;

typedef struct ThreadParams
{
	 HANDLE MutexHandleTop;
	 HANDLE MutexHandleArithmetic;
	 HANDLE MutexHandleGeometric;
	 HANDLE MutexHandleDiffrence;
     TopStatus *TopStatusPtr;
	// InputParams *InputParams;
}ThreadParams;

typedef struct SubSeqArray
{
	float Value;
	float ThreadNumber;
	float Time;
}SubSeqArray;


void DoCalculations(InputParams *inputParams,FILE **files);
int CreateThreads(InputParams *inputParams);

#endif //PROGRESSION_H
