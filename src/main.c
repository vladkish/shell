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

typedef enum { RUNNING, FINISHED } BgJobStateT;

static char *get_state_label(BgJobStateT state) {
  char *buf = malloc(sizeof(char) * 10);
  switch (state) {
  case RUNNING:
    buf = "Running";
    break;
  case FINISHED:
    buf = "Finished";
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

void bg_job_done_callback(int status_code, pid_t pid, int pipefd[2]) {
  printf("\nBg process %d done with status: %d\n", pid, status_code);
  char buf[15];
  printf("Reading from %d\n", pipefd[0]);
  if (read(pipefd[0], buf, sizeof(buf)) == -1) {
    perror("read");
    exit(1);
  }
  printf("Received from buf: %s\n", buf);
  int job_id = atoi(buf);
  printf("Job id: %d\n", job_id);
  background_jobs[job_id]->state = FINISHED;
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
    char *job_status = get_state_label(job->state);
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
  char *command_buf;
  char **params_buf;
} bg_job_runner_params;

void *run_bg_job(void *arg) {
  int statloc;
  bg_job_runner_params *cmd_args = arg;
  pid_t runner_pid = exec_command(cmd_args->command_buf, cmd_args->params_buf);
  int job_id =
      add_bg_job(cmd_args->command_buf, cmd_args->params_buf, runner_pid);
  waitpid(runner_pid, &statloc, 0);
  bg_job_done_callback(WEXITSTATUS(statloc), runner_pid, job_id);
  return NULL;
}

int main() {
  pid_t bg_pid;
  int argc, pipefd[2];
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
      printf("Executing in background\n");
      params_buf[argc - 1] = NULL;
      bg_job_runner_params thread_params = {.command_buf = command_buf,
                                            .params_buf = params_buf};
      if (pthread_create(NULL, NULL, run_bg_job, &thread_params) != 0) {
        handle_error("Failed to create thread");
      };
      /* if (pipe(pipefd) == -1) */
      /*   perror("pipe"); */
      /* bg_pid = fork(); */
      /* if (bg_pid == 0) { */
      /*   if (close(pipefd[1]) == -1) */
      /*     perror("close"); */
      /*   int exit_code = */
      /*       exec_command(command_buf, params_buf, pipefd,
         bg_job_done_callback); */
      /*   if (close(pipefd[0]) == -1) */
      /*     perror("close"); */
      /*   exit(exit_code); */
      /* } */
      /* if (close(pipefd[0]) == -1)  */
      /*   perror("close"); */
      int job_id = add_bg_job(command_buf, params_buf, bg_pid);
      size_t buf_size = snprintf(NULL, 0, "%d", job_id) +
                        1; // get the required size for buffer to allocate
      char buf[buf_size];
      snprintf(buf, buf_size, "%d", job_id);
      printf("Writing job id to pipe: %s|\n", buf);
      if (write(pipefd[1], buf, strlen(buf) + 1) != strlen(buf) + 1) {
        perror("write");
      }
      if (close(pipefd[1]) == -1)
        perror("close");
      /* printf("Adding bg job. pos: %d\n", bg_jobs_pos); */
      /* add_bg_job(command_buf, params_buf, bg_pid); */
      /* printf("Bg job added. pos: %d\n", bg_jobs_pos); */
    } else {
      printf("Normal execution\n");
      int status = exec_command(command_buf, params_buf, NULL, NULL);
      if (status != EXIT_SUCCESS) {
        printf("Failed to execute command!\n");
      }
    }
    free(command_buf);
    free(params_buf);
  }
}
