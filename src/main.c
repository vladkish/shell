#include "main.h"
#include "commands.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define handle_error(msg)                                                      \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

void type_prompt() { printf("Type your command: "); }

typedef enum { RUNNING, SUCCEEDED, FAILED } BgJobStateT;

static const char *get_state_label(BgJobStateT state) {
  const char *buf = malloc(sizeof(char) * 10);
  switch (state) {
  case RUNNING:
    buf = "Running";
    break;
  case SUCCEEDED:
    buf = "Finished";
    break;
  case FAILED:
    buf = "Failed";
    break;
  }
  return buf;
}

typedef struct {
  pid_t pid;
  BgJobStateT state;
  char command[COMMAND_SIZE];
} BgJob;

static char commands_history[HISTORY_LEN][COMMAND_SIZE];
static int history_pos;

static BgJob *background_jobs[BG_JOBS_SIZE];
static int bg_jobs_pos;

void complete_bg_job(int status_code, pid_t pid, int job_id) {
  printf("\nBg process %d done with status: %d\n", pid, status_code);
  background_jobs[job_id]->state = SUCCEEDED;
  printf("Job %d state updated\n", job_id);
  type_prompt();
}

void build_full_command(char buf[COMMAND_SIZE], char *command_buf,
                        char **params_buf) {
  strlcpy(buf, command_buf, COMMAND_SIZE);
  strlcat(buf, " ", COMMAND_SIZE);
  for (int i = 1; params_buf[i] != NULL && i < PARAMS_LIST_SIZE; i++)
    strlcat(buf, params_buf[i], COMMAND_SIZE);
}

int add_bg_job(char *command_buf, char **params_buf, pid_t job_pid) {
  BgJob *job = malloc(sizeof(BgJob));
  job->pid = job_pid;
  job->state = RUNNING;
  build_full_command(job->command, command_buf, params_buf);
  background_jobs[bg_jobs_pos] = job;
  printf("Job added: %d %s\n", job->pid, job->command);
  int job_idx = bg_jobs_pos;
  bg_jobs_pos++;
  return job_idx;
}

void show_bg_jobs() {
  if (bg_jobs_pos == 0) {
    printf("There are no active background jobs!\n");
    return;
  }
  for (int i = 0; i < bg_jobs_pos; i++) {
    BgJob *job = background_jobs[i];
    const char *job_status = get_state_label(job->state);
    printf("%d | %s | %s\n", job->pid, job_status, job->command);
  }
}

void add_to_history(char *command_buf, char **params_buf) {
  build_full_command(commands_history[history_pos], command_buf, params_buf);
  history_pos++;
}

void show_history() {
  if (history_pos == 0) {
    printf("Commands history is empty!\n");
    return;
  }
  for (int i = 0; i < history_pos; i++) {
    printf("%s\n", commands_history[i]);
  }
}

typedef struct {
  char *executable;
  char **params;
} command_t;

void *run_bg_job(void *arg) {
  command_t *cmd = arg;
  printf("Running bg cmd: %s with param: %s\n", cmd->executable,
         cmd->params[1]);
  int statloc;
  int *status = malloc(sizeof(int));
  pid_t runner_pid = exec_command(cmd->executable, cmd->params);
  printf("Runner pid: %d\n", runner_pid);
  if (runner_pid == -1) {
    status = &(int){1};
    return status;
  }

  int job_id = add_bg_job(cmd->executable, cmd->params, runner_pid);
  if (waitpid(runner_pid, &statloc, 0) == -1) {
    printf("waitpid failed\n");
    status = &(int){-1};
    return status;
  };
  status = &(int){WEXITSTATUS(statloc)};
  printf("\nBg process %d done with status: %d\n", runner_pid, *status);
  background_jobs[job_id]->state = *status == 0 ? SUCCEEDED : FAILED;
  printf("Job %d state updated\n", job_id);
  free(cmd->params);
  free(cmd->executable);
  type_prompt();
  return status;
}

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
      command_t thread_params = {.executable = command_buf,
                                 .params = params_buf};
      if (pthread_create(&thread, NULL, run_bg_job, &thread_params) != 0) {
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
