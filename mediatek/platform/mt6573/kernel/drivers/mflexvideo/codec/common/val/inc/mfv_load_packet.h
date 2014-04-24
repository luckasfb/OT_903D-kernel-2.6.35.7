

#ifndef MFV_LOAD_PACKET_H
#define MFV_LOAD_PACKET_H

#include "val_types.h"

#define LOAD_INSTRUCTION_AUTO

#ifdef __cplusplus
extern "C" {
#endif

VAL_INT32_T load_packet(VAL_CHAR_T *packet_name);
VAL_INT32_T load_instruction(P_MFV_DRV_CMD_T a_prCmd);
VAL_INT32_T load_instruction_rv9(VAL_UINT32_T u4basePa, VAL_UINT32_T u4baseVa, VAL_UINT32_T u4svld0);
VAL_INT32_T read_instruction(VAL_UINT32_T u4base, VAL_UINT32_T u4size);
VAL_VOID_T dump_instruction_memory();



#ifdef __cplusplus
}
#endif

#endif /* MFV_LOAD_PACKET_H */
