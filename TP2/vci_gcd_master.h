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

/////////////////////////////////////////////////////////////////////
// This component implements a VCI compliant "hardwired" master able
// to drive the VCI compliant "Larger Common Divider" coprocessor.
// It uses one VCI initiator port, and executes and infinite loop:
// It writes sucessively the two operands in two memory mapped
// registers, starts the coprocessor by writing in another register
// and reads the result in a fourth memory mapped register.
//
// The base address of the segment associated to the GCD coprocessor
// and the seed used for random generation of the operands are
// defined by constructor parameters.
//
// The gcd coprocessor memory map is defined as follows :
// - operand A : 0x0
// - operand B : 0x4
// - start     : 0x8
// - result    : 0xC
//
// Remark : this component emulates two complex functionnalities
// by using two stdlib functions :
// - the random generation of operands uses the rand() function
// - the result is displayed using the printf() function
//////////////////////////////////////////////////////////////////

#ifndef VCI_GCD_MASTER_H
#define VCI_GCD_MASTER_H

#include <systemc>
#include "vci_initiator.h"
#include "int_tab.h"
#include "mapping_table.h"

namespace soclib { namespace caba {

////////////////////////////
template<typename vci_param>
class VciGcdMaster
	: public sc_core::sc_module
{
	// FSM states
	enum vci_gcd_master_fsm_state_e {
		RANDOM,
		CMD_OPA,
		RSP_OPA,
		CMD_OPB,
		RSP_OPB,
		CMD_START,
		RSP_START,
		CMD_STATUS,
                RSP_STATUS,
		CMD_RESULT,
                RSP_RESULT,
		DISPLAY,
	};

	// Registers
	sc_core::sc_signal<int> 						r_fsm;
    sc_core::sc_signal<typename vci_param::data_t> 	r_opa;
    sc_core::sc_signal<typename vci_param::data_t> 	r_opb;
    sc_core::sc_signal<typename vci_param::data_t> 	r_res;
    sc_core::sc_signal<int>   						r_cycle;
    sc_core::sc_signal<int>   						r_iter;

	// Constants
	const typename vci_param::srcid_t				m_srcid;
	const typename vci_param::addr_t				m_base;

protected:
	SC_HAS_PROCESS(VciGcdMaster);

public:
	// ports
        sc_in<bool> 								p_resetn;
        sc_in<bool> 								p_clk;
        soclib::caba::VciInitiator<vci_param>		p_vci;

	// constructor & destructor
        VciGcdMaster(	sc_core::sc_module_name insname,
			const soclib::common::IntTab &index,
			const soclib::common::MappingTable &mt,
			const int seed,
			const typename vci_param::addr_t base);
    	~VciGcdMaster();

private:
	// member functions
    	void transition();
    	void genMoore();

}; // end class VciGcdMaster

}}

#endif 

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4

