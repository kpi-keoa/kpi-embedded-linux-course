==========================================================================
**Лабораторна робота №2 "Засоби відкладеної роботи: таймери та тасклети"**
==========================================================================

**Завдання:**
~~~~~~~~~~~~~
Написати модуль ядра, який:
        * Приймає два параметра: *cnt* і *delay*.
                - *cnt* - кількість циклів, які має відпрацювати таймер.
	        - *delay* - затримка між двома спрацьовуваннями таймера.
                - Модуль повинен відпрацьовувати при *cnt* та *delay* рівних нулю.
	* Спочатку *init* друкує поточне значення *jiffies* в лог ядра.
	* Далі запускає тасклет, який повинен надрукувати своє значення *jiffies* в лог ядра.
	* Потім виділяє масив розміру *cnt*, використовуючи динамічну алокацію. В цей масив таймер буде записувати значення.
	* *init* запускає таймер з затримкою *delay* і функція завершується.
		- При спрацюванні таймер кладе поточне значення *jiffies* до масиву і перезапускається з затримкою *delay*.
		- Загальна кількість разів запуску таймера дорівнює *cnt*.
	* В *exit* модуль повинен надрукувати поточне значення *jiffies* і вивести всі значення з масиву.
		- Якщо *exit* був викликаний до того, як таймер встиг відпрацювати *cnt* разів, необхідно скасувати виконання запланованого запуску таймера, вивести в лог ядра повідомлення про дострокове вивантаження модуля і надрукувати в лог ті елементи масиву, які встиг заповнити таймер.

**Хід роботи:**
~~~~~~~~~~~~~~~

	* Ініціалізація параметрів модуля *cnt* і *delay*
		-`module_param()`
    	* Декралація тасклера та таймера і їх функцій-обробників:
		- Функція-обробник тасклета виводить поточне значення *jiffies*.
		- Функція-обробник таймера записує поточне значення *jiffies* в масив. Викликається рекурсивно із періодом *delay*, поки кількість викликів не досягне *cnt*, або модуль не буде вивантажено.
	* Опис функції *init*, яка виконуєтсься при завантаженні модуля.
	 	- Виведення поточного значення *jiffies*.
		- Виклик тасклета.
		- Динамічне виділення пам'яті для масиву.
		- Виклик таймера.
		- Якщо *cnt* рівний нулю, то про це виводиться сповіщення. Модуль завершує роботу.
		- Якщо *delay* рівна нулю, то викликається функція-обробник таймера, яка буде викликана ще *cnt - 1* разів з нульовою затримкою. Тоді всі значення *jiffies* в масиві будуть майже однакові, оскільки таймер буде перезавантажуватися без затримки.
	* Опис функції *exit*, яка виконуєтсься при вивантаженні модуля.
		- Виведення поточного значення *jiffies*.
		- Деалокація ресурсів: тасклет - `tasklet_kill()`, таймер - `del_timer()`, масив - `kfree()`. Якщо модуль був вивантажений до закінчення лічби таймера, про це виводиться сповіщення `printk(KERN_WARNING "Exit: Timer kill\n");`.
		- Виведення значення масиву.

**Результат:**
~~~~~~~~~~~~~~

.. code-block:: bash

	Запуск без параметрів (cnt = 2; delay = 200)

	/ # insmod /mnt/mod_for_lab2.ko
	[   67.682029] mod_for_lab2: loading out-of-tree module taints kernel.
	[   67.682029] Init: Jiffies = 4294734831
	[   67.682029] Tasklet: Jiffies = 4294734831
	[   67.691982] insmod (94) used greatest stack depth: 13496 bytes left
	/ # rmmod mod_for_lab2.ko
	[  108.109779] Exit: Jiffies= 4294775251
	[  108.109939] Exit: array_of_jiffies[0] jiffies= 4294737754
	[  108.109939] Exit: array_of_jiffies[1] jiffies= 4294739011
	/ #

	Запуск із параметрами cnt = 10; delay = 50

	/ # insmod /mnt/mod_for_lab2.ko cnt=10 delay=50
	[  530.779892] Init: Jiffies = 4295197921
	[  530.780396] Tasklet: Jiffies = 4295197922
	/ # rmmod mod_for_lab2.ko
	[  538.409148] Exit: Jiffies= 4295205550
	[  538.409148] Exit: array_of_jiffies[0] jiffies= 4295197973
	[  538.409148] Exit: array_of_jiffies[1] jiffies= 4295198024
	[  538.409148] Exit: array_of_jiffies[2] jiffies= 4295198076
	[  538.409148] Exit: array_of_jiffies[3] jiffies= 4295198128
	[  538.409148] Exit: array_of_jiffies[4] jiffies= 4295198181
	[  538.409148] Exit: array_of_jiffies[5] jiffies= 4295198234
	[  538.409148] Exit: array_of_jiffies[6] jiffies= 4295198286
	[  538.409148] Exit: array_of_jiffies[7] jiffies= 4295198338
	[  538.409148] Exit: array_of_jiffies[8] jiffies= 4295198390
	[  538.409148] Exit: array_of_jiffies[9] jiffies= 4295198442
	/ #

	Якщо cnt = 0. Друкує попередження в лог ядра `printk(KERN_WARNING "Init: cnt == 0\n");`.
	Не створюється масив і не запускається таймер.

	/ # insmod /mnt/mod_for_lab2.ko cnt=0 delay=50
	[  792.106076] Init: Jiffies = 4295459248
	[  792.106290] Init: cnt == 0
	[  792.106416] Tasklet: Jiffies = 4295459248
	/ # rmmod mod_for_lab2.ko
	[  803.795470] Exit: Jiffies= 4295470937
	/ #

	Якщо delay = 0. Друкує попередження в лог ядра `printk(KERN_WARNING "Init: delay = 0\n");`.

	/ # insmod /mnt/mod_for_lab2.ko cnt=10 delay=0
	[  691.857228] Init: Jiffies = 4295359003
	[  691.857572] Tasklet: Jiffies = 4295359004
	[  691.857723] Init: delay = 0
	/ # rmmod mod_for_lab2.ko
	[  694.941446] Exit: Jiffies= 4295362087
	[  694.941974] Exit: array_of_jiffies[0] jiffies= 4295359006
	[  694.942143] Exit: array_of_jiffies[1] jiffies= 4295359007
	[  694.942280] Exit: array_of_jiffies[2] jiffies= 4295359009
	[  694.943022] Exit: array_of_jiffies[3] jiffies= 4295359010
	[  694.943747] Exit: array_of_jiffies[4] jiffies= 4295359012
	[  694.943924] Exit: array_of_jiffies[5] jiffies= 4295359014
	[  694.944038] Exit: array_of_jiffies[6] jiffies= 4295359015
	[  694.944169] Exit: array_of_jiffies[7] jiffies= 4295359016
	[  694.944355] Exit: array_of_jiffies[8] jiffies= 4295359017
	[  694.944516] Exit: array_of_jiffies[9] jiffies= 4295359019
	/ #

	Якщо модуль вивантажити до закінчення всіх відліків таймеру.

	/ # insmod /mnt/mod_for_lab2.ko cnt=10 delay=1000
	[  800.208502] Init: Jiffies = 4295467354
	[  800.208937] Tasklet: Jiffies = 4295467355
	/ # rmmod mod_for_lab2.ko
	[  805.039335] Exit: Jiffies= 4295472185
	[  805.039335] Exit: Timer kill
	[  805.039335] Exit: array_of_jiffies[0] jiffies= 4295468417
	[  805.039335] Exit: array_of_jiffies[1] jiffies= 4295469442
	[  805.039335] Exit: array_of_jiffies[2] jiffies= 4295470575
	[  805.039335] Exit: array_of_jiffies[3] jiffies= 4295471652
	/ #

**Відповіді на запитання:**
~~~~~~~~~~~~~~~~~~~~~~~~~~~

	* Значення *jiffies*, що виводиться із *init* не відрізняється від значення *jiffies*, що виводиться тасклетом, якщо ядро не навантажено. Тобто тасклет виконується, не очікуючи наступного такту. Різниця значень *jiffies* може бути рівна одиниці, якщо ядро навантежене. В такому разі ядро вже буде очікувати наступний такт. Значення *jiffies* відрізняється на 2, якщо ядро навантажене і перед цим відбулося перемикання контексту.
	* В даній роботі був використаний GFP-прапор GFP_KERNEL, оскільки модуль не є критичним до часу виконання. Ядро може призупинити виконання модуля для очікування виділення вільної пам'яті.
	* Різниця *jiffies* між двома запусками таймера рівна затримці *delay* у мілісекундах, оскільки частота тактів системного таймера рівна 1000 Гц а також затримці, яка виникає в результаті роботи ядра до наступного виклику таймера.
	* При затримці *delay* рівній нулю всі значення *jiffies* в масиві будуть майже однакові, оскільки таймер буде перезавантажуватися без затримки. Затримку буде формувати час роботи ядра.
