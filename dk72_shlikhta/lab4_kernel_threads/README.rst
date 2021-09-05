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

.. code-block:: bash

	/mnt # insmod no_lock.ko thread_num=15 thread_delay=0 thread_inccnt=1000
	/mnt # [   92.591396] thread is finished)
	[   92.598473] thread is finished)
	[   92.600160] thread is finished)
	[   92.600162] thread is finished)
	[   92.600214] thread is finished)
	[   92.600230] thread is finished)
	[   92.600265] thread is finished)
	[   92.600266] thread is finished)
	[   92.600283] thread is finished)
	[   92.600327] thread is finished)
	[   92.600341] thread is finished)
	[   92.600350] thread is finished)
	[   92.600363] thread is finished)
	[   92.600371] thread is finished)
	[   92.600382] thread is finished)
	/mnt # rmmod no_lock
	[   97.807584] glob_var per thread = 14734
	[   97.811375] glob_var per thread = 14736
	[   97.815106] glob_var per thread = 14746
	[   97.818358] glob_var per thread = 14746
	[   97.821618] glob_var per thread = 14746
	[   97.825049] glob_var per thread = 14746
	[   97.828319] glob_var per thread = 14746
	[   97.830785] glob_var per thread = 14746
	[   97.833670] glob_var per thread = 14746
	[   97.836376] glob_var per thread = 14746
	[   97.840640] glob_var per thread = 14746
	[   97.844136] glob_var per thread = 14746
	[   97.846643] glob_var per thread = 14746
	[   97.849226] glob_var per thread = 14746
	[   97.851887] glob_var per thread = 14746
	[   97.854401] Final glob_var is 14746
	[   97.856757] Goodbye:)
	/mnt # insmod kernel_lock.ko thread_num=15 thread_delay=0 thread_inccnt=1000
	/mnt # [ 2734.947281] thread is finished)
	[ 2734.949766] thread is finished)
	[ 2734.950173] thread is finished)
	[ 2734.950198] thread is finished)
	[ 2734.952132] thread is finished)
	[ 2734.952245] thread is finished)
	[ 2734.952444] thread is finished)
	[ 2734.955007] thread is finished)
	[ 2734.957059] thread is finished)
	[ 2734.958010] thread is finished)
	[ 2734.958511] thread is finished)
	[ 2734.959018] thread is finished)
	[ 2734.961075] thread is finished)
	[ 2734.964121] thread is finished)
	[ 2734.966248] thread is finished)
	/mnt # rmmod kernel_lock
	[ 2746.850777] glob_var per thread = 14967
	[ 2746.854511] glob_var per thread = 14967
	[ 2746.858190] glob_var per thread = 14972
	[ 2746.861532] glob_var per thread = 14976
	[ 2746.864793] glob_var per thread = 14978
	[ 2746.868263] glob_var per thread = 14981
	[ 2746.871984] glob_var per thread = 14981
	[ 2746.875440] glob_var per thread = 14985
	[ 2746.878759] glob_var per thread = 14993
	[ 2746.882186] glob_var per thread = 14994
	[ 2746.885642] glob_var per thread = 14994
	[ 2746.890903] glob_var per thread = 14994
	[ 2746.893276] glob_var per thread = 14997
	[ 2746.895625] glob_var per thread = 15000
	[ 2746.897363] glob_var per thread = 15000
	[ 2746.898526] Final glob_var is 15000
	[ 2746.899579] Goodbye:)
	/mnt # insmod my_lock.ko thread_num=15 thread_delay=0 thread_inccnt=1000
	/mnt # [ 2762.700196] thread is finished)
	[ 2762.704156] thread is finished)
	[ 2762.704233] thread is finished)
	[ 2762.704235] thread is finished)
	[ 2762.704301] thread is finished)
	[ 2762.706103] thread is finished)
	[ 2762.706314] thread is finished)
	[ 2762.706364] thread is finished)
	[ 2762.708153] thread is finished)
	[ 2762.708155] thread is finished)
	[ 2762.708212] thread is finished)
	[ 2762.709099] thread is finished)
	[ 2762.710112] thread is finished)
	[ 2762.710259] thread is finished)
	[ 2762.710305] thread is finished)
	/mnt # rmmod my_lock
	[ 2768.943851] glob_var per thread = 14967
	[ 2768.946793] glob_var per thread = 14981
	[ 2768.949695] glob_var per thread = 14988
	[ 2768.952561] glob_var per thread = 14988
	[ 2768.955562] glob_var per thread = 14991
	[ 2768.958716] glob_var per thread = 14991
	[ 2768.961550] glob_var per thread = 14993
	[ 2768.964259] glob_var per thread = 14994
	[ 2768.967089] glob_var per thread = 14998
	[ 2768.969829] glob_var per thread = 14998
	[ 2768.972687] glob_var per thread = 15000
	[ 2768.977328] glob_var per thread = 15000
	[ 2768.980187] glob_var per thread = 15000
	[ 2768.983010] glob_var per thread = 15000
	[ 2768.985872] glob_var per thread = 15000
	[ 2768.989439] Final glob_var is 15000
	[ 2768.992237] Goodbye:)





Висновки
-------
В ході роботи я навчився працювати з потоками та зі списками в ядрі, познайомився з методами синхронізації.
Модуль *no_lock* не має ніякої синхронізації, тому фінальне значення глобальної змінної в цьому модулі не відповідає дійсності.
Модулі *my_lock* та *kernel_lock* мають синхронізацію у вигляді спін-локів,
різниця лиш в тому, що *kernel_lock* використовує спін-локи, з бібліотеки, а в *my_lock* використовується власноруч написані функції для роботи зі спін-локами, які використовують атомарні функції, що у свою чергу допомогло зрозуміти принцип роботи спін-локів.



