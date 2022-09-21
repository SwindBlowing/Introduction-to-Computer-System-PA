#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  //panic("Not implemented");
  va_list ap;
  int len = 0;
  int d = 0;
  long long ld = 0, lf = 1;
  char *s = NULL;
  va_start(ap, fmt);
  while (*fmt != '\0') {
    if (*fmt == '%') {
      fmt++;
      switch(*fmt) {
        case 's' :
          s = va_arg(ap, char *);
          strcat(out, s);
          len = strlen(out);
          break;
        case 'd' :
          d = va_arg(ap, int);
          ld = d;
          lf = 1;
          if (ld < 0) {
            ld *= -1;
            *(out + len) = '-';
            len++;
          }
          while (lf * 10 <= ld) lf *= 10;
          while (lf) {
            *(out + len) = '0' + ld / lf;
            len++;
            ld %= lf;
            lf /= 10;
          }
          break;
        default : assert(0);
      }
    }
    else {
      *(out + len) = *fmt;
      len++;
    }
    fmt++;
  }
  va_end(ap);
  return strlen(out);
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
