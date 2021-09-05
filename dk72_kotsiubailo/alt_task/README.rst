==========================================================
**Альтернативне завдання **
==========================================================


**Суть завдання:**

 
Завдання полягає в тому, щоб зібрати і запустити через u-boot Linux Kernel на платі BeagleBone Black, який побудований на базі процесора ARM Cortex-A8 зібрати ядро на платі і на базі нього написати модуль ядра який запалює світлодіод при натисканні кнопки.   
      

 **Хід роботи:**


1.На початку було загружено Linaro gcc (нам був потрібний кросс-компілятор). Далі було загружено і налаштоване u-boot і Лінукс ядро. Після цього було записано зібране Лінукс ядро та u-boot на sd-карту. В результаті цього було загружено ядро лінукс на платі. Для керування було використано minicom, який здатен через протокол UART відображати командну строку на пк.  
2. Для того, щоб виконувати написаний драйвер потрібно загрузити linux headers. Для цього потрібно виконати команду 

.. code-block:: bash


    apt-get install linux-headers-$(uname -r)


3. Для того щоб розуміти принцип роботи GPIO і роботи з ним. В директорії `` /sys/class/gpio  `` можна побачити різні файли через які можна зробити налаштування GPIO. Для того щоб налаштувати порт 49 (хоча з перелічених gpio немає gpio49) потрібно виконати дію `` echo > 49 export `` .Після цього в переліку можна побачити `` gpio49 `` . Заходимо в файл gpio49 ( `` cd gpio49 ``). І бачимо, що в ньому знаходяться різні файли active_low  device  direction  edge  label  power  subsystem  uevent  value. Для того щоб нам налаштувати gpio49 як вихід треба записати out в direction файл
( `` echo out > direction `` ) . Далі запишемо 1 в файл value і світлодіод загориться ( `` echo 1 > value `` ). Тобто видно що можна керувати GPIO навіть в командній строкі.

.. code-block:: bash


    debian@arm:/sys/class$ cd gpio                                                  
    debian@arm:/sys/class/gpio$ ls                                                  
    export  gpio12  gpio13  gpiochip0  gpiochip32  gpiochip64  gpiochip96  unexport 
    debian@arm:/sys/class/gpio$ echo 49 > export                                    
    debian@arm:/sys/class/gpio$ ls                                                  
    export  gpio13  gpiochip0   gpiochip64  unexport                                
    gpio12  gpio49  gpiochip32  gpiochip96
    debian@arm:/sys/class/gpio$ cd gpio49                                           
    debian@arm:/sys/class/gpio/gpio49$ ls                                           
    active_low  device  direction  edge  label  power  subsystem  uevent  value     
    debian@arm:/sys/class/gpio/gpio49$ echo out >direction                          
    debian@arm:/sys/class/gpio/gpio49$ echo 1 > value                               
    debian@arm:/sys/class/gpio/gpio49$ echo 0 > value                               
    debian@arm:/sys/class/gpio/gpio49$ cd ..                                        
    debian@arm:/sys/class/gpio$ echo 49 > unexport                                  
    debian@arm:/sys/class/gpio$ ls                                                  
    export  gpio12  gpio13  gpiochip0  gpiochip32  gpiochip64  gpiochip96  unexport

4. Розглянемо основнці функції для контролю gpio, які будуть застосовуватися в даній роботі:

.. code-block:: bash


    static inline bool gpio_is_valid(int number)            // перевіряє чи входить число в межі існуючих GPIO портів (максимальний на BBB є 127)
    static inline int  gpio_request(unsigned gpio, const char *label)        // виділяє номер GPIO, намічає  призначена для sysfs
    static inline int  gpio_export(unsigned gpio, bool direction_may_change) // зробить доступним через sysfs, це те саме коли в файл export ми записували число для      того, щоб gpio був 
																		//доступним. Іще bool значенню дозволяє чи забороняє змінюватися від вводу до виводу і навпаки
    static inline int  gpio_direction_input(unsigned gpio)  // конфігурує номер порта як вхід
    static inline int  gpio_get_value(unsigned gpio)        // дістає значення 0 чи 1 з потра (треба якщо підключаємо кнопку
    static inline int  gpio_direction_output(unsigned gpio, int value)       // конфігурація, як вихід
    static inline int  gpio_set_debounce(unsigned gpio, unsigned debounce)   // час для того щоб не було брязкоту контактів на кнопкі в ms (в залежності від  платформи)
    static inline int  gpio_sysfs_set_active_low(unsigned gpio, int value)   // це записує інвертоване значення на потр gpio
    static inline void gpio_unexport(unsigned gpio)         // видаляє з sysfs
    static inline void gpio_free(unsigned gpio)             // видаляє  GPIO line (те саме коли ми після роботи з GPIO записуємо значення потра в файл unexport)
    static inline int  gpio_to_irq(unsigned gpio)           // взаємодія з IRQ (перериваннями)

5. Переривання. В даній роботі використовуються переривання. В LKM треба реєструвати функцію обробника переривань, дана функція визначає що потрібно робити під час переривання.
В даному прикладі функція обробника переривань називається gpio_irq_handler і вона має вигляд:

.. code-block:: bash
	
	
    static irq_handler_t gpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs) { 

    … }

Потім ця функція обробника реєструється із запитом на переривання (IRQ), використовуючи функцію request_irq () наступним чином:

.. code-block:: bash


    result = request_irq(irqNumber,               // Запит на число переривань (в нашому випадку він залежить від кнопки) 
            (irq_handler_t) ebbgpio_irq_handler, // Вказівник на функцію обробника
            IRQF_TRIGGER_RISING,                 // Перериванню по переднюму фронту (При нажиманні кнопки)
            "gpio_handler",                  // Використовується в /proc/interrupts щоб ідентифікувати власника
            NULL);                               // *dev_id

Щоб скасувати запит IRQ, існує також функція free_irq (). У даному прикладі функція free_irq () викликається всередині функції gpio_exit (), яка викликається при вивантаженні LKM.

**Код:**

.. code-block:: bash


    #include <linux/init.h>
    #include <linux/module.h>
    #include <linux/kernel.h>
    #include <linux/gpio.h>
    #include <linux/interrupt.h>

    MODULE_LICENSE("GPL");
    MODULE_AUTHOR("Anton Kotsiubailo");
    MODULE_DESCRIPTION("Flashing LED using button on BBB");
    MODULE_VERSION("0.1");

    static unsigned int gpioLED = 49;
    static unsigned int gpioButton = 115;
    static unsigned int irqNumber;
    static unsigned int numberPresses = 0;
     static bool ledOn = 0;

    static irq_handler_t gpio_irq_handler(unsigned int irq, void *dev_id,
					 				  structt pt_regs *regs);
    
    static int __init gpio_init(void)
    {
		    int result = 0;
		    printk(KERN_INFO " Initializing the GPIO\n");

		    if (!gpio_is_valid(gpioLED)) {
		    printk(KERN_INFO "invalid LED GPIO\n");
		    return -ENODEV;
		}

		    ledOn = true;
		    gpio_request(gpioLED, "sysfs");
		    gpio_direction_output(gpioLED, ledOn);
		    gpio_export(gpioLED, false);

		    gpio_request(gpioButton, "sysfs");
		    gpio_direction_input(gpioButton);
        gpio_set_debounce(gpioButton, 200);
	    	gpio_export(gpioButton, false);

		    printk(KERN_INFO "The button state is currently: %d\n",
                gpio_get_value(gpioButton));

		    irqNumber = gpio_to_irq(gpioButton);
		    printk(KERN_INFO "The button is mapped to IRQ: %d\n",
	             irqNumber);

		    result = request_irq(irqNumber, (irq_handler_t)gpio_irq_handler,
			     			 IRQF_TRIGGER_RISING, "gpio_handler", NULL);

		    printk(KERN_INFO "The interrupt request result is: %d\n",
	           result);
		
		    return result;
    }

    static void __exit gpio_exit(void)
    {
		    printk(KERN_INFO "The button state is currently: %d\n",
	              gpio_get_vv alue(gpioButton));
		    printk(KERN_INFO "The button was pressed %d times\n",
	             numberPressee s);
		    gpio_set_value(gpioLED, 0);
	    	gpio_unexport(gpioLED);
	    	free_irq(irqNumber, NULL);
		    gpio_unexport(gpioButton);
		    gpio_free(gpioLED);
		    gpio_free(gpioButton);
	    	printk(KERN_INFO "Goodbye !\n");
    }

    static irq_handler_t gpio_irq_handler(unsigned int irq, void *dev_id,
					    struct pt_regs *regs)
    {
	    	ledOn = !ledOn;
	    	gpio_set_value(gpioLED, ledOn);
		    printk(KERN_INFO "Interrupt! (button state is %d)\n",
	               gpio_get_vallue(gpioButton));
		
		    numberPresses++;
	
		    return (irq_handler_t)IRQ_HANDLED;
    }

    module_init(gpio_init);
    module_exit(gpio_exit);

Як видно з вище наведеного коду в ``int __init gpio_init(void)`` відбувається ініціалізація кнопки та світлодіода і реєстрація функції обробника переривань.
В функції обробника переривань в даному прикладі йде затухання потім запалювання світлодіоду і так періодично при нажиманні кнопки і збільшення числа нажимань кнопки на 1.
В функції ``__exit gpio_exit(void)`` відбувається деініціалізація портів gpio та переривання.

**Відео:**


 [Google drive](https://drive.google.com/file/d/15t8z7kgYSNciraIVofHUdxg5a1gTSMM_/view?usp=sharing) - по даній ссилці можна подивитися відео демонстрації роботи.

**Висновки**

В результаті даної роботи було зроблемо модуль ядра пристрій який здатний запалювати потім затухати світлодіод при натисканні кнопки. В даній роботі було розглянуто принцип роботи gpio
та взаємодія з ним в ядрі linux. Також слід зазначити, що було розглянуто принцип роботи переривання і взаємодія переривання з gpio.


