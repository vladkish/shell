#include <stdio.h>
#include <string.h>

int main() {
  char s[20] = "Hello My Cool World";
  strtok(s, " ");
  char *word = "First";
  while (word != NULL) {
    puts(word);
    puts(s);
    word = strtok(NULL, " ");
  }
}
