

#ifndef __TVROT_DRV_H__
#define __TVROT_DRV_H__

#include "disp_drv.h"

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------------

#define TVR_CHECK_RET(expr)             \
    do {                                \
        TVR_STATUS ret = (expr);        \
        ASSERT(TVR_STATUS_OK == ret);   \
    } while (0)

// ---------------------------------------------------------------------------

typedef enum
{	
   TVR_STATUS_OK = 0,

   TVR_STATUS_ERROR,
   TVR_STATUS_INSUFFICIENT_SRAM,
} TVR_STATUS;


typedef enum
{
    TVR_RGB565  = 2,
    TVR_YUYV422 = 4,
} TVR_FORMAT;


typedef enum
{
    TVR_ROT_0   = 0,
    TVR_ROT_90  = 1,
    TVR_ROT_180 = 2,
    TVR_ROT_270 = 3,
} TVR_ROT;

// ---------------------------------------------------------------------------

#define TVR_BUFFERS (2)

typedef struct
{
    UINT32 srcWidth;
    UINT32 srcHeight;
    
    TVR_FORMAT outputFormat;
    UINT32 dstBufAddr[TVR_BUFFERS];

    TVR_ROT rotation;
    BOOL    flip;
} TVR_PARAM;


TVR_STATUS TVR_Init(void);
TVR_STATUS TVR_Deinit(void);

TVR_STATUS TVR_PowerOn(void);
TVR_STATUS TVR_PowerOff(void);

TVR_STATUS TVR_Config(const TVR_PARAM *param);
TVR_STATUS TVR_Start(void);
TVR_STATUS TVR_Stop(void);

// Debug
TVR_STATUS TVR_DumpRegisters(void);

// ---------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __TVROT_DRV_H__
