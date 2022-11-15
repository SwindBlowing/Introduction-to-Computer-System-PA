#include <NDL.h>
#include <SDL.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

uint8_t keyState[83] = {0};

int SDL_PushEvent(SDL_Event *ev) {
  printf("begin1\n");
  assert(ev->type == SDL_KEYDOWN || ev->type == SDL_KEYUP);
  if (ev->type == SDL_KEYDOWN)
	keyState[ev->key.keysym.sym] = 1;
  else keyState[ev->key.keysym.sym] = 0;
  printf("end1\n");
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
	printf("begin2\n");
  char buf[100] = {'\0'};
  char tmpBuf[100] = {'\0'};
  if (NDL_PollEvent(buf, 50)) {
	if (*(buf + 1) == 'd') ev->type = SDL_KEYDOWN, ev->key.type = SDL_KEYDOWN;
	else if (*(buf + 1) == 'u') ev->type = SDL_KEYUP, ev->key.type = SDL_KEYUP;
	strncpy(tmpBuf, buf + 3, 50);
	tmpBuf[strlen(tmpBuf) - 1] = '\0';
	for (int i = 0; i < sizeof(keyname) / sizeof(char *); i++)
		if (strcmp(tmpBuf, keyname[i]) == 0) {
			//printf("%d\n", i);
			ev->key.keysym.sym = i;
			SDL_PushEvent(ev);
			printf("end2\n");
			return 1;
		}
  }
  return 0;
}

int SDL_WaitEvent(SDL_Event *event) {
	printf("begin3\n");
  char buf[100] = {'\0'};
  char tmpBuf[100] = {'\0'};
  while (1) {                             //can be better!!!!!!!!!!
  	//printf("still here\n");
	if (NDL_PollEvent(buf, 50)) {
		if (*(buf + 1) == 'd') event->type = SDL_KEYDOWN, event->key.type = SDL_KEYDOWN;
		else if (*(buf + 1) == 'u') event->type = SDL_KEYUP, event->key.type = SDL_KEYUP;
		strncpy(tmpBuf, buf + 3, 50);
		tmpBuf[strlen(tmpBuf) - 1] = '\0';
		for (int i = 0; i < sizeof(keyname) / sizeof(char *); i++)
			if (strcmp(tmpBuf, keyname[i]) == 0) {
				//printf("%d\n", i);
				event->key.keysym.sym = i;
				SDL_PushEvent(event);
				printf("end3\n");
				return 1;
			}
		return 0;
  	}
  }
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
	assert(0);
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
	printf("begin4\n");
  *numkeys = 83;
  printf("end4\n");
  return keyState;
}
