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
 *   \ingroup FRAMEWORK_MODULE_API
 *   \defgroup SYNC_LINK_API Sync Link API
 *
 *   Sync link is a connector link. Sync Link is particularly targeted to a set
 *   of specific use cases where there is a need for a set of video frames from
 *   multiple channels to be in sync (Frames captured at approximately at the
 *   same time)
 *
 *   @{
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file syncLink.h
 *
 * \brief Sync link API public header file.
 *
 * \version 0.0 (Aug 2013) : [NN] First version
 *
 *******************************************************************************
 */

#ifndef _SYNC_LINK_H_
#define _SYNC_LINK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <include/link_api/system.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/* @{ */
/**
 *******************************************************************************
 *
 *   \brief Sync link can optionally have a Master channel. Channel number to
 *          be provided when master channel selection is automatic by sync link.
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SYNC_LINK_MASTER_CH_AUTO (-1)

/**
 *******************************************************************************
 *
 *   \brief Number of channels supported by sync link. Currently sync link is
 *          used with capture link. Since capture link supports 8 output
 *          channels, sync also supports 8 channels
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SYNC_LINK_MAX_CHANNELS (8)

/* @} */

/**
 *******************************************************************************
 *
 *   \ingroup LINK_API_CMD
 *   \addtogroup SYNC_LINK_API_CMD Sync Link Control Commands
 *
 *   @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Link CMD: Set sync link channel parameters
 *
 *   MUST be set by user
 *
 *   \param SyncLink_ChannelParams *pPrm [IN]
 *
 *   \return SYSTEM_STATUS_SOK on success
 *
 *******************************************************************************
 */
 #define SYNC_LINK_CMD_SET_LINK_INFO               (0x6001)

/**
 *******************************************************************************
 *
 *   \brief Link CMD: Get sync link channel parameters
 *
 *   \param SyncLink_ChannelParams *pPrm [OUT]
 *
 *   \return SYSTEM_STATUS_SOK on success
 *
 *******************************************************************************
 */
 #define SYNC_LINK_CMD_GET_LINK_INFO               (0x6002)

/**
 *******************************************************************************
 *
 *   \brief Link CMD: Get sync link statistics
 *
 *   \param SyncLink_latestSyncDelta *pPrm [OUT]
 *
 *   \return SYSTEM_STATUS_SOK on success
 *
 *******************************************************************************
 */
 #define SYNC_LINK_CMD_GET_LATEST_SYNC_DELTA       (0x6003)

 /* @} */


/*******************************************************************************
 *  Data structures
 *******************************************************************************
*/

/**
 *******************************************************************************
 * \brief Sync link channel parameters.
 *
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef struct
{
    Int32    numCh;
    /**< Number of channels */

    Int32    numActiveCh;
    /**< Number of active channels */

    UInt32   channelSyncList[SYNC_LINK_MAX_CHANNELS];
    /**< list maintaining active channels
     *   TRUE means channel is active
     *   FALSE means channel is inactive */

    UInt32   syncDelta;
    /**< Delta on which to sync frames */

    UInt32   syncThreshold;
    /**< Threshold after which buffers from local queue can be dropped */
} SyncLink_ChannelParams;

/**
 *******************************************************************************
 * \brief Sync link create time parameters.
 *
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef struct
{
    System_LinkInQueParams    inQueParams;
    /**< Input queue information */

    SyncLink_ChannelParams    chParams;
    /**< Instance of sync link channel parameters */

    System_LinkOutQueParams   outQueParams;
    /**< output queue information */
} SyncLink_CreateParams;

/**
 *******************************************************************************
 * \brief Sync link delta information for channel.
 *
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32  bufferTimestamp[SYNC_LINK_MAX_CHANNELS];
    /**< timestamp of the buffer */
    UInt32  masterTimestamp;
    /**< master timestamp */
} SyncLink_LatestSyncDelta;


/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Init function for sync link. This function does the following for each
 *   sync link,
 *  - Creates a task for the link
 *  - Registers this link with the system
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 SyncLink_init();

/**
 *******************************************************************************
 *
 * \brief De-init function for sync link. This function de-registers this link
 *  from the system
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 SyncLink_deInit();

/**
 *******************************************************************************
 *
 * \brief Sync link set default parameters for create time params
 *   This function does the following
 *      - memset create params object
 *
 * \param  pPrm  [OUT]  SyncLink Create time Params
 *
 *******************************************************************************
 */
static inline void SyncLink_CreateParams_Init(SyncLink_CreateParams *pPrm)
{
    UInt32 chId;
    memset(pPrm, 0, sizeof(*pPrm));

    /*
     * Setting all channels as active.
    */
    pPrm->chParams.numCh = SYNC_LINK_MAX_CHANNELS;

    for (chId = 0; chId < pPrm->chParams.numCh; chId++)
    {
        pPrm->chParams.channelSyncList[chId] = TRUE;
    }
    return;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif


/*@}*/

