#include "main.h"
#include "commands.h"
#include "features/bg_jobs.h"
#include "features/history.h"
#include "utils.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define handle_error(msg)                                                      \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

int main() {
  int argc, statloc;
  pthread_t thread;
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
    if (*params_buf[argc - 1] == RUN_BACKGROUND) {
      params_buf[argc - 1] = NULL;
      command_t *thread_params = malloc(sizeof(command_t));
      thread_params->executable = command_buf;
      thread_params->params = params_buf;
      if (pthread_create(&thread, NULL, run_bg_job, thread_params) != 0) {
        handle_error("Failed to create thread");
      };
    } else {
      pid_t runner_pid = exec_command(command_buf, params_buf);
      if (runner_pid == -1) {
        printf("Failed to launch command!\n");
      } else {
        waitpid(runner_pid, &statloc, 0);
        if (WEXITSTATUS(statloc) != EXIT_SUCCESS) {
          printf("Failed to execute command!\n");
        }
      }
      free(command_buf);
      free(params_buf);
    }
  }
}
