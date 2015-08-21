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
 * \file utils_cbuf_ocmc.c
 *
 * \brief Utility functions for allocating on chip memory as circular buffers
 * Used for subframe capture and processing
 *
 * \version 0.0 (July 2014) : [VT] First version
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */

#include <src/utils_common/include/utils_cbuf_ocmc.h>

/**
 *******************************************************************************
 *
 * \brief Initialize and configure OCMC for allocation as CBUF
 *
 * \param  ocmcInstId       [IN] OCMC instance ID
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 Utils_cbufOcmcInit(Utils_OcmcInstanceId ocmcInstId)
{
    uint32_t baseAddr;
    ocmc_ret_type_t retval;

    //Get base address of OCMC based on instance
    baseAddr = OCMC_Inst_AddrGet((ocmc_inst_t)ocmcInstId);

    //Soft reset on the OCMC
    OCMCSoftReset(baseAddr);

    //Initialise OCMC region as circula buffer
    retval = OCMC_CBUF_Heap_Init(baseAddr);
    UTILS_assert(retval == e_ocmc_success);

    OCMCModeSet(baseAddr, NO_IDLE);

    /*Configure OCMC to Non ECC Code access mode*/
    OCMCEccConfig(baseAddr, OCMC_NON_ECC_DATA_ACCESS, 0, 0, 0);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Allocate a circular buffer from OCMC region
 *
 * \param  ocmcInstId       [IN] OCMC instance ID
 * \param  bpp              [IN] Bits per pixel, this is based on image format
 * \param  width            [IN] Image width
 * \param  height           [IN] Image height
 * \param  numLinesPerSlice [IN] number of lines per subframe
 * \param  numSlicesPerCbuf [IN] number of subframes per circular buffer
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Ptr Utils_cbufOcmcAlloc(Utils_OcmcInstanceId ocmcInstId,
    UInt32 bpp,UInt32 width, UInt32 height, UInt32 numLinesPerSlice, UInt32 numSlicesPerCbuf)
{
    Ptr virtStartAddr = NULL;
    vbuf_setup_image_params_auto_addr_t iParam;
    ocmc_ret_type_t err;

    // Initialise structure used to setup a VBUF config using Image parameters
    iParam.bytes_per_pixel = bpp;
    iParam.image_height    = height;
    iParam.image_width     = width;
    iParam.lines_per_slice = numLinesPerSlice;
    iParam.num_slices      = numSlicesPerCbuf;

    /* Allocate the circular buffer */
    virtStartAddr = (Ptr)(OCMC_CBUF_Heap_Alloc((ocmc_inst_t)ocmcInstId, &iParam, &err));
    UTILS_assert(virtStartAddr != NULL);

    return virtStartAddr;
}

/**
 *******************************************************************************
 *
 * \brief Initialize and configure OCMC for allocation as CBUF
 *
 * \param  ocmcInstId   [IN] OCMC instance ID
 * \param  addr         [IN] OCMC address to be freed
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 Utils_cbufOcmcFree(Utils_OcmcInstanceId ocmcInstId, Ptr addr)
{
    ocmc_ret_type_t retval;

    retval = OCMC_CBUF_Heap_Free((ocmc_inst_t)ocmcInstId, (ocmc_addr_t)addr);
    UTILS_assert(retval == e_ocmc_success);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Deinitialize the OCMC region specified
 *
 * \param  ocmcInstId       [IN] OCMC instance ID
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 Utils_cbufOcmcDeInit(Utils_OcmcInstanceId ocmcInstId)
{
    ocmc_ret_type_t retval;

    // disable the CBUF mode
    retval = OCMC_CBUF_Deinit((ocmc_inst_t)ocmcInstId);
    UTILS_assert(retval == e_ocmc_success);

    return SYSTEM_LINK_STATUS_SOK;
}
