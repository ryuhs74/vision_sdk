 /*
 *******************************************************************************
 *
 * Copyright (C) 2014s Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file networkCtrl_if.h
 *
 * \brief Header and command interface between PC application
 *        and target application
 *
 *        This file will be common bewteen the PC application
 *        and target application, hence it should not include any target
 *        specific data types and include files
 *
 *******************************************************************************
 */

#ifndef _NETWORK_CTRL_IF_H_
#define _NETWORK_CTRL_IF_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/*******************************************************************************
 *  \brief TCP/IP port on which the server is listening
 *******************************************************************************
 */
#define NETWORK_CTRL_SERVER_PORT    (5000)



/*******************************************************************************
 *  \brief Default TCP/IP port on which the network RX link will listen
 *******************************************************************************
 */
#define NETWORK_RX_SERVER_PORT      (6000)

/*******************************************************************************
 *  \brief Default TCP/IP port on which the network TX link will listen
 *******************************************************************************
 */
#define NETWORK_TX_SERVER_PORT      (7000)

/*******************************************************************************
 *  \brief Header packet magic number to confirm received packet is a header
 *******************************************************************************
 */
#define NETWORK_CTRL_HEADER         (0x1234ABCD)
#define NETWORK_RX_HEADER           (0x5678ABCD)
#define NETWORK_TX_HEADER           (0xABCD4321)

/*******************************************************************************
 *  \brief Max length of command string
 *******************************************************************************
 */
#define NETWORK_CTRL_CMD_STRLEN_MAX     (64)

/*******************************************************************************
 *  \brief Flag that is set in the 'flags' field of the header to indicate
 *         this packet is a ACK packet for a previously send command
 *******************************************************************************
 */
#define NETWORK_CTRL_FLAG_ACK            (0x00000001)



/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

typedef struct {

    unsigned int header;
    /**< Header magic number NETWORK_CTRL_HEADER */

    char         cmd[NETWORK_CTRL_CMD_STRLEN_MAX];
    /**< Command, specified as a string of char's */

    unsigned int returnValue;
    /**< Return value that is set by teh command handler */

    unsigned int flags;
    /**< command specified flags, see NETWORK_CTRL_FLAG_* */

    unsigned int prmSize;
    /**< Size of input parameters in units of bytes.
     *   Can be 0 if no parameters need to sned for a command
     */

} NetworkCtrl_CmdHeader;

/**
 *******************************************************************************
 *
 * \brief Types of payload's that can be exchanged over network
 *
 *******************************************************************************
 */
#define NETWORK_RX_TYPE_META_DATA                    (0x1)
#define NETWORK_RX_TYPE_BITSTREAM_MJPEG              (0x2)
#define NETWORK_RX_TYPE_VIDEO_FRAME_YUV422I_YUYV     (0x8)
#define NETWORK_RX_TYPE_VIDEO_FRAME_YUV420SP_UV      (0x9)


typedef struct {

    unsigned int header;
    /**< Header magic number NETWORK_RX_HEADER */

    unsigned int payloadType;
    /**< Payload type NETWORK_RX_TYPE_* */

    unsigned int chNum;
    /**< channel ID */

    unsigned int dataSize;
    /**< Size of payload data in bytes */

    unsigned int width;
    /**< Width of video frame */

    unsigned int height;
    /**< Height of video frame */

    unsigned int pitch[2];
    /**< Pitch of video frame in bytes */

} NetworkRx_CmdHeader;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */


