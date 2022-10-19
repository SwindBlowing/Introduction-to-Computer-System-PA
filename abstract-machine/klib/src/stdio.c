#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

const int PUT = 1, NOTPUT = 0;

void deal_with_args(char *out, const char *fmt, int *len, va_list *ap, bool *isError)
{
  int d = 0, numLen = 0;
  unsigned ud = 0;
  long long ld = 0, lf = 1;
  unsigned long long lud = 0, luf = 1;
  char *s = NULL;
  bool zeroCplt = 0, zeroCpltNum = 0;
  while (*fmt != '\0') {
    if (*fmt == '%') {
      fmt++;
      if (*fmt == '0') {
        zeroCplt = 1;
        zeroCpltNum = 0;
        fmt++;
        while (*fmt >= '0' && *fmt <= '9') {
          zeroCpltNum = zeroCpltNum * 10 + *fmt - '0';
          fmt++;
        }
      }
      switch(*fmt) {
        case 's' :
          s = va_arg(*ap, char *);
          memcpy(out + *len, s, strlen(s));
          *len += strlen(s);
          break;
        case 'd' :
          d = va_arg(*ap, int);
          ld = d;
          lf = 1; numLen = 0;
          if (ld < 0) {
            ld *= -1;
            *(out + *len) = '-';
            *len += 1;
          }
          while (lf * 10 <= ld) lf *= 10, numLen++;
          if (d < 0) numLen++;
          if (zeroCplt) {
            zeroCplt = 0;
            for (int i = 1; i <= zeroCpltNum - numLen; i++) {
              *(out + *len) = '0';
              *len += 1;
            }
          }
          while (lf) {
            *(out + *len) = '0' + ld / lf;
            *len += 1;
            ld %= lf;
            lf /= 10;
          }
          break;
        case 'u' :
          ud = va_arg(*ap, unsigned);
          lud = ud;
          luf = 1; numLen = 0;
          while (luf * 10 <= lud) luf *= 10, numLen++;
          if (zeroCplt) {
            zeroCplt = 0;
            for (int i = 1; i <= zeroCpltNum - numLen; i++) {
              *(out + *len) = '0';
              *len += 1;
            }
          }
          while (luf) {
            *(out + *len) = '0' + lud / luf;
            *len += 1;
            lud %= luf;
            luf /= 10;
          }
          break;
        default : *isError = 1; panic("Unrealized sprintf char"); return ;
      }
    }
    /*else if (*fmt == '/' && *(fmt + 1) == 'n') {
      fmt++;
      *(out + *len) = '\n';
      len += 1;
    }*/
    else {
      *(out + *len) = *fmt;
      *len += 1;
    }
    fmt++;
  }
  return ;
}

int printf(const char *fmt, ...) {
  //panic("Not implemented");
  va_list ap;
  int len = 0;
  va_start(ap, fmt);
  bool isError = 0;
  char out[14500] = {'\0'};
  deal_with_args(out, fmt, &len, &ap, &isError);
  va_end(ap);
  putstr(out);
  if (isError) return -1;
  return len;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  //panic("Not implemented");
  va_list ap;
  int len = 0;
  va_start(ap, fmt);
  bool isError = 0;
  deal_with_args(out, fmt, &len, &ap, &isError);
  va_end(ap);
  *(out + len) = '\0';
  if (isError) return -1;
  return strlen(out);
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
