ДКР
===

Завдання:
---------

Програма приймає довільну кількість координат X Y для точок
і виводить на координати центральної (рівновіддаленої) точки,
а також якщо вказаний флаг --dist відстань до них.

При використані флагу --verbose програма детально виводить дії які виконує.

 Хід роботи
------------

Програма складається з двох частин:

1) Обробник аргументів командного рядка *utils.c*
2) Основна програма для обрахунків точок *midp.c*


**Алгоритм**

1) Отримати від користувача набір точок;
2) Перевірити параметри на коректність: чи введені цифри, чи введені символи. Має бути мінімум дві точки, кількість координат має бути кратним двійці, щоб сформувати точки;
3) Видалити продубльовані точки;
4) Якщо у нас всього дві точки, знайти середню точку;
5) Якщо точок більше ніж дві: знайти коло по трьом точкам; (Якщо трикутник розгорнутий, спробувати створити трикутник з іншими точками, якщо не вийшло - помилка: "Не існує рівновіддаленої точки");
6) Якщо трикутник можливий, знайти координати описаного кола і його радіус;
7) Перевірити чи лежать інші точки, на цьому колі, за допомогою перевірки відстані до центра кола, якщо збігається з його радіусом, то точки лежать на ньому, інакше помилка: "Не існує рівновіддаленої точки");
8) Вивести результат на екран.


**Реалізація**

*Функція обробник параметрів:*
-v або -verbose - вивести додаткову інформацію на екран
-d або -dist - вивести відстань до рівновіддаленої точки на екран

Для того щоб вводити координати рекомендується писати -- перед переліком чисел, щоб парсер корректно зчитав від'ємні числа.

Приклад використання:
/midp.o  -v -dist -- 0 2 2 0 -2 0

        .. code-block:: bash

        error_t parse_opt(int key, char *arg, struct argp_state *state)
        {
                enum PARSE_STATUS parse_stat = _PARSE_OK;
                enum ATOI_STATUS atoi_status = _ATOI_OK;

                struct Parse_Args_Global *arguments = state->input;      // get struct as parameter via *state
                switch(key)
                {
                        case 'v':
                                arguments->verbose = 1;
                                break;
                        case 'd':
                                arguments->dist = 1;
                                break;
                        case ARGP_KEY_ARG:
                                if(arguments->verbose ==1){
                                        fprintf(stdout,"arguments = %s\n",arg);
                                }
                                atoi_status = addNewCoordinate(arguments,arg);

                                if(atoi_status){
                                    parse_stat =  _PARSE_ERR;
                                    fprintf(stderr, "Coordinate arguments should be integers!\n");
                                    return ARGP_KEY_ERROR;
                                }

                                break;
                        case ARGP_KEY_END:
                                if (state->arg_num < 1){
                                        argp_failure (state, 1, 0, "Too few arguments");
                                        argp_usage (state);
                                }
                                break;
                        default:
                                return ARGP_ERR_UNKNOWN;
                }

            return parse_stat;
        }


*Cтруктура з параметрами*
Використовується midp.c для отримання параметрів з командного рядка за допомогою utils.c

        .. code-block:: bash

        struct Parse_Args_Global
        {
                int coordinates[256];
                int dist;
                int verbose;
                int size;
        };


*Приклад розрахунків*
Розрахунки для двох точок розраховуються за простою формулою: sqrt((x1-x2)^2 + (y1-y2)^2)
        .. code-block:: bash

        struct MiddlePoint twoPoints(int *arr){
                struct MiddlePoint coordinates;

                double x = (arr[0] + arr[2]) / 2;
                double y = (arr[1] + arr[3]) / 2;
                double length = sqrt((arr[0] - arr[2]) * (arr[0] - arr[2]) + (arr[1] - arr[3]) * (arr[1] - arr[3]));

                coordinates.x = x;
                coordinates.y = y;
                coordinates.length = length / 2;

                return coordinates;
        }


А для розрахунків для точок більше за 2 використовується рівняння кола

        .. code-block:: bash

        struct MiddlePoint calcCircle(int *arr, int arr_len){

        struct MiddlePoint coordinates;
        double x, y, length;



        // from circle equation
        double a, b, c, d, e, f, g;
        int i = 0;
        int on_same_plane = 1;



        a = arr[2] - arr[0]; // x1 - x0
        b = arr[3] - arr[1]; // y1 - y0

        e = a * (arr[2] + arr[0]) + b * (arr[3] + arr[1]);

        while(on_same_plane){          // iterate throw all dots if g == 0 to find non 0
                c = arr[4 + i * 2] - arr[0]; // x2 - x0, then xn(n >= 2) - x0, if g == 0
                d = arr[5 + i * 2] - arr[1]; // y2 - y0, then yn(n >= 2) - y0, if g == 0

                f = c * (arr[4 + i * 2] + arr[0]) + d * (arr[5 + i * 2] + arr[1]);

                g = 2 * (a * (arr[5 + i * 2] - arr[3])) - b * (arr[4 + i * 2] - arr[2]);

                if(g == 0){
                        on_same_plane = 1;
                        i++;
                        if(i > ((arr_len / 2) - 3)){
                                coordinates.x = 0;
                                coordinates.y = 0;
                                coordinates.length = -1;        // error
                                return coordinates;
                        }
                }else{
                        on_same_plane = 0;
                        x = (d * e - b * f) / g;
                        y = (a * f - c * e) / g;

                        length = sqrt(((arr[0] - x)*(arr[0] - x))+((arr[1] - y)*(arr[1] - y)));

                        break;
                }
        }

        coordinates.x = x;
        coordinates.y = y;
        coordinates.length = length;

        return coordinates;
        }


**Приклад роботи**

Виведемо повну інформацію для точок Х1(0,2) Х2(2,0) Х3(-2, 0). У них рівновіддалена точка має бути в координатах 0, 0, а відстань 2.

        .. code-block:: bash

        ./midp.o -v  -dist -- 0 2 2 0 -2 0                                                                                                                    INT ✘  
        arguments = 0
        arguments = 2
        arguments = 2
        arguments = 0
        arguments = -2
        arguments = 0
        Dist = 1
        Verbose = 1
        Size = 6
        args->coordinates[0] = 0
        args->coordinates[1] = 2
        args->coordinates[2] = 2
        args->coordinates[3] = 0
        args->coordinates[4] = -2
        args->coordinates[5] = 0
        points = 3
        xi = 0, yi = 2
        xj = 2, yj = 0, swap_to_prev = 0
        points = 3
        xi = 0, yi = 2
        xj = -2, yj = 0, swap_to_prev = 0
        points = 3
        xi = 2, yi = 0
        xj = -2, yj = 0, swap_to_prev = 0
        Dist = 1
        Verbose = 1
        Size = 6
        args->coordinates[0] = 0
        args->coordinates[1] = 2
        args->coordinates[2] = 2
        args->coordinates[3] = 0
        args->coordinates[4] = -2
        args->coordinates[5] = 0
        Point[0] x = 0, y = 2
        Center: x = -0.000000, y = -0.000000
        length calc = 2.000000, length circle = 2.000000
        Point[1] x = 2, y = 0
        Center: x = -0.000000, y = -0.000000
        length calc = 2.000000, length circle = 2.000000
        Point[2] x = -2, y = 0
        Center: x = -0.000000, y = -0.000000
        length calc = 2.000000, length circle = 2.000000
        Middle Point: X = -0.000000, Y = -0.000000
        Middle Point: distance = 2.000000

Як бачимо, програма порахувала вірно.


Простий приклад для двох точок:

        .. code-block:: bash

        ./midp.o -dist -- 0 0 10 0                                                                                                                                ✔  
        Middle Point: X = 5.000000, Y = 0.000000
        Middle Point: distance = 5.000000

Розглянемо приклад для точок Х1(0,0) Х2(10,0) Х3(5,0) Х4(-2,1).
Програма має видати помилку, оскільки три точки лежать на одній лінії, а отже не можна побудувати трикутник яки буде проходити через усі точки.

        .. code-block:: bash

        ./midp.o -v -dist -- 0 0 10 0 5 0 -2 1
        arguments = 0
        arguments = 0
        arguments = 10
        arguments = 0
        arguments = 5
        arguments = 0
        arguments = -2
        arguments = 1
        Dist = 1
        Verbose = 1
        Size = 8
        args->coordinates[0] = 0
        args->coordinates[1] = 0
        args->coordinates[2] = 10
        args->coordinates[3] = 0
        args->coordinates[4] = 5
        args->coordinates[5] = 0
        args->coordinates[6] = -2
        args->coordinates[7] = 1
        points = 4
        xi = 0, yi = 0
        xj = 10, yj = 0, swap_to_prev = 0
        points = 4
        xi = 0, yi = 0
        xj = 5, yj = 0, swap_to_prev = 0
        points = 4
        xi = 0, yi = 0
        xj = -2, yj = 1, swap_to_prev = 0
        points = 4
        xi = 10, yi = 0
        xj = 5, yj = 0, swap_to_prev = 0
        points = 4
        xi = 10, yi = 0
        xj = -2, yj = 1, swap_to_prev = 0
        points = 4
        xi = 5, yi = 0
        xj = -2, yj = 1, swap_to_prev = 0
        Dist = 1
        Verbose = 1
        Size = 8
        args->coordinates[0] = 0
        args->coordinates[1] = 0
        args->coordinates[2] = 10
        args->coordinates[3] = 0
        args->coordinates[4] = 5
        args->coordinates[5] = 0
        args->coordinates[6] = -2
        args->coordinates[7] = 1
        Point[0] x = 0, y = 0
        Center: x = 5.000000, y = 12.500000
        length calc = 9.354143, length circle = 13.462912
        One or more point don`t form cyclic polygon, no equidistant point for such set of points!

Як і планувалося, програма видала помилку

Приклад без флага -d

        .. code-block:: bash

        ./midp.o 4 5 8 6                                                                                                                                          ✔  
        Middle Point: X = 6.000000, Y = 5.500000
