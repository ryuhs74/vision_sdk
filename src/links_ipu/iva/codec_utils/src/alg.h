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
 * \file alg.h
 *
 * \brief  Alg functions are defined
 *
 * \version 0.0 (Jan 2014) : [SS] First version
 *
 *******************************************************************************
 */

#ifndef ALG_
#define ALG_

#include <ti/xdais/ialg.h>

#ifdef __cplusplus
extern "C" {
#endif
/*
 *  ======== ALG_Handle ========
 *  This handle type is used to reference all ALG instance objects
 */
typedef IALG_Handle ALG_Handle;

/*
 *  ======== ALG_activate ========
 *  Restore all shared persistant data associated with algorithm object
 */
extern Void ALG_activate(ALG_Handle alg);

/*
 *  ======== ALG_control ========
 *  Control algorithm object
 */
extern Int ALG_control(ALG_Handle alg, IALG_Cmd cmd, IALG_Status *sptr);

/*
 *  ======== ALG_create ========
 *  Create algorithm object and initialize its memory
 */
extern ALG_Handle ALG_create(IALG_Fxns *fxns, IALG_Handle p, IALG_Params *prms);

/*
 *  ======== ALG_deactivate ========
 *  Save all shared persistant data associated with algorithm object
 *  to some non-shared persistant memory.
 */
extern Void ALG_deactivate(ALG_Handle alg);

/*
 *  ======== ALG_delete ========
 *  Delete algorithm object and release its memory
 */
extern Void ALG_delete(ALG_Handle alg);

/*
 *  ======== ALG_exit ========
 *  Module finalization
 */
extern Void ALG_exit(Void);

/*
 *  ======== ALG_init ========
 *  Module initialization
 */
extern Void ALG_init(Void);

#ifdef __cplusplus
}
#endif

#endif  /* ALG_ */

/* Nothing beyond this point */

