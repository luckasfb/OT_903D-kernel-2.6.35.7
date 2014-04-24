
#ifndef _MT6573_GPT_H_
#define _MT6573_GPT_H_

#include <mach/mt6573_typedefs.h>

//The operation mode of GPTn
typedef enum
{
    GPT_ONE_SHOT = 0x0000,
    GPT_REPEAT   = 0x4000
} GPT_CON_MODE;

//The operation mode of XGPTn
typedef enum
{
    XGPT_ONE_SHOT = 0x0000,
    XGPT_REPEAT   = 0x0010,
    XGPT_KEEP_GO  = 0x0020,
    XGPT_FREE_RUN = 0x0030
} XGPT_CON_MODE;

//GPTn input clock frequency
typedef enum
{
    GPT_CLK_DIV_1   = 0x0000,
    GPT_CLK_DIV_2   = 0x0001,
    GPT_CLK_DIV_4   = 0x0002,
    GPT_CLK_DIV_8   = 0x0003,
    GPT_CLK_DIV_16  = 0x0004,
    GPT_CLK_DIV_32  = 0x0005,
    GPT_CLK_DIV_64  = 0x0006,
    GPT_CLK_DIV_128 = 0x0007
} GPT_CLK_DIV;

//XGPTn input clock frequency
typedef enum
{
    XGPT_CLK_DIV_1   = 0x0000,
    XGPT_CLK_DIV_2   = 0x0001,
    XGPT_CLK_DIV_4   = 0x0002,
    XGPT_CLK_DIV_8   = 0x0003,
    XGPT_CLK_DIV_16  = 0x0004,
    XGPT_CLK_DIV_32  = 0x0005,
    XGPT_CLK_DIV_64  = 0x0006,
    XGPT_CLK_DIV_128 = 0x0007
} XGPT_CLK_DIV;

typedef enum
{
    GPT1 = 0,           /* 16K */
    GPT2,               /* 16K */
    GPT3,               /* 16K */
    GPT4,               /* 26M */
    GPT_TOTAL_COUNT
} GPT_NUM;

typedef enum
{
    XGPT1 = 0,          /* 26M */
    XGPT2,              /* 26M */
    XGPT3,              /* 32K */
    XGPT4,              /* 32K */
    XGPT5,              /* 32K */
    XGPT6,              /* 32K */
    XGPT7,              /* 32K */
    XGPT_TOTAL_COUNT
} XGPT_NUM;

typedef struct
{
    void (*gpt1_func)(UINT16);
    void (*gpt2_func)(UINT16);
} GPT_Func;

typedef struct
{
    void (*xgpt2_func)(UINT16);
    void (*xgpt3_func)(UINT16);
    void (*xgpt4_func)(UINT16);
    void (*xgpt5_func)(UINT16);
    void (*xgpt6_func)(UINT16);
    void (*xgpt7_func)(UINT16);
} XGPT_Func;

typedef struct
{
    GPT_NUM num;          //GPT1~2
    GPT_CON_MODE mode;
    GPT_CLK_DIV clkDiv;
    UINT32 u4Timeout;
} GPT_CONFIG;

typedef struct
{
    XGPT_NUM num;          //XGPT2~7
    XGPT_CON_MODE mode;
    XGPT_CLK_DIV clkDiv;
    BOOL bIrqEnable;
    UINT32 u4Compare;
} XGPT_CONFIG;

typedef enum
{
    USED = 0,
    NOT_USED
} Status;

/* for GPT */
extern BOOL GPT_IsStart(GPT_NUM numGPT);
extern void GPT_Start(GPT_NUM numGPT);
extern void GPT_Stop(GPT_NUM numGPT);

extern void GPT_SetOpMode(GPT_NUM numGPT, GPT_CON_MODE mode);
extern GPT_CON_MODE GPT_GetOpMode(GPT_NUM numGPT);
extern void GPT_SetClkDivisor(GPT_NUM numGPT, GPT_CLK_DIV clkDiv);
extern GPT_CLK_DIV GPT_GetClkDivisor(GPT_NUM numGPT);
extern void GPT_SetTimeout(GPT_NUM numGPT, UINT32 u4Dat);
extern UINT32 GPT_GetTimeout(GPT_NUM numGPT);

extern BOOL GPT_Config(GPT_CONFIG config);
extern void GPT_Init(GPT_NUM timerNum, void (*GPT_Callback)(UINT16));

/* for XGPT */
extern void XGPT_EnableIRQ(XGPT_NUM numXGPT);
extern void XGPT_DisableIRQ(XGPT_NUM numXGPT);
extern BOOL XGPT_IsIRQEnable(XGPT_NUM numXGPT);
extern BOOL XGPT_Check_IRQSTA(XGPT_NUM numXGPT);

extern BOOL XGPT_IsStart(XGPT_NUM numXGPT);
extern void XGPT_Start(XGPT_NUM numXGPT);
extern void XGPT_Restart(XGPT_NUM numXGPT);
extern void XGPT_Stop(XGPT_NUM numXGPT);

extern UINT32 XGPT_GetCounter(XGPT_NUM numXGPT);
extern void XGPT_ClearCount(XGPT_NUM numXGPT);
extern void XGPT_SetOpMode(XGPT_NUM numXGPT, XGPT_CON_MODE mode);
extern XGPT_CON_MODE XGPT_GetOpMode(XGPT_NUM numXGPT);

extern void XGPT_SetClkDivisor(XGPT_NUM numXGPT, XGPT_CLK_DIV clkDiv);
extern XGPT_CLK_DIV XGPT_GetClkDivisor(XGPT_NUM numXGPT);
extern void XGPT_SetCompare(XGPT_NUM numXGPT, UINT32 u4Compare);
extern UINT32 XGPT_GetCompare(XGPT_NUM numXGPT);

extern void XGPT_Reset(XGPT_NUM numXGPT);
extern BOOL XGPT_Config(XGPT_CONFIG config);
extern void XGPT_Init(XGPT_NUM timerNum, void (*XGPT_Callback)(UINT16));

#endif
