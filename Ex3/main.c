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
	int A;
	int d;
	int q;

}InputParams;

typedef struct SubSeqArray
{
	int Value;
	int ThreadNumber;
	int Time;
}SubSeqArray;

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

void SetArgumentsInStructre(char** argv, InputParams* inputParams)
{
	inputParams->NumOfWorkers = atoi(argv[1]);
	inputParams->N = atoi(argv[2]);
	inputParams->JobSize = atoi(argv[3]);
	inputParams->SubSeqLength = atoi(argv[4]);
	inputParams->FailurePeriod = (string) malloc (strlen(argv[5])*sizeof(string));
	if (inputParams->FailurePeriod == NULL)
		exit(1);
	inputParams->FailurePeriod = strcpy(inputParams->FailurePeriod, argv[5]);
	inputParams->A = atoi(argv[6]);
	inputParams->d = atoi(argv[7]);
	inputParams->q = atoi(argv[8]);
}

int GetNumberOfJobs(int SubSeqLength, int JobSize)
{
	return SubSeqLength / JobSize;
}
void CreateFiles(FILE **files)
{
	files[0] = fopen("arithmetic.txt","w+");
	files[1] = fopen("geometric.txt","w+");
	files[2] = fopen("differential.txt", "w+");
	if (files[0] == NULL || files[1] == NULL || files[2] == NULL)
	{
		printf("Error - CreateFiles Failed\n");
		exit(1);
	}
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

int CloseFiles(FILE **files)
{
	fclose(files[0]);
	fclose(files[1]);
	fclose(files[2]);

}
int main(int argc, char* argv[])
{
	InputParams *inputParams = (InputParams*) malloc(sizeof(inputParams));
	FILE **files = (FILE**) malloc(sizeof(FILE**) * 3);

	if (inputParams == NULL)
		return 1;

	if (argc != 9)
		return 1;

	SetArgumentsInStructre(argv, inputParams);
	
	CreateFiles(files);

	DoCalculations(inputParams,files);

	CloseFiles(files);



	printf("Hello World");
	while (1);
}