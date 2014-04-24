

#ifndef _HAL_HW_API_H_
#define _HAL_HW_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "val_types.h"
#include "hal_types.h"


///////////////////////////////////////////////
////// HW dependent function
///////////////////////////////////////////////
VAL_RESULT_T eHalHwPowerCtrl(
    HAL_POWER_T     *a_prParam, 
    VAL_UINT32_T    a_u4ParamSize
    );

VAL_RESULT_T eHalHwEnableClock(
    HAL_CLOCK_T *a_prParam, 
    VAL_UINT32_T a_u4ParamSize
);

VAL_RESULT_T eHalHwDisableClock(
    HAL_CLOCK_T *a_prParam, 
    VAL_UINT32_T a_u4ParamSize
);

#ifdef __cplusplus
}
#endif

#endif // #ifndef _HAL_HW_API_H_
