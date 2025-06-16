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

////////////////////////////////////////////////////////////
// This component implements a simple hardware coprocessor
// performing the Larger Common Divider computation.
// It uses two FIFO interfaces :
// It reads successively two (uint32_t) operands in the
// input FIFO, computes the PGCD, and writes the
// (uint32_t) result to the output FIFO.
////////////////////////////////////////////////////////////

#ifndef FIFO_GCD_COPROCESSOR_H
#define FIFO_GCD_COPROCESSOR_H

#include <systemc>
#include "fifo_ports.h"

namespace soclib {
namespace caba {

////////////////////////
class FifoGcdCoprocessor
	: public sc_core::sc_module
{
	enum coprocessor_fsm_state_e {
		READ_OPA,
        READ_OPB,
        COMPARE,
        DECR_A,
        DECR_B,
        WRITE_RES,
	};

	// Registers
        sc_core::sc_signal<uint32_t> r_opa;
        sc_core::sc_signal<uint32_t> r_opb;
        sc_core::sc_signal<int>	r_fsm;

protected:
	SC_HAS_PROCESS(FifoGcdCoprocessor);

public:
	// ports
        FifoInput<uint32_t> p_in;
        FifoOutput<uint32_t> p_out;
        sc_core::sc_in<bool> p_resetn;
        sc_core::sc_in<bool> p_clk;

	// constructor & destructor
		FifoGcdCoprocessor( sc_module_name insname );
        ~FifoGcdCoprocessor();
        
private:
	// member functions
    	void genMoore();
        void transition();

}; // end class FifoGcdCoprocessor

}}

#endif 

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4

