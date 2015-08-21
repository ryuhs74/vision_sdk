/*
 * SCI_CONFIG_TDA3x_H
 *
 * Statistic Collector Instrumentation Library
 * - Statistic Collector module specific definitions
 * - Device specific configurations
 *
 * Copyright (C) 20131 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

#ifndef SCI_CONFIG_TDA3x_H

#define SCI_CONFIG_TDA3x_H
#define SC_VER 116
#include "sc_reg.h"
#define SC_INTERFACE_VERSION 1.0

#define SC_LAT1_BASE 0x45002000
#define SC_LAT1_NUM_CNTRS 4
#define SC_LAT1_NUM_PROBES 6

#define SC_LAT2_BASE 0x45004000
#define SC_LAT2_NUM_CNTRS 4
#define SC_LAT2_NUM_PROBES 8

#define SC_LAT3_BASE 0x45003000
#define SC_LAT3_NUM_CNTRS 4
#define SC_LAT3_NUM_PROBES 7

#define SC_LAT4_BASE 0x45005000
#define SC_LAT4_NUM_CNTRS 4
#define SC_LAT4_NUM_PROBES 8


/* Statistic Collector counter element definition*/
struct sc_cnt_elements {
    int num_filters;
    uint32_t cnt_offset;
};

/* lat1 Statistic Collector counter register configurations */
struct sc_lat_1_counters {
        struct sc_lat_cnt_filter1 cnt[4];
    };


/* lat1 Statistic Collector register configuration */
struct sc_lat_1_reg_map {
sc_lat_regs regs;
struct sc_lat_1_counters counters;
};

/* lat2 Statistic Collector counter register configurations */
struct sc_lat_2_counters {
        struct sc_lat_cnt_filter1 cnt[4];
    };


/* lat2 Statistic Collector register configuration */
struct sc_lat_2_reg_map {
sc_lat_regs regs;
struct sc_lat_2_counters counters;
};

/* lat3 Statistic Collector counter register configurations */
struct sc_lat_3_counters {
        struct sc_lat_cnt_filter1 cnt[4];
    };


/* lat3 Statistic Collector register configuration */
struct sc_lat_3_reg_map {
sc_lat_regs regs;
struct sc_lat_3_counters counters;
};

/* lat4 Statistic Collector counter register configurations */
struct sc_lat_4_counters {
        struct sc_lat_cnt_filter1 cnt[4];
    };


/* lat4 Statistic Collector register configuration */
struct sc_lat_4_reg_map {
sc_lat_regs regs;
struct sc_lat_4_counters counters;
};

/* Statistic Collector counter maps */

struct sc_cnt_elements sc_lat_1_cnt_map[] = {
    1, (uint32_t)(&((struct sc_lat_1_reg_map *)0)->counters.cnt[0]),
    1, (uint32_t)(&((struct sc_lat_1_reg_map *)0)->counters.cnt[1]),
    1, (uint32_t)(&((struct sc_lat_1_reg_map *)0)->counters.cnt[2]),
    1, (uint32_t)(&((struct sc_lat_1_reg_map *)0)->counters.cnt[3])
};

struct sc_cnt_elements sc_lat_2_cnt_map[] = {
    1, (uint32_t)(&((struct sc_lat_2_reg_map *)0)->counters.cnt[0]),
    1, (uint32_t)(&((struct sc_lat_2_reg_map *)0)->counters.cnt[1]),
    1, (uint32_t)(&((struct sc_lat_2_reg_map *)0)->counters.cnt[2]),
    1, (uint32_t)(&((struct sc_lat_2_reg_map *)0)->counters.cnt[3])
};

struct sc_cnt_elements sc_lat_3_cnt_map[] = {
    1, (uint32_t)(&((struct sc_lat_3_reg_map *)0)->counters.cnt[0]),
    1, (uint32_t)(&((struct sc_lat_3_reg_map *)0)->counters.cnt[1]),
    1, (uint32_t)(&((struct sc_lat_3_reg_map *)0)->counters.cnt[2]),
    1, (uint32_t)(&((struct sc_lat_3_reg_map *)0)->counters.cnt[3])
};

struct sc_cnt_elements sc_lat_4_cnt_map[] = {
    1, (uint32_t)(&((struct sc_lat_4_reg_map *)0)->counters.cnt[0]),
    1, (uint32_t)(&((struct sc_lat_4_reg_map *)0)->counters.cnt[1]),
    1, (uint32_t)(&((struct sc_lat_4_reg_map *)0)->counters.cnt[2]),
    1, (uint32_t)(&((struct sc_lat_4_reg_map *)0)->counters.cnt[3])
};

/* Statistic Collector probe element definition */
struct sc_probe_element {
    uint32_t probe_id;
    int32_t req_port_num;
    int32_t rsp_port_num;
};

/* Statistic Collector probe element maps */
struct sc_probe_element sc_lat_1_probe_map[] = {
{SCI_EMIF_SYS, 0, 1},
{SCI_IPU, 2, 3},
{SCI_DSP1_MDMA, 4, 5},
{SCI_DSP2_MDMA, 6, 7},
{SCI_GMAC_SW, 8, 9},
{SCI_ISS_NRT1, 10, 11}
};

struct sc_probe_element sc_lat_2_probe_map[] = {
{SCI_DSS, 0, 1},
{SCI_MMU, 2, 3},
{SCI_EDMA_TC0_RD, 4, 5},
{SCI_EDMA_TC0_WR, 6, 7},
{SCI_DSP2_CFG, 8, 9},
{SCI_DSP2_EDMA, 10, 11}
};

struct sc_probe_element sc_lat_3_probe_map[] = {
{SCI_VIP_P1, 0, 1},
{SCI_VIP_P2, 2, 3},
{SCI_ISS_RT, 4, 5},
{SCI_ISS_NRT2, 6, 7},
{SCI_OCMC_RAM, 8, 9},
{SCI_DSP1_CFG, 10, 11},
{SCI_DSP1_EDMA, 12, 13}
};

struct sc_probe_element sc_lat_4_probe_map[] = {
{SCI_EVE_P1, 0, 1},
{SCI_EVE_P2, 2, 3},
{SCI_GPMC, 6, 7},
{SCI_MCASP, 8, 9},
{SCI_EDMA_TC1_RD, 10, 11},
{SCI_EDMA_TC1_WR, 12, 13}
};


enum sc_probe_valid {
    SCI_INVALID_PROBE,
     SCI_VALID_PROBE
};


/* Statistic Collector element definition */
enum sc_module_type {SDRAM, MSTR};
struct sc_element_map {
        enum sc_module_type mod_type;
        uint32_t base_addr;
        int mod_size;                   /* In bytes */
        int num_counters;
        struct sc_cnt_elements * cnt_map;
        int num_probes;
        struct sc_probe_element * probe_map;
        enum sc_probe_valid * sc_no_filter_valid_probe_map; /* First probe restricted counter*/
        uint32_t cnt_overflow_load; /*Counter saturation value */
        uint32_t cnt_overflow_lat; /*Latency counter saturation value */
};

/* Statistic Collector element maps */
struct sc_element_map sc_lat_1_map = {
        MSTR,
        SC_LAT1_BASE,
        sizeof(struct sc_lat_1_reg_map),
        SC_LAT1_NUM_CNTRS,
        sc_lat_1_cnt_map,
        SC_LAT1_NUM_PROBES,
        sc_lat_1_probe_map,
        NULL,
        SC_LAT_LOAD_CNT_OVERFLOW,
        SC_LAT_LATENCY_CNT_OVERFLOW
};

struct sc_element_map sc_lat_2_map = {
        MSTR,
        SC_LAT2_BASE,
        sizeof(struct sc_lat_2_reg_map),
        SC_LAT2_NUM_CNTRS,
        sc_lat_2_cnt_map,
        SC_LAT2_NUM_PROBES,
        sc_lat_2_probe_map,
        NULL,
        SC_LAT_LOAD_CNT_OVERFLOW,
        SC_LAT_LATENCY_CNT_OVERFLOW
};

struct sc_element_map sc_lat_3_map = {
        MSTR,
        SC_LAT3_BASE,
        sizeof(struct sc_lat_3_reg_map),
        SC_LAT3_NUM_CNTRS,
        sc_lat_3_cnt_map,
        SC_LAT3_NUM_PROBES,
        sc_lat_3_probe_map,
        NULL,
        SC_LAT_LOAD_CNT_OVERFLOW,
        SC_LAT_LATENCY_CNT_OVERFLOW
};

struct sc_element_map sc_lat_4_map = {
        MSTR,
        SC_LAT4_BASE,
        sizeof(struct sc_lat_4_reg_map),
        SC_LAT4_NUM_CNTRS,
        sc_lat_4_cnt_map,
        SC_LAT4_NUM_PROBES,
        sc_lat_4_probe_map,
        NULL,
        SC_LAT_LOAD_CNT_OVERFLOW,
        SC_LAT_LATENCY_CNT_OVERFLOW
};


struct sc_element_map * sc_map[] = {
&sc_lat_1_map,
&sc_lat_2_map,
&sc_lat_3_map,
&sc_lat_4_map
 };

 /* Statistic Collector names are in sc_map order */
const char * sci_unit_name_table[] = {
    "LAT0",
    "LAT1",
    "LAT2",
    "LAT3"
};


/* Statistic Collector probe name tables */

const char * sci_mstr_probe_name_table[] = {
         "EMIF_SYS",
         "IPU",
         "DSP1_MDMA",
         "DSP2_MDMA",
         "GMAC_SW",
         "ISS_NRT1",
         "DSS",
         "MMU",
         "EDMA_TC0_RD",
         "EDMA_TC0_WR",
         "DSP2_CFG",
         "DSP2_EDMA",
         "VIP_P1",
         "VIP_P2",
         "ISS_RT",
         "ISS_NRT2",
         "OCMC_RAM",
         "DSP1_CFG",
         "DSP1_EDMA",
         "EVE_P1",
         "EVE_P2",
         "GPMC",
         "MCASP",
         "EDMA_TC1_RD",
         "EDMA_TC1_WR"
};

const char * sci_sdram_probe_name_table[]; //not Implemented

/* Statistic Collector master address table */
enum sci_master_addr sci_master_addr_table[] = {
        SCI_MSTID_DSS,
        SCI_MSTID_VIP_P1,
        SCI_MSTID_VIP_P2,
        SCI_MSTID_ISS_RT,
        SCI_MSTID_ISS_NRT1,
        SCI_MSTID_ISS_NRT2,
        SCI_MSTID_IPU,
        SCI_MSTID_EVE_P1,
        SCI_MSTID_EVE_P2,
        SCI_MSTID_DSP1_MDMA,
        SCI_MSTID_DSP1_EDMA,
        SCI_MSTID_DSP1_CFG,
        SCI_MSTID_DSP2_MDMA,
        SCI_MSTID_DSP2_EDMA,
        SCI_MSTID_DSP2_CFG,
        SCI_MSTID_MMU,
        SCI_MSTID_EDMA_TC0_WR,
        SCI_MSTID_EDMA_TC0_RD,
        SCI_MSTID_EDMA_TC1_WR,
        SCI_MSTID_EDMA_TC1_RD,
        SCI_MSTID_GMAC_SW,
        SCI_MSTID_IEEE1500_2_OCP,
        SCI_MSTID_CS_DAP,
        SCI_MSTID_EVE1_SW_INSTR_ARP32,
        SCI_MSTID_EVE1_HW_INSTR_SMSET
};

/* Statistic Collector master name table.
        In sci_master_addr_table order.
       */
const char * sci_master_name_table[] = {
        "DSS",
        "VIP_P1",
        "VIP_P2",
        "ISS_RT",
        "ISS_NRT1",
        "ISS_NRT2",
        "IPU",
        "EVE_P1",
        "EVE_P2",
        "DSP1_MDMA",
        "DSP1_EDMA",
        "DSP1_CFG",
        "DSP2_MDMA",
        "DSP2_EDMA",
        "DSP2_CFG",
        "MMU",
        "EDMA_TC0_WR",
        "EDMA_TC0_RD",
        "EDMA_TC1_WR",
        "EDMA_TC1_RD",
        "GMAC_SW",
        "IEEE1500_2_OCP",
        "CS_DAP",
        "EVE1_SW_INSTR_ARP32",
        "EVE1_HW_INSTR_SMSET"
};

/* Statistic Collector slave address table */
enum sci_slave_addr sci_slave_addr_table[] = {
        SCI_SLVID_CRC,
        SCI_SLVID_DEBUGSS_CT_TBR,
        SCI_SLVID_DSP1_SDMA,
        SCI_SLVID_DSP2_SDMA,
        SCI_SLVID_DSS,
        SCI_SLVID_EMIF_SYS,
        SCI_SLVID_EVE,
        SCI_SLVID_GPMC,
        SCI_SLVID_IPU,
        SCI_SLVID_L3_INSTR,
        SCI_SLVID_L4_CFG,
        SCI_SLVID_L4_PER1_P1,
        SCI_SLVID_L4_PER1_P2,
        SCI_SLVID_L4_PER1_P3,
        SCI_SLVID_L4_PER2_P1,
        SCI_SLVID_L4_PER2_P2,
        SCI_SLVID_L4_PER2_P3,
        SCI_SLVID_L4_PER3_P1,
        SCI_SLVID_L4_PER3_P2,
        SCI_SLVID_L4_PER3_P3,
        SCI_SLVID_L4_WKUP,
        SCI_SLVID_MCASP,
        SCI_SLVID_MMU,
        SCI_SLVID_OCMC_RAM,
        SCI_SLVID_OCMC_ROM,
        SCI_SLVID_QSPI,
        SCI_SLVID_TESOC,
        SCI_SLVID_EDMA_TPCC,
        SCI_SLVID_EDMA_TC0,
        SCI_SLVID_EDMA_TC1,
        SCI_SLVID_HOST_CLK1_1,
        SCI_SLVID_HOST_CLK1_2,
        SCI_SLVID_HOST_CLK2_1
};

/* Statistic Collector slave name table.
       In sci_slave_addr_table order.
       */
const char * sci_slave_name_table[] = {
        "CRC",
        "DEBUGSS_CT_TBR",
        "DSP1_SDMA",
        "DSP2_SDMA",
        "DSS",
        "EMIF_SYS",
        "EVE",
        "GPMC",
        "IPU",
        "L3_INSTR",
        "L4_CFG",
        "L4_PER1_P1",
        "L4_PER1_P2",
        "L4_PER1_P3",
        "L4_PER2_P1",
        "L4_PER2_P2",
        "L4_PER2_P3",
        "L4_PER3_P1",
        "L4_PER3_P2",
        "L4_PER3_P3",
        "L4_WKUP",
        "MCASP",
        "MMU",
        "OCMC_RAM",
        "OCMC_ROM",
        "QSPI",
        "TESOC",
        "EDMA_TPCC",
        "EDMA_TC0",
        "EDMA_TC1",
        "HOST_CLK1_1",
        "HOST_CLK1_2",
        "HOST_CLK2_1"
};
#endif
