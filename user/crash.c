#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

uint
strspn(const char *str, const char *chars)
{
  uint i, j;
  for (i = 0; str[i] != '\0'; i++) {
    for (j = 0; chars[j] != str[i]; j++) {
      if (chars[j] == '\0')
        return i;
    }
  }
  return i;
}

uint
strcspn(const char *str, const char *chars)
{
  const char *p, *sp;
  char c, sc;
  for (p = str;;) {
    c = *p++;
    sp = chars;
    do {
      if ((sc = *sp++) == c) {
        return (p - 1 - str);
      }
    } while (sc != 0);
  }
}

char
*next_token(char **str_ptr, const char *delim)
{
  if (*str_ptr == 0) {
    return 0;
  }

  uint tok_start = strspn(*str_ptr, delim);
  uint tok_end = strcspn(*str_ptr + tok_start, delim);

  /* Zero length token. */
  if (tok_end  == 0) {
    *str_ptr = 0;
    return 0;
  }

  char *current_ptr = *str_ptr + tok_start;

  /* Shift pointer forward (to the end of the current token) */
  *str_ptr += tok_start + tok_end;

  if (**str_ptr == '\0') {
    *str_ptr = 0;
  } else {
    **str_ptr = '\0';
    (*str_ptr)++;
  }

  return current_ptr;
}

// ============================================================================
struct command {
  char **tokens;
  int stdout_pipe;
  int stdout_append;
  char *stdout_file;
  char *stdin_file;
};

// ============================================================================
// History 

#define MAX_HISTORY 100

char *history[MAX_HISTORY];
int history_count = 0;

void
add_to_history(char *cmd, int cmd_num)
{
  int index = (cmd_num - 1) % MAX_HISTORY;
  
  if (history[index] != 0) {
    free(history[index]);
  }
  
  history[index] = malloc(strlen(cmd) + 1);
  strcpy(history[index], cmd);
}

char *
get_history_by_num(int num)
{
  if (num <= 0 || num > history_count) {
    return 0;
  }
  
  int index = (num - 1) % MAX_HISTORY;
  if (history[index] == 0) {
    return 0;
  }
  
  char *result = malloc(strlen(history[index]) + 1);
  strcpy(result, history[index]);
  return result;
}

char *
get_last_history()
{
  if (history_count == 0) {
    return 0;
  }
  
  return get_history_by_num(history_count);
}

char *
get_history_by_prefix(char *prefix)
{
  for (int i = history_count; i > 0; i--) {
    int index = (i - 1) % MAX_HISTORY;
    if (history[index] == 0) {
      continue;
    }
    
    int match = 1;
    for (int j = 0; prefix[j] != '\0'; j++) {
      if (history[index][j] != prefix[j]) {
        match = 0;
        break;
      }
    }
    
    if (match) {
      char *result = malloc(strlen(history[index]) + 1);
      strcpy(result, history[index]);
      return result;
    }
  }
  
  return 0;
}

void
print_history()
{
  int start = (history_count > MAX_HISTORY) ? history_count - MAX_HISTORY + 1 : 1;
  
  for (int i = start; i <= history_count; i++) {
    int index = (i - 1) % MAX_HISTORY;
    if (history[index] != 0) {
      printf("[%d] %s\n", i, history[index]);
    }
  }
}

// ============================================================================
// Built-int commands

int
builtin_exit()
{
  exit(0);
  return 0;
}

int
builtin_cd(char **args)
{
  if (args[1] == 0) {
    fprintf(2, "cd: missing argument\n");
    return 1;
  }
  
  if (chdir(args[1]) != 0) {
    fprintf(2, "chdir: no file: %s\n", args[1]);
    return 1;
  }
  
  return 0;
}

int
builtin_history()
{
  print_history();
  return 0;
}


int
execute_builtin(char **args)
{
  if (args[0] == 0) {
    return -1;
  }
  
  if (strcmp(args[0], "exit") == 0) {
    builtin_exit();
    return 0;
  }
  
  if (strcmp(args[0], "cd") == 0) {
    return builtin_cd(args);
  }
  
  if (strcmp(args[0], "history") == 0) {
    builtin_history();
    return 0;
  }
  
  return -1;
}


// ============================================================================

int
main(int argc, char *argv[])
{
  printf("Welcome to my shell🐚\n\n");
  int exit_status = 0;
  int cmd_num = 1;
  int input_from_file = 0;
  int input_fd = 0;

   if (argc > 1) {
    // open file
    if ((input_fd = open(argv[1], O_RDONLY)) < 0) {
      fprintf(2, "Cannot open %s\n", argv[1]);
      exit(1);
    }
    input_from_file  = 1;
  }
  
  while (1) {
    char cwd[128];
    strcpy(cwd, "/");

    //  prompt
    if (!input_from_file) {
      printf("[%d]-[%d]─[%s]$ ", exit_status, cmd_num, cwd);
    }

    // read command
    char buf[256];
    int len;

    if (input_from_file) {
      // read file
      int i = 0;
      while (i < 255) {
        int n = read(input_fd, &buf[i], 1);
        if (n <= 0) {
          if (i == 0) {
            goto cleanup;
          }
          break;
        }
        if (buf[i] == '\n') {
          break;
        }
        i++;
      }
      buf[i] = '\0';
      len = i;
    } else {
      // read  stdin
      gets(buf, 256);
      len = strlen(buf);
      if (len > 0 && buf[len-1] == '\n') {
        buf[len-1] = '\0';
        len--;
      }
    }

    // skip empty lines
    if (len == 0 || buf[0] == '\0') {
      continue;
    }

    // skip comments
    if (buf[0] == '#') {
      continue;
    }

    for (int i = 0; i < len; i++) {
      if (buf[i] == '#') {
        buf[i] = '\0';
        len = i;
        break;
      }
    }

    
    while (len > 0 && (buf[len-1] == ' ' || buf[len-1] == '\t')) {
      buf[len-1] = '\0';
      len--;
    }

    // Check if line became empty after removing comments
    if (len == 0 || buf[0] == '\0') {
      continue;
    }

    // Tokenize
    char *tokens[128];
    int token_cnt = 0;
    char *next_tok = buf;
    char *curr_tok;

    while ((curr_tok = next_token(&next_tok, " \t\n")) != 0) {
      tokens[token_cnt++] = curr_tok;
    }
    tokens[token_cnt] = 0;

    // empty command after tokenization
    if (token_cnt == 0) {
      continue;
    }

    // add to history
    add_to_history(buf, cmd_num);
    history_count = cmd_num;  
    cmd_num++;

    // built-in commands 
    int builtin_result = execute_builtin(tokens);

    if (builtin_result >= 0) {
      exit_status = builtin_result;
      continue;
    }

    // fork and exec
    int pid = fork();

    if (pid == -1) {
      fprintf(2, "Fork failed\n");
      exit_status = 1;
    } else if (pid == 0) {
      // Child process
      exec(tokens[0], tokens);
      fprintf(2, "exec %s failed\n", tokens[0]);
      exit(1);
    } else {
      // Parent process
      int status;
      wait(&status);
      exit_status = status;
    }
  }

cleanup:
  if (input_from_file) {
    close(input_fd);
  }

  return 0;
}

