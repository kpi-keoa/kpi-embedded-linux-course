=============================================
Лабораторна робота №3
=============================================

Тема
------

**High-Resolution timers**

Завдання:
-------
Написати модуль ядра, який:
	* Приймає параметр *cnt* - кількість циклів, які повинен опрацювати *hr-таймер* 
	* Приймає параметр *delay* - являється затримкою між двума спрацюваннями *hr-таймера*
	* Спочатку роботи модуля, він повинен надрукувати поточне значення *jiffies*
	* Після цього запускає тасклет, який повинен надрукувати власне значення *jiffies*
	* Виділяє масив розміру *cnt*, використовуючи динамічну алокацію
	* init запускає *hr-таймер* з затримкою *delay* і функція завершається
	
Хід роботи
-------

В ході виконання лабораторної роботи, було поставлене завдання використовувати таймери з високою
роздільною здатністю. Ці таймери забезпечують високу точність і управління таймерами. Вони функціонують
з гранулярністю в наносекунди і цей час представлено в спеціальному типу ktime. При роботі з hrtimers 
існує відмінність між абсолютним і відносним часом, і при виклику слід вказувати необхідний тип.

hr-таймери представлені структурою *hrtimer*. В ній визначаються компоненти таймера, які будуть необхідні
користувачеві. Вказується інформація, яка використовується для управління. Процес починається з 
ініціалізації таймера за допомогою *hrtimer_init*. В цю функцію передається ім"я таймера, визначається
тип часу(структура *clockid_t*) і режим таймера(чи буде він працювати одноразово, чи перезапускатись).

Таймер ініціалізовано і його потрібно запустити. Цей крок виконується за допомогою *hrtimer_start*. 
У цьому виклику вказується час спрацювання таймера (який ініціалізовано в *ktime_t*) і режим для значення
часу - абсолютний або відносний час. Після спрацювання таймера його роботу можна закінчити за допомогою
*hrtimer_cancel*. Ця функція буде чекати завершення функції зворотнього визову. 

Висновки
-------

Зробивши висновки по виконаній роботі, можна відзначити, що *hr-таймери* є більш точними за стандартні
таймери ядра. Це випливає з того, що *hr-таймери* мають більшу роздільну здатність - наносекунди, від 
стандартних - мілісекунди. Таймери з високою роздільною здітністю, зважаючи на всі висновки, можна
використовувати для ШИМ-модуляції, кправління електро-двигунами тощо. З практики можна відзначити,
що навіть з цими таймерами отримувались деякі похибки.
