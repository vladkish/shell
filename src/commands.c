#include "commands.h"
#include "utils.h"
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void command_to_str(char buf[COMMAND_SIZE], command_t *cmd) {
  strlcpy(buf, cmd->executable, COMMAND_SIZE);
  strlcat(buf, " ", COMMAND_SIZE);
  for (int i = 1; cmd->argv[i] != NULL && i < PARAMS_LIST_SIZE; i++)
    strlcat(buf, cmd->argv[i], COMMAND_SIZE);
}

static int search_executable_path(char *executable) {
  // if command is already an absolute path - dont search for it
  if (*executable == '/') {
    return 0;
  }
  DIR *dir;
  struct dirent *ent;

  const char *env_path = getenv("PATH");
  if (env_path == NULL) {
    printf("Warning: No PATH variable in current environment\n");
    return 0;
  }
  char search_paths[2000] = {0};
  strlcpy(search_paths, env_path, sizeof(search_paths));
  // start search from current directory
  const char *path = "./";
  int i = 0;
  while (path != NULL) {
    dir = opendir(path);
    if (dir == NULL) {
      printf("Couldn't open directory %s: %s\n", path, strerror(errno));
      return 1;
    }
    while ((ent = readdir(dir)) && strcmp(ent->d_name, executable) != 0)
      ;
    closedir(dir);
    if (ent == NULL) {
      // supply search_paths only on first iteration
      path = strtok(i++ == 0 ? search_paths : NULL, ":");
      continue;
    }
    char buf[PARAM_LEN] = {0};
    strlcat(buf, path, sizeof(buf));
    // append slash on the end of path to correctly concatenate with
    // filename
    char *end_of_buf = strchr(buf, 0);
    *end_of_buf = '/';
    *(end_of_buf + 1) = 0;
    strlcat(buf, ent->d_name, sizeof(buf));
    /* printf("Found path for command %s  - %s\n", command_prompt, buf); */
    strlcpy(executable, buf, PARAM_LEN);
    return 0;
  }
  return -1;
}

void init_file_descriptors(int *descriptors) {
  for (int i = 0; i < 3; i++) {
    if (descriptors[i] == -1)
      continue;
    if (dup2(descriptors[i], i) == -1) {
      perror("dup2");
      exit(1);
    }
  }
}

// That function launches a new process for executing command and returns pid of
// newly created process
int exec_command(command_t *command, int *fds) {
  if (search_executable_path(command->executable) != 0) {
    return -1;
  }

  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    return -1;
  }
  if (is_parent_proc(pid))
    return pid;

  if (fds)
    init_file_descriptors(fds);
  printf("Executing command: %s\n", command->executable);
  int exec_status = execve(command->executable, command->argv, 0);
  if (exec_status == -1) {
    perror("execve");
    return -1;
  }
  return 0;
}

void show_history();
void show_bg_jobs();

int handle_shell_commands(char *executable) {
  int is_handled = 0;
  if (strcmp(executable, "exit") == 0) {
    printf("Exiting...\n");
    is_handled = 1;
    exit(0);
  } else if (strcmp(executable, "history") == 0) {
    show_history();
    is_handled = 1;
  } else if (strcmp(executable, "jobs") == 0) {
    show_bg_jobs();
    is_handled = 1;
  };
  return is_handled;
}

void free_cmd(command_t *cmd) {
  free(cmd->argv);
  free(cmd->executable);
  free(cmd);
}
