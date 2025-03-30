#include "bg_jobs.h"
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

int add_bg_job(command_t *cmd, pid_t job_pid) {
  BgJob *job = malloc(sizeof(BgJob));
  job->pid = job_pid;
  job->state = RUNNING;
  command_to_str(job->command, cmd);
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
  struct job_params *args = arg;
  int statloc;
  int *status = malloc(sizeof(int));
  pid_t runner_pid = exec_command(args->command, args->fds_to_dup);
  if (runner_pid == -1) {
    status = &(int){1};
    return status;
  }

  int job_id = add_bg_job(args->command, runner_pid);
  if (waitpid(runner_pid, &statloc, 0) == -1) {
    printf("waitpid failed\n");
    status = &(int){-1};
    return status;
  };
  status = &(int){WEXITSTATUS(statloc)};
  printf("\nBg process %d done with status: %d\n", runner_pid, *status);
  background_jobs[job_id]->state = *status == 0 ? SUCCEEDED : FAILED;
  printf("Job %d state updated\n", job_id);
  free_cmd(args->command);
  free(args);
  type_prompt();
  return status;
}
