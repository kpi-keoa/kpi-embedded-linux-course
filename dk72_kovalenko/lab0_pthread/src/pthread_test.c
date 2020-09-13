
/*  Код позаимствован из сайта:
    https://www.sites.google.com/site/miptcscourses/seminars/4-sem/4-fupm-frtk/parallelism
    и дополнен. */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

int count = 0;

void print_many_letters(char letter) {
  int i, k, n;
  k = 5;
  n = 100;
  for (i = 0; i < n; ++i) {
    // Замечание: printf в данном месте не очень хорошо подходит для
    // демонстрации, т.к. построчно буферизирует вывод.
    // write выводит данные на консоль сразу.
    write(1, &letter, 1);
    count = count + k;
  }
}

void *thread_procedure(void *arg) {
  // Необходимо преобразовать указатель из общего типа (void*) в частный.
  char *letter = (char *)arg;
  print_many_letters(*letter);
  return NULL;
}

int main(void) {
  char other_letter = 'b';

  // Породить вспомогательный поток.
  pthread_t thr;
  if (0 !=
      pthread_create(&thr, NULL, thread_procedure, (void *)&other_letter)) {
    printf("pthread_create failed!\n");
    return 1;
  }

  // Данный код будет исполняться параллельно с кодом созданного потока.
  print_many_letters('A');

  // Дождаться завершения вспомогательного потока.
  if (0 != pthread_join(thr, NULL)) {
    printf("pthread_join failed\n");
    return 1;
  }

  // Можно продолжить исполнять код в одном потоке.
  printf("\n");
  printf("%d\n", count);

  return 0;
}
