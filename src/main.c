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

typedef struct {
  pid_t pid;
  char command[COMMAND_SIZE];
} bg_job;

static char commands_history[HISTORY_LEN][COMMAND_SIZE];
static int history_pos;

static bg_job *background_jobs[BG_JOBS_SIZE];
static int bg_jobs_pos;

void build_full_command(char buf[COMMAND_SIZE], char *command_buf,
                        char **params_buf) {
  strlcpy(buf, command_buf, COMMAND_SIZE);
  strlcat(buf, " ", COMMAND_SIZE);
  for (int i = 1; params_buf[i] != NULL && i < PARAMS_LIST_SIZE; i++)
    strlcat(buf, params_buf[i], COMMAND_SIZE);
}

void add_bg_job(char *command_buf, char **params_buf, pid_t job_pid) {
  bg_job *job = malloc(sizeof(bg_job));
  job->pid = job_pid;
  build_full_command(job->command, command_buf, params_buf);
  background_jobs[bg_jobs_pos] = job;
  printf("Job added: %d %s\n", job->pid, job->command);
  bg_jobs_pos++;
}

void show_bg_jobs() {
  if (bg_jobs_pos == 0) {
    printf("There are no active background jobs!\n");
    return;
  }
  for (int i = 0; i < bg_jobs_pos; i++) {
    bg_job *job = background_jobs[i];
    printf("%d | %s\n", job->pid, job->command);
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

int handle_shell_commands(char *command_buf) {
  int is_handled = 0;
  if (strcmp(command_buf, "exit") == 0) {
    printf("Exiting...\n");
    is_handled = 1;
    exit(0);
  } else if (strcmp(command_buf, "history") == 0) {
    show_history();
    is_handled = 1;
  } else if (strcmp(command_buf, "jobs") == 0) {
    show_bg_jobs();
    is_handled = 1;
  };
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
    if (*params_buf[argc - 1] == RUN_BACKGROUND) {
      printf("Executing in background\n");
      params_buf[argc - 1] = NULL;
      bg_pid = fork();
      if (bg_pid == 0) {
        exit(exec_command(command_buf, params_buf, bg_process_done));
      }
      printf("Adding bg job. pos: %d\n", bg_jobs_pos);
      add_bg_job(command_buf, params_buf, bg_pid);
      printf("Bg job added. pos: %d\n", bg_jobs_pos);
    } else {
      printf("Normal execution\n");
      int status = exec_command(command_buf, params_buf, NULL);
      if (status != 0) {
        printf("Failed to execute command!\n");
      }
    }
    free(command_buf);
    free(params_buf);
  }
}
