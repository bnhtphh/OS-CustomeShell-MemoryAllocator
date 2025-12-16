#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc < 3) {
    fprintf(2, "Usage: nice <level> <command> [args...]\n");
    exit(1);
  }

  int nice_lv = atoi(argv[1]);
  
  // Set the nice level for this process
  nice(nice_lv);
  
  // Execute the command with remaining arguments
  exec(argv[2], &argv[2]);
  
  // If exec returns, it failed
  fprintf(2, "nice: exec %s failed\n", argv[2]);
  exit(1);
}
