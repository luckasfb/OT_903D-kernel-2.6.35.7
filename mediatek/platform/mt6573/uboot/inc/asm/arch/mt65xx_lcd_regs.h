

#ifndef __LCD_REG_H__
#define __LCD_REG_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif



typedef struct
{
    unsigned RUN        : 1;
    unsigned WAIT_CMDQ  : 1;
    unsigned WAIT_HWTR  : 1;
    unsigned WAIT_VSYNC : 1;
    unsigned WAIT_SYNC  : 1;
    unsigned BUSY       : 1;
    unsigned GMC        : 1;
    unsigned rsv_4      : 12;
} LCD_REG_STATUS, *PLCD_REG_STATUS;


typedef struct
{
    unsigned COMPLETED      : 1;
    unsigned REG_COMPLETED  : 1;
    unsigned CMDQ_COMPLETED : 1;
    unsigned HW_TRIG        : 1;
    unsigned VSYNC          : 1;
    unsigned SYNC           : 1;
    unsigned rsv_6          : 10;
} LCD_REG_INTERRUPT, *PLCD_REG_INTERRUPT;

 
typedef struct
{
    unsigned rsv_0     : 15;
    unsigned START     : 1;
    unsigned rsv_16    : 16;
} LCD_REG_START, *PLCD_REG_START;


typedef struct
{
    unsigned SPO        : 1;
    unsigned SPH        : 1;
    unsigned DIV        : 2;
    unsigned THREE_WIRE : 1;
    unsigned SDA_DIR    : 1;
    unsigned rsv_6      : 2;
    unsigned CSP0       : 1;
    unsigned CSP1       : 1;
    unsigned MODE       : 1;
    unsigned rsv_11     : 1;
    unsigned GAMMA_ID   : 2;
    unsigned IF_CLK     : 2;
    unsigned rsv_16     : 16;
} LCD_REG_SCNF, *PLCD_REG_SCNF;

typedef struct
{
    unsigned WST        : 6;
    unsigned rsv_6      : 2;
    unsigned C2WS       : 4;
    unsigned C2WH       : 2;
    unsigned rsv_14     : 2;
    unsigned RLT        : 6;
    unsigned rsv_22     : 2;
    unsigned C2RS       : 4;
    unsigned C2RH       : 2;
    unsigned rsv_30     : 2;
} LCD_REG_PCNF, *PLCD_REG_PCNF;

typedef struct
{
    unsigned ENABLE     : 1;
    unsigned EDGE_SEL   : 1;
    unsigned MODE       : 1;
    unsigned TE_REPEAT  : 1;
    unsigned HS_MCH_CNT : 11;
    unsigned SW_TE      : 1;
    unsigned VS_CNT_DIV : 2;
    unsigned rsv_18     : 2;
    unsigned VS_WLMT    : 12;
} LCD_REG_TECON, *PLCD_REG_TECON;

typedef struct
{
    unsigned PCNF0_DW   : 3;
    unsigned rsv_3      : 5;
    unsigned PCNF1_DW   : 3;
    unsigned rsv_11     : 5;
    unsigned PCNF2_DW   : 3;
    unsigned rsv_19     : 13;
} LCD_REG_PCNFDW, *PLCD_REG_PCNFDW;

typedef struct
{
    UINT16 WIDTH;
    UINT16 HEIGHT;
} LCD_REG_SIZE, *PLCD_REG_SIZE;

typedef struct
{
    UINT16 X;
    UINT16 Y;
} LCD_REG_COORD, *PLCD_REG_COORD;



// TODO: not modified for MT6573
typedef struct
{
    unsigned rsv_9      : 4;
    unsigned DBS_EN     : 1;
    unsigned rsv_14     : 18;
} LCD_REG_WROI_FMT, *PLCD_REG_WROI_FMT;


typedef struct
{
    unsigned RGB_ORDER      : 1;
    unsigned BYTE_ORDER     : 1;
    unsigned PADDING        : 1;
    unsigned DATA_FMT       : 3;
    unsigned IF_FMT         : 2;
    unsigned COMMAND        : 5;
    unsigned COM_SEL        : 1;
    unsigned W2M            : 1;
    unsigned ENC            : 1;
    unsigned PERIOD         : 8;
    unsigned SEND_RES_MODE  : 1;
    unsigned IF_24          : 1;
    unsigned EN5            : 1;
    unsigned EN4            : 1;
    unsigned EN3            : 1;
    unsigned EN2            : 1;
    unsigned EN1            : 1;
    unsigned EN0            : 1;
} LCD_REG_WROI_CON, *PLCD_REG_WROI_CON;


typedef struct
{
    unsigned SWREG      : 1;
    unsigned HWREF_MOD  : 1;
    unsigned SWREF_MOD  : 1;
    unsigned rsv_3      : 4;
    unsigned HWEN       : 1;
    unsigned rsv_8      : 8;
    unsigned HWREF_SEL  : 2;
    unsigned rsv_18     : 8;    
    unsigned EN5        : 1;
    unsigned EN4        : 1;
    unsigned EN3        : 1;
    unsigned EN2        : 1;
    unsigned EN1        : 1;
    unsigned EN0        : 1;
} LCD_WROI_HWREF, *PLCD_WROI_HWREF;


typedef struct
{
    unsigned rsv_0      : 16;
    unsigned AUTO_LOOP  : 1;
    unsigned rsv_17     : 9;
    unsigned EN5        : 1;
    unsigned EN4        : 1;
    unsigned EN3        : 1;
    unsigned EN2        : 1;
    unsigned EN1        : 1;
    unsigned EN0        : 1;
} LCD_REG_WROI_DC, *PLCD_REG_WROI_DC;


typedef struct
{
    unsigned OPA        : 8;
    unsigned OPAEN      : 1;
    unsigned rsv_9      : 2;
    unsigned ROTATE     : 3;
    unsigned SRC_KEY_EN : 1;
    unsigned SRC        : 1;
    unsigned SWP        : 1;
    unsigned SRCL_EN    : 1;
    unsigned DITHER_EN  : 1;
    unsigned GMA_EN     : 1;
    unsigned CLRDPT     : 3;
    unsigned rsv_23     : 1;
    unsigned DST_KEY_EN : 1;
    unsigned MATRIX_EN  : 1;
    unsigned RGB_SWAP   : 1;
    unsigned rsv_21     : 5;
} LCD_REG_LAYER_CON, *PLCD_REG_LAYER_CON;


typedef struct
{
    LCD_REG_LAYER_CON CONTROL;
    UINT32            COLORKEY;
    LCD_REG_COORD     OFFSET;
    UINT32            ADDRESS;
    LCD_REG_SIZE      SIZE;
    LCD_REG_COORD     SCRL_OFFSET;
    UINT32            WINDOW_OFFSET; 
    UINT16            WINDOW_PITCH;               // 001c
    UINT16            rsvr_001e;               //001e
    UINT32            DB_ADD;                  //0020
    UINT32            reserved_0024[3];        // 0024..0030
} LCD_REG_LAYER, *PLCD_REG_LAYER;



// TODO: not modified for MT6573
typedef struct
{
    unsigned COL2   : 10;
    unsigned COL1   : 10;
    unsigned COL0   : 10;
    unsigned rsv_30 : 2;
} LCD_REG_COEF_ROW, *PLCD_REG_COEF_ROW;

// TODO: not modified for MT6573
typedef struct
{
    UINT8  XOFF;
    UINT8  YOFF;
    UINT16 SLOPE;
} LCD_REG_GAMMA, *PLCD_REG_GAMMA;


// TODO: not modified for MT6573
typedef struct {
    UINT32 COMMAND[32];    // 0000..007F
} LCD_REG_COMMAND_QUEUE;



// TODO: not modified for MT6573
typedef struct {
    UINT32 rsv;
} LCD_REG_GAMCON;

typedef struct {
    unsigned MAX_BURST          : 3;
    unsigned rsv_3              : 1;
    unsigned THROTTLE_EN        : 1;
    unsigned rsv_5              : 11;
    unsigned THROTTLE_PERIOD    : 16;
} LCD_REG_GMCCON;


typedef struct {
    unsigned x      : 11;
    unsigned rsv_11 : 5;
    unsigned y      : 11;
    unsigned rsv_27 : 5;
} LCD_REG_W2M_OFFSET;

// TODO: not modified for MT6573
typedef struct
{
    unsigned W2LCM      : 1;
    unsigned W2M_FMT    : 2;
    unsigned DISCON     : 1;
    unsigned ADDINC_DIS : 1;
    unsigned DC_OUTEN   : 1;
    unsigned rsv_6      : 2;
    unsigned OUT_ALPHA  : 8;
    unsigned FB1_EN     : 1;
    unsigned FB2_EN     : 1;
    unsigned rsv_18     : 2;
    unsigned DLY_EN     : 1;
    unsigned rsv_21     : 2;
    unsigned WB_DIS     : 1;
    unsigned rsv_24     : 7;
    unsigned FBSEQ_RST  : 1;
} LCD_REG_WROI_W2MCON, *PLCD_REG_WROI_W2MCON;

typedef struct
{
    unsigned DC_DSI      : 1;
    unsigned BYTE_SWAP   : 1;
    unsigned RGB_SWAP    : 1;
    unsigned PAD_MSB     : 1;
    unsigned CLR_FMT     : 3;
    unsigned rsv_7       : 25;
} LCD_REG_DSI_DC, *PLCD_REG_DSI_DC;

typedef struct
{
    unsigned DB_B        : 2;
    unsigned rsv_2       : 2;
    unsigned DB_G        : 2;
    unsigned rsv_6       : 2;
    unsigned DB_R        : 2;
    unsigned rsv_10      : 2;
    unsigned LFSR_B_SEED : 4;
    unsigned LFSR_G_SEED : 4;
    unsigned LFSR_R_SEED : 4;    
    unsigned rsv_24      : 8;    
} LCD_REG_DITHER_CON, *PLCD_REG_DITHER_CON;


typedef struct
{
    LCD_REG_STATUS            STATUS;             // 3000
    LCD_REG_INTERRUPT         INT_ENABLE;         // 3004
    LCD_REG_INTERRUPT         INT_STATUS;         // 3008
    LCD_REG_START             START;              // 300C
    UINT32                    RESET;              // 3010
    UINT32                    rsv_0014[3];        // 3014..301C
    LCD_REG_SCNF              SERIAL_CFG;         // 3020
    UINT32                    rsv_0024[3];        // 3024..302C
    LCD_REG_PCNF              PARALLEL_CFG[3];    // 3030..3038
    LCD_REG_PCNFDW            PARALLEL_DW;        // 303C
    LCD_REG_GAMCON            GAMCON;             // 3040
    UINT32                    rsv_0044[3];        // 3044..304C
    LCD_REG_TECON             TEARING_CFG;        // 3050
    LCD_REG_GMCCON            GMC_CON;            // 3054
    UINT32                    rsv_0054[2];        // 3058..305C
    UINT32                    WROI_W2M_ADDR[3];   // 3060..3068
    UINT32                    rsv_006c;           // 306C
    UINT16                    W2M_PITCH;          // 3070
    UINT16                    rsv_0072;           // 3072    
    LCD_REG_W2M_OFFSET        WROI_W2M_OFFSET;    // 3074
    LCD_REG_WROI_W2MCON       WROI_W2M_CONTROL;   // 3078
    UINT32                    WROI_COUNT;         // 307C
    LCD_REG_WROI_CON          WROI_CONTROL;       // 3080
    LCD_REG_COORD             WROI_OFFSET;        // 3084
    UINT32                    WROI_CMD_ADDR;      // 3088
    UINT32                    WROI_DATA_ADDR;     // 308c
    LCD_REG_SIZE              WROI_SIZE;          // 3090
    LCD_WROI_HWREF            WROI_HW_REFRESH;    // 3094
    LCD_REG_WROI_DC           WROI_DC;            // 3098
    UINT32                    WROI_BG_COLOR;      // 309C
    LCD_REG_DSI_DC            DS_DSI_CON;         // 30A0
    UINT32                    rsv_00A4[3];        // 30A4..30AC
    LCD_REG_LAYER             LAYER[6];           // 30B0..31CC
    UINT32                    rsv_00D0[4];        // 31D0..31DC
    UINT32                    WROI_HWREF_BLK;     // 31E0
    UINT32                    WROI_HWREF_DLY;     // 31E4
    UINT32                    rsv_01E8[2];        // 31E8..31EC
    LCD_REG_DITHER_CON        DITHER_CON;         // 31F0
    UINT32                    rsv_01F4[3];        // 31F4..31FC
    UINT32                    LGMA_CON[20];       // 3200..324C
    LCD_REG_COEF_ROW          COEF_ROW[6];        // 3250..3264
    UINT32                    rsv_0268[870];      // 3268..3FFC
    UINT32                    PDATA0;             // 4000
    UINT32                    rsv_04004[1023];    // 4004..4FFC
    UINT32                    PDATA1;             // 5000
    UINT32                    rsv_05004[1023];    // 5004..5FFC
    UINT32                    PDATA2;             // 6000
    UINT32                    rsv_06004[2047];    // 6004..8FFC
    UINT32                    SDATA1;             // 8000
    UINT32                    rsv_08004[1023];    // 8004..8FFC
    UINT32                    SDATA0;             // 9000
    UINT32                    rsv_09004[3071];    // 9004..BFFC
    UINT32                    GAMMA[0x0100];      // C000..C3FF
    UINT32                    CMDQ[2][32];        // C400..C4FF
} volatile LCD_REGS, *PLCD_REGS;

#if 0
STATIC_ASSERT(0x30 == sizeof(LCD_REG_LAYER));

STATIC_ASSERT(0x0000 == offsetof(LCD_REGS, STATUS));
STATIC_ASSERT(0x0004 == offsetof(LCD_REGS, INT_ENABLE));
STATIC_ASSERT(0x0020 == offsetof(LCD_REGS, SERIAL_CFG));
STATIC_ASSERT(0x0030 == offsetof(LCD_REGS, PARALLEL_CFG));
STATIC_ASSERT(0x0040 == offsetof(LCD_REGS, GAMCON));
STATIC_ASSERT(0x0050 == offsetof(LCD_REGS, TEARING_CFG));

STATIC_ASSERT(0x0060 == offsetof(LCD_REGS, WROI_W2M_ADDR));
STATIC_ASSERT(0x00B0 == offsetof(LCD_REGS, LAYER));
STATIC_ASSERT(0x01E0 == offsetof(LCD_REGS, WROI_HWREF_BLK));

STATIC_ASSERT(0x01F0 == offsetof(LCD_REGS, DITHER_CON));

STATIC_ASSERT(0x0200 == offsetof(LCD_REGS, LGMA_CON));
STATIC_ASSERT(0x0250 == offsetof(LCD_REGS, COEF_ROW));
STATIC_ASSERT((0x4000-0x3000) == offsetof(LCD_REGS, PDATA0));
STATIC_ASSERT((0x8000-0x3000) == offsetof(LCD_REGS, SDATA1));
STATIC_ASSERT((0xC000-0x3000) == offsetof(LCD_REGS, GAMMA));

STATIC_ASSERT(0x9500 == sizeof(LCD_REGS));
#endif

extern PLCD_REGS const LCD_REG;

#define LCD_A0_LOW_OFFSET  (0x0)
#define LCD_A0_HIGH_OFFSET (0x100)


#endif // __LCD_REG_H__
