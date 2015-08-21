/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include "network_ctrl_priv.h"
#include <osa_file.h>

#define NETWORKCTRL_STEREO_CALIB_LUT_SIZE   		(3*1024U*1024U)

#define NETWORKCTRL_STEREO_CALIB_LUT_QSPI_OFFSET    (29U*1024U*1024U)

#define NETWORKCTRL_STEREO_CALIB_LUT_HEADER_SIZE 	(16U)

#define NETWORKCTRL_STEREO_CALIB_LUT_FILE_NAME_SIZE	(30U)

#define NETWORKCTRL_STEREO_CALIB_LUT_TAG_ID      	(0x00CCAABBU)

UInt8   pCalibData[NETWORKCTRL_STEREO_CALIB_LUT_SIZE];

void handleStereoCalibImageSave()
{
    UInt32 *pMem;
    UInt32 prmSize = 0;
    char filename[100];

    SendCommand(gNetworkCtrl_obj.command, NULL, 0);
    RecvResponse(gNetworkCtrl_obj.command, &prmSize);

    if(prmSize)
    {
        pMem = malloc(prmSize);
        if(pMem==NULL)
        {
            printf("# ERROR: Command %s: Unable to allocate memory for \
                            response parameters\n", gNetworkCtrl_obj.command);
            exit(0);
        }

        RecvResponseParams(gNetworkCtrl_obj.command, (UInt8*)pMem, prmSize);

        strcpy(filename,"right_");
        strcat(filename, gNetworkCtrl_obj.params[0]);
        strcat(filename, ".pgm");
        OSA_fileWriteFile(filename, (UInt8*)pMem, prmSize/2);

        strcpy(filename,"left_");
        strcat(filename, gNetworkCtrl_obj.params[0]);
        strcat(filename, ".pgm");
        OSA_fileWriteFile(filename, (UInt8*)pMem + prmSize/2, prmSize/2);
    }
    else
    {
        printf("# ERROR: Command %s: Target unable to save RAW data\n"
                                                    , gNetworkCtrl_obj.command);
    }
}

void handleStereoWriteCalibLUTDataToQSPI()
{
    Int32 status;
    UInt32 sizeL, sizeR;
    UInt32 *pCalibLUTBuf;

    /* Store the QSPI offset in the first word */
    pCalibLUTBuf = (UInt32 *)pCalibData;

    /* First four words contains header information, so save LUT
       bin file after first four words */
    //RIght Channel LUT
    status = OSA_fileReadFile(
        gNetworkCtrl_obj.params[0],
        (UInt8*)(pCalibLUTBuf + NETWORKCTRL_STEREO_CALIB_LUT_HEADER_SIZE/4),
        NETWORKCTRL_STEREO_CALIB_LUT_SIZE/2,
        &sizeR);

	//Left Channel LUT
	status = OSA_fileReadFile(
        gNetworkCtrl_obj.params[1],
        (UInt8*)(pCalibLUTBuf + NETWORKCTRL_STEREO_CALIB_LUT_HEADER_SIZE/4 + sizeR/4),
        NETWORKCTRL_STEREO_CALIB_LUT_SIZE/2,
        &sizeL);

    if (0 == status)
    {
    
        printf("# Writing the QSPI tags\n");
        /* Set the TAG Word */
        *pCalibLUTBuf = NETWORKCTRL_STEREO_CALIB_LUT_TAG_ID;
        pCalibLUTBuf ++;

        /* Set the Size of the DCC file */
        *pCalibLUTBuf = sizeR+sizeL+NETWORKCTRL_STEREO_CALIB_LUT_HEADER_SIZE;
        pCalibLUTBuf ++;

        *pCalibLUTBuf = NETWORKCTRL_STEREO_CALIB_LUT_QSPI_OFFSET;
		pCalibLUTBuf ++;

		*pCalibLUTBuf = 0U;

        SendCommand(gNetworkCtrl_obj.command, pCalibData,
                    sizeL + sizeR + NETWORKCTRL_STEREO_CALIB_LUT_HEADER_SIZE);
        RecvResponse(gNetworkCtrl_obj.command, NULL);
    }
}
