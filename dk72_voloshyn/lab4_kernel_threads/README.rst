=====================
Лабораторна робота №4
=====================

Тема
------

**Потоки ядра та методи синхронізації**

Завдання:
---------
 
Написати та протестувати власний модуль ядра, який:
* Включає глобальну змінну *glob_var* і параметри *thread_num*, *thread_inccnt*, *thread_delay*;
* Запускає *thread_num* потоків на одночасне виконання;
* Кожен поток інкрементує глобальну змінну *thread_inccnt* разів з затримкою *thread_delay*, кладе значення зміноої в список і завершається;
* При вигрузці модуль вовидить занчення глобальної змінної і зміст списку; 
* Для змінної, списку, потоків використовувати динамічну аллокацію. Змінну передавати в потік аргументом ;
* Передбачає можливість досрочної вигрузки і буде нормально відпрацьовувати у цьому випадку. 

Хід роботи
----------
Теоретичні відомості по роботі з потоками наведені у лабораторній роботі №0. 

Для створення потоку використовується бібліотека pthread і виклик pthread_create, для синхронізації в цій же бібліотеці використовуються спеціальні 
об'єкти - мутекси. Мутекс - це об'єкт може належати в якийсь момент часу тільки одному потоку і має два стани - зайнятий і вільний. Потік намагається 
отримати доступ до системи до звільнення об'єкта. На цьому, власне, і розраховється синхронізація - перед використанням загального ресурсу потоки 
спочатку звертаються до мутекса.

Результати:

Без синхронізації:

.. code-block:: bash
 
 / # insmod /mnt/nolocks.ko thread_num=10 thread_delay=1 thread_inccnt=100
 [   111.516141] Sad delay time is 1 ms
 [   111,517462] Sad variable after increment = 100
 [   111,518563] Sad delay time is 1 ms
 [   111,519974] Sad variable after increment = 200
 [   111,520968] Sad delay time is 2 ms
 [   111,522269] Sad variable after increment = 303
 [   111,523592] Sad delay time is 1 ms
 [   111,524691] Sad variable after increment = 393
 [   111,526102] Sad delay time is 1 ms
 [   111,527096] Sad variable after increment = 473
 [   111,528417] Sad delay time is 1 ms
 [   111,529518] Sad variable after increment = 572
 [   111,530929] Sad delay time is 2 ms
 [   111,531923] Sad variable after increment = 681
 [   111,533224] Sad delay time is 1 ms
 [   111,534545] Sad variable after increment = 777
 [   111,535646] Sad delay time is 2 ms
 [   111,535931] Sad variable after increment = 863
 [   111,537057] Sad delay time is 1 ms
 [   111,538051] Sad variable after increment = 946
 [   111,539372] Sad variable is - 946 now

З сингронізацією:

.. code-block:: bash

 / # insmod /mnt/nolocks.ko thread_num=10 thread_delay=1 thread_inccnt=100
 [   231.746141] Sad delay time is 2 ms
 [   231,747462] Sad variable after increment = 100
 [   231,748569] Sad delay time is 1 ms
 [   231,749972] Sad variable after increment = 200
 [   231,740968] Sad delay time is 1 ms
 [   231,742269] Sad variable after increment = 300
 [   231,743592] Sad delay time is 1 ms
 [   231,744691] Sad variable after increment = 400
 [   231,746106] Sad delay time is 2 ms
 [   231,747096] Sad variable after increment = 500
 [   231,748417] Sad delay time is 1 ms
 [   231,749518] Sad variable after increment = 600
 [   231,750929] Sad delay time is 1 ms
 [   231,751923] Sad variable after increment = 700
 [   231,753222] Sad delay time is 1 ms
 [   231,754545] Sad variable after increment = 800
 [   231,755646] Sad delay time is 1 ms
 [   231,755935] Sad variable after increment = 900
 [   231,757057] Sad delay time is 1 ms
 [   231,758052] Sad variable after increment = 1000
 [   231,759374] Sad variable is - 1000 now

З власною синхронізацією:

.. code-block:: bash

 / # insmod /mnt/nolocks.ko thread_num=10 thread_delay=1 thread_inccnt=100
 [   553.625758] Sad delay time is 1 ms
 [   553.625758] Sad variable after increment = 100
 [   553.627166] Sad delay time is 1 ms
 [   553.625758] Sad variable after increment = 200
 [   553.627309] Sad delay time is 1 ms
 [   553.625758] Sad variable after increment = 300
 [   553.627468] Sad delay time is 2 ms
 [   553.625758] Sad variable after increment = 400
 [   553.627596] Sad delay time is 1 ms
 [   553.625758] Sad variable after increment = 500
 [   553.627893] Sad delay time is 1 ms
 [   553.625758] Sad variable after increment = 600
 [   553.628160] Sad delay time is 2 ms
 [   553.625758] Sad variable after increment = 700
 [   553.628388] Sad delay time is 1 ms
 [   553.625758] Sad variable after increment = 800
 [   553.628700] Sad delay time is 1 ms
 [   553.625758] Sad variable after increment = 900
 [   553.628883] Sad delay time is 2 ms
 [   553.625758] Sad variable after increment = 1000
 [   553.630036] Sad variable is - 1000 now


Висновки
--------
Як видно з результатів виконання лабораторної роботи, При роботі модуля ядра без 
синхронізацій, глобальна змінна інкрементується з невеликими похибками. При роботі з готовими 
реалізаціями синхронізацій *mutex* видно, що глобальна змінна інкрементується вірно. Інструменти 
синхронізації використовуються в основному для захисту критичних точок спрацювання ядра. 
