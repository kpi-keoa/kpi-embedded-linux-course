	.file	"threads.c"
# GNU C17 (GCC) version 10.2.0 (x86_64-pc-linux-gnu)
#	compiled by GNU C version 10.2.0, GMP version 6.2.0, MPFR version 4.1.0, MPC version 1.1.0, isl version isl-0.21-GMP

# warning: MPFR header version 4.1.0 differs from library version 4.0.2.
# GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
# options passed:  -D_REENTRANT src/threads.c -march=x86-64 -mtune=generic
# -auxbase-strip build/threads.c -O0 -Wall -Wextra -Wpedantic -fverbose-asm
# options enabled:  -fPIC -fPIE -faggressive-loop-optimizations
# -fallocation-dce -fasynchronous-unwind-tables -fauto-inc-dec
# -fdelete-null-pointer-checks -fdwarf2-cfi-asm -fearly-inlining
# -feliminate-unused-debug-symbols -feliminate-unused-debug-types
# -ffp-int-builtin-inexact -ffunction-cse -fgcse-lm -fgnu-unique -fident
# -finline-atomics -fipa-stack-alignment -fira-hoist-pressure
# -fira-share-save-slots -fira-share-spill-slots -fivopts
# -fkeep-static-consts -fleading-underscore -flifetime-dse -fmath-errno
# -fmerge-debug-strings -fpeephole -fplt -fprefetch-loop-arrays
# -freg-struct-return -fsched-critical-path-heuristic
# -fsched-dep-count-heuristic -fsched-group-heuristic -fsched-interblock
# -fsched-last-insn-heuristic -fsched-rank-heuristic -fsched-spec
# -fsched-spec-insn-heuristic -fsched-stalled-insns-dep -fschedule-fusion
# -fsemantic-interposition -fshow-column -fshrink-wrap-separate
# -fsigned-zeros -fsplit-ivs-in-unroller -fssa-backprop
# -fstack-protector-strong -fstdarg-opt -fstrict-volatile-bitfields
# -fsync-libcalls -ftrapping-math -ftree-cselim -ftree-forwprop
# -ftree-loop-if-convert -ftree-loop-im -ftree-loop-ivcanon
# -ftree-loop-optimize -ftree-parallelize-loops= -ftree-phiprop
# -ftree-reassoc -ftree-scev-cprop -funit-at-a-time -funwind-tables
# -fverbose-asm -fzero-initialized-in-bss -m128bit-long-double -m64 -m80387
# -malign-stringops -mavx256-split-unaligned-load
# -mavx256-split-unaligned-store -mfancy-math-387 -mfp-ret-in-387 -mfxsr
# -mglibc -mieee-fp -mlong-double-80 -mmmx -mno-sse4 -mpush-args -mred-zone
# -msse -msse2 -mstv -mtls-direct-seg-refs -mvzeroupper

	.text
	.globl	gl_var
	.bss
	.align 8
	.type	gl_var, @object
	.size	gl_var, 8
gl_var:
	.zero	8
	.text
	.globl	func
	.type	func, @function
func:
.LFB6:
	.cfi_startproc
	pushq	%rbp	#
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp	#,
	.cfi_def_cfa_register 6
	subq	$48, %rsp	#,
	movq	%rdi, -40(%rbp)	# the_val, the_val
# src/threads.c:17:   type_cnt val_n = ((struct var *)the_val)->num;
	movq	-40(%rbp), %rax	# the_val, tmp85
	movq	(%rax), %rax	# MEM[(struct var *)the_val_6(D)].num, tmp86
	movq	%rax, -16(%rbp)	# tmp86, val_n
# src/threads.c:18:   type_cnt val_cnt = ((struct var *)the_val)->count;
	movq	-40(%rbp), %rax	# the_val, tmp87
	movq	8(%rax), %rax	# MEM[(struct var *)the_val_6(D)].count, tmp88
	movq	%rax, -8(%rbp)	# tmp88, val_cnt
# src/threads.c:20:   for (type_cnt i = 0; i < val_cnt; i++) {
	movq	$0, -24(%rbp)	#, i
# src/threads.c:20:   for (type_cnt i = 0; i < val_cnt; i++) {
	jmp	.L2	#
.L3:
# src/threads.c:21:     gl_var += val_n;
	movq	gl_var(%rip), %rdx	# gl_var, gl_var.0_1
	movq	-16(%rbp), %rax	# val_n, tmp89
	addq	%rdx, %rax	# gl_var.0_1, _2
	movq	%rax, gl_var(%rip)	# _2, gl_var
# src/threads.c:20:   for (type_cnt i = 0; i < val_cnt; i++) {
	addq	$1, -24(%rbp)	#, i
.L2:
# src/threads.c:20:   for (type_cnt i = 0; i < val_cnt; i++) {
	movq	-24(%rbp), %rax	# i, tmp90
	cmpq	-8(%rbp), %rax	# val_cnt, tmp90
	jb	.L3	#,
# src/threads.c:23:   pthread_exit(0);
	movl	$0, %edi	#,
	call	pthread_exit@PLT	#
	.cfi_endproc
.LFE6:
	.size	func, .-func
	.section	.rodata
.LC0:
	.string	"%d\"%c\""
	.text
	.globl	check_in
	.type	check_in, @function
check_in:
.LFB7:
	.cfi_startproc
	pushq	%rbp	#
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp	#,
	.cfi_def_cfa_register 6
	subq	$32, %rsp	#,
	movq	%rdi, -24(%rbp)	# arg, arg
	movq	%rsi, -32(%rbp)	# res, res
# src/threads.c:26: bool check_in(char *arg, type_cnt *res) {
	movq	%fs:40, %rax	# MEM[(<address-space-1> long unsigned int *)40B], tmp92
	movq	%rax, -8(%rbp)	# tmp92, D.3397
	xorl	%eax, %eax	# tmp92
# src/threads.c:29:   if (1 != sscanf(arg, "%d\"%c\"", &result, &unused))
	leaq	-13(%rbp), %rcx	#, tmp87
	leaq	-12(%rbp), %rdx	#, tmp88
	movq	-24(%rbp), %rax	# arg, tmp89
	leaq	.LC0(%rip), %rsi	#,
	movq	%rax, %rdi	# tmp89,
	movl	$0, %eax	#,
	call	__isoc99_sscanf@PLT	#
# src/threads.c:29:   if (1 != sscanf(arg, "%d\"%c\"", &result, &unused))
	cmpl	$1, %eax	#, _1
	je	.L5	#,
# src/threads.c:30:     return 1;
	movl	$1, %eax	#, _4
	jmp	.L7	#
.L5:
# src/threads.c:32:     *res = result;
	movl	-12(%rbp), %eax	# result, result.1_2
	movslq	%eax, %rdx	# result.1_2, _3
	movq	-32(%rbp), %rax	# res, tmp90
	movq	%rdx, (%rax)	# _3, *res_9(D)
# src/threads.c:33:   return 0;
	movl	$0, %eax	#, _4
.L7:
# src/threads.c:34: }
	movq	-8(%rbp), %rsi	# D.3397, tmp93
	subq	%fs:40, %rsi	# MEM[(<address-space-1> long unsigned int *)40B], tmp93
	je	.L8	#,
# src/threads.c:34: }
	call	__stack_chk_fail@PLT	#
.L8:
	leave	
	.cfi_def_cfa 7, 8
	ret	
	.cfi_endproc
.LFE7:
	.size	check_in, .-check_in
	.section	.rodata
	.align 8
.LC1:
	.string	"%s : wrrong amount of arguments : %d ; should be 2 \n"
.LC2:
	.string	"\nWrong input types\n"
.LC3:
	.string	"Expected value: %lu \n"
.LC4:
	.string	"Real value: %lu \n"
	.text
	.globl	main
	.type	main, @function
main:
.LFB8:
	.cfi_startproc
	pushq	%rbp	#
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp	#,
	.cfi_def_cfa_register 6
	subq	$64, %rsp	#,
	movl	%edi, -52(%rbp)	# argc, argc
	movq	%rsi, -64(%rbp)	# argv, argv
# src/threads.c:36: int main(int argc, char *argv[]) {
	movq	%fs:40, %rax	# MEM[(<address-space-1> long unsigned int *)40B], tmp121
	movq	%rax, -8(%rbp)	# tmp121, D.3400
	xorl	%eax, %eax	# tmp121
# src/threads.c:40:   if (argc != 3) {
	cmpl	$3, -52(%rbp)	#, argc
	je	.L10	#,
# src/threads.c:41:     fprintf(stderr, "%s : wrrong amount of arguments : %d ; should be 2 \n",
	movl	-52(%rbp), %eax	# argc, tmp106
	leal	-1(%rax), %ecx	#, _1
	movq	-64(%rbp), %rax	# argv, tmp107
	movq	(%rax), %rdx	# *argv_29(D), _2
	movq	stderr(%rip), %rax	# stderr, stderr.2_3
	leaq	.LC1(%rip), %rsi	#,
	movq	%rax, %rdi	# stderr.2_3,
	movl	$0, %eax	#,
	call	fprintf@PLT	#
# src/threads.c:43:     exit(1);
	movl	$1, %edi	#,
	call	exit@PLT	#
.L10:
# src/threads.c:46:   if (check_in(argv[1], &used_var.num) || check_in(argv[2], &used_var.count)) {
	movq	-64(%rbp), %rax	# argv, tmp108
	addq	$8, %rax	#, _4
# src/threads.c:46:   if (check_in(argv[1], &used_var.num) || check_in(argv[2], &used_var.count)) {
	movq	(%rax), %rax	# *_4, _5
	leaq	-32(%rbp), %rdx	#, tmp109
	movq	%rdx, %rsi	# tmp109,
	movq	%rax, %rdi	# _5,
	call	check_in	#
# src/threads.c:46:   if (check_in(argv[1], &used_var.num) || check_in(argv[2], &used_var.count)) {
	testb	%al, %al	# _6
	jne	.L11	#,
# src/threads.c:46:   if (check_in(argv[1], &used_var.num) || check_in(argv[2], &used_var.count)) {
	movq	-64(%rbp), %rax	# argv, tmp110
	addq	$16, %rax	#, _7
# src/threads.c:46:   if (check_in(argv[1], &used_var.num) || check_in(argv[2], &used_var.count)) {
	movq	(%rax), %rax	# *_7, _8
	leaq	-32(%rbp), %rdx	#, tmp111
	addq	$8, %rdx	#, tmp112
	movq	%rdx, %rsi	# tmp112,
	movq	%rax, %rdi	# _8,
	call	check_in	#
# src/threads.c:46:   if (check_in(argv[1], &used_var.num) || check_in(argv[2], &used_var.count)) {
	testb	%al, %al	# _9
	je	.L12	#,
.L11:
# src/threads.c:48:     fprintf(stderr, "\nWrong input types\n");
	movq	stderr(%rip), %rax	# stderr, stderr.3_10
	movq	%rax, %rcx	# stderr.3_10,
	movl	$19, %edx	#,
	movl	$1, %esi	#,
	leaq	.LC2(%rip), %rdi	#,
	call	fwrite@PLT	#
# src/threads.c:49:     exit(1);
	movl	$1, %edi	#,
	call	exit@PLT	#
.L12:
# src/threads.c:52:   pthread_t *threads = (pthread_t *)malloc(2 * sizeof(pthread_t));
	movl	$16, %edi	#,
	call	malloc@PLT	#
	movq	%rax, -40(%rbp)	# tmp113, threads
# src/threads.c:54:   for (int i = 0; i < 2; i++) {
	movl	$0, -48(%rbp)	#, i
# src/threads.c:54:   for (int i = 0; i < 2; i++) {
	jmp	.L13	#
.L14:
# src/threads.c:56:     pthread_create(&(threads[i]), NULL, &func, &used_var);
	movl	-48(%rbp), %eax	# i, tmp114
	cltq
	leaq	0(,%rax,8), %rdx	#, _12
# src/threads.c:56:     pthread_create(&(threads[i]), NULL, &func, &used_var);
	movq	-40(%rbp), %rax	# threads, tmp115
	leaq	(%rdx,%rax), %rdi	#, _13
	leaq	-32(%rbp), %rax	#, tmp116
	movq	%rax, %rcx	# tmp116,
	leaq	func(%rip), %rdx	#,
	movl	$0, %esi	#,
	call	pthread_create@PLT	#
# src/threads.c:54:   for (int i = 0; i < 2; i++) {
	addl	$1, -48(%rbp)	#, i
.L13:
# src/threads.c:54:   for (int i = 0; i < 2; i++) {
	cmpl	$1, -48(%rbp)	#, i
	jle	.L14	#,
# src/threads.c:59:   for (int i = 0; i < 2; i++)
	movl	$0, -44(%rbp)	#, i
# src/threads.c:59:   for (int i = 0; i < 2; i++)
	jmp	.L15	#
.L16:
# src/threads.c:60:     pthread_join(threads[i], NULL);
	movl	-44(%rbp), %eax	# i, tmp117
	cltq
	leaq	0(,%rax,8), %rdx	#, _15
	movq	-40(%rbp), %rax	# threads, tmp118
	addq	%rdx, %rax	# _15, _16
# src/threads.c:60:     pthread_join(threads[i], NULL);
	movq	(%rax), %rax	# *_16, _17
	movl	$0, %esi	#,
	movq	%rax, %rdi	# _17,
	call	pthread_join@PLT	#
# src/threads.c:59:   for (int i = 0; i < 2; i++)
	addl	$1, -44(%rbp)	#, i
.L15:
# src/threads.c:59:   for (int i = 0; i < 2; i++)
	cmpl	$1, -44(%rbp)	#, i
	jle	.L16	#,
# src/threads.c:62:   free(threads);
	movq	-40(%rbp), %rax	# threads, tmp119
	movq	%rax, %rdi	# tmp119,
	call	free@PLT	#
# src/threads.c:64:   printf("Expected value: %lu \n", 2 * used_var.num * used_var.count);
	movq	-32(%rbp), %rdx	# used_var.num, _18
# src/threads.c:64:   printf("Expected value: %lu \n", 2 * used_var.num * used_var.count);
	movq	-24(%rbp), %rax	# used_var.count, _19
# src/threads.c:64:   printf("Expected value: %lu \n", 2 * used_var.num * used_var.count);
	imulq	%rdx, %rax	# _18, _20
# src/threads.c:64:   printf("Expected value: %lu \n", 2 * used_var.num * used_var.count);
	addq	%rax, %rax	# _21
	movq	%rax, %rsi	# _21,
	leaq	.LC3(%rip), %rdi	#,
	movl	$0, %eax	#,
	call	printf@PLT	#
# src/threads.c:65:   printf("Real value: %lu \n", gl_var);
	movq	gl_var(%rip), %rax	# gl_var, gl_var.4_22
	movq	%rax, %rsi	# gl_var.4_22,
	leaq	.LC4(%rip), %rdi	#,
	movl	$0, %eax	#,
	call	printf@PLT	#
# src/threads.c:67:   return 0;
	movl	$0, %eax	#, _40
# src/threads.c:68: }
	movq	-8(%rbp), %rcx	# D.3400, tmp122
	subq	%fs:40, %rcx	# MEM[(<address-space-1> long unsigned int *)40B], tmp122
	je	.L18	#,
	call	__stack_chk_fail@PLT	#
.L18:
	leave	
	.cfi_def_cfa 7, 8
	ret	
	.cfi_endproc
.LFE8:
	.size	main, .-main
	.ident	"GCC: (GNU) 10.2.0"
	.section	.note.GNU-stack,"",@progbits
