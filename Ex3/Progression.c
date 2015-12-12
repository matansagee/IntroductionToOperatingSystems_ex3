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
			subSeqArray[threadNumber*inputParams->JobSize + i].Value = CalcArithmeticProgressionForItemI(inputParams->A, i + startIndex, inputParams->d);
			subSeqArray[threadNumber*inputParams->JobSize + i].ThreadNumber = 11;//TODO
			subSeqArray[threadNumber*inputParams->JobSize + i].Time = 11;//TODO
		}
		if (type == GEOMETRIC)
		{
			subSeqArray[threadNumber*inputParams->JobSize + i].Value = CalcGeometricProgressionForItemI(inputParams->A, i + startIndex, inputParams->q);
			subSeqArray[threadNumber*inputParams->JobSize + i].ThreadNumber = 11;//TODO
			subSeqArray[threadNumber*inputParams->JobSize + i].Time = 11;//TODO
		}
		if (type == DIFFERENCEPROG)
		{
			subSeqArray[threadNumber*inputParams->JobSize + i].Value = CalaDifferenceProgressionForItemI(inputParams->A, i + startIndex, inputParams->d, inputParams->q);
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