/** ==================================================================
 *  @file   TI_dcc.h
 *
 *  @desc   This  File contains.
 * ===================================================================
 *  Copyright (c) Texas Instruments Inc 2015
 *
 *  Use of this software is controlled by the terms and conditions found
 *  in the license agreement under which this software has been supplied
 * ===================================================================*/
#ifndef TI_DCC_
#define TI_DCC_

#ifdef __cplusplus
extern "C" {
#endif


Int32 Dcc_Create(AlgorithmLink_IssAewbObj *pAewbObj,
    System_LinkMemAllocInfo *pMemAllocInfo);
Void Dcc_delete(AlgorithmLink_IssAewbObj *pAewbObj,
    System_LinkMemAllocInfo *pMemAllocInfo);
Void Dcc_update_params(
    AlgorithmLink_IssAewbObj *pAlgObj,
    IssAewbAlgOutParams *pAlgOutPrms);
Int32 Dcc_parse_and_save_params(
    AlgorithmLink_IssAewbObj *pAlgObj,
    IssIspConfigurationParameters *pIspCfg,
    IssM2mSimcopLink_ConfigParams *pSimcopCfg,
    UInt32 dccBufSize);

Int32 Dcc_init_isp_config(
    AlgorithmLink_IssAewbObj *pAlgObj,
    IssIspConfigurationParameters *pIspCfg);

#ifdef __cplusplus
}
#endif

#endif /* end of TI_DCC_ */
