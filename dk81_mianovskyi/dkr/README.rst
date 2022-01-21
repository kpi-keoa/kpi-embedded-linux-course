=============================
**Домашня контрольна робота**
=============================

Завдання:
---------------
Написати гру "вгадай число", яка виконує наступні дії:

* Приймає в якості аргументу одне число у діапазоні від 1 до 6
* Генерує власне число у діапазоні від 1 до 6 за допомогою /dev/random
* Якщо числа зпівпали, то гравець отримує + 1 бал, якщо ні - компьютер отримує + 1
* Результати гри зберігаються у окремому файлі 
* Для парсингу аргументів командного рядка використовувати argp
* При уведенні прапора --verbose виводиться детальна інформація про стан програми, при --new - починається нова гра

**Хід роботи**

Гра містить файли: utils.h, utils.c, my_rand.h, my_rand.c, game.c.
У файлі utils.c описані функції для парсингу аргументів командного рядка. Обробкою аргументів займається функція parse_opt:

.. code-block:: bash

 int parse_opt(int key, char *arg, struct argp_state *state)
 {
	struct arguments *args = state->input;
	int i = 0;
	int len;
	args->parse_stat = NO_ERRORS;
	switch(key)
	{
	case 'v':
	args->verbose = 1;
	args->parse_stat = VERB_IS_ON;
	parse_check(args);
	break;
	case 'n':
	args->new_game = 1;
	args->parse_stat = NEW_GAME_START;
	parse_check(args);
	break;
	case ARGP_KEY_ARG:
	if(NULL == arg) goto skip;
	
	if(state->arg_num + 1 > 1) {
		args->parse_stat = TOO_MANY_ARGS;
		goto skip;
	}

	args->parse_stat = CHECK_ARGS;
	parse_check(args);

	len = strlen(arg);
	while((i < len) && (arg[i] >= 48) && (arg[i] <= 57)) i++;

	if(((i == len) && (i > 1)) || (strcmp(arg, "6")>0) || !strcmp(arg, "0")) {
		args->parse_stat = OUT_OF_RANGE;
		goto skip;
	} else if (i < len) {
		args->parse_stat = NEGATIVE_NUM;
		
		if(arg[i] != '-') args->parse_stat = NOT_A_NUM;

		goto skip;
	}
	args->parse_stat = CHECK_OK;
	
	args->num = atoi(arg);
 skip:
	parse_check(args);
	break;
	case ARGP_KEY_END:
	if(state->arg_num == 0) args->parse_stat = TOO_FEW_ARGS;
	parse_check(args);	
	break;
	}	
	args->parse_stat = NO_ERRORS;
	return 0;
 }

Для обробки помилок та повідомлень було написано функцію parse_check:

.. code-block:: bash

 static void parse_check(struct arguments *args)
 {
	fprintf(args->parse_stat >= 6 ? stdout : stderr, "%s%s%s",
		args->parse_stat >= 6 ? color(BLUE) : color(RED),
		args->verbose && args->parse_stat != 0 ? (args->parse_stat >= 6 ? "Parser: " :   "Error: ") : "",
	        (args->parse_stat < 6 || args->verbose) ? parse_err[args->parse_stat] : "");
	 if(args->parse_stat < 6 && args->parse_stat > 0)
		exit(args->parse_stat);
	 return;
 }


У файлі my_rand.c описані функції для генерації випадкових чисел від 1 до 6: init_rand, exit_rand, get_num.
Функція init_rand відкриває файли /dev/random і database.rst для подальшого зчитування випадкового біту та збереження поточного стану гри відповідно.
Функція exit_rand закриває файли /dev/random і database.rst. 
Функція get_num зчитує один байт, який був сгенерований у файлі /dev/random. 

.. code-block:: bash

 void get_num(struct my_rand *rn, struct arguments *args)
 {
	size_t read_stat = 1;
	rn->status = RND_NO_ERR;
	if (NULL == rn) 
		rn->status = RND_NO_ARG;
	
	status_check(rn, args);
	
	read_stat = fread(&(rn->rand_num), sizeof(rn->rand_num), 1, rn->_frnd);
	
	if(!read_stat) rn->status = RND_RDEV_F;
	else rn->status = RND_RDEV_OK;
	
	status_check(rn, args);
	
	if(args->verbose)
		fprintf(stdout, "\tRandom byte is %u\n", rn->rand_num);

	rn->rand_num %= 6;
	rn->rand_num++;

	return;
 }

Функція save_data зберігає поточний стан гри, а саме кількість очок у гравця і компьютера та кількість ігор.
Також при зчитуванні та записі у файл database.rst, який генерується через Makefile, виконується перевірка на успішність даних дій.

.. code-block:: bash
 
 void save_data(struct my_rand *rn, struct arguments *args)
 {
	uint8_t scores[3] = {0, 0, 0};
	size_t rw_stat;
	rn->status = RND_NO_ERR;
	if (NULL == rn)
		rn->status = RND_NO_ARG;
	
	status_check(rn, args);

	fseek(rn->_fdata, 0, SEEK_SET);

	if(!(args->new_game)) {
		rw_stat = fread(scores, sizeof(*scores), 3, rn->_fdata);
		if(scores[2] == 0) rw_stat = 3;
	}
	
	if(rw_stat < 3)	rn->status = RD_FAILURE;
	else rn->status = RD_OK;

	status_check(rn, args);

	if(args->verbose) {
		fprintf(stdout, "\tCurrent values are: %u, %u, %u\n",
				scores[0], scores[1], scores[2]);
	}

	rn->score_stat ? scores[1]++ : scores[0]++;
	scores[2]++;	// this is the number of played games
	
	fseek(rn->_fdata, 0, SEEK_SET);
	rw_stat = fwrite(scores, sizeof(*scores), 3, rn->_fdata);
	
	if(rw_stat < 3) rn->status = WD_FAILURE;
	else rn->status = WD_OK;

	status_check(rn, args);

	if(args->verbose) {
		fprintf(stdout, "\tNew values are: %u, %u, %u\n",
				scores[0], scores[1], scores[2]);
	}

	fprintf(stdout, "%sComputer scores:\tYour scores:\n%u\t\t\t%u\n"
			"Games played: %d\n",
			color(WHITE),
			scores[0], scores[1], scores[2]);

	return;
 }	

Функція status_check виконує обробку помилок та повідомлень:

.. code-block:: bash

 static void status_check(struct my_rand *rn, struct arguments *args)
 {
	fprintf(rn->status >= 7 ? stdout : stderr, "%s%s%s",
		rn->status >= 7 ? color(BLUE) : color(RED),
		args->verbose && rn->status != 0 ? (rn->status >= 7 ? "Rand: " : "Error: ") : "",
		(rn->status < 7 || args->verbose) ? rnd_err[rn->status] : "");
	if(rn->status < 7 && rn->status > 0) {
		fclose(rn->_frnd);
		rn->_frnd = NULL;
		fclose(rn->_fdata);
		rn->_fdata = NULL;
		exit(rn->status);
		}
	return;
 } 

**Результат роботи створеної програми**

Результат роботи програми при використанні прапора --help

.. code-block:: bash

 [vit@vm game]$ ./game.o --help
 Usage: game.o [OPTION...] Guess the number game
 Whelcome to the game!
 The main purpouse of this game is to guess a number,
 that is set by computer. Computer sets only positive
 numbers from 1 to 6, so should put only numbers, that
 fit in this diapazone. Good luck!

 Game options:
  -n, --new                  Start a new game

 Informational options:
  -v, --verbose              Give extra info

  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version
 [vit@vm game]$ 

Результат роботи при усіх можливих невірних комбінаціях вводу аргументів, останній ввод правильний:

.. code-block:: bash

 [vit@vm game]$ ./game.o -- -54f 2
 Number is negative!
 [vit@vm game]$ ./game.o -- 54f 2
 Not a number!
 [vit@vm game]$ ./game.o -- 54 2
 Number is out of range!
 [vit@vm game]$ ./game.o -- 5 2
 Too many arguments!
 [vit@vm game]$ ./game.o --
 Too few arguments!
 [vit@vm game]$ ./game.o -- 5
 Your number: 5
 Computer number: 6
 You missed
 Computer scores:	Your scores:
 2			0
 Games played: 2
 [vit@vm game]$ 


Результат роботи програми з використаним прапором --verbose:

.. code-block:: bash

 [vit@vm game]$ ./game.o --verbose 5
 Parser: Verbosity option is turned on
 Parser: Checking argument
 Parser: Argument has passed successfuly!
 Rand: "/dev/random" was opened successfuly
 Rand: "database" was opened successfuly
 Rand: Read from "/dev/random" was successful
	Random byte is 20
 Your number: 5
 Computer number: 3
 You missed
 Rand: Read from "database" was successful
	Current values are: 2, 0, 2
 Rand: Write to "database" was successful
	New values are: 3, 0, 3
 Computer scores:	Your scores:
 3			0
  Games played: 3
 [vit@vm game]$ 


Як видно, було виведено детальну інформацію про кожну дію програми.

Результат роботи програми з використаним прапором --new:

.. code-block:: bash

 Parser: Verbosity option is turned on
 Parser: New game was started
 Parser: Checking argument
 Parser: Argument has passed successfuly!
 Rand: "/dev/random" was opened successfuly
 Rand: "database" was opened successfuly
 Rand: Read from "/dev/random" was successful
	Random byte is 212
 Your number: 5
 Computer number: 3
 You missed
 Rand: Read from "database" was successful
	Current values are: 0, 0, 0
 Rand: Write to "database" was successful
	New values are: 1, 0, 1
 Computer scores:	Your scores:
 1			0
 Games played: 1
 [vit@vm game]$ 

Як видно, було виведено повідомлення про те, що було розпочато нову гру "Parser: New game was started". Також було анульовано усі попередні результати гри та записани нові.

**Використані бібліотеки**

* ``<argp.h>`` - потрібна для парсингу аргументів командного рядка   
* ``<stdio.h>`` - потрібна для роботи з файлами та виводу інформації у консоль
* ``<stdlib.h>`` - потрібна для виходу з програми
* ``<stdint.h>`` - потрібна для використання типу uint8_t 
* ``<string.h>`` - потрібна для роботи з рядками

**Використані функції**

* ``argp_parse`` - ініціалізація парсингу
* ``strcmp`` - порівняння двох рядків між собою
* ``strlen`` - отримання значення довжини рядка
* ``fopen`` - відкріття фалу
* ``fclose`` - закріття файлу
* ``fread`` - зчитування з фалу
* ``fwrite`` - запис у файл
* ``fprintf`` - вивид інформації у потік

Висновки:
-------------

В результаті виконання даної контрольної роботи, було написано гру "вгадай число", яка приймає та оброблює аргументи командного рядка. У результаті перевірки роботи програми було з'ясовано, що програма працює коректно.
