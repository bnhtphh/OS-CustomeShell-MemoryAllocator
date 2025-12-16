#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int
main(int argc, char *argv[])
{

  // Start time
  uint64 start = time();  
  
  int pid = fork();
  if (pid < 0) {
      fprintf(2, "Fork failed!\n");
      exit(1);
      
  } else if (pid == 0) {
      exec(argv[1], &argv[1]);
      fprintf(2, "Exec %s failed!\n", argv[1]);
      exit(1);
    
  } else  {
	  int status;
	  int count;
	  wait2(&status, &count);
	  
	  // End time
	  uint64 end = time();  
	  uint64 elapsed = end - start;
	  
	  //  nanoseconds to milliseconds
	  uint64 elapsed_ms = elapsed / 1000000;
	  
	  printf("------------------\n");
	  printf("Benchmark Complete\n");
	  printf("Time Elapsed: ");
	  printf("%d", (int)elapsed_ms);
	  printf(" ms\n");

	  printf("System Calls: ");
	  printf("%d", count);
	  printf("\n");
	  
	  exit(0);

  }
}
