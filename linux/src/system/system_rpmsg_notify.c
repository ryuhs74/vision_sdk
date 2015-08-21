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
 * \file system_ipc_notify.c
 *
 * \brief  This file implements the function for the IPC notify functionality
 *
 *         Notify is no more part of IPC 3.x, function names still use notify
 *         to keep compatibility with bios side notify mechanism.
 *
 *         On linux side these APIs use socket based implementation of RPMSG
 *
 * \version 0.0 (Apr 2014) : [YM] First version implemeted using RPMSG
 * \version 0.0 (Mar 2015) : [YM] Removed init de-init, implemented messaging
 *                                based on message types
 *
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "system_priv_ipc.h"

#include <sys/select.h>
#include <sys/eventfd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>


/* Ipc Socket Protocol Family */

#define __USE_GNU
#include <sys/socket.h>
#include <net/rpmsg.h>

System_IpcNotifyObj gSystem_ipcNotifyObj;

/**
 *******************************************************************************
 *
 * \brief Function to register notify callback for links.
 *
 *        Links register its notify handler. This file registers a single call
 *        back function with the notify sub-system. Form that callback
 *        it invokes the callback registered by links based on the linkId
 *        passed with  notify callback.
 *
 * \param   linkId      [IN] linkID of the registering link.
 * \param   notifyCb    [IN] callback function of the link to be registered
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success.
 *
 *******************************************************************************
 */
Int32 System_ipcRegisterNotifyCb(UInt32 linkId, System_ipcNotifyCb notifyCb)
{
    linkId = SYSTEM_GET_LINK_ID(linkId);
    OSA_assert(linkId < SYSTEM_LINK_ID_MAX);

    gSystem_ipcNotifyObj.notifyCb[linkId] = notifyCb;

    return SYSTEM_LINK_STATUS_SOK;
}


/**
 *******************************************************************************
 *
 * \brief Utility function to invoke the notify to remote processor.
 *
 *        This function generates the notify event. It takes linkId as input
 *        maps linkId with procId and generates notify event on that processor
 *
 * \param   linkId      [IN] Link Id to which notify needs to be generated.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success.
 *
 *******************************************************************************
 */
Int32 System_ipcSendNotify(UInt32 linkId)
{
    Int32 err;
    UInt32 procId = SYSTEM_GET_PROC_ID(linkId);

    if(procId == SYSTEM_PROC_EVE1 ||
       procId == SYSTEM_PROC_EVE2 ||
       procId == SYSTEM_PROC_EVE3 ||
       procId == SYSTEM_PROC_EVE4)
    {
        SYSTEM_LINK_ID_SET_ROUTE_BIT(linkId);
        procId = SYSTEM_PROC_IPU1_0;
    }

    err = send(gSystem_ipcNotifyObj.sockFdTx[procId],
                (Ptr)&linkId,
                sizeof(linkId),
                0);
    if (err < 0) {
        Vps_printf(" SYSTEM: IPC: [%s] Notify send failed (%s, %d) !!!\n",
                System_getProcName(procId), strerror(errno), errno);
        return -1;
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief This fuction generates unblock event and unblocks select()
 *        Mainly used in shutdown sequence
 *
 *
 *******************************************************************************
 */
Void System_ipcNotifyUnblock()
{
    uint64_t     buf = 1;

    /* Write 8 bytes to shutdown */
    write(gSystem_ipcNotifyObj.unblockFd, &buf, sizeof(buf));
}


/**
 *******************************************************************************
 *
 * \brief System wide notify handler function.
 *
 *        This function blocks on recvfrom() UNIX API to get messages from the
 *        kernel based on the message received it calls appropriate registered
 *        call back
 *
 * \return  NULL.
 *
 *******************************************************************************
 */
Void* System_ipcNotifyRecvFxn(Void * prm)
{

    socklen_t len;
    struct sockaddr_rpmsg rpmsg_addr;
    UInt32 payload;
    UInt32 linkProcId;
    UInt32 linkId;
    UInt32 procId;
    UInt32 type;
    OSA_TskHndl * pTsk;
    Int32 maxfd;
    Int32 nfds;
    Int32 retVal;
    fd_set rfds;
    Int32 i;
    Int32 status;
    Int32 done;

    done = FALSE;
    while(!done)
    {
        maxfd  = 0;
        retVal = 0;

        /* Wait (with timeout) and retreive message from socket: */
        FD_ZERO(&rfds);

        /* Initialize read fds for communication */
        i = 0;
        while (gSystem_ipcEnableProcId[i] != SYSTEM_PROC_MAX)
        {
            procId = gSystem_ipcEnableProcId[i];
            if ((procId != System_getSelfProcId()) &&
                (procId != SYSTEM_PROC_INVALID)    &&
                (procId != SYSTEM_PROC_EVE1)       &&
                (procId != SYSTEM_PROC_EVE2)       &&
                (procId != SYSTEM_PROC_EVE3)       &&
                (procId != SYSTEM_PROC_EVE4))
            {
                maxfd = MAX(maxfd, gSystem_ipcNotifyObj.sockFdRx[procId]);
                FD_SET(gSystem_ipcNotifyObj.sockFdRx[procId], &rfds);
            }
            i++;
        }

        /* Wait on the event fd, which may be written by System_ipcNotifyUnblock(): */
        FD_SET(gSystem_ipcNotifyObj.unblockFd, &rfds);

        /* Add one to last fd created, this is mandated by select() */
        nfds = MAX(maxfd, gSystem_ipcNotifyObj.unblockFd) + 1;

        retVal = select(nfds, &rfds, NULL, NULL, NULL);

        if (retVal)  {

            if (FD_ISSET(gSystem_ipcNotifyObj.unblockFd, &rfds))  {
                /*
                 * Our event was signalled by System_ipcNotifyUnblock().
                 *
                 * This is typically done during a shutdown sequence, where
                 * the intention of the client would be to ignore (i.e. not fetch)
                 * any pending messages in the transport's queue.
                 * Thus, we shall not check for nor return any messages.
                 */
                done = TRUE;
            }
            else {

                /* Process all messages received on different Rx sockets */
                i = 0;
                while (gSystem_ipcEnableProcId[i] != SYSTEM_PROC_MAX)
                {
                    procId = gSystem_ipcEnableProcId[i];
                    if ((procId != System_getSelfProcId()) &&
                        (procId != SYSTEM_PROC_INVALID)    &&
                        (procId != SYSTEM_PROC_EVE1)       &&
                        (procId != SYSTEM_PROC_EVE2)       &&
                        (procId != SYSTEM_PROC_EVE3)       &&
                        (procId != SYSTEM_PROC_EVE4)       &&
                         FD_ISSET(gSystem_ipcNotifyObj.sockFdRx[procId], &rfds))
                    {
                        len = sizeof(struct sockaddr);
                        status = recvfrom(
                                        gSystem_ipcNotifyObj.sockFdRx[procId],
                                        &payload, sizeof(UInt32), 0,
                                        (struct sockaddr *)&rpmsg_addr,
                                        &len);

                        if (status < 0) {
                            Vps_printf(" SYSTEM: IPC: [%s] Notify recvfrom failed "
                                       "(%s, %d) !!!\n",
                                System_getProcName(procId), strerror(errno), errno);

                            continue;
                        }

                        if (len != sizeof(rpmsg_addr)) {
                            Vps_printf(" SYSTEM: IPC: [%s] Notify got bad RP Msg addr !!!"
                                       ,System_getProcName(procId));

                            continue;
                        }

                        type = SYSTEM_LINK_ID_GET_NOTIFY_TYPE(payload);

                        if(type==SYSTEM_LINK_ID_NOTIFY_TYPE_LINK_ID)
                        {
                            linkProcId = SYSTEM_GET_PROC_ID(payload);
                            linkId = SYSTEM_GET_LINK_ID(payload);

                            OSA_assert(linkId < SYSTEM_LINK_ID_MAX);

                            OSA_assert(System_getSelfProcId() == linkProcId);

                            if (gSystem_ipcNotifyObj.notifyCb[linkId])
                            {
                                pTsk = System_getLinkTskHndl(linkId);

                                OSA_assert(pTsk!=NULL);
                                gSystem_ipcNotifyObj.notifyCb[linkId] (pTsk);
                            }
                        }
                        else
                        {
                            System_ipcMsgQHandler(payload);
                        }
                    }
                    i++;
                } /* while (gSystem_ipcEnableProcId[i] != SYSTEM_PROC_MAX) */
            }
        } /* if (retVal)  { */
    }   /* while(! done) */

    return(NULL);

}

/**
 *******************************************************************************
 *
 * \brief Utility to initialize notify sub-system for links and chains
 *        work.
 *
 *        Initialize the notify event for local processor. It also registers
 *        callback with notify sub-system to get notify events from remote
 *        processor.
 *
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success.
 *
 *******************************************************************************
 */
Int32 System_ipcNotifyInit()
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    Int32 i;
    UInt32 procId;

    Vps_printf(" SYSTEM: IPC: Notify init in progress !!!\n");

    memset(&gSystem_ipcNotifyObj, 0, sizeof(gSystem_ipcNotifyObj));

    /* Create Tx and Rx sockets for sending and receiving messages  */
    i = 0;
    while (gSystem_ipcEnableProcId[i] != SYSTEM_PROC_MAX)
    {
        procId = gSystem_ipcEnableProcId[i];
        if ((procId != System_getSelfProcId()) &&
            (procId != SYSTEM_PROC_INVALID)    &&
            (procId != SYSTEM_PROC_EVE1)       &&
            (procId != SYSTEM_PROC_EVE2)       &&
            (procId != SYSTEM_PROC_EVE3)       &&
            (procId != SYSTEM_PROC_EVE4))
        {
            gSystem_ipcNotifyObj.sockFdRx[procId] =
                System_ipcCreateChannel(procId,
                            SYSTEM_RPMSG_NOTIFY_ENDPT_HOST,
                            SYSTEM_RPMSG_RX_CHANNEL);

            if(gSystem_ipcNotifyObj.sockFdRx[procId] < 0)
            {
                Vps_printf(" SYSTEM: IPC: [%s] Notify RX channel create failed"
                           " (endpoint = %d) !!!\n",
                         System_getProcName(procId),
                         SYSTEM_RPMSG_NOTIFY_ENDPT_HOST);
                return -1;
            }

            /* host side Tx end point is dont care as it will be allocated by linux after connect */
            gSystem_ipcNotifyObj.sockFdTx[procId] =
                System_ipcCreateChannel(procId,
                                SYSTEM_RPMSG_ENDPT_REMOTE,
                                SYSTEM_RPMSG_TX_CHANNEL);

            if(gSystem_ipcNotifyObj.sockFdTx[procId] < 0)
            {
                Vps_printf(" SYSTEM: IPC: [%s] Notify TX channel create failed"
                           " (endpoint = %d) !!!\n",
                         System_getProcName(procId),
                         SYSTEM_RPMSG_ENDPT_REMOTE);
                return -1;
            }
        }

        i++;
    }

    /*
     * To support System_ipcNotifyUnblock() functionality, create an event object.
     * Writing to this event will unblock the select() call in MessageQ_get().
     */
    gSystem_ipcNotifyObj.unblockFd = eventfd(0, 0);
    if (gSystem_ipcNotifyObj.unblockFd == -1)  {
        Vps_printf(" SYSTEM: IPC: Notify unblock event create failed (%s, %d) !!!\n",
                strerror(errno), errno);
        return -1;
    }

    /* Create a thread that blocks on socket to receive message from kernel */
    status = OSA_thrCreate(
                &gSystem_ipcNotifyObj.thrHndl,
                System_ipcNotifyRecvFxn,
                SYSTEM_RPMSG_NOTIFY_TSK_PRI,
                OSA_THR_STACK_SIZE_DEFAULT,
                NULL);
    OSA_assertSuccess(status);

    Vps_printf(" SYSTEM: IPC: Notify init DONE !!!\n");

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Utility to de-initialize notify sub-system for links and chains
 *        work.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success.
 *
 *******************************************************************************
 */
Int32 System_ipcNotifyDeInit()
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    Int32 i;
    UInt32 procId;

    Vps_printf(" SYSTEM: IPC: Notify de-init in progress !!!\n");

    System_ipcNotifyUnblock();

    OSA_thrDelete(&gSystem_ipcNotifyObj.thrHndl);

    /* Initialize Endpoints for communication */
    i = 0;
    while (gSystem_ipcEnableProcId[i] != SYSTEM_PROC_MAX)
    {
        procId = gSystem_ipcEnableProcId[i];
        if ((procId != System_getSelfProcId()) &&
            (procId != SYSTEM_PROC_INVALID)    &&
            (procId != SYSTEM_PROC_EVE1)       &&
            (procId != SYSTEM_PROC_EVE2)       &&
            (procId != SYSTEM_PROC_EVE3)       &&
            (procId != SYSTEM_PROC_EVE4))
        {
            System_ipcDeleteChannel(gSystem_ipcNotifyObj.sockFdRx[procId]);
            System_ipcDeleteChannel(gSystem_ipcNotifyObj.sockFdTx[procId]);
        }
        i++;
    }

    close(gSystem_ipcNotifyObj.unblockFd);

    Vps_printf(" SYSTEM: IPC: Notify de-init DONE !!!\n");

    return status;
}
