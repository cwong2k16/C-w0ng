#include "wrappers.h"
#include "debug.h"

void*
Malloc(size_t size)
{
  void* ret;
  if ((ret = malloc(size)) == NULL) {
    perror("Out of Memory");
    exit(EXIT_FAILURE);
  }
  return ret;
}

void*
Calloc(size_t nmemb, size_t size)
{
  void* ret;
  if ((ret = calloc(nmemb, size)) == NULL) {
    perror("Out of Memory");
    exit(EXIT_FAILURE);
  }
  return ret;
}

int
Open(char const* pathname, int flags)
{
  int fd;
  if ((fd = open(pathname, flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
    perror("Could Not Open file");
    fprintf(stderr, "%s\n", pathname);
    exit(EXIT_FAILURE);
  }
  return fd;
}

ssize_t
read_to_bigendian(int fd, void* buf, size_t count)
{
  ssize_t bytes_read;

  bytes_read = read(fd, buf, count);
/*#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  reverse_bytes(buf, count);
#endif*/
  return bytes_read;
}

ssize_t
write_to_bigendian(int fd, void* buf, size_t count)
{
  ssize_t bytes_read;

/*#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  reverse_bytes(buf, count);
#endif*/
  bytes_read = write(fd, buf, count);
  return bytes_read;
}

void
reverse_bytes(void* bufp, size_t count)
{
  char* ptr = bufp;
  char temp;
  int i, j;

  for(i = 0, j = count-1; i < j; i++, j--){
    temp = ptr[i]&0xff;
    ptr[i] = ptr[j]&0xff;
    ptr[j] = temp;
  }
}

void
*memeset(void *s, int c, size_t n) {
  // register char* stackpointer asm("esp"); //initialize stackpointer pointer with the value of the actual stackpointer
  s = memset(s, c, n);
  return s;
};

void
*memecpy(void *dest, void const *src, size_t n) {
  // register char* stackpointer asm("esp"); //initialize stackpointer pointer with the value of the actual stackpointer
  dest = memcpy(dest, src, n);
  return dest;
};
