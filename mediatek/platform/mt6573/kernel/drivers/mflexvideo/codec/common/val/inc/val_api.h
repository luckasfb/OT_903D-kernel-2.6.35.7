

#ifndef _VAL_API_H_
#define _VAL_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "val_types.h"
#include "mfv_drv_base.h"

VAL_RESULT_T eVideoMemAlloc(
    VAL_MEMORY_T *a_prParam, 
    VAL_UINT32_T a_u4ParamSize
);

VAL_RESULT_T eVideoMemFree(
    VAL_MEMORY_T *a_prParam, 
    VAL_UINT32_T a_u4ParamSize
);

VAL_RESULT_T eVideoMemSet(
    VAL_MEMORY_T *a_prParam,
    VAL_UINT32_T a_u4ParamSize,
    VAL_INT32_T a_u4Value,
    VAL_UINT32_T a_u4Size
);

VAL_RESULT_T eVideoMemCpy(
    VAL_MEMORY_T *a_prParamDst,
    VAL_UINT32_T a_u4ParamDstSize,
    VAL_MEMORY_T *a_prParamSrc,
    VAL_UINT32_T a_u4ParamSrcSize,
    VAL_UINT32_T a_u4Size
);

VAL_RESULT_T eVideoMemCmp(
    VAL_MEMORY_T *a_prParamSrc1,
    VAL_UINT32_T a_u4ParamSrc1Size,
    VAL_MEMORY_T *a_prParamSrc2,
    VAL_UINT32_T a_u4ParamSrc2Size,
    VAL_UINT32_T a_u4Size
);

VAL_RESULT_T eVideoIntMemUsed(
    VAL_INTMEM_T *a_prParam, 
    VAL_UINT32_T a_u4ParamSize
);

VAL_RESULT_T eVideoIntMemAlloc(
    VAL_INTMEM_T *a_prParam, 
    VAL_UINT32_T a_u4ParamSize
);

VAL_RESULT_T eVideoIntMemFree(
    VAL_INTMEM_T *a_prParam, 
    VAL_UINT32_T a_u4ParamSize
);

VAL_RESULT_T eVideoCreateEvent(
    VAL_EVENT_T *a_prParam, 
    VAL_UINT32_T a_u4ParamSize
);

VAL_RESULT_T eVideoCloseEvent(
    VAL_EVENT_T *a_prParam, 
    VAL_UINT32_T a_u4ParamSize
);

VAL_RESULT_T eVideoWaitEvent(
    VAL_EVENT_T *a_prParam, 
    VAL_UINT32_T a_u4ParamSize
);

VAL_RESULT_T eVideoSetEvent(
    VAL_EVENT_T *a_prParam, 
    VAL_UINT32_T a_u4ParamSize
);

VAL_RESULT_T eVideoCreateMutex(
    VAL_MUTEX_T *a_prParam, 
    VAL_UINT32_T a_u4ParamSize
);

VAL_RESULT_T eVideoCloseMutex(
    VAL_MUTEX_T *a_prParam, 
    VAL_UINT32_T a_u4ParamSize
);

VAL_RESULT_T eVideoWaitMutex(
    VAL_MUTEX_T *a_prParam, 
    VAL_UINT32_T a_u4ParamSize
);

VAL_RESULT_T eVideoReleaseMutex(
    VAL_MUTEX_T *a_prParam, 
    VAL_UINT32_T a_u4ParamSize
);

VAL_RESULT_T eVideoGetTimeOfDay(
    VAL_TIME_T *a_prParam, 
    VAL_UINT32_T a_u4ParamSize
);

VAL_RESULT_T eVideoStrStr(
    VAL_STRSTR_T *a_prParam, 
    VAL_UINT32_T a_u4ParamSize
);

VAL_RESULT_T eVideoAtoi(
    VAL_ATOI_T *a_prParam, 
    VAL_UINT32_T a_u4ParamSize
);

#ifdef __cplusplus
}
#endif

#endif // #ifndef _VAL_API_H_
