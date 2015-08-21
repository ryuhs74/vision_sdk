/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/utils_common/include/network_api.h>



int Network_read(Network_SockObj *pObj, UInt8 *dataBuf, UInt32 *dataSize)
{
#if ( defined(NDK_PROC_TO_USE_IPU1_0) && defined(BUILD_M4_0) ) || \
    ( defined(NDK_PROC_TO_USE_IPU1_1) && defined(BUILD_M4_1) ) || \
    ( defined(NDK_PROC_TO_USE_A15_0) && defined(BUILD_A15) )

    UInt32 tmpDataSize;
    int actDataSize = 0;

    tmpDataSize = *dataSize;

    while(tmpDataSize > 0)
    {
        actDataSize = recv(pObj->connectedSockFd, (void*)dataBuf, tmpDataSize, 0);
        if(actDataSize<=0)
        {
            *dataSize = 0;
            return -1;
        }
        dataBuf += actDataSize;
        tmpDataSize -= actDataSize;
    }

#endif

    return 0;
}


int Network_write(Network_SockObj *pObj, UInt8 *dataBuf, UInt32 dataSize)
{
#if ( defined(NDK_PROC_TO_USE_IPU1_0) && defined(BUILD_M4_0) ) || \
    ( defined(NDK_PROC_TO_USE_IPU1_1) && defined(BUILD_M4_1) ) || \
    ( defined(NDK_PROC_TO_USE_A15_0) && defined(BUILD_A15) )
    int actDataSize=0;

    while(dataSize > 0 ) {
        actDataSize = send(pObj->connectedSockFd, dataBuf, dataSize, 0);

        if(actDataSize<=0)
            break;
        dataBuf += actDataSize;
        dataSize -= actDataSize;
    }

    if( dataSize > 0 ) {
        return -1;
    }
#endif

    return 0;
}

int Network_open(Network_SockObj *pObj, UInt32 port)
{
#if ( defined(NDK_PROC_TO_USE_IPU1_0) && defined(BUILD_M4_0) ) || \
    ( defined(NDK_PROC_TO_USE_IPU1_1) && defined(BUILD_M4_1) ) || \
    ( defined(NDK_PROC_TO_USE_A15_0) && defined(BUILD_A15) )

    struct sockaddr_in   sin1;
    int option = 1;

    pObj->connectedSockFd = INVALID_SOCKET;
    pObj->port = port;
    pObj->sockFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if( pObj->sockFd == INVALID_SOCKET)
    {
        Vps_printf(" NETWORK: Unable to open socket (port=%d)!!!\n", port);
        return -1;
    }

    /* Bind to the specified Server port */
    bzero( &sin1, sizeof(struct sockaddr_in) );
    sin1.sin_family     = AF_INET;
    sin1.sin_addr.s_addr = INADDR_ANY;
    sin1.sin_port       = htons(pObj->port);

    setsockopt ( pObj->sockFd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof( option ) );

    if( bind( pObj->sockFd,(struct sockaddr *)&sin1, sizeof(sin1) ) < 0 )
    {
        Vps_printf(" NETWORK: Unable to bind() (port=%d) !!!\n", port);
        fdClose( pObj->sockFd );
        pObj->sockFd = INVALID_SOCKET;
        return -1;
    }

    if( listen( pObj->sockFd, 5 ) < 0 )
    {
        fdClose( pObj->sockFd );
        pObj->sockFd = INVALID_SOCKET;
        return -1;
    }
#endif

    return 0;
}

int Network_close(Network_SockObj *pObj, Bool closeServerSock)
{
#if ( defined(NDK_PROC_TO_USE_IPU1_0) && defined(BUILD_M4_0) ) || \
    ( defined(NDK_PROC_TO_USE_IPU1_1) && defined(BUILD_M4_1) ) || \
    ( defined(NDK_PROC_TO_USE_A15_0) && defined(BUILD_A15) )

    if(pObj->connectedSockFd != INVALID_SOCKET)
    {
        fdClose( pObj->connectedSockFd );
        pObj->connectedSockFd = INVALID_SOCKET;
    }

    if(closeServerSock)
    {
        if(pObj->sockFd != INVALID_SOCKET)
        {
            fdClose( pObj->sockFd );
            pObj->sockFd = INVALID_SOCKET;
        }
    }
#endif

    return 0;
}

int Network_waitConnect(Network_SockObj *pObj, UInt32 timeout)
{
#if ( defined(NDK_PROC_TO_USE_IPU1_0) && defined(BUILD_M4_0) ) || \
    ( defined(NDK_PROC_TO_USE_IPU1_1) && defined(BUILD_M4_1) ) || \
    ( defined(NDK_PROC_TO_USE_A15_0) && defined(BUILD_A15) )

    pObj->pollitem[0].fd = pObj->sockFd;
    pObj->pollitem[0].eventsRequested = POLLIN;

    if( fdPoll( pObj->pollitem, 1, timeout ) == SOCKET_ERROR )
    {
        Vps_printf(" NETWORK: fdPoll() failed with SOCKET_ERROR (port=%d) !!!\n", pObj->port);
        return -1;
    }

    if( pObj->pollitem[0].eventsDetected == FALSE)
    {
        /* NO connection, retry */
        return 0;
    }

    if( pObj->pollitem[0].eventsDetected & POLLNVAL )
    {
        Vps_printf(" NETWORK: fdPoll() failed with POLLNVAL (port=%d) !!!\n", pObj->port);
        return -1;
    }

    if( pObj->pollitem[0].eventsDetected & POLLIN )
    {
        pObj->connectedSockFd = accept( pObj->sockFd, 0, 0 );

        if( pObj->connectedSockFd != INVALID_SOCKET )
        {
            return 1;
        }
    }
#endif
    /* NO connection, retry */
    return 0;
}

int Network_sessionOpen(BspOsal_TaskHandle handle)
{
#if ( defined(NDK_PROC_TO_USE_IPU1_0) && defined(BUILD_M4_0) ) || \
    ( defined(NDK_PROC_TO_USE_IPU1_1) && defined(BUILD_M4_1) ) || \
    ( defined(NDK_PROC_TO_USE_A15_0) && defined(BUILD_A15) )

    if(handle==NULL)
    {
        handle = Task_self();
    }

    /* Allocate the file environment for this task */
    fdOpenSession( handle );
#endif

    return 0;
}

int Network_sessionClose(BspOsal_TaskHandle handle)
{
#if ( defined(NDK_PROC_TO_USE_IPU1_0) && defined(BUILD_M4_0) ) || \
    ( defined(NDK_PROC_TO_USE_IPU1_1) && defined(BUILD_M4_1) ) || \
    ( defined(NDK_PROC_TO_USE_A15_0) && defined(BUILD_A15) )

    if(handle==NULL)
    {
        handle = Task_self();
    }

    fdCloseSession( handle );

#endif

    return 0;
}

