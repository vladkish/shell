#include "commands.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int parse_command(command_t *cmd) {
  char ch;
  int i = 0;
  for (; (ch = getchar()) != ' ' && ch != '\n' && i < PARAM_LEN; i++) {
    cmd->executable[i] = ch;
  }
  cmd->executable[i] = 0;
  if (!strlen(cmd->executable))
    return -1;
  cmd->argv[0] = cmd->executable;
  i = 1;
  // scanning parameters of the command
  for (; ch != '\n' && i < PARAMS_LIST_SIZE - 1; i++) {
    while ((ch = getchar()) == ' ') // skip all spaces
      ;
    int j = 0;
    for (; ch != ' ' && ch != '\n' && j < PARAM_LEN - 1; j++) {
      cmd->argv[i][j] = ch;
      ch = getchar();
    };
    cmd->argv[i][j] = 0;
  }
  cmd->argv[i] = NULL;
  return i;
}

void params_shift(char *params[], int shift_from) {
  int i = shift_from;
  for (; params[i + 1] != NULL; i++) {
    params[i] = params[i + 1];
  }
  params[i] = NULL;
}

// If invalid params supplied - returns NULL. Otherwise returns array with 3
// elements length of opened file descriptors, where idx corresponds to std
// stream for redirect from, and value corresponds to a new file descriptor
// Also that function removes found redirect params by shifting params array
int *extract_redirects(char *params[]) {
  int *fds_for_dup = malloc(3 * sizeof(int));
  int fd;
  for (int i = 0; i < 3; i++)
    fds_for_dup[i] = -1;
  for (int i = 1; params[i] != NULL; i++) {
    printf("Checking: %s|\n", params[i]);
    if (strcmp(params[i], ">") == 0 && params[i + 1] != NULL) {
      if ((fd = open(params[i + 1], O_CREAT | O_WRONLY)) == -1) {
        return NULL;
      }
      fds_for_dup[1] = fd;
      params_shift(params, i);
      params_shift(params, i);
      printf("Shift completed\n");
      for (int i = 0; params[i] != NULL; i++) {
        printf("param: %s\n", params[i]);
      }
    } else if (strcmp(params[i], "<") == 0 && params[i + 1] != NULL) {
      if ((fd = open(params[i + 1], O_CREAT | O_RDONLY)) == -1) {
        return NULL;
      }
      fds_for_dup[0] = fd;
      params_shift(params, i);
      params_shift(params, i);
    }
  }
  return fds_for_dup;
}
