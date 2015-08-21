/*
 * Copyright (c) 2012-2013, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <linux/examples/common/chains_common.h>
#include <include/link_api/epLink.h>
#include <vivi_plugin.h>

extern Int32 (*link_control)(UInt32 link_id, UInt32 cmd, Void *param, 
		UInt32 param_size, Bool ack);

/** Vision subsytem init/deinit
 */
Int32 Vision_subsys_init()
{
    System_init();

    /* allow remote prints to complete, before showing main menu */
    OSA_waitMsecs(500);

    /** Initialize function pointers for link control
     * We have these function pointers for two reasons....
     * (1) we don't want to expose too many header files from Vision subsys
     * (2) we don't want to create wrapper functions for such generic functions.
     * The plugin or the Core of the vivi framework will invoke these functions
     * via the following function pointers.
     */
    link_control = &System_linkControl;

    return 0;
}

Int32 Vision_subsys_deinit()
{
    System_deInit(FALSE);

    return 0;
}

/** The epLink control APIs invoked by the vivi framework Core or the plugins.
 */

/** This function is used to return the empty buffers to the sink eplink.
 */
Int32 eplink_control_put_empty_buf(UInt32 eplink_id, System_Buffer *obj)
{
    UInt16 queId;
    System_BufferList pBufList;
    Int32 status = SYSTEM_LINK_STATUS_EFAIL;

    pBufList.numBuf = 1;
    pBufList.buffers[0] = obj;

    /**
     * this being an endpoint link, queId is not used to send the buffers
     * back to the eplink
     */
    queId = 0;

    status = System_putLinksEmptyBuffers(eplink_id, queId, &pBufList);
    if (status < 0) {
        Vps_printf("VIVI: Endpoint: %x: Failed to put empty buf\n", eplink_id);
    }

    return status;
}

/** TODO This function is used to input the filled buffers to a source eplink.
 */
Int32 eplink_control_put_filled_buf(UInt32 eplink_id, System_Buffer *obj)
{
    Int32 status = SYSTEM_LINK_STATUS_EFAIL;

    return status;
}

/** This function is used to create buffer que handle - required by the eplink.
 */
Int32 eplink_control_create_qh(UInt32 eplink_id, struct ep_buf_que *que)
{
    Int32 status = SYSTEM_LINK_STATUS_EFAIL;

    status = System_linkControl(eplink_id, EP_CMD_CREATE_QUE_HANDLE, (Void*) que, sizeof(struct ep_buf_que), TRUE);

    return status;
}

/** This function is used to get the channel info from a eplink, acting as sink.
 */
Int32 eplink_control_get_ch_info(UInt32 eplink_id, System_LinkQueInfo *epInfo)
{
    System_LinkInfo linkInfo;
    Int32 status = SYSTEM_LINK_STATUS_EFAIL;

    status = System_linkGetInfo(eplink_id, &linkInfo);
    OSA_assert(linkInfo.numQue == 1);

    memcpy(epInfo, &linkInfo.queInfo[0], sizeof(System_LinkQueInfo));

    return status;
}

