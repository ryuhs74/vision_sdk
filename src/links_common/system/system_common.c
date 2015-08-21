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
 * \file system_common.c
 *
 * \brief   System level initialization for all components.
 *
 *          This file deals with system level initialization of links and
 *          chains framework. It creates a system tasks for handling all system
 *          wide functions. Initializes the system wide resources like
 *          and components performance counters, mail boxes, IPC, memory
 *          allocators. It also initializes all links in the system such that
 *          links are ready to take commands from application.
 *
 * \version 0.0 (Jun 2013) : [KC] First version taken from DVR RDK and
 *                            cleaned up for Vision_sdk
 * \version 0.1 (Jul 2013) : [HS] Commenting style update as per defined
 *                                format.
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "system_priv_ipc.h"

/**
 *******************************************************************************
 * \brief Link Stack
 *******************************************************************************
 */
#pragma DATA_ALIGN(gSystem_tskStack, 32)
#pragma DATA_SECTION(gSystem_tskStack, ".bss:taskStackSection")
UInt8 gSystem_tskStack[SYSTEM_TSK_STACK_SIZE];

/**
 *******************************************************************************
 * \brief Global object for system link.
 *
 *        All the links registers itself to system link. This global object
 *        has information about all links like link's call back functions
 *        and linkInfo
 *******************************************************************************
 */
System_CommonObj gSystem_objCommon;

/**
 *******************************************************************************
 * \brief Global object for defining which Processors needs to be enabled.
 *
 * This is required to loop around the IPC attach and other ipc functions
 * based on processors enabled in links and chains.
 *******************************************************************************
 */
const UInt32 gSystem_ipcEnableProcId[] = {
#ifdef PROC_IPU1_0_INCLUDE
    SYSTEM_PROC_IPU1_0,
#endif
#ifdef PROC_IPU1_1_INCLUDE
    SYSTEM_PROC_IPU1_1,
#endif
#ifdef PROC_DSP1_INCLUDE
    SYSTEM_PROC_DSP1,
#endif
#ifdef PROC_DSP2_INCLUDE
    SYSTEM_PROC_DSP2,
#endif
#ifdef PROC_EVE1_INCLUDE
    SYSTEM_PROC_EVE1,
#endif
#ifdef PROC_EVE2_INCLUDE
    SYSTEM_PROC_EVE2,
#endif
#ifdef PROC_EVE3_INCLUDE
    SYSTEM_PROC_EVE3,
#endif
#ifdef PROC_EVE4_INCLUDE
    SYSTEM_PROC_EVE4,
#endif
#ifdef PROC_A15_0_INCLUDE
    SYSTEM_PROC_A15_0,
#endif
    SYSTEM_PROC_MAX /* MUST be the last entry in this list */
};

/**
 *******************************************************************************
 * \brief Global object for mapping scan format enum to string.
 *
 *******************************************************************************
 */
const char *gSystem_nameScanFormat[] = SYSTEM_SCAN_FORMAT_STRINGS;

/**
 *******************************************************************************
 * \brief Global object for mapping memory type enum to string.
 *
 *******************************************************************************
 */
const char *gSystem_nameMemoryType[] = SYSTEM_MEMORY_TYPE_STRINGS;

/**
 *******************************************************************************
 * \brief Global object for mapping On/Off enum to string.
 *
 *******************************************************************************
 */
const char *gSystem_nameOnOff[] = SYSTEM_ON_OFF_STRINGS;

/**
 *******************************************************************************
 *
 * \brief This function waits for all cores to complete the application level
 *        initialization.
 *
 *        This function is effective only for the "host" processor
 *
 *******************************************************************************
 */
Void System_waitAppInitComplete()
{
    UInt32 procId;
    UInt32 initDone[SYSTEM_PROC_MAX] = {0,};
    UInt32 allInitDone;
    unsigned int state;

    initDone[SYSTEM_PROC_IPU1_0] = TRUE;

    if(System_getSelfProcId()==SYSTEM_PROC_IPU1_0)
    {
        while (1)
        {
            for(procId=0; procId<SYSTEM_PROC_MAX; procId++)
            {
                if(System_isProcEnabled(procId))
                {
                    if (FALSE == initDone[procId])
                    {
                        Utils_getAppInitState(procId, &state);

                        if (state == CORE_APP_INITSTATUS_INIT_DONE)
                        {
                            initDone[procId] = TRUE;
                        }
                    }
                }
            }

            allInitDone = TRUE;

            for(procId=0; procId<SYSTEM_PROC_MAX; procId++)
            {
                if(System_isProcEnabled(procId))
                {
                    if (FALSE == initDone[procId])
                    {
                        allInitDone = FALSE;
                        break;
                    }
                }
            }

            if(TRUE == allInitDone)
            {
                break;
            }
            BspOsal_sleep(1);
        }
    }
}

/**
 *******************************************************************************
 *
 * \brief This function waits for all cores to complete the application level
 *        initialization.
 *
 *        This function is effective only for the "host" processor
 *
 *******************************************************************************
 */
Void System_waitAppExitComplete()
{
    volatile UInt32 procId;
    UInt32 exitDone[SYSTEM_PROC_MAX] = {0,};
    UInt32 allInitDone;
    unsigned int state;

    exitDone[SYSTEM_PROC_IPU1_0] = TRUE;

    if(System_getSelfProcId()==SYSTEM_PROC_IPU1_0)
    {
        while (1)
        {
            for(procId=0; procId<SYSTEM_PROC_MAX; procId++)
            {
                if(System_isProcEnabled(procId))
                {
                    if (FALSE == exitDone[procId])
                    {
                        /* Workaround for EVEs as the Application
                         * never exits for EVE.
                         */
                        if ((procId == SYSTEM_PROC_EVE1) ||
                            (procId == SYSTEM_PROC_EVE2) ||
                            (procId == SYSTEM_PROC_EVE3) ||
                            (procId == SYSTEM_PROC_EVE4))
                        {
                            state = CORE_APP_INITSTATUS_EXIT_DONE;
                        }
                        else
                        {
                            Utils_getAppInitState((UInt32)procId, &state);
                        }

                        if (state == CORE_APP_INITSTATUS_EXIT_DONE)
                        {
                            exitDone[procId] = TRUE;
                        }
                    }
                }
            }

            allInitDone = TRUE;

            for(procId=0; procId<SYSTEM_PROC_MAX; procId++)
            {
                if(System_isProcEnabled(procId))
                {
                    if (FALSE == exitDone[procId])
                    {
                        allInitDone = FALSE;
                        break;
                    }
                }
            }

            if(TRUE == allInitDone)
            {
                break;
            }

            BspOsal_sleep(1);
        }
    }
}

/**
 *******************************************************************************
 *
 * \brief This function waits till the host triggers the core to start the
 *        application level initialization
 *
 *******************************************************************************
 */
Void System_waitAppInitTrigger()
{
    unsigned int state = CORE_APP_INITSTATUS_PRE_INIT;

    if(System_getSelfProcId()!=SYSTEM_PROC_IPU1_0)
    {
        do
        {
            Utils_getAppInitState(System_getSelfProcId(), &state);
        } while( state != CORE_APP_INITSTATUS_DO_INIT);
    }
}

/**
 *******************************************************************************
 *
 * \brief This function set the trigger to release the System_waitAppInitTrigger()
 *        for all remote processor's in the system
 *
 *        This function is effective only for the "host" processor
 *
 *******************************************************************************
 */
Void System_triggerAppInit()
{
    UInt32 procId;
    unsigned int state = CORE_APP_INITSTATUS_DO_INIT;

    if(System_getSelfProcId()==SYSTEM_PROC_IPU1_0)
    {
        for(procId=0; procId<SYSTEM_PROC_MAX; procId++ )
        {
            if(System_isProcEnabled(procId))
                Utils_setAppInitState(procId, state);
        }
    }
}

/**
 *******************************************************************************
 *
 * \brief This function set the trigger to exit the application
 *        for all remote processor's in the system
 *
 *        This function is effective only for the "host" processor
 *
 *******************************************************************************
 */
Void System_triggerAppExit()
{
    UInt32 procId;
    unsigned int state = CORE_APP_INITSTATUS_DO_EXIT;

    if(System_getSelfProcId()==SYSTEM_PROC_IPU1_0)
    {
        for(procId=0; procId<SYSTEM_PROC_MAX; procId++ )
        {
            if(System_isProcEnabled(procId))
                Utils_setAppInitState(procId, state);
        }
    }
}

/**
 *******************************************************************************
 *
 * \brief This function set the trigger to indicate exit o fthe application
 *        is complete
 *
 *        This function is effective only for the "host" processor
 *
 *******************************************************************************
 */
Void System_triggerAppExitComplete()
{
    UInt32 procId;
    unsigned int state = CORE_APP_INITSTATUS_EXIT_DONE;

    procId = System_getSelfProcId();

    Utils_setAppInitState(procId, state);
}

/**
 *******************************************************************************
 *
 * \brief This function set the trigger to release the
 *        host to come out of the System_waitAppInitComplete
 *
 *        This function is effective only for the remote processors
 *
 *******************************************************************************
 */
Void System_triggerAppInitComplete()
{
    UInt32 procId;
    unsigned int state = CORE_APP_INITSTATUS_INIT_DONE;

    procId = System_getSelfProcId();
    Utils_setAppInitState(procId, state);
}

/**
 *******************************************************************************
 *
 * \brief This function is the implementation of Idle state for system link.
 *
 *        This function initializes all the required components. It also
 *        initializes the link sub-systems like memory and performance. After
 *        that it jumps to the main function of application. Calls the use
 *        case specific function. It waits there as a task. On termination it
 *        de-initializes everything.
 *
 * \param arg0 Reserved
 * \param arg1 Reserved
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Void System_main(UArg arg0, UArg arg1)
{
    System_enumAssertCheck();

    System_initCommon();
    System_init();

    if(!System_isFastBootEnabled())
    {
        System_triggerAppInit();
        System_waitAppInitComplete();
        System_triggerAppInitComplete();

    }
    else
    {
        /* When fast boot is enabled System_waitAppInitComplete() is called
         * by Utils_syncSlaves() to sync up with other slaves.
         * This is valid only for tda3x.
         */
        if (System_getSelfProcId()!=SYSTEM_PROC_IPU1_0)
        {
            System_triggerAppInit();
            System_triggerAppInitComplete();
        }

    }

    #ifdef HCF_INCLUDE
    {
        Int32 System_hcfInit();

        System_hcfInit();
    }
    #endif

    UTILS_assert(gSystem_objCommon.chainsMainFunc != NULL);

    gSystem_objCommon.chainsMainFunc((xdc_UArg)NULL, (xdc_UArg)NULL);

    #ifdef HCF_INCLUDE
    {
        Int32 System_hcfDeInit();

        System_hcfDeInit();
    }
    #endif

    System_deInit();
    System_deInitCommon();

    System_triggerAppExit();

    /* Only IPU1-0 waits here */
    System_waitAppExitComplete();

    System_triggerAppExitComplete();

    /*
     * Jenkins script executes a DSS script. This DSS script is for nightly
     * automation. It runs the testsuite for a particular number of times and
     * then exits. The DSS script keeps checking the UART log to find a string
     * "DEMO END". Once this string is found on the console, DSS will exit.
    */
    if(System_getSelfProcId()==SYSTEM_PROC_IPU1_0)
    {
        Vps_printf("###############APPLICATION END###########################\n");
        System_printf("###############APPLICATION END###########################\n");
    }

    BspOsal_sleep(500);
}


/**
 *******************************************************************************
 *
 * \brief This function creates the task for system link.
 *
 *        It initializes the system link global object and creates the task.
 *
 * \param  chainsMainFunc   [IN] Function pointer for app start function.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 System_start(Task_FuncPtr chainsMainFunc)
{
    if(!System_isFastBootEnabled())
    {
        System_ipcStart();
    }
    else
    {
        /* When fast boot is enabled System_ipcStart() is called
         * by Utils_syncSlaves() to sync up with other slaves.
         * IPU1_0 call this after image is loaded on slave cores
         * and they are out of reset
         * This is valid only for tda3x.
         */

        if (System_getSelfProcId()!=SYSTEM_PROC_IPU1_0)
        {
            System_ipcStart();
        }
    }

    memset(&gSystem_objCommon, 0, sizeof(gSystem_objCommon));

    gSystem_objCommon.chainsMainFunc = chainsMainFunc;

    gSystem_objCommon.tsk = BspOsal_taskCreate(
                                (BspOsal_TaskFuncPtr)System_main,
                                SYSTEM_TSK_PRI,
                                gSystem_tskStack,
                                sizeof(gSystem_tskStack),
                                NULL
                            );
    UTILS_assert(gSystem_objCommon.tsk != NULL);

    return SYSTEM_LINK_STATUS_SOK;
}
/**
 *******************************************************************************
 *
 * \brief This function actually initializes components and sub-systems.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 System_initCommon()
{
    Int32 status;
    Utils_MbxCreatePrm mbxCreatePrm;

#ifdef SYSTEM_DEBUG
    Vps_printf(" SYSTEM: System Common Init in progress !!!\n");
#endif
    Utils_globalTimerInit();

    Utils_linkStatsCollectorInit(); /* Initialize Link Stat Collector */
    Utils_prfInit();

    System_ipcInit();

    /* All CPUs except IPU1-0 wait here */
    System_waitAppInitTrigger();

    Utils_mbxInit();

    /*
     * Init memory allocator
     */
    Utils_memInit();

    Utils_MbxCreatePrm_Init(&mbxCreatePrm);

    status = Utils_mbxCreate(&gSystem_objCommon.mbx, &mbxCreatePrm);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    Utils_prfLoadRegister(gSystem_objCommon.tsk, "SYSTEM  ");

    SystemLink_init();

#ifdef SYSTEM_DEBUG
    Vps_printf(" SYSTEM: System Common Init Done !!!\n");
#endif

    return status;

}

/**
 *******************************************************************************
 *
 * \brief This function actually de-initializes components and sub-systems.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 System_deInitCommon()
{
#ifdef SYSTEM_DEBUG
    Vps_printf(" SYSTEM: System Common De-Init in progress !!!\n");
#endif

    SystemLink_deInit();

    System_ipcDeInit();

    Utils_prfLoadUnRegister(gSystem_objCommon.tsk);

    Utils_mbxDelete(&gSystem_objCommon.mbx);

    /*
     * De-init memory allocator
     */
    Utils_memDeInit();

    Utils_mbxDeInit();

    Utils_prfDeInit();
    Utils_linkStatsCollectorDeInit(); /* DeInitialize Link Stat Collector */

#ifdef SYSTEM_DEBUG
    Vps_printf(" SYSTEM: System Common De-Init Done !!!\n");
#endif

    return SYSTEM_LINK_STATUS_SOK;
}

void System_memPrintHeapStatus()
{
    Utils_MemHeapStats heapStats;

    Vps_printf
          (" SYSTEM: SW Message Box Msg Pool, Free Msg Count = %d \r\n",
                Utils_mbxGetFreeMsgCount()
          );

    Utils_memGetHeapStats( UTILS_HEAPID_L2_LOCAL, &heapStats);

    if(heapStats.heapSize!=0)
    {
        Vps_printf
          (" SYSTEM: Heap = %-20s @ 0x%08x, Total size = %d B (%d KB), Free size = %d B (%d KB)\r\n",
             heapStats.heapName,
             heapStats.heapAddr,
             heapStats.heapSize,
             heapStats.heapSize/KB,
             heapStats.freeSize,
             heapStats.freeSize/KB
             );
    }
    #if 1
    Utils_memGetHeapStats( UTILS_HEAPID_DDR_CACHED_LOCAL, &heapStats);

    Vps_printf
          (" SYSTEM: Heap = %-20s @ 0x%08x, Total size = %d B (%d KB), Free size = %d B (%d KB)\r\n",
             heapStats.heapName,
             heapStats.heapAddr,
             heapStats.heapSize,
             heapStats.heapSize/KB,
             heapStats.freeSize,
             heapStats.freeSize/KB
             );
    #endif
    if(System_getSelfProcId()==SYSTEM_PROC_IPU1_0)
    {
        /* print SR heap free space info from IPU1-0 side only,
         * to avoid duplicate print's
         */

        Utils_memGetHeapStats( UTILS_HEAPID_OCMC_SR, &heapStats);

        Vps_printf
              (" SYSTEM: Heap = %-20s @ 0x%08x, Total size = %d B (%d KB), Free size = %d B (%d KB)\r\n",
                 heapStats.heapName,
                 heapStats.heapAddr,
                 heapStats.heapSize,
                 heapStats.heapSize/KB,
                 heapStats.freeSize,
                 heapStats.freeSize/KB
                 );

        Utils_memGetHeapStats( UTILS_HEAPID_DDR_CACHED_SR, &heapStats);

        Vps_printf
              (" SYSTEM: Heap = %-20s @ 0x%08x, Total size = %d B (%d MB), Free size = %d B (%d MB)\r\n",
                 heapStats.heapName,
                 heapStats.heapAddr,
                 heapStats.heapSize,
                 heapStats.heapSize/MB,
                 heapStats.freeSize,
                 heapStats.freeSize/MB
                 );

        Utils_memGetHeapStats( UTILS_HEAPID_DDR_NON_CACHED_SR0, &heapStats);

        Vps_printf
              (" SYSTEM: Heap = %-20s @ 0x%08x, Total size = %d B (%d MB), Free size = %d B (%d MB)\r\n",
                 heapStats.heapName,
                 heapStats.heapAddr,
                 heapStats.heapSize,
                 heapStats.heapSize/MB,
                 heapStats.freeSize,
                 heapStats.freeSize/MB
                 );
    }

    return;
}

/**
 *******************************************************************************
 *
 * \brief This function returns TRUE or FALSE based on FAST_BOOT_INCLUDE.
 *
 * \return  TRUE or FLASE
 *
 *******************************************************************************
 */
Bool System_isFastBootEnabled()
{
#ifdef FAST_BOOT_INCLUDE
    return TRUE;
#else
    return FALSE;
#endif
}


/**
 *******************************************************************************
 *
 * \brief Checks for the emum values compatibility between driver and Links at
 *        compile time
 *
 *        Links and chains defines the enums based on driver enums. So both
 *        value of both enums should match. This function does that at compile
 *        time.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */

int System_enumAssertCheck()
{
    UTILS_COMPILETIME_ASSERT(SYSTEM_BUFFER_ALIGNMENT == VPS_BUFFER_ALIGNMENT);
    UTILS_COMPILETIME_ASSERT(SYSTEM_MAX_PLANES == FVID2_MAX_PLANES);

    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_YUV422I_UYVY == FVID2_DF_YUV422I_UYVY);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_YUV422I_YUYV == FVID2_DF_YUV422I_YUYV);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_YUV422I_YVYU == FVID2_DF_YUV422I_YVYU);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_YUV422I_VYUY == FVID2_DF_YUV422I_VYUY);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_YUV422SP_UV == FVID2_DF_YUV422SP_UV);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_YUV422SP_VU == FVID2_DF_YUV422SP_VU);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_YUV422P == FVID2_DF_YUV422P);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_YUV420SP_UV == FVID2_DF_YUV420SP_UV);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_YUV420SP_VU == FVID2_DF_YUV420SP_VU);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_YUV420P == FVID2_DF_YUV420P);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_YUV444P == FVID2_DF_YUV444P);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_YUV444I == FVID2_DF_YUV444I);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_RGB16_565 == FVID2_DF_RGB16_565);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_RGB24_888 == FVID2_DF_RGB24_888);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_BGRA32_8888 == FVID2_DF_BGRA32_8888);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_BITMAP8 == FVID2_DF_BITMAP8);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_BITMAP1_BGRA32_OFFSET7 ==
                 FVID2_DF_BITMAP1_BGRA32_OFFSET7);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_BAYER_RAW == FVID2_DF_BAYER_RAW);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_RAW_VBI == FVID2_DF_RAW_VBI);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_MISC == FVID2_DF_MISC);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_BGR16_565_A8 == FVID2_DF_BGR16_565_A8);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_RGB16_565_A8 == FVID2_DF_RGB16_565_A8);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_BAYER_RAW    == FVID2_DF_BAYER_RAW);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_BAYER_GRBG   == FVID2_DF_BAYER_GRBG);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_BAYER_RGGB   == FVID2_DF_BAYER_RGGB);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_BAYER_BGGR   == FVID2_DF_BAYER_BGGR);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_BAYER_GBRG   == FVID2_DF_BAYER_GBRG);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_RAW_VBI      == FVID2_DF_RAW_VBI);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_RAW24        == FVID2_DF_RAW24);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_RAW16        == FVID2_DF_RAW16);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_RAW08        == FVID2_DF_RAW08);

    UTILS_COMPILETIME_ASSERT(SYSTEM_DF_INVALID == FVID2_DF_INVALID);

    UTILS_COMPILETIME_ASSERT(SYSTEM_BUF_FMT_FIELD == FVID2_BUF_FMT_FIELD);
    UTILS_COMPILETIME_ASSERT(SYSTEM_BUF_FMT_FRAME == FVID2_BUF_FMT_FRAME);

    UTILS_COMPILETIME_ASSERT(SYSTEM_BPP_BITS1 == FVID2_BPP_BITS1);
    UTILS_COMPILETIME_ASSERT(SYSTEM_BPP_BITS2 == FVID2_BPP_BITS2);
    UTILS_COMPILETIME_ASSERT(SYSTEM_BPP_BITS4 == FVID2_BPP_BITS4);
    UTILS_COMPILETIME_ASSERT(SYSTEM_BPP_BITS8 == FVID2_BPP_BITS8);
    UTILS_COMPILETIME_ASSERT(SYSTEM_BPP_BITS12 == FVID2_BPP_BITS12);
    UTILS_COMPILETIME_ASSERT(SYSTEM_BPP_BITS16 == FVID2_BPP_BITS16);
    UTILS_COMPILETIME_ASSERT(SYSTEM_BPP_BITS24 == FVID2_BPP_BITS24);
    UTILS_COMPILETIME_ASSERT(SYSTEM_BPP_BITS32 == FVID2_BPP_BITS32);
    UTILS_COMPILETIME_ASSERT(SYSTEM_BPP_BITS10 == FVID2_BPP_BITS10);
    UTILS_COMPILETIME_ASSERT(SYSTEM_BPP_BITS7 == FVID2_BPP_BITS7);
    UTILS_COMPILETIME_ASSERT(SYSTEM_BPP_BITS9 == FVID2_BPP_BITS9);
    UTILS_COMPILETIME_ASSERT(SYSTEM_BPP_BITS11 == FVID2_BPP_BITS11);
    UTILS_COMPILETIME_ASSERT(SYSTEM_BPP_BITS13 == FVID2_BPP_BITS13);
    UTILS_COMPILETIME_ASSERT(SYSTEM_BPP_BITS14 == FVID2_BPP_BITS14);
    UTILS_COMPILETIME_ASSERT(SYSTEM_BPP_BITS15 == FVID2_BPP_BITS15);
    UTILS_COMPILETIME_ASSERT(SYSTEM_BPP_BITS20 == FVID2_BPP_BITS20);
    UTILS_COMPILETIME_ASSERT(SYSTEM_BPP_BITS6 == FVID2_BPP_BITS6);

    UTILS_COMPILETIME_ASSERT(SYSTEM_FPS_85 == FVID2_FPS_85);
    UTILS_COMPILETIME_ASSERT(SYSTEM_FPS_75 == FVID2_FPS_75);
    UTILS_COMPILETIME_ASSERT(SYSTEM_FPS_72 == FVID2_FPS_72);
    UTILS_COMPILETIME_ASSERT(SYSTEM_FPS_70 == FVID2_FPS_70);
    UTILS_COMPILETIME_ASSERT(SYSTEM_FPS_60 == FVID2_FPS_60);
    UTILS_COMPILETIME_ASSERT(SYSTEM_FPS_50 == FVID2_FPS_50);
    UTILS_COMPILETIME_ASSERT(SYSTEM_FPS_30 == FVID2_FPS_30);
    UTILS_COMPILETIME_ASSERT(SYSTEM_FPS_25 == FVID2_FPS_25);
    UTILS_COMPILETIME_ASSERT(SYSTEM_FPS_24 == FVID2_FPS_24);
    UTILS_COMPILETIME_ASSERT(SYSTEM_FPS_15 == FVID2_FPS_15);
    UTILS_COMPILETIME_ASSERT(SYSTEM_FPS_10 == FVID2_FPS_10);
    UTILS_COMPILETIME_ASSERT(SYSTEM_FPS_5 == FVID2_FPS_5);

    UTILS_COMPILETIME_ASSERT(SYSTEM_SF_INTERLACED == FVID2_SF_INTERLACED);
    UTILS_COMPILETIME_ASSERT(SYSTEM_SF_PROGRESSIVE == FVID2_SF_PROGRESSIVE);
    UTILS_COMPILETIME_ASSERT(SYSTEM_SF_MAX == FVID2_SF_MAX);

    UTILS_COMPILETIME_ASSERT(SYSTEM_MT_NONTILEDMEM == VPS_VPDMA_MT_NONTILEDMEM);
    UTILS_COMPILETIME_ASSERT(SYSTEM_MT_TILEDMEM == VPS_VPDMA_MT_TILEDMEM);
    UTILS_COMPILETIME_ASSERT(SYSTEM_MT_MAX == VPS_VPDMA_MT_MAX);

    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_NTSC == FVID2_STD_NTSC);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_PAL == FVID2_STD_PAL);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_480I == FVID2_STD_480I);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_576I == FVID2_STD_576I);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_CIF == FVID2_STD_CIF);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_HALF_D1 == FVID2_STD_HALF_D1);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_D1 == FVID2_STD_D1);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_480P == FVID2_STD_480P);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_576P == FVID2_STD_576P);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_720P_60 == FVID2_STD_720P_60);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_720P_50 == FVID2_STD_720P_50);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_1080I_60 == FVID2_STD_1080I_60);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_1080I_50 == FVID2_STD_1080I_50);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_1080P_60 == FVID2_STD_1080P_60);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_1080P_50 == FVID2_STD_1080P_50);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_1080P_24 == FVID2_STD_1080P_24);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_1080P_30 == FVID2_STD_1080P_30);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_VGA_60 == FVID2_STD_VGA_60);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_VGA_72 == FVID2_STD_VGA_72);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_VGA_75 == FVID2_STD_VGA_75);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_VGA_85 == FVID2_STD_VGA_85);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_SVGA_60 == FVID2_STD_SVGA_60);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_SVGA_72 == FVID2_STD_SVGA_72);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_SVGA_75 == FVID2_STD_SVGA_75);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_SVGA_85 == FVID2_STD_SVGA_85);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_XGA_60 == FVID2_STD_XGA_60);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_XGA_DSS_TDM_60 == FVID2_STD_XGA_DSS_TDM_60);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_XGA_70 == FVID2_STD_XGA_70);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_XGA_75 == FVID2_STD_XGA_75);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_XGA_85 == FVID2_STD_XGA_85);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_WXGA_60 == FVID2_STD_WXGA_60);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_WXGA_75 == FVID2_STD_WXGA_75);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_WXGA_85 == FVID2_STD_WXGA_85);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_SXGA_60 == FVID2_STD_SXGA_60);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_SXGA_75 == FVID2_STD_SXGA_75);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_SXGA_85 == FVID2_STD_SXGA_85);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_SXGAP_60 == FVID2_STD_SXGAP_60);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_SXGAP_75 == FVID2_STD_SXGAP_75);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_UXGA_60 == FVID2_STD_UXGA_60);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_MUX_2CH_D1 == FVID2_STD_MUX_2CH_D1);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_MUX_2CH_HALF_D1 ==
        FVID2_STD_MUX_2CH_HALF_D1);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_MUX_2CH_CIF == FVID2_STD_MUX_2CH_CIF);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_MUX_4CH_D1 == FVID2_STD_MUX_4CH_D1);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_MUX_4CH_CIF == FVID2_STD_MUX_4CH_CIF);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_MUX_4CH_HALF_D1 ==
        FVID2_STD_MUX_4CH_HALF_D1);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_MUX_8CH_CIF == FVID2_STD_MUX_8CH_CIF);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_MUX_8CH_HALF_D1 ==
        FVID2_STD_MUX_8CH_HALF_D1);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_AUTO_DETECT == FVID2_STD_AUTO_DETECT);
    UTILS_COMPILETIME_ASSERT(SYSTEM_STD_CUSTOM == FVID2_STD_CUSTOM);


    UTILS_COMPILETIME_ASSERT(SYSTEM_VIFW_8BIT == FVID2_VIFW_8BIT);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIFW_10BIT == FVID2_VIFW_10BIT);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIFW_12BIT == FVID2_VIFW_12BIT);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIFW_14BIT == FVID2_VIFW_14BIT);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIFW_16BIT == FVID2_VIFW_16BIT);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIFW_18BIT == FVID2_VIFW_18BIT);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIFW_20BIT == FVID2_VIFW_20BIT);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIFW_24BIT == FVID2_VIFW_24BIT);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIFW_30BIT == FVID2_VIFW_30BIT);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIFW_1LANES == FVID2_VIFW_1LANES);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIFW_2LANES == FVID2_VIFW_2LANES);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIFW_3LANES == FVID2_VIFW_3LANES);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIFW_4LANES == FVID2_VIFW_4LANES);

    UTILS_COMPILETIME_ASSERT(SYSTEM_VIFM_SCH_ES == FVID2_VIFM_SCH_ES);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIFM_MCH_LINE_MUX_ES ==
        FVID2_VIFM_MCH_LINE_MUX_ES);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIFM_MCH_PIXEL_MUX_ES ==
        FVID2_VIFM_MCH_PIXEL_MUX_ES);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIFM_SCH_DS_HSYNC_VBLK ==
        FVID2_VIFM_SCH_DS_HSYNC_VBLK);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIFM_SCH_DS_HSYNC_VSYNC ==
        FVID2_VIFM_SCH_DS_HSYNC_VSYNC);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIFM_SCH_DS_AVID_VBLK ==
        FVID2_VIFM_SCH_DS_AVID_VBLK);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIFM_SCH_DS_AVID_VSYNC ==
        FVID2_VIFM_SCH_DS_AVID_VSYNC);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIFM_MCH_LINE_MUX_SPLIT_LINE_ES ==
        FVID2_VIFM_MCH_LINE_MUX_SPLIT_LINE_ES);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIFM_SCH_CSI2 ==
        FVID2_VIFM_SCH_CSI2);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIFM_SCH_LVDS ==
        FVID2_VIFM_SCH_LVDS);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIFM_SCH_CPI ==
        FVID2_VIFM_SCH_CPI);

    UTILS_COMPILETIME_ASSERT(SYSTEM_CAPT_BCM_FRM_DROP == VPS_CAPT_BCM_FRM_DROP);
    UTILS_COMPILETIME_ASSERT(SYSTEM_CAPT_BCM_LAST_FRM_REPEAT == VPS_CAPT_BCM_LAST_FRM_REPEAT);
    UTILS_COMPILETIME_ASSERT(SYSTEM_CAPT_BCM_CIRCULAR_FRM_REPEAT == VPS_CAPT_BCM_CIRCULAR_FRM_REPEAT);

    UTILS_COMPILETIME_ASSERT(SYSTEM_FID_TOP == FVID2_FID_TOP);
    UTILS_COMPILETIME_ASSERT(SYSTEM_FID_BOTTOM == FVID2_FID_BOTTOM);
    UTILS_COMPILETIME_ASSERT(SYSTEM_FID_FRAME == FVID2_FID_FRAME);

    UTILS_COMPILETIME_ASSERT(SYSTEM_POL_LOW == FVID2_POL_LOW);
    UTILS_COMPILETIME_ASSERT(SYSTEM_POL_HIGH == FVID2_POL_HIGH);
    UTILS_COMPILETIME_ASSERT(SYSTEM_POL_MAX == FVID2_POL_MAX);

    UTILS_COMPILETIME_ASSERT(SYSTEM_EDGE_POL_RISING == FVID2_EDGE_POL_RISING);
    UTILS_COMPILETIME_ASSERT(SYSTEM_EDGE_POL_FALLING == FVID2_EDGE_POL_FALLING);

    UTILS_COMPILETIME_ASSERT(SYSTEM_SC_DS_SET_ANTI_FLICKER ==
        VPS_SC_DS_SET_ANTI_FLICKER);
    UTILS_COMPILETIME_ASSERT(SYSTEM_SC_DS_SET_3_16 == VPS_SC_DS_SET_3_16);
    UTILS_COMPILETIME_ASSERT(SYSTEM_SC_DS_SET_4_16 == VPS_SC_DS_SET_4_16);
    UTILS_COMPILETIME_ASSERT(SYSTEM_SC_DS_SET_5_16 == VPS_SC_DS_SET_5_16);
    UTILS_COMPILETIME_ASSERT(SYSTEM_SC_DS_SET_6_16 == VPS_SC_DS_SET_6_16);
    UTILS_COMPILETIME_ASSERT(SYSTEM_SC_DS_SET_7_16 == VPS_SC_DS_SET_7_16);
    UTILS_COMPILETIME_ASSERT(SYSTEM_SC_DS_SET_8_16 == VPS_SC_DS_SET_8_16);
    UTILS_COMPILETIME_ASSERT(SYSTEM_SC_DS_SET_9_16 == VPS_SC_DS_SET_9_16);
    UTILS_COMPILETIME_ASSERT(SYSTEM_SC_DS_SET_10_16 == VPS_SC_DS_SET_10_16);
    UTILS_COMPILETIME_ASSERT(SYSTEM_SC_DS_SET_11_16 == VPS_SC_DS_SET_11_16);
    UTILS_COMPILETIME_ASSERT(SYSTEM_SC_DS_SET_12_16 == VPS_SC_DS_SET_12_16);
    UTILS_COMPILETIME_ASSERT(SYSTEM_SC_DS_SET_13_16 == VPS_SC_DS_SET_13_16);
    UTILS_COMPILETIME_ASSERT(SYSTEM_SC_DS_SET_14_16 == VPS_SC_DS_SET_14_16);
    UTILS_COMPILETIME_ASSERT(SYSTEM_SC_DS_SET_15_16 == VPS_SC_DS_SET_15_16);
    UTILS_COMPILETIME_ASSERT(SYSTEM_SC_US_SET == VPS_SC_US_SET);
    UTILS_COMPILETIME_ASSERT(SYSTEM_SC_SET_1_1 == VPS_SC_SET_1_1);

    UTILS_COMPILETIME_ASSERT( SYSTEM_VIP_CTRL_CHAN_SEL_7_0
        == VPS_VIP_CTRL_CHAN_SEL_7_0 );
    UTILS_COMPILETIME_ASSERT( SYSTEM_VIP_CTRL_CHAN_SEL_15_8
        == VPS_VIP_CTRL_CHAN_SEL_15_8 );
    UTILS_COMPILETIME_ASSERT( SYSTEM_VIP_CTRL_CHAN_SEL_23_16
        == VPS_VIP_CTRL_CHAN_SEL_23_16);
    UTILS_COMPILETIME_ASSERT( SYSTEM_VIP_CTRL_CHAN_DONT_CARE
        == VPS_VIP_CTRL_CHAN_DONT_CARE);

    UTILS_COMPILETIME_ASSERT( SYSTEM_VIP_FID_DETECT_MODE_PIN
        == VPS_VIP_FID_DETECT_MODE_PIN);
    UTILS_COMPILETIME_ASSERT( SYSTEM_VIP_FID_DETECT_MODE_VSYNC
        == VPS_VIP_FID_DETECT_MODE_VSYNC);
    UTILS_COMPILETIME_ASSERT( SYSTEM_VIP_FID_DETECT_MODE_DONT_CARE
        == VPS_VIP_FID_DETECT_MODE_DONT_CARE);

    UTILS_COMPILETIME_ASSERT(SYSTEM_VIP_SYNC_TYPE_EMB_SINGLE_422_YUV
        == VPS_VIP_SYNC_TYPE_EMB_SINGLE_422_YUV);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIP_SYNC_TYPE_EMB_2X_422_YUV
        == VPS_VIP_SYNC_TYPE_EMB_2X_422_YUV);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIP_SYNC_TYPE_EMB_4X_422_YUV
        == VPS_VIP_SYNC_TYPE_EMB_4X_422_YUV);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIP_SYNC_TYPE_EMB_LINE_YUV
        == VPS_VIP_SYNC_TYPE_EMB_LINE_YUV);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIP_SYNC_TYPE_DIS_SINGLE_YUV
        == VPS_VIP_SYNC_TYPE_DIS_SINGLE_YUV);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIP_SYNC_TYPE_EMB_SINGLE_RGB_OR_444_YUV
        == VPS_VIP_SYNC_TYPE_EMB_SINGLE_RGB_OR_444_YUV);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIP_SYNC_TYPE_EMB_2X_RGB
        == VPS_VIP_SYNC_TYPE_EMB_2X_RGB);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIP_SYNC_TYPE_EMB_4X_RGB
        == VPS_VIP_SYNC_TYPE_EMB_4X_RGB);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIP_SYNC_TYPE_EMB_LINE_RGB
        == VPS_VIP_SYNC_TYPE_EMB_LINE_RGB);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIP_SYNC_TYPE_DIS_SINGLE_8B_RGB
        == VPS_VIP_SYNC_TYPE_DIS_SINGLE_8B_RGB);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIP_SYNC_TYPE_DIS_SINGLE_24B_RGB
    == VPS_VIP_SYNC_TYPE_DIS_SINGLE_24B_RGB);

    UTILS_COMPILETIME_ASSERT(SYSTEM_VIP_LINE_CAPTURE_STYLE_HSYNC
        == VPS_VIP_LINE_CAPTURE_STYLE_HSYNC);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIP_LINE_CAPTURE_STYLE_ACTVID
        == VPS_VIP_LINE_CAPTURE_STYLE_AVID);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIP_LINE_CAPTURE_STYLE_DONT_CARE
        == VPS_VIP_LINE_CAPTURE_STYLE_DONT_CARE);

    UTILS_COMPILETIME_ASSERT(SYSTEM_VIP_FID_DETECT_MODE_PIN
        == VPS_VIP_FID_DETECT_MODE_PIN);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIP_FID_DETECT_MODE_VSYNC
        == VPS_VIP_FID_DETECT_MODE_VSYNC);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIP_FID_DETECT_MODE_DONT_CARE
        == VPS_VIP_FID_DETECT_MODE_DONT_CARE);

    UTILS_COMPILETIME_ASSERT(SYSTEM_VIP_ANC_CH_SEL_8B_LUMA_SIDE
        == VPS_VIP_ANC_CH_SEL_8B_LUMA_SIDE);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIP_ANC_CH_SEL_8B_CHROMA_SIDE
        == VPS_VIP_ANC_CH_SEL_8B_CHROMA_SIDE);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIP_ANC_CH_SEL_DONT_CARE
        == VPS_VIP_ANC_CH_SEL_DONT_CARE);

    UTILS_COMPILETIME_ASSERT(SYSTEM_VIP_REPACK_CBA_TO_CBA
        == VPS_VIP_REPACK_CBA_TO_CBA);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIP_REPACK_CBA_TO_ABC
        == VPS_VIP_REPACK_CBA_TO_ABC);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIP_REPACK_CBA_TO_BCA
        == VPS_VIP_REPACK_CBA_TO_BCA);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIP_REPACK_CBA_TO_CAB
        == VPS_VIP_REPACK_CBA_TO_CAB);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIP_REPACK_CBA_TO_ACB
        == VPS_VIP_REPACK_CBA_TO_ACB);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIP_REPACK_CBA_TO_BAC
        == VPS_VIP_REPACK_CBA_TO_BAC);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIP_REPACK_RAW16_TO_RGB565
        == VPS_VIP_REPACK_RAW16_TO_RGB565);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIP_REPACK_RAW12_SWAP
        == VPS_VIP_REPACK_RAW12_SWAP);
    UTILS_COMPILETIME_ASSERT(SYSTEM_VIP_REPACK_DONT_CARE
        == VPS_VIP_REPACK_DONT_CARE);

    UTILS_COMPILETIME_ASSERT(SYSTEM_DCTRL_DVOFMT_BT656_EMBSYNC
        == VPS_DCTRL_DVOFMT_BT656_EMBSYNC);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DCTRL_DVOFMT_BT1120_EMBSYNC
        == VPS_DCTRL_DVOFMT_BT1120_EMBSYNC);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DCTRL_DVOFMT_GENERIC_DISCSYNC
        == VPS_DCTRL_DVOFMT_GENERIC_DISCSYNC);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DCTRL_DVOFMT_MAX
        == VPS_DCTRL_DVOFMT_MAX);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DCTRL_A_OUTPUT_COMPOSITE
        == VPS_DCTRL_A_OUTPUT_COMPOSITE);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DCTRL_A_OUTPUT_SVIDEO
        == VPS_DCTRL_A_OUTPUT_SVIDEO);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DCTRL_A_OUTPUT_COMPONENT
        == VPS_DCTRL_A_OUTPUT_COMPONENT);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DCTRL_A_OUTPUT_MAX
        == VPS_DCTRL_A_OUTPUT_MAX);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DCTRL_POLARITY_ACT_LOW
        == VPS_DCTRL_POLARITY_ACT_LOW);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DCTRL_POLARITY_ACT_HIGH
        == VPS_DCTRL_POLARITY_ACT_HIGH);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DCTRL_POLARITY_MAX
        == VPS_DCTRL_POLARITY_MAX);

    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_HVSYNC_NOT_ALIGNED
        == VPS_DSS_DISPC_HVSYNC_NOT_ALIGNED);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_HVSYNC_ALIGNED
        == VPS_DSS_DISPC_HVSYNC_ALIGNED);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_HVSYNC_MAX
        == VPS_DSS_DISPC_HVSYNC_MAX);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_ZORDER0
        == VPS_DSS_DISPC_ZORDER0);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_ZORDER1
        == VPS_DSS_DISPC_ZORDER1);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_ZORDER2
        == VPS_DSS_DISPC_ZORDER2);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_ZORDER3
        == VPS_DSS_DISPC_ZORDER3);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_ZORDER_MAX
        == VPS_DSS_DISPC_ZORDER_MAX);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_OVLY_FETCH_ALLDATA
        == VPS_DSS_DISPC_OVLY_FETCH_ALLDATA);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_OVLY_FETCH_OPTIMIZED
        == VPS_DSS_DISPC_OVLY_FETCH_OPTIMIZED);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_OVLY_FETCH_MAX
        == VPS_DSS_DISPC_OVLY_FETCH_MAX);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_TRANS_COLOR_KEY_DEST
        == VPS_DSS_DISPC_TRANS_COLOR_KEY_DEST);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_TRANS_COLOR_KEY_SRC
        == VPS_DSS_DISPC_TRANS_COLOR_KEY_SRC);

    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_TRANS_COLOR_MAX
        == VPS_DSS_DISPC_TRANS_COLOR_MAX);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_CSC_LIMITED
        == VPS_DSS_DISPC_CSC_LIMITED);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_CSC_FULL
        == VPS_DSS_DISPC_CSC_FULL);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_CSC_MAX
        == VPS_DSS_DISPC_CSC_MAX);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_SC_NONE
        == VPS_DSS_DISPC_SC_NONE);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_SC_HORIZONTAL
        == VPS_DSS_DISPC_SC_HORIZONTAL);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_SC_VERTICAL
        == VPS_DSS_DISPC_SC_VERTICAL);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_SC_BOTH
        == VPS_DSS_DISPC_SC_BOTH);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_SC_MAX
        == VPS_DSS_DISPC_SC_MAX);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_OVLY_DPI1
        == VPS_DSS_DISPC_OVLY_DPI1);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_OVLY_DPI2
        == VPS_DSS_DISPC_OVLY_DPI2);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_OVLY_DPI3
        == VPS_DSS_DISPC_OVLY_DPI3);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_OVLY_HDMI
        == VPS_DSS_DISPC_OVLY_HDMI);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_OVLY_WB
        == VPS_DSS_DISPC_OVLY_WB);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_OVLY_MAX
        == VPS_DSS_DISPC_OVLY_MAX);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_PIPE_VID1
        == VPS_DSS_DISPC_PIPE_VID1);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_PIPE_VID2
        == VPS_DSS_DISPC_PIPE_VID2);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_PIPE_VID3
        == VPS_DSS_DISPC_PIPE_VID3);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_PIPE_GFX1
        == VPS_DSS_DISPC_PIPE_GFX1);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_PIPE_WB
        == VPS_DSS_DISPC_PIPE_WB);
    UTILS_COMPILETIME_ASSERT(SYSTEM_DSS_DISPC_PIPE_MAX
        == VPS_DSS_DISPC_PIPE_MAX);

    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_MPEG1
        == IVIDEO_MPEG1);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_MPEG2SP
        == IVIDEO_MPEG2SP);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_MPEG2MP
        == IVIDEO_MPEG2MP);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_MPEG2HP
        == IVIDEO_MPEG2HP);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_MPEG4SP
        == IVIDEO_MPEG4SP);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_MPEG4ASP
        == IVIDEO_MPEG4ASP);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_H264BP
        == IVIDEO_H264BP);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_H264MP
        == IVIDEO_H264MP);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_H264HP
        == IVIDEO_H264HP);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_VC1SP
        == IVIDEO_VC1SP);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_VC1MP
        == IVIDEO_VC1MP);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_VC1AP
        == IVIDEO_VC1AP);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_H264RCDO
        == IVIDEO_H264RCDO);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_RV8
        == IVIDEO_RV8);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_RV9
        == IVIDEO_RV9);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_RV10
        == IVIDEO_RV10);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_ON2VP6
        == IVIDEO_ON2VP6);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_ON2VP7
        == IVIDEO_ON2VP7);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_AVS10
        == IVIDEO_AVS10);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_SORENSONSPARK
        == IVIDEO_SORENSONSPARK);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_H263_PROFILE0
        == IVIDEO_H263_PROFILE0);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_H263_PROFILE3
        == IVIDEO_H263_PROFILE3);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_H264SVC
        == IVIDEO_H264SVC);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_MULTIVIEW
        == IVIDEO_MULTIVIEW);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_MJPEG
        == IVIDEO_MJPEG);


    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_NA_FRAME
        == IVIDEO_NA_FRAME);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_I_FRAME
        == IVIDEO_I_FRAME);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_P_FRAME
        == IVIDEO_P_FRAME);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_B_FRAME
        == IVIDEO_B_FRAME);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_IDR_FRAME
        == IVIDEO_IDR_FRAME);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_II_FRAME
        == IVIDEO_II_FRAME);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_IP_FRAME
        == IVIDEO_IP_FRAME);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_IB_FRAME
        == IVIDEO_IB_FRAME);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_PI_FRAME
        == IVIDEO_PI_FRAME);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_PP_FRAME
        == IVIDEO_PP_FRAME);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_PB_FRAME
        == IVIDEO_PB_FRAME);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_BI_FRAME
        == IVIDEO_BI_FRAME);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_BP_FRAME
        == IVIDEO_BP_FRAME);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_BB_FRAME
        == IVIDEO_BB_FRAME);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_MBAFF_I_FRAME
        == IVIDEO_MBAFF_I_FRAME);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_MBAFF_P_FRAME
        == IVIDEO_MBAFF_P_FRAME);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_MBAFF_B_FRAME
        == IVIDEO_MBAFF_B_FRAME);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_MBAFF_IDR_FRAME
        == IVIDEO_MBAFF_IDR_FRAME);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_FRAMETYPE_DEFAULT
        == IVIDEO_FRAMETYPE_DEFAULT);

    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_CONTENTTYPE_NA
        == IVIDEO_CONTENTTYPE_NA);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_PROGRESSIVE
        == IVIDEO_PROGRESSIVE);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_PROGRESSIVE_FRAME
        == IVIDEO_PROGRESSIVE_FRAME);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_INTERLACED
        == IVIDEO_INTERLACED);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_INTERLACED_FRAME
        == IVIDEO_INTERLACED_FRAME);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_INTERLACED_TOPFIELD
        == IVIDEO_INTERLACED_TOPFIELD);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_INTERLACED_BOTTOMFIELD
        == IVIDEO_INTERLACED_BOTTOMFIELD);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_CONTENTTYPE_DEFAULT
        == IVIDEO_CONTENTTYPE_DEFAULT);

    UTILS_COMPILETIME_ASSERT(SYSTEM_XDM_DEFAULT
        == XDM_DEFAULT);
    UTILS_COMPILETIME_ASSERT(SYSTEM_XDM_HIGH_QUALITY
        == XDM_HIGH_QUALITY);
    UTILS_COMPILETIME_ASSERT(SYSTEM_XDM_HIGH_SPEED
        == XDM_HIGH_SPEED);
    UTILS_COMPILETIME_ASSERT(SYSTEM_XDM_USER_DEFINED
        == XDM_USER_DEFINED);
    UTILS_COMPILETIME_ASSERT(SYSTEM_XDM_HIGH_SPEED_MED_QUALITY
        == XDM_HIGH_SPEED_MED_QUALITY);
    UTILS_COMPILETIME_ASSERT(SYSTEM_XDM_MED_SPEED_MED_QUALITY
        == XDM_MED_SPEED_MED_QUALITY);
    UTILS_COMPILETIME_ASSERT(SYSTEM_XDM_MED_SPEED_HIGH_QUALITY
        == XDM_MED_SPEED_HIGH_QUALITY);
    UTILS_COMPILETIME_ASSERT(SYSTEM_XDM_ENCODING_PRESET_MAX
        == XDM_ENCODING_PRESET_MAX);
    UTILS_COMPILETIME_ASSERT(SYSTEM_XDM_PRESET_DEFAULT
        == XDM_PRESET_DEFAULT);

    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_LOW_DELAY
        == IVIDEO_LOW_DELAY);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_STORAGE
        == IVIDEO_STORAGE);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_TWOPASS
        == IVIDEO_TWOPASS);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_NONE
        == IVIDEO_NONE);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_USER_DEFINED
        == IVIDEO_USER_DEFINED);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDEO_RATECONTROLPRESET_DEFAULT
        == IVIDEO_RATECONTROLPRESET_DEFAULT);

    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDENC2_MOTIONVECTOR_PIXEL
        == IVIDENC2_MOTIONVECTOR_PIXEL);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDENC2_MOTIONVECTOR_HALFPEL
        == IVIDENC2_MOTIONVECTOR_HALFPEL);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDENC2_MOTIONVECTOR_QUARTERPEL
        == IVIDENC2_MOTIONVECTOR_QUARTERPEL);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDENC2_MOTIONVECTOR_EIGHTHPEL
        == IVIDENC2_MOTIONVECTOR_EIGHTHPEL);
    UTILS_COMPILETIME_ASSERT(SYSTEM_IVIDENC2_MOTIONVECTOR_MAX
        == IVIDENC2_MOTIONVECTOR_MAX);

    return SYSTEM_LINK_STATUS_SOK;
}

Int32 SystemLink_registerHandler(SystemLink_CmdHandler handler)
{
    int i, freeIdx = -1, status = SYSTEM_LINK_STATUS_EFAIL;
    Bool registerHandler = TRUE;

    for(i=0; i<SYSTEM_COMMON_MAX_CMD_HANDLERS; i++)
    {
        if(gSystem_objCommon.systemLinkCmdHandler[i]==NULL)
        {
            freeIdx = i;
        }
        if(gSystem_objCommon.systemLinkCmdHandler[i]==handler)
        {
            /* already registered, ignore */
            registerHandler = FALSE;
        }
    }

    if(registerHandler && freeIdx >= 0)
    {
        gSystem_objCommon.systemLinkCmdHandler[freeIdx] = handler;
        status = SYSTEM_LINK_STATUS_SOK;
    }
    if(registerHandler==FALSE)
    {
        /* handler already registered, ignore and return success */
        status = SYSTEM_LINK_STATUS_SOK;
    }

    return status;
}


Int32 SystemLink_unregisterHandler(SystemLink_CmdHandler handler)
{
    int i;

    for(i=0; i<SYSTEM_COMMON_MAX_CMD_HANDLERS; i++)
    {
        if(gSystem_objCommon.systemLinkCmdHandler[i]==handler)
        {
            gSystem_objCommon.systemLinkCmdHandler[i] = NULL;
            break;
        }
    }

    return SYSTEM_LINK_STATUS_SOK;
}

Void SystemLink_userCmdHandler(UInt32 cmd, Void * pPrm)
{
    int i;

    for(i=0; i<SYSTEM_COMMON_MAX_CMD_HANDLERS; i++)
    {
        if(gSystem_objCommon.systemLinkCmdHandler[i]!=NULL)
        {
            gSystem_objCommon.systemLinkCmdHandler[i](cmd, pPrm);
        }
    }
}

UInt32 System_useLinkMemAllocInfo(System_LinkMemAllocInfo *info)
{
    UInt32 useInfo = TRUE;

    if(info->memAddr == (UInt32) NULL
        ||
       info->memSize == 0
        )
    {
        useInfo = FALSE;
    }

    return useInfo;
}

Void System_assertLinkMemAllocOutOfMem(
                    System_LinkMemAllocInfo *info,
                    char *linkName)
{
    if(System_useLinkMemAllocInfo(info)==TRUE)
    {
        if(info->memAllocOffset > info->memSize)
        {
            /* Insufficient memory provided, assert */
            Vps_printf(" %s: Insuffcient memory provided by user !!!",
                            linkName);
            Vps_printf(" %s: Memory requirement is %d bytes.",
                            linkName,
                            info->memAllocOffset);
            Vps_printf(" %s: Memory provided by user is %d bytes.",
                            linkName,
                            info->memSize);
            Vps_printf(" %s: Additional Memory required is %d bytes.",
                            linkName,
                            info->memAllocOffset-info->memSize);
            Vps_printf(" %s: ERROR: Cannot continue execution !!!",
                            linkName);
            UTILS_assert(0);
        }
        else
        if(info->memSize > info->memAllocOffset)
        {
            /* Extra memory provided, log and continue */
            Vps_printf(" %s: Memory requirement is %d bytes.",
                            linkName,
                            info->memAllocOffset);
            Vps_printf(" %s: Memory provided by user is %d bytes.",
                            linkName,
                            info->memSize);
            Vps_printf(" %s: Unused Memory is %d bytes.",
                            linkName,
                            info->memSize-info->memAllocOffset);
        }
    }
}

Void System_resetLinkMemAllocInfo(System_LinkMemAllocInfo *info)
{
    info->memAllocOffset = 0;
}

Ptr System_allocLinkMemAllocInfo(System_LinkMemAllocInfo *info,
                UInt32 size,
                UInt32 align)
{
    UInt32 addr;

    if(System_useLinkMemAllocInfo(info)==FALSE)
    {
        addr = (UInt32)NULL;
    }
    else
    {
        if(align<sizeof(UInt32))
            align = sizeof(UInt32);

        /* calculate address based on current allocated offset */
        addr = info->memAddr + info->memAllocOffset;

        /* increment addr based on alignment requested */
        addr = SystemUtils_align(addr, align);

        /* calculate new offset into user memory buffer, based on alignement */
        info->memAllocOffset =
            addr - info->memAddr;

        /* increment allocated offset */
        info->memAllocOffset += size;

        /* check for memory out of bounds */
        if(info->memAllocOffset
            >
            info->memSize
          )
        {
            /*
             * Memory provided by user is not sufficient
             * Allow execution to continue with a warning
             * Later execution will assert, with total memory requirement
             * logged to allow user to increase memory size and try again
             */
            Vps_printf(" SYSTEM: WARNING: Memory address out of bounds"
                       " (0x%08x, %d B). Memory region is [ 0x%08x, %d B ]"
                       " !!!\n",
                        addr,
                        size,
                        info->memAddr,
                        info->memSize
                       );
        }
    }

    return ((Ptr)addr);
}

