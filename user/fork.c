/**
 * @file fork.c
 * @author mmalensek
 *
 * Demonstrates the fork() and exec() system calls, parent and child processes.
 */

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  int my_pid = getpid();
  printf("Starting up, my PID = %d\n", my_pid);

  int child = fork();
  if (child == -1) {
    /* Something went wrong */
    printf("fork failed!\n");
  } else if (child == 0) {
    /* If fork() returns 0, this process is the child */
    int my_pid = getpid();
    sleep(2); // what happens if we remove this?
    printf("Hello from the child! PID = %d.\n", my_pid);

    char *cmd[] = { "ls", "/", 0 };
    exec(cmd[0], cmd);
    printf("Hi world!\n"); /* Will this print? */
    sleep(5);
  } else {
    /* If fork() returns a nonzero number, this process is the parent,
     * AKA the original process that called fork().  */
    int my_pid = getpid();
    printf("Hello from the parent! PID = %d\n", my_pid);

    printf("PID %d waiting for its child (%d)!\n", my_pid, child);
    int status;
    wait(&status);
    printf("Child exited. Return = %d. Parent exiting.\n", status);
  }

  return 0;
}
