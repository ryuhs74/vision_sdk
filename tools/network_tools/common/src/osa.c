

#include <osa.h>
#include <sys/time.h>

Uint32 OSA_getCurTimeInMsec()
{
  struct timeval tv;

  if (gettimeofday(&tv, NULL) < 0)
    return 0;

  return tv.tv_sec * 1000 + tv.tv_usec/1000;
}

static char xtod(char c) {
  if (c>='0' && c<='9') return c-'0';
  if (c>='A' && c<='F') return c-'A'+10;
  if (c>='a' && c<='f') return c-'a'+10;
  return c=0;        // not Hex digit
}

static int HextoDec(char *hex, int l)
{
  if (*hex==0)
    return(l);

  return HextoDec(hex+1, l*16+xtod(*hex)); // hex+1?
}

int xstrtoi(char *hex)      // hex string to integer
{
  return HextoDec(hex,0);
}
