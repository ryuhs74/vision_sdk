/**
 *                                                                            
 * Copyright (c) 2015 CAMMSYS - http://www.cammsys.net/                       
 *                                                                            
 * All rights reserved.                                                       
 *                                                                            
 * @file	uartCmd_priv.h
 * @author  Raven
 * @date	Sep 21, 2015
 * @brief	                                                                     
 */
#ifndef EXAMPLES_TDA2XX_SRC_LINKS_UARTCMD_UARTCMD_PRIV_H_
#define EXAMPLES_TDA2XX_SRC_LINKS_UARTCMD_UARTCMD_PRIV_H_

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */


/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

#define UARTCMD_LINK_TSK_STACK_SIZE        (SYSTEM_DEFAULT_TSK_STACK_SIZE)
#define UARTCMD_LINK_TSK_PRI               (12)


#define GET_START_TAG(buf)		(buf[0])
#define GET_TARGET_ID(buf)		(buf[1])
#define GET_SOURCE_ID(buf)		(buf[2])
#define GET_LENGTH(buf)			(buf[3] << 8 | buf[4])
#define GET_BCC(buf)			(buf[5+(buf[3]<<8|buf[4])])
#define GET_END_TAG(buf)		(buf[6+(buf[3]<<8|buf[4])])
#define GET_COMMAND(buf)		(buf[5])
#define GET_ACK(buf)			(buf[6])
#define GET_ARG1(buf)			(buf[6])
#define GET_ARG2(buf)			(buf[7])
#define GET_ARG3(buf)			(buf[8])
#define GET_ARG4(buf)			(buf[9])

#define SET_START_TAG(buf)		{buf[0] = START_TAG;}
#define SET_TARGET_ID(buf, tid)	{buf[1] = tid;}
#define SET_SOURCE_ID(buf, sid)	{buf[2] = sid;}
#define SET_LENGTH(buf, len)	{buf[3] = (len>>8)&0xFF; buf[4] = (len&0xff);}
#define SET_DATA(buf,data,len)	{memcpy(&buf[5], data, len);}
#define SET_BCC(buf, bcc)		{buf[5+(buf[3]<<8|buf[4])] = bcc;}
#define SET_END_TAG(buf)		{buf[6+(buf[3]<<8|buf[4])] = END_TAG;}


#define MSG_NO_ERROR 			(0)
#define MSG_ERROR_BAD_PARAMETER	(-1)
#define MSG_ERROR_NAK_RECEIVED	(-2)
#define MSG_ERROR_TIMEOUT		(-3)
#define MSG_ERROR_UNKNOWN_DEVICE (-4)
#define MSG_ERROR_BCC			(-5)
#define MSG_ERROR_WAIT_END  	(-6)

#define MSG_DATA_INDEX			(5)
#define MSG_HEADER_LEN			(7)
#define OBD_FW_HEADER_LEN		(8)
#define DATA_MAX_LEN			(1024)
#define MIN_MSG_SIZE			(MSG_HEADER_LEN+1)
#define MAX_MSG_SIZE			(MSG_HEADER_LEN+2+OBD_FW_HEADER_LEN+DATA_MAX_LEN)	// 2 is command and ACK

#define MAX_RETRY_CNT			(3)

#define START_TAG				(0x02)
#define END_TAG					(0x03)
#define ACK						(0x06)
#define NAK						(0x15)

#define DEV_ID_AVM_MICOM		(0xA0)
#define DEV_ID_AVM_DSP			(0xA1)
#define DEV_ID_OBD_MICOM		(0x10)
#define DEV_ID_BBOX_EDR			(0x21)
#define DEV_ID_BBOX_AEGIS		(0xB0)


typedef enum
{
	CMD_REQ_DSP_STANDBY = 0x11,// Send standby command when ACC off, and then perprom power sequencing
	CMD_REQ_DSP_STATUS,	// Request status of DSP, DSP response AVM/FACTORY/UPDATE mode
	CMD_REQ_MICOM_VER,		// Send version of AVM MICOM
	CMD_SEND_IRDA_KEY,		// Send IrDA command with Key value
	CMD_SEND_RGEAR,			// Send Rear Gear On/Off
	CMD_CONTROL_VIDEO_OUT,	// CVBS1/CVBS2 output enable
	CMD_SDCARD_HPD,
	CMD_REQ_VBAT_ADC,		// Request VBAT ADC value of MICOM
	CMD_DSP_REQ_TEST,

	CMD_SEND_TURN_SIGNAL = (0x20),
	CMD_SEND_BUTTON_PRESSED,
	CMD_REQ_LVDS_STATUS,
	CMD_REQ_AUDIO_OUT,

	CMD_REQ_HDMI_ONOFF = (0x39),
	CMD_REQ_ETHERNET_ONOFF = (0x40),
	CMD_SEND_HDMI_ON_OFF,
	//ryuhs74@20151104 - Add AVM-E500 View Mode CMD
	CMD_REQ_FRONT_VIEW,
	CMD_REQ_REAR_VIEW,
	CMD_REQ_RIGHT_VIEW,
	CMD_REQ_LEFT_VIEW,
	CMD_REQ_FULL_FRONT_VIEW,
	CMD_REQ_FULL_REAR_VIEW

} CMD_EXTMICOM_TO_DSP;
#if 0
#define CMD_REQ_DSP_STANDBY		(0x11)	// Send standby command when ACC off, and then perprom power sequencing
#define CMD_REQ_DSP_STATUS		(0x12)	// Request status of DSP, DSP response AVM/FACTORY/UPDATE mode
#define CMD_REQ_MICOM_VER		(0x13)	// Send version of AVM MICOM
#define CMD_SEND_IRDA_KEY		(0x14)	// Send IrDA command with Key value
#define CMD_SEND_RGEAR			(0x15)	// Send Rear Gear On/Off
#define CMD_CONTROL_VIDEO_OUT 	(0x17)// CVBS1/CVBS2 output enable
#define CMD_SDCARD_HPD			(0x18)
#define CMD_REQ_VBAT_ADC		(0x19)	// Request VBAT ADC value of MICOM
#define CMD_DSP_REQ_TEST		(0x1C)

#define CMD_SEND_TURN_SIGNAL 	(0x20)
#define CMD_SEND_BUTTON_PRESSED (0x21)
#define CMD_REQ_LVDS_STATUS		(0x22)
#define CMD_REQ_AUDIO_OUT		(0x23)
#endif

#define DSP_STATUS_NONE			(0xFF)	// Not received status yet
#define DSP_STATUS_AVM_MODE		(0x10)	// In AVM menu
#define DSP_STATUS_UPGRADE_MODE	(0x20)	// In Upgrade menu
#define DSP_STATUS_OBD_FW_UPGRADING (0x21)
#define DSP_STATUS_FACTORY_MODE	(0x30)
#define DSP_STATUS_BUSY			(0x40)
#define DSP_STATUS_HALTED		(0x50)


//// for gear state
#define GEAR_P	(0x0A)
#define GEAR_N	(0x0B)
#define GEAR_R 	(0x0C)
#define GEAR_D	(0x0D)

//// for turn signal
#define TURN_OFF	(0)
#define LEFT_TURN	(1)
#define RIGHT_TURN	(2)
#define EMERGENCY	(3)

//// for Button
#define BUTTON_1_SHORT_PRESSED	(1)
#define BUTTON_1_LONG_PRESSED	(2)
#define BUTTON_2_SHORT_PRESSED	(3)
#define BUTTON_2_LONG_PRESSED	(4)

#define IRDA_CUSTOM_CODE1 (0x1869)	// °³¹ß¿ë ž®žðÄÜ
#define IRDA_DEV_KEY_PWR	(0x01)
#define IRDA_DEV_KEY_FULL	(0x15)
#define IRDA_DEV_KEY_LOCK	(0x13)
#define IRDA_DEV_KEY_UP		(0x05)
#define IRDA_DEV_KEY_DOWN	(0x11)
#define IRDA_DEV_KEY_LEFT	(0x07)
#define IRDA_DEV_KEY_RIGHT	(0x09)

#define IRDA_CUSTOM_CODE2 (0x2694)	// Ÿç»ê¿ë ž®žðÄÜ
#define IRDA_KEY_PWR	(0x09)
#define IRDA_KEY_FULL	(0x05)
#define IRDA_KEY_LOCK	(0x5C)
#define IRDA_KEY_UP		(0x0F)
#define IRDA_KEY_DOWN	(0x0E)
#define IRDA_KEY_LEFT	(0x0B)
#define IRDA_KEY_RIGHT	(0x0A)

#define IRDA_CUSTOM_CODE3 (0x866b)	// žðŽÏÅÍ¿ë ž®žðÄÜ
#define IRDA_KEY_MONITOR_PWR	(0x12)
#define IRDA_KEY_MONITOR_UP		(0x1B)
#define IRDA_KEY_MONITOR_DOWN	(0x1A)
#define IRDA_KEY_MONITOR_LEFT	(0x04)
#define IRDA_KEY_MONITOR_RIGHT	(0x06)
#define IRDA_KEY_MONITOR_LOCK	(0x05)





 /**
 *******************************************************************************
 *
 * \brief Maximum number of UARTCMD link objects
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define UARTCMD_LINK_OBJ_MAX    (1)

 /**
 *******************************************************************************
 *
 * \brief Maximum value of UARTCMD factor
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define UARTCMD_LINK_MAX_NUM_UARTCMDS    (3)

/**
 *******************************************************************************
 *
 * \brief Maximum frmaes an output queue can support
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define UARTCMD_LINK_MAX_FRAMES_PER_OUT_QUE    \
                           (SYSTEM_LINK_FRAMES_PER_CH*SYSTEM_MAX_CH_PER_OUT_QUE)

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */


#endif /* EXAMPLES_TDA2XX_SRC_LINKS_UARTCMD_UARTCMD_PRIV_H_ */
