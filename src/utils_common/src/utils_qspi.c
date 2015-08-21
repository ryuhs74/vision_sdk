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
 * \file utils_uart.c
 *
 * \brief  This file has the implementataion for QSPI
 *
 * \version 0.0 (Dec 2013) : [SS] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */

#include <xdc/std.h>
#include <string.h>
#include "qspi.h"
#include "qspi_flash.h"
#include "hw_qspi.h"
#include "hw_types.h"
#include "soc_defines.h"
#include "platform.h"

#include <src/utils_common/include/utils_qspi.h>


/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \brief Macro to enable/disable the Memory mapped mode with given chip select
 *        0: Data read from the Flash in cfg port mode
 *        1: Memory mapped mode with given chip select
 *******************************************************************************
 */
#define _WRITE_MM_MODE_           (0)

/**
 *******************************************************************************
 * \brief MAX size in bytes for a single QSPI read or write
 *******************************************************************************
 */
#define QSPI_READ_WRITE_SIZE      (256)


/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Initializes the QSPI and sets the GIO handles for the Tx and Rx
 *
 * \return  None
 *
 *******************************************************************************
 */

Void System_qspiInit(void)
{
    uint32_t          device_Id;
    qspi_DeviceType_e DeviceType = DEVICE_TYPE_QSPI4;

    Vps_printf(" QSPI Init Started \n");

    /* Configure PADs. */
    PlatformQSPISetPinMux();

    QSPI_Initialize(DeviceType);

    /* Data read from the Flash in cfg port mode */
    QSPISetMAddrSpace(SOC_QSPI_ADDRSP0_BASE,
                      QSPI_SPI_SWITCH_REG_MMPT_S_SEL_CFG_PORT,
                      QSPI_MMR);

    device_Id = QSPI_GetDeviceId();

    /* Change to Memory mapped mode with given chip select */
    QSPISetMAddrSpace(SOC_QSPI_ADDRSP0_BASE,
                      QSPI_SPI_SWITCH_REG_MMPT_S_SEL_MM_PORT,
                      QSPI_CS0);

    Vps_printf (" MID - %x \n", (device_Id & 0xFF));
    Vps_printf (" DID - %x \n", (device_Id & 0xFF0000) >> 16);

    Vps_printf(" QSPI Init Completed Sucessfully \n");
}

/**
 *******************************************************************************
 *
 * \brief Read one or several sectors from the QSPI Memory described
 *        in the Device Descriptor.
 *        Note: API read from QSPI in sector vise & one sector size is 256
 *        bytes. So, you need to allocate the destination buffer size in
 *        multiple of QSPI_READ_WRITE_SIZE = 256 bytes
 *
 * \param dstAddr       [OUT] Destination address into
 *                            which the data to be read
 * \param srcOffsetAddr [IN]  Address Offset of QSPI from
 *                            which the data to be read
 * \param length        [IN]  Size of the data to be read in bytes
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 System_qspiReadSector(UInt32 dstAddr, UInt32 srcOffsetAddr, Int32 length)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    /* Check if the  Dst buffer length a multiple of 256,
       This is done becuase QSPI_ReadSectors API read
       from QSPI in sector vise & one sector size is 256 bytes */

    if (0U != (length % QSPI_READ_WRITE_SIZE))
    {
        Vps_printf(" SYSTEM: QSPI Read - Dst buffer length must be a multiple of %d bytes! \n",
                     QSPI_READ_WRITE_SIZE);
        status = SYSTEM_LINK_STATUS_EFAIL;
        return (status);
    }

    Vps_printf(" QSPI Read Started, please wait! \n");

    while (length > 0)
    {
        if (_WRITE_MM_MODE_)
        {
            QSPISetMAddrSpace(SOC_QSPI_ADDRSP0_BASE,
                              QSPI_SPI_SWITCH_REG_MMPT_S_SEL_MM_PORT,
                              QSPI_CS0);
            QSPI_ReadSectors((void *) dstAddr, (void *) &srcOffsetAddr,
                              QSPI_READ_WRITE_SIZE);
        }
        else
        {
            QSPISetMAddrSpace(SOC_QSPI_ADDRSP0_BASE,
                              QSPI_SPI_SWITCH_REG_MMPT_S_SEL_CFG_PORT,
                              QSPI_MMR);

            QSPI_ReadCfgMode(dstAddr, srcOffsetAddr, QSPI_READ_WRITE_SIZE/4);
        }
        length -= QSPI_READ_WRITE_SIZE;
        dstAddr += QSPI_READ_WRITE_SIZE;
        srcOffsetAddr += QSPI_READ_WRITE_SIZE;
    }

    /* Change to Memory mapped mode with given chip select */
    QSPISetMAddrSpace(SOC_QSPI_ADDRSP0_BASE,
                      QSPI_SPI_SWITCH_REG_MMPT_S_SEL_MM_PORT,
                      QSPI_CS0);

    Vps_printf(" QSPI Read Completed Sucessfully \n");

    return (status);
}

/**
 *******************************************************************************
 *
 * \brief Writes one or several sectors to the QSPI Memory described
 *        in the Device Descriptor.
 *        Note: QSPI erase API erases at block level which is of size
 *        QSPIFLASH_BLOCKSIZE = 64k. So, the size of buffer/memory reserved
 *        in QSPI should be in multiple of QSPIFLASH_BLOCKSIZE = 64k bytes
 *
 * \param srcAddr       [IN]  Source address from which the data to be
 *                            written into the QSPI flash
 * \param dstOffsetAddr [OUT  Address Offset of QSPI from
 *                            which the data to be written.
 *                            dstOffsetAddr Should be 64KB aligned.
 *                            size of buffer/memory reserved in QSPI should be
 *                            in multiple of QSPIFLASH_BLOCKSIZE = 64k bytes
 * \param length        [IN]  Size of the data to be written in bytes
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 System_qspiWriteSector(UInt32 dstOffsetAddr, UInt32 srcAddr, Int32 length)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    /* Check if the dstOffsetAddr is 64KB aligned & QSPI buffer length
       a multiple of 64k, This is done becuase EraseSector API erases
       at block level which is of size QSPIFLASH_BLOCKSIZE = 64k */
    if ((0U != (dstOffsetAddr % QSPIFLASH_BLOCKSIZE)) ||
        (0U != (length % QSPIFLASH_BLOCKSIZE)))
    {
        Vps_printf(" SYSTEM: QSPI Write - Dst address must be aligned to %d bytes! &"
                   " QSPI buffer length must be a multiple of %d bytes! \n",
                     QSPIFLASH_BLOCKSIZE, QSPIFLASH_BLOCKSIZE);
        status = SYSTEM_LINK_STATUS_EFAIL;
        return (status);
    }

    Vps_printf(" QSPI Write Started, please wait! \n");

    /* Erase the block before any write operation */
    System_qspiEraseSector (dstOffsetAddr, length);

    while (length > 0)
    {
        if (_WRITE_MM_MODE_)
        {
            QSPISetMAddrSpace(SOC_QSPI_ADDRSP0_BASE,
                              QSPI_SPI_SWITCH_REG_MMPT_S_SEL_MM_PORT,
                              QSPI_CS0);
            QSPI_WriteEnable();
            QSPI_WriteSectors(dstOffsetAddr, srcAddr, QSPI_READ_WRITE_SIZE);
        }
        else
        {
            QSPISetMAddrSpace(SOC_QSPI_ADDRSP0_BASE,
                              QSPI_SPI_SWITCH_REG_MMPT_S_SEL_CFG_PORT,
                              QSPI_MMR);
            QSPI_WriteEnable();
            QSPI_WriteCfgMode(dstOffsetAddr, srcAddr, QSPI_READ_WRITE_SIZE/4);
        }
        length -= QSPI_READ_WRITE_SIZE;
        srcAddr += QSPI_READ_WRITE_SIZE;
        dstOffsetAddr += QSPI_READ_WRITE_SIZE;
    }

    /* Change to Memory mapped mode with given chip select */
    QSPISetMAddrSpace(SOC_QSPI_ADDRSP0_BASE,
                      QSPI_SPI_SWITCH_REG_MMPT_S_SEL_MM_PORT,
                      QSPI_CS0);

    Vps_printf(" QSPI Write Completed Sucessfully \n");

    return (status);
}

/**
 *******************************************************************************
 *
 * \brief Function to erase the whole QSPI flash memory.
 *
 * \return  None
 *
 *******************************************************************************
 */
Void System_qspiEraseFull(void)
{

    Vps_printf(" QSPI Erase Full Started, Will take a couple of seconds \n");
    if(!System_isFastBootEnabled())
    {
        /* If this is not present, UART prints don't happen until after the
         * QSPI operations are completed
         */
        BspOsal_sleep(10);
    }

    QSPISetMAddrSpace(SOC_QSPI_ADDRSP0_BASE,
                      QSPI_SPI_SWITCH_REG_MMPT_S_SEL_CFG_PORT,
                      QSPI_MMR);

    QSPI_WriteEnable();
    QSPI_FlashFullErase();

    Vps_printf(" QSPI Erase Full Completed Sucessfully \n");
}

/**
 *******************************************************************************
 *
 * \brief API to erase a block or mutiple blocks (sector) from the QSPI flash.
 *
 * \param dstOffsetAddr [IN]  Address Offset of QSPI from
 *                            which the data to be erased.
 *                            dstOffsetAddr Should be 64KB aligned.
 * \param length        [IN]  Size of the data to be erased in bytes
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 System_qspiEraseSector(UInt32 dstOffsetAddr, Int32 length)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    UInt32 blkno;

    /* Check if the dstOffsetAddr is 64KB aligned & QSPI buffer length
       a multiple of 64k, This is done becuase EraseSector API erases
       at block level which is of size QSPIFLASH_BLOCKSIZE = 64k */
    if ((0U != (dstOffsetAddr % QSPIFLASH_BLOCKSIZE)) ||
        (0U != (length % QSPIFLASH_BLOCKSIZE)))
    {
        Vps_printf(" SYSTEM: QSPI Erase - Dst address must be aligned to %d bytes! &"
                   " QSPI buffer length must be a multiple of %d bytes! \n",
                     QSPIFLASH_BLOCKSIZE, QSPIFLASH_BLOCKSIZE);
        status = SYSTEM_LINK_STATUS_EFAIL;
        return (status);
    }

    Vps_printf(" QSPI Erase Block Started, please wait! \n");
    if(!System_isFastBootEnabled())
    {
        /* If this is not present, UART prints don't happen until after the
         * QSPI operations are completed
         */
        BspOsal_sleep(10);
    }

    QSPISetMAddrSpace(SOC_QSPI_ADDRSP0_BASE,
                      QSPI_SPI_SWITCH_REG_MMPT_S_SEL_CFG_PORT,
                      QSPI_MMR);

    blkno     = dstOffsetAddr / QSPIFLASH_BLOCKSIZE;
    /* Size of one block QSPIFLASH_BLOCKSIZE = 64k */
    while (length > 0)
    {
        QSPI_WriteEnable();
        QSPI_FlashBlockErase(blkno++);
        length -= QSPIFLASH_BLOCKSIZE;
    }

    Vps_printf(" QSPI Erase Block Completed Sucessfully \n");

    return (status);
}



/* Nothing past this point */
