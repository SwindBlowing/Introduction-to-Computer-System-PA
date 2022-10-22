#include <stdio.h>
#include <sys/time.h>

struct timeval tv;

int main()
{
	gettimeofday(&tv, NULL);
	unsigned long now = tv.tv_usec;
	int j = 0;
	while (1) {
		j++;
		if (j == 10000) {
			j = 0;
			gettimeofday(&tv, NULL);
			if (tv.tv_usec - now >= 500000) {
				now = tv.tv_usec;
				printf("0.5s passed!\n");
			}
		}
	}
	return 0;
}