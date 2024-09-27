	.file	"philosophers.c"
	.text
	.p2align 4
	.type	think_for_a_random_time, @function
think_for_a_random_time:
.LFB1:
	.cfi_startproc
	subq	$24, %rsp
	.cfi_def_cfa_offset 32
	call	rand@PLT
	movl	%eax, %edx
	imulq	$109962159, %rdx, %rdx
	shrq	$40, %rdx
	imull	$9999, %edx, %edx
	subl	%edx, %eax
	je	.L1
	movl	%eax, %edx
	xorl	%eax, %eax
	.p2align 4,,10
	.p2align 3
.L4:
	movl	12(%rsp), %ecx
	andl	$1, %ecx
	jne	.L3
	movl	12(%rsp), %ecx
	addl	$1, %ecx
	movl	%ecx, 12(%rsp)
.L3:
	addl	$1, %eax
	cmpl	%eax, %edx
	jne	.L4
.L1:
	addq	$24, %rsp
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE1:
	.size	think_for_a_random_time, .-think_for_a_random_time
	.set	eat_for_a_random_time,think_for_a_random_time
	.p2align 4
	.type	update_keyboard_LED, @function
update_keyboard_LED:
.LFB0:
	.cfi_startproc
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	movl	%edi, %ebx
	.p2align 4,,10
	.p2align 3
.L12:
	movl	$100, %edi
	call	inb@PLT
	testb	$2, %al
	jne	.L12
	movl	$237, %esi
	movl	$96, %edi
	call	outb@PLT
	.p2align 4,,10
	.p2align 3
.L13:
	movl	$96, %edi
	call	inb@PLT
	cmpb	$-6, %al
	jne	.L13
	movzbl	%bl, %esi
	movl	$96, %edi
	call	outb@PLT
	movl	$100, %edi
	popq	%rbx
	.cfi_def_cfa_offset 8
	jmp	ms_delay@PLT
	.cfi_endproc
.LFE0:
	.size	update_keyboard_LED, .-update_keyboard_LED
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"Phil."
.LC1:
	.string	"Running"
.LC2:
	.string	"F0,F1,num"
.LC3:
	.string	"Assertion failure: "
	.section	.rodata.str1.8,"aMS",@progbits,1
	.align 8
.LC4:
	.string	"scroll_eating + caps_eating == 0"
	.section	.rodata.str1.1
.LC5:
	.string	"file: "
.LC6:
	.string	"philosophers.c"
.LC7:
	.string	"line: "
.LC8:
	.string	"Num    "
	.text
	.p2align 4
	.globl	num
	.type	num, @function
num:
.LFB3:
	.cfi_startproc
	endbr64
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	leaq	fork(%rip), %rdi
	movl	$1, print_to_screen(%rip)
	call	lock_init@PLT
	leaq	16+fork(%rip), %rdi
	call	lock_init@PLT
	leaq	32+fork(%rip), %rdi
	call	lock_init@PLT
	leaq	phil(%rip), %rdi
	call	condition_init@PLT
	leaq	8+phil(%rip), %rdi
	call	condition_init@PLT
	leaq	16+phil(%rip), %rdi
	call	condition_init@PLT
	movl	print_to_screen(%rip), %edx
	movl	$1, forks_initialized(%rip)
	testl	%edx, %edx
	jne	.L30
.L18:
	leaq	phil(%rip), %rbx
.L24:
	call	think_for_a_random_time
	leaq	fork(%rip), %rdi
	call	lock_acquire@PLT
	leaq	16+fork(%rip), %rdi
	call	lock_acquire@PLT
	movl	scroll_eating(%rip), %eax
	movl	caps_eating(%rip), %edx
	addl	%edx, %eax
	je	.L19
.L20:
	movq	%rbx, %rsi
	leaq	fork(%rip), %rdi
	call	condition_wait@PLT
	movq	%rbx, %rsi
	leaq	16+fork(%rip), %rdi
	call	condition_wait@PLT
	movl	scroll_eating(%rip), %eax
	movl	caps_eating(%rip), %edx
	addl	%edx, %eax
	jne	.L20
.L19:
	leaq	.LC2(%rip), %rdx
	movl	$30, %esi
	movl	$10, %edi
	call	print_str@PLT
	movl	$2, %edi
	call	update_keyboard_LED
	movl	$1, num_eating(%rip)
	movl	scroll_eating(%rip), %eax
	movl	caps_eating(%rip), %edx
	addl	%edx, %eax
	jne	.L31
	movl	print_to_screen(%rip), %eax
	testl	%eax, %eax
	jne	.L32
.L23:
	call	eat_for_a_random_time
	leaq	fork(%rip), %rdi
	movl	$0, num_eating(%rip)
	call	lock_release@PLT
	leaq	16+fork(%rip), %rdi
	call	lock_release@PLT
	leaq	8+phil(%rip), %rdi
	call	condition_broadcast@PLT
	leaq	16+phil(%rip), %rdi
	call	condition_broadcast@PLT
	leaq	.LC2(%rip), %rdi
	call	strlen@PLT
	movl	$11, %ecx
	movl	$10, %esi
	movl	$30, %edi
	leal	30(%rax), %edx
	call	clear_screen@PLT
	call	yield@PLT
	jmp	.L24
.L32:
	xorl	%esi, %esi
	leaq	.LC0(%rip), %rdx
	movl	$11, %edi
	call	print_str@PLT
	leaq	.LC8(%rip), %rdx
	xorl	%esi, %esi
	movl	$12, %edi
	call	print_str@PLT
	jmp	.L23
.L31:
	xorl	%esi, %esi
	xorl	%edi, %edi
	leaq	.LC3(%rip), %rdx
	call	print_str@PLT
	xorl	%edi, %edi
	movl	$19, %esi
	leaq	.LC4(%rip), %rdx
	call	print_str@PLT
	xorl	%esi, %esi
	movl	$1, %edi
	leaq	.LC5(%rip), %rdx
	call	print_str@PLT
	movl	$6, %esi
	movl	$1, %edi
	leaq	.LC6(%rip), %rdx
	call	print_str@PLT
	xorl	%esi, %esi
	movl	$2, %edi
	leaq	.LC7(%rip), %rdx
	call	print_str@PLT
	movl	$136, %edx
	movl	$6, %esi
	movl	$2, %edi
	call	print_int@PLT
#APP
# 136 "philosophers.c" 1
	cli
# 0 "" 2
#NO_APP
.L22:
	jmp	.L22
.L30:
	xorl	%esi, %esi
	leaq	.LC0(%rip), %rdx
	movl	$11, %edi
	call	print_str@PLT
	leaq	.LC1(%rip), %rdx
	xorl	%esi, %esi
	movl	$12, %edi
	call	print_str@PLT
	jmp	.L18
	.cfi_endproc
.LFE3:
	.size	num, .-num
	.section	.rodata.str1.1
.LC9:
	.string	"F1,F2,caps"
	.section	.rodata.str1.8
	.align 8
.LC10:
	.string	"scroll_eating + num_eating == 0"
	.section	.rodata.str1.1
.LC11:
	.string	"Caps   "
.LC12:
	.string	"F0,F1,caps"
	.text
	.p2align 4
	.globl	caps
	.type	caps, @function
caps:
.LFB4:
	.cfi_startproc
	endbr64
	movl	forks_initialized(%rip), %eax
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	testl	%eax, %eax
	jne	.L34
.L35:
	call	yield@PLT
	movl	forks_initialized(%rip), %eax
	testl	%eax, %eax
	je	.L35
.L34:
	leaq	8+phil(%rip), %rbx
.L41:
	call	think_for_a_random_time
	leaq	16+fork(%rip), %rdi
	call	lock_acquire@PLT
	leaq	32+fork(%rip), %rdi
	call	lock_acquire@PLT
	movl	scroll_eating(%rip), %eax
	movl	num_eating(%rip), %edx
	addl	%edx, %eax
	je	.L36
.L37:
	movq	%rbx, %rsi
	leaq	16+fork(%rip), %rdi
	call	condition_wait@PLT
	movq	%rbx, %rsi
	leaq	32+fork(%rip), %rdi
	call	condition_wait@PLT
	movl	scroll_eating(%rip), %eax
	movl	num_eating(%rip), %edx
	addl	%edx, %eax
	jne	.L37
.L36:
	leaq	.LC9(%rip), %rdx
	movl	$30, %esi
	movl	$11, %edi
	call	print_str@PLT
	movl	$4, %edi
	call	update_keyboard_LED
	movl	$1, caps_eating(%rip)
	movl	scroll_eating(%rip), %eax
	movl	num_eating(%rip), %edx
	addl	%edx, %eax
	jne	.L49
	movl	print_to_screen(%rip), %eax
	testl	%eax, %eax
	jne	.L50
.L40:
	call	eat_for_a_random_time
	leaq	16+fork(%rip), %rdi
	movl	$0, caps_eating(%rip)
	call	lock_release@PLT
	leaq	32+fork(%rip), %rdi
	call	lock_release@PLT
	leaq	phil(%rip), %rdi
	call	condition_broadcast@PLT
	leaq	16+phil(%rip), %rdi
	call	condition_broadcast@PLT
	leaq	.LC12(%rip), %rdi
	call	strlen@PLT
	movl	$12, %ecx
	movl	$11, %esi
	movl	$30, %edi
	leal	30(%rax), %edx
	call	clear_screen@PLT
	call	yield@PLT
	jmp	.L41
.L50:
	xorl	%esi, %esi
	leaq	.LC0(%rip), %rdx
	movl	$11, %edi
	call	print_str@PLT
	leaq	.LC11(%rip), %rdx
	xorl	%esi, %esi
	movl	$12, %edi
	call	print_str@PLT
	jmp	.L40
.L49:
	xorl	%esi, %esi
	xorl	%edi, %edi
	leaq	.LC3(%rip), %rdx
	call	print_str@PLT
	xorl	%edi, %edi
	movl	$19, %esi
	leaq	.LC10(%rip), %rdx
	call	print_str@PLT
	xorl	%esi, %esi
	movl	$1, %edi
	leaq	.LC5(%rip), %rdx
	call	print_str@PLT
	movl	$6, %esi
	movl	$1, %edi
	leaq	.LC6(%rip), %rdx
	call	print_str@PLT
	xorl	%esi, %esi
	movl	$2, %edi
	leaq	.LC7(%rip), %rdx
	call	print_str@PLT
	movl	$186, %edx
	movl	$6, %esi
	movl	$2, %edi
	call	print_int@PLT
#APP
# 186 "philosophers.c" 1
	cli
# 0 "" 2
#NO_APP
.L39:
	jmp	.L39
	.cfi_endproc
.LFE4:
	.size	caps, .-caps
	.section	.rodata.str1.1
.LC13:
	.string	"F2,F0,scroll"
.LC14:
	.string	"caps_eating + num_eating == 0"
.LC15:
	.string	"Scroll "
	.text
	.p2align 4
	.globl	scroll_th
	.type	scroll_th, @function
scroll_th:
.LFB5:
	.cfi_startproc
	endbr64
	pushq	%r12
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	pushq	%rbx
	.cfi_def_cfa_offset 24
	.cfi_offset 3, -24
	subq	$8, %rsp
	.cfi_def_cfa_offset 32
	movl	forks_initialized(%rip), %eax
	testl	%eax, %eax
	jne	.L52
.L53:
	call	yield@PLT
	movl	forks_initialized(%rip), %eax
	testl	%eax, %eax
	je	.L53
.L52:
	leaq	16+phil(%rip), %rbx
.L59:
	call	think_for_a_random_time
	leaq	32+fork(%rip), %rdi
	call	lock_acquire@PLT
	leaq	fork(%rip), %rdi
	call	lock_acquire@PLT
	movl	caps_eating(%rip), %eax
	movl	num_eating(%rip), %edx
	addl	%edx, %eax
	je	.L54
.L55:
	movq	%rbx, %rsi
	leaq	32+fork(%rip), %rdi
	call	condition_wait@PLT
	movq	%rbx, %rsi
	leaq	fork(%rip), %rdi
	call	condition_wait@PLT
	movl	caps_eating(%rip), %eax
	movl	num_eating(%rip), %edx
	addl	%edx, %eax
	jne	.L55
.L54:
	leaq	.LC13(%rip), %rdx
	movl	$30, %esi
	movl	$12, %edi
	call	print_str@PLT
	movl	$1, %edi
	call	update_keyboard_LED
	leaq	.LC13(%rip), %rdi
	movl	$1, scroll_eating(%rip)
	movl	caps_eating(%rip), %r12d
	call	strlen@PLT
	movl	$12, %edi
	movl	%r12d, %edx
	leal	31(%rax), %esi
	call	print_int@PLT
	movl	num_eating(%rip), %r12d
	leaq	.LC13(%rip), %rdi
	call	strlen@PLT
	movl	$12, %edi
	leal	32(%rax), %esi
	movl	%r12d, %edx
	call	print_int@PLT
	movl	caps_eating(%rip), %eax
	movl	num_eating(%rip), %edx
	addl	%edx, %eax
	jne	.L67
	movl	print_to_screen(%rip), %eax
	testl	%eax, %eax
	jne	.L68
.L58:
	call	eat_for_a_random_time
	leaq	32+fork(%rip), %rdi
	movl	$0, scroll_eating(%rip)
	call	lock_release@PLT
	leaq	fork(%rip), %rdi
	call	lock_release@PLT
	leaq	phil(%rip), %rdi
	call	condition_broadcast@PLT
	leaq	8+phil(%rip), %rdi
	call	condition_broadcast@PLT
	leaq	.LC13(%rip), %rdi
	call	strlen@PLT
	movl	$13, %ecx
	movl	$12, %esi
	movl	$30, %edi
	leal	30(%rax), %edx
	call	clear_screen@PLT
	call	yield@PLT
	jmp	.L59
.L68:
	xorl	%esi, %esi
	leaq	.LC0(%rip), %rdx
	movl	$11, %edi
	call	print_str@PLT
	leaq	.LC15(%rip), %rdx
	xorl	%esi, %esi
	movl	$12, %edi
	call	print_str@PLT
	jmp	.L58
.L67:
	xorl	%esi, %esi
	xorl	%edi, %edi
	leaq	.LC3(%rip), %rdx
	call	print_str@PLT
	xorl	%edi, %edi
	movl	$19, %esi
	leaq	.LC14(%rip), %rdx
	call	print_str@PLT
	xorl	%esi, %esi
	movl	$1, %edi
	leaq	.LC5(%rip), %rdx
	call	print_str@PLT
	movl	$6, %esi
	movl	$1, %edi
	leaq	.LC6(%rip), %rdx
	call	print_str@PLT
	xorl	%esi, %esi
	movl	$2, %edi
	leaq	.LC7(%rip), %rdx
	call	print_str@PLT
	movl	$238, %edx
	movl	$6, %esi
	movl	$2, %edi
	call	print_int@PLT
#APP
# 238 "philosophers.c" 1
	cli
# 0 "" 2
#NO_APP
.L57:
	jmp	.L57
	.cfi_endproc
.LFE5:
	.size	scroll_th, .-scroll_th
	.comm	print_to_screen,4,4
	.globl	caps_eating
	.bss
	.align 4
	.type	caps_eating, @object
	.size	caps_eating, 4
caps_eating:
	.zero	4
	.globl	scroll_eating
	.align 4
	.type	scroll_eating, @object
	.size	scroll_eating, 4
scroll_eating:
	.zero	4
	.globl	num_eating
	.align 4
	.type	num_eating, @object
	.size	num_eating, 4
num_eating:
	.zero	4
	.comm	phil,24,16
	.comm	fork,48,32
	.globl	forks_initialized
	.align 4
	.type	forks_initialized, @object
	.size	forks_initialized, 4
forks_initialized:
	.zero	4
	.ident	"GCC: (Ubuntu 9.4.0-1ubuntu1~20.04.1) 9.4.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	 1f - 0f
	.long	 4f - 1f
	.long	 5
0:
	.string	 "GNU"
1:
	.align 8
	.long	 0xc0000002
	.long	 3f - 2f
2:
	.long	 0x3
3:
	.align 8
4:
