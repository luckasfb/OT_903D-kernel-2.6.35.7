
#ifndef MFV_DRV_BASE_H
#define MFV_DRV_BASE_H

#include "val_types.h"

#define LOAD_INSTRUCTION_AUTO

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __MFV_DRV_CMD_T *P_MFV_DRV_CMD_T;
typedef struct __MFV_DRV_CMD_T
{
    VAL_UINT32_T type;      // enum MFV_DRV_CMD_TYPE
    VAL_UINT32_T address;   // phisical address
    VAL_UINT32_T value;     // 1. Value. 2. to be "Virtual address" in Read_xxx_CMD destination. 3. to be type in LOADxxx or ENABLExxx/DISABLExxx.
    VAL_UINT32_T mask;
} MFV_DRV_CMD_T;

typedef struct __MFV_DRV_CMD_QUEUE_T *P_MFV_DRV_CMD_QUEUE_T;
typedef struct __MFV_DRV_CMD_QUEUE_T
{
    VAL_UINT32_T        CmdNum;
    P_MFV_DRV_CMD_T     pCmd;
} MFV_DRV_CMD_QUEUE_T;

typedef enum
{
    LOAD_INST_START_CMD,
    LOAD_INST_END_CMD,
    LOAD_INST_CMD,
    ENABLE_HW_CMD,
    DISABLE_HW_CMD,
    CACHE_FLUSH, //cache invalidate
    CACHE_CLEAN, //write cache to memory
    WRITE_REG_CMD,
    READ_REG_CMD,
    WRITE_SYSRAM_CMD,
    READ_SYSRAM_CMD,
    POLL_CMD,
    SETUP_ISR_CMD,
	WAIT_ISR_CMD,	
    TIMEOUT_CMD,
	LOAD_RV9_TABLE,
	READ_INST_PACKET,
    WRITE_SYSRAM_RANGE_CMD,
    END_CMD
} MFV_DRV_CMD_TYPE;


#ifdef __cplusplus
}
#endif

#endif /* MFV_DRV_BASE_H */
