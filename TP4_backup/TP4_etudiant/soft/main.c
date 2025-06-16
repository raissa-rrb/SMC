#include "stdio.h"

__attribute__((constructor)) void main()
{
    while (1) {

	timer_set_mode(3);
	timer_set_period(500000);

    }
    exit();
} // end main
