/*
 * Copyright (c) 2013, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== system_rsc_table_dsp.h ========
 *
 *  Define the resource table entries for all DSP cores. This will be
 *  incorporated into corresponding base images, and used by the remoteproc
 *  on the host-side to allocated/reserve resources.
 *
 */

#ifndef _RSC_TABLE_DSP_H_
#define _RSC_TABLE_DSP_H_

#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <ti/ipc/remoteproc/rsc_types.h>
#include <include/link_api/system_vring_config.h>

/* Number of entries in resource table */
#define RSC_NUM_ENTRIES         24

/* DSP Memory Map */
#define DSP_SYSTEM_DMA_BASE_ADDR  0x43300000
#define DSP_SYSTEM_DMA_SIZE       0x400000

#define DSP_OCMC1_BASE          0x40300000
#define DSP_OCMC2_BASE          0x40400000
#define DSP_OCMC3_BASE          0x40500000

#define DSP_L2SRAM_BASE         0x00800000
#define DSP_L2SRAM_SIZE         0x00020000

#define L4_DRA7XX_BASE          0x4A000000

#define L4_PERIPHERAL_L4CFG     (L4_DRA7XX_BASE)
#define DSP_PERIPHERAL_L4CFG    0x4A000000

#define L4_PERIPHERAL_L4PER1    0x48000000
#define DSP_PERIPHERAL_L4PER1   0x48000000

#define L4_PERIPHERAL_L4PER2    0x48400000
#define DSP_PERIPHERAL_L4PER2   0x48400000

#define L4_PERIPHERAL_L4PER3    0x48800000
#define DSP_PERIPHERAL_L4PER3   0x48800000

#define L4_PERIPHERAL_L4EMU     0x54000000
#define DSP_PERIPHERAL_L4EMU    0x54000000

#define L3_PERIPHERAL_DMM       0x4E000000
#define DSP_PERIPHERAL_DMM      0x4E000000

#define L3_PERIPHERAL_ISS       0x52000000
#define DSP_PERIPHERAL_ISS      0x52000000

#define L3_TILER_MODE_0_1       0x60000000
#define DSP_TILER_MODE_0_1      0x60000000

#define L3_TILER_MODE_2         0x70000000
#define DSP_TILER_MODE_2        0x70000000

#define L3_TILER_MODE_3         0x78000000
#define DSP_TILER_MODE_3        0x78000000

#define DSP_EVE_CONFIG_SPACE       0x42000000
#define DSP_EVE_CONFIG_SPACE_SIZE  0x400000

#ifdef BUILD_DSP_1
#define DSP_MEM_TEXT            XDC_CFG_DSP1_CODE
#define DSP_MEM_DATA            XDC_CFG_DSP1_DATA
#endif

#ifdef BUILD_DSP_2
#define DSP_MEM_TEXT            XDC_CFG_DSP2_CODE
#define DSP_MEM_DATA            XDC_CFG_DSP2_DATA
#endif

#define SYSTEM_COMMON_SHM_VIRT  XDC_CFG_SYSTEM_COMMON_SHM_VIRT
#define SYSTEM_COMMON_SHM       XDC_CFG_SYSTEM_COMMON_SHM_VIRT

#define DSP_SR1_VIRT            XDC_CFG_SR1_VIRT
#define DSP_SR1                 XDC_CFG_SR1_VIRT

#define DSP_MEM_IPC_DATA        XDC_CFG_IPC_DATA

#define DSP_MEM_IPC_VRING_SIZE  SZ_1M

#ifdef BUILD_DSP_1
#define DSP_MEM_TEXT_SIZE       XDC_CFG_DSP1_CODE_SIZE
#define DSP_MEM_DATA_SIZE       XDC_CFG_DSP1_DATA_SIZE
#endif

#ifdef BUILD_DSP_2
#define DSP_MEM_TEXT_SIZE       XDC_CFG_DSP2_CODE_SIZE
#define DSP_MEM_DATA_SIZE       XDC_CFG_DSP2_DATA_SIZE
#endif

#define SYSTEM_COMMON_SHM_SIZE  XDC_CFG_SYSTEM_COMMON_SHM_SIZE
#define DSP_SR1_SIZE            XDC_CFG_SR1_SIZE
#define DSP_OCMC_SIZE           XDC_CFG_OCMC_SIZE
#define DSP_MEM_IPC_DATA_SIZE   XDC_CFG_IPC_DATA_SIZE





/*
 * Sizes of the virtqueues (expressed in number of buffers supported,
 * and must be power of 2)
 */
#define DSP_RPMSG_VQ0_SIZE      256
#define DSP_RPMSG_VQ1_SIZE      256

/* flip up bits whose indices represent features we support */
#define RPMSG_DSP_C0_FEATURES         1

struct my_resource_table {
    struct resource_table base;

    UInt32 offset[RSC_NUM_ENTRIES];  /* Should match 'num' in actual definition */

    /* rpmsg vdev entry */
    struct fw_rsc_vdev rpmsg_vdev;
    struct fw_rsc_vdev_vring rpmsg_vring0;
    struct fw_rsc_vdev_vring rpmsg_vring1;

    /* text carveout entry */
    struct fw_rsc_carveout text_cout;

    /* data carveout entry */
    struct fw_rsc_carveout data_cout;

    /* ipcdata carveout entry */
    struct fw_rsc_carveout ipcdata_cout;

    /* trace entry */
    struct fw_rsc_trace trace;

    /* devmem entry */
    struct fw_rsc_devmem devmem0;

    /* devmem entry */
    struct fw_rsc_devmem devmem1;

    /* devmem entry */
    struct fw_rsc_devmem devmem2;

    /* devmem entry */
    struct fw_rsc_devmem devmem3;

    /* devmem entry */
    struct fw_rsc_devmem devmem4;

    /* devmem entry */
    struct fw_rsc_devmem devmem5;

    /* devmem entry */
    struct fw_rsc_devmem devmem6;

    /* devmem entry */
    struct fw_rsc_devmem devmem7;

    /* devmem entry */
    struct fw_rsc_devmem devmem8;

    /* devmem entry */
    struct fw_rsc_devmem devmem9;

    /* devmem entry */
    struct fw_rsc_devmem devmem10;

    /* devmem entry */
    struct fw_rsc_devmem devmem11;

    /* devmem entry */
    struct fw_rsc_devmem devmem12;

    /* devmem entry */
    struct fw_rsc_devmem devmem13;

    /* devmem entry */
    struct fw_rsc_devmem devmem14;

    /* devmem entry */
    struct fw_rsc_devmem devmem15;

    /* devmem entry */
    struct fw_rsc_devmem devmem16;

    /* devmem entry */
    struct fw_rsc_devmem devmem17;

    /* devmem entry */
    struct fw_rsc_devmem devmem18;

};

extern char ti_trace_SysMin_Module_State_0_outbuf__A;
#define TRACEBUFADDR (UInt32)&ti_trace_SysMin_Module_State_0_outbuf__A

#pragma DATA_SECTION(ti_ipc_remoteproc_ResourceTable, ".resource_table")
#pragma DATA_ALIGN(ti_ipc_remoteproc_ResourceTable, 4096)

struct my_resource_table ti_ipc_remoteproc_ResourceTable = {
    1,      /* we're the first version that implements this */
    RSC_NUM_ENTRIES,     /* number of entries in the table */
    0, 0,   /* reserved, must be zero */
    /* offsets to entries */
    {
        offsetof(struct my_resource_table, rpmsg_vdev),
        offsetof(struct my_resource_table, text_cout),
        offsetof(struct my_resource_table, data_cout),
        offsetof(struct my_resource_table, ipcdata_cout),
        offsetof(struct my_resource_table, trace),
        offsetof(struct my_resource_table, devmem0),
        offsetof(struct my_resource_table, devmem1),
        offsetof(struct my_resource_table, devmem2),
        offsetof(struct my_resource_table, devmem3),
        offsetof(struct my_resource_table, devmem4),
        offsetof(struct my_resource_table, devmem5),
        offsetof(struct my_resource_table, devmem6),
        offsetof(struct my_resource_table, devmem7),
        offsetof(struct my_resource_table, devmem8),
        offsetof(struct my_resource_table, devmem9),
        offsetof(struct my_resource_table, devmem10),
        offsetof(struct my_resource_table, devmem11),
        offsetof(struct my_resource_table, devmem12),
        offsetof(struct my_resource_table, devmem13),
        offsetof(struct my_resource_table, devmem14),
        offsetof(struct my_resource_table, devmem15),
        offsetof(struct my_resource_table, devmem16),
        offsetof(struct my_resource_table, devmem17),
        offsetof(struct my_resource_table, devmem18),
    },
    /* rpmsg vdev entry */
    {
        TYPE_VDEV, VIRTIO_ID_RPMSG, 0,
        RPMSG_DSP_C0_FEATURES, 0, 0, 0, 2, { 0, 0 },
        /* no config data */
    },
    /* the two vrings */
    { DSP_MEM_RPMSG_VRING0, 4096, DSP_RPMSG_VQ0_SIZE, 1, 0 },
    { DSP_MEM_RPMSG_VRING1, 4096, DSP_RPMSG_VQ1_SIZE, 2, 0 },

    {
        TYPE_CARVEOUT,
        DSP_MEM_TEXT, 0,
        DSP_MEM_TEXT_SIZE, 0, 0, "DSP_MEM_TEXT",
    },

    {
        TYPE_CARVEOUT,
        DSP_MEM_DATA, 0,
        DSP_MEM_DATA_SIZE, 0, 0, "DSP_MEM_DATA",
    },

    {
        TYPE_CARVEOUT,
        DSP_MEM_IPC_DATA, 0,
        DSP_MEM_IPC_DATA_SIZE, 0, 0, "DSP_MEM_IPC_DATA",
    },

    {
        TYPE_TRACE, TRACEBUFADDR, 0x8000, 0, "trace:dsp",
    },


    {
        TYPE_DEVMEM,
        DSP_MEM_IPC_VRING, DSP_PHYS_MEM_IPC_VRING,
        DSP_MEM_IPC_VRING_SIZE, 0, 0, "DSP_MEM_IPC_VRING",
    },


    {
        TYPE_DEVMEM,
        DSP_TILER_MODE_0_1, L3_TILER_MODE_0_1,
        SZ_256M, 0, 0, "DSP_TILER_MODE_0_1",
    },

    {
        TYPE_DEVMEM,
        DSP_TILER_MODE_2, L3_TILER_MODE_2,
        SZ_128M, 0, 0, "DSP_TILER_MODE_2",
    },

    {
        TYPE_DEVMEM,
        DSP_TILER_MODE_3, L3_TILER_MODE_3,
        SZ_128M, 0, 0, "DSP_TILER_MODE_3",
    },

    {
        TYPE_DEVMEM,
        DSP_PERIPHERAL_L4CFG, L4_PERIPHERAL_L4CFG,
        SZ_16M, 0, 0, "DSP_PERIPHERAL_L4CFG",
    },

    {
        TYPE_DEVMEM,
        DSP_PERIPHERAL_L4PER1, L4_PERIPHERAL_L4PER1,
        SZ_2M, 0, 0, "DSP_PERIPHERAL_L4PER1",
    },

    {
        TYPE_DEVMEM,
        DSP_PERIPHERAL_L4PER2, L4_PERIPHERAL_L4PER2,
        SZ_4M, 0, 0, "DSP_PERIPHERAL_L4PER2",
    },

    {
        TYPE_DEVMEM,
        DSP_PERIPHERAL_L4PER3, L4_PERIPHERAL_L4PER3,
        SZ_8M, 0, 0, "DSP_PERIPHERAL_L4PER3",
    },

    {
        TYPE_DEVMEM,
        DSP_SYSTEM_DMA_BASE_ADDR, DSP_SYSTEM_DMA_BASE_ADDR,
        DSP_SYSTEM_DMA_SIZE, 0, 0, "DSP_SYSTEM_DMA",
    },

    {
        TYPE_DEVMEM,
        DSP_PERIPHERAL_L4EMU, L4_PERIPHERAL_L4EMU,
        SZ_16M, 0, 0, "DSP_PERIPHERAL_L4EMU",
    },

    {
        TYPE_DEVMEM,
        DSP_PERIPHERAL_DMM, L3_PERIPHERAL_DMM,
        SZ_1M, 0, 0, "DSP_PERIPHERAL_DMM",
    },

    {
        TYPE_DEVMEM,
        DSP_PERIPHERAL_ISS, L3_PERIPHERAL_ISS,
        SZ_256K, 0, 0, "DSP_PERIPHERAL_ISS",
    },

    {
        TYPE_DEVMEM,
        DSP_OCMC1_BASE, DSP_OCMC1_BASE,
        DSP_OCMC_SIZE, 0, 0, "DSP_OCMC1",
    },

    {
        TYPE_DEVMEM,
        DSP_OCMC2_BASE, DSP_OCMC2_BASE,
        DSP_OCMC_SIZE, 0, 0, "DSP_OCMC2",
    },

    {
        TYPE_DEVMEM,
        DSP_OCMC3_BASE, DSP_OCMC3_BASE,
        DSP_OCMC_SIZE, 0, 0, "DSP_OCMC3",
    },

    {
        TYPE_DEVMEM,
        SYSTEM_COMMON_SHM_VIRT, SYSTEM_COMMON_SHM,
        SYSTEM_COMMON_SHM_SIZE, 0, 0, "SYSTEM_COMMON_SHM",
    },

    {
        TYPE_DEVMEM,
        DSP_SR1_VIRT, DSP_SR1,
        DSP_SR1_SIZE, 0, 0, "DSP_SR1",
    },

    {
        TYPE_DEVMEM,
        DSP_L2SRAM_BASE, DSP_L2SRAM_BASE,
        DSP_L2SRAM_SIZE, 0, 0, "DSP_L2SRAM",
    },

    {
        TYPE_DEVMEM,
        DSP_EVE_CONFIG_SPACE, DSP_EVE_CONFIG_SPACE,
        DSP_EVE_CONFIG_SPACE_SIZE, 0, 0, "DSP_EVE_CONFIG_SPACE",
    },



};

#endif /* _RSC_TABLE_DSP_H_ */
