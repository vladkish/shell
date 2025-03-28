#include "bg_jobs.h"
#include "../commands.h"
#include <stdio.h>
#include <stdlib.h>

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

static BgJob *background_jobs[BG_JOBS_SIZE];
static int bg_jobs_pos;

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

void *run_bg_job(void *arg) {
  printf("Arg addr: %p\n", arg);
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
  free(cmd);
  type_prompt();
  return status;
}
