==========================================================
**Домашня контрольна робота**
==========================================================

**Завдання:**
~~~~~~~~~~~~~

* Написати програму, яка приймає довільну кількість координат X Y для точок і виводить координати центральної (рівновіддаленої від інших) точки, а також, якщо вказано прапор --dist, відстань до них. 

**Хід роботи:**
~~~~~~~~~~~~~~~

Программа приймає аргументи командного рядка. Для парсингу використовується Argp.
Використовуєтья параметр --verbose (-v), що змушує програму виводити результат з детальною інформацією. А також параметр --dist, що показує відстань до рівновідалленої точки від заданих точок.

.. code-block:: c

	static struct argp_option options[] = {
		{"verbose",     'v',    0,      OPTION_ALIAS,    "Produce verbose output" }, 
		{"dist",        'd',    0,      OPTION_ALIAS,    "Distance to the equidistant point" }, 
		{ 0 }
	};

	error_t parse_opt(int key, char *arg, struct argp_state *state)
	{
		struct arguments *arguments = state -> input;

		switch (key) {
		case 'v':
			arguments -> verbose = 1;
			break;
		case 'd':
			arguments -> dist = 1;
			break;
		case ARGP_KEY_ARG:
			arguments -> args[state -> arg_num] = arg;
			break;
		case ARGP_KEY_END:
			if (state -> arg_num < 4 || state -> arg_num % 2 != 0) {
				fprintf(stderr, "Not enough arguments.\n", arg);
				argp_usage(state);
				break;
			}
		default:
			return ARGP_ERR_UNKNOWN;
		}
		return 0;
	}

Після зчитування аргументів з командного рядку значення координаат записуються в масиви x та y за допомогою функції atoi():

.. code-block:: c

	x[i] = atoi(arguments.args[j]);
	y[i] = atoi(arguments.args[j + 1]);

Далі виконується перевірка можливості створення трикутників з усіх точок, для того щоб визначити чи якісь точки не лежать поміж двома іншими на одній ліній, або чи точки не співпадають.

.. code-block:: c

	static void check_triangle(float *x, float *y, int size)
	{
		int i, j, l;
		float k, b;

		for (l = 0; l < size; l++) {
			for (i = 0; i < size; i++) {
				for (j = 0; j < size; j++) {
					k = (y[i] - y[j]) / (x[i] - x[j]);
					b = y[j] - k * x[j];
					if ((y[l] == k * x[l] + b) && (l != i) && (l != j)) {
						fprintf(stderr, "There are points that coincide, "
							"or are on the same line\n");
						exit(0);
					}
				}
			}
		}
	}

Виконується розрахунок рівновідаленої точки від введених точок.
Якщо точок 2, то використовується проста формула середнього арифметичного:

.. code-block:: c

	static float* equidist_point_twop(float *x, float *y)
	{
		int i;

		for (i = 0; i < 2; i++) {
			equdist_point[1] = equdist_point[1] + x[i] / 2;
			equdist_point[2] = equdist_point[2] + y[i] / 2;
		}

		    return equdist_point;
	}

Якщо точок 3, то використовується формула радіусу описаного кола (x - a)^2 + (y - b)^2 = R^2, з якої виводяться координати центру цього кола, яка і є рівновіддаленою точкою.
При кількості точок більшою чим 3, після розрахунку координат центру описаного кола, перевіряється чи належать цьому описаному колу інші точки шляхом перевірки довжини до центру кола.

.. code-block:: c

	static float* equidist_point_threep(float *x, float *y)
	{
		equdist_point[1] = -((y[0] - y[1]) * (pow(x[2], 2) + pow(y[2], 2)) 
			+ (y[1] - y[2]) * (pow(x[0], 2) + pow(y[0], 2)) 
			+ (y[2] - y[0]) * (pow(x[1], 2) + pow(y[1], 2))) 
			/ (2 * ((x[0] - x[1]) * (y[2] - y[0]) - (y[0] - y[1]) * (x[2] - x[0])));
		equdist_point[2] = ((x[0] - x[1]) * (pow(x[2], 2) + pow(y[2], 2)) 
			+ (x[1] - x[2]) * (pow(x[0], 2) + pow(y[0], 2)) 
			+ (x[2] - x[0]) * (pow(x[1], 2) + pow(y[1], 2))) 
			/ (2 * ((x[0] - x[1]) * (y[2] - y[0]) - (y[0] - y[1]) * (x[2] - x[0])));

		    return equdist_point;
	}
	
	static void check_points(float *x, float *y, int size)
	{
		int i;

		for (i = 3; i < size; i++) {
			if (sqrt(pow((equdist_point[1] - x[i]), 2) 
					+ pow((equdist_point[2] - y[i]), 2)) != d) {
				fprintf(stderr, "Equidistant point doesn't exist\n");
				exit(0);
			}
		}
	}

Якщо точка не належить колу то, виводиться інформація про те, що рівновіддаленої точки не існує.
В іншому випадку виводяться координати рівновіддаленої точки. В залежності від параметру dist виводиться додатково відстань до цієї точки, а в залежності від параметру verbose - додаткова інформація.
Результат виводиться в stdout, а помилки – в stderr.

.. code-block:: c

	static void print_results(int verbose_flag, int dist_flag)
	{
		if (!verbose_flag) {
			fprintf(stdout, "%f %f\n", equdist_point[1], equdist_point[2]);
			if (dist_flag) {
				fprintf(stdout, "%f", d);
			}
		} else {
			fprintf(stdout, "Coordinates of the equidistant point: x = %f, y = %f\n", 
				equdist_point[1], equdist_point[2]);
			if (dist_flag) {
				fprintf(stdout, "Distance to the equidistant point: %f", d);
			}
		}
	}

Збирається програма за допомогою Make файлу. Makefile містить додаткові цілі build та clean, а також прапор -lm для завантаження бібліотеки math.

.. code-block:: bash

	.PHONY: clean all

	all: build

	build:
		gcc -o main.o src/main.c src/utils.c -lm
		
	clean:
		rm *.o

**Отримані результати:**
~~~~~~~~~~~~~~~

Приклад для 2 точок:

.. code-block:: bash

	./main.o 1 2 2 1
	1.500000 0.707107

Приклад для 3 точок з виведенням відстані до рівновіддаленої точки:

.. code-block:: bash

	./main.o -d 3 1 7 2 4 3
	5.071429 1.214286
	2.082483

Приклад для 4 точок з виведенням відстані та додаткової інформації:

.. code-block:: bash

	./main.o -v -d -- 4 2 -4 2 4 -2 -4 -2
	Coordinates of the equidistant point: x = 0.000000, y = -0.000000
	Distance to the equidistant point: 4.472136


Приклад з неможливістю рівновіддаленої точки:

.. code-block:: bash

	./main.o -v -d -- 5 2 3 8 4 -2 -4 -2
	Equidistant point doesn't exist

