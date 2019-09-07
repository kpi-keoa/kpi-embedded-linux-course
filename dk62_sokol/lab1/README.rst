Лабораторна робота №1

Хід роботи:
	
1. Ознайомитись з Reference Manual BBXM.
2. Розархівувати та записати на SD-карту операційну систему Debian Stretch for BeagleBoard.
Виконується за допомогою команди:
sudo sh -c "xz --decompress --keep --stdout bbxm-debian.img.xz | dd status=progress bs=4M of=/dev/mmcblk1; sync"
3. Далі вставляємо SD-карту в ВВХМ та підключаємо джерело живлення.
4. Переконуємось, що софтверний USB-хаб ID 1d6b:0104 Linux Foundation Multifunction Composite Gadget присутній.
Виконується за допомогою команди "lsusb"
5. Налаштовуємо IP адресс вручну (192.168.7.1/24).
6. Перевіряємо, що мережеві пакети доходять нормально за допомогою команди "ping 192.168.7.2"
7. Підключаємось до ВВХМ по ssh. Пароль: temppwd, Логін; debian.
Виконуємо команду "ssh debian@192.168.7.2" після чого вводимо пароль.
8. Створюємо програму, яка запускає 2 потоки(використовуючи бібліотеку pthread).
При цьому кожен потік повинен збільшувати глобальну змінну на 1 в циклі до 100000000.
Код програми описаний в наступному лістингу:
...

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

void *thread_function();

int main()
{
	pthread_t a_thread;
	int res = pthread_create(&a_thread, NULL, thread_function, NULL);

	clock_t a1 = clock();

	for (int cnt1 = 0; cnt1 < 1000000000; cnt1++);

	clock_t b1 = clock();
	printf("cnt1 = %f second\n", (float)(b1 - a1)/CLOCKS_PER_SEC);	

	res = pthread_join(a_thread, NULL);
	return 0;
}

void *thread_function()
{
	clock_t a2 = clock();

	for (int cnt2 = 0; cnt2 < 1000000000; cnt2++);

	clock_t  b2 = clock();
	printf("cnt2 = %f second\n", (float)(b2 - a2)/CLOCKS_PER_SEC);


}

...

9. Створюємо Makefile для збирання програми використовуючи флаги -O0 та -O2.
Код програми описаний в наступному лістингу:
...

CC = gcc
CFILES = thread.c

CFLAGS1 = -std=c11 -O0 -D_REENTRANT -lpthread
CFLAGS2 = -std=c11 -O2 -D_REENTRANT -lpthread

NAME1 = thread1
NAME2 = thread2

all: install, clean
install:
	@$(CC) $(CFILES) $(CFLAGS1) -o $(NAME1)
	@$(CC) $(CFILES) $(CFLAGS2) -o $(NAME2)
clean:
	rm -f $(NAME1) $(NAME2)

...

10.Збираємо та запускаємо программу на платформах х86 та ARM.
Результатом для х86 буде:
*з флагом -O0: 0.486631 секунд та 0.490275 секунд
*з флагом -O2: 0.00006 секунд та 0.000708 секунд
Результатом для ARM буде:
*з флагом -O0: 1.506652 секунд та 1.502990 секунд
*з флагом -O2: 0.000153 секунд та 0.000092 секунд


 