
###  Результати виконання лабораторної роботи №2


Я працював із платою **Beaglebone Black Rev. C**, на якій було встановлено дистрибутив **Debian Stretch GNU/Linux**, із версією ядра, наведеною у наступному лістингу:

```
debian@beaglebone:~$ uname -r
4.14.71-ti-r80
```
<br/>
Після цього були знайдені та завантажені необхідні Linux headers для ядра на моїй платі:


```
Package: linux-headers-4.14.71-ti-r80
Source: linux-upstream
Version: 1stretch
Architecture: armhf
Maintainer: rcn-ee <robertcnelson@gmail.com>
Installed-Size: 75918
Homepage: http://www.kernel.org/
Priority: optional
Section: kernel
Filename: pool/main/l/linux-upstream/linux-headers-4.14.71-ti-r80_1stretch_armhf.deb
Size: 11211528
SHA256: 8d14a6c9919463fe608a680d95b7d0b52f1b9a5042069427de6f7c2c8c7e4293
SHA1: 8a5e924db5e331a8be71a5b37521bcb3722f8cf5
MD5sum: a3b4f1e7a2ba1c7fa66f686459f29077
Description: Linux kernel headers for 4.14.71-ti-r80 on armhf
This package provides kernel header files for 4.14.71-ti-r80 on armhf.
This is useful for people who need to build external modules
```
 
 За допомогою утиліти _wget_ я завантажив архів із заголовками ядра та перевірив контрольну суму утилітою _sha256sum_:
 
 ```
 debian@beaglebone:~$ wget repos.rcn-ee.com/debian/pool/main/l/linux-upstream/linux-headers-4.14.71-ti-r80_1stretch_armhf.deb
--2019-09-16 17:02:42--  http://repos.rcn-ee.com/debian/pool/main/l/linux-upstream/linux-headers-4.14.71-ti-r80_1stretch_armhf.deb
Resolving repos.rcn-ee.com (repos.rcn-ee.com)... 45.33.2.10, 2600:3c00::f03c:91ff:fe37:6ad5
Connecting to repos.rcn-ee.com (repos.rcn-ee.com)|45.33.2.10|:80... connected.
HTTP request sent, awaiting response... 200 OK
Length: 11211528 (11M) [application/x-debian-package]
Saving to: ‘linux-headers-4.14.71-ti-r80_1stretch_armhf.deb’

linux-headers-4.14.71-ti-r80_1stretch_arm 100%[=====================================================================================>]  10.69M   382KB/s    in 66s     

2019-09-16 17:03:49 (165 KB/s) - ‘linux-headers-4.14.71-ti-r80_1stretch_armhf.deb’ saved [11211528/11211528]

debian@beaglebone:~$ sha256sum linux-headers-4.14.71-ti-r80_1stretch_armhf.deb 
8d14a6c9919463fe608a680d95b7d0b52f1b9a5042069427de6f7c2c8c7e4293  linux-headers-4.14.71-ti-r80_1stretch_armhf.deb
 ```
<br/>
Далі я зібрав тестовий модуль ядра на Beaglebone Black. після цього я взявся виконувати завдання лабораторної роботи та вирішив що сидіти з секундоміром як мавпочка - це не серйозно і подивився як з Kernel space  можна дізнатися поточний час у секундах. Це можливо засобами бібліотеки **linux/ktime.h**, а саме функцією 

```void getnstimeofday(struct timespec *ts);```
<br/>
Модифікований файл з сирцевим кодом модуля знаходиться у директорії _src/firstmod.c_.
Отримав наступні результати:
```
root@beaglebone:/home/debian/lab2# insmod firstmod.ko
root@beaglebone:/home/debian/lab2# dmesg 
[ 5945.449859] Hello, $username!
               jiffies = 1411283
               seconds = 1568657424
root@beaglebone:/home/debian/lab2# rmmod firstmod
root@beaglebone:/home/debian/lab2# insmod firstmod.ko
root@beaglebone:/home/debian/lab2# dmesg 
[ 5945.449859] Hello, $username!
               jiffies = 1411283
               seconds = 1568657424
[ 5964.071288] Long live the Kernel!
[ 5970.771086] Hello, $username!
               jiffies = 1417613
               seconds = 1568657449
```
<br/>
Підрахуємо скільки системних "тіків" система робить за секунду:

```
(1417613 - 1411283) / (1568657449 - 1568657424) = 6330 / 25 = 253 ticks/second
```
Тобто, значення одного _jiffies_ буде рівним:
```
1 / 253 = 3.95 мс
```
<br/>
Тепер давайте подивимося на теоретичне значення ticks/second:   

```
debian@beaglebone:~$ grep 'CONFIG_HZ=' /boot/config-$(uname -r)
CONFIG_HZ=250
```
Воно співпадає із реально розрахованим значенням у рамках допустимої похибки.
<br/>

Далі порівняємо цей результат із отриманим на системі x86. Я не бачу сенсу повторювати всі ці дії ще раз - тому скористаюся результатами своїх колег.
У якості джерела я взяв [звіт до лабораторної роботи №2 Якименка Олега](https://github.com/olegovich22/kpi-embedded-linux-course/blob/master/dk61_yakymenko/lab2_simplest_kernel_modules/README.rst).
Цікаво, що його результати не співпали із моїми через те, що у нас можуть бути різні версії дистрибутиву із різними налаштуваннями частоти системного "тіку".

Частота системного тіку у мене виявилася ідентичною системі х86. Некоректно говорити що частота залежить від системи, це значення визначається значенням константи **CONFIG_HZ**.

Є різниця у значенні _jiffies_, що виводиться при ініціалізації модуля та під час виконання функції-хендлера першого тасклету у тому що між цими подіями є певний час, за який значення змінної _jiffies_ встигає збільшитись.
