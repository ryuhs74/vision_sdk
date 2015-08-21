/*
 *******************************************************************************
 *
 * Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \file utils_temperature.c
 *
 * \brief  APIs to execute CPU Junction temperature and alerts an interrupt on
 *         core
 *
 * \version 0.0 (Jan 2015) : [CM] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/utils_common/include/utils_temperature.h>
#include <ti/sysbios/family/shared/vayu/IntXbar.h>
#include "irq_xbar_interrupt_ids.h"
#include <ti/sysbios/knl/Task.h>
#include "soc_defines.h"
#include "platform.h"
#include "soc.h"

/**
 *******************************************************************************
 * \brief Macro
 *******************************************************************************
 */

#define TEMP_IRQ_NO                      (61)
/**< \brief IRQ number for Temperature Sensor Interrupt */

#define HOT_EVT_TEMP_THRESH              (80000)
/**< \brief Default HOT temperature Threshold Value */

#define MAX_HOT_EVT_TEMP_THRESH          (120000)
/**< \brief Maximum HOT temperature Threshold Value */

#define COLD_EVT_TEMP_THRESH             (10000)
/**< \brief Cold temperature Threshold Value */

#define MIN_COLD_EVT_TEMP_THRESH         (00000)
/**< \brief Minimum COLD temperature Threshold Value */

#define DEFAULT_STEP_SIZE                (5000)
/**< \brief Default Step Size */

#define DEFAULT_COUNTER_DELAY            (PMHAL_BGAP_BAND_GAP_1_MS)
/**< \brief Default value of the counter delay */

#if defined(TDA2XX_BUILD) || defined(TDA2EX_BUILD)
char *voltageDomain_t_names[] =
{
    "PMHAL_PRCM_VD_MPU",
    "PMHAL_PRCM_VD_CORE",
    "PMHAL_PRCM_VD_IVAHD",
    "PMHAL_PRCM_VD_DSPEVE",
    "PMHAL_PRCM_VD_GPU",
};
#elif defined(TDA3XX_BUILD)
char *voltageDomain_t_names[] =
{
    "PMHAL_PRCM_VD_CORE",
    "PMHAL_PRCM_VD_DSPEVE",
};
#endif
/**< Voltage domains present in Platforms  */

/**
 ******************************************************************************
 *
 * \brief Utils Temperature Object.
 *        This object is used to save the hot and cold event thresholds and
 *        the step size for thermal hysterisis. In case of limp home mode
 *        the structure holds the variables to increase or reset FPS.
 *
 ******************************************************************************
 */
typedef struct {
    pmhalPrcmVdId_t voltId;
    /*< Voltage Domain ID of the Temperature object */
    Int32   hotEventThreshold;
    /*< Threshold at which the Hot event is raised. Fps is reduced at this
     *  point.
     */
    Int32   coldEventThreshold;
    /*< Threshold at which the cold event is raised. The FPS is reset to
     *  application programmed after this point.
     */
    UInt32  stepSize;
    /*< Step difference between the temperatures of hot and cold events
     *  between hot and cold event threshold. Used in thermal hysterisis
     */
    UInt32 limpHomeModeActive;
    /*< Flag to Indicate if Limp Home Mode is Active */
} Utils_TempObj;

Utils_TempObj gUtils_tempObj[PMHAL_BGAP_NUM_FSM];
BspOsal_IntrHandle gUtils_tempHwiHandle;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

static void Utils_tempBgapIntInit(void);

Void Utils_tempBgapEventIsr(UArg arg);

/**
 *******************************************************************************
 *
 * \brief   DeInit during system turndown
 *
 * \return NONE
 *
 *******************************************************************************
*/
Void Utils_tempConfigDeInit()
{
/*
 * Deinit if any thread created here.
 */
    Hwi_clearInterrupt(TEMP_IRQ_NO);
    Hwi_disableInterrupt(TEMP_IRQ_NO);

    BspOsal_unRegisterIntr(&gUtils_tempHwiHandle);

    IntXbar_disconnectIRQ(TEMP_IRQ_NO);
}

/**
 *******************************************************************************
 *
 * \brief Prepare the Temperature Configuration .
 *          The hot cold and step size are set for default
 *
 * \return NONE
 *
 *******************************************************************************
 */
Void Utils_tempConfigInit()
{
    pmhalPrcmVdId_t voltId = PMHAL_PRCM_VD_CORE;

    PMHALBgapSetMeasureDelay(PMHAL_BGAP_BAND_GAP_IMMEDIATE);
    /* For Errata ID: i827 */
    PMHALBgapSetSIldeMode(PMHAL_BGAP_BAND_GAP_NO_IDLE);

    for(voltId = PMHAL_PRCM_VD_MIN;
            voltId < (pmhalPrcmVdId_t)PMHAL_BGAP_NUM_FSM;
            voltId++)
    {
        gUtils_tempObj[voltId].voltId = voltId;
        gUtils_tempObj[voltId].hotEventThreshold = HOT_EVT_TEMP_THRESH;
        gUtils_tempObj[voltId].coldEventThreshold = COLD_EVT_TEMP_THRESH;
        Vps_printf(
                " UTILS_TEMP: Voltage [%s], Hot Threshold = [%d.%d], \
Cold Threshold = [%d.%d]\n",
                voltageDomain_t_names[voltId],
                gUtils_tempObj[voltId].hotEventThreshold/1000,
                abs(gUtils_tempObj[voltId].hotEventThreshold) % 1000,
                gUtils_tempObj[voltId].coldEventThreshold/1000,
                abs(gUtils_tempObj[voltId].coldEventThreshold) % 1000);

        gUtils_tempObj[voltId].stepSize = DEFAULT_STEP_SIZE;
        gUtils_tempObj[voltId].limpHomeModeActive = UTILS_TEMP_LIMP_HOME_INACTIVE;
        PMHALBgapSetColdThreshold(voltId,
                gUtils_tempObj[voltId].coldEventThreshold);
        PMHALBgapSetHotThreshold(voltId,
                gUtils_tempObj[voltId].hotEventThreshold);
        PMHALBgapClearState(voltId);
    }
    Utils_tempBgapIntInit();
    PMHALBgapSetMeasureDelay(DEFAULT_COUNTER_DELAY);
}

/**
 *******************************************************************************
 *
 * \brief Change Hot Threshold Value for given Voltage Id
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_tempChangeHotThreshold(Int32 Value)
{
    UInt32 cookie;
    pmhalPrcmVdId_t voltId = PMHAL_PRCM_VD_CORE;
    cookie = Hwi_disable();
    /* Disable the system interrupt */
    Hwi_disableInterrupt(TEMP_IRQ_NO);
    PMHALBgapSetMeasureDelay(PMHAL_BGAP_BAND_GAP_IMMEDIATE);
    for(voltId = PMHAL_PRCM_VD_MIN;
            voltId < (pmhalPrcmVdId_t)PMHAL_BGAP_NUM_FSM;
            voltId++)
    {
        gUtils_tempObj[voltId].hotEventThreshold = Value;
        PMHALBgapSetHotThreshold(voltId,
                gUtils_tempObj[voltId].hotEventThreshold);
        PMHALBgapClearState(voltId);
    }
    BspOsal_sleep(50);
    /* Clear any pending interrupts */
    Hwi_clearInterrupt(TEMP_IRQ_NO);
    PMHALBgapSetMeasureDelay(DEFAULT_COUNTER_DELAY);
     /* Enable the system interrupt */
    Hwi_enableInterrupt(TEMP_IRQ_NO);
    Hwi_restore(cookie);
}

/**
 *******************************************************************************
 *
 * \brief Change Cold Threshold Value for given Voltage Id
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_tempChangeColdThreshold(Int32 Value)
{
    UInt32 cookie;
    pmhalPrcmVdId_t voltId = PMHAL_PRCM_VD_CORE;
    cookie = Hwi_disable();
    /* Disable the system interrupt */
    Hwi_disableInterrupt(TEMP_IRQ_NO);
    PMHALBgapSetMeasureDelay(PMHAL_BGAP_BAND_GAP_IMMEDIATE);
    for(voltId = PMHAL_PRCM_VD_MIN;
            voltId < (pmhalPrcmVdId_t)PMHAL_BGAP_NUM_FSM;
            voltId++)
    {
        gUtils_tempObj[voltId].coldEventThreshold = Value;
        PMHALBgapSetColdThreshold(voltId,
                gUtils_tempObj[voltId].coldEventThreshold);
        PMHALBgapClearState(voltId);
    }
    BspOsal_sleep(50);
    /* Clear any pending interrupts */
    Hwi_clearInterrupt(TEMP_IRQ_NO);
    PMHALBgapSetMeasureDelay(DEFAULT_COUNTER_DELAY);
     /* Enable the system interrupt */
    Hwi_enableInterrupt(TEMP_IRQ_NO);
    Hwi_restore(cookie);
}

/**
 *******************************************************************************
 *
 * \brief Change the Temperature Step Window
 *
 * \param   Value     New Step Size Value
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_tempChangeStepSize(UInt32 Value)
{
    UInt32 cookie;
    pmhalPrcmVdId_t voltId = PMHAL_PRCM_VD_CORE;

    cookie = Hwi_disable();
    for(voltId = PMHAL_PRCM_VD_MIN;
            voltId < (pmhalPrcmVdId_t)PMHAL_BGAP_NUM_FSM;
            voltId++)
    {
        gUtils_tempObj[voltId].stepSize = Value;
    }
    Hwi_restore(cookie);
}

/**
 *******************************************************************************
 *
 * \brief Read the Temperature Hot Threshold
 *
 * \param   voltId     Voltage Id of the Temperature Sensor.
 *
 * \return  Hot event Threshold
 *
 *******************************************************************************
 */
Int32 Utils_tempGetHotThreshold(UInt32 voltId)
{
    return gUtils_tempObj[voltId].hotEventThreshold;
}

/**
 *******************************************************************************
 *
 * \brief Read the Temperature Cold Threshold
 *
 * \param   voltId     Voltage Id of the Temperature Sensor.
 *
 * \return  Cold event Threshold
 *
 *******************************************************************************
 */
Int32 Utils_tempGetColdThreshold(UInt32 voltId)
{
    return gUtils_tempObj[voltId].coldEventThreshold;
}

/**
 *******************************************************************************
 *
 * \brief Read the Temperature Step Size
 *
 * \param   voltId     Voltage Id of the Temperature Sensor.
 *
 * \return  Step Size by which to scale hot and cold thresholds.
 *
 *******************************************************************************
 */
Int32 Utils_tempGetStepSize(UInt32 voltId)
{
    return gUtils_tempObj[voltId].stepSize;
}

/**
 *******************************************************************************
 *
 * \brief Read the Current Temperature
 *
 * \param   voltId     Voltage Id of the Temperature Sensor.
 *
 * \return  Current Temperature in milli Degrees.
 *
 *******************************************************************************
 */
Int32 Utils_tempGetCurrTemperature(UInt32 voltId)
{
    Int32 retVal = PM_SUCCESS;
    pmhalBgapRange_t currTempRange = {0, 0};

    retVal = PMHALBgapGetCurrTemperature((pmhalPrcmVdId_t)voltId,
                                            &currTempRange);
    if (PM_SUCCESS == retVal)
    {
        return currTempRange.maxTemp;
    }
    else
    {
        return UTILS_TEMP_INVALID;
    }
}

/**
 *******************************************************************************
 *
 * \brief Print the Thermal Hot Thresholds for all Voltage Domains
 *
 * \return NONE
 *
 *******************************************************************************
 */
Void Utils_tempReadAllHotThreshold(Void)
{
    pmhalPrcmVdId_t voltId = PMHAL_PRCM_VD_CORE;
    for(voltId = PMHAL_PRCM_VD_MIN;
            voltId < (pmhalPrcmVdId_t)PMHAL_BGAP_NUM_FSM;
            voltId++)
    {
        Vps_printf(" UTILS_TEMP: Voltage Domain [ %s ] HOT Threshold = [%d.%d]\n",
             pmhalVoltageDomain_t_names[voltId],
             Utils_tempGetHotThreshold(voltId)/1000,
             abs(Utils_tempGetHotThreshold(voltId))%1000);
    }

}

/**
 *******************************************************************************
 *
 * \brief Print the Thermal Cold Thresholds for all Voltage Domains
 *
 * \return NONE
 *
 *******************************************************************************
 */
Void Utils_tempReadAllColdThreshold(Void)
{
    pmhalPrcmVdId_t voltId = PMHAL_PRCM_VD_CORE;
    for(voltId = PMHAL_PRCM_VD_MIN;
            voltId < (pmhalPrcmVdId_t)PMHAL_BGAP_NUM_FSM;
            voltId++)
    {
        Vps_printf(" UTILS_TEMP: Voltage Domain [ %s ] COLD Threshold = [%d.%d]\n",
             pmhalVoltageDomain_t_names[voltId],
             Utils_tempGetColdThreshold(voltId)/1000,
             abs(Utils_tempGetColdThreshold(voltId))%1000);
    }
}

/**
 *******************************************************************************
 *
 * \brief Update the state of the temperature object for whether the limp
 *        home mode is active or not.
 *
 * \param voltId     Voltage Domain for which the Limp Home mode needs to be
 *                   set.
 * \param state      UTILS_TEMP_LIMP_HOME_ACTIVE or
 *                   UTILS_TEMP_LIMP_HOME_INACTIVE
 *
 * \return NONE
 *
 *******************************************************************************
 */
Void Utils_tempUpdateLimpHomeState(UInt32 voltId, UInt32 state)
{
    /* Can take any value like UTILS_TEMP_LIMP_HOME_ACTIVE or
     * UTILS_TEMP_LIMP_HOME_INACTIVE*/
    gUtils_tempObj[voltId].limpHomeModeActive = state & 0x1;
}

/**
 *******************************************************************************
 *
 * \brief Update the state of the temperature object for whether the limp
 *        home mode is active or not.
 *
 * \param state      UTILS_TEMP_LIMP_HOME_ACTIVE or
 *                   UTILS_TEMP_LIMP_HOME_INACTIVE
 *
 * \return NONE
 *
 *******************************************************************************
 */
Void Utils_tempUpdateAllVoltLimpHomeState(UInt32 state)
{
    pmhalPrcmVdId_t voltId = PMHAL_PRCM_VD_CORE;
    for(voltId = PMHAL_PRCM_VD_MIN;
            voltId < (pmhalPrcmVdId_t)PMHAL_BGAP_NUM_FSM;
            voltId++)
    {
        /* Can take any value like UTILS_TEMP_LIMP_HOME_ACTIVE or
         * UTILS_TEMP_LIMP_HOME_INACTIVE*/
        gUtils_tempObj[voltId].limpHomeModeActive = state & 0x1;
    }
}

/**
 *******************************************************************************
 *
 * \brief Get the status which indicates if limp Home mode is enabled or
 *        disabled.
 *
 * \return state       UTILS_TEMP_LIMP_HOME_ACTIVE or
 *                     UTILS_TEMP_LIMP_HOME_INACTIVE
 *
 *******************************************************************************
 */
UInt32 Utils_tempGetLimpHomeState(Void)
{
    pmhalPrcmVdId_t voltId = PMHAL_PRCM_VD_CORE;
    UInt32 state = 0;
    for(voltId = PMHAL_PRCM_VD_MIN;
            voltId < (pmhalPrcmVdId_t)PMHAL_BGAP_NUM_FSM;
            voltId++)
    {
        state = state | gUtils_tempObj[voltId].limpHomeModeActive;
    }
    return state;
}

/* -------------------------------------------------------------------------- */
/*                 Internal Function Definitions                              */
/* -------------------------------------------------------------------------- */

/*******************************************************************************
 * \brief  Configure the Temperature Sensor Interrupt
 *
 * \return None
 *
 *******************************************************************************
 */
static void Utils_tempBgapIntInit(void)
{
    IntXbar_connectIRQ(TEMP_IRQ_NO, CTRL_MODULE_CORE_IRQ_THERMAL_ALERT);

    /* This task sleep is required to make sure that the spurious interrupts
     * if any arrive at the IPU before proceeding.
     */
    BspOsal_sleep(50);
    /* Follow this sequence to resolve silicon Errata ID: i813 which can
     * cause a spurious interrupt.
     */
    Hwi_clearInterrupt(TEMP_IRQ_NO);

    Vps_printf(" UTILS_TEMP: TEMPERATUE INTERRUPT: HWI Create for INT%d !!!\n", TEMP_IRQ_NO);

    gUtils_tempHwiHandle = BspOsal_registerIntr(TEMP_IRQ_NO,
                    (BspOsal_IntrFuncPtr)Utils_tempBgapEventIsr,
                    (void*)TEMP_IRQ_NO
                    );

    if (gUtils_tempHwiHandle == NULL)
    {
        Vps_printf(" UTILS_TEMP: TEMPERATUE INTERRUPT: HWI Create Failed !!!\n");
        UTILS_assert(0);
    }

    /* Clear any previous interrupts to ensure we are not getting false interrupts */
    Hwi_clearInterrupt(TEMP_IRQ_NO);

    /* Enable the interrupt */
    Hwi_enableInterrupt(TEMP_IRQ_NO);
}

/**
 *******************************************************************************
 * \brief  Sensor interrupt service routine. This will Enable the Temperature
 *          sensor interrupt.
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_tempBgapEventIsr(UArg arg)
{
    pmhalPrcmVdId_t voltId;
    volatile UInt32 hotStatus[PMHAL_BGAP_NUM_FSM] = {0U};
    volatile UInt32 coldStatus[PMHAL_BGAP_NUM_FSM] = {0U};
    UInt32 counterChangedFlag = 0U;
#ifdef TEMP_DEBUG
    Int32 retVal = PM_SUCCESS;
    pmhalBgapRange_t currTempRange = {0, 0};
#endif
    UInt32 cookie;

    /* Disabling the global interrupts */
    cookie = Hwi_disable();
    /* Disable the system interrupt */
    Hwi_disableInterrupt(TEMP_IRQ_NO);
#ifdef TEMP_DEBUG
    Vps_printf(
            " UTILS_TEMP: ----------------------------------------------------\n");
#endif
    for(voltId = PMHAL_PRCM_VD_MIN;
            voltId < (pmhalPrcmVdId_t)PMHAL_BGAP_NUM_FSM;
            voltId++)
    {
        /* First read all the status bits in one go. Reading it in phases is
         * found to give incorrect values.
         */
        PMHALBgapGetHotAlertStatus(voltId,(UInt32 *)&hotStatus[voltId]);
        PMHALBgapGetColdAlertStatus(voltId,(UInt32 *)&coldStatus[voltId]);
        counterChangedFlag = 1U;
    }
    if (1U == counterChangedFlag)
    {
        PMHALBgapSetMeasureDelay(PMHAL_BGAP_BAND_GAP_IMMEDIATE);
    }
    for(voltId = PMHAL_PRCM_VD_MIN;
            voltId < (pmhalPrcmVdId_t)PMHAL_BGAP_NUM_FSM;
            voltId++)
    {
        /* Do a clear state before reading the current temperature.
         * Ensures the temperature read is correct.
         */
        PMHALBgapClearState(voltId);
#ifdef TEMP_DEBUG
        retVal = PMHALBgapGetCurrTemperature((pmhalPrcmVdId_t)voltId,
                                            &currTempRange);
        if (PM_SUCCESS != retVal)
        {
            Vps_printf(
                " UTILS_TEMP: Error! Could not read current temperature!! [%s]\n",
                voltageDomain_t_names[voltId]);
        }
        else
        {
            Vps_printf(
                " UTILS_TEMP: [%d.%d] deg C : Voltage Domain [%s] \n",
                currTempRange.maxTemp/1000,(abs(currTempRange.maxTemp) % 1000),
                voltageDomain_t_names[voltId]);

        }
#endif
        if(1U == hotStatus[voltId])
        {
            if(gUtils_tempObj[voltId].coldEventThreshold <= MIN_COLD_EVT_TEMP_THRESH)
            {
                PMHALBgapEnableColdEvent(voltId);
                /* App can react as temperature has increased more
                 * than threshold.
                 */
            }
            if(gUtils_tempObj[voltId].hotEventThreshold < MAX_HOT_EVT_TEMP_THRESH)
            {
#ifdef TEMP_DEBUG
                Vps_printf(
                        " UTILS_TEMP: HOT: Setting Hot Threshold to [%d.%d] deg C\
 for Voltage Domain [%s]\n",
                             gUtils_tempObj[voltId].hotEventThreshold/1000,
                             gUtils_tempObj[voltId].hotEventThreshold % 1000,
                             voltageDomain_t_names[voltId]);
#endif
                System_linkControl(
                        SYSTEM_LINK_ID_IPU1_0,
                        UTILS_TEMP_CMD_EVENT_HOT,
                        &gUtils_tempObj[voltId].voltId,
                        sizeof(pmhalPrcmVdId_t),
                        FALSE);

            }
            else
            {
                PMHALBgapDisableHotEvent(voltId);
                Vps_printf(
                        " UTILS_TEMP: EMERGENCY!! Temperature MAX Threshold Reached...\
 Disabling Hot Event \n");
                /* Once a HOT event is received than the max hot event,
                 * App can post a semaphore to turn off certain modules to cool
                 * System , or take precaution mesaures
                 */
            }
#ifdef TEMP_DEBUG
            Vps_printf(" UTILS_TEMP: HOT: Setting Cold Threshold to [%d.%d] deg C\
 for Voltage Domain [%s]\n", gUtils_tempObj[voltId].coldEventThreshold/1000,
                             gUtils_tempObj[voltId].coldEventThreshold % 1000,
                             voltageDomain_t_names[voltId]);
#endif

        }
        if(1U == coldStatus[voltId])
        {
            if(gUtils_tempObj[voltId].hotEventThreshold >= MAX_HOT_EVT_TEMP_THRESH)
            {
                PMHALBgapEnableHotEvent(voltId);
                /* App can restart certain modules as the
                 * temperature has reduced.
                 */
            }
            if(gUtils_tempObj[voltId].coldEventThreshold > MIN_COLD_EVT_TEMP_THRESH)
            {
#ifdef TEMP_DEBUG
            Vps_printf(" UTILS_TEMP: COLD: Setting Cold Threshold to [%d.%d] deg C\
 for Voltage Domain [%s]\n", gUtils_tempObj[voltId].coldEventThreshold/1000,
                             gUtils_tempObj[voltId].coldEventThreshold % 1000,
                             voltageDomain_t_names[voltId]);
#endif
                 System_linkControl(
                        SYSTEM_LINK_ID_IPU1_0,
                        UTILS_TEMP_CMD_EVENT_COLD,
                        &gUtils_tempObj[voltId].voltId,
                        sizeof(pmhalPrcmVdId_t),
                        FALSE);
            }
            else
            {
                PMHALBgapDisableColdEvent(voltId);
                Vps_printf(
                        " UTILS_TEMP: EMERGENCY!! Temperature MIN Threshold Reached...\
 Disabling Cold Event\n ");
            }

#ifdef TEMP_DEBUG
            Vps_printf(" UTILS_TEMP: COLD: Setting Hot Threshold to [%d.%d] deg C\
 for Voltage Domain [%s]\n", gUtils_tempObj[voltId].hotEventThreshold/1000,
                             gUtils_tempObj[voltId].hotEventThreshold % 1000,
                             voltageDomain_t_names[voltId]);
#endif
        }
    }

    /* Clear any pending interrupts */
    Hwi_clearInterrupt(TEMP_IRQ_NO);
    if (1U == counterChangedFlag)
    {
        PMHALBgapSetMeasureDelay(DEFAULT_COUNTER_DELAY);
    }

    /* Restore interrupts */
    Hwi_restore(cookie);

}
