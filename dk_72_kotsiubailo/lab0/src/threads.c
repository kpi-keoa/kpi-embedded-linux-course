#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef unsigned long type_cnt;

volatile type_cnt gl_var = 0;

struct var {
  type_cnt num;
  type_cnt count;
};

void *func(void *the_val) {

  type_cnt val_n = ((struct var *)the_val)->num;
  type_cnt val_cnt = (( struct var *)the_val)->count;

  for (type_cnt i = 0; i < val_cnt; i++) {
    gl_var += val_n;
  }
  pthread_exit(0);
}

bool is_valfit(char *arg, type_cnt *res) {  // was partially taken from Nikita Luchenko lab_0
  int result;
  char unused;
  if (1 != sscanf(arg, "%d\"%c\"", &result, &unused))
    return 1;
  else
    *res = result;
  return 0;
}

int main(int argc, char *argv[]) {

  struct var used_var;

  if (argc != 3) {
    fprintf(stderr, "%s : wrrong amount of arguments : %d ; should be 2 \n",
            argv[0], argc - 1);
    exit(EXIT_FAILURE);
  }

  if (is_valfit(argv[1], &used_var.num) || is_valfit(argv[2], &used_var.count)) {
    fprintf(stderr, "\nWrong input types\n");
    exit(EXIT_FAILURE);
  }

  pthread_t *threads = (pthread_t *)malloc(2 * sizeof(pthread_t));

  for (int i = 0; i < 2; i++) {
    pthread_create(&(threads[i]), NULL, &func, &used_var);
  }

  for (int i = 0; i < 2; i++) {
    pthread_join(threads[i], NULL);
  }

  free(threads);

  printf("Expected value: %lu \n", 2 * used_var.num * used_var.count);
  printf("Real value: %lu \n", gl_var);

  return 0;
}
