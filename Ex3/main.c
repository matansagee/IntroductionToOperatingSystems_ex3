#define _CRT_SECURE_NO_WARNINGS /* to suppress Visual Studio 2010 compiler warning */
#include "Progression.h"
#include <stdio.h>
#include <stdlib.h>
#include "windows.h"
#include <math.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <tchar.h>

//---------------------------------------------------------------------------
// Main.c includes functions of handling input parameters and file creations
//---------------------------------------------------------------------------

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
	inputParams->A1 = atof(argv[6]);
	inputParams->d =  atof(argv[7]);
	inputParams->q =  atof(argv[8]);
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

int CloseFiles(FILE **files)
{
	fclose(files[0]);
	fclose(files[1]);
	fclose(files[2]);
	return 1;

}
int main(int argc, char* argv[])
{
	InputParams *inputParams = (InputParams*) malloc(sizeof(inputParams));

	FILE **files = (FILE**) malloc(sizeof(FILE**) * 3);

	if (inputParams == NULL || argc != 9)
		return 1;

	SetArgumentsInStructre(argv, inputParams);

	CreateFiles(files);

	__try
		{
			CreateThreads(inputParams,files);
	}
	__finally
		{
		CloseFiles(files);
	}
}