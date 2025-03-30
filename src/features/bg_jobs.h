#ifndef BG_JOBS_H
#define BG_JOBS_H

#include "../commands.h"
#include "../utils.h"
#include "unistd.h"

#define BG_JOBS_SIZE 100

typedef enum { RUNNING, SUCCEEDED, FAILED } BgJobStateT;

struct job_params {
  command_t *command;
  int *fds_to_dup;
};

typedef struct {
  pid_t pid;
  BgJobStateT state;
  char command[COMMAND_SIZE];
} BgJob;

int add_bg_job(command_t *cmd, pid_t job_pid);
void show_bg_jobs();

void *run_bg_job(void *arg);

#endif
