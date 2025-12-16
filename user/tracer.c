#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
	int p = fork();

	if (p == 0) {
		stracing();
		exec(argv[1], argv + 1);
		printf("Tracer: exec %s failed\n", argv[1]);
  	 	exit(1);
    
  } else if(p > 0) {
	    // Wait for the child process to complete
	    int status;
	    wait(&status);
	    
  } else {
	    // Fork failed
	    printf("Tracer: fork failed\n");
	    exit(1);
  }
  // Parent exits normally
  exit(0);
}
