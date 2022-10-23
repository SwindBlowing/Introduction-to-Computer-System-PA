#include <stdio.h>
#include <sys/time.h>
#include <assert.h>

struct timeval timeVal;

int main()
{
	//assert(0);
	gettimeofday(&timeVal, NULL);
	assert(0);
	unsigned long now = timeVal.tv_usec;
	printf("%u\n", now);
	int j = 0;
	while (1) {
		j++;
		if (j == 10000) {
			j = 0;
			gettimeofday(&timeVal, NULL);
			if (timeVal.tv_usec - now >= 500000) {
				now = timeVal.tv_usec;
				printf("0.5s passed!\n");
			}
		}
	}
	return 0;
}