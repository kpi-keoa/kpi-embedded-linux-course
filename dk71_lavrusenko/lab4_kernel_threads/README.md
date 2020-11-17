
Лабораторна робота №4
====
Тема: Робота з потаками ядра
====
Завдання:
===
 1. Подивитися відео, переконайтеся, що зрозуміли, як працювати з потоками ядра

   2. Ознайомтеся зі списками ядра.
    
3. Почитайте про механізми синхронізації в ядрі:
    
4. Напишіть модуль ядра, який:
    - містить глобальну змінну `glob_var` і параметри `thread_num`, `thread_inccnt`, `thread_delay`
    - запускає `thread_num` потоків на одночасне виконання
    - кожен потік інкрементує змінну `glob_var` `thread_inccnt` раз з затримкою `thread_delay` (може бути 0 - в цьому випадку, без затримки), кладе значення змінної в список і завершується
    - під час вивантаження модуль виводить значення змінної `glob_var` і вміст списку
    - для змінної, списку, потоків використовувати динамічну аллокації. Змінну передавати в потік аргументом
    - передбачає можливість дострокової вивантаження і буде нормально відпрацьовувати в цьому випадку

5. Продемонструйте, що без синхронізації підсумкове значення глобальної змінної `glob_var` неправильне

6. Захистіть збільшення лічильника, а також доступ до списку (і іншим шареним елементів) механізмами блокування і продемонструйте, що кількість відліків правильне

7. Результати по 4 і 5 пункту включите в звіт

8. Зробіть другий варіант коду модуля, який би використовував власні реалізації `lock()` і `unlock()` з використанням атомарних операцій ядра (atomic.h, відрізняється в залежності від архітектури). Продемонструйте працездатність. Можна використовувати функцію xchg
   

Хід роботи:
====

В даній роботі для синхронізації потоків було використано мьюткейси, вони дозволять уникнути ситуації гонки потоків. Альтернатива проти мьюткейсів це Симофори, але вони є будівним матеріалом для інших механізмів синхронізацій, тому було прийнято рішення використовувати не такий низькорівневий механіхм синхроноізації. 
Для реалізації власного механізму синхронізації було використано сигнал "LOCK", який дозволяє заблокувати шину від інших процессорів.

Для запуску модуля потрібно спочатку скомпілювати його:
`make KBUILDDIR="<KBuild directory>"`

Запускаємо ядро за допомогою QEMU:
`qemu-system-x86_64 -enable-kvm -m 512M -smp 4 -kernel "<bzImage path>" -initrd "<path BusyBox shell utilits .gz>" \
                    -append "console=ttyS0" -nographic \
                    -drive file=fat:rw:./<path to module>,format=raw,media=disk`
                    
Підготовчі дії:
`mkdir mnt`
`mount -t vfat /dev/sda1 /mnt`

Запуск модуля:
`insmode /mnt/<module_name.ko>`

Вигрузка модуля:
`rmmod <module_name>`

Висновки:
===

В ході виконання лабараторної роботи було завоєна робота з потоками ядра. Було реалізовано три модулі ядра, два з яких мали механізми синхронізації. результати роботи іх можна побачити нижче.

Мьюткейси:
```
/ # insmod /mnt/lab4_mutex.ko thread_num=3 thread_inccnt=100 thread_delay=5
/ # [  253.521473] Task is runinng!
[  254.103560] The incremention is done!
[  254.105531] Task is runinng!
[  254.707527] The incremention is done!
[  254.709978] Task is runinng!
[  255.312568] The incremention is done!
[  255.315117] Task is runinng!

/ # rmmod lab4_mutex
[  255.917582] The incremention is done!
[  285.315265] Exit jiffies is 4294952479
[  285.319244] Delta time = 6ms
...
[  285.549722] Delta time = 6ms
[  285.550227] Delta time = 6ms
[  285.550756] global var: 300
[  285.551321] lab4_mutex: exit
[  285.552649] Ave Kernel!
```

Без синхронізації:
```
/ # insmod /mnt/lab3_no_sync.ko thread_num=3 thread_inccnt=100 thread_delay=5
[  460.009127] Task is runinng!
[  460.009636] Task is runinng!
[  460.010144] Task is runinng!
/ # [  460.011263] Task is runinng!
[  460.162572] The incremention is done!
[  460.165060] The incremention is done!
[  460.168215] The incremention is done!
/ # rmmod lab3_no_sync
[  460.171280] The incremention is done!
[  472.531072] Exit jiffies is 4295139695
[  472.533801] Delta time = 6ms
...
[  472.595203] Delta time = 7ms
[  472.595821] Delta time = 8ms
[  472.596412] global var: 213
[  472.597009] lab3_no_sync: exit
[  472.598117] Ave Kernel!
/ # ^C
/ # 
```

Власна синхронізація:
```
/ # insmod /mnt/lab4_my_sync.ko thread_num=3 thread_inccnt=100 thread_delay=5
[  113.322554] lab4_my_sync: loading out-of-tree module taints kernel.
[  113.326030] Task is runinng!
[  113.335163] insmod (95) used greatest stack depth: 13928 bytes left
/ # [  113.937740] Task is runinng!
[  113.937742] The incremention is done!
[  114.540770] The incremention is done!
[  114.540771] Task is runinng!
[  115.151743] The incremention is done!
[  115.151746] Task is runinng!
rmmod lab4_my_sync
[  115.752928] The incremention is done!
[  130.798278] Exit jiffies is 4294797947
[  130.803245] Delta time = 12ms
[  130.803247] Delta time = 6ms
...
[  131.080749] Delta time = 6ms
[  131.081360] global var: 300
[  131.081988] lab4_my_sync: exit
[  131.083203] Ave Kernel!
/ # 
```
В ході виконання лабараторної роботи було завоєно роботу з потоками ядра
