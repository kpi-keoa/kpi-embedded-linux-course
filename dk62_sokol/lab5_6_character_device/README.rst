=====================
Лабораторна робота №5_6
=====================
**Завдання:**

- Виконати ``TODO`` (доробити функції ``cdev_read``, ``cdev_write``, і т.д.).

- Написати тестовий файл для перевірки роботоздатності функцій.

--------------------

**Хід роботи:**


**Red-Black Treee:**

- Для початку було зроблено замість списку дерево(Red-Black Treee). Для цього необхідно ініціалізувати його таким чином:

.. code-block::

  struct rb_root my_tree = RB_ROOT;

- Далі необхідно створити самі вузли цього дерева. Для цього було реалізовано функцію ``static int rb_insert()`` . Його стуктура описана нижче:

.. code-block::

  static int rb_insert(struct rb_root *root, struct ftree_item *item)
  {
	struct rb_node **new = &(root->rb_node), *parent = NULL;
	while (*new) {
		struct ftree_item *this = container_of(*new, struct ftree_item, node);
		int result = memcmp(item->file, this->file, sizeof(struct file));
		parent = *new;
		if (result < 0) {
			new = &((*new)->rb_left);
		} else if (result > 0) {
			new = &((*new)->rb_right);
		} else
			return 1;
	}
	rb_link_node(&item->node, parent, new);
	rb_insert_color(&item->node, root);
	return 0;
  }

- Далі для роботи з файлом маємо таку структуру:

.. code-block::

  struct ftree_item {
	struct rb_node node;
	struct file *file;
	char *buffer;
	long length;
	long rdoffset;
	long wroffset;
  };

- Тому для цієї структури необхідно спочатку виділити пам'ять та обнулити всі данні(нам необіхно було виділити пам'ять так, щоб вона була встановлена на нуль, тому використовуємо ``kzalloc`` ):

.. code-block::

  static inline struct ftree_item *ftree_new(void)
  {
	struct ftree_item *item = kzalloc(sizeof *item, GFP_KERNEL);
	if (NULL == item) {
		return NULL;
	}
	item->buffer = NULL;
	item->length = 0;
	item->rdoffset = 0;
	item->wroffset = 0;
	return item;
  }

- При закриванні файлу одним необхідно буде видалення пам'яті, тому для цього реалізовуємо наступну функцію:

.. code-block::

  static inline void ftree_rm(struct ftree_item *item)
  {
	if (NULL == item)
		return;
	rb_erase(&item->node, &my_tree);
	kfree(item->buffer);
	kfree(item);
  }

- Також під час роботи необхідно буде реалізовуати пошук даного дерева, тому для цього використовується наступна функція:

.. code-block::

  static struct ftree_item *ftree_get(struct rb_root *root, struct file *file)
  {
	struct rb_node *node = root->rb_node;
	while (node) {
		struct ftree_item *data = container_of(node, struct ftree_item, node);
		int result = memcmp(data->file, file, sizeof(file));
		if (result < 0) {
			node = node->rb_left;
		} else if (result > 0) {
			node = node->rb_right;
		} else {
			return data;
		}
	}
	return NULL;
  }

**Робота з файловими функціями:**

- Для роботи з файлами реалізовано 6 функцій, тому для їх виклику було реалізовано структуру, яка має масив вказівників на різні функції:

.. code-block::

  static struct file_operations hive_fops = {
	.open = &cdev_open,
	.release = &cdev_release,
	.read =	&cdev_read,
	.write = &cdev_write,
	.unlocked_ioctl = &cdev_ioctl,
	.llseek = &cdev_llseek,
	// required to prevent module unloading while fops are in use
	.owner = THIS_MODULE,
  };

- Під час відкривання файлу спочатку необхідно створити власне дерево, ініціалізації структурних файлових даних:

.. code-block::

  static int cdev_open(struct inode *inode, struct file *file)
  {
	struct ftree_item *item = ftree_new();
	if (NULL == item) {
		MOD_DEBUG(KERN_ERR, "Buffer allocate failed for %p", file);
		return -ENOMEM;
	}
	// fill the rest
	item->file = file;
	if(!rb_insert(&my_tree, item)) {
		MOD_DEBUG(KERN_DEBUG, "New file entry %p created", file);
	} else {
		MOD_DEBUG(KERN_DEBUG, "New file not created");
	}
	return 0;
  }

- Для закривання файлу реалізовується наступна функція:

.. code-block::

  static int cdev_release(struct inode *inode, struct file *file)
  {
	struct ftree_item *item = ftree_get(&my_tree, file);
	if (NULL == item)
		return -EBADF;
	// remove item from list and free its memory
	ftree_rm(item);
	MOD_DEBUG(KERN_DEBUG, "File entry %p unlinked", file);
	return 0;
  }

- Для запису спочатку виділяється пам'ять для запису рядка, після чого за допомогою ``copy_from_user()`` копіюємо блок даних із простору користувача в постір ядра і в кінці зсовуємо курсор та записуємо його довжину:

.. code-block::

  static ssize_t cdev_write(struct file *file, const char __user *buf,
			  size_t count, loff_t *loff)
  {
	struct ftree_item *item = ftree_get(&my_tree, file);
	if (NULL == item) {
		MOD_DEBUG(KERN_DEBUG, "Write ERROR");
		return -EBADF;
	}

	char *buf_m = kzalloc(sizeof(*buf_m) * count, GFP_KERNEL);
	if (NULL == buf_m) {
		MOD_DEBUG(KERN_DEBUG, "Write ERROR");
		return -EBADF;
	}
	item->buffer = buf_m;
	
	if(copy_from_user(item->buffer + *loff, buf, count) != 0) {
		MOD_DEBUG(KERN_DEBUG, "Failed to write file");
		return -EFAULT;
	}
	*loff += count;
	MOD_DEBUG(KERN_INFO, "buffer = %s | %li", item->buffer, strlen(item->buffer));

	if(item->length < *loff) {
		item->length = *loff;
	}
	return count;
  }

- Для зчитування виконуємо зворотню функцію ``copy_to_user()`` , яка копіює блок даних із простору ядра в простір користувача відносно значення ``loff_t *loff``. 

.. code-block::

  static ssize_t cdev_read(struct file *file, char __user *buf,
			 size_t count, loff_t *loff)
  {
	struct ftree_item *item = ftree_get(&my_tree, file);
	if (NULL == item) {
		return -EBADF;
	}
	if(*loff >= item->length) {
		MOD_DEBUG(KERN_DEBUG, "Read pointer above file size");
		return -ENOMEM;
	}
	if(*loff + count > item->length) {
		count = item->length - *loff;
	}
	if(copy_to_user(buf, item->buffer + *loff, count)) {
		MOD_DEBUG(KERN_DEBUG, "Failed to read file");
		return -EFAULT;
	}
	*loff += count;
	return count;
  }

- Далі було реалізовано функцію ``cdev_llseek()``, за допомогою якої можна змінити місце курсора:

.. code-block::

  static loff_t cdev_llseek(struct file *file, loff_t offset, int origin)
  {
	struct ftree_item *item = ftree_get(&my_tree, file);
	if (NULL == item)
		return -EBADF;
	loff_t newpos;
	switch(origin) {
	case SEEK_SET:
		newpos = offset;
		break;
	case SEEK_CUR:
		newpos = offset + file->f_pos;
		break;
	case SEEK_END:
		newpos = item->length + offset;
		break;
	default:
		MOD_DEBUG(KERN_DEBUG, "Macross name is incorrect");
		return -EINVAL;
		break;
	}
	if(newpos < 0) {
		return -EINVAL;
	}
	file->f_pos = newpos;
	return newpos;
  }

- Останньою функцією є ``cdev_ioctl()`` . За допомогою неї можна з викликом макроса ``LENGTH`` можна завантажити розмір буфера, і за допомогою ``BUFFER`` виконується завантаження самого рядка(буфера) із простору користувача. Для такої реалізації було використано ``_IOW`` (перший аргумент описує до якої підсистеми застосовується ``ioctl`` , другий аргумент ідентифікує ``ioctl``, третім аргументом є типом переданого параметру):

.. code-block::

  #define LENGTH _IOW('i', 0, int *)
  #define BUFFER _IOW('i', 1, char *)

  static long cdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
  {
	struct ftree_item *item = ftree_get(&my_tree, file);
	if (NULL == item)
		return -EBADF;
	switch(cmd) {
	case BUFFER:
		MOD_DEBUG(KERN_INFO, "Flag BUFFER:");
		char *buf = kzalloc(sizeof(*buf) * item->length, GFP_KERNEL);
		if (NULL == buf) {
			MOD_DEBUG(KERN_DEBUG, "Write ERROR");
			return -EBADF;
		}
		item->buffer = buf;
		if(copy_from_user(item->buffer, (char *)arg, item->length) != 0) {
			MOD_DEBUG(KERN_DEBUG, "Failed to write file");
			return -EFAULT;
		}
		MOD_DEBUG(KERN_INFO, "BUFFER = %s", item->buffer);
		break;
	case LENGTH:
		MOD_DEBUG(KERN_INFO, "Flag LENGTH:");
		item->length = arg;
		MOD_DEBUG(KERN_INFO, "LENGTH = %li", item->length);
		break;
	default:
		return -ENOTTY;
	}
	return 0;
  }

- В кінці необхідно прибирати за собою, тому для цього виконуємо наступне:

.. code-block::

  static void module_cleanup(void)
  {
	// notice: deallocations happen in *reverse* order
	if(alloc_flags.dev_registered) {
		device_destroy(hive_class, hive_dev);
	}
	if(alloc_flags.class_created)  {
		class_unregister(hive_class);
		class_destroy(hive_class);
	}
	if (alloc_flags.cdev_added) {
		cdev_del(&hive_cdev);
	}
	if (alloc_flags.dev_created) {
		unregister_chrdev_region(hive_dev, 1);
	}
	// paranoid cleanup (afterwards to ensure all fops ended)
	struct ftree_item *item;
	struct rb_node *rbp = rb_first(&my_tree);
	struct rb_node *rb_l = rb_last(&my_tree);
	while(rbp != rb_l) {
		item = rb_entry_safe(rbp, struct ftree_item, node);
		ftree_rm(item);
		rbp = rb_next(rbp);
	}
  }

- В кінці було додано для створення класу пристроїв та створення пристрою і його реалізації за допомогою ``sysfs`` :

.. code-block::
	
  static struct class *hive_class = NULL;

  if ((hive_class = class_create(THIS_MODULE, "hive_class")) == NULL) {
	unregister_chrdev_region(hive_dev, 1);
	return -1;
  }
  alloc_flags.class_created = 1;
  if (device_create(hive_class, NULL, hive_dev, NULL, "hive_dev") == NULL) {
	class_destroy(hive_class);
	unregister_chrdev_region(hive_dev, 1);
	return -1;
  }
  alloc_flags.dev_registered = 1;

- Було додали тестовий файл, те було протестовано флаги, запису/зчитування, відкривання/закривання файлу, та запис за допомогою функції ``ioctl()`` . Результати можна побачити нижче:

.. image:: img/lab5_6_res.png


