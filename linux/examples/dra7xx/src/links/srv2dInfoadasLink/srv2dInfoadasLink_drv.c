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
 * \file srv2DInfoAdasLink_drv.c
 *
 * \brief  This file has the implementation of srv2DInfoAdasLink Link
 *
 *         srv2DInfoAdasLink Link is used to feed video frames to SGX for
 *         rendering.
 *         The rendered output will be pushed to display via DRM.
 *
 * \version 0.0 (Jun 2014) : [SS] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "srv2dInfoadasLink_priv.h"
#include <linux/src/osa/include/osa_mem.h>

extern srv2dInfoAdasLink_Obj gsrv2dInfoAdasLink_obj[];
/**
 *******************************************************************************
 *
 * \brief srv2DInfoAdasLink link create function
 *
 *        This Set the Link and driver create time parameters.
 *        - Get the channel info from previous link
 *        - Set the internal data structures
 *        - Call the create and control functions
 *
 * \param   pObj     [IN] srv2dInfoAdasLink Link Instance handle
 * \param   pPrm     [IN] srv2dInfoAdasLink link create parameters
 *                        This need to be configured by the application
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 srv2dInfoAdasLink_drvCreate(srv2dInfoAdasLink_Obj *pObj,
                             srv2dInfoAdasLink_CreateParams *pPrm)
{
    Int32 i,status = SYSTEM_LINK_STATUS_SOK;
    UInt32                  inQue, channelId,surfaceId;
    srv2dInfoAdasLink_InputQueId    inputQId;
    System_LinkChInfo     * pPrevChInfo;
    struct link_cb cb;

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SRV2D: Create in progress !!!\n");
#endif

    memcpy(&pObj->createArgs, pPrm, sizeof(*pPrm));

    status = OSA_mutexCreate(&(pObj->lock));
    OSA_assert(status == OSA_SOK);

    OSA_assert(pPrm->numInQue == SRV2DINFOADAS_LINK_IPQID_MAXIPQ);

    for (inQue = 0; inQue < pPrm->numInQue; inQue++)
    {
        status = System_linkGetInfo(pPrm->inQueParams[inQue].prevLinkId,
                                    &pObj->inTskInfo[inQue]);
        OSA_assert(status == SYSTEM_LINK_STATUS_SOK);
        OSA_assert(pPrm->inQueParams[inQue].prevLinkQueId <
                   pObj->inTskInfo[inQue].numQue);
        memcpy(&pObj->inQueInfo[inQue],
               &pObj->inTskInfo[inQue].queInfo[pPrm->inQueParams[inQue].prevLinkQueId],
               sizeof(pObj->inQueInfo));
    }

    /*
    OSA_assert(pObj->createArgs.inBufType[SRV2DINFOADAS_LINK_IPQID_MULTIVIEW] ==
                     SYSTEM_BUFFER_TYPE_VIDEO_FRAME_CONTAINER);
    OSA_assert(pObj->createArgs.inBufType[SRV2DINFOADAS_LINK_IPQID_STITCHED_DSP] ==
                     SYSTEM_BUFFER_TYPE_VIDEO_FRAME);
    */
    
    inputQId = SRV2DINFOADAS_LINK_IPQID_MULTIVIEW;
    channelId = 0;
    pPrevChInfo   =
        &(pObj->inQueInfo[inputQId].chInfo[channelId]);
    surfaceId = 0;

    for(i = 0; i < 4; i++) {
            pObj->multiView.buffers[i].surface_id                    = pObj->surface_Id[surfaceId];
            pObj->multiView.buffers[i].buf_type                      = SCENE_BUF_TYPE_PADDR;
            pObj->multiView.buffers[i].width      = pPrevChInfo->width;
	    pObj->multiView.buffers[i].height     = pPrevChInfo->height;
	    pObj->multiView.buffers[i].pitches[0] = pPrevChInfo->pitch[0];
	    pObj->multiView.buffers[i].pitches[1] = pPrevChInfo->pitch[1];
	    pObj->multiView.buffers[i].offsets[0] = 0;
	    pObj->multiView.buffers[i].offsets[1] = pPrevChInfo->pitch[0] * pPrevChInfo->height;
	    pObj->multiView.buffers[i].fourcc     = FOURCC_STR("NV12");
            surfaceId++;
	}
    	
    channelId = 0;
    inputQId = SRV2DINFOADAS_LINK_IPQID_STITCHED_DSP;
    pPrevChInfo   =
        &(pObj->inQueInfo[inputQId].chInfo[channelId]);

    pObj->stitched.buffers[0].surface_id                    = pObj->surface_Id[surfaceId];
    pObj->stitched.buffers[0].buf_type                      = SCENE_BUF_TYPE_PADDR;
    pObj->stitched.buffers[0].width      = pPrevChInfo->width;
    pObj->stitched.buffers[0].height     = pPrevChInfo->height;
    pObj->stitched.buffers[0].pitches[0] = pPrevChInfo->pitch[0];
    pObj->stitched.buffers[0].pitches[1] = pPrevChInfo->pitch[1];
    pObj->stitched.buffers[0].offsets[0] = 0;
    pObj->stitched.buffers[0].offsets[1] = pPrevChInfo->pitch[0] * pPrevChInfo->height;
    pObj->stitched.buffers[0].fourcc     = FOURCC_STR("NV12");

  cb.recipient        = RCVR_2DLINK;
  cb.surface_info     = srv2dInfoAdasLink_getSurfaceIds;
  cb.return_frame_buf = srv2dInfoAdasLink_putEmptyBuffers;

  LINK_register_cb(&cb);

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SRV2D: Create Done !!!\n");
#endif

    return SYSTEM_LINK_STATUS_SOK;
}
/**
 *******************************************************************************
 * \brief Function to get the surfaceId and scene-Ids . This is a callback function and will simply copies
 *    surfaceIds to instance's copy
 *
 * \param  pObj     [IN]  SRV2D_INFO_ADAS link instance handle
 *
 * \return SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Void srv2dInfoAdasLink_getSurfaceIds(UInt32 num_surfaces, UInt32 *surfaceId)
{
    Int32 i;
    srv2dInfoAdasLink_Obj *pObj = &gsrv2dInfoAdasLink_obj[0];

    if(pObj == NULL)
       return ;

    if(num_surfaces > SRV2DINFOADAS_LINK_MAX_SURFACES)
        return;

    pObj->num_surfaces = num_surfaces;

    for(i = 0 ; i < num_surfaces; i++)
       pObj->surface_Id[i] = surfaceId[i];

    for(i = 0; i < 4; i++)
        pObj->multiView.buffers[i].surface_id = surfaceId[i];

    pObj->stitched.buffers[0].surface_id = surfaceId[i];


    return;
}


    
/**
 *******************************************************************************
 *
 * \brief This function de-queue and process/srv2d the input frames
 *
 *        Function perform the following operations
 *        - De-queue the frames from the previous link output Q
 *        - Put them in intenal link queues
 *        - Get/De-queue the valid frames from the internal queue
 *        - Populate/Map OpenGL/Drm frame structure from system buffer
 *        - Call the OpenGL/DRM process function
 *        - Free-up these frames by send back to previous link after display
 *        - Immediately Free-up the frames which are not part of the display
 *
 * \param   pObj     [IN] srv2dInfoAdasLink Link Instance handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */

Int32 srv2dInfoAdasLink_drvDoProcessFrames(srv2dInfoAdasLink_Obj *pObj)
{
    srv2dInfoAdasLink_CreateParams  * pPrm;
    srv2dInfoAdasLink_InputQueId      inputQId;
    UInt32                    bufId;
    Int32                     i, status = SYSTEM_LINK_STATUS_SOK;
    System_BufferList            inputBufList;
    System_Buffer              * pSysBufferInput;
    System_VideoFrameCompositeBuffer *pVideoCompositeFrame;
    System_VideoFrameBuffer *pVideoFrame;

    pPrm = &pObj->createArgs;
	
    inputQId = SRV2DINFOADAS_LINK_IPQID_MULTIVIEW;

    System_getLinksFullBuffers(
        pPrm->inQueParams[inputQId].prevLinkId,
        pPrm->inQueParams[inputQId].prevLinkQueId,
        &inputBufList);
		
    if(inputBufList.numBuf)
    {
        for (bufId = 0; bufId < inputBufList.numBuf; bufId++)
        {
            pSysBufferInput = inputBufList.buffers[bufId];

            if (pSysBufferInput != NULL)
            {
   		    struct scene_buffer_object *bufObject = OSA_memAlloc(sizeof(struct scene_buffer_object));
			pVideoCompositeFrame = (System_VideoFrameCompositeBuffer *)
                                       (pSysBufferInput->payload);

			memcpy(bufObject, &pObj->multiView, sizeof(struct scene_buffer_object));

                        bufObject->num_buffers = 4;
                        bufObject->sbo_priv     = (Void*)pSysBufferInput;

			for (i = 0; i < 4; i++)
			{
			      bufObject->buffers[i].paddrs[0] = 
			                    (void *)OSA_memVirt2Phys((UInt32)pVideoCompositeFrame->bufAddr[0][i], OSA_MEM_REGION_TYPE_AUTO);
			}			
		
              		//POST THE BUFFER_OBJECT HERE to UI_CLIENT
                        SCENE_post_new_buffer(bufObject, SNDR_2DLINK);
			OSA_memFree(bufObject);
            }
        }
    }	
	
    inputQId = SRV2DINFOADAS_LINK_IPQID_STITCHED_DSP;

    System_getLinksFullBuffers(
        pPrm->inQueParams[inputQId].prevLinkId,
        pPrm->inQueParams[inputQId].prevLinkQueId,
        &inputBufList);
		
    if(inputBufList.numBuf)
    {
        for (bufId = 0; bufId < inputBufList.numBuf; bufId++)
        {
            pSysBufferInput = inputBufList.buffers[bufId];

            if (pSysBufferInput != NULL)
            {
		    struct scene_buffer_object *bufObject = OSA_memAlloc(sizeof(struct scene_buffer_object));
		    pVideoFrame = (System_VideoFrameBuffer *)
                                       (pSysBufferInput->payload);

		    memcpy(bufObject, &pObj->stitched, sizeof(struct scene_buffer_object));

                    bufObject->num_buffers = 1;
                    bufObject->sbo_priv     = (Void*)pSysBufferInput;

		    bufObject->buffers[0].paddrs[0] = 
			                    (void *)OSA_memVirt2Phys((UInt32)pVideoFrame->bufAddr[0], OSA_MEM_REGION_TYPE_AUTO);
									
               
                   //POST THE BUFFER_OBJECT HERE to UI_CLIENT
                   SCENE_post_new_buffer(bufObject, SNDR_2DLINK);
		   OSA_memFree(bufObject);
            }

        }
    }	
    
    return status;
}

/**
 *******************************************************************************
 * \brief Function called by thread from ui-client .
 *    Fromt he bufferPayload, identifies which buffer to be removed from
   list and to be freed.
 *
 * \param  ptr      [IN]  buffer_object pointer
 *
 * \return SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Void srv2dInfoAdasLink_putEmptyBuffers(Void *buf)
{
   Int32 inQue = SRV2DINFOADAS_LINK_IPQID_MULTIVIEW;
   System_Buffer *pBuf;
   System_BufferList freeBufList;
   srv2dInfoAdasLink_CreateParams *pCreateArgs;
   srv2dInfoAdasLink_Obj *pObj = &gsrv2dInfoAdasLink_obj[0];
   struct scene_buffer_object *buffer = buf;

   if(pObj == NULL)
       return ;

   pCreateArgs = &pObj->createArgs;

   OSA_mutexLock(&(pObj->lock));

   if(buffer->num_buffers == 1)
   {
        inQue = SRV2DINFOADAS_LINK_IPQID_STITCHED_DSP;
   }

   else if(buffer->num_buffers == 4)
   {
        inQue = SRV2DINFOADAS_LINK_IPQID_MULTIVIEW;
   }
   else
     OSA_assert(0);


   pBuf = (System_Buffer*)buffer->sbo_priv;
   OSA_assert(pBuf != NULL);


   freeBufList.numBuf = 1;
   freeBufList.buffers[0] = pBuf;
   System_putLinksEmptyBuffers(pCreateArgs->inQueParams[inQue].
                                       prevLinkId,
                                       pCreateArgs->inQueParams[inQue].
                                       prevLinkQueId,
                                       &freeBufList);


   OSA_mutexUnlock(&(pObj->lock));

   return;
}
/**
 *******************************************************************************
 *
 * \brief This function delete the srv2d link & driver
 *
 *        De-queue any frames which are held inside the driver, then
 *        - Delete the simply driver
 *        - delete the semaphore and other link data structures
 *        - delete the link periodic object
 *
 * \param   pObj     [IN] srv2dInfoAdasLink Link Instance handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 srv2dInfoAdasLink_drvDelete(srv2dInfoAdasLink_Obj *pObj)
{
    
    struct link_cb cb;
#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SRV2D: Delete in progress !!!\n");
#endif
    
#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SRV2D: Delete Done !!!\n");
#endif

  cb.recipient = RCVR_2DLINK;
    LINK_unregister_cb(&cb);

    OSA_mutexDelete(&pObj->lock);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief This function start the simply driver
 *
 *        Primming of a few frames are required to start the srv2dInfoAdasLink driver.
 *        Use blank buffers to prime and start the simply driver even
 *        before the actual frames are received by the srv2d link. This
 *        primming is done while srv2d link create. Start shall be called
 *        only after the link create function
 *
 * \param   pObj     [IN] srv2dInfoAdasLink Link Instance handle
 *
 * \return  status
 *
 *******************************************************************************
 */
Int32 srv2dInfoAdasLink_drvStart(srv2dInfoAdasLink_Obj *pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SRV2D: Start in progress !!!\n");
#endif

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SRV2D: Start Done !!!\n");
#endif

    return status;
}

/**
 *******************************************************************************
 *
 * \brief This function stop the simply driver
 *
 *        When ever the driver is stopped, enable the srv2d link periodic
 *        call back function. This will initiate to free-up the input frames
 *        in STOP state. The driver call back will be stopped when srv2d
 *        driver stop is done
 *
 * \param   pObj     [IN] srv2dInfoAdasLink Link Instance handle
 *
 * \return  status
 *
 *******************************************************************************
 */
Int32 srv2dInfoAdasLink_drvStop(srv2dInfoAdasLink_Obj *pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SRV2D: Stop in progress !!!\n");
#endif

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SRV2D: Stop Done !!!\n");
#endif

    return status;
}

