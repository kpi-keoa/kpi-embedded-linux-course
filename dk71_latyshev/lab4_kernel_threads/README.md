
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
/ # insmod /mnt/lab4_mutex.ko thread_num=2 thread_inccnt=50 thread_delay=3
/ # [  390.836755] <----|| This Task is runinng!!!  yeesss
[  391.516219] <----|| The incremention is done!!! yeesss
[  391.519418] <----|| This Task is runinng!!!  yeesss
/ # rmmod lab4_mutex
[  392.202276] <----|| The incremention is done!!! yeesss
[  394.250851] Exit jiffies is 4295061563
[  394.252135] <----|| My Delta time = 4ms !!! yeesss
[  394.252172] <----|| My Delta time = 16ms !!! yeesss
[  394.252453] <----|| My Delta time = 4ms !!! yeesss
[  394.252774] <----|| My Delta time = 9ms !!! yeesss
[  394.252932] <----|| My Delta time = 4ms !!! yeesss
[  394.253411] <----|| My Delta time = 5ms !!! yeesss
[  394.254428] <----|| My Delta time = 5ms !!! yeesss
[  394.254709] <----|| My Delta time = 5ms !!! yeesss
[  394.254970] <----|| My Delta time = 6ms !!! yeesss
[  394.255557] <----|| My Delta time = 5ms !!! yeesss
[  394.257853] <----|| My Delta time = 5ms !!! yeesss
[  394.259321] <----|| My Delta time = 28ms !!! yeesss
[  394.259549] <----|| My Delta time = 14ms !!! yeesss
[  394.259767] <----|| My Delta time = 27ms !!! yeesss
[  394.259960] <----|| My Delta time = 6ms !!! yeesss
[  394.260309] <----|| My Delta time = 4ms !!! yeesss
[  394.260524] <----|| My Delta time = 4ms !!! yeesss
[  394.260794] <----|| My Delta time = 170ms !!! yeesss
[  394.261117] <----|| My Delta time = 5ms !!! yeesss
[  394.261424] <----|| My Delta time = 5ms !!! yeesss
[  394.267000] <----|| My Delta time = 5ms !!! yeesss
[  394.270571] <----|| My Delta time = 21ms !!! yeesss
[  394.270862] <----|| My Delta time = 4ms !!! yeesss
[  394.271394] <----|| My Delta time = 5ms !!! yeesss
[  394.271865] <----|| My Delta time = 5ms !!! yeesss
[  394.272171] <----|| My Delta time = 7ms !!! yeesss
[  394.272435] <----|| My Delta time = 8ms !!! yeesss
[  394.275839] <----|| My Delta time = 5ms !!! yeesss
[  394.275972] <----|| My Delta time = 6ms !!! yeesss
[  394.276508] <----|| My Delta time = 5ms !!! yeesss
[  394.276778] <----|| My Delta time = 5ms !!! yeesss
[  394.278136] <----|| My Delta time = 5ms !!! yeesss
[  394.278449] <----|| My Delta time = 241ms !!! yeesss
[  394.278733] <----|| My Delta time = 5ms !!! yeesss
[  394.278973] <----|| My Delta time = 10ms !!! yeesss
[  394.279429] <----|| My Delta time = 11ms !!! yeesss
[  394.281861] <----|| My Delta time = 16ms !!! yeesss
[  394.282382] <----|| My Delta time = 14ms !!! yeesss
[  394.282977] <----|| My Delta time = 85ms !!! yeesss
[  394.283545] <----|| My Delta time = 21ms !!! yeesss
[  394.284254] <----|| My Delta time = 6ms !!! yeesss
[  394.286555] <----|| My Delta time = 4ms !!! yeesss
[  394.287347] <----|| My Delta time = 4ms !!! yeesss
[  394.287937] <----|| My Delta time = 5ms !!! yeesss
[  394.288863] <----|| My Delta time = 4ms !!! yeesss
[  394.289571] <----|| My Delta time = 4ms !!! yeesss
[  394.290093] global var: 100
[  394.290565] lab4_mutex: exit
[  394.290999] DADADADA Kernel!

```

Без синхронізації:
```
/ # insmod /mnt/lab3_no_sync.ko thread_num=2 thread_inccnt=50 thread_delay=3
/ # [  410.836755] <----|| This Task is runinng!!!  yeesss
[  410.516219] <----|| The incremention is done!!! yeesss
[  410.519418] <----|| This Task is runinng!!!  yeesss
/ # rmmod lab4_mutex
[  412.202276] <----|| The incremention is done!!! yeesss
[  412.250851] Exit jiffies is 4295061563
[  412.252135] <----|| My Delta time = 4ms !!! yeesss
[  412.252172] <----|| My Delta time = 16ms !!! yeesss
[  412.252453] <----|| My Delta time = 4ms !!! yeesss
[  412.252774] <----|| My Delta time = 9ms !!! yeesss
[  412.252932] <----|| My Delta time = 4ms !!! yeesss
[  412.253411] <----|| My Delta time = 5ms !!! yeesss
[  412.254428] <----|| My Delta time = 5ms !!! yeesss
[  412.254709] <----|| My Delta time = 5ms !!! yeesss
[  412.254970] <----|| My Delta time = 6ms !!! yeesss
[  412.255557] <----|| My Delta time = 5ms !!! yeesss
[  412.257853] <----|| My Delta time = 5ms !!! yeesss
[  412.259321] <----|| My Delta time = 28ms !!! yeesss
[  412.259549] <----|| My Delta time = 14ms !!! yeesss
[  412.259767] <----|| My Delta time = 27ms !!! yeesss
[  412.259960] <----|| My Delta time = 6ms !!! yeesss
[  412.260309] <----|| My Delta time = 4ms !!! yeesss
[  412.260524] <----|| My Delta time = 4ms !!! yeesss
[  412.260794] <----|| My Delta time = 170ms !!! yeesss
[  412.261117] <----|| My Delta time = 5ms !!! yeesss
[  412.261424] <----|| My Delta time = 5ms !!! yeesss
[  412.267000] <----|| My Delta time = 5ms !!! yeesss
[  412.267682] <----|| My Delta time = 5ms !!! yeesss
[  412.269772] <----|| My Delta time = 5ms !!! yeesss
[  412.270034] <----|| My Delta time = 10ms !!! yeesss
[  412.270315] <----|| My Delta time = 7ms !!! yeesss
[  412.270571] <----|| My Delta time = 21ms !!! yeesss
[  412.270862] <----|| My Delta time = 4ms !!! yeesss
[  412.271394] <----|| My Delta time = 5ms !!! yeesss
[  412.271865] <----|| My Delta time = 5ms !!! yeesss
[  412.272171] <----|| My Delta time = 7ms !!! yeesss
[  412.278136] <----|| My Delta time = 5ms !!! yeesss
[  412.278449] <----|| My Delta time = 241ms !!! yeesss
[  412.278733] <----|| My Delta time = 5ms !!! yeesss
[  412.278973] <----|| My Delta time = 10ms !!! yeesss
[  412.279429] <----|| My Delta time = 11ms !!! yeesss
[  412.281861] <----|| My Delta time = 16ms !!! yeesss
[  412.282382] <----|| My Delta time = 14ms !!! yeesss
[  412.282977] <----|| My Delta time = 85ms !!! yeesss
[  412.283545] <----|| My Delta time = 21ms !!! yeesss
[  412.284254] <----|| My Delta time = 6ms !!! yeesss
[  412.286555] <----|| My Delta time = 4ms !!! yeesss
[  412.287347] <----|| My Delta time = 4ms !!! yeesss
[  412.287937] <----|| My Delta time = 5ms !!! yeesss
[  412.288863] <----|| My Delta time = 4ms !!! yeesss
[  412.289571] <----|| My Delta time = 4ms !!! yeesss
[  412.290093] global var: 92
[  412.290565] lab4_mutex: exit
[  412.290999] DADADADA Kernel!

```

Власна синхронізація:
```
/ # insmod /mnt/lab4_my_sync.ko thread_num=3 thread_inccnt=100 thread_delay=5
[  113.322554] lab4_my_sync: loading out-of-tree module taints kernel.
[  113.326030] <----|| Task is runinng!
[  113.335163] insmod (95) used greatest stack depth: 13928 bytes left
/ # [  113.937740] <----|| Task is runinng!
[  113.937742] <----|| The incremention is done!
[  114.540770] <----|| The incremention is done!
[  114.540771] <----|| Task is runinng!
[  115.151743] <----|| The incremention is done!
[  115.151746] <----|| Task is runinng!
rmmod lab4_my_sync
[  115.752928] The incremention is done!
[  130.798278] Exit jiffies is 4294797947
[  130.803245] <----|| Delta time = 12ms
[  130.803247] <----|| Delta time = 6ms
...
[  131.080749] <----|| Delta time = 6ms
[  131.081360] <----|| global var: 300
[  131.081988] lab4_my_sync: exit
[  131.083203] Ave Kernel!
/ # 
```
В ході виконання лабараторної роботи було заcвоєно роботу з потоками ядра
