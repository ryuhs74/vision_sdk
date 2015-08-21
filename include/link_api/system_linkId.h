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
 *  \ingroup SYSTEM_LINK_API
 *  \defgroup SYSTEM_LINK_ID  System Link ID's
 *
 *  The unique 32-bit Link ID for the links present in the system are defined
 *  in this module
 *
 *  @{
*/

/**
 *******************************************************************************
 *
 *  \file system_linkId.h
 *  \brief  System Link ID's
 *
 *******************************************************************************
*/

#ifndef _SYSTEM_LINK_ID_H_
#define _SYSTEM_LINK_ID_H_

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Max possible Link ID
 *
 *******************************************************************************
 */
#define SYSTEM_LINK_ID_MAX                  (128)

/**
 *******************************************************************************
 *
 * \brief Invalid Link ID
 *
 *******************************************************************************
 */
#define SYSTEM_LINK_ID_INVALID              (0xFFFFFFFF)


/* @{ */

/**
 *******************************************************************************
 *
 * \brief IPU1 Core 0 Proc ID
 *
 *******************************************************************************
*/
#define SYSTEM_PROC_IPU1_0      (0)

/**
 *******************************************************************************
 *
 * \brief IPU Core 1 Proc ID
 *
 *******************************************************************************
*/
#define SYSTEM_PROC_IPU1_1      (1)

/**
 *******************************************************************************
 *
 * \brief MPU Proc ID
 *
 *******************************************************************************
*/
#define SYSTEM_PROC_A15_0       (2)

/**
 *******************************************************************************
 *
 * \brief DSP Proc ID
 *
 *******************************************************************************
*/
#define SYSTEM_PROC_DSP1        (3)

/**
 *******************************************************************************
 *
 * \brief DSP Proc ID
 *
 *******************************************************************************
*/
#define SYSTEM_PROC_DSP2        (4)

/**
 *******************************************************************************
 *
 * \brief EVE Proc ID
 *
 *******************************************************************************
*/
#define SYSTEM_PROC_EVE1        (5)

/**
 *******************************************************************************
 *
 * \brief EVE Proc ID
 *
 *******************************************************************************
*/
#define SYSTEM_PROC_EVE2        (6)

/**
 *******************************************************************************
 *
 * \brief EVE Proc ID
 *
 *******************************************************************************
*/
#define SYSTEM_PROC_EVE3        (7)

/**
 *******************************************************************************
 *
 * \brief EVE Proc ID
 *
 *******************************************************************************
*/
#define SYSTEM_PROC_EVE4        (8)

/**
 *******************************************************************************
 *
 * \brief Max supported processors
 *
 *******************************************************************************
*/
#define SYSTEM_PROC_MAX         (9)

/**
 *******************************************************************************
 *
 * \brief Invalid proc Id, if received indicates some corruption
 *
 *******************************************************************************
*/
#define SYSTEM_PROC_INVALID     (0xFFFF)

/* @} */

/**
 *******************************************************************************
 *
 * \brief Create link id which indicates the link & processor in which it
 *        resides
 *
 *******************************************************************************
*/
#define SYSTEM_MAKE_LINK_ID(p, x) ((((p) & 0xF) << 8) | ((x) & 0x000000FF))

/**
 *******************************************************************************
 *
 * \brief Get the link id - strip off proc id
 *
 *******************************************************************************
*/
#define SYSTEM_GET_LINK_ID(x)     ((x) & 0x000000FF)

/**
 *******************************************************************************
 *
 * \brief Get the proc id - strip off link id
 *
 *******************************************************************************
*/
#define SYSTEM_GET_PROC_ID(x)     (((x) & ~0xFFFFF0FF)>> 8)

/**
 *******************************************************************************
 *
 * \brief Set route bit - bit 32 of LinkId is used as route bit
 *        This is used only when message needs to be routed through
 *        some other core.
 *
 *******************************************************************************
*/
#define SYSTEM_LINK_ID_SET_ROUTE_BIT(x)    ((x) |= (1u << 0x1F))

/**
 *******************************************************************************
 *
 * \brief Clear route bit
 *
 *******************************************************************************
*/
#define SYSTEM_LINK_ID_CLEAR_ROUTE_BIT(x)  ((x) &= ~((1u) << 0x1F))

/**
 *******************************************************************************
 *
 * \brief Test route bit
 *
 *******************************************************************************
*/
#define SYSTEM_LINK_ID_TEST_ROUTE_BIT_TRUE(x)  ((x) & (1u << 0x1F))


/**
 *******************************************************************************
 *
 * \brief Type of payload used with notify payload: Payload is Link ID
 *
 *******************************************************************************
*/
#define SYSTEM_LINK_ID_NOTIFY_TYPE_LINK_ID         (0)

/**
 *******************************************************************************
 *
 * \brief Type of payload used with notify payload: Payload is a message
 *
 *******************************************************************************
*/
#define SYSTEM_LINK_ID_NOTIFY_TYPE_MSG             (1)

/**
 *******************************************************************************
 *
 * \brief Type of payload used with notify payload: Payload is a message ACK
 *
 *******************************************************************************
*/
#define SYSTEM_LINK_ID_NOTIFY_TYPE_MSG_ACK         (2)

/**
 *******************************************************************************
 *
 * \brief Set route bit - bit 32 of LinkId is used as route bit
 *        This is used only when message needs to be routed through
 *        some other core.
 *
 *******************************************************************************
*/
#define SYSTEM_LINK_ID_MAKE_NOTIFY_TYPE(dest_p, src_p, t)    ( SYSTEM_MAKE_LINK_ID(dest_p, src_p) | ( ( (t) & 0xF ) << 12) )

/**
 *******************************************************************************
 *
 * \brief Test route bit
 *
 *******************************************************************************
*/
#define SYSTEM_LINK_ID_GET_NOTIFY_TYPE(x)    (((x) & (0x0000F000)) >> 12)


/* @{ */

#define IPU1_0_LINK(x)          SYSTEM_MAKE_LINK_ID(SYSTEM_PROC_IPU1_0 , (x))

/**
 *******************************************************************************
 *
 * \brief IPU1_1 System Link - used for non-link specific proc level
 *        communication
 *
 *******************************************************************************
*/
#define IPU1_1_LINK(x)          SYSTEM_MAKE_LINK_ID(SYSTEM_PROC_IPU1_1 , (x))

/**
 *******************************************************************************
 *
 * \brief DSP1 System Link - used for non-link specific proc level communication
 *
 *******************************************************************************
*/
#define DSP1_LINK(x)            SYSTEM_MAKE_LINK_ID(SYSTEM_PROC_DSP1   , (x))

/**
 *******************************************************************************
 *
 * \brief DSP2 System Link - used for non-link specific proc level communication
 *
 *******************************************************************************
*/
#define DSP2_LINK(x)            SYSTEM_MAKE_LINK_ID(SYSTEM_PROC_DSP2   , (x))

/**
 *******************************************************************************
 *
 * \brief EVE1 System Link - used for non-link specific proc level communication
 *
 *******************************************************************************
*/
#define EVE1_LINK(x)            SYSTEM_MAKE_LINK_ID(SYSTEM_PROC_EVE1   , (x))

/**
 *******************************************************************************
 *
 * \brief EVE2 System Link - used for non-link specific proc level communication
 *
 *******************************************************************************
*/
#define EVE2_LINK(x)            SYSTEM_MAKE_LINK_ID(SYSTEM_PROC_EVE2   , (x))

/**
 *******************************************************************************
 *
 * \brief EVE3 System Link - used for non-link specific proc level communication
 *
 *******************************************************************************
*/
#define EVE3_LINK(x)            SYSTEM_MAKE_LINK_ID(SYSTEM_PROC_EVE3   , (x))

/**
 *******************************************************************************
 *
 * \brief EVE4 System Link - used for non-link specific proc level communication
 *
 *******************************************************************************
*/
#define EVE4_LINK(x)            SYSTEM_MAKE_LINK_ID(SYSTEM_PROC_EVE4   , (x))

/**
 *******************************************************************************
 *
 * \brief A15 Link - used for non-link specific proc level communication
 *
 *******************************************************************************
*/
#define A15_0_LINK(x)            SYSTEM_MAKE_LINK_ID(SYSTEM_PROC_A15_0 ,(x))


/* @} */

/* @{ */

/**
 *******************************************************************************
 *
 * \brief Link ID for the generic processor link task
 *
 *******************************************************************************
 */
#define SYSTEM_LINK_ID_PROCK_LINK_ID        (SYSTEM_LINK_ID_MAX-1)

/**
 *******************************************************************************
 *
 * \brief IPU1_0 System Link - used for non-link specific proc level
 *        communication
 *
 *******************************************************************************
 */
#define SYSTEM_LINK_ID_IPU1_0               IPU1_0_LINK(SYSTEM_LINK_ID_PROCK_LINK_ID)

/**
 *******************************************************************************
 *
 * \brief IPU1_1 System Link - used for non-link specific proc level
 *        communication
 *
 *******************************************************************************
 */
#define SYSTEM_LINK_ID_IPU1_1               IPU1_1_LINK(SYSTEM_LINK_ID_PROCK_LINK_ID)

/**
 *******************************************************************************
 *
 * \brief DSP1 System Link - used for non-link specific proc level
 *        communication
 *
 *******************************************************************************
 */
#define SYSTEM_LINK_ID_DSP1                 DSP1_LINK(SYSTEM_LINK_ID_PROCK_LINK_ID)

/**
 *******************************************************************************
 *
 * \brief DSP2 System Link - used for non-link specific proc level
 *        communication
 *
 *******************************************************************************
 */
#define SYSTEM_LINK_ID_DSP2                 DSP2_LINK(SYSTEM_LINK_ID_PROCK_LINK_ID)

/**
 *******************************************************************************
 *
 * \brief EVE1 System Link - used for non-link specific proc level
 *        communication
 *
 *******************************************************************************
 */
#define SYSTEM_LINK_ID_EVE1                 EVE1_LINK(SYSTEM_LINK_ID_PROCK_LINK_ID)

/**
 *******************************************************************************
 *
 * \brief EVE2 System Link - used for non-link specific proc level
 *        communication
 *
 *******************************************************************************
 */
#define SYSTEM_LINK_ID_EVE2                 EVE2_LINK(SYSTEM_LINK_ID_PROCK_LINK_ID)

/**
 *******************************************************************************
 *
 * \brief EVE3 System Link - used for non-link specific proc level
 *        communication
 *
 *******************************************************************************
 */
#define SYSTEM_LINK_ID_EVE3                 EVE3_LINK(SYSTEM_LINK_ID_PROCK_LINK_ID)

/**
 *******************************************************************************
 *
 * \brief EVE4 System Link - used for non-link specific proc level
 *        communication
 *
 *******************************************************************************
 */
#define SYSTEM_LINK_ID_EVE4                 EVE4_LINK(SYSTEM_LINK_ID_PROCK_LINK_ID)

/**
 *******************************************************************************
 *
 * \brief A15_0 System Link - used for non-link specific proc level
 *        communication
 *
 *******************************************************************************
 */
#define SYSTEM_LINK_ID_A15_0                A15_0_LINK(SYSTEM_LINK_ID_PROCK_LINK_ID)

/* @} */

/*******************************************************************************
 *  Enums
 *******************************************************************************
 */

 /**
 *******************************************************************************
 *
 *  \brief Link Id for links that can reside on any processor
 *
 *         These identifiers are not valid as-is. They need to be combined with
 *         processor Id in order to make a valid link ID.
 *
 *         The macros below can be used to create the processor specific
 *         link Id.
 *          - EVEx_LINK(common link id)
 *          - DSPx_LINK(common link id)
 *          - A15_x_LINK(common link id)
 *          - IPUx_y_LINK(common link id)
 *
 *         NOTE: Even though a link ID is listed here, for some core's
 *               the actually number of instances created will vary.
 *
 *               Ex, for EVE the number of link instances created is
 *                     IPC IN  = 1 instance
 *                     IPC OUT = 1 instance
 *                     ALG     = 1 instance
 *
 *         TO increase number of links on a given processor
 *               - Add more enum IDs for that link here
 *               - AND increase the number of link instances in file
 *               /vision_sdk/src/links_common/[link]/[link]Link_cfg.h
 *                 OR
 *               /vision_sdk/src/links_common/[link]/[link]Link_priv.h
 *
 *               - Modify "#define [link]_LINK_OBJ_MAX" to match number of enums
 *                 in this file
 *
 *         The processor specific link ID should be used with all link APIs
 *
 *******************************************************************************
 */
typedef enum
{
    SYSTEM_LINK_ID_IPC_OUT_0 = 0,
    /**< IPC Output Link Id  - used to xfr data across
     *   processors */

    SYSTEM_LINK_ID_IPC_OUT_1,
    /**< IPC Output Link Id  - used to xfr data across
     *   processors */

    SYSTEM_LINK_ID_IPC_OUT_2,
    /**< IPC Output Link Id  - used to xfr data across
     *   processors */

    SYSTEM_LINK_ID_IPC_OUT_3,
    /**< IPC Output Link Id  - used to xfr data across
     *   processors */

    SYSTEM_LINK_ID_IPC_OUT_4,
    /**< IPC Output Link Id  - used to xfr data across
     *   processors */

    SYSTEM_LINK_ID_IPC_OUT_5,
    /**< IPC Output Link Id  - used to xfr data across
     *   processors */

    SYSTEM_LINK_ID_IPC_OUT_6,
    /**< IPC Output Link Id  - used to xfr data across
     *   processors */

    SYSTEM_LINK_ID_IPC_OUT_7,
    /**< IPC Output Link Id  - used to xfr data across
     *   processors */

    SYSTEM_LINK_ID_IPC_IN_0,
    /**< IPC Input Link Id - used to xfr data across
      *  processors */

    SYSTEM_LINK_ID_IPC_IN_1,
    /**< IPC Input Link Id - used to xfr data across
     *   processors */

    SYSTEM_LINK_ID_IPC_IN_2,
    /**< IPC Input Link Id - used to xfr data across
     *   processors */

    SYSTEM_LINK_ID_IPC_IN_3,
    /**< IPC Input Link Id - used to xfr data across
     *   processors */

    SYSTEM_LINK_ID_IPC_IN_4,
    /**< IPC Input Link Id - used to xfr data across
     *   processors */

    SYSTEM_LINK_ID_IPC_IN_5,
    /**< IPC Input Link Id - used to xfr data across
     *   processors */

    SYSTEM_LINK_ID_IPC_IN_6,
    /**< IPC Input Link Id - used to xfr data across
     *   processors */

    SYSTEM_LINK_ID_IPC_IN_7,
    /**< IPC Input Link Id - used to xfr data across
     *   processors */

    SYSTEM_LINK_ID_NULL_0,
    /**< Null Link - Can be used as a tap point to verify various
     *   sub-chains. Doesnt do any processing : 4*/

    SYSTEM_LINK_ID_NULL_1,
    /**< Null Link - Can be used as a tap point to verify various
     *   sub-chains. Doesnt do any processing : 5*/

    SYSTEM_LINK_ID_GRPX_SRC_0,
    /**< Null source link - can be used as a source link providing
     *   dummy data */

    SYSTEM_LINK_ID_GRPX_SRC_1,
    /**< Null source link - can be used as a source link providing
     *   dummy data */

    SYSTEM_LINK_ID_DUP_0,
    /**< Dup Link - Duplicate frames and provides multiple outputs */

    SYSTEM_LINK_ID_DUP_1,
    /**< Dup Link - Duplicate frames and provides multiple outputs */

    SYSTEM_LINK_ID_DUP_2,
    /**< Dup Link - Duplicate frames and provides multiple outputs */

    SYSTEM_LINK_ID_DUP_3,
    /**< Dup Link - Duplicate frames and provides multiple outputs */

    SYSTEM_LINK_ID_DUP_4,
    /**< Dup Link - Duplicate frames and provides multiple outputs */

    SYSTEM_LINK_ID_GATE_0,
    /**< Gate Link - Acts a on/off switch and allows partial data
         flow to exist */

    SYSTEM_LINK_ID_GATE_1,
    /**< Gate Link - Acts a on/off switch and allows partial data
         flow to exist */

    SYSTEM_LINK_ID_GATE_2,
    /**< Gate Link - Acts a on/off switch and allows partial data
         flow to exist */

    SYSTEM_LINK_ID_GATE_3,
    /**< Gate Link - Acts a on/off switch and allows partial data
         flow to exist */

    SYSTEM_LINK_ID_SYNC_0,
    /**< Sync Link - creates a composite frame by composing multiple incoming
         Frames */

    SYSTEM_LINK_ID_SYNC_1,
    /**< Sync Link - creates a composite frame by composing multiple incoming
         Frames */

    SYSTEM_LINK_ID_SYNC_2,
    /**< Sync Link - creates a composite frame by composing multiple incoming
         Frames */

    SYSTEM_LINK_ID_SYNC_3,
    /**< Sync Link - creates a composite frame by composing multiple incoming
         Frames */

    SYSTEM_LINK_ID_MERGE_0,
    /**< Merge Link - Merge different input queue frames & provide them as
     *   single output source. Channel numbering is sequential wrt to
     *   input queues */

    SYSTEM_LINK_ID_MERGE_1,
    /**< Merge Link - Merge different input queue frames & provide them as
     *   single output source. Channel numbering is sequential wrt to
     *   input queues */

    SYSTEM_LINK_ID_MERGE_2,
    /**< Merge Link - Merge different input queue frames & provide them as
     *   single output source. Channel numbering is sequential wrt to
     *   input queues */

    SYSTEM_LINK_ID_MERGE_3,
    /**< Merge Link - Merge different input queue frames & provide them as
     *   single output source. Channel numbering is sequential wrt to
     *   input queues */

    SYSTEM_LINK_ID_MERGE_4,
    /**< Merge Link - Merge different input queue frames & provide them as
     *   single output source. Channel numbering is sequential wrt to
     *   input queues */


    SYSTEM_LINK_ID_SELECT_0,
    /**< Select Link enables configurable mapping of specific channels
     *   to be sent out in multiple queues */

    SYSTEM_LINK_ID_SELECT_1,
    /**< Select Link enables configurable mapping of specific channels
     *   to be sent out in multiple queues */

    SYSTEM_LINK_ID_ALG_0,
    /**< Algorithm link Id */

    SYSTEM_LINK_ID_ALG_1,
    /**< Algorithm link Id */

    SYSTEM_LINK_ID_ALG_2,
    /**< Algorithm link Id */

    SYSTEM_LINK_ID_ALG_3,
    /**< Algorithm link Id */

    SYSTEM_LINK_ID_ALG_4,
    /**< Algorithm link Id */

    SYSTEM_LINK_ID_ALG_5,
    /**< Algorithm link Id */

    SYSTEM_LINK_ID_ALG_6,
    /**< Algorithm link Id */

    SYSTEM_LINK_ID_ALG_7,
    /**< Algorithm link Id */

    SYSTEM_LINK_ID_NULL_SRC_0,
    /**< Null source link - can be used as a source link providing
     *   dummy data */

    SYSTEM_LINK_ID_AVB_RX,
    /**< AvbRx link Id - can be used for supported CPUs only.
     *
     *   Supported CPUs: ipu1_0, ipu1_1, a15_0 in TDA2xx
     *   Supported CPUs: ipu1_0, ipu1_1        in TDA3xx
     */

    SYSTEM_LINK_COMMON_LINKS_MAX_ID
    /**< Common Links - Max Id */
} SYSTEM_LINK_IDS_COMMON;

/**
 *******************************************************************************
 *
 *  \brief Links Ids specific to IPU1_0.
 *
 *         These are valid Link Ids available in IPU1_0.
 *         Few of the links create multiple instances with unique
 *         identifier - like xxx_0, xxx_1
 *
 *******************************************************************************
 */
typedef  enum
{
    SYSTEM_LINK_ID_CAPTURE_0           =
                                IPU1_0_LINK(SYSTEM_LINK_COMMON_LINKS_MAX_ID+1),
    /**< Capture link. Present in IPU1_0 */

    SYSTEM_LINK_ID_CAPTURE_1           =
                                IPU1_0_LINK(SYSTEM_LINK_COMMON_LINKS_MAX_ID+2),
    /**< Capture link. Present in IPU1_0 */

    SYSTEM_LINK_ID_CAPTURE             = SYSTEM_LINK_ID_CAPTURE_0,
    /**< Capture link ID to keep the name backward compatible */

    SYSTEM_LINK_ID_DISPLAYCTRL      =
                                IPU1_0_LINK(SYSTEM_LINK_COMMON_LINKS_MAX_ID+3),
    /**< Display link Control - enables configurations of Vencs.
         Present in IPU1_0 */

    SYSTEM_LINK_ID_DISPLAY_0         =
                                IPU1_0_LINK(SYSTEM_LINK_COMMON_LINKS_MAX_ID+4),
    /**< Display link - enables one of the outputs like HDTV, SDTV etc.
         Present in IPU1_0 */

    SYSTEM_LINK_ID_DISPLAY_1         =
                                IPU1_0_LINK(SYSTEM_LINK_COMMON_LINKS_MAX_ID+5),
    /**< Display link - enables one of the outputs like HDTV, SDTV etc.
         Present in IPU1_0 */

    SYSTEM_LINK_ID_DISPLAY_2         =
                                IPU1_0_LINK(SYSTEM_LINK_COMMON_LINKS_MAX_ID+6),

    /**< Display link - enables one of the outputs like HDTV, SDTV etc.
         Present in IPU1_0 */

    SYSTEM_LINK_ID_DISPLAY_3        =
                                IPU1_0_LINK(SYSTEM_LINK_COMMON_LINKS_MAX_ID+7),
    /**< Display link - enables one of the outputs like HDTV, SDTV etc.
         Present in IPU1_0 */

    SYSTEM_LINK_ID_VPE_0            =
                                IPU1_0_LINK(SYSTEM_LINK_COMMON_LINKS_MAX_ID+8),
    /**< VPE Link Id */

    SYSTEM_LINK_ID_VPE_1            =
                                IPU1_0_LINK(SYSTEM_LINK_COMMON_LINKS_MAX_ID+9),
    /**< VPE Link Id */

    SYSTEM_LINK_ID_VPE_2            =
                                IPU1_0_LINK(SYSTEM_LINK_COMMON_LINKS_MAX_ID+10),
    /**< VPE Link Id */

    SYSTEM_LINK_ID_VPE_3            =
                                IPU1_0_LINK(SYSTEM_LINK_COMMON_LINKS_MAX_ID+11),
    /**< VPE Link Id */

    SYSTEM_LINK_ID_VENC_0           =
                                IPU1_0_LINK(SYSTEM_LINK_COMMON_LINKS_MAX_ID+12),
    /**< Video encode link Id */

    SYSTEM_LINK_ID_VDEC_0           =
                                IPU1_0_LINK(SYSTEM_LINK_COMMON_LINKS_MAX_ID+13),
    /**< Video decode link Id */

    SYSTEM_LINK_ID_ISSCAPTURE_0     =
                                IPU1_0_LINK(SYSTEM_LINK_COMMON_LINKS_MAX_ID+14),
    /**< Iss Capture link 0. Present in IPU1_0 */

    SYSTEM_LINK_ID_ISSM2MISP_0     =
                                IPU1_0_LINK(SYSTEM_LINK_COMMON_LINKS_MAX_ID+15),
    /**< Iss Capture link 0. Present in IPU1_0 */

    /**< Ultrasonic capture link Id */
    SYSTEM_LINK_ID_ISSM2MSIMCOP_0 =
                                IPU1_0_LINK(SYSTEM_LINK_COMMON_LINKS_MAX_ID+16),
    /**< Simcop link Id */

    SYSTEM_LINK_ID_APP_CTRL         =
                                IPU1_0_LINK(SYSTEM_LINK_COMMON_LINKS_MAX_ID+17),
    /**< Application specific control link Id */

    SYSTEM_LINK_ID_ULTRASONIC_CAPTURE =
                                IPU1_0_LINK(SYSTEM_LINK_COMMON_LINKS_MAX_ID+18),
    /**< Ultrasonic capture link */

    SYSTEM_LINK_ID_HCF_0 =
                                IPU1_0_LINK(SYSTEM_LINK_COMMON_LINKS_MAX_ID+19),
    /**< HCF link */

    SYSTEM_LINK_ID_SPLIT_0 =
                                IPU1_0_LINK(SYSTEM_LINK_COMMON_LINKS_MAX_ID+20),
    /**< Split Link - Split frames into different queues */

    SYSTEM_IPU1_0_LINK_ID_MAX = 0x3FFFFFFF
    /**< Max link ID for IPU1-0 specfic links */

} SYSTEM_IPU1_0_LINK_IDS;

/**
 *******************************************************************************
 *
 *  \brief Links Ids specific to A15 (Linux).
 *
 *         These are valid Link Ids available in A15 (Linux).
 *         Few of the links create multiple instances with unique
 *         identifier - like xxx_0, xxx_1
 *
 *******************************************************************************
 */
typedef  enum
{
    SYSTEM_LINK_ID_SGXDISPLAY_0     =
                                A15_0_LINK(SYSTEM_LINK_COMMON_LINKS_MAX_ID+1),
    /**< SgxDiaply link - enables DRM display along with SGX rendering,
         Present only on A15_0 */

    SYSTEM_LINK_ID_SGX3DSRV_0     =
                                A15_0_LINK(SYSTEM_LINK_COMMON_LINKS_MAX_ID+2),
    /**< Sgx3Dsrv link - For 3D SRV output creation/rendering on SGX,
         Present only on A15_0 */

    SYSTEM_LINK_ID_SRV2DINFOADAS_0 =
                                A15_0_LINK(SYSTEM_LINK_COMMON_LINKS_MAX_ID+4),
    /**< srv2D link - enables srv2D ,
         Present only on A15_0 */
    SYSTEM_LINK_ID_SRV3DINFOADAS_0 =
                                A15_0_LINK(SYSTEM_LINK_COMMON_LINKS_MAX_ID+5),
    /**< srv3D link - enables srv3D ,
         Present only on A15_0 */
    SYSTEM_LINK_ID_EP_0 =
                                A15_0_LINK(SYSTEM_LINK_COMMON_LINKS_MAX_ID+6),
    /**< endpoint link id - 0,
         Present only on A15 */
    SYSTEM_LINK_ID_EP_1 =
                                A15_0_LINK(SYSTEM_LINK_COMMON_LINKS_MAX_ID+7)
    /**< endpoint link id - 1,
         Present only on A15 */
} SYSTEM_A15_0_LINK_IDS;


#ifdef  __cplusplus
}
#endif

#endif

/*@}*/

