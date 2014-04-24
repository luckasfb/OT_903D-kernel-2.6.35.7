
#ifndef MFV_DRV_API_H
#define MFV_DRV_API_H

#include "val_types.h"
#include "hal_types.h"

#define LOAD_INSTRUCTION_AUTO

#ifdef __cplusplus
extern "C" {
#endif

VAL_VOID_T MFVInit(VAL_VOID_T);
VAL_VOID_T MFVDeInit(VAL_VOID_T);
VAL_VOID_T MFVSetCmd(P_MFV_DRV_CMD_QUEUE_T cmd);
VAL_VOID_T MFVSetPower(HAL_POWER_T *a_prParam);
VAL_VOID_T MFVSetIsr(HAL_ISR_T *a_prParam);
VAL_VOID_T MFV_ISR(VAL_VOID_T);

#ifdef __cplusplus
}
#endif

#endif /* MFV_DRV_API_H */
