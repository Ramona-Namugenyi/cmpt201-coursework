#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* ASSERT macro with file and line */
#define ASSERT(expr)                                                           \
  {                                                                            \
    if (!(expr)) {                                                             \
      fprintf(stderr, "Assertion failed: %s\n", #expr);                        \
      fprintf(stderr, "File: %s Line: %d\n", __FILE__, __LINE__);              \
      exit(1);                                                                 \
    }                                                                          \
  }

#define TEST(expr)                                                             \
  {                                                                            \
    if (!(expr)) {                                                             \
      fprintf(stderr, "Test failed: %s\n", #expr);                             \
      fprintf(stderr, "File: %s Line: %d\n", __FILE__, __LINE__);              \
      exit(1);                                                                 \
    } else {                                                                   \
      printf("Test passed: %s\n", #expr);                                      \
    }                                                                          \
  }
typedef struct node1 {
  uint64_t data;
  struct node1 *next;
} node1_t;

node1_t *head1 = NULL;

void insert_sorted1(uint64_t data) {
  node1_t *new_node = malloc(sizeof(node1_t));
  ASSERT(new_node != NULL);

  new_node->data = data;
  new_node->next = NULL;

  if (head1 == NULL || data < head1->data) {
    new_node->next = head1;
    head1 = new_node;
    return;
  }

  node1_t *curr = head1;

  while (curr->next != NULL && curr->next->data < data) {
    curr = curr->next;
  }

  new_node->next = curr->next;
  curr->next = new_node;
}

int index_of1(uint64_t data) {
  node1_t *curr = head1;
  int index = 0;

  while (curr != NULL) {
    if (curr->data == data) {
      return index;
    }
    curr = curr->next;
    index++;
  }

  return -1;
}

void run_example1() {
  insert_sorted1(1);
  insert_sorted1(2);
  insert_sorted1(5);
  insert_sorted1(3);

  TEST(index_of1(3) == 2);

  insert_sorted1(0);
  insert_sorted1(4);

  TEST(index_of1(4) == 4);
}

typedef struct node2 {
  uint64_t data;
  struct node2 *next;
} node2_t;

typedef struct info {
  uint64_t sum;
} info_t;

node2_t *head2 = NULL;
info_t info = {0};

void insert_sorted2(uint64_t data) {
  node2_t *new_node = malloc(sizeof(node2_t));
  ASSERT(new_node != NULL);

  new_node->data = data;
  new_node->next = NULL;

  if (head2 == NULL || data < head2->data) {
    new_node->next = head2;
    head2 = new_node;
  } else {
    node2_t *curr = head2;

    while (curr->next != NULL && curr->next->data < data) {
      curr = curr->next;
    }

    new_node->next = curr->next;
    curr->next = new_node;
  }

  info.sum += data;
}

int index_of2(uint64_t data) {
  node2_t *curr = head2;
  int index = 0;

  while (curr != NULL) {
    if (curr->data == data) {
      return index;
    }
    curr = curr->next;
    index++;
  }

  return -1;
}

uint64_t calculate_sum() {
  uint64_t sum = 0;
  node2_t *curr = head2;

  while (curr != NULL) {
    sum += curr->data;
    curr = curr->next;
  }

  return sum;
}

void run_example2() {
  insert_sorted2(1);
  insert_sorted2(3);
  insert_sorted2(5);
  insert_sorted2(2);

  TEST(info.sum == 1 + 3 + 5 + 2);
  TEST(index_of2(2) == 1);

  ASSERT(info.sum == calculate_sum());
}

int main() {
  printf("Running Example 1...\n");
  run_example1();

  printf("Running Example 2...\n");
  run_example2();

  printf("All tests passed.\n");
  return 0;
}
