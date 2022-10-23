#include <stdio.h>
#include <sys/time.h>
#include "../../libndl/include/NDL.h"



int main()
{
	NDL_Init(0);
	unsigned long pre = NDL_GetTicks();
	unsigned long now = pre;
	//printf("fuck!!!\n");
	//printf("%lu\n", now);
	int j = 0;
	while (1) {
		j++;
		if (j == 10000) {
			j = 0;
			now = NDL_GetTicks();
			//printf("%lu\n", timeVal.tv_usec);
			if (now - pre >= 500) {
				pre = now;
				printf("0.5s passed!\n");
			}
		}
	}
	return 0;
}