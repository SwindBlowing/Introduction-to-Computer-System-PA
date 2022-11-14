#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
static int sys_w = 0, sys_h = 0;
static uint32_t boot = 0; //usec
struct timeval tv;

uint32_t NDL_GetTicks() {
  gettimeofday(&tv, NULL);
  return (tv.tv_usec - boot) / 1000;
}

int NDL_PollEvent(char *buf, int len) {
  int nowLen = read(3, buf, len);
  for (int i = 0; i < nowLen; i++)
	if (*(buf + i) == '\n') {
		close(3);
		return 1;
	}
  close(3);
  return 0;
}

void NDL_OpenCanvas(int *w, int *h) {
  if (!(*w) && !(*h)) *w = sys_w, *h = sys_h;
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }

  gettimeofday(&tv, NULL);
  boot = tv.tv_usec;

  if (!sys_w) {
	char sys_size[64];
	read(5, sys_size, sizeof(sys_size) - 1);
	int p = 0;
	while (sys_size[p] < '0' || sys_size[p] > '9') p++;
	while (sys_size[p] >= '0' && sys_size[p] <= '9') 
		sys_w = sys_w * 10 + sys_size[p] - '0', p++;
	while (sys_size[p] < '0' || sys_size[p] > '9') p++;
	while (sys_size[p] >= '0' && sys_size[p] <= '9') 
		sys_h = sys_h * 10 + sys_size[p] - '0', p++;
  }
  printf("!!%d %d 2!!\n", sys_w, sys_h);
  return 0;
}

void NDL_Quit() {
  boot = 0;
  sys_w = sys_h = 0;
}
