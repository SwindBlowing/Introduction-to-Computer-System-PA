#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  //panic("Not implemented");
  size_t len = 0;
  while (*(s + len) != '\0') 
    len++;
  return len;
}

char *strcpy(char *dst, const char *src) {
  //panic("Not implemented");
  size_t len = 0;
  while (*(src + len) != '\0') {
    *(dst + len) = *(src + len);
    len++;
  }
  *(dst + len) = *(src + len);
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  //panic("Not implemented");
  size_t len = 0;
  while (*(src + len) != '\0' && len < n) {
    *(dst + len) = *(src + len);
    len++;
  }
  if (len < n) {
    while (len < n) {
      *(dst + len) = '\0';
      len++;
    }
  }
  return dst;
}

char *strcat(char *dst, const char *src) {
  //panic("Not implemented");
  size_t len1 = 0, len2 = 0;
  while (*(dst + len1) != '\0') len1++;
  while (*(src + len2) != '\0') {
    *(dst + len1 + len2) = *(src + len2);
    len2++;
  }
  *(dst + len1 + len2) = '\0';
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  panic("Not implemented");
}

int strncmp(const char *s1, const char *s2, size_t n) {
  panic("Not implemented");
}

void *memset(void *s, int c, size_t n) {
  panic("Not implemented");
}

void *memmove(void *dst, const void *src, size_t n) {
  panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
  panic("Not implemented");
}

int memcmp(const void *s1, const void *s2, size_t n) {
  panic("Not implemented");
}

#endif
