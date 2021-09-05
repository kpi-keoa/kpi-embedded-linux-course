
Лабораторна робота №3
====
Тема: Робота з високотончими таймерами
====
Завдання:
===
  - Замінити `timer` на `hrtimer`;
   

Хід роботи:
====

Високоточні таймери забезпечують точність синхронізації на рівні наносекунд, щоб задовольнити гостру потребу в програмах точного часу або драйверах ядра. 
Оскільки вихідний таймер був відносно досконалий, щоб уникнути великих змін, розробники ядра ​​переробили архітектуру програмного забезпечення для високоточного таймера.

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

В ході виконання лабараторної роботи було розроблено простий модуль ядра, який використовує вискоточні таймери та тасклети. Було випробувано різні режими запуску (у вікні qemu або у власному терміналі)
Було отримано такі дані при не вказуванні параметру:

```
/ # insmod /mnt/lab3_task.ko
[  111.516141] lab2_task: loading out-of-tree module taints kernel.
[  111.517882] Invalid <cnt> <= 0 :(
insmod: can't in[  111.535961] insmod (94) used greatest stack depth: 14555 bytt
sert '/mnt/lab3_task.ko': invalid parameter
```

```
/ # insmod /mnt/lab3_task.ko
[  197.924241] lab3_task: loading out-of-tree module taints kernel.
[  197.927837] Invalid <cnt> <= 0 :(
insmod: can't insert '/mnt/lab3_task.ko': invalid parameter
[  197.947408] insmod (94) used greatest stack depth: 13928 bytes left
/ # insmod /mnt/lab3_task.ko cnt=5 delay=5
[  234.728081] 5 msec is 5 jiffies
[  234.730518] Init jiffies is 4294901883
[  234.732476] Tasklet jiffies 4294901885
/ # [  234.737511] Iteration 0
[  234.742505] Iteration 1
[  234.747504] Iteration 2
[  234.752539] Iteration 3
[  234.757534] Iteration 4
rmmod lab3_task
[  329.135933] Exit jiffies is 4294996289
[  329.138044] Array[0] = 4294901891
[  329.139814] Array[1] = 4294901896
[  329.141508] Array[2] = 4294901901
[  329.143056] Array[3] = 4294901907
[  329.144607] Array[4] = 4294901911
[  329.146160] Ave Kernel!



/ # insmod /mnt/lab3_task.ko cnt=10 delay=10
[  494.211723] 10 msec is 10 jiffies
[  494.213999] Init jiffies is 4295161367
[  494.216369] Tasklet jiffies 4295161369
/ # [  494.226428] Iteration 0
[  494.236411] Iteration 1
[  494.246512] Iteration 2
[  494.256444] Iteration 3
[  494.266446] Iteration 4
[  494.276475] Iteration 5
[  494.286447] Iteration 6
[  494.296429] Iteration 7
[  494.306439] Iteration 8
[  494.316430] Iteration 9
rmmod lab3_task
[  503.626206] Exit jiffies is 4295170779
[  503.629660] Array[0] = 4295161380
[  503.632759] Array[1] = 4295161390
[  503.634356] Array[2] = 4295161399
[  503.635047] Array[3] = 4295161409
[  503.635720] Array[4] = 4295161419
[  503.636657] Array[5] = 4295161429
[  503.637632] Array[6] = 4295161439
[  503.638621] Array[7] = 4295161449
[  503.639561] Array[8] = 4295161459
[  503.640482] Array[9] = 4295161469
[  503.641405] Ave Kernel!
/ # 
```

В даній роботі було використано високоточні таймери, які гарно працюють на ріхних архітектурах, як на 32бітних так і на 64. Головна перевага даного `hrtimer` в тому що він може рахувати час в наносекундах, що корисно для систем реального часу.
