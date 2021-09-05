=====================
Лабораторна робота №4
=====================

Тема
------

**Потоки ядра та методи синхронізації**

Завдання:
---------
 
* Написати та протестувати власний модуль ядра, який:
  
    содержит глобальную переменную glob_var и параметры thread_num, thread_inccnt, thread_delay      
    запускает thread_num потоков на одновременное выполнение      
    каждый поток инкрементирует переменную glob_var thread_inccnt раз с задержкой thread_delay (может быть 0 — в этом случае, без 
    задержки), кладет значение переменной в список и завершается      
    при выгрузке модуль выводит значение переменной glob_var и содержимое списка      
    для переменной, списка, потоков использовать динамическую аллокацию. Переменную передавать в поток аргументом      
    предусматривает возможность досрочной выгрузки и будет нормально отрабатывать в этом случае    

Хід роботи
----------

Модулі було створено на базі коду з попередніх лабораторних робіт, та навчального відео матеріалу по курсу. Реалізація робилась по методу - *подивись як в src ядра
і зроби так само*. В основному робота була в вивченні методів синхронізації. Я зупинився на *mutex* так як ще ніколи їх не використовував. 

Результати:

.. code-block:: bash

 [   57.625758] Delta time = 7ms
 [   57.627166] Delta time = 7ms
 [   57.627309] Delta time = 7ms
 [   57.627468] Delta time = 6ms
 [   57.627596] Delta time = 6ms
 [   57.627893] Delta time = 6ms
 [   57.628160] Delta time = 6ms
 [   57.628388] Delta time = 7ms
 [   57.628700] Delta time = 7ms
 [   57.628883] Delta time = 7ms
 [   57.629024] Delta time = 7ms
 [   57.629160] Delta time = 7ms
 [   57.629291] Delta time = 7ms
 [   57.629515] Delta time = 8ms
 [   57.629666] Delta time = 8ms
 [   57.630036] global var: 873
 [   57.630210] nolocks: exit
 / # insmod /mnt/nolocks.ko thread_num=4 thread_delay=5 thread_inccnt=220

.. code-block:: bash

 [   23.392791] Delta time = 6ms
 [   23.393221] Delta time = 7ms
 [   23.393575] Delta time = 6ms
 [   23.393877] Delta time = 6ms
 [   23.393987] Delta time = 6ms
 [   23.394094] Delta time = 6ms
 [   23.394201] Delta time = 6ms
 [   23.394307] Delta time = 6ms
 [   23.394413] Delta time = 7ms
 [   23.394517] Delta time = 6ms
 [   23.394517] Delta time = 6ms
 [   23.394517] Delta time = 6ms
 [   23.394947] Delta time = 6ms
 [   23.395085] Delta time = 6ms
 [   23.395193] Delta time = 6ms
 [   23.395300] Delta time = 6ms
 [   23.395407] Delta time = 6ms
 [   23.395514] Delta time = 6ms
 [   23.395620] Delta time = 6ms
 [   23.395727] Delta time = 6ms
 [   23.396055] global var: 880
 [   23.396198] mutex: exit
 / # insmod /mnt/mutex.ko thread_num=4 thread_delay=5 thread_inccnt=220

.. code-block:: bash

 [   38.781284] Delta time = 6ms
 [   38.781423] Delta time = 7ms
 [   38.781551] Delta time = 6ms
 [   38.782220] Delta time = 6ms
 [   38.783511] Delta time = 6ms
 [   38.783659] Delta time = 6ms
 [   38.783767] Delta time = 6ms
 [   38.783873] Delta time = 6ms
 [   38.784107] Delta time = 6ms
 [   38.784247] Delta time = 6ms
 [   38.784354] Delta time = 6ms
 [   38.784459] Delta time = 8ms
 [   38.784654] Delta time = 6ms
 [   38.784814] Delta time = 6ms
 [   38.784920] Delta time = 6ms
 [   38.785025] Delta time = 8ms
 [   38.785159] global var: 880
 [   38.785299] ownlock: exit
 / # insmod /mnt/ownlock.ko thread_num=4 thread_delay=5 thread_inccnt=220

Висновки
--------
В ході виконання лабораторної роботи було створено та протестовано власні модулі ядра. 
Модуль без синхронізації при вхідних параметрах *thread_num=4 thread_delay=5 thread_inccnt=220* видавав неправильні результати. Значення глобальної змінної варіювалось від *850* до *880*.
Відповідно модулі з синхронізацією видавали стабільний результат при аналогічних вхідних данних - *880*
