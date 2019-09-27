#!/bin/bash -p
scp ./lab3_module.c debian@192.168.6.2:/home/debian/lab3/lab3_module.c
scp ./Makefile debian@192.168.6.2:/home/debian/lab3/Makefile
scp ./test_script.sh debian@192.168.6.2:/home/debian/lab3/test_script.sh
ssh debian@192.168.6.2 "cd /home/debian/lab3; make"
