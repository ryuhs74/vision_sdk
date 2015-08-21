/** ==================================================================
 *  @file   alg_aewb_ctrl_priv.h                                                  
 *                                                                    
 *  @path    /proj/vsi/users/venu/DM812x/IPNetCam_rel_1_8/ti_tools/iss_02_bkup/packages/ti/psp/iss/alg/aewb/ti2a/awb/inc/                                                 
 *                                                                    
 *  @desc   This  File contains.                                      
 * ===================================================================
 *  Copyright (c) Texas Instruments Inc 2011, 2012                    
 *                                                                    
 *  Use of this software is controlled by the terms and conditions found
 *  in the license agreement under which this software has been supplied
 * ===================================================================*/
#ifndef _ALG_AEWB_CTRL_PRIV_H_
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#define _ALG_AEWB_CTRL_PRIV_H_

typedef	struct _CONTROL3A{
	short	IMAGE_SHARPNESS;
	short   IMAGE_CONTRAST;
	short   IMAGE_BRIGHTNESS;
	short   IMAGE_SATURATION;
	short	IMAGE_BACKLIGHT;
	short   INDOUTDOOR;
	short   VIDEO_MODE;    		/*default  = 0    NTSC */
	short   PAUSE_AWWB;			/*default  = 0	*/
	short   SKIP_BINNING_MODE; 	/*default  = 0    binning mode */
	short	AUTO_IRIS;
	short	DAY_NIGHT;
}CONTROL3AS;

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
