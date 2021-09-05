#!/bin/bash -p
scp ./lab4_module.c debian@192.168.6.2:/home/debian/lab4/lab4_module.c
scp ./Makefile debian@192.168.6.2:/home/debian/lab4/Makefile
scp ./test_script.sh debian@192.168.6.2:/home/debian/lab4/test_script.sh
ssh debian@192.168.6.2 "cd /home/debian/lab4; make"
