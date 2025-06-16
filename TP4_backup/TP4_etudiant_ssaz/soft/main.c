#include "stdio.h"


__attribute__((constructor)) void main()
{
	char cmd;
    	while(1){

	//4.2
	/*
	timer_set_mode(3); //activated and periodic interrupts ok
	timer_set_period(500000); //period ? make sure it's good
	*/
	
	//4.3
	/* reads one char that sends commands
		- a : activates Timer's interrupts
		- d : desactivates timer's interrupts
		- q : exit()
	*/
	
	tty_printf("hello :) please type a command :\n");
	tty_getc_irq(&cmd); //blocking func, fetches car from term

	tty_printf("got it : %c\n", cmd);
	
	if (cmd == 'a') { 
		tty_printf("so you want to activate the timer's irq\n");
		timer_set_mode(2); //activate IRQs but don't generate them
		
		
	}
	else {if (cmd == 'd') { 
		tty_printf("so you want to deactivate the timer's irq\n");
		timer_set_mode(0); //deactivate IRQs
	}
	else {if (cmd == 'q') { 
		tty_printf("so you want to exit :(\n");
		exit();
	}
	else {
		tty_printf("sorry that's not a valid command :/\n");
	}}}


	tty_printf("\n");

		
	}

    exit();
} // end main
