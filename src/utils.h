
#ifndef UTILS_H
#define UTILS_H

#include "main.h"

void type_prompt();
void build_full_command(char buf[COMMAND_SIZE], char *command_buf,
                        char **params_buf);

#endif
