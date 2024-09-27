#	entry.S
#	This file contains the code for kernel_entry, as well as
#	scheduler_entry, which saves the context of the current process
#	before calling the scheduler. The file is pre-processed before it
#	is assembled, allowing (among other things) the use of macros
#	and C-style comments.

.data
	.align 4	
.text
.code32
	.align 4

# Make symbols visible for ld
.globl	kernel_entry
.globl	scheduler_entry


# void kernel_entry(int fn)
#
# SYSCALL(i) in syslib.c calls this function
# fn = system call number (defined in kernel.h)
#
ke_scratch:
	.long	0
ke_arg:
	.long	0
kernel_entry:

	# Stores the value in the eax register into a memory location ke_scratch
	movl	%eax, (ke_scratch)
	
	# Save argument
	movl	4(%esp), %eax
	movl	%eax, (ke_arg)

	# This code save register in current pcb and swiches from user stack to kernel stack. 

	# Push all flag and 
	pusha
	pushf

	
	movl current_running, %eax	 	# %eax = pcb_t *current_running;

	movl %esp, 0(%eax) 			 	# Move esp into user space 
	movl 4(%eax), %esp 			 	# Then load esp value from kernel stack

	
	pushl (ke_arg)				 	# Save argument on stack

	
	call kernel_entry_helper     	# call on the kernel_entry_helper function. 

	
	pop (ke_arg) 				 	# Pop argument
	
	
	movl current_running, %eax	 	# agin --> %eax = pcb_t *current_running;
	
	# Move user stack from kernel stack
	movl %esp, 4(%eax)			 	# Move value of esp into the kernel stack
	movl 0(%eax), %esp			 	# Load esp value intp user stack
	
	# Pop all argument and register
	popf
	popa
	

	ret


	
loop:
	jmp		loop

#	This function gets called to enter the scheduler, saving registers before doing so.
scheduler_entry:

# This code switch between to pcb stacks. Save register values onto pcb stack, call scheduler and start on a new process/ thread

	# push the register og all flags onto stack
	pushal
	pushfl
	
	# Move stored value in curren_running into esp register in two step.

	movl (current_running), %eax # First move current_running into a general register
	movl %esp, 4(%eax) 			 # Then move the general register into eax offset 4. e.g. kernel stack

	 fsave 8(%eax)				# Save floating point in buffer space with offset 8.

	
	call scheduler 				# call scheduler

	movl (current_running), %eax # So the offsett can be used agin in the next line.
	movl 4(%eax), %esp 			# move esp registere out of kernel stack


	frstor 8(%eax)				# Load floting value


	# Pop all reigster and flags from stack
	popfl
	popal
	 
	# Retun
	ret 
	
