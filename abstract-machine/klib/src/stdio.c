#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

void deal_with_args(char *out, const char *fmt, int *len, va_list *ap, bool *isError)
{
  int d = 0;
  long long ld = 0, lf = 1;
  char *s = NULL;
  switch(*fmt) {
    case 's' :
      s = va_arg(*ap, char *);
      memcpy(out + *len, s, strlen(s));
      *len += strlen(s);
      break;
    case 'd' :
      d = va_arg(*ap, int);
      ld = d;
      lf = 1;
      if (ld < 0) {
        ld *= -1;
        *(out + *len) = '-';
        *len += 1;
      }
      while (lf * 10 <= ld) lf *= 10;
      while (lf) {
        *(out + *len) = '0' + ld / lf;
        *len += 1;
        ld %= lf;
        lf /= 10;
      }
      break;
    default : *isError = 1; return ;
  }
  assert(*len);
}

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
  va_start(ap, fmt);
  while (*fmt != '\0') {
    if (*fmt == '%') {
      fmt++;
      bool isError = 0;
      deal_with_args(out, fmt, &len, &ap, &isError);
      if (isError) return -1;
      /*switch(*fmt) {
        case 's' :
          s = va_arg(ap, char *);
          memcpy(out + len, s, strlen(s));
          len += strlen(s);
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
        default : return -1;
      }*/
    }
    else {
      *(out + len) = *fmt;
      len++;
    }
    fmt++;
  }
  va_end(ap);
  *(out + len) = '\0';
  return strlen(out);
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
