/**********************************************************************
 * File : tp6_top_advanced.cpp
 * Date : 01/01/2014
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

#include "vci_signals.h"
#include "vci_param.h"
#include "mapping_table.h"
#include "vci_vgmn.h"
#include "vci_xcache_wrapper_advanced.h"
#include "mips32.h"
#include "vci_multi_tty.h"
#include "vci_timer.h"
#include "vci_icu.h"
#include "vci_gcd_coprocessor.h"
#include "vci_dma.h"
#include "vci_block_device.h"
#include "vci_framebuffer.h"
#include "vci_simple_ram.h"

#define SEG_RESET_BASE  0xBFC00000
#define SEG_RESET_SIZE  0x00001000

#define SEG_KERNEL_BASE 0x80000000
#define SEG_KERNEL_SIZE 0x00004000

#define SEG_KDATA_BASE  0x81000000
#define SEG_KDATA_SIZE  0x00004000

#define SEG_KUNC_BASE   0x82000000
#define SEG_KUNC_SIZE   0x00001000

#define SEG_CODE_BASE   0x00400000
#define SEG_CODE_SIZE   0x00004000

#define SEG_DATA_BASE   0x01000000
#define SEG_DATA_SIZE   0x00004000

#define SEG_STACK_BASE  0x02000000
#define SEG_STACK_SIZE  0x01000000

#define SEG_TTY_BASE    0x90000000
#define SEG_TTY_SIZE    0x00000040

#define SEG_TIM_BASE    0x91000000
#define SEG_TIM_SIZE    0x00000040

#define SEG_IOC_BASE    0x92000000
#define SEG_IOC_SIZE    0x00000020

#define SEG_DMA_BASE    0x93000000
#define SEG_DMA_SIZE    0x00000014

#define SEG_GCD_BASE    0x95000000
#define SEG_GCD_SIZE    0x00000014

#define SEG_FBF_BASE    0x96000000
//#define SEG_FBF_SIZE    0x00004000
#define SEG_FBF_SIZE    0x400000 + 0x1000

#define SEG_ICU_BASE    0x9F000000
#define SEG_ICU_SIZE    0x00000014

// SRCID definition
#define SRCID_PROC      0
#define SRCID_IOC       1
#define SRCID_DMA       2

// TGTID definition
#define TGTID_ROM       0
#define TGTID_RAM       1
#define TGTID_TTY       2
#define TGTID_GCD       3
#define TGTID_TIM       4
#define TGTID_IOC       5
#define TGTID_DMA       6
#define TGTID_FBF       7
#define TGTID_ICU       8

// VCI fields width definition
#define cell_size       4
#define plen_size       8
#define addr_size       32
#define rerror_size     1
#define clen_size       1
#define rflag_size      1
#define srcid_size      12
#define pktid_size      1
#define trdid_size      4
#define wrplen_size     1

// Cache parameters definition
#define icache_ways     4
#define icache_sets     128
#define icache_words    8
#define dcache_ways     4
#define dcache_sets     128
#define dcache_words    8

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
                      pktid_size,
                      trdid_size,
                      wrplen_size> vci_param;

    ///////////////////////////////////////////////////////////////
    // command line arguments
    ///////////////////////////////////////////////////////////////
    int     ncycles             = 1000000000;       // simulated cycles
    char    sys_path[256]       = "soft/sys.bin";   // pathname for the system code
    char    app_path[256]       = "soft/app.bin";   // pathname for the application code
    char    ioc_filename[256]   = "images.raw";  // pathname for the ioc file
    size_t  fbf_size            = 128;              // number of lines = number of pixels
    bool    debug_ok            = false;            // debug activated
    int     from_cycle          = 0;                // debug start cycle
    bool    stats_ok            = false;            // statistics activated
    int     stats_period        = 100000;           // statistics periodicity
    int     noc_latency         = 20;               // intrinsic NOC latency
    int     wbuf_nlines         = 8;                 // number of lines in the write_buffer
    int     wbuf_nwords         = 4;                // number of words in a line of the write_buffer

    std::cout << std::endl << "********************************************************" << std::endl;
    std::cout << std::endl << "******        tp6_soclib                          ******" << std::endl;
    std::cout << std::endl << "********************************************************" << std::endl;

    if (argc > 1)
    {
        for( int n=1 ; n<argc ; n=n+2 )
        {
            if( (strcmp(argv[n],"-NCYCLES") == 0) && (n+1<argc) )
            {
                ncycles = atoi(argv[n+1]);
            }
            else if( (strcmp(argv[n],"-DEBUG") == 0) && (n+1<argc) )
            {
                debug_ok = true;
                from_cycle = atoi(argv[n+1]);
            }
            else if( (strcmp(argv[n],"-STATS") == 0) && (n+1<argc) )
            {
                stats_ok = true;
                stats_period = atoi(argv[n+1]);
            }
            else if( (strcmp(argv[n],"-SYS") == 0) && (n+1<argc) )
            {
                strcpy(sys_path, argv[n+1]) ;
            }
            else if( (strcmp(argv[n],"-APP") == 0) && (n+1<argc) )
            {
                strcpy(app_path, argv[n+1]) ;
            }
            else if( (strcmp(argv[n],"-IOCFILE") == 0) && (n+1<argc) )
            {
                strcpy(ioc_filename, argv[n+1]) ;
            }
            else if( (strcmp(argv[n],"-FBFSIZE") == 0) && (n+1<argc) )
            {
                fbf_size = atoi(argv[n+1]) ;
            }
            else if( (strcmp(argv[n],"-LATENCY") == 0) && (n+1<argc) )
            {
                noc_latency = atoi(argv[n+1]);
            }
            else if( (strcmp(argv[n],"-WBUFL") == 0) && (n+1<argc) )
            {
                wbuf_nlines = atoi(argv[n+1]);
            }
            else if( (strcmp(argv[n],"-WBUFW") == 0) && (n+1<argc) )
            {
                wbuf_nwords = atoi(argv[n+1]);
            }
            else
            {
                std::cout << "   Arguments on the command line are (key,value) couples." << std::endl;
                std::cout << "   The order is not important." << std::endl;
                std::cout << "   Accepted arguments are :" << std::endl << std::endl;
                std::cout << "   -NCYCLES number_of_simulated_cycles" << std::endl;
                std::cout << "   -IOCFILE file_name" << std::endl;
                std::cout << "   -FBFSIZE number_of_pixels" << std::endl;
                std::cout << "   -SYS sys_elf_pathname" << std::endl;
                std::cout << "   -APP app_elf_pathname" << std::endl;
                std::cout << "   -DEBUG debug_start_cycle" << std::endl;
                std::cout << "   -STATS statistics_period" << std::endl;
                std::cout << "   -LATENCY intrinsic_noc_latency" << std::endl;
                std::cout << "   -WBUFL number_of_lines_in_writebuffer" << std::endl;
                std::cout << "   -WBUFW number_of_words_in_a_line_of_writebuffer" << std::endl;
                exit(0);
            }
        }
    }
    std::cout << std::endl;
    std::cout << "    ncycles      = " << ncycles << std::endl;
    std::cout << "    sys_pathname = " << sys_path << std::endl;
    std::cout << "    app_pathname = " << app_path << std::endl;
    std::cout << "    ioc_pathname = " << ioc_filename << std::endl;
    std::cout << "    icache_sets  = " << icache_sets << std::endl;
    std::cout << "    icache_words = " << icache_words << std::endl;
    std::cout << "    icache_ways  = " << icache_ways << std::endl;
    std::cout << "    dcache_sets  = " << dcache_sets << std::endl;
    std::cout << "    dcache_words = " << dcache_words << std::endl;
    std::cout << "    dcache_ways  = " << dcache_ways << std::endl;
    std::cout << "    noc_latency  = " << noc_latency << std::endl;
    std::cout << "    wbuf_nlines  = " << wbuf_nlines << std::endl;
    std::cout << "    wbuf_nwords  = " << wbuf_nwords << std::endl;

    //////////////////////////////////////////////////////////////////////////
    // Mapping Table
    //////////////////////////////////////////////////////////////////////////
    MappingTable maptab(32, IntTab(8), IntTab(2), 0xFF000000);

    maptab.add(Segment("seg_reset" , SEG_RESET_BASE , SEG_RESET_SIZE , IntTab(TGTID_ROM), true));

    maptab.add(Segment("seg_kernel", SEG_KERNEL_BASE, SEG_KERNEL_SIZE, IntTab(TGTID_RAM), true));
    maptab.add(Segment("seg_kdata" , SEG_KDATA_BASE , SEG_KDATA_SIZE , IntTab(TGTID_RAM), true));
    maptab.add(Segment("seg_kunc"  , SEG_KUNC_BASE  , SEG_KUNC_SIZE  , IntTab(TGTID_RAM), false));
    maptab.add(Segment("seg_code"  , SEG_CODE_BASE  , SEG_CODE_SIZE  , IntTab(TGTID_RAM), true));
    maptab.add(Segment("seg_data"  , SEG_DATA_BASE  , SEG_DATA_SIZE  , IntTab(TGTID_RAM), true));
    maptab.add(Segment("seg_stack" , SEG_STACK_BASE , SEG_STACK_SIZE , IntTab(TGTID_RAM), true));

    maptab.add(Segment("seg_tty"   , SEG_TTY_BASE   , SEG_TTY_SIZE   , IntTab(TGTID_TTY), false));
    maptab.add(Segment("seg_timer" , SEG_TIM_BASE   , SEG_TIM_SIZE   , IntTab(TGTID_TIM), false));
    maptab.add(Segment("seg_icu"   , SEG_ICU_BASE   , SEG_ICU_SIZE   , IntTab(TGTID_ICU), false));
    maptab.add(Segment("seg_dma"   , SEG_DMA_BASE   , SEG_DMA_SIZE   , IntTab(TGTID_DMA), false));
    maptab.add(Segment("seg_fbf"   , SEG_FBF_BASE   , SEG_FBF_SIZE   , IntTab(TGTID_FBF), false));
    maptab.add(Segment("seg_ioc"   , SEG_IOC_BASE   , SEG_IOC_SIZE   , IntTab(TGTID_IOC), false));
    maptab.add(Segment("seg_gcd"   , SEG_GCD_BASE   , SEG_GCD_SIZE   , IntTab(TGTID_GCD), false));

    std::cout << std::endl << maptab << std::endl;

    //////////////////////////////////////////////////////////////////////////
    // Signals
    //////////////////////////////////////////////////////////////////////////
    sc_clock        signal_clk("signal_clk", sc_time( 1, SC_NS ), 0.5 );
    sc_signal<bool> signal_resetn("signal_resetn");

    VciSignals<vci_param>   signal_vci_init_proc("signal_vci_init_proc");
    VciSignals<vci_param>   signal_vci_init_dma("signal_vci_init_dma");
    VciSignals<vci_param>   signal_vci_init_ioc("signal_vci_init_ioc");

    VciSignals<vci_param>   signal_vci_tgt_rom("signal_vci_tgt_rom");
    VciSignals<vci_param>   signal_vci_tgt_ram("signal_vci_tgt_ram");
    VciSignals<vci_param>   signal_vci_tgt_tty("signal_vci_tgt_tty");
    VciSignals<vci_param>   signal_vci_tgt_gcd("signal_vci_tgt_gcd");
    VciSignals<vci_param>   signal_vci_tgt_tim("signal_vci_tgt_tim");
    VciSignals<vci_param>   signal_vci_tgt_icu("signal_vci_tgt_icu");
    VciSignals<vci_param>   signal_vci_tgt_fbf("signal_vci_tgt_fbf");
    VciSignals<vci_param>   signal_vci_tgt_ioc("signal_vci_tgt_ioc");
    VciSignals<vci_param>   signal_vci_tgt_dma("signal_vci_tgt_dma");

    sc_signal<bool> signal_false("signal_false");

    sc_signal<bool> signal_irq_proc("signal_irq_proc");
    sc_signal<bool> signal_irq_tim("signal_irq_tim");
    sc_signal<bool> signal_irq_tty("signal_irq_tty");
    sc_signal<bool> signal_irq_dma("signal_irq_dma");
    sc_signal<bool> signal_irq_ioc("signal_irq_ioc");

    ///////////////////////////////////////////////////////////////
    // VCI Components : 3 initiators / 9 targets
    // The IOC & DMA components are both initiator & target.
    ///////////////////////////////////////////////////////////////
    // The ICU controls 4 input IRQs :
    // - IRQ[0] : timer
    // - IRQ[1] : tty
    // - IRQ[2] : ioc
    // - IRQ[3] : dma
    //////////////////////////////////////////////////////////////

    Loader loader(sys_path, app_path);

    VciXcacheWrapperAdvanced<vci_param, Mips32ElIss >* proc;
    proc = new VciXcacheWrapperAdvanced<vci_param,Mips32ElIss>("proc", 0,maptab,IntTab(SRCID_PROC),
                                                    icache_ways, icache_sets, icache_words,
                                                    dcache_ways, dcache_sets, dcache_words,
                                                    wbuf_nlines, wbuf_nwords);

    VciSimpleRam<vci_param>* rom;
    rom = new VciSimpleRam<vci_param>("rom", IntTab(TGTID_ROM), maptab, loader);

    VciSimpleRam<vci_param>* ram;
    ram = new VciSimpleRam<vci_param>("ram", IntTab(TGTID_RAM), maptab, loader);

    VciMultiTty<vci_param>* tty;
    tty = new VciMultiTty<vci_param>("tty", IntTab(TGTID_TTY), maptab, "tty", NULL);

    VciGcdCoprocessor<vci_param>* gcd;
    gcd = new VciGcdCoprocessor<vci_param>("gcd", IntTab(TGTID_GCD), maptab);

    VciTimer<vci_param>* timer;
    timer = new VciTimer<vci_param>("timer", IntTab(TGTID_TIM), maptab, 1);

    VciIcu<vci_param>* icu;
    icu = new VciIcu<vci_param>("icu", IntTab(TGTID_ICU), maptab, 4);

    VciDma<vci_param>* dma;
    dma = new VciDma<vci_param>("dma", maptab, IntTab(SRCID_DMA), IntTab(TGTID_DMA), 128);

    VciFrameBuffer<vci_param>* fbf;
    fbf = new VciFrameBuffer<vci_param>("fbf", IntTab(TGTID_FBF), maptab, fbf_size, fbf_size);

    VciBlockDevice<vci_param>* ioc;
    ioc = new VciBlockDevice<vci_param>("ioc", maptab, IntTab(SRCID_IOC), 
                                         IntTab(TGTID_IOC), ioc_filename, 512, 200000);

    VciVgmn<vci_param>* noc;
    noc = new VciVgmn<vci_param>("noc", maptab, 3, 9, noc_latency, 16);

    //////////////////////////////////////////////////////////////////////////
    // Net-List
    //////////////////////////////////////////////////////////////////////////
    proc->p_clk                     (signal_clk);
    proc->p_resetn                  (signal_resetn);
    proc->p_vci                     (signal_vci_init_proc);
    proc->p_irq[0]                  (signal_irq_proc);
    proc->p_irq[1]                  (signal_false);
    proc->p_irq[2]                  (signal_false);
    proc->p_irq[3]                  (signal_false);
    proc->p_irq[4]                  (signal_false);
    proc->p_irq[5]                  (signal_false);

    rom->p_clk                      (signal_clk);
    rom->p_resetn                   (signal_resetn);
    rom->p_vci                      (signal_vci_tgt_rom);

    ram->p_clk                      (signal_clk);
    ram->p_resetn                   (signal_resetn);
    ram->p_vci                      (signal_vci_tgt_ram);

    gcd->p_clk                      (signal_clk);
    gcd->p_resetn                   (signal_resetn);
    gcd->p_vci                      (signal_vci_tgt_gcd);

    tty->p_clk                      (signal_clk);
    tty->p_resetn                   (signal_resetn);
    tty->p_vci                      (signal_vci_tgt_tty);
    tty->p_irq[0]                   (signal_irq_tty);

    timer->p_clk                    (signal_clk);
    timer->p_resetn                 (signal_resetn);
    timer->p_vci                    (signal_vci_tgt_tim);
    timer->p_irq[0]                 (signal_irq_tim);

    icu->p_clk                      (signal_clk);
    icu->p_resetn                   (signal_resetn);
    icu->p_vci                      (signal_vci_tgt_icu);
    icu->p_irq                      (signal_irq_proc);
    icu->p_irq_in[0]                (signal_irq_tim);
    icu->p_irq_in[1]                (signal_irq_tty);
    icu->p_irq_in[2]                (signal_irq_ioc);
    icu->p_irq_in[3]                (signal_irq_dma);

    fbf->p_clk                      (signal_clk);
    fbf->p_resetn                   (signal_resetn);
    fbf->p_vci                      (signal_vci_tgt_fbf);

    ioc->p_clk                      (signal_clk);
    ioc->p_resetn                   (signal_resetn);
    ioc->p_vci_initiator            (signal_vci_init_ioc);
    ioc->p_vci_target               (signal_vci_tgt_ioc);
    ioc->p_irq                      (signal_irq_ioc);

    dma->p_clk                      (signal_clk);
    dma->p_resetn                   (signal_resetn);
    dma->p_vci_initiator            (signal_vci_init_dma);
    dma->p_vci_target               (signal_vci_tgt_dma);
    dma->p_irq                      (signal_irq_dma);

    noc->p_clk                      (signal_clk);
    noc->p_resetn                   (signal_resetn);
    noc->p_to_initiator[SRCID_PROC] (signal_vci_init_proc);
    noc->p_to_initiator[SRCID_DMA]  (signal_vci_init_dma);
    noc->p_to_initiator[SRCID_IOC]  (signal_vci_init_ioc);
    noc->p_to_target[TGTID_ROM]     (signal_vci_tgt_rom);
    noc->p_to_target[TGTID_RAM]     (signal_vci_tgt_ram);
    noc->p_to_target[TGTID_TTY]     (signal_vci_tgt_tty);
    noc->p_to_target[TGTID_GCD]     (signal_vci_tgt_gcd);
    noc->p_to_target[TGTID_TIM]     (signal_vci_tgt_tim);
    noc->p_to_target[TGTID_ICU]     (signal_vci_tgt_icu);
    noc->p_to_target[TGTID_DMA]     (signal_vci_tgt_dma);
    noc->p_to_target[TGTID_FBF]     (signal_vci_tgt_fbf);
    noc->p_to_target[TGTID_IOC]     (signal_vci_tgt_ioc);

    //////////////////////////////////////////////////////////////////////////
    // simulation
    //////////////////////////////////////////////////////////////////////////

    signal_false = false;
    signal_resetn = false;
    sc_start( sc_time( 1, SC_NS ) ) ;

    signal_resetn = true;
    for ( int n=1 ; n<ncycles ; n++ )
    {
        if( debug_ok && (n > from_cycle) )
        {
            std::cout << "***************** cycle " << std::dec << n << std::endl;
            proc->print_trace(1);
            rom->print_trace();
            ram->print_trace();
        }
        if( stats_ok && ( n%stats_period == 1 ) )
        {
            proc->print_stats();
        }

        sc_start( sc_time( 1 , SC_NS ) ) ;
    }

    sc_stop();

    delete proc;
    delete rom;
    delete ram;
    delete tty;
    delete timer;
    delete icu;
    delete dma;
    delete gcd;
    delete fbf;
    delete ioc;
    delete noc;

    return(0);

} // end _main

int sc_main (int argc, char *argv[])
{
    try {
        return _main(argc, argv);
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    } catch (...) {
        std::cout << "Unknown exception occured" << std::endl;
        throw;
    }
    return 1;
}

// Local Variables:
// tab-width: 4;
// c-basic-offset: 4;
// c-file-offsets:((innamespace . 0)(inline-open . 0));
// indent-tabs-mode: nil;
// End:
//
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4

