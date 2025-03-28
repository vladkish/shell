#ifndef BG_JOBS_H
#define BG_JOBS_H

#include "../utils.h"
#include "unistd.h"

#define BG_JOBS_SIZE 100

typedef enum { RUNNING, SUCCEEDED, FAILED } BgJobStateT;

typedef struct {
  pid_t pid;
  BgJobStateT state;
  char command[COMMAND_SIZE];
} BgJob;

int add_bg_job(char *command_buf, char **params_buf, pid_t job_pid);
void show_bg_jobs();
typedef struct {
  char *executable;
  char **params;
} command_t;

void *run_bg_job(void *arg);

#endif
