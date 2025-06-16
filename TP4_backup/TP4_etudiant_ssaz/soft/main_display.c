#include "stdio.h"

#define NBLOCKS 128*128/512

__attribute__((constructor)) void main()
{
	char buffer[128][128];
	int img = 10;
	int i;
	char next;

    	while(1){
	
	for(i = 0; i < img; i++){

	//4.4
	/* use ioc_read and ioc_completed to load image in buffer. 
	   use fb_write to display this image
	*/

	//ret 0 if success otherwise >0
	if( ioc_read(i*NBLOCKS, &buffer, NBLOCKS)!=0) {
		tty_printf("error ioc_read\n");
		exit();
	}
	
	//blocking func
	//ret 0 if success or 1 if error
	if (ioc_completed() != 0){
		tty_printf("error ioc_completed\n");
		exit();
	}
	
	//non blocking
	//ret 0 if success, otherwise >0
 	if ( fb_write(0,  &buffer , 128*128) != 0) {
		tty_printf("error fb_write\n");
		exit();
	}
	
	tty_printf("tap to get next img\n");
	tty_getc_irq(&next);

	}//endof for

	
	}//endof while

    exit();
} // end main
