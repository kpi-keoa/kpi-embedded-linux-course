
###  Результати виконання лабораторної роботи №3

Почав виконання роботи я з того що необхідно створити кілька потоків, які будуть асинхронно інкрементувати змінну. У якості джерела інформації я використав дві статті: [перша](http://tuxthink.blogspot.com/2011/02/kernel-thread-creation-1.html) і [друга](https://subscription.packtpub.com/book/application_development/9781785883057/1/ch01lvl1sec13/kernel-threads).


Наступним кроком стало створення зв'язного списку та тестування виконання програми без використання синхронізації. У якості джерел інформації я використовував [stackoverflow](https://stackoverflow.com/questions/33933344/adding-items-to-a-linux-kernel-linked-list?answertab=votes#tab-top) та [kernel.org](https://www.kernel.org/doc/htmldocs/kernel-api/adt.html).


Тепер про виконання програми без використання примітивів синхронізації. Результат на х86-комп'ютері мене не здивував, все як і очікувалося, потоки конкурують між собою і отримані значення глобальної змінної є некоректними:
```
[26348.254578] Hello from module	Запустився модуль
[26348.254692] Hello from thread	Запускаються п'ять потоків
[26348.254693] Hello from thread
[26348.254694] Hello from thread
[26348.254695] Threads started		
[26348.254784] Hello from thread
[26348.254904] Hello from thread
[26350.272144] Thread stopped		Потоки завершають свою роботу
[26350.272156] Thread stopped
[26350.272163] Thread stopped
[26350.272170] Thread stopped
[26350.272178] Thread stopped
[26350.272179] List val is 37883	Вивід даних зі списку, як ви бачите, дані зіпсовані (має бути: 50000 40000 30000 20000 10000)
[26350.272180] List val is 32250
[26350.272181] List val is 22250
[26350.272181] List val is 13139
[26350.272182] List val is 10694
[26350.272182] Final val is 37883	Вивід фінального значення глобальної змінної: має бути 50000, дані зіпсовано
[26350.272183] Long live the Kernel!
```

Дані ж, отримані з Beaglebone Black набагато цікавіші. Чомусь навіть без використання примітивів синхронізації, отримані результати є правильними:
```
[11557.709262] Hello from module
[11557.726235] Threads started

[11557.731160] Hello from thread
[11557.731242] Hello from thread[21842.670703] Hello from module
[21842.671278] Hello from thread
[21842.671280] Hello from thread
[21842.671283] Threads started
[21842.671287] Hello from thread
[21842.671290] Hello from thread
[21842.671328] Hello from thread
[21844.689028] Thread stopped
[21844.689052] Thread stopped
[21844.689061] Thread stopped
[21844.689069] Thread stopped
[21844.689076] Thread stopped
[21844.689077] List val is 50000
[21844.689078] List val is 40000
[21844.689079] List val is 30000
[21844.689081] List val is 20000
[21844.689081] List val is 10000
[21844.689083] Final val is 50000
[21844.689083] Long live the Kernel!

[11557.732839] Hello from thread
[11557.738285] Hello from thread
[11557.749503] Hello from thread

[11560.304960] Thread stopped
[11560.305389] Thread stopped
[11560.305499] Thread stopped
[11560.305575] Thread stopped
[11560.305642] Thread stopped

[11560.305649] List val is 50000
[11560.305655] List val is 40000
[11560.305660] List val is 30000
[11560.305666] List val is 20000
[11560.305670] List val is 10000

[11560.305676] Final val is 50000
[11560.305681] Long live the Kernel!

```

У мене виникло припущення, що причина цього - низька завантаженість системи у момент роботи мого модуля. Тобто ядро дозволяє кожному із потоків коректно відпрацювати необхідний йому час. Або, можливо, причина у тому, що в основі Beaglebone Black лежить одноядерний процесор.

Наступний крок - реалізація примітиву синхронізації **spinlock** із використанням атомарного доступу до змінної в пам'яті. Згідно завдання треба було використовувати архітектурно-залежний код із файлу *asm/atomic.h* і, відповідно, передбачити можливість компіляції на різних платформах, але цей файл виявився абсолютно ідентичним на x86 та ARM - тому не бачу в цьому сенсу. Він все-одно є лише обгорткою над більш низькорівневим файлом *asm/cmpxchg.h*, який у свою чергу є обгорткою над файлом *asm/cmpxchg-local.h*.    
Файл *asm/atomic.h* надає нам операцію наступний атомарний макрос, що дозволяє реалізувати spinlock: **atomic_cmpxchg**. Про його принцип роботи докладно написано [тут](https://lwn.net/Articles/695257/). Макрос **atomic_cmpxchg**, в свою чергу є обгорткою над макросом **cmpxchg**, який є обгорткою над **cmpxchg_local**, який є обгорткою над функцією **__cmpxchg_local_generic**.   
Розписувати його принцип роботи я не бачу сенсу, можу розповісти про нього на захисті лабораторної.  


Реалізація spinlock дала свої плоди: на Beaglebone нічого не змінилося, бо там і без синхронізації не спостерігалося жодних data races, а от на x86 програма почала працювати як потрібно:
```
[21842.670703] Hello from module
[21842.671278] Hello from thread
[21842.671280] Hello from thread
[21842.671283] Threads started
[21842.671287] Hello from thread
[21842.671290] Hello from thread
[21842.671328] Hello from thread
[21844.689028] Thread stopped
[21844.689052] Thread stopped
[21844.689061] Thread stopped
[21844.689069] Thread stopped
[21844.689076] Thread stopped
[21844.689077] List val is 50000
[21844.689078] List val is 40000
[21844.689079] List val is 30000
[21844.689081] List val is 20000
[21844.689081] List val is 10000
[21844.689083] Final val is 50000
[21844.689083] Long live the Kernel!

```




