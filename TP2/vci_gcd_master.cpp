/* -*- c++ -*-
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


#include <iostream>
#include <cstdlib>


#include "vci_gcd_master.h"
#include "gcd.h"

using namespace sc_core;
using namespace soclib::caba;

namespace soclib { namespace caba {

////////////////////////////
template<typename vci_param>
VciGcdMaster<vci_param>::VciGcdMaster(  sc_module_name insname,
					const soclib::common::IntTab &index,
					const soclib::common::MappingTable &mt,
					const int seed,
					const typename vci_param::addr_t base)
      : sc_module(insname),
      	r_fsm("r_fsm"),
      	r_opa("r_opa"),
      	r_opb("r_opb"),
      	r_res("r_res"),
	r_cycle("r_cycle"),
	m_srcid(mt.indexForId(index)),
	m_base(base),
        p_resetn("p_resetn"),
        p_clk("p_clk"),
        p_vci("p_vci")
{
	SC_METHOD(transition);
	dont_initialize();
	sensitive << p_clk.pos();
	
	SC_METHOD(genMoore);
	dont_initialize();
	sensitive << p_clk.neg();

	srand(seed);
}

////////////////////////////
template<typename vci_param>
VciGcdMaster<vci_param>::~VciGcdMaster( )
{
}

////////////////////////////////
template<typename vci_param>
void VciGcdMaster<vci_param>::transition()
{
	if ( !p_resetn.read() ) {
		r_fsm = RANDOM;
		r_cycle = 0;
		r_iter = 0;
		return;
	}

#ifdef SOCLIB_MODULE_DEBUG
std::cout << "#################### cycle = " << r_cycle.read() << std::endl; 
std::cout << name() << "  fsm = " << r_fsm.read() << std::endl;
std::cout << name() << "  opa = " << r_opa.read() << std::endl;
std::cout << name() << "  opb = " << r_opb.read() << std::endl;
#endif
  
	switch ( r_fsm ) {
	case RANDOM :
		r_iter = r_iter.read() + 1;
		r_opa = rand() ;
		r_opb = rand() ;
		r_fsm =  CMD_OPA ;
		std::cout << "opa =" << r_opa << std::endl;
		std::cout << "opb ="<< r_opb << std::endl;
		break;
	case CMD_OPA :
		if ( p_vci.cmdack.read() ) {
			r_fsm = RSP_OPA;
		}
		break;
	case RSP_OPA :
		if ( p_vci.rspval.read() ) {
			r_fsm = CMD_OPB;
		}
		break;
	case CMD_OPB :
		if ( p_vci.cmdack.read() ) {
			r_fsm = RSP_OPB;
		}
		break;
	case RSP_OPB :
		if ( p_vci.rspval.read() ) {
			r_fsm = CMD_START;
		}
		break;
	case CMD_START :
		if ( p_vci.cmdack.read() ) {
			r_fsm =  RSP_START ;
		}
		break;
	case RSP_START :
		if ( p_vci.rspval.read() ) {
			r_fsm = CMD_STATUS ;
		}
		break;
	case CMD_STATUS :
		if ( p_vci.cmdack.read() ) {
			r_fsm = RSP_STATUS;
		}
		break;
	case RSP_STATUS :
		if ( p_vci.rspval.read() ) {
			if( p_vci.rdata.read() == 0 )	r_fsm = CMD_RESULT;
			else				r_fsm =  CMD_STATUS;
		}
		break;
	case CMD_RESULT :
		if ( p_vci.cmdack.read() ) {
			r_fsm = RSP_RESULT;
		}
		break;
	case RSP_RESULT :
		if ( p_vci.rspval.read() ) {
			r_res =  p_vci.rdata.read();
			r_fsm = DISPLAY;
		}
		break;
	case DISPLAY :
		r_fsm = RANDOM;
		std::cout << std::dec << std::noshowbase;
		std::cout << "*** " << name() << " *** iteration " << r_iter.read() << std::endl;
		std::cout << "  cycle  = " << r_cycle.read() << std::endl;
		std::cout << "  opa    = " << r_opa.read() << std::endl;
		std::cout << "  opb    = " << r_opb.read() << std::endl;
		std::cout << "  pgcd   = " << r_res.read() << std::endl;
		break;
        } // end switch
	
	// always increment cycle count 
	r_cycle = r_cycle + 1;

} // end transition()

//////////////////////////////
template<typename vci_param>
void VciGcdMaster<vci_param>::genMoore()
{
	// contant VCI command fields
	p_vci.be 	= 1;	// no masking //be = nb octets mot data ?
	p_vci.plen	= 4;	// all transactions are single flit
	p_vci.eop	= 1;	// all transactions are single flit
	p_vci.srcid	= 1;	// initiator ID
	p_vci.trdid	= 0;		// unused
	p_vci.pktid	= 0;		// unused
	p_vci.contig	= true;		// unused
	p_vci.cons	= false;	// unused
	p_vci.wrap	= false;	// unused
	p_vci.cfixed	= false;	// unused
	p_vci.clen	= 0;		// unused
	
	switch ( r_fsm.read() ) {
	case RANDOM : 
        case DISPLAY :
		p_vci.cmdval	= false ; //pas d'envoie de com ou att de rep
		p_vci.rspack	= false ; //pas d'envoie de com ou att de rep
		break; 
	case CMD_OPA :
		p_vci.cmdval	= true;
		p_vci.cmd	= vci_param::CMD_WRITE;
		p_vci.address	= m_base + 4*GCD_OPA;
		p_vci.wdata	= r_opa.read();
		p_vci.rspack	= false;
		break; 
	case CMD_OPB :
		p_vci.cmdval	= true; //j'envoie une cmd
		p_vci.cmd	= vci_param::CMD_WRITE;
		p_vci.address	= m_base + 4*GCD_OPB;
		p_vci.wdata	= r_opb.read();
		p_vci.rspack	= false; //true si j'ai reçu une réponse de la cible
		break; 
	case CMD_START :
		p_vci.cmdval	= true; //j'envoie une commande
		p_vci.cmd	= vci_param::CMD_WRITE;
		p_vci.address	= m_base + 4*GCD_START;
		p_vci.wdata	=  0; 
		p_vci.rspack	=false; //je n'ai pas reçu de cmd à acknowledge
		break; 
	case CMD_RESULT :
		p_vci.cmdval	= true;
		p_vci.cmd	= vci_param::CMD_READ; //READ ?
		p_vci.address	= m_base + 4*GCD_OPA;
		p_vci.wdata	= r_res.read();
		p_vci.rspack	= false;
		break; 
	case CMD_STATUS :
		p_vci.cmdval	= true; 
		p_vci.cmd	= vci_param::CMD_READ;
		p_vci.address	= m_base + 4*GCD_STATUS;
		p_vci.wdata	= 0;
		p_vci.rspack	= false;
		break; 
	case RSP_OPA :
	case RSP_OPB :
	case RSP_START :
	case RSP_RESULT :
	case RSP_STATUS :
		p_vci.cmdval	= false;
		p_vci.cmd	= 0;
		p_vci.address	= 0;
		p_vci.wdata	= 0;
		p_vci.rspack	= true;
		break; 
     } // end switch
} // end genMoore()

template class VciGcdMaster<soclib::caba::VciParams<4, 8, 32, 1, 1, 1, 12, 1, 1, 1>>;

}}

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4

