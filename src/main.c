#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int parse_command(char *command_buf, char **params_buf);
int exec_command(char *command, char *params[],
                 void (*done_callback)(int, pid_t));

void type_prompt() { printf("Type your command: "); }

void bg_process_done(int status, pid_t pid) {
  printf("\nBg process %d done with status: %d\n", status, pid);
  type_prompt();
}

static char commands_history[HISTORY_LEN][COMMAND_SIZE];
static int cmd_counter;

int add_to_history(char *command_buf, char **params_buf) {
  char *cmd = commands_history[cmd_counter];
  strlcpy(cmd, command_buf, COMMAND_SIZE);
  strlcat(cmd, " ", COMMAND_SIZE);
  for (int i = 1; params_buf[i] != NULL && i < PARAMS_LIST_SIZE; i++)
    strlcat(cmd, params_buf[i], COMMAND_SIZE);
  cmd_counter++;
  return 0;
}

void show_history() {
  if (cmd_counter == 0) {
    printf("Commands history is empty!\n");
    return;
  }
  for (int i = 0; i < cmd_counter; i++) {
    printf("%s\n", commands_history[i]);
  }
}

int handle_shell_commands(char *command_buf) {
  int is_handled = 0;
  if (strcmp(command_buf, "exit") == 0) {
    printf("Exiting...\n");
    is_handled = 1;
    exit(0);
  };
  if (strcmp(command_buf, "history") == 0) {
    show_history();
    is_handled = 1;
  }
  return is_handled;
}

int main() {
  pid_t pid, bg_pid;
  int statloc, argc;
  while (1) {
    type_prompt();
    char *command_buf = malloc(sizeof(char) * PARAM_LEN);
    char **params_buf = malloc(sizeof(char[PARAMS_LIST_SIZE]));
    for (int i = 0; i < PARAMS_LIST_SIZE; i++)
      params_buf[i] = malloc(PARAM_LEN);
    argc = parse_command(command_buf, params_buf);
    if (argc == -1) {
      continue;
    }
    int is_handled = handle_shell_commands(command_buf);
    if (is_handled)
      continue;
    add_to_history(command_buf, params_buf);
    pid = fork();
    if (pid == 0) {
      if (*params_buf[argc - 1] == RUN_BACKGROUND) {
        params_buf[argc - 1] = NULL;
        bg_pid = fork();
        if (bg_pid == 0) {
          exit(exec_command(command_buf, params_buf, bg_process_done));
        }
        exit(0);
      }
      exit(exec_command(command_buf, params_buf, NULL));
    }
    waitpid(pid, &statloc, 0);
    printf("Process %d finished. Status: %d\n", pid, WEXITSTATUS(statloc));
    if (WEXITSTATUS(statloc) != 0) {
      perror(command_buf);
    }

    free(command_buf);
    free(params_buf);
  }
}
