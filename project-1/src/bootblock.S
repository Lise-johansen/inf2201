# bootblock.s
# Source: load kenrel into memory is inspierd of this: https://en.wikipedia.org/wiki/INT_13H 

# .equ symbol, expression
# These directive set the value of the symbol to the expression
  .equ    BOOT_SEGMENT,      0x07c0
  .equ    DISPLAY_SEGMENT,   0xb800
  .equ    KERNEL_SEGMENT,    0x0000
  .equ    KERNEL_OFFSET,     0x1000

# Decided where to put the stack, have 8 stack the 2. one. The firt one is from powerpoint
 .equ    STACK_SEGMENT,  0x07e0 # 0x7F00 # 0x7e001 # 0x7e00:0x1000 0x07e0, thinik 52
 .equ    STACK_POINTER,  0x8F00 # 0x8e001 # 0x8e00:0x1000 0x9fe0

.text                   # Code segment
.globl    _start        # The entry point must be global
.code16                 # Real mode 
.org 0x0 # Start point 

_start:
  jmp myCode

kernelSize:
  .word 0                   # bootimage will write size of kernel, in sectors

myCode:
   #Setup stack
  movw $STACK_SEGMENT, %ax  # Move stack to a general purpose register
  movw %ax, %ss             # then to a segment register
  movw $STACK_POINTER, %sp  # stack pointer moved to segment pointer reg
 
  jmp loadKernel

loadKernel:
 # Setup data segment
  movw  $BOOT_SEGMENT, %ax  # memory address to general register
  movw %ax, %ds             # general register --> segment register  

  #Setup kernel
  movw $KERNEL_SEGMENT, %ax   # Move kernel segment to reg ax
  movw %ax, %es               # general reg --> segment reg    FROM ds to fs
  movw $KERNEL_OFFSET, %bx    # offset move to reg bx

  jmp readKernel

readKernel:
  #Move value 0 into Boot segment
  movw $0, %ax          # 0 into ax
  movw %ax, %ds         # then ax into ds, ds = BOOT_SEGMENT, Ds have value 0.

  #Load kernel into memory
  movb $0x02, %ah # Read from drive, int 13H function 2
  movb $0x09, %al # Hardcore = 0x09 for 9 sectores.  Dynamic = (kernelSize), where creatimage takes care of kernalSize. 
  movb $0x00, %ch # cylinder number = 0
  movb $0x02, %cl # sector number 2
  movb $0x00, %dh # Head number

  int $0x13 # calls BIOS

  jmp contorlTransfersToKernal

contorlTransfersToKernal:
  ljmp $KERNEL_SEGMENT, $KERNEL_OFFSET #jump and transfers control to kernel

  