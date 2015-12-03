#include <stdio.h>
#include "windows.h"
#include <math.h>

typedef char* string; 
typedef struct InputParams
{
	int num_of_workers;
	int N;
	int job_size;
	int sub_seq_length;
	string failure_period;
	int A1;
	int d;
	int q;

}
InputParams;

int CalcArithmeticProgressionForItemI(int a1,int n, int d)
{
	return a1+(n-1)*d;
}
double CalcGeometricProgressionForItemI(int a1,int n,int q)
{
	return a1*pow((double) q,(double) n-1);
}
int main(int argc, char* argv[])
{
	if (argc != 9)
		return 1;




	printf("Hello World");
	while (1);
}