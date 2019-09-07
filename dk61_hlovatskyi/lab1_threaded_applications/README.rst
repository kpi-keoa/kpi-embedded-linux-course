===========================
**Lab1: Threaded applications**
===========================

**Description** 

That program creates N threads and increment global variable by standalone function.

For compiling this code use commnad 'make' <argument> from directory where Makefile is located.
For showing executed time use command 'time' <argument>, 
where <argument> is output file, which was created after compilation.

**Explaining**
Results depends on compilatiom flags which are used during compilation.
Whether it's [-O0] - result is incorrect, executed time compare to O2
but with [-O2] - result is great, you can check the differences in asm code.

Wrong results happens 'cause two threads interrupts each other.
It fixes with mutex lock, which guarantees a correct result.




**Contacts**
You can also contact me:
**Telegram:** @Bramory <https://tg.me/Bramory>

