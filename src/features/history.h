#ifndef HISTORY_H
#define HISTORY_H

#include "../commands.h"

#define HISTORY_LEN 100
void show_history();
void add_to_history(command_t *cmd);

#endif
