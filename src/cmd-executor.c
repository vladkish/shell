#include "main.h"
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int search_command_path(char *command_prompt) {
  // if command is already an absolute path - dont search for it
  if (*command_prompt == '/') {
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
    while ((ent = readdir(dir)) && strcmp(ent->d_name, command_prompt) != 0)
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
    strlcpy(command_prompt, buf, PARAM_LEN);
    return 0;
  }
  return -1;
}

int exec_command(char *command, char *params[],
                 void (*done_callback)(int, pid_t)) {

  // That function divides execution flow to 2 processes. One for executing
  // command itself and second one for processing execution result

  pid_t pid = fork();
  int statloc = 0;
  if (pid == 0) {
    if (search_command_path(command) != 0) {
      return 2;
    }
    int exec_status = execve(command, params, 0);
    if (exec_status == -1) {
      perror(command);
      return 1;
    }
  } else {
    waitpid(pid, &statloc, 0);
    if (done_callback != NULL) {
      done_callback(WEXITSTATUS(statloc), pid);
    }
    return WEXITSTATUS(statloc);
  }
  return 0;
}
