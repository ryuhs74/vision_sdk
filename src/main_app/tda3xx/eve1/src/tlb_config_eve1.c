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
 * \file tlb_config_eve1.c
 *
 * \brief  This file implements the MMU configuration of EVE1
 *
 * \version 0.0 (Jul 2013) : [SS] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/ipc/Ipc.h>

/*******************************************************************************
 *  DEFINES
 *******************************************************************************
 */

/*******************************************************************************
 *  The macros to read and write from/to the memory
 *******************************************************************************
 */
#define WR_MEM_32(addr, data)    *(volatile unsigned int*)(addr) =(unsigned int)(data)
#define RD_MEM_32(addr)          *(volatile unsigned int*)(addr)

/*******************************************************************************
 *  MMU base address - Eve's view
 *******************************************************************************
 */
#define MMU_BASE 0x40081000

/*******************************************************************************
 *  MMU base address - IPU's view
 *******************************************************************************
 */
//#define MMU_BASE 0x42081000

/*******************************************************************************
 *  MMU address (base + offset) for various MMU settings
 *******************************************************************************
 */
#define TESLASS_MMU__MMU_CNTL    ( MMU_BASE + 0x44 )
#define TESLASS_MMU__MMU_CAM     ( MMU_BASE + 0x58 )
#define TESLASS_MMU__MMU_RAM     ( MMU_BASE + 0x5c )
#define TESLASS_MMU__MMU_LOCK    ( MMU_BASE + 0x50 )
#define TESLASS_MMU__MMU_LD_TLB  ( MMU_BASE + 0x54 )

#define MMU_PAGE_SIZE     (16*1024*1024)

/*******************************************************************************
 *  Defines for each register physical and virtual address base
 *******************************************************************************
 */
#define PHY_ADDR1   0x48000000
#define VIRT_ADDR1  0x48000000

#define PHY_ADDR2   0x40300000
#define VIRT_ADDR2  0x40300000

#define PHY_ADDR3   0x49000000
#define VIRT_ADDR3  0x49000000

#define PHY_ADDR4   0x4A000000
#define VIRT_ADDR4  0x4A000000

#define PHY_ADDR5   0x42000000
#define VIRT_ADDR5  0x42000000

#ifdef TDA3XX_64MB_DDR

/* 0x80000000 to 0x80FFFFFF is mapped in
   GEL and SBL
 */

#define PHY_ADDR6   (0x81000000)
#define VIRT_ADDR6  (PHY_ADDR6)

#define PHY_ADDR7   (0x82000000)
#define VIRT_ADDR7  (PHY_ADDR7)

/*
   PHY_ADDR9 to PHY_ADDR22
   maps beyond 64MB DDR (0x8400000 and above),
   but this is not accessed by EVE.

   As such doing the mapping in MMU has no side effect
 */

#else

/* SR0, REMOTE_LOG_SIZE mapping */
#define PHY_ADDR6   (0x9F000000)
#define VIRT_ADDR6  (PHY_ADDR6)

/* start of SR1 mapping */
#define PHY_ADDR7   (0x85000000)
#define VIRT_ADDR7  (PHY_ADDR7)

#endif

#define PHY_ADDR8   (PHY_ADDR7+MMU_PAGE_SIZE)
#define VIRT_ADDR8  (PHY_ADDR8)

#define PHY_ADDR9   (PHY_ADDR8+MMU_PAGE_SIZE)
#define VIRT_ADDR9  (PHY_ADDR9)

#define PHY_ADDR10  (PHY_ADDR9+MMU_PAGE_SIZE)
#define VIRT_ADDR10 (PHY_ADDR10)

#define PHY_ADDR11  (PHY_ADDR10+MMU_PAGE_SIZE)
#define VIRT_ADDR11 (PHY_ADDR11)

#define PHY_ADDR12  (PHY_ADDR11+MMU_PAGE_SIZE)
#define VIRT_ADDR12 (PHY_ADDR12)

#define PHY_ADDR13  (PHY_ADDR12+MMU_PAGE_SIZE)
#define VIRT_ADDR13 (PHY_ADDR13)

#define PHY_ADDR14  (PHY_ADDR13+MMU_PAGE_SIZE)
#define VIRT_ADDR14 (PHY_ADDR14)

#define PHY_ADDR15  (PHY_ADDR14+MMU_PAGE_SIZE)
#define VIRT_ADDR15 (PHY_ADDR15)

#define PHY_ADDR16  (PHY_ADDR15+MMU_PAGE_SIZE)
#define VIRT_ADDR16 (PHY_ADDR16)

#define PHY_ADDR17  (PHY_ADDR16+MMU_PAGE_SIZE)
#define VIRT_ADDR17 (PHY_ADDR17)

#define PHY_ADDR18  (PHY_ADDR17+MMU_PAGE_SIZE)
#define VIRT_ADDR18 (PHY_ADDR18)

#define PHY_ADDR19  (PHY_ADDR18+MMU_PAGE_SIZE)
#define VIRT_ADDR19 (PHY_ADDR19)

#define PHY_ADDR20  (PHY_ADDR19+MMU_PAGE_SIZE)
#define VIRT_ADDR20 (PHY_ADDR20)

#define PHY_ADDR21  (PHY_ADDR20+MMU_PAGE_SIZE)
#define VIRT_ADDR21 (PHY_ADDR21)

#define PHY_ADDR22  (PHY_ADDR21+MMU_PAGE_SIZE)
#define VIRT_ADDR22 (PHY_ADDR22)

#define PHY_ADDR23   0x41000000
#define VIRT_ADDR23  (PHY_ADDR23)

/* This mapping required for Utils_VIP_Interrupt_ */
#define PHY_ADDR24   0x48000000
#define VIRT_ADDR24  0x68000000

/* end of SR1 mapping */

/**
 *******************************************************************************
 *
 * \brief This function implements the MMU configuration of EVE1
 *
 * \return  void
 *
 *******************************************************************************
 */
void eve1MmuConfig(void)
{
    /* ------------------------------------------------------------------------------------------------------- */
    WR_MEM_32(TESLASS_MMU__MMU_CAM, 0x0000000f | (VIRT_ADDR1 & 0xFFFFE000));
    WR_MEM_32(TESLASS_MMU__MMU_RAM, 0x000001c0 | (PHY_ADDR1  & 0xFFFFE000));

    /* tlbEntry is bits 8:4
    #define TESLASS_MMU__MMU_LOCK__CURRENTVICTIM          BITFIELD(8, 4) */
    WR_MEM_32(TESLASS_MMU__MMU_LOCK, ((RD_MEM_32(TESLASS_MMU__MMU_LOCK)) & 0xFFFFFE0F) | ( 3 << 4 ));
    WR_MEM_32(TESLASS_MMU__MMU_LD_TLB, 1 );
    /* ------------------------------------------------------------------------------------------------------- */

    /* ------------------------------------------------------------------------------------------------------- */
    WR_MEM_32(TESLASS_MMU__MMU_CAM, 0x0000000f | (VIRT_ADDR2 & 0xFFFFE000));
    WR_MEM_32(TESLASS_MMU__MMU_RAM, 0x000001c0 | (PHY_ADDR2  & 0xFFFFE000));

    /* tlbEntry is bits 8:4
    #define TESLASS_MMU__MMU_LOCK__CURRENTVICTIM          BITFIELD(8, 4) */
    WR_MEM_32(TESLASS_MMU__MMU_LOCK, ((RD_MEM_32(TESLASS_MMU__MMU_LOCK)) & 0xFFFFFE0F) | ( 4 << 4 ));
    WR_MEM_32(TESLASS_MMU__MMU_LD_TLB, 1 );
    /* ------------------------------------------------------------------------------------------------------- */

    /* ------------------------------------------------------------------------------------------------------- */
    WR_MEM_32(TESLASS_MMU__MMU_CAM, 0x0000000f | (VIRT_ADDR3 & 0xFFFFE000));
    WR_MEM_32(TESLASS_MMU__MMU_RAM, 0x000001c0 | (PHY_ADDR3  & 0xFFFFE000));

    /* tlbEntry is bits 8:4
    #define TESLASS_MMU__MMU_LOCK__CURRENTVICTIM          BITFIELD(8, 4) */
    WR_MEM_32(TESLASS_MMU__MMU_LOCK, ((RD_MEM_32(TESLASS_MMU__MMU_LOCK)) & 0xFFFFFE0F) | ( 5 << 4 ));
    WR_MEM_32(TESLASS_MMU__MMU_LD_TLB, 1 );
    /* ------------------------------------------------------------------------------------------------------- */

    /* ------------------------------------------------------------------------------------------------------- */
    WR_MEM_32(TESLASS_MMU__MMU_CAM, 0x0000000f | (VIRT_ADDR4 & 0xFFFFE000));
    WR_MEM_32(TESLASS_MMU__MMU_RAM, 0x000001c0 | (PHY_ADDR4  & 0xFFFFE000));

    /* tlbEntry is bits 8:4
    #define TESLASS_MMU__MMU_LOCK__CURRENTVICTIM          BITFIELD(8, 4) */
    WR_MEM_32(TESLASS_MMU__MMU_LOCK, ((RD_MEM_32(TESLASS_MMU__MMU_LOCK)) & 0xFFFFFE0F) | ( 6 << 4 ));
    WR_MEM_32(TESLASS_MMU__MMU_LD_TLB, 1 );
    /* ------------------------------------------------------------------------------------------------------- */

    /* ------------------------------------------------------------------------------------------------------- */
    WR_MEM_32(TESLASS_MMU__MMU_CAM, 0x0000000f | (VIRT_ADDR5 & 0xFFFFE000));
    WR_MEM_32(TESLASS_MMU__MMU_RAM, 0x000001c0 | (PHY_ADDR5  & 0xFFFFE000));

    /* tlbEntry is bits 8:4
    #define TESLASS_MMU__MMU_LOCK__CURRENTVICTIM          BITFIELD(8, 4) */
    WR_MEM_32(TESLASS_MMU__MMU_LOCK, ((RD_MEM_32(TESLASS_MMU__MMU_LOCK)) & 0xFFFFFE0F) | ( 7 << 4 ));
    WR_MEM_32(TESLASS_MMU__MMU_LD_TLB, 1 );
    /* ------------------------------------------------------------------------------------------------------- */

    /* ------------------------------------------------------------------------------------------------------- */
    WR_MEM_32(TESLASS_MMU__MMU_CAM, 0x0000000f | (VIRT_ADDR6 & 0xFFFFE000));
    WR_MEM_32(TESLASS_MMU__MMU_RAM, 0x000001c0 | (PHY_ADDR6  & 0xFFFFE000));

    /* tlbEntry is bits 8:4
    #define TESLASS_MMU__MMU_LOCK__CURRENTVICTIM          BITFIELD(8, 4) */
    WR_MEM_32(TESLASS_MMU__MMU_LOCK, ((RD_MEM_32(TESLASS_MMU__MMU_LOCK)) & 0xFFFFFE0F) | ( 8 << 4 ));
    WR_MEM_32(TESLASS_MMU__MMU_LD_TLB, 1 );
    /* ------------------------------------------------------------------------------------------------------- */


    /* ------------------------------------------------------------------------------------------------------- */
    WR_MEM_32(TESLASS_MMU__MMU_CAM, 0x0000000f | (VIRT_ADDR7 & 0xFFFFE000));
    WR_MEM_32(TESLASS_MMU__MMU_RAM, 0x000001c0 | (PHY_ADDR7  & 0xFFFFE000));

    /* tlbEntry is bits 8:4
    #define TESLASS_MMU__MMU_LOCK__CURRENTVICTIM          BITFIELD(8, 4) */
    WR_MEM_32(TESLASS_MMU__MMU_LOCK, ((RD_MEM_32(TESLASS_MMU__MMU_LOCK)) & 0xFFFFFE0F) | ( 9 << 4 ));
    WR_MEM_32(TESLASS_MMU__MMU_LD_TLB, 1 );
    /* ------------------------------------------------------------------------------------------------------- */


    /* ------------------------------------------------------------------------------------------------------- */
    WR_MEM_32(TESLASS_MMU__MMU_CAM, 0x0000000f | (VIRT_ADDR8 & 0xFFFFE000));
    WR_MEM_32(TESLASS_MMU__MMU_RAM, 0x000001c0 | (PHY_ADDR8  & 0xFFFFE000));

    /* tlbEntry is bits 8:4
    #define TESLASS_MMU__MMU_LOCK__CURRENTVICTIM          BITFIELD(8, 4) */
    WR_MEM_32(TESLASS_MMU__MMU_LOCK, ((RD_MEM_32(TESLASS_MMU__MMU_LOCK)) & 0xFFFFFE0F) | ( 10 << 4 ));
    WR_MEM_32(TESLASS_MMU__MMU_LD_TLB, 1 );
    /* ------------------------------------------------------------------------------------------------------- */


    /* ------------------------------------------------------------------------------------------------------- */
    WR_MEM_32(TESLASS_MMU__MMU_CAM, 0x0000000f | (VIRT_ADDR9 & 0xFFFFE000));
    WR_MEM_32(TESLASS_MMU__MMU_RAM, 0x000001c0 | (PHY_ADDR9  & 0xFFFFE000));

    /* tlbEntry is bits 8:4
    #define TESLASS_MMU__MMU_LOCK__CURRENTVICTIM          BITFIELD(8, 4) */
    WR_MEM_32(TESLASS_MMU__MMU_LOCK, ((RD_MEM_32(TESLASS_MMU__MMU_LOCK)) & 0xFFFFFE0F) | ( 11 << 4 ));
    WR_MEM_32(TESLASS_MMU__MMU_LD_TLB, 1 );
    /* ------------------------------------------------------------------------------------------------------- */


    /* ------------------------------------------------------------------------------------------------------- */
    WR_MEM_32(TESLASS_MMU__MMU_CAM, 0x0000000f | (VIRT_ADDR10 & 0xFFFFE000));
    WR_MEM_32(TESLASS_MMU__MMU_RAM, 0x000001c0 | (PHY_ADDR10  & 0xFFFFE000));

    /* tlbEntry is bits 8:4
    #define TESLASS_MMU__MMU_LOCK__CURRENTVICTIM          BITFIELD(8, 4) */
    WR_MEM_32(TESLASS_MMU__MMU_LOCK, ((RD_MEM_32(TESLASS_MMU__MMU_LOCK)) & 0xFFFFFE0F) | ( 12 << 4 ));
    WR_MEM_32(TESLASS_MMU__MMU_LD_TLB, 1 );
    /* ------------------------------------------------------------------------------------------------------- */


    /* ------------------------------------------------------------------------------------------------------- */
    WR_MEM_32(TESLASS_MMU__MMU_CAM, 0x0000000f | (VIRT_ADDR11 & 0xFFFFE000));
    WR_MEM_32(TESLASS_MMU__MMU_RAM, 0x000001c0 | (PHY_ADDR11  & 0xFFFFE000));

    /* tlbEntry is bits 8:4
    #define TESLASS_MMU__MMU_LOCK__CURRENTVICTIM          BITFIELD(8, 4) */
    WR_MEM_32(TESLASS_MMU__MMU_LOCK, ((RD_MEM_32(TESLASS_MMU__MMU_LOCK)) & 0xFFFFFE0F) | ( 13 << 4 ));
    WR_MEM_32(TESLASS_MMU__MMU_LD_TLB, 1 );
    /* ------------------------------------------------------------------------------------------------------- */


    /* ------------------------------------------------------------------------------------------------------- */
    WR_MEM_32(TESLASS_MMU__MMU_CAM, 0x0000000f | (VIRT_ADDR12 & 0xFFFFE000));
    WR_MEM_32(TESLASS_MMU__MMU_RAM, 0x000001c0 | (PHY_ADDR12  & 0xFFFFE000));

    /* tlbEntry is bits 8:4
    #define TESLASS_MMU__MMU_LOCK__CURRENTVICTIM          BITFIELD(8, 4) */
    WR_MEM_32(TESLASS_MMU__MMU_LOCK, ((RD_MEM_32(TESLASS_MMU__MMU_LOCK)) & 0xFFFFFE0F) | ( 14 << 4 ));
    WR_MEM_32(TESLASS_MMU__MMU_LD_TLB, 1 );
    /* ------------------------------------------------------------------------------------------------------- */


    /* ------------------------------------------------------------------------------------------------------- */
    WR_MEM_32(TESLASS_MMU__MMU_CAM, 0x0000000f | (VIRT_ADDR13 & 0xFFFFE000));
    WR_MEM_32(TESLASS_MMU__MMU_RAM, 0x000001c0 | (PHY_ADDR13  & 0xFFFFE000));

    /* tlbEntry is bits 8:4
    #define TESLASS_MMU__MMU_LOCK__CURRENTVICTIM          BITFIELD(8, 4) */
    WR_MEM_32(TESLASS_MMU__MMU_LOCK, ((RD_MEM_32(TESLASS_MMU__MMU_LOCK)) & 0xFFFFFE0F) | ( 15 << 4 ));
    WR_MEM_32(TESLASS_MMU__MMU_LD_TLB, 1 );
    /* ------------------------------------------------------------------------------------------------------- */


    /* ------------------------------------------------------------------------------------------------------- */
    WR_MEM_32(TESLASS_MMU__MMU_CAM, 0x0000000f | (VIRT_ADDR14 & 0xFFFFE000));
    WR_MEM_32(TESLASS_MMU__MMU_RAM, 0x000001c0 | (PHY_ADDR14  & 0xFFFFE000));

    /* tlbEntry is bits 8:4
    #define TESLASS_MMU__MMU_LOCK__CURRENTVICTIM          BITFIELD(8, 4) */
    WR_MEM_32(TESLASS_MMU__MMU_LOCK, ((RD_MEM_32(TESLASS_MMU__MMU_LOCK)) & 0xFFFFFE0F) | ( 16 << 4 ));
    WR_MEM_32(TESLASS_MMU__MMU_LD_TLB, 1 );
    /* ------------------------------------------------------------------------------------------------------- */


    /* ------------------------------------------------------------------------------------------------------- */
    WR_MEM_32(TESLASS_MMU__MMU_CAM, 0x0000000f | (VIRT_ADDR15 & 0xFFFFE000));
    WR_MEM_32(TESLASS_MMU__MMU_RAM, 0x000001c0 | (PHY_ADDR15  & 0xFFFFE000));

    /* tlbEntry is bits 8:4
    #define TESLASS_MMU__MMU_LOCK__CURRENTVICTIM          BITFIELD(8, 4) */
    WR_MEM_32(TESLASS_MMU__MMU_LOCK, ((RD_MEM_32(TESLASS_MMU__MMU_LOCK)) & 0xFFFFFE0F) | ( 17 << 4 ));
    WR_MEM_32(TESLASS_MMU__MMU_LD_TLB, 1 );
    /* ------------------------------------------------------------------------------------------------------- */


    /* ------------------------------------------------------------------------------------------------------- */
    WR_MEM_32(TESLASS_MMU__MMU_CAM, 0x0000000f | (VIRT_ADDR16 & 0xFFFFE000));
    WR_MEM_32(TESLASS_MMU__MMU_RAM, 0x000001c0 | (PHY_ADDR16  & 0xFFFFE000));

    /* tlbEntry is bits 8:4
    #define TESLASS_MMU__MMU_LOCK__CURRENTVICTIM          BITFIELD(8, 4) */
    WR_MEM_32(TESLASS_MMU__MMU_LOCK, ((RD_MEM_32(TESLASS_MMU__MMU_LOCK)) & 0xFFFFFE0F) | ( 18 << 4 ));
    WR_MEM_32(TESLASS_MMU__MMU_LD_TLB, 1 );
    /* ------------------------------------------------------------------------------------------------------- */

    /* ------------------------------------------------------------------------------------------------------- */
    WR_MEM_32(TESLASS_MMU__MMU_CAM, 0x0000000f | (VIRT_ADDR17 & 0xFFFFE000));
    WR_MEM_32(TESLASS_MMU__MMU_RAM, 0x000001c0 | (PHY_ADDR17  & 0xFFFFE000));

    /* tlbEntry is bits 8:4
    #define TESLASS_MMU__MMU_LOCK__CURRENTVICTIM          BITFIELD(8, 4) */
    WR_MEM_32(TESLASS_MMU__MMU_LOCK, ((RD_MEM_32(TESLASS_MMU__MMU_LOCK)) & 0xFFFFFE0F) | ( 19 << 4 ));
    WR_MEM_32(TESLASS_MMU__MMU_LD_TLB, 1 );
    /* ------------------------------------------------------------------------------------------------------- */


    /* ------------------------------------------------------------------------------------------------------- */
    WR_MEM_32(TESLASS_MMU__MMU_CAM, 0x0000000f | (VIRT_ADDR18 & 0xFFFFE000));
    WR_MEM_32(TESLASS_MMU__MMU_RAM, 0x000001c0 | (PHY_ADDR18  & 0xFFFFE000));

    /* tlbEntry is bits 8:4
    #define TESLASS_MMU__MMU_LOCK__CURRENTVICTIM          BITFIELD(8, 4) */
    WR_MEM_32(TESLASS_MMU__MMU_LOCK, ((RD_MEM_32(TESLASS_MMU__MMU_LOCK)) & 0xFFFFFE0F) | ( 20 << 4 ));
    WR_MEM_32(TESLASS_MMU__MMU_LD_TLB, 1 );
    /* ------------------------------------------------------------------------------------------------------- */


    /* ------------------------------------------------------------------------------------------------------- */
    WR_MEM_32(TESLASS_MMU__MMU_CAM, 0x0000000f | (VIRT_ADDR19 & 0xFFFFE000));
    WR_MEM_32(TESLASS_MMU__MMU_RAM, 0x000001c0 | (PHY_ADDR19  & 0xFFFFE000));

    /* tlbEntry is bits 8:4
    #define TESLASS_MMU__MMU_LOCK__CURRENTVICTIM          BITFIELD(8, 4) */
    WR_MEM_32(TESLASS_MMU__MMU_LOCK, ((RD_MEM_32(TESLASS_MMU__MMU_LOCK)) & 0xFFFFFE0F) | ( 21 << 4 ));
    WR_MEM_32(TESLASS_MMU__MMU_LD_TLB, 1 );
    /* ------------------------------------------------------------------------------------------------------- */


    /* ------------------------------------------------------------------------------------------------------- */
    WR_MEM_32(TESLASS_MMU__MMU_CAM, 0x0000000f | (VIRT_ADDR20 & 0xFFFFE000));
    WR_MEM_32(TESLASS_MMU__MMU_RAM, 0x000001c0 | (PHY_ADDR20  & 0xFFFFE000));

    /* tlbEntry is bits 8:4
    #define TESLASS_MMU__MMU_LOCK__CURRENTVICTIM          BITFIELD(8, 4) */
    WR_MEM_32(TESLASS_MMU__MMU_LOCK, ((RD_MEM_32(TESLASS_MMU__MMU_LOCK)) & 0xFFFFFE0F) | ( 22 << 4 ));
    WR_MEM_32(TESLASS_MMU__MMU_LD_TLB, 1 );
    /* ------------------------------------------------------------------------------------------------------- */


    /* ------------------------------------------------------------------------------------------------------- */
    WR_MEM_32(TESLASS_MMU__MMU_CAM, 0x0000000f | (VIRT_ADDR21 & 0xFFFFE000));
    WR_MEM_32(TESLASS_MMU__MMU_RAM, 0x000001c0 | (PHY_ADDR21  & 0xFFFFE000));

    /* tlbEntry is bits 8:4
    #define TESLASS_MMU__MMU_LOCK__CURRENTVICTIM          BITFIELD(8, 4) */
    WR_MEM_32(TESLASS_MMU__MMU_LOCK, ((RD_MEM_32(TESLASS_MMU__MMU_LOCK)) & 0xFFFFFE0F) | ( 23 << 4 ));
    WR_MEM_32(TESLASS_MMU__MMU_LD_TLB, 1 );
    /* ------------------------------------------------------------------------------------------------------- */


    /* ------------------------------------------------------------------------------------------------------- */
    WR_MEM_32(TESLASS_MMU__MMU_CAM, 0x0000000f | (VIRT_ADDR22 & 0xFFFFE000));
    WR_MEM_32(TESLASS_MMU__MMU_RAM, 0x000001c0 | (PHY_ADDR22  & 0xFFFFE000));

    /* tlbEntry is bits 8:4
    #define TESLASS_MMU__MMU_LOCK__CURRENTVICTIM          BITFIELD(8, 4) */
    WR_MEM_32(TESLASS_MMU__MMU_LOCK, ((RD_MEM_32(TESLASS_MMU__MMU_LOCK)) & 0xFFFFFE0F) | ( 24 << 4 ));
    WR_MEM_32(TESLASS_MMU__MMU_LD_TLB, 1 );
    /* ------------------------------------------------------------------------------------------------------- */

    /* ------------------------------------------------------------------------------------------------------- */
    WR_MEM_32(TESLASS_MMU__MMU_CAM, 0x0000000f | (VIRT_ADDR23 & 0xFFFFE000));
    WR_MEM_32(TESLASS_MMU__MMU_RAM, 0x000001c0 | (PHY_ADDR23  & 0xFFFFE000));

    /* tlbEntry is bits 8:4
    #define TESLASS_MMU__MMU_LOCK__CURRENTVICTIM          BITFIELD(8, 4) */
    WR_MEM_32(TESLASS_MMU__MMU_LOCK, ((RD_MEM_32(TESLASS_MMU__MMU_LOCK)) & 0xFFFFFE0F) | ( 25 << 4 ));
    WR_MEM_32(TESLASS_MMU__MMU_LD_TLB, 1 );
    /* ------------------------------------------------------------------------------------------------------- */

    /* ------------------------------------------------------------------------------------------------------- */
    WR_MEM_32(TESLASS_MMU__MMU_CAM, 0x0000000f | (VIRT_ADDR24 & 0xFFFFE000));
    WR_MEM_32(TESLASS_MMU__MMU_RAM, 0x000001c0 | (PHY_ADDR24  & 0xFFFFE000));

    /* tlbEntry is bits 8:4
    #define TESLASS_MMU__MMU_LOCK__CURRENTVICTIM          BITFIELD(8, 4) */
    WR_MEM_32(TESLASS_MMU__MMU_LOCK, ((RD_MEM_32(TESLASS_MMU__MMU_LOCK)) & 0xFFFFFE0F) | ( 26 << 4 ));
    WR_MEM_32(TESLASS_MMU__MMU_LD_TLB, 1 );
    /* ------------------------------------------------------------------------------------------------------- */

    /*Enable MMU*/
    WR_MEM_32(TESLASS_MMU__MMU_CNTL, ((RD_MEM_32(TESLASS_MMU__MMU_CNTL)) & 0xFFFFFFFD) | 0x2);

}

/* Nothing beyond this point */

