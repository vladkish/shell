#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int parse_command(char *command_buf, char **params_buf);
int exec_command(char *command, char *params[], void (*done_callback)(int));

void type_prompt() { printf("Type your command: "); }

void bg_process_done(int status) {
  printf("\nBg process done with status: %d", status);
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
    printf("ARGC: %d\n", argc);
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
