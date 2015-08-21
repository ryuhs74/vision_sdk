/* ======================================================================
 *   Copyright (C) 2013 Texas Instruments Incorporated
 *
 *   All rights reserved. Property of Texas Instruments Incorporated.
 *   Restricted rights to use, duplicate or disclose this code are
 *   granted through contract.
 *
 *   The program may not be used without the written permission
 *   of Texas Instruments Incorporated or against the terms and conditions
 *   stipulated in the agreement under which this program has been
 *   supplied.
 * ==================================================================== */
/**
 *   Component:         SBL
 *
 *   Filename:          sbl_rprc_parse.c
 *
 *   Description:      This file contains functions to parse the multi-core
 *                     image file & loads into CPU internal memory & enternal
 *                     memory.
 */

/****************************************************************
 *  INCLUDE FILES
 ****************************************************************/
#include "stdint.h"
#include <src/utils_common/include/utils.h>
#include <stdio.h>
#include <string.h>
#include "utils_rprc_parse.h"
#include "utils_tda2xx_platform.h"
#include "platform.h"
// #include "cp15.h"
#include "edma.h"

#ifdef IPU1_LOAD_EVES
/* This expected to be defined in IPU1 application */
extern unsigned char gTDA2XX_EVE_FIRMWARE;
#endif

#ifdef SBL_REGRESSION
extern UInt32           SBL_REGRESSION_LOG_BFR_SEG;
sbl_regression_log_bfr_t *sbl_regression_log_bfr_ptr;

#endif

/* ============================================================================
 * LOCAL VARIABLES DECLARATIONS
 * =============================================================================
 */

/*TODO: TI814x CPU ID - change it to enum*/
#ifndef CORE_A8
#define CORE_A8 (0xFFFFFFFF)
#endif

#ifndef CORE_DSP
#define CORE_DSP (0xFFFFFFFF)
#endif

#ifndef CORE_M3VIDEO
#define CORE_M3VIDEO (0xFFFFFFFF)
#endif

#ifndef CORE_M3VPSS
#define CORE_M3VPSS (0xFFFFFFFF)
#endif

#ifndef CORE_ARP32
#define CORE_ARP32 (0xFFFFFFFF)
#endif

extern UInt32 entryPoint_EVE1;
extern UInt32 entryPoint_EVE2;
extern UInt32 entryPoint_EVE3;
extern UInt32 entryPoint_EVE4;

Int32 MulticoreImageParse(void *srcAddr, UInt32 ImageOffset);
static Int32 RprcImageParse(void *srcAddr, UInt32 *entryPoint,
                              Int32 CoreId);
void BootCore(UInt32 entry, UInt32 CoreID);

UInt32 (*fp_readData)(const void *dstAddr, void *srcAddr,
                               UInt32 length) = NULL;
void (*fp_seek)(const void *srcAddr, UInt32 location) = NULL;

/******************************************************************************
**                       ZEBU RPRC parse functions
*******************************************************************************/

/**
 * \brief       DDR3ReadLocal function reads N bytes from DDR memory
 *              and advances the cursor.
 *
 * \param[out]  dst1 - Pointer to data buffer
 * \param[in]       offsetPtr - Read head pointer
 * \param[in]       length - Number of bytes to read
 *
 *
 * \return            Error code
 */
UInt32 DDR3ReadLocal(const void *dst1, void *offsetPtr, UInt32 length)
{

    memcpy((UInt32 *)dst1, (UInt32 *)offsetPtr, length);

    /* Forward the srcaddr for next section */
    *((UInt32 *) (offsetPtr)) += length;

    return 0;
}

/**
 * \brief       DR3_seek function to move the read head by n bytes
 *
 * \param[in]  srcAddr - Read head pointer
 * \param[in]   location - Move the read head pointer by n bytes
 *
 *
 * \return  none
 */
void DDR3_seek(const void *srcAddr, UInt32 location)
{
    *((UInt32 *) srcAddr) = location;
}

/**
 * \brief       DDR3BootRprc function parse the multi-core app image
 *              stored in the DDR.
 *              It Parses the AppImage & copies the section into CPU
 *              internal memory & external memory.
 *              CPUs entry loctions are stored into entry point
 *              global pointers.
 *
 *
 * \param               none
 *
 *
 * \return         error status.If error has occured it returns a non zero
 *                 value.
 *                 If no error has occured then return status will be
 *                 zero.
 */
Int32 DDR3BootRprc(void)
{
    fp_readData = DDR3ReadLocal;

    fp_seek = DDR3_seek;

    return (MulticoreImageParse((void *) &gTDA2XX_EVE_FIRMWARE, 0x0));
}

/**
 * \brief  Function to read the device ID
 *
 *
 * \param   None.
 *
 * \return   Return the device id
 *
 **/

UInt32 GetDeviceId()
{
    /* In Control_Module->Device_ID register specify the Device ID, PartNum &
     * MFGR.
     *   For CentEve, Device ID value (Bit31:28 ) 1000b. DevId = 8
     *  For Centaurus, Device ID value(Bit31:28) 0011b. DevId = 3
     */
    return (0x37); // for tda2xx : TBD
}


/**
 * \brief                   MulticoreImageParse function parses the multi-core
 *                          app image.
 *                          Read the image header & check for AppImage.
 *                          Device ID field confirms the boot device ID.
 *                          parses the meta header & finds the number
 *                          executables
 *                          Parses & load each section into CPU internal memory
 *                          & external memory
 *
 * \param[in]       srcAddr - Start address of AppImage
 *                          ImageOffset - Dummy
 *
 * \return            error status.If error has occured it returns a non zero
 *                     value.
 *                     If no error has occured then return status will be
 *                     zero.
 */

Int32 MulticoreImageParse(void *srcAddr, UInt32 ImageOffset)
{
    UInt32          i = 0;
    UInt32          entryPoint = 0;
    meta_header_start mHdrStr;
    meta_header_core  mHdrCore[MAX_INPUT_FILES];
    meta_header_end   mHdrEnd;
    Int32           magic_str = META_HDR_MAGIC_STR;
    Int32           retVal    = SYSTEM_LINK_STATUS_SOK;
    UInt8 *          curAddr  = srcAddr;
    UInt8 *          rprcAddr = srcAddr;

    if (fp_readData == NULL || fp_seek == NULL)
    {
        return SYSTEM_LINK_STATUS_EFAIL;
    }
    /* Read Meta Header Start and get the Number of Input RPRC Files */
    memcpy(&mHdrStr, curAddr, sizeof(meta_header_start));
    curAddr += sizeof(meta_header_start);

    if (mHdrStr.magic_str != (UInt32) magic_str)
    {
        Vps_printf( " UTILS: EVELOADER: Invalid magic number in Single image header\r\n");
        return SYSTEM_LINK_STATUS_EFAIL;
    }

    if (mHdrStr.dev_id != GetDeviceId())
    {
        Vps_printf( "\n UTILS: EVELOADER: WARNING: Device Id Doesnot match\r\n");
    }

    /* Read all the Core offset addresses */
    for (i = 0; i < mHdrStr.num_files; i++)
    {
        memcpy(&mHdrCore[i], curAddr, sizeof (meta_header_core));
        curAddr += sizeof (meta_header_core);

    }

    /* Add Base Offset address for All core Image start offset */
    for (i = 0; i < mHdrStr.num_files; i++)
    {
        mHdrCore[i].image_offset += ImageOffset;  // ImageOffset = 0; redundant code
    }

    /* Read Meta Header End */
    memcpy(&mHdrEnd, curAddr, sizeof (meta_header_end));
    curAddr += sizeof (meta_header_end);


    /* Now Parse Individual RPRC files */

    for (i = 0; i < mHdrStr.num_files; i++)
    {
        if (mHdrCore[i].core_id != (0xFFFFFFFF))
        {
            if (RprcImageParse(rprcAddr + mHdrCore[i].image_offset, &entryPoint,
                               mHdrCore[i].core_id) != SYSTEM_LINK_STATUS_SOK)
            {
                /* Error occurred parsing the RPRC file continue to parsing next
                 * image and skip booting the particular core
                 */
                retVal = SYSTEM_LINK_STATUS_EFAIL;
            }
            else
            {
                BootCore(entryPoint, mHdrCore[i].core_id);
            }

        }
    }
    return retVal;
}

/**
 * \brief           RprcImageParse function parse the RPRC executable image.
 *                  Copies individual section into destination location
 *
 * \param[in]    srcAddr - Pointer RPRC image
 * \param[out] entryPoint - CPU entry point address
 * \param[in]    CoreId - CPU ID to identify the CPU core
 *
 *
 * \return UInt32: Status (success or failure)
 */
static Int32 RprcImageParse(void *srcAddr, UInt32 *entryPoint,
                              Int32 CoreId)
{
    rprcFileHeader    header;
    rprcSectionHeader section;
    Int32           i;
    UInt8 *        curAddr   = srcAddr;


    /*read application image header*/
    memcpy(&header, curAddr, sizeof(rprcFileHeader));
    curAddr += sizeof(rprcFileHeader);


    /*check magic number*/
    if (header.magic != RPRC_MAGIC_NUMBER)
    {
        Vps_printf( " UTILS: EVELOADER: Invalid magic number in boot image\r\n");
        return (SYSTEM_LINK_STATUS_EFAIL);
    }

    /* Set the Entry Point */
    *entryPoint = header.entry;

    /*read entrypoint and copy sections to memory*/
    for (i = 0; i < header.SectionCount; i++)
    {
        /*read new section header*/
        memcpy(&section, curAddr, sizeof(rprcSectionHeader));
        curAddr += sizeof(rprcSectionHeader);


        if (section.addr >= SOC_OCMC_RAM1_BASE && section.addr <
            (SOC_OCMC_RAM1_BASE + SOC_OCMC_RAM1_SIZE) ||
            section.addr >= SOC_OCMC_RAM2_BASE && section.addr <
            (SOC_OCMC_RAM2_BASE + SOC_OCMC_RAM2_SIZE) ||
            section.addr >= SOC_OCMC_RAM3_BASE && section.addr <
            (SOC_OCMC_RAM3_BASE + SOC_OCMC_RAM3_SIZE)
            )
        {
            // Internal OCMC RAM Space for all the cores
        }
        /*copy section to memory*/
        /*check for section mapped into CPU internal memories*/
        else if (section.addr < 0x80000000)
        {
            switch (CoreId)
            {
                case EVE1_ID:

                    /*DMEM*/
                    if (section.addr >= SOC_EVE_DMEM_BASE && section.addr <
                        (SOC_EVE_DMEM_BASE + 0x8000))
                    {
                        section.addr = section.addr - SOC_EVE_DMEM_BASE;
                        section.addr = MPU_EVE1_DMEM_BASE + section.addr;
                    }
                    /*WMEM*/
                    else if (section.addr >= SOC_EVE_WBUF_BASE &&
                             section.addr < (SOC_EVE_WBUF_BASE + 0x8000))
                    {
                        section.addr = section.addr - SOC_EVE_WBUF_BASE;
                        section.addr = MPU_EVE1_WBUF_BASE + section.addr;
                    }
                    /*IBUFLA*/
                    else if (section.addr >= SOC_EVE_IBUF_LA_BASE &&
                             section.addr < (SOC_EVE_IBUF_LA_BASE + 0x4000))
                    {
                        section.addr = section.addr - SOC_EVE_IBUF_LA_BASE;
                        section.addr = MPU_EVE1_IBUF_LA_BASE + section.addr;
                    }
                    /*IBUFLH*/
                    else if (section.addr >= SOC_EVE_IBUF_HA_BASE &&
                             section.addr < (SOC_EVE_IBUF_HA_BASE + 0x4000))
                    {
                        section.addr = section.addr - SOC_EVE_IBUF_HA_BASE;
                        section.addr = MPU_EVE1_IBUF_HA_BASE + section.addr;
                    }
                    /*IBUFLB*/
                    else if (section.addr >= SOC_EVE_IBUF_LB_BASE &&
                             section.addr < (SOC_EVE_IBUF_LB_BASE + 0x4000))
                    {
                        section.addr = section.addr - SOC_EVE_IBUF_LB_BASE;
                        section.addr = MPU_EVE1_IBUF_LB_BASE + section.addr;
                    }
                    /*IBUFHB*/
                    else if (section.addr >= SOC_EVE_IBUF_HB_BASE &&
                             section.addr < (SOC_EVE_IBUF_HB_BASE + 0x4000))
                    {
                        section.addr = section.addr - SOC_EVE_IBUF_HB_BASE;
                        section.addr = MPU_EVE1_IBUF_HB_BASE + section.addr;
                    }
                    else
                    {
                        Vps_printf( " UTILS: EVELOADER: EVE1 - Invalid Memory section\n");
                    }

                    break;

                case EVE2_ID:
                    // DMEM
                    if (section.addr >= SOC_EVE_DMEM_BASE && section.addr <
                        (SOC_EVE_DMEM_BASE + 0x8000))
                    {
                        section.addr = section.addr - SOC_EVE_DMEM_BASE;
                        section.addr = MPU_EVE2_DMEM_BASE + section.addr;
                    }
                    // WMEM
                    else if (section.addr >= SOC_EVE_WBUF_BASE &&
                             section.addr < (SOC_EVE_WBUF_BASE + 0x8000))
                    {
                        section.addr = section.addr - SOC_EVE_WBUF_BASE;
                        section.addr = MPU_EVE2_WBUF_BASE + section.addr;
                    }
                    // IBUFLA
                    else if (section.addr >= SOC_EVE_IBUF_LA_BASE &&
                             section.addr < (SOC_EVE_IBUF_LA_BASE + 0x4000))
                    {
                        section.addr = section.addr - SOC_EVE_IBUF_LA_BASE;
                        section.addr = MPU_EVE2_IBUF_LA_BASE + section.addr;
                    }
                    // IBUFH
                    else if (section.addr >= SOC_EVE_IBUF_HA_BASE &&
                             section.addr < (SOC_EVE_IBUF_HA_BASE + 0x4000))
                    {
                        section.addr = section.addr - SOC_EVE_IBUF_HA_BASE;
                        section.addr = MPU_EVE2_IBUF_HA_BASE + section.addr;
                    }
                    // IBUFLB
                    else if (section.addr >= SOC_EVE_IBUF_LB_BASE &&
                             section.addr < (SOC_EVE_IBUF_LB_BASE + 0x4000))
                    {
                        section.addr = section.addr - SOC_EVE_IBUF_LB_BASE;
                        section.addr = MPU_EVE2_IBUF_LB_BASE + section.addr;
                    }
                    else if (section.addr >= SOC_EVE_IBUF_HB_BASE &&
                             section.addr < (SOC_EVE_IBUF_HB_BASE + 0x4000))
                    {
                        section.addr = section.addr - SOC_EVE_IBUF_HB_BASE;
                        section.addr = MPU_EVE2_IBUF_HB_BASE + section.addr;
                    }
                    else
                    {
                        Vps_printf( " UTILS: EVELOADER: EVE2 - Invalid Memory section\n");
                    }

                    break;

                case EVE3_ID:
                    // DMEM
                    if (section.addr >= SOC_EVE_DMEM_BASE && section.addr <
                        (SOC_EVE_DMEM_BASE + 0x8000))
                    {
                        section.addr = section.addr - SOC_EVE_DMEM_BASE;
                        section.addr = MPU_EVE3_DMEM_BASE + section.addr;
                    }
                    // WMEM
                    else if (section.addr >= SOC_EVE_WBUF_BASE &&
                             section.addr < (SOC_EVE_WBUF_BASE + 0x8000))
                    {
                        section.addr = section.addr - SOC_EVE_WBUF_BASE;
                        section.addr = MPU_EVE3_WBUF_BASE + section.addr;
                    }
                    // IBUFLA
                    else if (section.addr >= SOC_EVE_IBUF_LA_BASE &&
                             section.addr < (SOC_EVE_IBUF_LA_BASE + 0x4000))
                    {
                        section.addr = section.addr - SOC_EVE_IBUF_LA_BASE;
                        section.addr = MPU_EVE3_IBUF_LA_BASE + section.addr;
                    }
                    // IBUFH
                    else if (section.addr >= SOC_EVE_IBUF_HA_BASE &&
                             section.addr < (SOC_EVE_IBUF_HA_BASE + 0x4000))
                    {
                        section.addr = section.addr - SOC_EVE_IBUF_HA_BASE;
                        section.addr = MPU_EVE3_IBUF_HA_BASE + section.addr;
                    }
                    // IBUFLB
                    else if (section.addr >= SOC_EVE_IBUF_LB_BASE &&
                             section.addr < (SOC_EVE_IBUF_LB_BASE + 0x4000))
                    {
                        section.addr = section.addr - SOC_EVE_IBUF_LB_BASE;
                        section.addr = MPU_EVE3_IBUF_LB_BASE + section.addr;
                    }
                    else if (section.addr >= SOC_EVE_IBUF_HB_BASE &&
                             section.addr < (SOC_EVE_IBUF_HB_BASE + 0x4000))
                    {
                        section.addr = section.addr - SOC_EVE_IBUF_HB_BASE;
                        section.addr = MPU_EVE3_IBUF_HB_BASE + section.addr;
                    }
                    else
                    {
                        Vps_printf( " UTILS: EVELOADER: EVE3 - Invalid Memory section\n");
                    }

                    break;

                case EVE4_ID:
                    // DMEM
                    if (section.addr >= SOC_EVE_DMEM_BASE && section.addr <
                        (SOC_EVE_DMEM_BASE + 0x8000))
                    {
                        section.addr = section.addr - SOC_EVE_DMEM_BASE;
                        section.addr = MPU_EVE4_DMEM_BASE + section.addr;
                    }
                    // WMEM
                    else if (section.addr >= SOC_EVE_WBUF_BASE &&
                             section.addr < (SOC_EVE_WBUF_BASE + 0x8000))
                    {
                        section.addr = section.addr - SOC_EVE_WBUF_BASE;
                        section.addr = MPU_EVE4_WBUF_BASE + section.addr;
                    }
                    // IBUFLA
                    else if (section.addr >= SOC_EVE_IBUF_LA_BASE &&
                             section.addr < (SOC_EVE_IBUF_LA_BASE + 0x4000))
                    {
                        section.addr = section.addr - SOC_EVE_IBUF_LA_BASE;
                        section.addr = MPU_EVE4_IBUF_LA_BASE + section.addr;
                    }
                    // IBUFH
                    else if (section.addr >= SOC_EVE_IBUF_HA_BASE &&
                             section.addr < (SOC_EVE_IBUF_HA_BASE + 0x4000))
                    {
                        section.addr = section.addr - SOC_EVE_IBUF_HA_BASE;
                        section.addr = MPU_EVE4_IBUF_HA_BASE + section.addr;
                    }
                    // IBUFLB
                    else if (section.addr >= SOC_EVE_IBUF_LB_BASE &&
                             section.addr < (SOC_EVE_IBUF_LB_BASE + 0x4000))
                    {
                        section.addr = section.addr - SOC_EVE_IBUF_LB_BASE;
                        section.addr = MPU_EVE4_IBUF_LB_BASE + section.addr;
                    }
                    else if (section.addr >= SOC_EVE_IBUF_HB_BASE &&
                             section.addr < (SOC_EVE_IBUF_HB_BASE + 0x4000))
                    {
                        section.addr = section.addr - SOC_EVE_IBUF_HB_BASE;
                        section.addr = MPU_EVE4_IBUF_HB_BASE + section.addr;
                    }
                    else
                    {
                        Vps_printf( " UTILS: EVELOADER: EVE4 - Invalid Memory section\n");
                    }
                    break;
            }
        }
        memcpy((void *) section.addr, curAddr, section.size);
        curAddr += section.size;

    }

    return SYSTEM_LINK_STATUS_SOK;
}
/**
 * \brief           BootCore function stores the CPU entry location into global
 *                  pointer
 *
 * \param[in]    entry - CPU Entry location
 * \param[in] entryPoint - CPU ID
 *
 * \return   none
 */

void BootCore(UInt32 entry, UInt32 CoreID)
{
    switch (CoreID)
    {
        case EVE1_ID:
            /* EVE1*/
            Vps_printf( "\n UTILS: EVELOADER:  EVE1 image load completed \n");
            entryPoint_EVE1 = entry;
            break;

        case EVE2_ID:
            /* EVE2*/
            Vps_printf( "\n UTILS: EVELOADER:  EVE2 image load completed \n");
            entryPoint_EVE2 = entry;
            break;

        case EVE3_ID:
            /* EVE3*/
            Vps_printf( "\n UTILS: EVELOADER:  EVE3 image load completed \n");
            entryPoint_EVE3 = entry;
            break;

        case EVE4_ID:
            /* EVE4*/
            Vps_printf( "\n UTILS: EVELOADER:  EVE4 image load completed \n");
            entryPoint_EVE4 = entry;
            break;
    }
}

/***************************** End Of File ***********************************/
