#include "stdio.h"

__attribute__((constructor)) void  main()
{
	char  byte;

	while(1) 
        {
	    tty_puts("\nhello world\n");
	    tty_getc(&byte);
	}

} // end main
