#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#define PARAMS_LIST_SIZE 50
#define PARAM_LEN 50
#define RUN_BACKGROUND '&'
#define REDIRECT_STDOUT '>'
#define REDIRECT_STDOUT_APPEND ">>"
#define REDIRECT_STDIN '<'
#define REDIRECT_STDIN_APPEND "<<"

void type_prompt() { printf("Type your command: "); }

int parse_command(char *command_buf, char **params_buf) {
  char ch;
  int i = 0;
  for (; (ch = getchar()) != ' ' && ch != '\n' && i < PARAM_LEN; i++) {
    command_buf[i] = ch;
  }
  command_buf[i] = 0;
  params_buf[0] = command_buf;
  i = 1;
  // scanning parameters of the command
  for (; ch != '\n' && i < PARAMS_LIST_SIZE - 1; i++) {
    while ((ch = getchar()) == ' ') // skip all spaces
      ;
    int j = 0;
    for (; ch != ' ' && ch != '\n' && j < PARAM_LEN - 1; j++) {
      params_buf[i][j] = ch;
      ch = getchar();
    };
    params_buf[i][j] = 0;
  }
  params_buf[i] = NULL;
  return i;
}

void is_shell_param(char *params[]) {
  int i = 0;
  while (params[i] != NULL) {
    if (strncmp(params[i], REDIRECT_STDOUT_APPEND, 2) &&
        params[i + 1] != NULL) {
    }
  }
}

int search_command_path(char *command_prompt) {
  // if command is already an absolute path - dont search for it
  if (*command_prompt == '/') {
    return 0;
  }
  DIR *dir;
  struct dirent *ent;

  char *search_paths = getenv("PATH");
  if (search_paths == NULL) {
    printf("Warning: No PATH variable in current environment\n");
    return 0;
  }
  // start search from current directory
  char *path = "./";
  int i = 0;
  while (path != NULL) {
    dir = opendir(path);
    if (dir != NULL) {
      while ((ent = readdir(dir)) && strcmp(ent->d_name, command_prompt) != 0)
        ;
      closedir(dir);
      if (ent) {
        char buf[PARAM_LEN] = {0};
        strlcat(buf, path, sizeof(buf));
        // append slash on the end of path to correctly concatenate with
        // filename
        char *end_of_buf = strchr(buf, 0);
        *end_of_buf = '/';
        *(end_of_buf + 1) = 0;
        strlcat(buf, ent->d_name, sizeof(buf));
        strlcpy(command_prompt, buf, PARAM_LEN);
        return 0;
      }
    } else {
      printf("Couldn't open directory %s: %s\n", path, strerror(errno));
      return 1;
    }
    // supply search_paths only on first iteration
    path = strtok(i++ == 0 ? search_paths : NULL, ":");
  }
  return 0;
}

int exec_command(char *command, char *params[], void (*done_callback)(int)) {
  // if command is not abs path - search corresponding executable path
  if (search_command_path(command) != 0) {
    return 2;
  }
  pid_t pid = fork();
  int statloc = 0;
  if (pid == 0) {
    int exec_status = execve(command, params, 0);
    if (exec_status == -1) {
      perror(command);
      return 1;
    }
  } else {
    waitpid(pid, &statloc, 0);
    if (done_callback != NULL) {
      done_callback(WEXITSTATUS(statloc));
    }
    return WEXITSTATUS(statloc);
  }
  return 0;
}
void bg_process_done(int status) {
  printf("\nBg process done with status: %d", status);
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
    printf("ARGC: %d\n", argc);
    pid = fork();
    if (pid == 0) {
      if (*params_buf[argc - 1] == RUN_BACKGROUND) {
        params_buf[argc - 1] = NULL;
        bg_pid = fork();
        if (bg_pid == 0) {
          exit(exec_command(command_buf, params_buf, bg_process_done));
        }
        exit(0);
      }
      exit(exec_command(command_buf, params_buf, NULL));
    }
    waitpid(pid, &statloc, 0);
    printf("Process %d finished. Status: %d\n", pid, WEXITSTATUS(statloc));
    if (WEXITSTATUS(statloc) != 0) {
      perror(command_buf);
    }

    free(command_buf);
    free(params_buf);
  }
}
