#define _CRT_SECURE_NO_WARNINGS /* to suppress Visual Studio 2010 compiler warning */
#include <stdio.h>
#include <stdlib.h>
#include "windows.h"
#include <math.h>
#include <string.h>


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

int CalcArithmeticProgressionForItemI(int a1,int n, int d)
{
	return a1+(n-1)*d;
}

double CalcGeometricProgressionForItemI(int a1,int n,int q)
{
	return a1*pow((double) q,(double) n-1);
}


void SetArgumentsInStructre(char** argv, InputParams* inputParams)
{
	inputParams->NumOfWorkers = atoi(argv[1]);
	inputParams->N = atoi(argv[2]);
	inputParams->JobSize = atoi(argv[3]);
	inputParams->SubSeqLength = atoi(argv[4]);
	inputParams->FailurePeriod = argv[5];
	inputParams->A1 = atoi(argv[6]);
	inputParams->d = atoi(argv[7]);
	inputParams->q = atoi(argv[8]);
}

int GetNumberOfJobs(int SubSeqLength, int JobSize)
{
	return SubSeqLength / JobSize;
}
void CreateFiles(FILE *files[])
{
	files[0] = fopen("arithmetic.txt","w");
	files[1] = fopen("geometric.txt","w");
	files[2] = fopen("differential.txt", "w");
	if (files[0] == NULL || files[1] == NULL || files[2] == NULL)
	{
		printf("Error - CreateFiles Failed\n");
		exit(1);
	}
}
void CalculateArithmetic(int* subSeqArray, InputParams *inputParams,FILE* file)
{
	printf("CalculateArithmetic\n");
}

void CalculateGeometric(int* subSeqArray, InputParams *inputParams, FILE* file)
{
	printf("CalculateGeometric\n");

}
void DoCalculations(int* subSeqArray, InputParams *inputParams,FILE* files[])
{
	CalculateArithmetic(subSeqArray, inputParams, files[0]);
	CalculateGeometric(subSeqArray, inputParams, files[1]);
	//Do Differential
}
int main(int argc, char* argv[])
{
	InputParams *inputParams = malloc(sizeof(inputParams));
	int* subSeqArray;
	FILE *files[3];

	if (inputParams == NULL)
		return 1;

	if (argc != 9)
		return 1;
	
	SetArgumentsInStructre(argv, inputParams);
	subSeqArray = (int*) malloc(7*sizeof(int));
	if (subSeqArray == NULL)
		exit(1);
	
	CreateFiles(files);

	DoCalculations(subSeqArray, inputParams,files);



	printf("Hello World");
	while (1);
}