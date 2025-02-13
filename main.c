#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#define PARAMS_LIST_SIZE 50
#define PARAM_LEN 100
#define RUN_BACKGROUND '&'

void type_prompt() { printf("Type your command: "); }

int read_command(char *command, char *params[]) {
  char ch;
  int i = 0;
  for (; (ch = getchar()) != ' ' && ch != '\n' && i < PARAM_LEN; i++) {
    command[i] = ch;
  }
  command[i] = 0;
  params[0] = command;
  i = 1;
  for (; ch != '\n' && i < PARAMS_LIST_SIZE - 1; i++) {
    while (ch == ' ')
      ch = getchar();
    int j = 0;
    for (; ch != ' ' && ch != '\n' && j < PARAM_LEN; j++) {
      if (params[i] == NULL) {
        params[i] = malloc(PARAM_LEN);
      }
      params[i][j] = ch;
      ch = getchar();
    };
    params[i][j] = 0;
  }
  params[i] = NULL;
  return i;
}

void handle_process_result(pid_t target_pid, char *command) {
  int statloc;
  pid_t pid = waitpid(target_pid, &statloc, 0);
  printf("Process %d finished. Status: %d\n", pid, WEXITSTATUS(statloc));
  if (WEXITSTATUS(statloc)) {
    perror(command);
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
  puts(search_paths);
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
        char buf[PARAM_LEN];
        *buf = 0;
        strlcat(buf, path, sizeof(buf));
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
    path = strtok(i++ == 0 ? search_paths : NULL, ":");
  }
  return 0;
}

void exec_command(char *command, char *params[], void (*done_callback)(int)) {
  if (*command != '/' && search_command_path(command) != 0) {
    exit(2);
  }
  pid_t pid = fork();
  int statloc = 0;
  if (pid == 0) {
    int exec_status = execve(command, params, 0);
    if (exec_status == -1) {
      perror(command);
      exit(EXIT_FAILURE);
    }
  } else {
    waitpid(pid, &statloc, 0);
    if (done_callback != NULL) {
      done_callback(WEXITSTATUS(statloc));
    }
    exit(WEXITSTATUS(statloc));
  }
}
void bg_process_done(int status) {
  printf("\nBg process done with status: %d", status);
}

int main() {
  char command[PARAM_LEN] = {0};
  pid_t pid, bg_pid;
  char *buf;
  char *params[PARAMS_LIST_SIZE];
  for (int i = 0; i < PARAMS_LIST_SIZE; i++) {
    buf = malloc(PARAM_LEN);
    if (buf == NULL) {
      printf("Failed to allocate memory for index: %d\n", i);
      exit(1);
    }
    params[i] = buf;
  }

  int statloc, argc;
  while (1) {
    type_prompt();
    argc = read_command(command, params);
    pid = fork();
    if (pid == 0) {
      /* printf("Executing command: %s, with params: ", command); */
      /* for (int i = 0; i < PARAMS_LIST_SIZE && strlen(params[i]) > 0; i++) */
      /*   printf("%s ", params[i]); */
      /* putchar('\n'); */
      if (*params[argc - 1] == RUN_BACKGROUND) {
        params[argc - 1] = NULL;
        bg_pid = fork();
        if (bg_pid == 0) {
          exec_command(command, params, bg_process_done);
        }
        exit(0);
      } else {
        exec_command(command, params, NULL);
      }
    } else {
      /* bg_pid = -1; */
      /* if (*params[argc - 1] == RUN_BACKGROUND) { */
      /*   params[argc - 1] = NULL; */
      /*   bg_pid = fork(); */
      /* } */
      /* // if running in background and it's a parent process - skip waiting */
      /* if (bg_pid > 0) */
      /*   continue; */
      waitpid(pid, &statloc, 0);
      printf("Process %d finished. Status: %d\n", pid, WEXITSTATUS(statloc));
      if (WEXITSTATUS(statloc)) {
        perror(command);
      }
    }
  }
}
