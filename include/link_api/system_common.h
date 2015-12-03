/*
 *******************************************************************************
 *
 * Copyright (c) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \ingroup SYSTEM_LINK_API System Link API
 *
 * @{
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file system_common.h
 *
 * \brief This module has the system data structures and constants shared
 *        across the processors.
 *
 * \version 0.0 (Jun 2013) : [KC] First Version
 * \version 0.1 (Jun 2013) : [KC] Updates as per code review comments
 * \version 0.2 (Aug 2013) : [KC] Added CMDs and structures to print statistics
 * \version 0.2 (Mar 2015) : [YM] Added rmpsg messgae types and changed endpts
 *
 *******************************************************************************
 */

#ifndef _SYSTEM_COMMON_H_
#define _SYSTEM_COMMON_H_

/*******************************************************************************
 * Include files
 *******************************************************************************
 */


/*******************************************************************************
 * Defines
 *******************************************************************************
 */

/* @{ */

/**
 *******************************************************************************
 *
 * \brief Frequency values of various cores set explicitly during system init
 *
 *******************************************************************************
 */
#if defined(TDA2XX_BUILD)
#define SYSTEM_DSP_FREQ            (600000000)
#define SYSTEM_EVE_FREQ            (267500000)
#define SYSTEM_IPU_FREQ            (212000000)
#define SYSTEM_A15_FREQ            (588000000)
#endif

#if defined(TDA3XX_BUILD)
#define SYSTEM_DSP_FREQ            (500000000)
#define SYSTEM_EVE_FREQ            (250000000)
#define SYSTEM_IPU_FREQ            (212000000)
#endif

#if defined(TDA2EX_BUILD)
#define SYSTEM_DSP_FREQ            (600000000)
#define SYSTEM_IPU_FREQ            (212000000)
#define SYSTEM_A15_FREQ            (588000000)
#endif

/**
 *******************************************************************************
 *
 * \brief System command to create a link
 *
 * \param Void *    [IN] Link specific parameters
 *
 *******************************************************************************
 */
#define SYSTEM_CMD_CREATE                   (0x00000000)

/**
 *******************************************************************************
 *
 * \brief System command to start a link
 *
 * \param None
 *
 *******************************************************************************
 */
#define SYSTEM_CMD_START                    (0x00000001)

/**
 *******************************************************************************
 *
 * \brief System command to stop a link
 *
 * \param None
 *
 *******************************************************************************
 */
#define SYSTEM_CMD_STOP                     (0x00000002)

/**
 *******************************************************************************
 *
 * \brief System command to delete a link
 *
 * \param None
 *
 *******************************************************************************
 */
#define SYSTEM_CMD_DELETE                   (0x00000003)

/**
 *******************************************************************************
 *
 * \brief System command to send data a link
 *
 * \param None
 *
 *******************************************************************************
 */
#define SYSTEM_CMD_NEW_DATA                 (0x00000004)

/**
 *******************************************************************************
 *
 * \brief System command to get information about a link. Normally this is
 *        used by a link to get the previous link's System_LinkInfo
 *
 * \param System_LinkInfo * [OUT] Link information
 *
 *******************************************************************************
 */
#define SYSTEM_CMD_GET_INFO                 (0x00000005)

/**
 *******************************************************************************
 *
 * \brief System command to print link statistics
 *
 * \param None
 *
 *******************************************************************************
 */
#define SYSTEM_CMD_PRINT_STATISTICS         (0x00000FFF)

/**
 *******************************************************************************
 *
 * \brief System command to print buffer related statistics for a link
 *
 * \param None
 *
 *******************************************************************************
 */
#define SYSTEM_CMD_PRINT_BUFFER_STATISTICS  (0x00000007)

/**
 *******************************************************************************
 * ryuhs74@20151028 - Add System Command
 * \brief AVM-E500 System command Define
 *
 * \param None
 *
 *******************************************************************************
 */
#define SYSTEM_CMD_FRONT_SIDE_VIEW (0x00000008)

#define SYSTEM_CMD_REAR_SIDE_VIEW (0x00000009)

#define SYSTEM_CMD_RIGH_SIDE_VIEW (0x00000010)

#define SYSTEM_CMD_LEFT_SIDE_VIEW (0x00000011)

#define SYSTEM_CMD_FULL_FRONT_VIEW (0x00000012)

#define SYSTEM_CMD_FULL_REAR_VIEW (0x00000013)

#define SYSTEM_CMD_FILE_SAVE_START (0x00000014)

#define SYSTEM_CMD_FILE_SAVE_DONE (0x00000015)

#define SYSTEM_CMD_CLEAR_E500_UI (0x00000016)

#define SYSTEM_CMD_REDRAW_E500_UI (0x00000017)
/**
 *******************************************************************************
 *
 * \brief The line and event ID of IPC Notify
 *
 *******************************************************************************
 */
#define SYSTEM_IPC_NOTIFY_LINE_ID   (0)
#define SYSTEM_IPC_NOTIFY_EVENT_ID  (15)

/**
 *******************************************************************************
 *
 * \brief Define the IPC Message Queue heap and message parameters.
 *
 *******************************************************************************
 */
#define SYSTEM_IPC_MSGQ_MSG_PAYLOAD_PTR(msg)    (Void*)((UInt32)(msg)+sizeof(SystemIpcMsgQ_Msg))

/**
 * \brief Remote end point, This will be created on slave cores
 */
#define SYSTEM_RPMSG_ENDPT_REMOTE          80

/**
 * \brief This will be created at host and used by rpmsg notify module
 *        to receive notifications from slave
 */
#define SYSTEM_RPMSG_NOTIFY_ENDPT_HOST     81

/**
 * \brief This will be created at host and used by rpmsg msgQ module
 *        to receive data messages from slave
 */
#define SYSTEM_RPMSG_MSGQ_DATA_ENDPT_HOST  82

/**
 * \brief This will be created at host and used by rpmsg msgQ module
 *        to receive ack messages from slave
 */
#define SYSTEM_RPMSG_MSGQ_ACK_ENDPT_HOST   83


/**
 *******************************************************************************
 *
 * \brief The names of processor cores in the system
 *
 *******************************************************************************
 */
#define SYSTEM_IPC_PROC_NAME_DSP1            "DSP1"
#define SYSTEM_IPC_PROC_NAME_DSP2            "DSP2"
#define SYSTEM_IPC_PROC_NAME_EVE1            "EVE1"
#define SYSTEM_IPC_PROC_NAME_EVE2            "EVE2"
#define SYSTEM_IPC_PROC_NAME_EVE3            "EVE3"
#define SYSTEM_IPC_PROC_NAME_EVE4            "EVE4"
#define SYSTEM_IPC_PROC_NAME_IPU1_0          "IPU1-0"
#define SYSTEM_IPC_PROC_NAME_IPU1_1          "IPU1-1"
#define SYSTEM_IPC_PROC_NAME_A15_0           "HOST"
#define SYSTEM_IPC_PROC_NAME_INVALID         "INVALID PROC"


/**
 *******************************************************************************
 *
 * \brief Macro used as an initializer of const char * type entity for the
 *        system scan format names. Mainly used in log/debug.
 *
 *
 *******************************************************************************
 */
#define SYSTEM_SCAN_FORMAT_STRINGS  \
        { "INTERLACED ",    \
          "PROGRESSIVE"     \
        }

/**
 *******************************************************************************
 *
 * \brief Macro used as an initializer of const char * type entity for the
 *        system memory type names. Mainly used in log/debug.
 *
 *******************************************************************************
 */
#define SYSTEM_MEMORY_TYPE_STRINGS  \
        { "NON-TILED  ",    \
          "TILED      "     \
        }
/**
 *******************************************************************************
 *
 * \brief Macro used as an initializer of const char * type entity for the
 *        System names for on/off condition. Mainly used in log/debug.
 *
 *******************************************************************************
 */
#define SYSTEM_ON_OFF_STRINGS  \
        { "OFF",    \
          "ON"     \
        }


/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

/*******************************************************************************
 * Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Message exchanged using the IPC message queue
 *
 *******************************************************************************
 */
typedef struct {

    UInt32 linkId;
    /**< The unique ID of the link the message is sent to */
    UInt32 cmd;
    /**< The command passed between links */
    UInt32 prmSize;
    /**< The size of the parameter */
    UInt32 waitAck;
    /**< wait for ACK flag */
    UInt32 status;
    /**< Message passing status */

} SystemIpcMsgQ_Msg;


/*******************************************************************************
 * Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \brief Global object for defining which Processors needs to be enabled.
 *
 * This is required to loop around the IPC attach and other ipc functions
 * based on processors enabled in links and chains.
 *******************************************************************************
 */
extern const UInt32 gSystem_ipcEnableProcId[];

/**
 *******************************************************************************
 *
 * \brief Convert from LCFW Proc ID to Syslink Proc ID
 *
 *        Syslink/IPC Proc ID could be different from Links and Chains FW
 *        Proc ID and framework does not rely on syslink/IPC Proc ID and
 *        LCFW Proc ID being same.
 *
 * \param procId    [IN] Syslink Proc ID
 *
 * \return Links & Chains FW Processor ID
 *
 *******************************************************************************
 */
UInt32 System_getSyslinkProcId(UInt32 procId);

#endif

/* @} */
