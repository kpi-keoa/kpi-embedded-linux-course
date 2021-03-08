=============================================
Лабораторна робота №3
=============================================

Тема
------

**Потоки у ядрі та синхронізація**

Завдання:
-------
Написати та зібрати  власний модуль ядра, який:
* содержит глобальную переменную glob_var и параметры thread_num, thread_inccnt, thread_delay
* запускает thread_num потоков на одновременное выполнение
* каждый поток инкрементирует переменную glob_var thread_inccnt раз с задержкой thread_delay (может быть 0 — в этом случае, без задержки), кладет значение переменной в список и завершается
* при выгрузке модуль выводит значение переменной glob_var и содержимое списка
* для переменной, списка, потоков использовать динамическую аллокацию. Переменную передавать в поток аргументом
* предусматривает возможность досрочной выгрузки и будет нормально отрабатывать в этом случае

А також:
* Защитите увеличение счетчика, а также доступ к списку (и другим шареным элементам) механизмами блокировки и продемонстрируйте, что количество отсчетов правильное
* Сделайте второй вариант кода модуля, который бы использовал собственные реализации lock() и unlock() с использованием атомарных операций ядра (atomic.h, отличается в зависимости от архитектуры). Продемонстрируйте работоспособность. Можно использовать функцию xchg

Хід роботи
-------

**Опис виконаної роботи** 
В ході роботи було написано модуль ядра мовою С.

Спочатку створено параметри модуля *thread_num*, *thread_delay* та *thread_inccnt* та додано їхні описи.

Далі було створено вказівник на потоки *thread*, глобальну змінну *glob_var*, та зв'язний список *list*. Далі створено структуру(*_tmp_head*) для списку, яка містить змінну такого ж типу, як і *glob_var*.

Потім було створенно функцію-обробник для потоків *foo()*. У якій запускаємо цикл *for*, який *thread_inccnt* разів збільшує змінну *glob_var* на 1 із затримкою *thread_delay* мілісекунд, а також перевіряє, чи не викликав користувач раптом дострокове завершення потоку. Після закінчення циклу записуємо поточне значення змінної *glob_var* до списку, виводимо повідомлення про те, що потік завершився і виходимо з функції.

В *init_callback* функції лише перевіряються введені параметри на корректність, виділяється пам'ять під масив структур, які описують потоки і безпосереднью створюються та запускаються потоки.

В *exit_callback* функції зупиняємо потоки, звільнюємо пам'ять від масиву структур на потоки. Відображаємо в термінал вміст списку і фінальне значення змінної *glob_var*. І завершаємо роботу модуля.

Файл *no_lock.c* від інших відрізняється тим, що в ньому відсутня синхронізація. А файли *my_lock.c* та *kernel_lock.c* відрізняються між собою тим, що у *kernel_lock.c* використано "заводську" реалізацію спін-локів для синхронізації, а у *my_lock.c* - реалізацію спін-локів, написану в рамках лабораторної роботи.

Тести:
1) Без синхронізації потоків:

.. code-block:: bash

 / # insmod /mnt/no_lock.ko thread_num=10 thread_delay=1000 thread_inccnt=10
 [  121.581060] no_lock: loading out-of-tree module taints kernel.
 [  121.658445] insmod (80) used greatest stack depth: 13904 bytes left
 / # [  131.928178] thread is finished)
 [  131.928699] thread is finished)
 [  131.928937] thread is finished)
 [  131.930084] thread is finished)
 [  131.930300] thread is finished)
 [  131.930300] thread is finished)
 [  131.936528] thread is finished)
 [  131.937769] thread is finished)
 [  131.938417] thread is finished)
 [  131.938596] thread is finished) 
 / # rmmod no_lock.kp
 [  154.186539] glob_var per thread = 91
 [  154.186539] glob_var per thread = 92
 [  154.186539] glob_var per thread = 93
 [  154.189053] glob_var per thread = 94
 [  154.189301] glob_var per thread = 95
 [  154.189924] glob_var per thread = 96
 [  154.189924] glob_var per thread = 97
 [  154.189924] glob_var per thread = 98
 [  154.189924] glob_var per thread = 99
 [  154.189924] glob_var per thread = 100
 [  154.191737] Final glob_var is 100
 [  154.192538] Exit...

2) З синхронізацією за допомогою Spin lock`ів:
 
.. code-block:: bash

 / # insmod /mnt/kernel_lock.ko thread_num=10 thread_delay=1000 thread_inccnt=10
 / # [  151.488449] thread is finished)
 [  151.488559] thread is finished)
 [  151.489273] thread is finished)
 [  151.489109] thread is finished)
 [  151.489109] thread is finished)
 [  151.489273] thread is finished)
 [  151.489273] thread is finished)
 [  151.489273] thread is finished)
 [  151.489273] thread is finished)
 [  151.489273] thread is finished)
 / # rmmod kernel_lock
 [  157.427747] glob_var per thread = 100
 [  157.427747] glob_var per thread = 100
 [  157.427747] glob_var per thread = 100
 [  157.427747] glob_var per thread = 100
 [  157.427747] glob_var per thread = 100
 [  157.427747] glob_var per thread = 100
 [  157.427747] glob_var per thread = 100
 [  157.427747] glob_var per thread = 100
 [  157.427747] glob_var per thread = 100
 [  157.427747] glob_var per thread = 100
 [  157.427747] Final glob_var is 100
 [  157.427747] Exit...

3) Власна реалізація lock() та unlock():

.. code-block:: bash

 / # insmod /mnt/my_lock.ko thread_num=10 thread_delay=1000 thread_inccnt=10
 [  103.715502] my_lock: loading out-of-tree module taints kernel.
 [  103.756943] insmod (78) used greatest stack depth: 13904 bytes left
 / # [  114.569231] thread is finished)
 [  114.577393] thread is finished)
 [  114.580204] thread is finished)
 [  114.585232] thread is finished)
 [  114.586028] thread is finished)
 [  114.586028] thread is finished)
 [  114.586028] thread is finished)
 [  114.586028] thread is finished)
 [  114.592877] thread is finished)
 [  114.597518] thread is finished)
 / # rmmod my_lock
 [  126.867572] glob_var per thread = 100
 [  126.872589] glob_var per thread = 100
 [  126.873318] glob_var per thread = 100
 [  126.873318] glob_var per thread = 100
 [  126.873318] glob_var per thread = 100
 [  126.873318] glob_var per thread = 100
 [  126.873318] glob_var per thread = 100
 [  126.873318] glob_var per thread = 100
 [  126.873318] glob_var per thread = 100
 [  126.873318] glob_var per thread = 100
 [  126.873318] Final glob_var is 100
 [  126.873318] Exit...


Висновки
-------
В ході роботи отримано практичні навички роботи з потоками та зі списками в ядрі, ознайомлено з методами синхронізації.
Спочатку було створено модуль без синхронізації, як видно з результатів відпрацювання модуля для коректної роботи потрібно
додати синхронізацію до критичних частин коду.
Модулі *my_lock* та *kernel_lock* мають синхронізацію у вигляді спін-локів,
різниця лиш в тому, що *kernel_lock* використовує спін-локи, з бібліотеки, а в *my_lock* використовується власноруч написані функції для роботи зі спін-локами, які використовують атомарні функції, що у свою чергу допомогло зрозуміти принцип роботи спін-локів.



