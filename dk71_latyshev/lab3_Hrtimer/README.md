
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
Оскільки вихідний таймер був відносно досконалий, щоб уникнути великих змін, розробники ядра ​​переробили архітектуру програмного забезпечення для високоточного таймера.`

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
/ # insmod /mnt/mymod.ko
[  135.706478] mymod: loading out-of-tree module taints kernel.
[  135.748413] Invalid cnt value has to be(unsigned)cnt > 0
insmod: can't insert '/mnt/mymod.ko': invalid parameter
[  135.836648] insmod (93) used greatest stack depth: 13928 bytes left

```

```
/ # insmod /mnt/lab3_task.ko
[  197.924241] lab3_task: loading out-of-tree module taints kernel.
[  197.927837] Invalid <cnt> <= 0 :(
insmod: can't insert '/mnt/lab3_task.ko': invalid parameter
[  197.947408] insmod (94) used greatest stack depth: 13928 bytes left
/ # insmod /mnt/mymod.ko cnt=6 delay=6
[  276.978523] Init>> jiffies is 4294944291
[  276.980598] tasklet>> jiffies = 4294944294

/ # rmmod mymod.ko 
[  320.342030] exit>> tim_cnt: 6
[  320.342074] exit>> countrer number 0 has the 4294944299jiffies val
[  320.342620] exit>> countrer number 1 has the 4294944305jiffies val
[  320.342843] exit>> countrer number 2 has the 4294944312jiffies val
[  320.344292] exit>> countrer number 3 has the 4294944317jiffies val
[  320.344943] exit>> countrer number 4 has the 4294944325jiffies val
[  320.345304] exit>> countrer number 5 has the 4294944332jiffies val
[  320.345884] exit>> dealoc: array is deallocated
[  320.345884] exit>> god save the Kernel!; jiffies = 4294987658




/ # insmod /mnt/mymod.ko cnt=11 delay=11
[  381.223822] Init>> jiffies is 4295048536
[  381.227225] tasklet>> jiffies = 4295048540
/ # rmmod mymod.ko 
[  384.745309] exit>> tim_cnt: 11
[  384.745352] exit>> countrer number 0 has the 4295048551jiffies val
[  384.746326] exit>> countrer number 1 has the 4295048563jiffies val
[  384.746585] exit>> countrer number 2 has the 4295048574jiffies val
[  384.747079] exit>> countrer number 3 has the 4295048586jiffies val
[  384.747079] exit>> countrer number 4 has the 4295048597jiffies val
[  384.747079] exit>> countrer number 5 has the 4295048608jiffies val
[  384.747079] exit>> countrer number 6 has the 4295048623jiffies val
[  384.747079] exit>> countrer number 7 has the 4295048629jiffies val
[  384.747079] exit>> countrer number 8 has the 4295048648jiffies val
[  384.747079] exit>> countrer number 9 has the 4295048653jiffies val
[  384.752117] exit>> countrer number 10 has the 4295048789jiffies val
[  384.753153] exit>> dealoc: array is deallocated
[  384.753453] exit>> god save the Kernel!; jiffies = 4295052066

```

В даній роботі було використано високоточні таймери, які гарно працюють на ріхних архітектурах, як на 32бітних так і на 64. 
Головна перевага даного `hrtimer` в тому що він може рахувати час в наносекундах, що корисно для систем реального часу.
