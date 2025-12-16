#include "kernel/fcntl.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define BUFFER_SIZE 512

char read_buffer[BUFFER_SIZE];
int buffer_pos = 0;
int buffer_len = 0;

int
fgets2(char *buf, int max, int fd)
{
  int i;
  char c;

  for (i = 0; i + 1 < max; ) {
    // Refill buffer if empty
    if (buffer_pos >= buffer_len) {
      buffer_len = read(fd, read_buffer, BUFFER_SIZE);
      buffer_pos = 0;
      if (buffer_len <= 0)
        break;
    }
    
    // Get next character from buffer
    c = read_buffer[buffer_pos++];
    buf[i++] = c;
    if(c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
  return i;
}

int
main(int argc, char *argv[])
{
  if (argc <= 1) {
    fprintf(2, "Usage: %s filename\n", argv[0]);
    return 1;
  }

  int fd = open(argv[1], O_RDONLY);
  char buf[128];
  int line_count = 0;
  
  // Reset buffer state
  buffer_pos = 0;
  buffer_len = 0;
  
  while (fgets2(buf, 128, fd) > 0 ) {
    printf("Line %d: %s", line_count++, buf);
  }
  close(fd);

  return 0;
}
