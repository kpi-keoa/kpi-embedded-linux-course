#include <stdio.h>
#include <time.h>

 int main() {
  int ee = 0, computer = 0;
  srand(time(NULL));
  ee = rand() % 6 + 1;
  computer = rand() % 6 + 1;
  printf("player = %d \n computer = %d \n", ee, computer);
  if(ee > computer){
    printf("player Win\n");
  }else if(computer > ee){
    printf("computer Win\n");
  }else{
    printf("nothing Win\n");
  }
  return 0;
}
