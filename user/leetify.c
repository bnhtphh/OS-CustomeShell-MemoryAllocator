/**
 * @file leetify.c
 *
 * Scaffolding to create an amazing l337speak generator using only external
 * commands combined with pipelines.
 */

#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

/**
 * Represents a command in a pipeline.
 * Note that EITHER stdout_pipe can be true, or a stdout_file can be set.
 *
 * - tokens Array of strings that describe the command to run
 * - stdout_pipe set to true if this command's output should be written to pipe
 * - stdout_file set to a file name if this command's output should be written
 *     to a file
 */
struct command {
  char **tokens;
  int stdout_pipe;
  char *stdout_file;
};

void
execute_pipeline(struct command *cmd)
{
  /**
   * TODO: design an algorithm that sets up a pipeline piece by piece.
   * Solutions will probably either iterate over the pieces of the pipeline or
   * work recursively, creating a new process for each piece with fork() and
   * executing the corresponding command with exec().
   *
   * While we aren't at the last command, set up a pipe for the current
   * command's output to go into before forking. For example, let's say our
   * command is `cat file.txt`. We will create a pipe and send the stdout of the
   * command to the pipe. Before running the next command, we'll make stdin of
   * the parent come from the pipe (thus the CLONED next process will also be
   * receiving its stdin on the same pipe!), and execute_pipeline will run
   * whatever command comes next (for instance, `lowercase`).
   *
   * Here's some pseudocode to help:
   *
   * create a new pipe
   * fork a new process
   * in the child:
   *   send stdout to pipe
   *   close other end of pipe
   *   exec command
   * in the parent:
   *   receive stdin from pipe
   *   close other end of pipe
   *   move on to the next command in the pipeline
   *
   * The special case is when there are no more commands left. Simply exec the
   * final command (no need to create another pipe). The extra process we
   * created in main() to call execute_pipeline will be replaced by this last
   * call to exec.
   */

   /* Base case check first */
  if (!cmd->stdout_pipe) {
   /* Last command - just exec it */
    if (cmd->stdout_file != 0) {
      close(1);
      open(cmd->stdout_file, O_CREATE | O_WRONLY);
     }
    exec(cmd->tokens[0], cmd->tokens);
    fprintf(2, "exec %s failed\n", cmd->tokens[0]);
    exit(1); 
    }

  int newfd[2];
  pipe(newfd);

  int pid = fork();
  
  if (pid == -1) {
    fprintf(2, "Fork failed");
    exit(1);
    
  } else if (pid == 0) {
    close(newfd[0]);
    close(1);
    dup(newfd[1]);
    close(newfd[1]);

    exec(cmd->tokens[0], cmd->tokens);
    fprintf(2, "exec failed\n");
    
  } else {
    close(newfd[1]);
    close(0);
    dup(newfd[0]);
    close(newfd[0]);
    
    //wait(0);
    execute_pipeline(cmd + 1);
    exit(0);

    
  }
}

int
main(int argc, char *argv[])
{
  char *input_file = 0;
  char *output_file = 0;

  if (argc < 2 || argc > 3) {
    printf("Usage: %s file-to-leetify [output-file]\n", argv[0]);
    return 1;
  }

  input_file = argv[1];

  if (argc == 3) {
    output_file = argv[2];
  }

  printf("Input file: %s\n", input_file);
  if (output_file) {
    printf("Writing to output file: %s\n", output_file);
  }

  /**
   * TODO: Next steps:
   * - Read and understand the setup code above
   * - Finish preparing the commands that will be run
   * - Set up the array of command structs
   * - Implement execute_pipeline
   */

  /* TODO: These commands aren't quite finished; check the lab spec. */
  char *command1[] = { "cat", input_file, (char *) 0 };
  char *command2[] = { "tolower", (char *) 0 };
  char *command3[] = { "fnr", "a", "4", "e", "3", "i", "!", (char *) 0 }; 
  char *command4[] = { "fnr", "l", "1", "o", "0", "s", "5", (char *) 0 };

  /* TODO: Finish these. Only some struct fields are populated. */
  struct command cmds[4] = { 0 };
  cmds[0].tokens = command1;  /* What this command runs */
  cmds[0].stdout_pipe = 1;
  cmds[0].stdout_file = 0; /* This command is not writing to a file. */

  cmds[1].tokens = command2;
  cmds[1].stdout_pipe = 1; /* This command's output goes to a pipe. */

  cmds[2].tokens = command3;
  cmds[2].stdout_pipe = 1;

  cmds[3].tokens = command4; /* Last command so set stdout_pipe = false */
  cmds[3].stdout_pipe = 0;
  cmds[3].stdout_file = output_file;

  int pid = fork();
  if (pid == -1) {
    fprintf(2, "Fork failed\n");
    return 1;
  } else if (pid == 0) {
    execute_pipeline(cmds);
  } else {
    int status;
    wait(&status);
    printf("Child exited with status %d\n", status);
  }

  return 0;
}
