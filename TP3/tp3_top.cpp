/**********************************************************************
 * File : tp3_top.cpp
 * Date : 15/11/2010
 * Author :  Alain Greiner
 * UPMC - LIP6
 * This program is released under the GNU public license
 *********************************************************************/

/*
 * SOCLIB_LGPL_HEADER_BEGIN
 * 
 * This file is part of SoCLib, GNU LGPLv2.1.
 * 
 * SoCLib is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; version 2.1 of the License.
 * 
 * SoCLib is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with SoCLib; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 * 
 * SOCLIB_LGPL_HEADER_END
 *
 * Copyright (c) UPMC, Lip6, Asim
 *         alain.greiner@lip6.fr
 *
 * Maintainers: alain
 */

#include <systemc>
#include <limits>

#include "vci_vgsb.h"
#include "vci_xcache_wrapper.h"
#include "mips32.h"
#include "vci_vgsb.h"
#include "vci_simple_ram.h"
#include "vci_multi_tty.h"
#include "vci_gcd_coprocessor.h"
#include "vci_signals.h"
#include "vci_param.h"
#include "mapping_table.h"

#define SEG_RESET_BASE	0xBFC00000 //true
#define SEG_RESET_SIZE	0x1000

#define SEG_KCODE_BASE	0x80000000
#define SEG_KCODE_SIZE	0x10000

#define SEG_KDATA_BASE	0x81000000
#define SEG_KDATA_SIZE	0x10000

#define SEG_KUNC_BASE 	0x82000000 //false
#define SEG_KUNC_SIZE 	0x10000

#define SEG_DATA_BASE 	0x01000000
#define SEG_DATA_SIZE 	0x10000

#define SEG_CODE_BASE 	0x00400000 //ok
#define SEG_CODE_SIZE 	0x10000

#define SEG_STACK_BASE	0x02000000
#define SEG_STACK_SIZE	0x10000

#define SEG_TTY_BASE  	0x90000000 //false
#define SEG_TTY_SIZE  	0x40

#define SEG_GCD_BASE  	0x95000000
#define SEG_GCD_SIZE  	0x10

// TGTID definition
#define TGTID_ROM	0
#define TGTID_RAM	1
#define TGTID_TTY	2
#define TGTID_GCD	3

// VCI fields width definition
#define cell_size	4
#define plen_size	8
#define addr_size	32
#define rerror_size	1
#define clen_size	1
#define rflag_size	1
#define srcid_size	12
#define trdid_size	1
#define pktid_size	1
#define wrplen_size	1

// Cache parameters definition
#define	icache_ways	4
#define	icache_sets	128
#define	icache_words	8
#define	dcache_ways	4
#define	dcache_sets	128
#define	dcache_words	8

int _main(int argc, char *argv[])
{
        using namespace sc_core;
	using namespace soclib::caba;
	using namespace soclib::common;

	typedef VciParams<cell_size,
                          plen_size,
                          addr_size,
                          rerror_size,
                          clen_size,
                          rflag_size,
                          srcid_size,
                          trdid_size,
                          pktid_size,
                          wrplen_size> vci_param;

	//////////////////////////////////
	// command line arguments 
	//////////////////////////////////
        int 	ncycles 	= 1000000000;		// Number of simulation cycles
        bool	debug 		= false;		// Debug trace activation
        int     from_cycle	= 0;			// start cycle for the trace
        char	sys_path[256]	= "soft/sys.bin";	// path-name for the system binary code
        char	app_path[256]	= "soft/app.bin";	// path-name for the application binary code

        if (argc > 1)
        {
            for( int n=1 ; n<argc ; n=n+2 )
            {
                if ( (strcmp(argv[n], "-DEBUG") == 0) && (n+1<argc) )
                {
                    debug = true;
                    from_cycle = atoi(argv[n+1]);
                }
                else if ( (strcmp(argv[n], "-NCYCLES") == 0) && (n+1<argc) )
                {
                    ncycles = atoi(argv[n+1]);
                }
                else if( (strcmp(argv[n],"-SYS") == 0) && (n+1<argc) )
                {
                    strcpy(sys_path, argv[n+1]);
                }
                else if( (strcmp(argv[n],"-APP") == 0) && (n+1<argc) )
                {
                    strcpy(app_path, argv[n+1]);
                }
                else
                {
                std::cout << "   Arguments on the command line are (key,value) couples." << std::endl;
                std::cout << "   The order is not important." << std::endl;
                std::cout << "   Accepted arguments are :" << std::endl << std::endl;
                std::cout << "   -NCYCLES number_of_simulated_cycles" << std::endl;
                std::cout << "   -SYS system elf pathname" << std::endl;
                std::cout << "   -APP application elf pathname" << std::endl;
                std::cout << "   -DEBUG debug_start_cycle" << std::endl;
                exit(0);

                }
            }
        }

	//////////////////////////////////////////////////////////////////////////
	// Mapping Table
	//////////////////////////////////////////////////////////////////////////
	MappingTable maptab(32, IntTab(8), IntTab(8), 0xFF000000);

	maptab.add(Segment("seg_reset", SEG_RESET_BASE, SEG_RESET_SIZE, IntTab(TGTID_ROM), true));

	maptab.add(Segment("seg_kcode", SEG_KCODE_BASE, SEG_KCODE_SIZE, IntTab(TGTID_RAM), true));
	maptab.add(Segment("seg_kdata", SEG_KDATA_BASE, SEG_KDATA_SIZE, IntTab(TGTID_RAM), true));
	maptab.add(Segment("seg_kunc" , SEG_KUNC_BASE , SEG_KUNC_SIZE , IntTab(TGTID_RAM), false));
	maptab.add(Segment("seg_code" , SEG_CODE_BASE , SEG_CODE_SIZE , IntTab(TGTID_RAM), true));
	maptab.add(Segment("seg_data" , SEG_DATA_BASE , SEG_DATA_SIZE , IntTab(TGTID_RAM), true));
	maptab.add(Segment("seg_stack", SEG_STACK_BASE, SEG_STACK_SIZE, IntTab(TGTID_RAM), true));

	maptab.add(Segment("seg_tty"  , SEG_TTY_BASE  , SEG_TTY_SIZE  , IntTab(TGTID_TTY), false));

	maptab.add(Segment("seg_gcd"  , SEG_GCD_BASE  , SEG_GCD_SIZE  , IntTab(TGTID_GCD), true));

	std::cout << std::endl << maptab << std::endl;

	//////////////////////////////////////////////////////////////////////////
        // Signals
	//////////////////////////////////////////////////////////////////////////
        sc_clock               		signal_clk("signal_clk", sc_time( 1, SC_NS ), 0.5 );
        sc_signal<bool> 		signal_resetn("signal_resetn");
        VciSignals<vci_param> 		signal_vci_proc("signal_vci_proc");
        VciSignals<vci_param> 		signal_vci_rom("signal_vci_rom");
        VciSignals<vci_param> 		signal_vci_ram("signal_vci_ram");
        VciSignals<vci_param> 		signal_vci_tty("signal_vci_tty");
        VciSignals<vci_param> 		signal_vci_gcd("signal_vci_gcd");
        sc_signal<bool> 		signal_false("signal_false");
        sc_signal<bool> 		signal_dummy("signal_dummy");

	//////////////////////////////////////////////////////////////////////////
	// Components
	//////////////////////////////////////////////////////////////////////////

	Loader	loader("soft/app.bin", "soft/sys.bin");

	VciXcacheWrapper<vci_param, Mips32ElIss>* proc;
	proc = new VciXcacheWrapper<vci_param, Mips32ElIss>("cacheWrapper", 0, maptab, IntTab(0), 1, 256, 4, 1, 256, 4); //0 = procid, ways, sets, words

	VciSimpleRam<vci_param>* rom;
	rom = new VciSimpleRam<vci_param>("rom", IntTab(TGTID_ROM), maptab, loader);//last arg = latence (opt)

	VciSimpleRam<vci_param>* ram;
	ram = new VciSimpleRam<vci_param>("ram", IntTab(TGTID_RAM), maptab, loader);

	VciMultiTty<vci_param>* tty;
	tty = new VciMultiTty<vci_param>("tty", IntTab(TGTID_TTY), maptab, "SSAZ ET MIMIPS",'\0');

        std::cout << "c'est bon" << std::endl;

	VciGcdCoprocessor<vci_param>* gcd;
	gcd = new VciGcdCoprocessor<vci_param>("coproc", IntTab(TGTID_GCD), maptab);

	VciVgsb<vci_param>* bus;		
	bus = new VciVgsb<vci_param>("vgsb", maptab, 1, 4);

	//////////////////////////////////////////////////////////////////////////
	// Net-List
	//////////////////////////////////////////////////////////////////////////
	proc->p_clk(signal_clk);
	proc->p_resetn(signal_resetn);
	proc->p_vci(signal_vci_proc);
	proc->p_irq[0](signal_false);
	proc->p_irq[1](signal_false);
	proc->p_irq[2](signal_false);
	proc->p_irq[3](signal_false);
	proc->p_irq[4](signal_false);
	proc->p_irq[5](signal_false);

	rom->p_clk(signal_clk);
	rom->p_resetn(signal_resetn);
	rom->p_vci(signal_vci_rom);

	ram->p_clk(signal_clk);
	ram->p_resetn(signal_resetn);
	ram->p_vci(signal_vci_ram);

	tty->p_clk(signal_clk);
	tty->p_resetn(signal_resetn);
	tty->p_vci(signal_vci_tty);
	tty->p_irq[0](signal_dummy);

	gcd->p_clk(signal_clk);
	gcd->p_resetn(signal_resetn);
	gcd->p_vci(signal_vci_gcd);

	bus->p_clk(signal_clk);
	bus->p_resetn(signal_resetn);
        bus->p_to_initiator[0](signal_vci_proc);
        bus->p_to_target[0](signal_vci_rom);
        bus->p_to_target[1](signal_vci_ram);
        bus->p_to_target[2](signal_vci_tty);
        bus->p_to_target[3](signal_vci_gcd);

	//////////////////////////////////////////////////////////////////////////
	// simulation
	//////////////////////////////////////////////////////////////////////////
        signal_false = false;
        signal_resetn = false;
        sc_start( sc_time( 1, SC_NS ) ) ;

        signal_resetn = true;
	for ( int n=1 ; n<ncycles ; n++ ) 
        {
            if ( debug && (n > from_cycle) )
            {
		std::cout << std::dec << "******************************************* cycle = "
                          << n << " **************************" << std::endl;
                proc->print_trace();
                rom->print_trace();
                ram->print_trace();
                gcd->print_trace();
                tty->print_trace();
                bus->print_trace();
            }

            sc_start( sc_time( 1 , SC_NS ) ) ;
	}

        return(0);

} // end _main

int sc_main(int argc, char *argv[])
{
	try {
		return _main(argc, argv);
	} catch ( std::exception &e ) {
		std::cout << e.what() << std::endl;
	}
	return 0;
}

