	.file	"createimage.c"
	.text
	.local	options
	.comm	options,12,4
	.local	image
	.comm	image,32,32
	.section	.rodata.str1.4,"aMS",@progbits,1
	.align 4
.LC0:
	.string	"[--extended] [--vm] [--kernel] <bootblock> <executable-file> ..."
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC1:
	.string	"usage: %s %s\n"
.LC2:
	.string	"vm"
.LC3:
	.string	"extended"
.LC4:
	.string	"kernel"
	.section	.rodata.str1.4
	.align 4
.LC5:
	.string	"%s: invalid option\nusage: %s %s\n"
	.section	.text.startup,"ax",@progbits
	.p2align 2
	.globl	main
	.type	main, @function
main:
	endbr32
	leal	4(%esp), %ecx
	andl	$-16, %esp
	pushl	-4(%ecx)
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	pushl	%ecx
	subl	$24, %esp
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	movl	(%ecx), %eax
	movl	%eax, -28(%ebp)
	movl	4(%ecx), %edi
	movl	(%edi), %edx
	movl	%edx, -40(%ebp)
	movl	$0, options@GOTOFF(%ebx)
	movl	$0, 4+options@GOTOFF(%ebx)
	movl	$0, 8+options@GOTOFF(%ebx)
	decl	%eax
	jle	.L9
	leal	.LC2@GOTOFF(%ebx), %eax
	movl	%eax, -32(%ebp)
	leal	.LC3@GOTOFF(%ebx), %eax
	movl	%eax, -36(%ebp)
	jmp	.L2
	.p2align 2
.L16:
	movl	$1, options@GOTOFF(%ebx)
.L4:
	decl	-28(%ebp)
	movl	-28(%ebp), %eax
	addl	$4, %edi
	decl	%eax
	je	.L7
.L2:
	movl	4(%edi), %esi
	cmpb	$45, (%esi)
	jne	.L7
	cmpb	$45, 1(%esi)
	jne	.L7
	addl	$2, %esi
	subl	$8, %esp
	pushl	-32(%ebp)
	pushl	%esi
	call	strcmp@PLT
	addl	$16, %esp
	testl	%eax, %eax
	je	.L16
	subl	$8, %esp
	pushl	-36(%ebp)
	pushl	%esi
	call	strcmp@PLT
	addl	$16, %esp
	testl	%eax, %eax
	jne	.L5
	movl	$1, 4+options@GOTOFF(%ebx)
	decl	-28(%ebp)
	movl	-28(%ebp), %eax
	addl	$4, %edi
	decl	%eax
	jne	.L2
	.p2align 2
.L7:
	movl	8+options@GOTOFF(%ebx), %eax
	incl	%eax
	movl	-28(%ebp), %edx
	cmpl	%edx, %eax
	jge	.L9
	subl	$8, %esp
	addl	$4, %edi
	pushl	%edi
	movl	%edx, %eax
	decl	%eax
	pushl	%eax
	call	create_images
	addl	$16, %esp
	xorl	%eax, %eax
	leal	-16(%ebp), %esp
	popl	%ecx
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	leal	-4(%ecx), %esp
	ret
	.p2align 2
.L5:
	subl	$8, %esp
	leal	.LC4@GOTOFF(%ebx), %eax
	pushl	%eax
	pushl	%esi
	call	strcmp@PLT
	addl	$16, %esp
	testl	%eax, %eax
	jne	.L6
	movl	$1, 8+options@GOTOFF(%ebx)
	jmp	.L4
.L9:
	pushl	%eax
	leal	.LC0@GOTOFF(%ebx), %eax
	pushl	%eax
	pushl	-40(%ebp)
	leal	.LC1@GOTOFF(%ebx), %eax
	pushl	%eax
	call	error
.L6:
	leal	.LC0@GOTOFF(%ebx), %eax
	pushl	%eax
	movl	-40(%ebp), %eax
	pushl	%eax
	pushl	%eax
	leal	.LC5@GOTOFF(%ebx), %eax
	pushl	%eax
	call	error
	.size	main, .-main
	.section	.rodata.str1.1
.LC6:
	.string	"w"
.LC7:
	.string	"./image"
.LC8:
	.string	"createimage.c"
.LC9:
	.string	"image.img != NULL"
	.section	.rodata.str1.4
	.align 4
.LC10:
	.string	"(image.nbytes % SECTOR_SIZE) == 0"
	.text
	.p2align 2
	.type	create_images, @function
create_images:
	endbr32
	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$36, %esp
	call	__x86.get_pc_thunk.di
	addl	$_GLOBAL_OFFSET_TABLE_, %edi
	leal	.LC6@GOTOFF(%edi), %eax
	pushl	%eax
	leal	.LC7@GOTOFF(%edi), %eax
	pushl	%eax
	movl	%edi, %ebx
	call	fopen@PLT
	movl	%eax, image@GOTOFF(%edi)
	addl	$16, %esp
	testl	%eax, %eax
	je	.L28
	movl	$0, 8+image@GOTOFF(%edi)
	subl	$8, %esp
	movl	60(%esp), %eax
	pushl	(%eax)
	leal	image@GOTOFF(%edi), %ebp
	pushl	%ebp
	call	create_image
	movl	64(%esp), %eax
	leal	-1(%eax), %ebx
	movl	68(%esp), %eax
	leal	4(%eax), %esi
	addl	$16, %esp
	cmpl	$1, options@GOTOFF(%edi)
	je	.L29
.L19:
	testl	%ebx, %ebx
	jle	.L21
	leal	options@GOTOFF, %eax
	movl	%eax, 12(%esp)
	jmp	.L23
	.p2align 2
.L22:
	testl	%ebx, %ebx
	je	.L21
.L23:
	subl	$8, %esp
	pushl	(%esi)
	pushl	%ebp
	call	create_image
	decl	%ebx
	addl	$4, %esi
	addl	$16, %esp
	movl	12(%esp), %eax
	cmpl	$1, (%eax,%edi)
	jne	.L22
	subl	$12, %esp
	pushl	%ebp
	call	write_process_directory
	addl	$16, %esp
	testl	%ebx, %ebx
	jne	.L23
	.p2align 2
.L21:
	movl	options@GOTOFF(%edi), %edx
	testl	%edx, %edx
	je	.L30
.L24:
	testl	$511, 8+image@GOTOFF(%edi)
	jne	.L31
	subl	$12, %esp
	pushl	image@GOTOFF(%edi)
	movl	%edi, %ebx
	call	fclose@PLT
	addl	$44, %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.p2align 2
.L29:
	cmpl	$1, 8+options@GOTOFF(%edi)
	je	.L32
.L20:
	subl	$12, %esp
	pushl	%ebp
	call	write_os_size
	movl	%ebp, (%esp)
	call	prepare_process_directory
	popl	%ecx
	pushl	image@GOTOFF(%edi)
	call	write_bootloader_signature
	addl	$16, %esp
	jmp	.L19
	.p2align 2
.L30:
	subl	$12, %esp
	pushl	%ebp
	call	write_os_size
	popl	%eax
	pushl	image@GOTOFF(%edi)
	call	write_bootloader_signature
	addl	$16, %esp
	jmp	.L24
.L32:
	subl	$8, %esp
	movl	60(%esp), %eax
	pushl	4(%eax)
	pushl	%ebp
	call	create_image
	movl	64(%esp), %eax
	leal	-2(%eax), %ebx
	movl	68(%esp), %eax
	leal	8(%eax), %esi
	addl	$16, %esp
	jmp	.L20
.L31:
	leal	__PRETTY_FUNCTION__.2873@GOTOFF(%edi), %eax
	pushl	%eax
	pushl	$148
	leal	.LC8@GOTOFF(%edi), %eax
	pushl	%eax
	leal	.LC10@GOTOFF(%edi), %eax
	pushl	%eax
	movl	%edi, %ebx
	call	__assert_fail@PLT
.L28:
	leal	__PRETTY_FUNCTION__.2873@GOTOFF(%edi), %eax
	pushl	%eax
	pushl	$109
	leal	.LC8@GOTOFF(%edi), %eax
	pushl	%eax
	leal	.LC9@GOTOFF(%edi), %eax
	pushl	%eax
	call	__assert_fail@PLT
	.size	create_images, .-create_images
	.section	.rodata.str1.1
.LC11:
	.string	"r"
.LC12:
	.string	"im->fp != NULL"
.LC13:
	.string	"0x%04x: %s\n"
	.text
	.p2align 2
	.type	create_image, @function
create_image:
	endbr32
	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$132, %esp
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	movl	%ebx, 20(%esp)
	movl	152(%esp), %ebp
	movl	156(%esp), %esi
	leal	.LC11@GOTOFF(%ebx), %eax
	pushl	%eax
	pushl	%esi
	call	fopen@PLT
	movl	%eax, 4(%ebp)
	addl	$16, %esp
	testl	%eax, %eax
	je	.L40
	subl	$8, %esp
	pushl	%eax
	leal	72(%esp), %eax
	movl	%eax, 20(%esp)
	pushl	%eax
	call	read_ehdr
	pushl	%esi
	pushl	104(%esp)
	movl	36(%esp), %ebx
	leal	.LC13@GOTOFF(%ebx), %eax
	pushl	%eax
	pushl	$1
	call	__printf_chk@PLT
	addl	$32, %esp
	cmpw	$0, 104(%esp)
	je	.L35
	xorl	%ebx, %ebx
	leal	28(%esp), %eax
	movl	%eax, 4(%esp)
	jmp	.L37
	.p2align 2
.L36:
	subl	$44, %esp
	movl	$8, %ecx
	movl	%esp, %edi
	movl	48(%esp), %esi
	rep movsl
	pushl	%ebp
	call	write_segment
	incl	%ebx
	movzwl	152(%esp), %eax
	addl	$48, %esp
	cmpl	%ebx, %eax
	jle	.L35
.L37:
	subl	$52, %esp
	movl	$13, %ecx
	movl	%esp, %edi
	movl	60(%esp), %esi
	rep movsl
	pushl	%ebx
	pushl	4(%ebp)
	pushl	64(%esp)
	call	read_phdr
	addl	$64, %esp
	testl	%ebx, %ebx
	jne	.L36
	subl	$8, %esp
	pushl	44(%esp)
	pushl	%ebp
	call	process_start
	addl	$16, %esp
	jmp	.L36
	.p2align 2
.L35:
	subl	$12, %esp
	pushl	%ebp
	call	process_end
	popl	%eax
	pushl	4(%ebp)
	movl	28(%esp), %ebx
	call	fclose@PLT
	addl	$140, %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
.L40:
	movl	12(%esp), %ebx
	leal	__PRETTY_FUNCTION__.2884@GOTOFF(%ebx), %eax
	pushl	%eax
	pushl	$159
	leal	.LC8@GOTOFF(%ebx), %eax
	pushl	%eax
	leal	.LC12@GOTOFF(%ebx), %eax
	pushl	%eax
	call	__assert_fail@PLT
	.size	create_image, .-create_image
	.section	.rodata.str1.1
.LC14:
	.string	"ret == 1"
.LC15:
	.string	"ehdr->e_ident[EI_MAG1] == 'E'"
.LC16:
	.string	"ehdr->e_ident[EI_MAG2] == 'L'"
.LC17:
	.string	"ehdr->e_ident[EI_MAG3] == 'F'"
	.text
	.p2align 2
	.type	read_ehdr, @function
read_ehdr:
	endbr32
	pushl	%esi
	pushl	%ebx
	pushl	%edx
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	movl	16(%esp), %esi
	pushl	20(%esp)
	pushl	$1
	pushl	$52
	pushl	%esi
	call	fread@PLT
	addl	$16, %esp
	decl	%eax
	jne	.L47
	cmpb	$69, 1(%esi)
	jne	.L48
	cmpb	$76, 2(%esi)
	jne	.L49
	cmpb	$70, 3(%esi)
	jne	.L50
	popl	%eax
	popl	%ebx
	popl	%esi
	ret
.L47:
	leal	__PRETTY_FUNCTION__.2893@GOTOFF(%ebx), %eax
	pushl	%eax
	pushl	$185
	leal	.LC8@GOTOFF(%ebx), %eax
	pushl	%eax
	leal	.LC14@GOTOFF(%ebx), %eax
	pushl	%eax
	call	__assert_fail@PLT
.L50:
	leal	__PRETTY_FUNCTION__.2893@GOTOFF(%ebx), %eax
	pushl	%eax
	pushl	$188
	leal	.LC8@GOTOFF(%ebx), %eax
	pushl	%eax
	leal	.LC17@GOTOFF(%ebx), %eax
	pushl	%eax
	call	__assert_fail@PLT
.L49:
	leal	__PRETTY_FUNCTION__.2893@GOTOFF(%ebx), %eax
	pushl	%eax
	pushl	$187
	leal	.LC8@GOTOFF(%ebx), %eax
	pushl	%eax
	leal	.LC16@GOTOFF(%ebx), %eax
	pushl	%eax
	call	__assert_fail@PLT
.L48:
	leal	__PRETTY_FUNCTION__.2893@GOTOFF(%ebx), %eax
	pushl	%eax
	pushl	$186
	leal	.LC8@GOTOFF(%ebx), %eax
	pushl	%eax
	leal	.LC15@GOTOFF(%ebx), %eax
	pushl	%eax
	call	__assert_fail@PLT
	.size	read_ehdr, .-read_ehdr
	.section	.rodata.str1.1
.LC18:
	.string	"\tsegment %d\n"
.LC19:
	.string	"\t\toffset 0x%04x"
.LC20:
	.string	"\t\tvaddr 0x%04x\n"
.LC21:
	.string	"\t\tfilesz 0x%04x"
.LC22:
	.string	"\t\tmemsz 0x%04x\n"
	.text
	.p2align 2
	.type	read_phdr, @function
read_phdr:
	endbr32
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	movl	16(%esp), %edi
	movl	20(%esp), %esi
	pushl	%edx
	pushl	$0
	movzwl	78(%esp), %eax
	imull	32(%esp), %eax
	addl	64(%esp), %eax
	pushl	%eax
	pushl	%esi
	call	fseek@PLT
	pushl	%esi
	pushl	$1
	pushl	$32
	pushl	%edi
	call	fread@PLT
	addl	$32, %esp
	decl	%eax
	jne	.L55
	cmpl	$1, 4+options@GOTOFF(%ebx)
	je	.L56
	popl	%ebx
	popl	%esi
	popl	%edi
	ret
	.p2align 2
.L56:
	pushl	%eax
	pushl	28(%esp)
	leal	.LC18@GOTOFF(%ebx), %eax
	pushl	%eax
	pushl	$1
	call	__printf_chk@PLT
	addl	$12, %esp
	pushl	4(%edi)
	leal	.LC19@GOTOFF(%ebx), %eax
	pushl	%eax
	pushl	$1
	call	__printf_chk@PLT
	addl	$12, %esp
	pushl	8(%edi)
	leal	.LC20@GOTOFF(%ebx), %eax
	pushl	%eax
	pushl	$1
	call	__printf_chk@PLT
	addl	$12, %esp
	pushl	16(%edi)
	leal	.LC21@GOTOFF(%ebx), %eax
	pushl	%eax
	pushl	$1
	call	__printf_chk@PLT
	addl	$12, %esp
	pushl	20(%edi)
	leal	.LC22@GOTOFF(%ebx), %eax
	pushl	%eax
	pushl	$1
	call	__printf_chk@PLT
	addl	$16, %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	ret
.L55:
	leal	__PRETTY_FUNCTION__.2901@GOTOFF(%ebx), %eax
	pushl	%eax
	pushl	$196
	leal	.LC8@GOTOFF(%ebx), %eax
	pushl	%eax
	leal	.LC14@GOTOFF(%ebx), %eax
	pushl	%eax
	call	__assert_fail@PLT
	.size	read_phdr, .-read_phdr
	.section	.rodata.str1.4
	.align 4
.LC23:
	.string	"(im->nbytes % SECTOR_SIZE) == 0"
	.section	.rodata.str1.1
.LC24:
	.string	"vaddr == 0"
	.text
	.p2align 2
	.type	process_start, @function
process_start:
	endbr32
	pushl	%ebx
	subl	$8, %esp
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	movl	16(%esp), %edx
	movl	8(%edx), %eax
	testl	$511, %eax
	jne	.L64
	movl	%eax, %ecx
	shrl	$9, %ecx
	movl	%ecx, 24(%edx)
	testl	%eax, %eax
	je	.L65
	movl	options@GOTOFF(%ebx), %ecx
	testl	%ecx, %ecx
	jne	.L62
	movl	$-3584, 12(%edx)
	addl	$8, %esp
	popl	%ebx
	ret
	.p2align 2
.L65:
	movl	20(%esp), %eax
	testl	%eax, %eax
	jne	.L66
	movl	$0, 12(%edx)
	addl	$8, %esp
	popl	%ebx
	ret
	.p2align 2
.L62:
	movl	20(%esp), %ecx
	andl	$-4096, %ecx
	subl	%ecx, %eax
	movl	%eax, 12(%edx)
	addl	$8, %esp
	popl	%ebx
	ret
.L64:
	leal	__PRETTY_FUNCTION__.2906@GOTOFF(%ebx), %eax
	pushl	%eax
	pushl	$209
	leal	.LC8@GOTOFF(%ebx), %eax
	pushl	%eax
	leal	.LC23@GOTOFF(%ebx), %eax
	pushl	%eax
	call	__assert_fail@PLT
.L66:
	leal	__PRETTY_FUNCTION__.2906@GOTOFF(%ebx), %eax
	pushl	%eax
	pushl	$215
	leal	.LC8@GOTOFF(%ebx), %eax
	pushl	%eax
	leal	.LC24@GOTOFF(%ebx), %eax
	pushl	%eax
	call	__assert_fail@PLT
	.size	process_start, .-process_start
	.section	.rodata.str1.1
.LC25:
	.string	"\t\tpadding up to 0x%04x\n"
	.section	.rodata.str1.4
	.align 4
.LC26:
	.string	"\tProcess starts at sector %d, and spans for %d sectors\n"
	.text
	.p2align 2
	.type	process_end, @function
process_end:
	endbr32
	pushl	%esi
	pushl	%ebx
	pushl	%ecx
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	movl	16(%esp), %esi
	movl	8(%esi), %eax
	testl	$511, %eax
	jne	.L68
	movl	4+options@GOTOFF(%ebx), %ecx
.L69:
	movl	24(%esi), %edx
	shrl	$9, %eax
	subl	%edx, %eax
	movl	%eax, 28(%esi)
	decl	%ecx
	je	.L75
	popl	%eax
	popl	%ebx
	popl	%esi
	ret
	.p2align 2
.L68:
	subl	$8, %esp
	pushl	(%esi)
	pushl	$0
	call	fputc@PLT
	movl	8(%esi), %eax
	incl	%eax
	movl	%eax, 8(%esi)
	addl	$16, %esp
	testl	$511, %eax
	jne	.L68
	cmpl	$1, 4+options@GOTOFF(%ebx)
	je	.L76
	shrl	$9, %eax
	subl	24(%esi), %eax
	movl	%eax, 28(%esi)
	popl	%eax
	popl	%ebx
	popl	%esi
	ret
	.p2align 2
.L75:
	pushl	%eax
	pushl	%edx
	leal	.LC26@GOTOFF(%ebx), %eax
	pushl	%eax
	pushl	$1
	call	__printf_chk@PLT
	addl	$16, %esp
	popl	%eax
	popl	%ebx
	popl	%esi
	ret
	.p2align 2
.L76:
	pushl	%edx
	pushl	%eax
	leal	.LC25@GOTOFF(%ebx), %eax
	pushl	%eax
	pushl	$1
	call	__printf_chk@PLT
	movl	8(%esi), %eax
	movl	4+options@GOTOFF(%ebx), %ecx
	addl	$16, %esp
	jmp	.L69
	.size	process_end, .-process_end
	.section	.rodata.str1.1
.LC27:
	.string	"memory conflict\n"
.LC28:
	.string	"\t\twriting 0x%04x bytes\n"
	.text
	.p2align 2
	.type	write_segment, @function
write_segment:
	endbr32
	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$28, %esp
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	movl	48(%esp), %ebp
	movl	72(%esp), %eax
	movl	%eax, 12(%esp)
	testl	%eax, %eax
	je	.L77
	movl	12(%ebp), %esi
	addl	60(%esp), %esi
	cmpl	%esi, 8(%ebp)
	ja	.L94
	movl	68(%esp), %eax
	movl	%eax, 8(%esp)
	jb	.L81
.L80:
	cmpl	$1, 4+options@GOTOFF(%ebx)
	jne	.L83
	pushl	%edi
	pushl	16(%esp)
	leal	.LC28@GOTOFF(%ebx), %eax
	pushl	%eax
	pushl	$1
	call	__printf_chk@PLT
	addl	$16, %esp
	jmp	.L83
	.p2align 2
.L81:
	subl	$8, %esp
	pushl	0(%ebp)
	pushl	$0
	call	fputc@PLT
	movl	8(%ebp), %eax
	incl	%eax
	movl	%eax, 8(%ebp)
	addl	$16, %esp
	cmpl	%esi, %eax
	jb	.L81
	cmpl	$1, 4+options@GOTOFF(%ebx)
	je	.L95
.L83:
	pushl	%esi
	pushl	$0
	pushl	64(%esp)
	pushl	4(%ebp)
	call	fseek@PLT
	movl	24(%esp), %eax
	leal	-1(%eax), %esi
	addl	$16, %esp
	testl	%eax, %eax
	je	.L96
	.p2align 2
.L85:
	movl	0(%ebp), %edi
	subl	$12, %esp
	pushl	4(%ebp)
	call	fgetc@PLT
	popl	%edx
	popl	%ecx
	pushl	%edi
	pushl	%eax
	call	fputc@PLT
	incl	8(%ebp)
	decl	%esi
	addl	$16, %esp
	cmpl	$-1, %esi
	jne	.L85
	movl	12(%esp), %eax
	leal	-1(%eax), %esi
	movl	8(%esp), %edx
	subl	%edx, %esi
	cmpl	%eax, %edx
	je	.L77
	.p2align 2
.L88:
	subl	$8, %esp
	pushl	0(%ebp)
	pushl	$0
	call	fputc@PLT
	incl	8(%ebp)
	decl	%esi
	addl	$16, %esp
	cmpl	$-1, %esi
	jne	.L88
.L77:
	addl	$28, %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.p2align 2
.L96:
	movl	12(%esp), %esi
	decl	%esi
	jmp	.L88
	.p2align 2
.L95:
	pushl	%eax
	pushl	%esi
	leal	.LC25@GOTOFF(%ebx), %eax
	pushl	%eax
	pushl	$1
	call	__printf_chk@PLT
	addl	$16, %esp
	jmp	.L80
.L94:
	subl	$12, %esp
	leal	.LC27@GOTOFF(%ebx), %eax
	pushl	%eax
	call	error
	.size	write_segment, .-write_segment
	.section	.rodata.str1.1
.LC29:
	.string	"os_size: %d sectors\n"
	.text
	.p2align 2
	.type	write_os_size, @function
write_os_size:
	endbr32
	pushl	%esi
	pushl	%ebx
	subl	$20, %esp
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	movl	32(%esp), %esi
	movl	8(%esi), %eax
	testl	$511, %eax
	jne	.L101
	shrl	$9, %eax
	decl	%eax
	movw	%ax, 14(%esp)
	pushl	%ecx
	pushl	$0
	pushl	$2
	pushl	(%esi)
	call	fseek@PLT
	pushl	(%esi)
	pushl	$1
	pushl	$2
	leal	42(%esp), %eax
	pushl	%eax
	call	fwrite@PLT
	addl	$32, %esp
	cmpl	$1, 4+options@GOTOFF(%ebx)
	je	.L102
.L99:
	pushl	%eax
	pushl	$2
	pushl	$0
	pushl	(%esi)
	call	fseek@PLT
	addl	$36, %esp
	popl	%ebx
	popl	%esi
	ret
	.p2align 2
.L102:
	pushl	%edx
	movswl	18(%esp), %eax
	pushl	%eax
	leal	.LC29@GOTOFF(%ebx), %eax
	pushl	%eax
	pushl	$1
	call	__printf_chk@PLT
	addl	$16, %esp
	jmp	.L99
.L101:
	leal	__PRETTY_FUNCTION__.2931@GOTOFF(%ebx), %eax
	pushl	%eax
	pushl	$295
	leal	.LC8@GOTOFF(%ebx), %eax
	pushl	%eax
	leal	.LC23@GOTOFF(%ebx), %eax
	pushl	%eax
	call	__assert_fail@PLT
	.size	write_os_size, .-write_os_size
	.section	.rodata.str1.1
.LC30:
	.string	"options.vm"
	.text
	.p2align 2
	.type	prepare_process_directory, @function
prepare_process_directory:
	endbr32
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	movl	16(%esp), %esi
	movl	8(%esi), %eax
	testl	$511, %eax
	jne	.L109
	movl	options@GOTOFF(%ebx), %edx
	testl	%edx, %edx
	je	.L110
	movl	%eax, 16(%esi)
	addl	$512, %eax
	movl	%eax, 20(%esi)
	movl	$512, %edi
	.p2align 2
.L106:
	subl	$8, %esp
	pushl	(%esi)
	pushl	$0
	call	fputc@PLT
	incl	8(%esi)
	addl	$16, %esp
	decl	%edi
	jne	.L106
	popl	%ebx
	popl	%esi
	popl	%edi
	ret
.L109:
	leal	__PRETTY_FUNCTION__.2936@GOTOFF(%ebx), %eax
	pushl	%eax
	pushl	$314
	leal	.LC8@GOTOFF(%ebx), %eax
	pushl	%eax
	leal	.LC23@GOTOFF(%ebx), %eax
	pushl	%eax
	call	__assert_fail@PLT
.L110:
	leal	__PRETTY_FUNCTION__.2936@GOTOFF(%ebx), %eax
	pushl	%eax
	pushl	$315
	leal	.LC8@GOTOFF(%ebx), %eax
	pushl	%eax
	leal	.LC30@GOTOFF(%ebx), %eax
	pushl	%eax
	call	__assert_fail@PLT
	.size	prepare_process_directory, .-prepare_process_directory
	.section	.rodata.str1.4
	.align 4
.LC31:
	.string	"Too many processes! Can't hold them in the directory!\n"
	.text
	.p2align 2
	.type	write_process_directory, @function
write_process_directory:
	endbr32
	pushl	%esi
	pushl	%ebx
	pushl	%ecx
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	movl	16(%esp), %esi
	movl	options@GOTOFF(%ebx), %eax
	testl	%eax, %eax
	je	.L115
	movl	16(%esi), %eax
	leal	8(%eax), %edx
	cmpl	20(%esi), %edx
	jnb	.L116
	pushl	%edx
	pushl	$0
	pushl	%eax
	pushl	(%esi)
	call	fseek@PLT
	pushl	(%esi)
	pushl	$1
	pushl	$8
	leal	24(%esi), %eax
	pushl	%eax
	call	fwrite@PLT
	addl	$20, %esp
	pushl	(%esi)
	call	ftell@PLT
	movl	%eax, 16(%esi)
	addl	$12, %esp
	pushl	$2
	pushl	$0
	pushl	(%esi)
	call	fseek@PLT
	addl	$20, %esp
	popl	%ebx
	popl	%esi
	ret
.L115:
	leal	__PRETTY_FUNCTION__.2943@GOTOFF(%ebx), %eax
	pushl	%eax
	pushl	$328
	leal	.LC8@GOTOFF(%ebx), %eax
	pushl	%eax
	leal	.LC30@GOTOFF(%ebx), %eax
	pushl	%eax
	call	__assert_fail@PLT
.L116:
	subl	$12, %esp
	leal	.LC31@GOTOFF(%ebx), %eax
	pushl	%eax
	call	error
	.size	write_process_directory, .-write_process_directory
	.section	.rodata.str1.4
	.align 4
.LC32:
	.string	"writing bootable signature: 0x%04x\n"
	.text
	.p2align 2
	.type	write_bootloader_signature, @function
write_bootloader_signature:
	endbr32
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$16, %esp
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	movl	32(%esp), %esi
	movw	$-21931, 14(%esp)
	cmpl	$1, 4+options@GOTOFF(%ebx)
	je	.L120
.L118:
	subl	$12, %esp
	pushl	%esi
	call	ftell@PLT
	movl	%eax, %edi
	addl	$12, %esp
	pushl	$0
	pushl	$510
	pushl	%esi
	call	fseek@PLT
	pushl	%esi
	pushl	$1
	pushl	$2
	leal	42(%esp), %eax
	pushl	%eax
	call	fwrite@PLT
	addl	$28, %esp
	pushl	$0
	pushl	%edi
	pushl	%esi
	call	fseek@PLT
	addl	$32, %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	ret
	.p2align 2
.L120:
	pushl	%eax
	pushl	$43605
	leal	.LC32@GOTOFF(%ebx), %eax
	pushl	%eax
	pushl	$1
	call	__printf_chk@PLT
	addl	$16, %esp
	jmp	.L118
	.size	write_bootloader_signature, .-write_bootloader_signature
	.p2align 2
	.type	error, @function
error:
	endbr32
	pushl	%ebx
	subl	$8, %esp
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	leal	20(%esp), %eax
	pushl	%eax
	pushl	20(%esp)
	pushl	$1
	movl	stderr@GOT(%ebx), %eax
	pushl	(%eax)
	call	__vfprintf_chk@PLT
	call	__errno_location@PLT
	addl	$16, %esp
	movl	(%eax), %eax
	testl	%eax, %eax
	jne	.L124
.L122:
	subl	$12, %esp
	pushl	$1
	call	exit@PLT
.L124:
	subl	$12, %esp
	pushl	$0
	call	perror@PLT
	addl	$16, %esp
	jmp	.L122
	.size	error, .-error
	.section	.rodata
	.align 4
	.type	__PRETTY_FUNCTION__.2873, @object
	.size	__PRETTY_FUNCTION__.2873, 14
__PRETTY_FUNCTION__.2873:
	.string	"create_images"
	.align 4
	.type	__PRETTY_FUNCTION__.2884, @object
	.size	__PRETTY_FUNCTION__.2884, 13
__PRETTY_FUNCTION__.2884:
	.string	"create_image"
	.align 4
	.type	__PRETTY_FUNCTION__.2893, @object
	.size	__PRETTY_FUNCTION__.2893, 10
__PRETTY_FUNCTION__.2893:
	.string	"read_ehdr"
	.align 4
	.type	__PRETTY_FUNCTION__.2901, @object
	.size	__PRETTY_FUNCTION__.2901, 10
__PRETTY_FUNCTION__.2901:
	.string	"read_phdr"
	.align 4
	.type	__PRETTY_FUNCTION__.2906, @object
	.size	__PRETTY_FUNCTION__.2906, 14
__PRETTY_FUNCTION__.2906:
	.string	"process_start"
	.align 4
	.type	__PRETTY_FUNCTION__.2931, @object
	.size	__PRETTY_FUNCTION__.2931, 14
__PRETTY_FUNCTION__.2931:
	.string	"write_os_size"
	.align 4
	.type	__PRETTY_FUNCTION__.2936, @object
	.size	__PRETTY_FUNCTION__.2936, 26
__PRETTY_FUNCTION__.2936:
	.string	"prepare_process_directory"
	.align 4
	.type	__PRETTY_FUNCTION__.2943, @object
	.size	__PRETTY_FUNCTION__.2943, 24
__PRETTY_FUNCTION__.2943:
	.string	"write_process_directory"
	.section	.text.__x86.get_pc_thunk.bx,"axG",@progbits,__x86.get_pc_thunk.bx,comdat
	.globl	__x86.get_pc_thunk.bx
	.hidden	__x86.get_pc_thunk.bx
	.type	__x86.get_pc_thunk.bx, @function
__x86.get_pc_thunk.bx:
	movl	(%esp), %ebx
	ret
	.section	.text.__x86.get_pc_thunk.di,"axG",@progbits,__x86.get_pc_thunk.di,comdat
	.globl	__x86.get_pc_thunk.di
	.hidden	__x86.get_pc_thunk.di
	.type	__x86.get_pc_thunk.di, @function
__x86.get_pc_thunk.di:
	movl	(%esp), %edi
	ret
	.ident	"GCC: (Ubuntu 9.4.0-1ubuntu1~20.04.1) 9.4.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 4
	.long	 1f - 0f
	.long	 4f - 1f
	.long	 5
0:
	.string	 "GNU"
1:
	.align 4
	.long	 0xc0000002
	.long	 3f - 2f
2:
	.long	 0x3
3:
	.align 4
4:
