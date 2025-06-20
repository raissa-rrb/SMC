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


#include <iostream>

#include "gcd.h"
#include "vci_gcd_coprocessor.h"

using namespace sc_core;
using namespace soclib::caba;

namespace soclib {
namespace caba {

template<typename vci_param> 
VciGcdCoprocessor<vci_param>::VciGcdCoprocessor(sc_module_name insname,
						const soclib::common::IntTab &index,
						const soclib::common::MappingTable &mt)
	: sc_module(insname),
      	r_vci_fsm("r_vci_fsm"),
      	r_exe_fsm("r_exe_fsm"),
      	r_srcid("r_srcid"),
        r_trdid("r_trdid"),
        r_pktid("r_pktid"),
        r_opa("r_opa"),
        r_opb("r_opb"),
        m_segment(mt.getSegment(index)),
        p_resetn("resetn"),
        p_clk("clk"),
        p_vci("vci")
{
	SC_METHOD(transition);
	dont_initialize();
	sensitive << p_clk.pos();
	
	SC_METHOD(genMoore);
	dont_initialize();
	sensitive << p_clk.neg(); 
	//TODO genMoore sensible au front descendant de clk (tp1)
}

////////////////////////////
template<typename vci_param> 
VciGcdCoprocessor<vci_param>::~VciGcdCoprocessor()
{
}

////////////////////////////
template<typename vci_param> 
void VciGcdCoprocessor<vci_param>::transition()
{
	if ( !p_resetn.read() ) {
		r_vci_fsm = VCI_GET_CMD ; 
		r_exe_fsm = EXE_IDLE ; 
		return;
	}

#ifdef SOCLIB_MODULE_DEBUG
std::cout << name() << "  vci_fsm = " << r_vci_fsm.read() << std::endl;
std::cout << name() << "  exe_fsm = " << r_exe_fsm.read() << std::endl;
std::cout << name() << "  opa_reg = " << r_opa.read() << std::endl;
std::cout << name() << "  opb_reg = " << r_opb.read() << std::endl;
#endif
	/////////////////////////////
	switch ( r_exe_fsm.read() ) {
	case EXE_IDLE:
		if( r_vci_fsm.read() == VCI_RSP_START ) r_exe_fsm = EXE_COMPARE ; 
		break;
	case EXE_COMPARE:
		if      ( r_opa.read() < r_opb.read() )	r_exe_fsm = EXE_DECB;
		else if ( r_opb.read() < r_opa.read() )	r_exe_fsm = EXE_DECA;
		else					r_exe_fsm = EXE_IDLE ;
		break;
	case EXE_DECA:
		r_opa     = r_opa.read() - r_opb.read() ;
		r_exe_fsm =  EXE_COMPARE;
		break;
	case EXE_DECB:
		r_opb     = r_opb.read() - r_opa.read();
		r_exe_fsm = EXE_COMPARE;
		break;
	} // end switch exe-fsm

	/////////////////////////////
	switch ( r_vci_fsm.read() ) {
	case VCI_GET_CMD:
		if ( p_vci.cmdval.read() ) {
			typename vci_param::addr_t address = p_vci.address.read();
			uint32_t cell  = ( address - m_segment.baseAddress() ) / vci_param::B;
			// only accepts single word requests & checks for segmentation violations
			assert ( ( p_vci.eop.read() ) && 
			 ( p_vci.plen.read() == 4 ) && 
			  ( p_vci.cmd.read() != vci_param::CMD_LOCKED_READ ) && 
			 ( p_vci.cmd.read() != vci_param::CMD_STORE_COND ) &&
			 ( m_segment.contains(address) ) &&
                         	   "illegal command received by the GCD coprocessor");
			// store the VCI command in registers
			r_srcid	= p_vci.srcid ;
			r_trdid	= p_vci.trdid ;
			r_pktid	= p_vci.pktid ;
			// test the command
			if ( ( p_vci.cmd.read() == vci_param::CMD_READ ) && ( cell == GCD_OPA ) ) { 
				r_vci_fsm =  VCI_RSP_RESULT;
			} else if ( ( p_vci.cmd.read() == vci_param::CMD_READ ) && ( cell == GCD_STATUS ) ) {
				r_vci_fsm = VCI_RSP_STATUS;
			} else if ( ( p_vci.cmd.read() == vci_param::CMD_WRITE ) && ( cell == GCD_OPA ) ) {
				r_opa     = p_vci.wdata.read();
				r_vci_fsm = VCI_RSP_OPA;
			} else if ( ( p_vci.cmd.read() == vci_param::CMD_WRITE ) && ( cell == GCD_OPB ) ) {
				r_opb     = p_vci.wdata.read();//2;//p_vci.wdata;
				r_vci_fsm = VCI_RSP_OPB;
			} else if ( ( p_vci.cmd.read() == vci_param::CMD_WRITE ) && ( cell == GCD_START ) ) {
				r_vci_fsm = VCI_RSP_START;
			} else {	
				std::cout << "illegal command to the GCD coprocessor" << std::endl;
				exit(0);
			}
		}
 		break;
	case VCI_RSP_OPA:
	case VCI_RSP_OPB:
	case VCI_RSP_START:
	case VCI_RSP_STATUS:
	case VCI_RSP_RESULT:
		if ( p_vci.rspack.read() ) 	r_vci_fsm = VCI_GET_CMD;
		break;
	} // end switch vci_fsm
} // end transition()

////////////////////////////////////////
template<typename vci_param> 
void VciGcdCoprocessor<vci_param>::genMoore()
{
	// sorties ind�pendantes de l'�tat de l'automate
	p_vci.rsrcid = r_srcid ;
	p_vci.rtrdid = r_trdid ;
	p_vci.rpktid = r_pktid ;
	p_vci.rerror = (( p_vci.eop.read() ) && 
			 	 ( p_vci.plen.read() == 4 ) &&
				 ( p_vci.cmd.read() != vci_param::CMD_LOCKED_READ ) &&
				 ( p_vci.cmd.read() != vci_param::CMD_STORE_COND ) &&
			 	 ( m_segment.contains(p_vci.address.read()) )) ? 1 : 0;
	p_vci.reop   = p_vci.eop ;

	switch (r_vci_fsm) {
	case VCI_GET_CMD:
		p_vci.cmdack = true ;
		p_vci.rspval = false ;
		p_vci.rdata  = 0 ;
		break;
	case VCI_RSP_OPA:
	case VCI_RSP_OPB:
	case VCI_RSP_START:
		p_vci.cmdack = false ; // <=> ok cmd valide 
		p_vci.rspval = true;
		p_vci.rdata  = 0;
		break;
	case VCI_RSP_STATUS:
		p_vci.cmdack = false; 
		p_vci.rspval = true ; 
		p_vci.rdata  = (r_exe_fsm == EXE_IDLE ) ? 0 : -1 ;
		break;
	case VCI_RSP_RESULT:
		p_vci.cmdack = false ;
		p_vci.rspval = true ; //return res if coproc done
		p_vci.rdata  = r_opa.read() ; //if done then in exe_idle ?
	}
} // end genMoore()

template class VciGcdCoprocessor<soclib::caba::VciParams<4, 8, 32, 1, 1, 1, 12, 1, 1, 1> >;

}}

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4
