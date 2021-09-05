==========================================================
**Лабораторна робота №6. символьний пристрій**
==========================================================


**Завдання:**
~~~~~~~~~~~~~
   
— Напишите свое символьное устройство с ioctl. Оно должно делать что-то полезное. Например, предоставлять шареную память, или генерировать что-то, или рассчитывать. Например, считать количество вхождений определённого символа или сортировать строки или считать интеграл        
      

**Хід роботи:**
~~~~~~~~~~~~~~~
В ході роботи над власнтм символьним проистоєм було вирішено зробити символьний пристрій який шифрує символьну строку за допомогою xor кодування та вертає закодовану строку. Було реалізовано наступні fops: write read devopen devreleas ioctrl додаткові можливості введіні за допомогою ioctrl включають в себе зміну розміру буфферу строки та ключа та вибір буферу в який відбувається запис. для тестування роботи була написана простенька userspace програма яка відкриває файловий дескриптор нашого модулю записує почергово строку для кодування та ключ зчитує та демострує результат кодування та потім ще раз проганяє результат кодування з тим самим ключем щоб декодувати закодовану строку та впевнитись що все працює як треба
Тест модуля:   

.. code-block:: bash
	
	/ # insmod mnt/xoritd.ko && cd /dev && mknod xorit c 248 0 && mdev -s && cd ..
	[   11.810622] xoritd: loading out-of-tree module taints kernel.
	[   11.848828] xoritd: Static dev 248:0 created
	/ # /mnt/usrxor wow 1
	[   30.934361] xoritd: New file entry (____ptrval____) created
	[   30.935576] search: found (____ptrval____) val
	[   30.936324] cdev_write>> checking the buffers of the structure hrere
	[   30.936324] dat_b_sz = 100
	[   30.936324] key_b_sz = 100
	[   30.936324] dat_lenth = 13
	[   30.936324] key_lenth = 0
	[   30.936324] mode = 1(0 = KEY)
	[   30.938846] w 
	[   30.938882] 0 
	[   30.939086] o 
	[   30.939279] 1 
	[   30.939475] w 
	[   30.939596] 2 
	[   30.940205] cdev_write>> wow data str that goes into the struct
	[   30.940439] cdev_write>>
	[   30.940439]  read = 3
	[   30.941676] search: found (____ptrval____) val
	[   30.943207] search: found (____ptrval____) val
	[   30.943869] cdev_write>> checking the buffers of the structure hrere
	[   30.943869] dat_b_sz = 100
	[   30.943869] key_b_sz = 100
	[   30.943869] dat_lenth = 3
	[   30.943869] key_lenth = 0
	[   30.943869] mode = 0(0 = KEY)
	[   30.946109] 1 
	[   30.946119] 0 
	[   30.946447] cdev_write>> 1 key str that goes into the struct
	[   30.946634] cdev_write>>
	[   30.946634]  read = 1
	[   30.947467] search: found (____ptrval____) val
	[   30.948643] cdev_write>> checking the buffers of the structure hrere
	[   30.948643] dat_b_sz = 100
	[   30.948643] key_b_sz = 100
	[   30.948643] dat_lenth = 3
	[   30.948643] key_lenth = 1
	[   30.948643] mode = 0(0 = KEY)
	[   30.964484] search: found (____ptrval____) val
	 F^F
	[   30.967773] search: found (____ptrval____) val
	[   30.968260] cdev_write>> checking the buffers of the structure hrere
	[   30.968260] dat_b_sz = 100
	[   30.968260] key_b_sz = 100
	[   30.968260] dat_lenth = 3
	[   30.968260] key_lenth = 1
	[   30.968260] mode = 1(0 = KEY)
	[   30.970582] F 
	[   30.970601] 0 
	[   30.970995] ^ 
	[   30.971261] 1 
	[   30.971450] F 
	[   30.971632] 2 
	[   30.971971] cdev_write>> F^F data str that goes into the struct
	[   30.972274] cdev_write>>
	[   30.972274]  read = 3
	[   30.973249] search: found (____ptrval____) val
	[   30.974581] cdev_write>> checking the buffers of the structure hrere
	[   30.974581] dat_b_sz = 100
	[   30.974581] key_b_sz = 100
	[   30.974581] dat_lenth = 3
	[   30.974581] key_lenth = 1
	[   30.974581] mode = 1(0 = KEY)
	 wow
	[   30.977836] search: found (____ptrval____) val
	[   30.979099] xoritd: File entry (____ptrval____) unlinked
	[   30.985227] usrxor (101) used greatest stack depth: 13280 bytes left
	мінімальний вивід данних логу:
	/ # /mnt/usrxor test 1qwe
	 E
 	test
	/ # /mnt/usrxor hi_how_are_you TEST
	 <,
	   <;2
	      5& 
	         -;0
	 hi_how_are_you
	/ # 

Висновки
~~~~~~~~
В результаті виконання лабораторної роботи було створено модуль ядра що відповідає поставленій задачі роботу модулю було протестовано та відлагодженно результати тестування вказані вище, під час першого тесту було використано максимальний рівень логінгу ядра а під час наступних 4-ий.

