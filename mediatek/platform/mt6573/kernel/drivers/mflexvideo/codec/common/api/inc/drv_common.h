
#ifndef DRV_COMMON_H
#define DRV_COMMON_H

#include "val_types.h"
#include "mfv_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_ENABLE_MFV_INTERRUPT_)
#define WAITING_MODE VAL_INTERRUPT_MODE
#else
#define WAITING_MODE VAL_POLLING_MODE
#endif

#define ADD_QUEUE(queue, index, q_type, q_address, q_value, q_mask)       \
{                                                                         \
  queue[index].type     = q_type;                                         \
  queue[index].address  = q_address;                                      \
  queue[index].value    = q_value;                                        \
  queue[index].mask     = q_mask;                                         \
  index = index + 1;                                                      \
}

#define DO_MFV_REEST(cmd, index)                                                            \
{                                                                                           \
    ADD_QUEUE(cmd, index, WRITE_REG_CMD, MFV_RESET, 1, 0);                                  \
    ADD_QUEUE(cmd, index, WRITE_REG_CMD, MFV_RESET, 0, 0);                                  \
    ADD_QUEUE(cmd, index, TIMEOUT_CMD, 0, 0xFFFFFFFF, 0);                                   \
    ADD_QUEUE(cmd, index, POLL_CMD, MFV_RESET, 2, 2);                                       \
	ADD_QUEUE(cmd, index, SETUP_ISR_CMD, MFV_IRQ_STS, 0, WAITING_MODE);                     \
}

// load instruction packet                                                                
#define LOAD_INSTRUCTION_PACKET(cmd, index, driver_type, gmc_base)                          \
{                                                                                           \
  ADD_QUEUE(cmd, index, LOAD_INST_START_CMD, 0, driver_type, 0);                            \
  DO_MFV_REEST(cmd, index);                                                                 \
  ADD_QUEUE(cmd, index, LOAD_INST_CMD, MFV_IDMA_PAC_BASE, driver_type, 4);                  \
  ADD_QUEUE(cmd, index, LOAD_INST_CMD, MFV_MEM_SWITCH, driver_type, 1);                     \
  ADD_QUEUE(cmd, index, WRITE_REG_CMD, MFV_IDMA_GMC_BASE, (VAL_UINT32_T)gmc_base, 0);       \
  ADD_QUEUE(cmd, index, WRITE_REG_CMD, MFV_IDMA_STR, 1, 0);                                 \
  ADD_QUEUE(cmd, index, WAIT_ISR_CMD, MFV_IRQ_STS, 0, 2);                                   \
  ADD_QUEUE(cmd, index, WRITE_REG_CMD, MFV_IRQ_ACK, 2, 0);                                  \
  ADD_QUEUE(cmd, index, LOAD_INST_END_CMD, 0, driver_type, 0);                              \
}

#ifdef __cplusplus
}
#endif

#endif /* DRV_COMMON_H */
