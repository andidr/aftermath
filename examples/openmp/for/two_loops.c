#include <stdio.h>

int main(int argc, char** argv)
{
	#pragma omp parallel
	{
		#pragma omp for
		for(int i = 0; i < 10; i++)
			printf("Loop 1, iteration %d\n", i);

		#pragma omp for
		for(int i = 0; i < 10; i++)
			printf("Loop 2, iteration %d\n", i);
	}

	return 0;
}
