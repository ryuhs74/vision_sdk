/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */
#ifndef __SRV_CHAIN_H__
#define __SRV_CHAIN_H__

enum srv_chain_id {
    LVDS_SRV_2D_880x1080,
    SRV_NUM_CHAINS,
};

Int32 chains_lvdsVip2dSurroundView_CreateApp(struct vivi_sink *sink, struct vivi_source *source);
Int32 chains_lvdsVip2dSurroundView_StartApp();
Int32 chains_lvdsVip2dSurroundView_StopApp();
Int32 chains_lvdsVip2dSurroundView_DeleteApp();
Int32 chains_lvdsVip2dSurroundView_PauseApp();
Int32 chains_lvdsVip2dSurroundView_ResumeApp();

#endif /* __SRV_CHAIN_H__ */
