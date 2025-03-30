#include "main.h"
#include "commands.h"
#include "features/bg_jobs.h"
#include "features/history.h"
#include "utils.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int parse_command(command_t *cmd);
int *extract_redirects(char *params[]);

int count_argv(char **argv) {
  int i = 0;
  for (; argv[i] != NULL; i++)
    ;
  return i;
}

int main() {
  int argc, statloc;
  pthread_t thread;
  while (1) {
    type_prompt();
    // TODO: рассмотреть выделение памяти для команды изначально в стеке, а если
    // команда запускается в фоне - копировать ее памяти в кучу
    command_t *cmd = malloc(sizeof(command_t));
    cmd->executable = malloc(sizeof(char) * PARAM_LEN);
    cmd->argv = malloc(sizeof(char[PARAMS_LIST_SIZE]));
    for (int i = 0; i < PARAMS_LIST_SIZE; i++)
      cmd->argv[i] = malloc(PARAM_LEN);
    argc = parse_command(cmd);
    if (argc == -1) {
      continue;
    }
    int is_handled = handle_shell_commands(cmd->executable);
    if (is_handled)
      continue;
    add_to_history(cmd);
    int *fds_for_dup = extract_redirects(cmd->argv);
    if (fds_for_dup == NULL) // indicates error
    {
      perror("open");
      continue;
    }
    argc = count_argv(cmd->argv);
    printf("count %d\n", argc);
    if (*cmd->argv[argc - 1] == RUN_BACKGROUND) {
      cmd->argv[argc - 1] = NULL;
      struct job_params *job_params = malloc(sizeof(struct job_params));
      job_params->command = cmd;
      job_params->fds_to_dup = fds_for_dup;
      if (pthread_create(&thread, NULL, run_bg_job, job_params) != 0) {
        handle_error("Failed to create thread");
      };
    } else {
      pid_t runner_pid = exec_command(cmd, fds_for_dup);
      if (runner_pid == -1) {
        printf("Failed to launch command!\n");
      } else {
        waitpid(runner_pid, &statloc, 0);
        if (WEXITSTATUS(statloc) != EXIT_SUCCESS) {
          printf("Failed to execute command!\n");
        }
      }
      free_cmd(cmd);
    }
  }
}
