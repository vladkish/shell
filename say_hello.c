#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
  char name[50] = "Anonymous";
  if (argc > 1) {
    strncpy(name, argv[1], 50);
  }
  printf("Hello, %s\n", name);
}
