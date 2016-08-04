#include <stdio.h>
#include <math.h>

static inline int imax(int a, int b)
{
	return (a > b) ? a : b;
}

int isprime_naive(int n)
{
	if(n % 2 == 0 && n != 2)
		return 0;

	for(int j = 3; j <= sqrt(n); j += 2)
		if(n % j == 0)
			return 0;

	return 1;
}

int main(int argc, char** argv)
{
	int n = 1;
	int slices = 100;
	int nloc[slices];
	int max = 1000000;
	int slice_sz = max / slices;

	#pragma omp parallel
	{
		#pragma omp for schedule(static)
		for(int i = 0; i < slices; i++) {
			nloc[i] = 0;

			#pragma omp task
			for(int j = imax(i*slice_sz, 3); j < (i+1)*slice_sz; j++)
				nloc[i] += isprime_naive(j);
		}

		#pragma omp taskwait

		#pragma omp for schedule(static) reduction(+:n)
		for(int i = 0; i < slices; i++)
			n += nloc[i];
	}

	printf("There are %d prime numbers in the interval\n", n);

	return 0;
}
