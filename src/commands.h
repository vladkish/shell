#ifndef COMMANDS_H

#define COMMANDS_H

#include <unistd.h>

int parse_command(char *command_buf, char **params_buf);
int exec_command(char *command, char *params[]);
int handle_shell_commands(char *command_buf);

#endif
