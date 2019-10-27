#!/bin/bash -p
scp ./src/hivemod.c debian@192.168.6.2:/home/debian/lab5/src/hivemod.c
scp ./test_prog.c debian@192.168.6.2:/home/debian/lab5/test_prog.c
scp ./Makefile debian@192.168.6.2:/home/debian/lab5/Makefile
scp ./test_script.sh debian@192.168.6.2:/home/debian/lab5/test_script.sh
ssh debian@192.168.6.2 "cd /home/debian/lab5; make"
ssh debian@192.168.6.2 "cd /home/debian/lab5; gcc test_prog.c -o test_prog"
