#define SDL_malloc  malloc
#define SDL_free    free
#define SDL_realloc realloc

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"

SDL_Surface* IMG_Load_RW(SDL_RWops *src, int freesrc) {
  assert(src->type == RW_TYPE_MEM);
  assert(freesrc == 0);
  assert(0);
  return NULL;
}

SDL_Surface* IMG_Load(const char *filename) {
  FILE *fp = fopen(filename, "r+");
  assert(fp);

  fseek(fp, 0, SEEK_END);
  uint32_t nowSize = ftell(fp);

  uint8_t *buf = malloc(nowSize);
  fseek(fp, 0, SEEK_SET);
  fread(buf, nowSize, 1, fp);

  SDL_Surface* val = STBIMG_LoadFromMemory(buf, nowSize);

  fclose(fp);
  free(buf);

  return val;
}

int IMG_isPNG(SDL_RWops *src) {
	assert(0);
  return 0;
}

SDL_Surface* IMG_LoadJPG_RW(SDL_RWops *src) {
	assert(0);
  return IMG_Load_RW(src, 0);
}

char *IMG_GetError() {
  return "Navy does not support IMG_GetError()";
}
