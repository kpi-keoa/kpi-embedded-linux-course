==============================
**Домашня контрольна робота**
==============================

**Завдання:**
~~~~~~~~~~~~~
Написати програму, яка:
    * Приймає аргументи командного рядка. Для парсингу використати *Argp*.
    * Збирається за допомогою *Make*. *Makefile* містить додаткові цілі *build* і *clean*, де *build* є метою-посиланням на основну, а *clean* – PHONY-метою, що очищає всі артефакти складання.
    * Виводить результат у *stdout*, а помилки – у *stderr*.
    * Містить частину службових функцій, винесених в окремий модуль *utils* (*utils.c* та *utils.h*).
    * Використовує прапорець *--verbose* (*-v*), який змушує програму виводити виконувані дії детально. Інакше вважається, що її вивід використовуватиметься іншою програмою.
    * Повертає *exit code 0* у разі успішного завершення,інакше відповідний код помилки. Для визначення помилок використовується *enum*.
    * Використовує об'єктний стиль коду для внутрішніх конструкцій, що реалізують основний функціонал.
    * Програма виводить всі файли в директорії, создані після вказаної дати.

**Хід роботи:**
~~~~~~~~~~~~~~~

    * Був створений *Makefile* із цілями *build* та *clean*. Основна ціль *all* посилається на *build*.
    * Програма була оформлена у вигляді декількох файлів.
    * Частина службових функцій винесена в окремий модуль *utils* (*utils.c* та *utils.h*).
    * У головному source-файлі *search_files.c* відбувається оброблення аргументів командної строки та виклик необхідних функцій.
    * Є можливість використовувати прапорці *--verbose* (*-v*).
    * Є можливість передачі шляху директорії, файли якої необхідно вивести в термінал.
    * Є можливість передати дату та час, по яким необхідно фільтрувати файли.
    * У випадку відсутності аргументів, програма виведе файли директорії, в якій знаходиться виконуваний файл програми.
    * Виконується перевірка на правильність введення дати та часу, по яким відбувається фільт файлів.
    * Результат роботи програми виводиться у *stdout*, а помилки – у *stderr*.
    * Перелік можливих помилок заданий за допомогою *enum*.
    * Загалом був використаний об'єктний стиль коду для внутрішніх конструкцій, що реалізують основний функціонал.

**Результат:**
~~~~~~~~~~~~~~

.. code-block:: bash

        Запуск програми без аргументів:

        ./search_files.o
        .
        Makefile
        search_files.o
        ..
        utils.h
        search_files.c
        utils.c
        README.rst
        .gitignore

.. code-block:: bash

        Запуск програми з аргументом -v (--verbose) та вказанням шляху до директорії:

        /search_files.o . -v
        -verbose enabled
        Enter condition:
        day:0
        hour:0
        min:0
        dir_path: .
        File: ., mday: 12, hours: 1, min: 17
        File: Makefile, mday: 12, hours: 1, min: 18
        File: search_files.o, mday: 12, hours: 1, min: 17
        File: .., mday: 10, hours: 11, min: 39
        File: utils.h, mday: 12, hours: 0, min: 58
        File: search_files.c, mday: 12, hours: 1, min: 15
        File: utils.c, mday: 12, hours: 0, min: 59
        File: README.rst, mday: 12, hours: 1, min: 29
        File: .gitignore, mday: 10, hours: 11, min: 39

.. code-block:: bash

        Запуск програми з аргументом -v (--verbose), датою та часом:

        /search_files.o . -v 12 1 18

        -verbose enabled
        Enter condition:
        day:12
        hour:1
        min:18
        dir_path: .
        File: Makefile, mday: 12, hours: 1, min: 18
        File: README.rst, mday: 12, hours: 1, min: 31


.. code-block:: bash

        Запуск програми з неправильно переданими аргументами дати:

        ./search_files.o . -v 45 1 1

        -verbose enabled
        Enter condition:
        day:45
        hour:1
        min:1
        INVALID ENTERED DATE
        dir_path: .
        File: ., mday: 12, hours: 1, min: 17
        File: Makefile, mday: 12, hours: 1, min: 18
        File: search_files.o, mday: 12, hours: 1, min: 17
        File: .., mday: 10, hours: 11, min: 39
        File: utils.h, mday: 12, hours: 0, min: 58
        File: search_files.c, mday: 12, hours: 1, min: 15
        File: utils.c, mday: 12, hours: 0, min: 59
        File: README.rst, mday: 12, hours: 1, min: 33
        File: .gitignore, mday: 10, hours: 11, min: 39

.. code-block:: bash

        Запуск програми з вказанням шляху директорії:

        ./search_files.o ../

        lab1
        .
        ..
        dkr
        README.rst
        lab0

.. code-block:: bash

        Неправильно вказаний шлях директорії

        ./search_files.o ....

        Directory .... is not found!

Отже, програма правильно обробляє передані для неї дані, та правильно відпрацьовує.
