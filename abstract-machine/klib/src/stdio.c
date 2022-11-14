#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

char chs[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

void deal_with_args(char *out, const char *fmt, int *len, va_list *ap, bool *isError)
{
  int d = 0, numLen = 0;
  unsigned ud = 0;
  long long ld = 0, lf = 1;
  unsigned long long lud = 0, luf = 1;
  char *s = NULL;
  bool zeroCplt = 0;
  int zeroCpltNum = 0;
  bool isLongLong = 0;
  bool shouldPlus = 0;
  char zeroChar = 0;
  while (*fmt != '\0') {
    if (*fmt == '%') {
      fmt++;
      if (*fmt == '0') {
		zeroChar = '0';
        zeroCplt = 1;
        zeroCpltNum = 0;
        fmt++;
        while (*fmt >= '0' && *fmt <= '9') {
          zeroCpltNum = zeroCpltNum * 10 + *fmt - '0';
          fmt++;
        }
      }
	  else if (*fmt >= '0' && *fmt <= '9') {
		zeroChar = ' ';
		zeroCplt = 1;
        zeroCpltNum = 0;
        fmt++;
        while (*fmt >= '0' && *fmt <= '9') {
          zeroCpltNum = zeroCpltNum * 10 + *fmt - '0';
          fmt++;
        }
	  }
      switch(*fmt) {
		case 'l' :
		  isLongLong = 1;
		  shouldPlus = 1;
		  if (*(fmt + 1) == 'u') goto label_u;
		  else if (*(fmt + 1) == 'd') goto label_d;
		  else {
			shouldPlus = 0;
			goto label_d;
		  }
		  break;

        case 's' :
          s = va_arg(*ap, char *);
          memcpy(out + *len, s, strlen(s));
          *len += strlen(s);
          break;

        case 'd' : label_d :
          if (!isLongLong) d = va_arg(*ap, int), ld = d;
		  else isLongLong = 0, ld = va_arg(*ap, long long);
          lf = 1; numLen = 1;
          if (ld < 0) {
            ld *= -1;
            *(out + *len) = '-';
            *len += 1;
          }
          while (lf * 10 <= ld && lf * 10 > lf) lf *= 10, numLen++;
          if (d < 0) numLen++;
          if (zeroCplt) {
            zeroCplt = 0;
            for (int i = 1; i <= zeroCpltNum - numLen; i++) {
              *(out + *len) = zeroChar;
              *len += 1;
            }
          }
          while (lf) {
            *(out + *len) = chs[ld / lf];
            *len += 1;
            ld %= lf;
            lf /= 10;
          }
		  if (shouldPlus) shouldPlus = 1, fmt++;
          break;

        case 'u' : label_u :
		  if (!isLongLong) ud = va_arg(*ap, unsigned), lud = ud;
		  else isLongLong = 0, lud = va_arg(*ap, unsigned long long);
          luf = 1; numLen = 1;
          while (luf * 10 <= lud && luf * 10 > luf) luf *= 10, numLen++;
          if (zeroCplt) {
            zeroCplt = 0;
            for (int i = 1; i <= zeroCpltNum - numLen; i++) {
              *(out + *len) = zeroChar;
              *len += 1;
            }
          }
          while (luf) {
            *(out + *len) = chs[lud / luf];
            *len += 1;
            lud %= luf;
            luf /= 10;
          }
		  if (shouldPlus) shouldPlus = 1, fmt++;
          break;

		case 'x' :
		  ud = va_arg(*ap, unsigned);
          lud = ud;
          luf = 1; numLen = 1;
          while (luf * 16 <= lud && luf * 16 > luf) luf *= 16, numLen++;
          if (zeroCplt) {
            zeroCplt = 0;
            for (int i = 1; i <= zeroCpltNum - numLen; i++) {
              *(out + *len) = zeroChar;
              *len += 1;
            }
          }
          while (luf) {
            *(out + *len) = chs[lud / luf];
            *len += 1;
            lud %= luf;
            luf /= 16;
          }
          break;

		case 'p' :
		  /*if (!zeroCplt) {
			zeroCplt = 1, zeroCpltNum = 16;
			zeroChar = ' ';
		  }*/
		  lud = (unsigned)va_arg(*ap, const void *);
          luf = 1; numLen = 1;
          while (luf * 16 <= lud && luf * 16 > luf) luf *= 16, numLen++;
          if (zeroCplt) {
            zeroCplt = 0;
            for (int i = 1; i <= zeroCpltNum - numLen; i++) {
              *(out + *len) = zeroChar;
              *len += 1;
            }
          }
          while (luf) {
            *(out + *len) = chs[lud / luf];
            *len += 1;
            lud %= luf;
            luf /= 16;
          }
          break;

        default : *isError = 1; putch(*fmt); panic("-Unrealized sprintf char"); return ;
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
