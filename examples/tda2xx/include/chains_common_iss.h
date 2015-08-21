/*
*******************************************************************************
*
* Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
*
*******************************************************************************
*/

/*
 *******************************************************************************
 *
 * \file chains_common_iss.h
 *
 * \brief This file contains common functions and definitions for the ISS
 *
 *******************************************************************************
 */


#ifndef _CHAINS_COMMON_ISS_H_
#define _CHAINS_COMMON_ISS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Includes
 *******************************************************************************
 */

#include <include/link_api/algorithmLink_issAewb.h>



/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/*  \brief Command to save DCC file
 *         This command is used to DCC file in the QSPI. This command is sent
 *         by the network tool and processed by the chains common layer
 */
#define SYSTEM_LINK_CMD_SAVE_DCC_FILE           (0x8000)

/*  \brief Command to clear DCC QSPI mem for the give sensor
 *         This command is used to clear QSPI memory for the given sensor .
 *         This command is sent by the network tool and processed by
 *         the chains common layer
 */
#define SYSTEM_LINK_CMD_CLEAR_DCC_QSPI_MEM      (0x8001)

/*  \brief Command to read a sensor Register, used by the network utility
 *         for reading sensor register using video sensor layer
 */
#define SYSTEM_LINK_CMD_READ_SENSOR_REG         (0x8002)

/*  \brief Command to write to sensor register, used by network utility
 *         writes given value to given sensor register using video
 *         sensor layer
 */
#define SYSTEM_LINK_CMD_WRITE_SENSOR_REG        (0x8003)

/* Based QSPI offset where DCC binary files are stored */
#define CHAINS_COMMON_ISS_DCC_QSPI_OFFSET    (24U*1024U*1024U)

/* QSPI Offset for the AR0140 sensor */
#define CHAINS_COMMON_ISS_DCC_QSPI_OFFSET_AR0140 (                             \
                                            CHAINS_COMMON_ISS_DCC_QSPI_OFFSET)

/* QSPI Offset for the OV10640 sensor */
#define CHAINS_COMMON_ISS_DCC_QSPI_OFFSET_OV10640                              \
                                        (CHAINS_COMMON_ISS_DCC_QSPI_OFFSET +   \
                                         ALGORITHM_AEWB1_DCC_IN_BUF_SIZE)

/* QSPI Offset for the IMX224 sensor */
#define CHAINS_COMMON_ISS_DCC_QSPI_OFFSET_IMX224                               \
                                        (CHAINS_COMMON_ISS_DCC_QSPI_OFFSET +   \
                                         ALGORITHM_AEWB1_DCC_IN_BUF_SIZE * 2U)

/* QSPI Offset for the AR0132 sensor */
#define CHAINS_COMMON_ISS_DCC_QSPI_OFFSET_AR0132                               \
                                        (CHAINS_COMMON_ISS_DCC_QSPI_OFFSET +   \
                                         ALGORITHM_AEWB1_DCC_IN_BUF_SIZE * 3U)

/* Size of the header in DCC bin file */
#define CHAINS_COMMON_ISS_DCC_BIN_HEADER_SIZE           (16U)

/* offset of the qSPI address in dcc bin header */
#define CHAINS_COMMON_ISS_DCC_BIN_DCC_ID_OFFSET         (2U)

/* DCC Bin file Tag/magic number */
#define CHAINS_COMMON_ISS_DCC_BIN_FILE_TAG_ID           (0x00DCCBEEU)



/*******************************************************************************
 *  Structure declaration
 *******************************************************************************
 */

/* None */

/*******************************************************************************
 *  Function's
 *******************************************************************************
 */

/*  \brief Function to get the QSPI offset for give sensor DCC ID.
 *
 *  params sensorDccIf      DCC Id of the sensor
 *
 *  returns qspiOffset      Qspi offset for this give sensor
 *          0               If sensor is not supported. Offset 0 cannot be
 *                          used for writing, so using this for
 *                          unsupported sensor
 */
UInt32 ChainsCommon_issGetQspiOffsetFromDccId(UInt32 sensorDccId);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* end of _CHAINS_COMMON_ISS_H_ */

/* @} */

/**
 *******************************************************************************
 *
 *   \defgroup EXAMPLES_API Example code implementation
 *
 *******************************************************************************
 */
