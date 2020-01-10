###  Результати виконання лабораторної роботи №5-6

У цій лабораторній я створив модуль ядра, який демонструє роботу із символьним пристроєм, а також використовує структуру даних **rbtree**.
  
Суть роботи заключалася у реалізації 7 ToDo, підемо у порядку зростання складності:
1. TODO: fix to make it zero'ed first  
Вирішується дуже просто: замість виклику _kmalloc_, використати _kzalloc_ для буферу, який буде доступним для користувача:
```
char *buf = kzalloc(sizeof(*buf) * buffsize, GFP_KERNEL);
```
2. TODO: add stuff here to make module register itself in /dev  
Також проста задача, необхідно лише додати _udev_class_ та _udev_device_ та коректно їх ініціалізувати:
```
static struct class *hive_udev_class = NULL;
static struct device *hive_udev_device = NULL;
...
hive_udev_class = class_create(THIS_MODULE, udev_class_name);
hive_udev_device = device_create(hive_udev_class,
                                NULL, 
                                hive_dev, 
                                NULL,
                                udev_device_name,
                                MINOR(hive_dev));
...
device_destroy(hive_udev_class, hive_dev);
class_destroy(hive_udev_class);
```
3. TODO: Add buffer read logic.  
Була реалізована підтримка системного виклику _read()_:
```
static ssize_t cdev_read(struct file *file, char __user *buf, 
			 size_t length, loff_t *offset)
{  
	struct hive_ftree_item *item = hive_ftree_get(&hive_tree_root, file);
	if (NULL == item)
		return -EBADF;
	// TODO: Add buffer read logic.
        char *ptr_kern_buff = item->buffer;
        int bytes_read = 0;
        while (length && *ptr_kern_buff) {
                if (0 != put_user(*ptr_kern_buff, buf++) ) {
                        return -1;
                }
                length--;
                bytes_read++;
                ptr_kern_buff++;
        }
        
	return bytes_read;
}
```
Треба бути уважним з одним моментом: під час роботи у kernel-space не можна просто так розіменувати вказівник, що вказує на пам'ять, що відноситься до user-space. Для роботи із такою пам'яттю необхідно використовувати функції _get_user()_ та _put_user()_.  
4. TODO: Add buffer write logic    
Завдання дуже схоже на попередній пункт:
```
static ssize_t cdev_write(struct file *file, const char __user *buf,
			  size_t length, loff_t *offset)
{
	struct hive_ftree_item *item = hive_ftree_get(&hive_tree_root, file);
	if (NULL == item)
		return -EBADF;
	// TODO: Add buffer write logic.
        if (length > buffsize)
                length = buffsize;
        int cnt = 0;
        while (cnt < length) {
                if (0 != get_user((item->buffer)[cnt], buf) ) {
                        MOD_DEBUG(KERN_DEBUG, "write error\n");
                        return -1;
                }
                buf++;
                cnt++;
        }
        
	return cnt;
}
```
5. TODO: add two ioctl calls  
Я додав підтримку двох дій, що реалізуються драйвером за допомогою ioctl:
```
#define IOCTL_TWERK               ((unsigned int)0) // поміщає у буфер кодову фразу
#define IOCTL_SET_NEW_BUFFER_SIZE ((unsigned int)1) // змінює розмір буферу
```
Сама функція має вигляд:
```
static long cdev_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        struct hive_ftree_item *item = hive_ftree_get(&hive_tree_root, file);
	if (NULL == item)
		return -EBADF;
		
        switch (cmd) {
        case IOCTL_TWERK:
                size_t magic_phrase_len = strlen(magic_phrase);
                if (magic_phrase_len > item->length) {
                        MOD_DEBUG(KERN_DEBUG, "ioctl: the magic phrase is too"
                                  " long, increase the buffer size first\n");
                        return -1;
                }
                for (size_t i = 0; i < magic_phrase_len; i++)
                        (item->buffer)[i] = magic_phrase[i];
                break;
        case IOCTL_SET_NEW_BUFFER_SIZE:
                /* arg is new buffer size */
                char *new_buffer = kmalloc(sizeof(*new_buffer) * arg, GFP_KERNEL);
                if ('\0' != (item->buffer)[0]) {
                        /* to avoid new buffer overflow */
                        if (arg < item->length)
                                (item->buffer)[arg-1] = '\0';
                        char *tmp = item->buffer;
                        item->buffer = strcpy(new_buffer, item->buffer);
                        item->length = arg;
                        kfree(tmp);
                        MOD_DEBUG(KERN_DEBUG, "ioctl: new buffer size = %li \n", item->length);       
                }
        }
        return 0;
}
```
6. TODO: change this to proper associative array or tree  
У демо-прикладі в якості структури, що зберігає файлові дескриптори, буфери і тд був обраний _linked list_. Він погано підходить для цієї задачі, адже, поошук в ньому відбувається за лінійний час, а операція пошуку у моїй реалізації драйвера виконується частіше, ніж вставка/видалення елементів списка. У якості заміни _linked list_ я обрав _rbtree_ - реалізацію червоно-чорного дерева у Linux. Це дозволило зменшити час, необхідний для пошуку елементу у дереві.


Для перевірки роботи модуля була написана тестова програма _src/test_prog.c_ , яка відкриває символьний девайс у режимі _read-write_, записує у нього рядок, зчитує його(не повністю, а лише певну частину), а потім робить 2 виклики _ioctl_:
```
int write_cnt = write(filedesc,write_str, strlen(write_str));
int read_cnt = read(filedesc, read_buff, num_read_symbols);
ioctl(filedesc, IOCTL_TWERK);   
ioctl(filedesc, IOCTL_SET_NEW_BUFFER_SIZE, TEST_NEW_BUFF_SIZE);
```
Фінальний лог модуля має вигляд:
```
[  101.695177] hivemod: loading out-of-tree module taints kernel.
[  101.702437] hivemod: Static dev 241:0 created
[  101.702461] hivemod: This hive has 4 bees
[  104.781432] hivemod: New file entry dc4e0780 created
[  104.781471] search: found dc4e0780 val
[  104.787271] search: found dc4e0780 val
[  104.792756] search: found dc4e0780 val
[  104.797949] search: found dc4e0780 val
[  104.802862] hivemod: ioctl: new buffer size = 22     виклик ioctl змінив розмір буфера

[  104.802880] search: found dc4e0780 val
[  104.808103] search: found dc4e0780 val
[  104.812894] hivemod: File entry dc4e0780 unlinked
[  104.901350] hivemod: All honey reclaimed


write(): Write bytes: expected 27, really: 27           записали у буфер рядок з 27 символів
read(): read bytes: expected 16, really: 16             прочитали рядок (лише 16 символів)
Read buff: Hivemod test str                             вивели цей рядок

ioctl: Read bytes: 21, Read buff: Wow, we made these be виклик ioctl записав кодову фразу у буфер девайса, потмі змінили розмір буфера (він менше ніж був спочатку і фраза оррізалася) та прочитали весь рядок з нього
```
Зверніть увагу скільки разів було викликано функцію пошуку. Це демонструє, що для зберігання даних у нашому випадку необхідно використовувати структуру даних, що дозволяє знайти елемент за найменший час.

У ході виконання лабораторної роботи були використані наступні джерела:
[1](https://www.google.com/search?client=firefox-b-d&q=char+device+driver+example), [2](https://linux-kernel-labs.github.io/master/labs/device_drivers.html), [3](https://www.kernel.org/doc/Documentation/rbtree.txt), [4](https://lwn.net/Articles/218239/)






