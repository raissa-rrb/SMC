/* -*- c++ -*-
 *
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
 * Copyright (c) UPMC, Lip6
 *         Alain Greiner <alain.greiner@lip6.fr>, 2009
 *
 * Maintainers: alain
 */

/////////////////////////////////////////////////////////////////
// This component implements an "hardwired" master able
// to drive the "Larger Common Divider" coprocessor.
// It uses two FIFO interfaces, executing and infinite loop:
// It writes sucessively two (uint32_t) operands to the
// output fifo port, reads the result on the input fifo
// interface (uint32_t), and finally displays the results.
//
// Remark : this component emulates two complex functionnalities
// by using two stdlib functions :
// - the random generation of operands uses the rand() function
// - the result is displayed using the printf() function
//////////////////////////////////////////////////////////////////

#ifndef FIFO_GCD_MASTER_H
#define FIFO_GCD_MASTER_H

#include <systemc>
#include "fifo_ports.h"

namespace soclib {
namespace caba {

///////////////////
class FifoGcdMaster
	: public sc_core::sc_module
{
	enum fsm_state_e {
		RANDOM,
		WRITE_OPA,
		WRITE_OPB,
                READ_RES,
		DISPLAY,
	};

	// Registers
	sc_core::sc_signal<int>			r_fsm;
        sc_core::sc_signal<uint32_t>   		r_opa;
        sc_core::sc_signal<uint32_t>   		r_opb;
        sc_core::sc_signal<uint32_t>   		r_res;
        sc_core::sc_signal<uint32_t>   		r_cyclecount;
        sc_core::sc_signal<uint32_t>   		r_iterationcount;

protected:
	SC_HAS_PROCESS(FifoGcdMaster);

public:
	// ports
        sc_core::sc_in<bool>			p_resetn;
        sc_core::sc_in<bool> 			p_clk;
        soclib::caba::FifoInput<uint32_t> 	p_in;
        soclib::caba::FifoOutput<uint32_t> 	p_out;

	// constructor & destructor
        FifoGcdMaster( sc_core::sc_module_name insname, int seed );
    	~FifoGcdMaster();

private:
	// member functions
    	void transition();
    	void genMoore();

}; // end class FifoGcdMaster

}}

#endif 

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4

