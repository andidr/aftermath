#include <stdio.h>

int main(int argc, char** argv)
{
	#pragma omp parallel for schedule(static, 5)
	for(int i = 0; i < 10; i++)
		printf("Iteration %d\n", i);

	return 0;
}
