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
  //panic("Not implemented");
  size_t len = 0;
  while (*(s1 + len) != '\0' && *(s2 + len) != '\0' && *(s1 + len) == *(s2 + len))
    len++;
  if (*(s1 + len) == '\0' && *(s2 + len) == '\0') return 0;
  else if (*(s1 + len) == '\0') return -1;
  else return 1;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  //panic("Not implemented");
  size_t len = 0;
  while (len < n && *(s1 + len) != '\0' && *(s2 + len) != '\0' && *(s1 + len) == *(s2 + len))
    len++;
  if (len == n) return 0;
  else if (*(s1 + len) == '\0' && *(s2 + len) == '\0') return 0;
  else if (*(s1 + len) == '\0') return -1;
  else return 1;
}

void *memset(void *s, int c, size_t n) {
  //panic("Not implemented");
  size_t len = 0;
  while (len < n) {
    *((char *)s + len) = c;
    len++;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  //panic("Not implemented");
  size_t len = 0;
  if (dst < src) {
    while (len < n) {
      *((char *)dst + len) = *((char *)src + len);
      len++;
    }
  }
  else {
    len = n - 1;
    while (len >= 0) {
      *((char *)dst + len) = *((char *)src + len);
      len--;
    }
  }
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  //panic("Not implemented");
  size_t len = 0;
  while (len < n) {
    *((char *)out + len) = *((char *)in + len);
    len++;
  }
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  //panic("Not implemented");
  size_t len = 0;
  while (len < n && *((unsigned char *)s1 + len) != '\0' &&
     *((unsigned char *)s2 + len) != '\0' && 
     *((unsigned char *)s1 + len) == *((unsigned char *)s2 + len))
    len++;
  if (len == n) return 0;
  else if (*((unsigned char *)s1 + len) == '\0' && 
    *((unsigned char *)s2 + len) == '\0') 
    return 0;
  else if (*((unsigned char *)s1 + len) == '\0') return -1;
  else return 1;
}

#endif
