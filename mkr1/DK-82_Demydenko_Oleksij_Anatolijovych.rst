==============================
Демиденко Олексій Анатолійович
==============================


#. Що таке "fork" в термінології Git. Вкажіть принципи роботи з fork'ами. Наведіть команду для створення локальної копії віддаленого
   репозиторію https://github.com/kpi-keoa/kpi-embedded-linux-course
    Fork -  розгалудження, тобто коли ми копіюємо репозиторій певного проекту. Створення нової гілки у своєму локальному репозиторії.
	Команда: git clone https://github.com/kpi-keoa/kpi-embedded-linux-course
#. Яким чином можна додати зміни до вже створеного (останнього) коміту? Наведіть приклад.
   Чи варто так робити в основних публічних гілках віддаленого репозиторію? Якщо так, коли? Якщо ні, чому?
   Не варто, бо це може додати путанину у файлах, хіба якщо дуже якийсь специфічний випадок.

#. Є невелика програма (на стадії розробки), що збирається для GNU/Linux оточення та складається з вихідних файлів
   main.c dep.c та заголовку dep.h. Наведіть команду для збірки у бінарний файл prog. При збірці використовується компілятор
   GCC з другим рівнем оптимізації, код написано за стандартом C18 з розширеннями GNU. При компіляції необхідно побачити всі
   можливі попередження (warning). Який флаг необхідно додати, аби трактувати warning як error
   gcc -c main.c -o main.o
   gcc -c dep.c -o dep.o
   gcc -lgtk -lgdk -lglib -lXext -lX11 -lm dep.o main.o -o prog
#. Для чого використовуються ramdisk (initramfs)? Наведіть приклад сценарію реального використання.
   Що таке bzImage, його історія та для чого він використовується.
   Ramdisk - файлова система памяті. initramsf - стартова файлова система у оперативній памяті, яка основана на файловій    системи зі змінним розміром. Як приклад використання - формування    скриптів та утилітів, що потрібні для монтування    файливоих систем при запуску файлу init.
   BzImage - формат, що був розроблений для переходу через обмеження у розміру сгенерованих ядер старих архітектур. Суть у тому, що відбувається розділ ядра на несуміжні області памяті. 

#. Чи можна використовувати операції з плаваючою крапкою в модулях ядра? Якщо так, коли це доцільно? Якщо ні, чому?
   Модулі ядра не можуть просто так працювати з числами з плаваючою точкою. Це зв'язано з особливостями математичного сопроцесора ( FPU). Різниця у тому, що у "user-space" можливість    задіяти математичний сопроцесор видає ядро. А у просторі розробника ( kernel space) модуль повинен самостійно дати доступність FPU на час проведення певної операції. Тобто,    виктористовувати не можна, в kernel немає операцій х плаваючою точкою.
#. Що таке макромодифікатори __init та __exit в коді модуля ядра? Як працюють та для чого використовуються?
   макрос __init викликає відбрасування функції ініціалізації та звільнення памяті, після завершення ініціалізацій функцій для вбудованих драйверів, проте не для завантажувальних модулів.    Макрос __exit викликає  пропуск функцій, якщо модуль вбудований у ядро не впливає на завантажувані модулі. Цей макрос важливий тим, що вбудовані драйвери не потребують певних функцій очищення а завантажуванні модулі потребують.
#. Що таке callback-функції та для чого використовуються? Наведіть приклад використання container_of у callback-функції?
   Функція, яка передається як аргумент іншій функції і викликається після завершення " батьківської" функції. Виклик аргументу може бути як у синхронному та асинхронному порядку.
container_of дозволяє спроситити структуру даних, тобто вказує на "батьківські" струрктури.
#. Що таке Soft IRQ та які обмеження він накладає на відповідний код? Опишіть коротко.
   Які з вивчених механізмів відкладеної роботи виконуються в Soft IRQ?
    Soft IRQ -  Програмні преривання, зазвичай відповідають перериванням від програмних таймерів. Генеруються ядром та їм же відпрацьовуються.Ці переривання використовуються для витіснення     тасклетів при виконанні функції обробника