=============================================
Лабораторная работа №5-6
=============================================

Цель: Необходимо дополнить полученный scull модуль "hivemod.c".
Основные задачи:
        - Реализовать работу с буфером чтения/записи
        - Добавить необходимые функции для инициализации символьного устройства в /dev
        - Использовать более оптимальный тип данных. (взамен klist)
        - Реализовать метод ioctl.Он должен изменить размер имеющего буфера и присоединить имеющуюся строку к тому же буферу.

Выполнение
-----
Первой решенной задачей было добавление символьного устройства в директорию `/dev/`. Для этого необходимо добавить символьное устройво в device tree.Для этой задачи было использовано функции `class_create(...)` и `device_create(...)`. 


Для поблочного отображения из пространства пользователя в пространство ядра и наоборот существуют функции `copy_from_user` и `copy_to_user'. На их основе было реализовано основные операции символьного устройсва - `read()` и `write()`.
Также задачу записи возможно решить с помощью функции put_user(...), которая делает то же самое, но побайтно. Поблочное копирование было принято как более оптимальное - размеры блоков должны быть кратны 2 и скорость записи/чтения должно расти с ростом размером таких блоков. В обоих методах выполняются необходимые проверки сравнения длины входных с имеющимся буфером и выполняется соотвественные процедуры чтения/записи.

Базовая логика метода write()
-----
	.. code-block:: C

                        if( item->length <=  *loff ) {
                                ret_status = 0;
                                goto wr_error;
                        }
                        
                        if ( (item->length - (size_t)*loff ) < size) {
                                ret_status = -ENOSPC;
                                goto wr_error;
                        }
                        
                        if (copy_from_user(item->buffer + (size_t)*loff, buf, size)) {
                                ret_status = -EFAULT;
                                printk(KERN_DEBUG "User buffer is empty or queried offset if too small: %ld\n", ret_status);
                                goto wr_error;
                        }
                        
                        ret_status = size;
                        *loff += ret_status;       

        
Метод ioctl необходим для задания допольнительных операций для работы с устройстом. В данном случае метод реализован для изменения размера буффера имеющихся данных (флаг BUFFER_REALLOC) и добавления в конец того же буфера условной строки (флаг ADD_MAGIC_PHRASE).Для того чтобы присоединить magic_phrase к буфферу производятся операции реаллокации памяти в большую сторону с необходимыми копированями и освобождениями памяти. Логику можно увидеть ниже.

Для инициализации этих флагов необходимы следующие макросы:
-----
	.. code-block:: C

                        #define BUFFER_REALLOC _IOR('C', 1, unsigned long*)
                        #define ADD_MAGIC_PHRASE _IO('H', 2)

Эти же макросы должны быть объявлены там где будет вызываться этот метод.Например в программе в пользовательском пространстве.
                        
Базовая логика метода ioctl()
-----
	.. code-block:: C
	
                        switch(ioctl_cmd) {
                                case BUFFER_REALLOC:
                                        new_buffer_size = arg;
                                        if (item->length < new_buffer_size ) 
                                                break;
                                        if( NULL == krealloc(item->buffer, new_buffer_size, GFP_KERNEL) ) {
                                                printk(KERN_DEBUG "Cannot reallocate item buffer \n");
                                                return -EBADF;
                                        }
                                        item->length = new_buffer_size;
                                        printk(KERN_DEBUG, "Buffer size has been changed \n");
                                break;
                                case ADD_MAGIC_PHRASE:
                                        new_buffer_size = item->length + sizeof(magic_phrase);
                                        
                                        char *tmpbuff = vmalloc(new_buffer_size);
                                        if (NULL == tmpbuff) {
                                                printk(KERN_DEBUG "Cannot allocate temp char buffer \n");
                                                return -EBADF;
                                        }
                                        memcpy(tmpbuff, item->buffer, item->length);
                                        vfree(item->buffer);
                                        
                                        /* append phrase to an increased buffer */
                                        strncat(tmpbuff, magic_phrase, new_buffer_size);
                                        
                                        if( NULL == (item->buffer = vmalloc(new_buffer_size) )) {
                                                printk(KERN_DEBUG "Cannot reallocate a new buffer \n");
                                                return -EBADF;
                                        }
                                        
                                        memcpy(item->buffer, tmpbuff, new_buffer_size);
                                        
                                        item->length += sizeof(magic_phrase);
                                        file->f_pos += sizeof(magic_phrase);
                                break;

                                
Зачастую при работе с файлом может потребоваться установить определенное положение каретки относительно начала файла. Для этого необхим метод lseek(...), в который необходимо будет указать положение и соотвествующий файловый дескриптор.
Данный метод имеет возможность устанавливать определенное положение в файле и становиться в конец файла. Для этого в роли аргументов используются флаги `SEEK_SET` и `SEEK_END`.


Базовая логика метода lseek()
------
	.. code-block:: C
	
			switch (flag) {
				case SEEK_SET:
					seek = (f_offset < 0) ? 0 : f_offset;
			break;
				case SEEK_END:
					seek = item->length;
			break;
			}
	

Узнать список всех возможных операций (методов) для работы с модулем ядра можно найти в самом модуле ядра.
Для разных устройств будет своё колличество этих методов.Этот список представляет из себя структуру, содержащую набор указателей на определенные раннее функции.
        
        
Доступные методы для работы с символьным устройством
------
	.. code-block:: C
	
			static struct file_operations hive_fops = {
				.owner =   THIS_MODULE,
				.llseek =  &cdev_lseek,
				.open =    &cdev_open,
				.release = &cdev_release,
				.read =    &cdev_read,
				.write =   &cdev_write,
				.unlocked_ioctl = &cdev_ioctl
			};

                        
                        
Базово для поиска имеющегося файла использовался циклический список klist с линейной сложностью O(n). 
Для решения данной задачи этот тип данных не есть максимально эффективным.Следуя поставленной задачи и с целью оптимизации был выбран другой тип данных для работы с символьным устройством - хэш таблицы.
В роли ключа для поиска необходимого указателя на файл является сам файл. Следовательно отпадает необходимость хранить файловый дескриптор в ноде.
В действительности, каждый сегмент hlist table хранит в себе собственный связанный список.По каждому из них можно пройти и изъять/внести необходиммые данные.
Данная особенность не была использована по причине ненадобности и в каждом сегменте таблицы хранится файловый дескриптор, который можно найти по ключу.
Сложность такого поиска будет составлять O(1) в идеальном случае.

        
Интерпретация работы модуля


Операции записи/считывания
------

                Open op
                
                Write op: Honey, can you please cum to me?
                
                Teleport to start <- lseek(0)
                
                Read op: Honey, can you please cum to me?
                
                Close op

                        
Операция ioctr append
------

                Open op

                Write op: Honey, can you please cum to me?
                
                appended: Honey, can you please cum to me?Wow, we made these bees TWERK ! 
                
                Close op


Выводы
------
Изучены основные подходы в работе с символьными устройствами.Разобран принцип работы с буферами и указателями (lseek). Изучен новый тип данных ядра - hlist. Полученные знания были закреплены на практике.
