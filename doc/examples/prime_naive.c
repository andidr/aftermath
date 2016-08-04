#include <stdio.h>
#include <math.h>

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

	#pragma omp parallel
	{
		#pragma omp for schedule(static) reduction(+:n)
		for(int i = 3; i < 1000000; i += 2)
			n += isprime_naive(i);
	}

	printf("There are %d prime numbers in the interval\n", n);

	return 0;
}
