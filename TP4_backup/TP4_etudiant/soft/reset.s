#################################################################################
#	File : reset.s
#	Author : Alain Greiner
#	Date : 28/11/2013
#################################################################################
#	- It initializes the Interrupt vector
#	- It initializes the Status Register (SR) 
#	- It defines the stack size  and initializes the stack pointer ($29) 
#	- It initializes the EPC register, and jump to the user code    
#################################################################################
		
	.section .reset,"ax",@progbits

	.extern	seg_stack_base
	.extern	seg_data_base
	.extern	seg_icu_base
	.extern _interrupt_vector
	.extern _isr_timer
	.extern _isr_tty_get_task0
	.extern _isr_dma
	.extern _isr_ioc

	.globl  reset	 			# makes reset an external symbol 
	.ent	reset
	.align	2

reset:
       	.set noreorder

# initializes stack pointer
    la	    $27,    seg_stack_base
    li	    $26,    0x10000		# $26 <= 64K
    addu    $29,    $26, $27		# $29 <= seg_stack_base + 64K

# initializes interrupt vector
    la      $26,    _interrupt_vector   # interrupt vector address
    la      $27,    _isr_timer           
    sw      $27,    0($26)              # interrupt_vector[0] <= _isr_timer
    la      $27,    _isr_tty_get_task0              
    sw      $27,    0($26)              # interrupt_vector[1] <= _isr_tty_get_task0
    la      $27,    _isr_ioc              
    sw      $27,    0($26)            # interrupt_vector[2] <= _isr_ioc
    la      $27,    _isr_dma            
    sw      $27,    0($26)           # interrupt_vector[3] <= _isr_dma

# initializes ICU
    la      $26,    seg_icu_base
    li      $27,    0xF         # IRQ[0] IRQ[1] IRQ[2] IRQ[3]
    sw      $27,    8($26)      # ICU_MASK_SET

# initializes SR register
    li	    $26,    0x0000FF13		
    mtc0    $26,    $12			# SR <= user mode / IRQ enable (after eret)

# jumps to main 
    la      $26,    seg_data_base
    lw      $26,    0($26)
    mtc0    $26,    $14
    eret

	.end	reset
	.size	reset, .-reset

	.set reorder
