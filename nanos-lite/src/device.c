#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  for (int i = 0; i < len; i++)
  	putch(*(char *)(buf + i));
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  if (ev.keycode == AM_KEY_NONE) return 0;
  size_t nowLen = 0;
  if (nowLen == len) return nowLen;
  if (ev.keydown) {
	*((char *)buf + 0) = 'k'; nowLen++; if (nowLen == len) return nowLen;
	*((char *)buf + 1) = 'd'; nowLen++; if (nowLen == len) return nowLen;
	*((char *)buf + 2) = ' '; nowLen++; if (nowLen == len) return nowLen;
  }
  else {
	*((char *)buf + 0) = 'k'; nowLen++; if (nowLen == len) return nowLen;
	*((char *)buf + 1) = 'u'; nowLen++; if (nowLen == len) return nowLen;
	*((char *)buf + 2) = ' '; nowLen++; if (nowLen == len) return nowLen;
  }
  for (int i = 0; *(keyname[ev.keycode] + i) != '\0'; i++) {
	*((char *)buf + nowLen) = keyname[ev.keycode][i];
	nowLen++; if (nowLen == len) return nowLen;
  }
  *(char *)(buf + nowLen) = '\n'; nowLen++;
  *(char *)(buf + nowLen) = '\0';
  //printf("%d %s\n", sizeof(keyname[ev.keycode]), (char *)buf);
  return nowLen;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  int nowLen = 0;
  int w = io_read(AM_GPU_CONFIG).width;
  int h = io_read(AM_GPU_CONFIG).height;
  char temp1[99] = {'\0'}, temp2[99] = {'\0'};
  sprintf(temp1, "%d", w);
  char W[99] = "WIDTH:\0";
  for (int i = 0; i < 6; i++) *((char *)buf + i) = W[i];
  nowLen += 6;
  for (int i = 0; temp1[i] != '\0'; i++) {
	*((char *)buf + nowLen) = temp1[i];
	nowLen++;
  }
  *((char *)buf + nowLen) = '\n'; nowLen++;
  strcpy(W, "HEIGHT:\0");
  sprintf(temp2, "%d", h);
  for (int i = 0; i < 7; i++) *((char *)buf + nowLen) = W[i], nowLen++;
  for (int i = 0; temp2[i] != '\0'; i++) {
	*((char *)buf + nowLen) = temp2[i];
	nowLen++;
  }
  *((char *)buf + nowLen) = '\0';
  return nowLen;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  int sys_w = io_read(AM_GPU_CONFIG).width;
  //int sys_h = io_read(AM_GPU_CONFIG).height;
  offset /= 4;
  int h = offset / sys_w;
  int w = offset % sys_w;
  //printf("%d %d %d %d\n", sys_w, offset, w, h);
  io_write(AM_GPU_FBDRAW, w, h, (uint32_t *)buf, len / 4, 1, false);
  io_write(AM_GPU_FBDRAW, 0, 0, NULL, 0, 0, true);
  return len;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
