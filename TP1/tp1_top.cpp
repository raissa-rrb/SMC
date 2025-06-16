#include <systemc>
#include <limits>

#include "fifo_gcd_master.h"
#include "fifo_gcd_coprocessor.h"
#include "fifo_ports.h"
#include "fifo_signals.h"
#include <stdlib.h>

int sc_main(int argc, char *argv[])
{
        using namespace sc_core;
	using namespace soclib::caba;

	/////////////////////////////////////////////////////////////////
	// Arguments : number of cycles & seed for the random generation
	/////////////////////////////////////////////////////////////////
	int ncycles = std::numeric_limits<int>::max();
        int seed    = 123456789;
	if (argc > 1) ncycles = atoi(argv[1]) ;
	if (argc > 2) seed = atoi(argv[2]) ;

	/////////////////////////////////////////////////////////////////
        // Signals
	/////////////////////////////////////////////////////////////////
        sc_clock                		signal_clk("signal_clk", sc_time( 1, SC_NS ), 0.5 );
        sc_signal<bool> 			signal_resetn("signal_resetn");
        FifoSignals<uint32_t> 			signal_fifo_m2c("signal_m2c");
	FifoSignals<uint32_t> 			signal_fifo_c2m("signal_c2m");

	/////////////////////////////////////////////////////////////////
	// Components
	/////////////////////////////////////////////////////////////////
        FifoGcdMaster 				master("master", seed);
	FifoGcdCoprocessor			coproc("coproc");

	/////////////////////////////////////////////////////////////////
	// Net-List
	/////////////////////////////////////////////////////////////////
	master.p_clk(signal_clk); 
	master.p_resetn(signal_resetn);
	master.p_in(signal_fifo_c2m);
	master.p_out(signal_fifo_m2c);
	coproc.p_clk(signal_clk); 
	coproc.p_resetn(signal_resetn);
	coproc.p_in(signal_fifo_m2c);
	coproc.p_out(signal_fifo_c2m);

	/////////////////////////////////////////////////////////////////
	// simulation
	/////////////////////////////////////////////////////////////////
	sc_start(0);

	signal_resetn = false;
	sc_start( sc_time( 1, SC_NS ) ) ;

	signal_resetn = true;
	for (size_t n=1 ; n<ncycles ; n++ )  sc_start( sc_time( 1, SC_NS ) ) ;

	return(0);

} // end sc_main












