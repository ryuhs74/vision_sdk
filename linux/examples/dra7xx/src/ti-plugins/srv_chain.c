/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vivi_plugin.h>
#include <vivi_srv.h>
#include <srv_priv.h>
#include "include/srv_chain.h"

/* 2d-srv-info */
#define LVDS_SRV_2D_880x1080_NUM_EP_SINK   (2)
#define LVDS_SRV_2D_880x1080_NUM_EP_SOURCE (0)
/* TODO 3d-srv-info */

struct srv_chain {
    struct vivi_chain_info info;
    struct srv_chain_func func;
};

static const struct srv_chain srv_ctab[SRV_NUM_CHAINS] = 
{
    {
        .info = {
            "lvds-srv-2d-880x1080",
            LVDS_SRV_2D_880x1080,
            LVDS_SRV_2D_880x1080_NUM_EP_SINK,
            LVDS_SRV_2D_880x1080_NUM_EP_SOURCE
        },
        .func = {
            {
                &chains_lvdsVip2dSurroundView_CreateApp,
                &chains_lvdsVip2dSurroundView_StartApp,
                &chains_lvdsVip2dSurroundView_StopApp,
                &chains_lvdsVip2dSurroundView_DeleteApp
            },
            &chains_lvdsVip2dSurroundView_PauseApp,
            &chains_lvdsVip2dSurroundView_ResumeApp
        }
    }
#if 0
    ,{
        /* TODO update for 3D SRV */
    }
#endif
}; 

DEFINE_PLUGIN_NUM_CHAINS_FUNC(SRV_PNAME_TOKEN)
{
    return (SRV_NUM_CHAINS);
}

DEFINE_PLUGIN_CHAIN_INFO_FUNC(SRV_PNAME_TOKEN, chain_info, idx)
{
    if (idx < SRV_NUM_CHAINS) {
        strcpy(chain_info->cname, srv_ctab[idx].info.cname);
        chain_info->id = srv_ctab[idx].info.id;
        chain_info->num_sink_eplink = srv_ctab[idx].info.num_sink_eplink;
        chain_info->num_source_eplink = srv_ctab[idx].info.num_source_eplink;
    }
}

DEFINE_PLUGIN_CHAIN_CONTROL_FUNC(SRV_PNAME_TOKEN, chain_func, chain_id)
{
    struct srv_chain_func *srv_cfunc = (struct srv_chain_func*) chain_func;
    
    if (chain_id < SRV_NUM_CHAINS) {
        chain_func->create = srv_ctab[chain_id].func.create;
        chain_func->start = srv_ctab[chain_id].func.start;
        chain_func->stop = srv_ctab[chain_id].func.stop;
        chain_func->delete = srv_ctab[chain_id].func.delete;
        /* srv specific */
        srv_cfunc->pause = srv_ctab[chain_id].func.pause;
        srv_cfunc->resume = srv_ctab[chain_id].func.resume;
    }
}

void srv_get_egl_link(uint32_t *link_id, uint32_t *chain_id)
{
    /* TODO */
    *link_id = 0xff;
    *chain_id = 0xff;
}
