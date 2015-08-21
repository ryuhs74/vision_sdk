/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


/**
 *******************************************************************************
 *
 *  \file system_vring_config.h  Linux <-> BIOS VRING Config
 *
 *******************************************************************************
*/

/**
 *******************************************************************************
 *
 *  \file system_vring_config.h
 *  \brief System viring addresses
 *
 *******************************************************************************
*/

#ifndef _SYSTEM_VRING_CONFIG_H_
#define _SYSTEM_VRING_CONFIG_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/* @{ */


/*
 * Vring virtual addresses
 *
 * Vring virtual addresses are hard coded by ipc, if it is changed here it needs to be
 * changed in ipc package ipc_x_yy_zz_aa/packages/ti/ipc/family/<platform>/VirtQueue.c
 *
 * Ipc lib and bios side ipu/dsp executable needs to be re-built after the change.
 */

#define IPU_MEM_IPC_VRING       0x60000000
#define IPU_MEM_RPMSG_VRING0    0x60000000
#define IPU_MEM_RPMSG_VRING1    0x60004000
#define IPU_MEM_VRING_BUFS0     0x60040000
#define IPU_MEM_VRING_BUFS1     0x60080000

#define DSP_MEM_IPC_VRING       0xA0000000
#define DSP_MEM_RPMSG_VRING0    0xA0000000
#define DSP_MEM_RPMSG_VRING1    0xA0004000
#define DSP_MEM_VRING_BUFS0     0xA0040000
#define DSP_MEM_VRING_BUFS1     0xA0080000

/*
 * Vring physical addresses
 *
 * These are expected to be matched with beginning of cma section mentioned in
 * linux kernel linux/arch/arm/mach-omap2/remotproc.c, reserved for carveouts of
 * particular remote-core.
 *
 * If there is a change in physical address of a cma section in linux side it should be
 * manually done here. Vrings are expected to be at the beginning of cma section.
 *
 * Following addresses denote where the cma section for a particular core is physically
 * allocated / reserved by linux remoteproc module.
 *
 */
#ifdef TDA2EX_BUILD

#define IPU_PHYS_MEM_IPC_VRING      0x84000000

#ifdef BUILD_DSP_1
#define DSP_PHYS_MEM_IPC_VRING      0x86000000
#endif

#else

#define IPU_PHYS_MEM_IPC_VRING      0x9d000000

#ifdef BUILD_DSP_1
#define DSP_PHYS_MEM_IPC_VRING      0x99000000
#endif

#ifdef BUILD_DSP_2
#define DSP_PHYS_MEM_IPC_VRING      0x9f000000
#endif
#endif

/*@}*/

#ifdef  __cplusplus
}
#endif

#endif /* _SYSTEM_VRING_CONFIG_H_ */



