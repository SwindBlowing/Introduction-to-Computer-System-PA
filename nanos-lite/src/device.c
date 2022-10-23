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
  nowLen = 3;
  for (const char *p = keyname[ev.keycode]; *p; p++) {
	*((char *)buf + nowLen) = *p;
	nowLen++; if (nowLen == len) return nowLen;
  }
  return nowLen;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  return 0;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  return 0;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
