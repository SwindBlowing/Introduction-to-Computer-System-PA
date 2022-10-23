#include <stdio.h>
#include <sys/time.h>

struct timeval timeVal;

int main()
{
	gettimeofday(&timeVal, NULL);
	unsigned long now = timeVal.tv_usec;
	//printf("fuck!!!\n");
	//printf("%lu\n", now);
	int j = 0;
	while (1) {
		j++;
		if (j == 10000) {
			j = 0;
			gettimeofday(&timeVal, NULL);
			//printf("%lu\n", timeVal.tv_usec);
			if (timeVal.tv_usec - now >= 500000) {
				now = timeVal.tv_usec;
				printf("0.5s passed!\n");
			}
		}
	}
	return 0;
}