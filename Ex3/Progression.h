#define _CRT_SECURE_NO_WARNINGS /* to suppress Visual Studio 2010 compiler warning */
#include <stdio.h>
#include <stdlib.h>
#include "windows.h"
#include <math.h>
#include <string.h>

enum CALCULATION {ARITHMETIC,GEOMETRIC,DIFFERENCEPROG};

typedef char* string; 
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

typedef struct SubSeqArray
{
	int Value;
	int ThreadNumber;
	int Time;
}SubSeqArray;

void DoCalculations(InputParams *inputParams,FILE **files);
