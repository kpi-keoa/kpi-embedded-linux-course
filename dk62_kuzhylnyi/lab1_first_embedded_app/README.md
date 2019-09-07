Multithread application 
==================

Calculate both **volatile** and **non-volatile** int variable between 2 Posix threads (ptreads).

The goal is to check the differents in calculation result between 2 different architectures: x86_64 and ARM.

Synopsis
--------

```
# make [all|clean|help]
# ./emb_uthreads
```
**O0 gcc optimization x86_64 | ARM**
![](doc/img/O0_thread.png)

**O2 gcc optimization x86_64 | ARM**
![](doc/img/O2_thread.png)
