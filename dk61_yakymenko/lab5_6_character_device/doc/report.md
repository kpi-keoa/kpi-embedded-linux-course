# Звіт з виконання лабораторної роботи №5-6
## Завдання
Модифікувати приклад символьного пристрою по факту виконати 7 TODO
- Додати ініціалізацію памяті нулями при динамічногму виділенні
- Додати механізми читання та запису
- Додати створення пристрою в папці /dev
- Додати операції ioctl, які змінюють розмір буфера та записують магічну фразу в буфер

## Хід роботи
1. Найпростішим було пофіксити *// TODO: fix to make it zero'ed first*. Для цього я замість функції *kmalloc* викликав функцію *kzalloc*. Ці дві функції є ідентичними - єдина різниця тільки в тому, що остання заповнює виділену память нулями
2. Наступним кроком було запровадження механізму створення пристрою у папці /dev, щоб було просто взаємодіяти з модулем.
Для цього спершу нам потрібний обєкт класу
`struct class *hive_class = NULL;`
Далі при ініціалізації потрібно створити клас та девайс
```c
    hive_class = class_create(THIS_MODULE, "hivemod_class");
    device_create(hive_class, NULL, hive_dev, NULL, "hivemod");
```
Після цих строчок коду девайс в папці /dev створиться.
Для видалення потрібно викликати наступні строки, які відповідно видаляють і девайс і клас
```c
    device_destroy(hive_class, hive_dev);
    сlass_destroy(hive_class);
```
3. В модулі, що було надано у якості структури даних, що зберігає обєкти структур кожного відкритого файлу є звязний список. Це викликає дуже великі проблеми, бо пошук в такому разі є **О(n)**, що є критично, оскільки операція пошуку структури виконується при кожній операції з модулем.
Для вирішення цієї проблеми список був замінений на червоно чорне дерево. Це дерево має функцію автобалансування, та швидкість пошуку є **O(log n)**, що є дуже швидким результатом. Весь код можете переглянути в папці /src

4. Наступним кроком є імплеметація читання та запису. Основна проблема в цих функціях є те, що потрібно якимось чином прокидати дані з буферу кернел спейс в юзер спейс і навпаки. Для вирішення цієї проблеми є чарівні функції 
```c
copy_from_user(item->buffer, buf, count);
copy_to_user(buf, item->buffer, count);
```
Тому вигляд функцій читання та запису мають такий вигляд

```c
static ssize_t cdev_read(struct file *file, char __user *buf, 
			 size_t count, loff_t *loff)
{
	struct hive_ftree_item *item = hive_ftree_get(&tree, file);
	if (NULL == item)
		return -EBADF;
	
	if(item->length < count)
		count = item->length;
	
	copy_to_user(buf, item->buffer, count);
	MOD_DEBUG(KERN_DEBUG, "read() - have read %i bytes", count);
	return count;
}

static ssize_t cdev_write(struct file *file, const char __user *buf,
			  size_t count, loff_t *loff)
{
	struct hive_ftree_item *item = hive_ftree_get(&tree, file);
	if (NULL == item)
		return -EBADF;
		
	if(item->length < count)
		count = item->length;
	
	copy_from_user(item->buffer, buf, count);	
	MOD_DEBUG(KERN_DEBUG, "write() - have written %i bytes", count);
	return count;
}
```

5. Останні кроком при виконанні роботи було впровадження двох команд ioctl. Перша це зміна розміру буферу конкретного файлового дескриптора. Інша це просто запис у буфер магічної фрази.
Для реалізації спершу потрібно вкащати вказівник на функцію обробник в file_operations
```c
static struct file_operations hive_fops = {
	.open =    &cdev_open,
	.release = &cdev_release,
	.read =    &cdev_read,
	.write =   &cdev_write,
	.unlocked_ioctl = &ioctl_fun, //тут вказано вказівник на функцію обробник
	.owner =   THIS_MODULE,
};
```
Реалізація такої функції наведена нижче
```c
long ioctl_fun(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct hive_ftree_item *item = hive_ftree_get(&tree, file);
	if (NULL == item)
		return -EBADF;	
	
	switch(cmd) {
		case IOCTL_BEE_TWERK:
		
		if(item->length < sizeof(magic_phrase))
			resize_buffer(item, sizeof(magic_phrase));
		
		int i = 0;
		for(i = 0; i < sizeof(magic_phrase); i++)
        	(item->buffer)[i] = magic_phrase[i];
		
		MOD_DEBUG(KERN_DEBUG, "Twerk request has been sent");
		
		break;
		case IOCTL_CHG_BUF_SIZE:
			if(arg == 0)
				arg = sizeof(magic_phrase);	
			
			resize_buffer(item, arg);
			MOD_DEBUG(KERN_DEBUG, "New buf size %i", item->length);
		break;
	}
}
```
### Тестова програма
На цьому етапі виконання роботи завершено, залишилось все протестувати. 
Для цього було написано просту тестову програму, яка перевіряє всі можливості модуля

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define IOCTL_BEE_TWERK 0
#define IOCTL_CHG_BUF_SIZE 1

int main()
{
	printf("Hello\n");
	
	char buf[] = "My twerk letters My twerk letters My twerk letters My twerk letters My twerk letters";
	const int size = sizeof(buf);
	char read_buf[size];
	memset(read_buf, 0, size);
	
	const char *devname = "/dev/hivemod";
	
	int fd = open(devname, O_RDWR);
	int fd2 = open(devname, O_RDWR);
	int fd3 = open(devname, O_RDWR);
	printf("FD: %i\n", fd);
	printf("FD2: %i\n", fd2);
	printf("FD3: %i\n", fd3);
	
	ioctl(fd, IOCTL_CHG_BUF_SIZE, 85);
	write(fd, buf, size);
	read(fd, read_buf, size);
	printf("BORING READ BUF before super twerk: %s\n", read_buf);
	ioctl(fd, IOCTL_BEE_TWERK, 0);
	read(fd, read_buf, size);
	
	close(fd);
	//close(fd2);
	//close(fd3);
	printf("READ BUF REALLY TWERK??: %s\n", read_buf);
	
	
	return 0;
}

```
### Тести
Настав час запускати.
Отримав такий результат
```
[ 2537.699149] hivemod: New file entry 00000000bbf4c1b0 created
[ 2537.699160] hivemod: New file entry 00000000f3abbc51 created
[ 2537.699168] hivemod: New file entry 00000000ec8b4a6b created
[ 2537.699224] hive_ftree_get() - FOUND   Кожний такий вівід каже, що відбувся пошук потрібної структури. Це ще раз підкреслює важливість використання найшвидшого методу пошуку.
[ 2537.699228] hivemod: New buf size 85   Змінено розмір буферу
[ 2537.699232] hive_ftree_get() - FOUND
[ 2537.699235] hivemod: write() - have written 85 bytes Записано 85 байтів в буфер
[ 2537.699240] hive_ftree_get() - FOUND
[ 2537.699244] hivemod: read() - have read 85 bytes Вичитано 85 байтів
[ 2537.699268] hive_ftree_get() - FOUND
[ 2537.699271] hivemod: Twerk request has been sent Використана команда ioctl для тверку
[ 2537.699275] hive_ftree_get() - FOUND
[ 2537.699278] hivemod: read() - have read 85 bytes Знову вичитано 85 байтів
[ 2537.699283] hive_ftree_get() - FOUND
[ 2537.699287] hivemod: File entry 00000000bbf4c1b0 unlinked
[ 2537.699494] hive_ftree_get() - FOUND
[ 2537.699497] hivemod: File entry 00000000ec8b4a6b unlinked
[ 2537.699500] hive_ftree_get() - FOUND
[ 2537.699501] hivemod: File entry 00000000f3abbc51 unlinked

```

Вивід в юзер спейсі має такий вигляд
```
Hello
FD: 3
FD2: 4
FD3: 5
BORING READ BUF before super twerk: My twerk letters My twerk letters My twerk letters My twerk letters My twerk letters
READ BUF REALLY TWERK??: Wow, we made these bees TWERK !

```
Бачимо, що запланований тверк відбувся за планом)
