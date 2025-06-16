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
 * WITHOUT ANY WARr_opaNTY; without even the implied warranty of
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

#include <stdlib.h>

#include "fifo_gcd_master.h"

using namespace sc_core;

namespace soclib {
namespace caba {

//////////////////////////////////////////////////////
FifoGcdMaster::FifoGcdMaster( sc_module_name insname, int seed )
	: sc_module(insname),
      	r_fsm("r_fsm"),
      	r_opa("r_opa"),
      	r_opb("r_opb"),
      	r_res("r_res"),
        p_resetn("p_resetn"),
        p_clk("p_clk"),
        p_in("p_in"),
	p_out("p_out")
{
	SC_METHOD(transition);
	dont_initialize();
	sensitive << p_clk.pos();
	
	SC_METHOD(genMoore);
	dont_initialize();
	sensitive << p_clk.neg();

	srand(seed);
}

FifoGcdMaster::~FifoGcdMaster( )
{
}

////////////////////////////////
void FifoGcdMaster::transition()
{
	if ( !p_resetn.read() ) {
		r_fsm = RANDOM;
		r_cyclecount = 0;
		r_iterationcount = 0;
		return;
	}

	switch ( r_fsm.read() ) {
	case RANDOM :
		r_iterationcount = r_iterationcount.read() + 1;
		r_opa = rand();
		r_opb = rand();
		r_fsm = WRITE_OPA;
		break;
	case WRITE_OPA :
		if ( p_out.wok.read() ) {
			r_fsm = WRITE_OPB;
		}
		break;
	case WRITE_OPB :
		if ( p_out.wok.read() ) {
			r_fsm = READ_RES;
		}
		break;
	case READ_RES :
		if ( p_in.rok.read() ) {
			r_res = p_in.data.read();
			r_fsm = DISPLAY;
		}
		break;
	case DISPLAY :
		r_fsm = RANDOM;
		std::cout << "************************ iteration " << r_iterationcount.read() << std::endl;
		std::cout << "  cycle = " << r_cyclecount.read() << std::endl;
		std::cout << "  opa   = " << r_opa.read() << std::endl;
		std::cout << "  opb   = " << r_opb.read() << std::endl;
		std::cout << "  pgcd  = " << r_res.read() << std::endl;
		break;
        } // end switch
	
	// always increment cycle count 
	r_cyclecount = r_cyclecount.read() + 1;

} // end transition()

//////////////////////////////
void FifoGcdMaster::genMoore()
{
	switch ( r_fsm.read() ) {
	case RANDOM : 
        case DISPLAY :
		p_in.r 		= false;
		p_out.w		= false;
                p_out.data	= 0;
		break; 
	case WRITE_OPA :
		p_in.r 		= false;
		p_out.w		= true;
                p_out.data	= r_opa;
		break; 
	case WRITE_OPB :
		p_in.r 		= false;
		p_out.w		= true;
                p_out.data	= r_opb;
		break; 
	case READ_RES :
		p_in.r 		= true;
		p_out.w		= false;
                p_out.data	= 0;
		break; 
     } // end switch
} // end genMoore()

}}

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4

