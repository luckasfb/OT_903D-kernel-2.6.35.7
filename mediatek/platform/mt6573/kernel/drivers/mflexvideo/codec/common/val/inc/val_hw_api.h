

#ifndef _VAL_HW_API_H_
#define _VAL_HW_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "val_types.h"
#include "mfv_drv_base.h"


VAL_RESULT_T eVideoRegIsr(
    VAL_ISR_T *a_prParam, 
    VAL_UINT32_T a_u4ParamSize
);

VAL_RESULT_T eVideoFreeIsr(
    VAL_ISR_T *a_prParam, 
    VAL_UINT32_T a_u4ParamSize
);

VAL_RESULT_T eVideoCleanCache(
    VAL_CACHE_T *a_prParam, 
    VAL_UINT32_T a_u4ParamSize
);

VAL_RESULT_T eVideoFlushCache(
    VAL_CACHE_T *a_prParam, 
    VAL_UINT32_T a_u4ParamSize
);

VAL_RESULT_T eVideoMfvWriteReg(
    MFV_DRV_CMD_T   *a_prParam, 
    VAL_UINT32_T    a_u4ParamSize
);

VAL_RESULT_T eVideoMfvReadReg(
    MFV_DRV_CMD_T   *a_prParam, 
    VAL_UINT32_T    a_u4ParamSize
);

VAL_RESULT_T eVideoMfvWriteSysRam(
    MFV_DRV_CMD_T   *a_prParam, 
    VAL_UINT32_T    a_u4ParamSize
);

VAL_RESULT_T eVideoMfvWriteSysRamRange(
    MFV_DRV_CMD_T   *a_prParam, 
    VAL_UINT32_T    a_u4ParamSize
);

VAL_RESULT_T eVideoMfvReadSysRam(
    MFV_DRV_CMD_T   *a_prParam, 
    VAL_UINT32_T    a_u4ParamSize
);

VAL_UINT32_T u4MfvHWRead(
    VAL_UINT32_T   a_u4Addr
);

VAL_UINT32_T u4MfvHWWrite(
    VAL_UINT32_T   a_u4Addr,
    VAL_UINT32_T   a_u4Value
);

#ifdef __cplusplus
}
#endif

#endif // #ifndef _VAL_HW_API_H_
