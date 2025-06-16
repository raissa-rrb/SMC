#################################################################################
#	File : reset.s
#	Author : Alain Greiner
#	Date : 28/08/2006
#################################################################################
#	- It initializes the Status Register (SR) 
#	- It defines the stack size  and initializes the stack pointer ($29) 
#	- It initializes the EPC register, and jumps to user code
#################################################################################
		
	.section .reset,"ax",@progbits

	.extern	seg_stack_base
	.extern	seg_data_base

	.globl  reset	 			# makes reset an external symbol 
	.ent	reset
	.func	reset
	.align	2

reset:
       	.set noreorder

# initializes stack pointer
	la	 $27,	seg_stack_base
    li	 $26,	0x10000			# $26 <= 64K
	addu $29, 	$27, $26	# $29 <= seg_stack_base + 64K

# initializes EPC register   
	#la	$26,	main		# $26 <= address main
	#mtc0	$26,	$14		# EPC <= main

# initializes SR register
    li	$26,	0x0000FF13		
    mtc0	$26,	$12		# SR <= user mode / IRQ enable (after eret)

# jumps to main 
    la      $26,	seg_data_base         # $26 <= seg_data_base
    lw      $26,    0($26)    	# $26 <= M[seg_data_base]
    mtc0    $26,	$14         # EPC <= $26  (EPC == $14)
	eret

	.end	reset
	.size	reset, .-reset

	.set reorder
