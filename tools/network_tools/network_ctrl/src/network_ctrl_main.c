/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include "network_ctrl_priv.h"


NetworkCtrl_Obj gNetworkCtrl_obj;

void Init()
{
    memset(&gNetworkCtrl_obj, 0, sizeof(gNetworkCtrl_obj));

    RegisterHandler("echo", handleEcho, 1);
    RegisterHandler("mem_rd", handleMemRd, 2);
    RegisterHandler("mem_wr", handleMemWr, 2);
    RegisterHandler("mem_save", handleMemSave, 3);
    RegisterHandler("iss_raw_save", handleIssRawSave, 1);
    RegisterHandler("iss_yuv_save", handleIssYuvSave, 1);
    RegisterHandler("iss_send_dcc_file", handleIssSendDccFile, 1);
    RegisterHandler("iss_save_dcc_file", handleIssSaveDccFile, 2);
    RegisterHandler("iss_write_sensor_reg", handleIssSensorRegWrite, 3);
    RegisterHandler("iss_read_sensor_reg", handleIssSensorRegRead, 2);
    RegisterHandler("iss_clear_dcc_qspi_mem", handleIssClearDccQspiMem, 1);
    RegisterHandler("iss_read_2a_params", handleIssRead2AParams, 0);
    RegisterHandler("iss_write_2a_params", handleIssWrite2AParams, 9);
    RegisterHandler("stereo_calib_image_save", handleStereoCalibImageSave, 1);
    RegisterHandler("stereo_calib_lut_to_qspi", handleStereoWriteCalibLUTDataToQSPI, 2);
    RegisterHandler("stereo_set_params", handleStereoCalibSetParams, 11);
    RegisterHandler("stereo_set_dynamic_params", handleStereoSetDynamicParams, 8);
    RegisterHandler("qspi_wr", handleQspiSendFile, 2);
    RegisterHandler("sys_reset", handleSysReset, 0);
}

void ShowUsage()
{
    printf(" \n");
    printf("# \n");
    printf("# network_ctrl --ipaddr <ipaddr> [--port <server port>] --cmd <command string> <command parameters>\n");
    printf("# \n");
    printf("# (c) Texas Instruments 2014\n");
    printf("# \n");
    printf("# Supported commands,\n");
    printf("# -------------------\n");
    printf("# echo <string to echo>\n");
    printf("# mem_rd <memory address in hex> <size of memory to read in units of 32-bit words>\n");
    printf("# mem_wr <memory address in hex> <value to be written in units of 32-bit words>\n");
    printf("# mem_save <memory address in hex> <size of memory to read in bytes> <filename in which data is saved>\n");
    printf("# iss_raw_save <filename in which data is saved>\n");
    printf("# iss_yuv_save <filename in which data is saved>\n");
    printf("# iss_send_dcc_file <file name to be sent>\n");
    printf("# iss_save_dcc_file <Sensor ID> <file name to be sent>\n");
    printf("#       Supported sensors are 140, 10640, 132 and 224\n");
    printf("# iss_clear_dcc_qspi_mem <Sensor ID> \n");
    printf("#       Supported sensors are 140, 10640, 132 and 224\n");
    printf("# iss_write_sensor_reg <chan num> <RegAddr> <RegVal>\n");
    printf("# iss_read_sensor_reg <chan num> <RegAddr>\n");
    printf("# iss_read_2a_params \n");
    printf("# iss_write_2a_params <AE Mode> {Digital Gain} {Analog Gain}"
           " {Exposure Time} <AWB Mode> {Red Gain} {Green Gain} {Blue Gain} {Color Temparature}\n");
    printf("# stereo_calib_image_save <filename prefix in which data is saved>\n");
    printf("# stereo_calib_lut_to_qspi <Right sensor's LUT bin filename> <Left sensor's LUT bin filename>\n");
    printf("# stereo_set_params <numDisparities> <disparityStepSize> <disparitySearchDir> <disparitySupportWinWidth>"
           " <disparitySupportWinHeight> <leftRightCheckEna> <censusWinWidth> <censusWinHeight> <censusWinHorzStep> <censusWinVertStep> <pp_colormap_index>"
           "\n[EXAMPLE] stereo_set_params 128 4 0 11 11 0 9 9 2 2 0\n");
    printf("# stereo_set_dynamic_params <pp_cost_max_thresh> <pp_conf_min_thresh> <pp_texture_lumalothresh>"
            " <pp_texture_lumahithresh> <pp_texture_thresh> <pp_leftright_thresh> <pp_maxdisp_dissimilarity> <pp_minconf_nseg_thresh>"
            "\n[EXAMPLE] stereo_set_dynamic_params 95 98 0 100 85 255 2 2\n");
    printf("# qspi_wr <QSPI address in hex> <file name to be sent>\n");
    printf("# stereo_calib_lut_to_qspi <rectMapRight_int_converted.bin> <rectMapLeft_int_converted.bin>\n");
    printf("# sys_reset\n");
    printf("# \n");
    exit(0);
}


int main(int argc, char *argv[])
{
    Init();
    ParseCmdLineArgs(argc, argv);

    ConnectToServer();
    CommandExecute();
    CloseConnection();

    return 0;
}

int isValidCommand()
{
    int i;

    /* check if command is already registered */
    for(i=0; i<NETWORK_CTRL_MAX_CMDS; i++)
    {
        if(gNetworkCtrl_obj.cmdHandler[i].handler)
        {
           if(strncmp(
                gNetworkCtrl_obj.cmdHandler[i].cmd,
                gNetworkCtrl_obj.command,
                NETWORK_CTRL_CMD_STRLEN_MAX)
                ==0)
            {
               return 1;
            }
        }
    }

    return 0;
}

int isValidNumParams()
{
    int i;

    /* check if command is already registered */
    for(i=0; i<NETWORK_CTRL_MAX_CMDS; i++)
    {
        if(gNetworkCtrl_obj.cmdHandler[i].handler)
        {
           if(strncmp(
                gNetworkCtrl_obj.cmdHandler[i].cmd,
                gNetworkCtrl_obj.command,
                NETWORK_CTRL_CMD_STRLEN_MAX)
                ==0)
            {
               if(gNetworkCtrl_obj.cmdHandler[i].numParams==-1)
               {
                    /* variable number of parameters possible */
                    return 1;
               }
               if(gNetworkCtrl_obj.numParams
                    ==
                   gNetworkCtrl_obj.cmdHandler[i].numParams
                 )
               {
                    return 1;
               }

               return 0;
            }
        }
    }

    return 0;
}

void CommandExecute()
{
    int i;

    /* check if command is already registered */
    for(i=0; i<NETWORK_CTRL_MAX_CMDS; i++)
    {
        if(gNetworkCtrl_obj.cmdHandler[i].handler)
        {
           if(strncmp(
                gNetworkCtrl_obj.cmdHandler[i].cmd,
                gNetworkCtrl_obj.command,
                NETWORK_CTRL_CMD_STRLEN_MAX)
                ==0)
            {
               gNetworkCtrl_obj.cmdHandler[i].handler();
               return;
            }
        }
    }
}

void RegisterHandler(char *command, void (*handler)(), int numParams)
{
    NetworkCtrl_Obj *pObj = &gNetworkCtrl_obj;
    int i;
    int firstFreeIdx;

    firstFreeIdx = -1;

    /* check if command is already registered */
    for(i=0; i<NETWORK_CTRL_MAX_CMDS; i++)
    {
        if(pObj->cmdHandler[i].handler)
        {
            if(strncmp(
                pObj->cmdHandler[i].cmd,
                command,
                NETWORK_CTRL_CMD_STRLEN_MAX)
                ==0)
            {
                /* command already register, exit with error */
                printf("# ERROR: Command [%s] already registered !!!\n", command);
                exit(0);
            }
        }
        else
        {
            if(firstFreeIdx==-1)
            {
                firstFreeIdx = i;
            }
        }
    }

    /* no space to register command */
    if(firstFreeIdx==-1)
    {
        printf("# ERROR: No space to register command [%s] !!!\n", command);
        exit(0);
    }

    /* command not registered, register it */
    pObj->cmdHandler[firstFreeIdx].handler = handler;
    strcpy(pObj->cmdHandler[firstFreeIdx].cmd, command);
    pObj->cmdHandler[firstFreeIdx].numParams = numParams;
}

void ConnectToServer()
{
    int status;

    Network_init();

    status = Network_connect(&gNetworkCtrl_obj.sockObj, gNetworkCtrl_obj.ipAddr, gNetworkCtrl_obj.serverPort);
    if(status<0)
    {
        exit(0);
    }
}

void CloseConnection()
{
    Network_close(&gNetworkCtrl_obj.sockObj);

    Network_deInit();
}

int RecvResponseParams(char *command, UInt8 *pPrm, UInt32 prmSize)
{
    int status;

    status = Network_read(&gNetworkCtrl_obj.sockObj, pPrm, &prmSize);
    if(status<0)
    {
        printf("# ERROR: Could not read reponse parameters for command (%s)\n", command);
        exit(0);
    }

    return 0;
}

int RecvResponse(char *command, UInt32 *prmSize)
{
    NetworkCtrl_CmdHeader cmdHeader;
    UInt32 dataSize;
    int status;

    memset(&cmdHeader, 0, sizeof(cmdHeader));

    dataSize = sizeof(cmdHeader);

    status = Network_read(&gNetworkCtrl_obj.sockObj, (UInt8*)&cmdHeader, &dataSize);
    if(status<0)
    {
        printf("# ERROR: Could not read reponse header for command (%s)\n", command);
        exit(0);
    }

    if(cmdHeader.header != NETWORK_CTRL_HEADER)
    {
        printf("# ERROR: Invalid response header received for command (%s)\n", command);
        exit(0);
    }

    if( strcmp(cmdHeader.cmd, command) != 0)
    {
        printf("# ERROR: Response header received for incorrect command (%s)\n", command);
        exit(0);
    }

    if(prmSize)
    {
        *prmSize = cmdHeader.prmSize;
    }

    printf("# Command %s: Received reponse (status = %d, prmSize = %d)\n", command, cmdHeader.returnValue, cmdHeader.prmSize);

    return (int)cmdHeader.returnValue;
}

void SendCommand(char *command, void *params, int prmSize)
{
    NetworkCtrl_CmdHeader cmdHeader;
    int status;

    memset(&cmdHeader, 0, sizeof(cmdHeader));

    cmdHeader.header = NETWORK_CTRL_HEADER;
    strcpy(cmdHeader.cmd, command);
    cmdHeader.returnValue = 0;
    cmdHeader.flags = 0;
    cmdHeader.prmSize = prmSize;

    status = Network_write(&gNetworkCtrl_obj.sockObj, (UInt8*)&cmdHeader, sizeof(cmdHeader));
    if(status<0)
    {
        printf("# ERROR: Could not write header for command (%s)\n", command);
        exit(0);
    }

    status = Network_write(&gNetworkCtrl_obj.sockObj, (UInt8*)params, prmSize);
    if(status<0)
    {
        printf("# ERROR: Could not write parameters for command (%s)\n", command);
        exit(0);
    }

    printf("# Command %s: Sent %d bytes\n", command, prmSize);
}

void ParseCmdLineArgs(int argc, char *argv[])
{
    int i, p;

    gNetworkCtrl_obj.serverPort = NETWORK_CTRL_SERVER_PORT;
    gNetworkCtrl_obj.numParams = 0;

    for(i=0; i<argc; i++)
    {
        if(strcmp(argv[i], "--ipaddr")==0)
        {
            i++;
            if(i>=argc)
            {
                ShowUsage();
            }
            strcpy(gNetworkCtrl_obj.ipAddr, argv[i]);
        }
        else
        if(strcmp(argv[i], "--port")==0)
        {
            i++;
            if(i>=argc)
            {
                ShowUsage();
            }
            gNetworkCtrl_obj.serverPort = atoi(argv[i]);
        }
        else
        if(strcmp(argv[i], "--cmd")==0)
        {
            i++;
            if(i>=argc)
            {
                ShowUsage();
            }
            strcpy(gNetworkCtrl_obj.command, argv[i]);

            i++;
            p=0;
            for( ;i<argc;i++)
            {
                strcpy(gNetworkCtrl_obj.params[p], argv[i]);
                p++;
            }

            gNetworkCtrl_obj.numParams = p;
        }
    }

    if(gNetworkCtrl_obj.ipAddr[0]==0
            ||
       gNetworkCtrl_obj.command[0]==0
            ||
        isValidCommand() == FALSE
            ||
        isValidNumParams() == FALSE
        )
    {
        if(gNetworkCtrl_obj.ipAddr[0]==0)
        {
            printf("# ERROR: IP Address of server MUST be specified\n");
        }
        else
        if(gNetworkCtrl_obj.command[0]==0)
        {
            printf("# ERROR: Command MUST be specified\n");
        }
        else
        if(isValidCommand() == FALSE)
        {
            printf("# ERROR: Command [%s] NOT SUPPORTED\n", gNetworkCtrl_obj.command);
        }
        else
        if(isValidNumParams() == FALSE)
        {
            printf("# ERROR: Insufficient parameters (%d) specified for command [%s] \n", gNetworkCtrl_obj.numParams, gNetworkCtrl_obj.command);
        }

        ShowUsage();
        exit(0);
    }
}
