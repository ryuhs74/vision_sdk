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
 *
 * \ingroup UTILS_MEM_API
 * \defgroup UTILS_MEM_CFG_API Memory heap configuration parameters
 *
 * \brief  Parameters to enable / disable L2 heap and set size of L2 heap
 *
 * @{
 *
 *******************************************************************************
*/

/**
 *******************************************************************************
 *
 * \file utils_mem_cfg.h
 *
 * \brief Memory heap configuration parameters
 *
 * \version 0.0 First version
 *
 *******************************************************************************
*/

#ifndef _UTILS_MEM_CFG_H_
#define _UTILS_MEM_CFG_H_

#undef ENABLE_HEAP_L2
#undef UTILS_MEM_HEAP_DDR_CACHED_SIZE
#undef UTILS_MEM_HEAP_OCMC_SIZE
#undef UTILS_MEM_HEAP_L2_SIZE
#undef ENABLE_HEAP_SR0

#ifdef BUILD_DSP
    #define ENABLE_HEAP_L2
    /* The size of the DSP L2 Heap is reduced by 128 Bytes to make space for
     * the DSP Idle Function. The DSP Idle Function runs from L2 to make sure
     * the DSP pre-fetch pipeline is cleared before going to Idle.
     */
    #define UTILS_MEM_HEAP_L2_SIZE  (32*1024 - 128)
#endif

#ifdef BUILD_ARP32
    #define ENABLE_HEAP_L2
    #define UTILS_MEM_HEAP_L2_SIZE  (24*1024)
#endif

#ifdef BUILD_M4_0

#define ENABLE_HEAP_SR0

/*
 * Make sure value for UTILS_MEM_HEAP_OCMC_SIZE is <=
 *    OCMC1_SIZE specified in the SoC specific .xs file
 *
 * Make sure value for UTILS_MEM_HEAP_DDR_CACHED_SIZE is <=
 *    SR1_FRAME_BUFFER_SIZE specified in the SoC specific .xs file
 *
 * The .xs file to look at depends on SoC, A15 OS and DDR memory config selected
 *
 * SoC          A15 OS DDR config           .xs file
 * =================================================
 * TDA2XX_BUILD Bios   TDA2XX_256MB_DDR     vision_sdk\build\tda2xx\mem_segment_definition_256mb_bios.xs
 * TDA2XX_BUILD Bios   TDA2XX_1024MB_DDR    vision_sdk\build\tda2xx\mem_segment_definition_1024mb_bios.xs
 * TDA2XX_BUILD Linux  A15_TARGET_OS_LINUX  vision_sdk\build\tda2xx\mem_segment_definition_1024mb_linux.xs
 *
 * TDA3XX_BUILD NA     TDA3XX_64MB_DDR      vision_sdk\build\tda3xx\mem_segment_definition_64mb.xs
 * TDA3XX_BUILD NA     none                 vision_sdk\build\tda3xx\mem_segment_definition_512mb.xs
 *
 * TDA2EX_BUILD Bios   TDA2EX_1024MB_DDR    vision_sdk\build\tda2ex\mem_segment_definition_1024mb_bios.xs
 * TDA2EX_BUILD Linux  A15_TARGET_OS_LINUX  vision_sdk\build\tda2ex\mem_segment_definition_1024mb_linux.xs
 *
 */

#define UTILS_MEM_HEAP_OCMC_SIZE        (512*1024)

#ifdef TDA2XX_BUILD
    #ifdef A15_TARGET_OS_BIOS
        #ifdef TDA2XX_256MB_DDR
            #define UTILS_MEM_HEAP_DDR_CACHED_SIZE      (109*1024*1024)
        #endif
        #ifdef TDA2XX_1024MB_DDR
            #define UTILS_MEM_HEAP_DDR_CACHED_SIZE      (256*1024*1024)
        #endif
    #endif
    #ifdef A15_TARGET_OS_LINUX
        #define UTILS_MEM_HEAP_DDR_CACHED_SIZE          (250*1024*1024)
    #endif
#endif

#ifdef TDA2EX_BUILD
    #ifdef A15_TARGET_OS_BIOS
        #ifdef TDA2EX_1024MB_DDR
            #define UTILS_MEM_HEAP_DDR_CACHED_SIZE      (256*1024*1024)
        #endif
    #endif

    #ifdef A15_TARGET_OS_LINUX
        #define UTILS_MEM_HEAP_DDR_CACHED_SIZE          (254*1024*1024)
    #endif
#endif

#ifdef TDA3XX_BUILD
    #ifdef TDA3XX_64MB_DDR
        #define UTILS_MEM_HEAP_DDR_CACHED_SIZE          (39*1024*1024)
    #else
        #define UTILS_MEM_HEAP_DDR_CACHED_SIZE          (256*1024*1024)
    #endif
#endif

#endif /* #ifdef BUILD_M4_0 */

#endif /* _UTILS_MEM_CFG_H_ */

/* @} */
