#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HISTORY_SIZE 5

static void strip_newline(char *s) {
  if (!s)
    return;
  size_t n = strlen(s);
  if (n > 0 && s[n - 1] == '\n')
    s[n - 1] = '\0';
}

static void add_history(char *history[], int *count, int *next,
                        const char *line) {

  if (history[*next] != NULL) {
    free(history[*next]);
    history[*next] = NULL;
  }

  history[*next] = malloc(strlen(line) + 1);
  if (!history[*next]) {
    perror("malloc");
    exit(1);
  }
  strcpy(history[*next], line);

  // Update counters
  if (*count < HISTORY_SIZE)
    (*count)++;
  *next = (*next + 1) % HISTORY_SIZE;
}

static void print_history(char *history[], int count, int next) {
  // Oldest item index:
  int start = (count < HISTORY_SIZE) ? 0 : next;

  for (int i = 0; i < count; i++) {
    int idx = (start + i) % HISTORY_SIZE;
    printf("%s\n", history[idx]);
  }
}

int main(void) {
  char *history[HISTORY_SIZE] = {NULL, NULL, NULL, NULL, NULL};
  int count = 0; // how many lines currently stored (<= 5)
  int next = 0;  // next index to write to (circular)

  char *line = NULL; // getline buffer (owned by getline)
  size_t cap = 0;

  while (1) {
    printf("Enter input: ");
    fflush(stdout);

    ssize_t len = getline(&line, &cap, stdin);
    if (len == -1) {
      // EOF or error (Ctrl+D, etc.)
      break;
    }

    // Make a clean version without trailing newline
    strip_newline(line);

    // Store it in history (including "print")
    add_history(history, &count, &next, line);

    // If command is "print" (no args), print last up to 5 lines
    if (strcmp(line, "print") == 0) {
      print_history(history, count, next);
    }
  }

  // Cleanup
  free(line);
  for (int i = 0; i < HISTORY_SIZE; i++) {
    free(history[i]);
  }
  return 0;
}
