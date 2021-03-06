=============================================
Лабораторна робота №3
=============================================

Тема
------

**Тасклети, таймери, аллокація пам'яті**

Завдання:
---------
Переписати код з другої лабораторної роботи так, щоб замість звичайного таймера використовувався *hrtimer*.

Хід роботи
----------

**Опис виконаної роботи** 
Модуль побудований на основі лабораторної №2. Замість *timer_list* наразі використовуємо *hrtimer* та *ktime*.
Ініціалізація відбувається за допомогою *hrtimer_init*. Адрес функції що буде викликатись при спрацюванні записується в поле
*function* структури *hrtimer*. Запуск таймеру за допомогою ф-ї - *hrtimer_start*. В обробнику переривань перезапускаємо таймер (*HRTIMER_RESTART*)
якщо ми не прорахували задану кількість разів. В іншому випадку перезапуску не відбувається (*HRTIMER_NORESTART*). Вимикаємо таймер - *hrtimer_cancel*.

Результат:

.. code-block:: bash

 / # insmod /mnt/khmod.ko cnt=10 delay=100
 [   41.234527] khmod: loading out-of-tree module taints kernel.
 [   41.259188] Initial Jiffies is 4294708576
 [   41.259553] Tasklet jiffies is 4294708576
 [   41.262490] insmod (93) used greatest stack depth: 13928 bytes left
 / # rmmod khmod
 [   46.826591] Exit jiffies is 4294714143
 [   46.826591] ArSray[0] = 4294708754
 [   46.827868] ArSray[1] = 4294708754
 [   46.828178] ArSray[2] = 4294708754
 [   46.828537] ArSray[3] = 4294708754
 [   46.828735] ArSray[4] = 4294708754
 [   46.828735] ArSray[5] = 4294708754
 [   46.828735] ArSray[6] = 4294708754
 [   46.828735] ArSray[7] = 4294708754
 [   46.830486] ArSray[8] = 4294708754
 [   46.830990] ArSray[9] = 4294708754
 [   46.831373] Exit
 / #

Висновки
--------
В ході роботи було написано модуль що використовує *hrtimer*. Можна помітити, що *hrtimer* є в рази точнішим, дельта *jiffies* співпадає з заданою нами. 
Хочеться відмітити, як написав в своєму звіті О. Матюша, *hrtimer* в рази зручніше конфігурувати. Я думаю що цей момент та велика точність є причиною його використання при написанні модулів ядра. 
