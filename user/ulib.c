#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

//
// wrapper so that it's OK if main() does not call exit().
//
void
start() // start function to call main and exit
{
  extern int main();
  main();
  exit(0); 
}

char*
strcpy(char *s, const char *t) // copy string t to s
{
  char *os; // original s

  os = s; // save original s
  while((*s++ = *t++) != 0) // copy t to s, including null terminator
    ;
  return os; // return original s
}

int
strcmp(const char *p, const char *q) // compare strings p and q
{
  while(*p && *p == *q) // while characters are equal and not null terminator
    p++, q++; // move to next characters
  return (uchar)*p - (uchar)*q; // return difference of first non-matching characters (or 0 if all characters match)
}

uint
strlen(const char *s) // calculate length of string s
{
  int n;

  for(n = 0; s[n]; n++)
    ;
  return n;
}

void*
memset(void *dst, int c, uint n) // set n bytes of memory at dst to character c
{
  char *cdst = (char *) dst;
  int i;
  for(i = 0; i < n; i++){
    cdst[i] = c;
  }
  return dst;
}

char*
strchr(const char *s, char c) // find first occurrence of character c in string s
{
  for(; *s; s++) // iterate through the string until null terminator
    if(*s == c)
      return (char*)s; // if character matches, return pointer to it
  return 0;  // if character not found, return null pointer
}

char*
gets(char *buf, int max) // read a line from standard input into buf, up to max characters
{
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){ // leave space for null terminator
    cc = read(0, &c, 1); // read one character from standard input
    if(cc < 1) 
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r') // if newline or carriage return is encountered, stop reading
      break;
  }
  buf[i] = '\0'; // null-terminate the string
  return buf;
}

int
stat(const char *n, struct stat *st) // get file status for file named n and store it in st
{
  int fd;
  int r;

  fd = open(n, O_RDONLY);
  if(fd < 0)
    return -1;
  r = fstat(fd, st);
  close(fd);
  return r;
}

int
atoi(const char *s) // convert string s to an integer
{
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
    n = n*10 + *s++ - '0'; // convert each character to its integer value and accumulate
  return n;
}

void*
memmove(void *vdst, const void *vsrc, int n) // move n bytes from vsrc to vdst, handling overlapping regions
{
  char *dst;
  const char *src;

  dst = vdst;
  src = vsrc;
  if (src > dst) {
    while(n-- > 0)
      *dst++ = *src++;
  } else {
    dst += n;
    src += n;
    while(n-- > 0)
      *--dst = *--src;
  }
  return vdst;
}

int
memcmp(const void *s1, const void *s2, uint n) // compare n bytes of memory at s1 and s2
{
  const char *p1 = s1, *p2 = s2;
  while (n-- > 0) {
    if (*p1 != *p2) {
      return *p1 - *p2;
    }
    p1++;
    p2++;
  }
  return 0;
}

void *
memcpy(void *dst, const void *src, uint n)
{
  return memmove(dst, src, n);
}
