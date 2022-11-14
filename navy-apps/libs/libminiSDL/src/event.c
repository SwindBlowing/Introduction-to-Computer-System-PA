#include <NDL.h>
#include <SDL.h>
#include <stdio.h>
#include <string.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  return 0;
}

int SDL_WaitEvent(SDL_Event *event) {
  char buf[100] = {'\0'};
  char tmpBuf[100] = {'\0'};
  while (1) {                             //can be better!!!!!!!!!!
	if (NDL_PollEvent(buf, 50)) {
		if (*(buf + 1) == 'd') event->type = SDL_KEYDOWN, event->key.type = SDL_KEYDOWN;
		else if (*(buf + 1) == 'u') event->type = SDL_KEYUP, event->key.type = SDL_KEYUP;
		strncpy(tmpBuf, buf + 3, 50);
		for (int i = 0; i < sizeof(keyname) / sizeof(char *); i++)
			if (strcmp(tmpBuf, keyname[i]) == 0) {
				event->key.keysym.sym = i;
				return 1;
			}
		return 0;
  	}
  }
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  return NULL;
}
