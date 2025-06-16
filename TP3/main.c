#include "stdio.h"

__attribute__((constructor)) void  main()
{
	char  byte;
	int iteration = 0;

	while(1) 
        {
	    //tty_puts("\nhello world\n");
	   // tty_getc(&byte);
		
		tty_printf("cycle %d\n", proctime() );
	
		unsigned int a = rand();
		unsigned int b = rand();
		unsigned int status, res;

		status = 1;

		gcd_set_opa(a);
		gcd_set_opb(b);

		gcd_start();
		
		while (status != 0 )
		{
			gcd_get_status(&status);	
		}

		gcd_get_result(&res);

		tty_printf("cycle %d iteration %d opA %d opB %d pgcd %d\n",
				proctime(), iteration, a, b, res);

		iteration ++;

		tty_getc(&byte); //juste pour avoir un seul tour de boucle pour le moment
		
	}

} // end main
