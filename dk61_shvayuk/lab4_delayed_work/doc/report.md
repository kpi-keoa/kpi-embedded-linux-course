###  Результати виконання лабораторної роботи №4

У цій лабораторній я створив модуль ядра, який використовує таймер та workqueue.  
Почав я із реалізації таймера. У моїй програмі створюється потік, а вже в середині нього я виділяю пам'ять під таймер та ініціалізую його.  
У callback-функції таймер паревіряє поточне значення jiffies, якщо воно є кратним значенню **COMPARE_DIVIDER_VAL** - необхідно завершити роботу потоку та видалити таймер. Із цим у мене виникли певні труднощі. Мені знадобився певний час щоб усвідомити здавалося б, очевидну річ. Справа в тому, що callback функція таймера виконується у контексті переривання - тому у ній не можна використовувати функції, які можуть спати, або використовувати busy wait. А у  функції _kthread_stop_, яку я намагався використовувати спочатку є рядок ``wait_for_completion(&kthread->exited);``, в якому вона _очікує_ на завершення потоку, а цього робити у контексті переривання ні в якому разі не можна.  
Вирішив цю проблему я використавши **completion**. Після того як потік створив та проініціалізував таймер - він викликає функцію ``wait_for_completion(&timer_comp);``, що "зупиняє" його виконання до того часу, доки звідкись не буде надіслана notification(не  знаю наскільки коректно так казати у контексті completion). А ця notification надсилається із callback-функції таймера викликом ``complete_all(&timer_comp);``, який продовжує виконання коду із раніше зупиненого потоку, який видаляє таймер та завершує потік.  
Якщо ж поточне значення jiffies не було кратним COMPARE_DIVIDER_VAL - воно додається у list, а таймеру задається новий timeout по завершенню якого callback-функція буде викликана ще раз: ``mod_timer(timer_ptr, jiffies + TIMER_PERIOD_JIFFIES);``.  
У випадку, якщо за час роботи модуля жодного разу не настала умова при якій буде викликана функція ``complete_all(&timer_comp);`` - передбачена перевірка
```
if(!completion_done(&timer_comp)) {
        complete(&timer_comp);
        printk(KERN_INFO "Main thread sent the completion "
                "to stop the timer_thread\n");
}
```
Якщо жодного разу не була надіслана notification у обраний completion (тобто, потік не прокинувся, не видалив таймер та не завершив себе) - я надсилаю цю notification із основного потоку. Таким чином, програма коректно звільнить всі ресурси у будь-якому випадку.


Принцип роботи із workqueue аналогічний, за вийнятком того, яким чином реалізується завершення потоку із функції-воркера. Тут також не можна використати _kthread_stop()_, не зважаючи на те, що функція-worker виконується у звичайному контексті тому що у такому випадку не буде видалена workqueue:
1. Виклик _kthread_stop()_ із worker-функції
2. _kthread_should_stop()_ у потоці повертає _true_, потік викликає ``cancel_delayed_work_sync(&wq_delay_work);``, в результаті worker-функція, яка очікувала на завершення потоку, відміняється
3. Чомусь потік при цьому також завершує своє виконання і не доходить до ``do_exit(THREADS_RETVAL);``, яка йде одразу після видалення workqueue.

Щоб пофіксити цю проблему я вирішив що можна виставити прапорець **KTHREAD_SHOULD_STOP** і не чекати доки завершиться потік. Щоб зрозуміти як виставити цей прапорець я звернувся до вихідного коду функції _kthread_stop()_, яка, власне, і виставляє цей прапорець, пробуджує потік і чекає на його завершення. Я взяв увесь код, до моменту очікування завершення потоку та застосував його у своїй worker-функції:
```
#define KTHREAD_SHOULD_STOP (1)
struct kthread *kthread_;
get_task_struct(thread_workqueue);
kthread_ = to_kthread(thread_workqueue);
set_bit(KTHREAD_SHOULD_STOP, &kthread_->flags);
wake_up_process(thread_workqueue);
#undef KTHREAD_SHOULD_STOP
```
define я використовую щоб задати KTHREAD_SHOULD_STOP його реальне значення (1), бо enum, у якому це значення ініціалізоване знаходиться у файлі _kernel/kthread.c_ - і отримати до нього доступ із іншого файлу, не лізучи в код ядра - неможливо, тому я замінив його звичайною числовою константою, якій він дорівнює, що видно із наступного лістингу:
```
enum KTHREAD_BITS {
	KTHREAD_IS_PER_CPU = 0,
	KTHREAD_SHOULD_STOP,
	KTHREAD_SHOULD_PARK,
};
```
ну а щоб після виконання цього коду мій define нікому не заважав - я його скасовую за допомогою _#undef_.  
Цікавіше із _struct kthread_ та _to_kthread_. Річ у тім, що ці функції реалізовані у файлі _kernel/kthread.c_ як статичні - тому використовувати їх можливо лише у тому файлі. Єдиний спосіб вирішити цю проблему, не лізучи в код ядра - **Ctrl-C** - **Ctrl-V** цієї функції та структури у мій файл.  
Я розумію, що це дуже сильні костилі, які в реальних проектах використовувати не можна ні в якому разі, тут набагато доречніше виглядало б використання completion, але для навчальних цілей дослідити саму можливість та життєздатність цього способу було корисно.

Також я реалізував макрос, що допомагає під час розробки модуля виводити дебаг-інформацію у kernel log, а після закінчення розробки прибрати її закоментувавши всього один рядок:
```
#define DEBUG_MODE 

#ifdef DEBUG_MODE
#define DEBUG_LOG(message)         \
do {                               \
        printk(KERN_INFO message); \
} while(0)

#define DEBUG_LOG_VAR(message,var_val)      \
do {                                        \
        printk(KERN_INFO message, var_val); \
} while(0)
#else
#define DEBUG_LOG(message)
#define DEBUG_LOG_VAR(message,var_val)
#endif
```
А ось-так виглядає його застосування:
```
DEBUG_LOG("Hello from timer thread\n");           // for a simple message
DEBUG_LOG_VAR("timer: jiffies = %lu\n", jiffies); // to print a value of the "jiffies" variable
```

Ось повний вивід програми у лог ядра, який доводить що все працює(**DEBUG_MODE** увімкнений, також додаються мої коментарі, що пояснюють кожне повідомлення):
```
[   86.386468] Hello from module                        запуск модуля
[   86.386873] Threads started                          виділили пам'ять для потоків і запустили їх на виконання
[   86.391799] Hello from timer thread                  функція-handler of timer_thread стартувала
[   86.394866] Hello from wq thread                     функція-handler of workqueue_thread стартувала
[   86.462101] timer: jiffies = 4294913912              почергові виклики callback-функцій таймера і workqueue, під час яких умова завершення не виконується
[   86.470190] wq: jiffies = 4294913914
[   86.534091] timer: jiffies = 4294913930
[   86.542224] wq: jiffies = 4294913932
[   86.606100] timer: jiffies = 4294913948
[   86.614206] wq: jiffies = 4294913950
[   86.678088] timer: jiffies = 4294913966
[   86.686210] wq: jiffies = 4294913968
[   86.750093] timer: jiffies = 4294913984
[   86.758181] wq stopped the wq_thread just now        callback-функція workqueue, спрацювала умова завершення, виставлений прапорець KTHREAD_SHOULD_STOP для workqueue_thread
[   86.758437] wq thread is going to be stopped         повідомлення із workqueue_thread, яке показує, що workqueue була видалена, а потік зараз завершить роботу
[   86.822271] timer: jiffies = 4294914002
[   86.894272] timer: jiffies = 4294914020
[   86.966268] timer: jiffies = 4294914038
[   87.038267] timer: jiffies = 4294914056
[   87.110291] Timer stopped the timer_thread just now  callback-функція таймера, спрацювала умова завершення, активований completion
[   87.110384] Timer thread is going to be stopped      повідомлення із timer_thread, completion's notification was received, the timer was deleted and the thread is going to do_exit()
[   89.487035] try to exit                              користувач намагається вивантажити модуль
[   89.487063] Timer list val is 4294914056             вивід вмісту list_timer
[   89.487069] Timer list val is 4294914038
[   89.487075] Timer list val is 4294914020
[   89.487080] Timer list val is 4294914002
[   89.487085] Timer list val is 4294913984
[   89.487091] Timer list val is 4294913966
[   89.487096] Timer list val is 4294913948
[   89.487102] Timer list val is 4294913930
[   89.487107] Timer list val is 4294913912
[   89.487112] Timer's list deleted                     list_timer видалено
[   89.487118] wq list val is 4294913968                вивід вмісту list_workqueue
[   89.487123] wq list val is 4294913950
[   89.487128] wq list val is 4294913932
[   89.487133] wq list val is 4294913914
[   89.487138] Workqueue's list deleted                 list_workqueue видалено
[   89.487143] Long live the Kernel!                    кінець роботи програми
```

Якщо вимкнути **DEBUG_MODE** - лог виконання програми матиме наступний вигляд:
```
[ 1171.358421] Hello from module
[ 1174.447184] Timer list val is 217859
[ 1174.447203] wq list val is 217969
[ 1174.447209] wq list val is 217951
[ 1174.447214] wq list val is 217933
[ 1174.447219] wq list val is 217915
[ 1174.447224] wq list val is 217897
[ 1174.447229] wq list val is 217879
[ 1174.447234] wq list val is 217861
[ 1174.447239] Long live the Kernel!
```

У ході виконання лабораторної роботи були використані наступні джерела:
[1](https://github.com/cirosantilli/linux-kernel-module-cheat/blob/ad077d3943f79c0f6481dab929970613c33c31a7/kernel_module/workqueue_cheat.c), [2](https://gist.github.com/yagihiro/309746), [3](https://habr.com/ru/company/embox/blog/244155/), [4](https://sysplay.in/blog/tag/kernel-threads/), [5](https://elixir.bootlin.com/).






