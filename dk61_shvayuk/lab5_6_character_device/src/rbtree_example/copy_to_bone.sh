#!/bin/bash -p
scp ./rbtree_example_mod.c debian@192.168.6.2:/home/debian/lab5/rbtree_test/rbtree_example_mod.c
scp ./Makefile debian@192.168.6.2:/home/debian/lab5/rbtree_test/Makefile
scp ./test_script.sh debian@192.168.6.2:/home/debian/lab5/rbtree_test/test_script.sh
ssh debian@192.168.6.2 "cd /home/debian/lab5/rbtree_test/; make"
