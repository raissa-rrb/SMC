#include <systemc>
#include <limits>

#include "int_tab.h"
#include "vci_vgsb.h"
#include "vci_gcd_master.h"
#include "vci_gcd_coprocessor.h"
#include "vci_signals.h"
#include "vci_param.h"
#include "mapping_table.h"

#define GCD_BASE	0x96000000
#define GCD_BASE2	0x97000000
#define GCD_BASE3	0x98000000
#define GCD_SIZE	0x10
	
int sc_main(int argc, char *argv[])
{
        using namespace sc_core;
	using namespace soclib::caba;
	using namespace soclib::common;

	// VCI fields width definition
	//	cell_size	= 4;
	// 	plen_size	= 8;
	// 	addr_size	= 32;
	// 	rerror_size	= 1;
	// 	clen_size	= 1;
	// 	rflag_size	= 1;
	// 	srcid_size	= 12;
	// 	trdid_size	= 1;
	// 	pktid_size	= 1;
	// 	wrplen_size	= 1;

	typedef VciParams< 4, 8, 32, 1, 1, 1, 12, 1, 1, 1 > vci_param;

	///////////////////////////////////////////////////////////////////////////
	// simulation arguments : number of cycles & seed for the random generation
	///////////////////////////////////////////////////////////////////////////
        int ncycles = std::numeric_limits<int>::max();
        int seed    = 123456789;
        if (argc > 1) ncycles = atoi(argv[1]) ;
        if (argc > 2) seed = atoi(argv[2]) ;

	//////////////////////////////////////////////////////////////////////////
	// Mapping Table
	//////////////////////////////////////////////////////////////////////////
	MappingTable maptab(32, IntTab(8), IntTab(8), 0x03000000);
	maptab.add(soclib::common::Segment("GCD", GCD_BASE, GCD_SIZE, IntTab(0), true));
	maptab.add(soclib::common::Segment("GCD2", GCD_BASE2, GCD_SIZE, IntTab(1), true));
	maptab.add(soclib::common::Segment("GCD3", GCD_BASE3, GCD_SIZE, IntTab(2), true));
	//le segment ajouté est le segment du coproc (la cible) car pour chaque cible un segment est attribué 
	// ET ce segment possède l'index VCI de la cible à laquelle il est assigné
	//ici on peut voir que le segment possède l'index IntTab(0) <=> l'index du coproc est IntTab(0)
	std::cout << std::endl << maptab << std::endl;

	//////////////////////////////////////////////////////////////////////////
        // Signals
	//////////////////////////////////////////////////////////////////////////
        sc_clock               		signal_clk("signal_clk", sc_time( 1, SC_NS ), 0.5 );
        sc_signal<bool> 		signal_resetn("signal_resetn");
        VciSignals<vci_param> 		signal_vci_i("signal_vci_i");
        VciSignals<vci_param> 		signal_vci1_i("signal_vci1_i");
        VciSignals<vci_param> 		signal_vci2_i("signal_vci2_i");

        VciSignals<vci_param> 		signal_vci_t("signal_vci_t");
        VciSignals<vci_param> 		signal_vci1_t("signal_vci1_t");
        VciSignals<vci_param> 		signal_vci2_t("signal_vci2_t");

	//////////////////////////////////////////////////////////////////////////
	// Components
	//////////////////////////////////////////////////////////////////////////
	VciGcdCoprocessor<vci_param>	coproc("coproc", IntTab(0), maptab );
	VciGcdCoprocessor<vci_param>	coproc1("coproc1", IntTab(1), maptab );
	VciGcdCoprocessor<vci_param>	coproc2("coproc2", IntTab(2), maptab );

        VciGcdMaster<vci_param> 	master("master", IntTab(3), maptab, seed, GCD_BASE);
        VciGcdMaster<vci_param> 	master1("master1", IntTab(4), maptab, seed, GCD_BASE2);
        VciGcdMaster<vci_param> 	master2("master2", IntTab(5), maptab, seed, GCD_BASE3);
	
	VciVgsb<vci_param>		vgsb("vgsb", maptab, 3, 3);
	//////////////////////////////////////////////////////////////////////////
	// Net-List
	//////////////////////////////////////////////////////////////////////////
	master.p_clk(signal_clk);
	master.p_resetn(signal_resetn);
	master.p_vci(signal_vci_i);

	master1.p_clk(signal_clk);
	master1.p_resetn(signal_resetn);
	master1.p_vci(signal_vci1_i);
	
	master2.p_clk(signal_clk);
	master2.p_resetn(signal_resetn);
	master2.p_vci(signal_vci2_i);

	coproc.p_clk(signal_clk);
	coproc.p_resetn(signal_resetn);
	coproc.p_vci(signal_vci_t);
	
	coproc1.p_clk(signal_clk);
	coproc1.p_resetn(signal_resetn);
	coproc1.p_vci(signal_vci1_t);
	
	coproc2.p_clk(signal_clk);
	coproc2.p_resetn(signal_resetn);
	coproc2.p_vci(signal_vci2_t);
	
	vgsb.p_clk(signal_clk);	
	vgsb.p_resetn(signal_resetn);
	vgsb.p_to_initiator[0](signal_vci_i);
	vgsb.p_to_initiator[1](signal_vci1_i);
	vgsb.p_to_initiator[2](signal_vci2_i);
	vgsb.p_to_target[0](signal_vci_t);
	vgsb.p_to_target[1](signal_vci1_t);
	vgsb.p_to_target[2](signal_vci2_t);
	//////////////////////////////////////////////////////////////////////////
	// simulation
	//////////////////////////////////////////////////////////////////////////
        signal_resetn = false;
        sc_start( sc_time( 1, SC_NS ) ) ;

        signal_resetn = true;
        for ( size_t n=1 ; n<ncycles ; n++) sc_start( sc_time( 1, SC_NS ) ) ;

        return(0);

} // end sc_main

