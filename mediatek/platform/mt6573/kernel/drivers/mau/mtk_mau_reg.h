


#ifndef __MTK_MAU_REG_H__
#define __MTK_MAU_REG_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    unsigned STR    : 30;
    unsigned RD     : 1;
    unsigned WR     : 1;
} MAU_REG_RANGE_START, *PMAU_REG_RANGE_START;

// MPU ASSERT ID bit:  7
// MAU1 ASSERT ID bit: 7
// MAU2 ASSERT ID bit: 6

typedef struct {
    unsigned ASSERT_ID : 7;
    unsigned ASSERT    : 1;
    unsigned rsv_8     : 24;
} MAU_REG_ENT_STATUS, *PMAU_REG_ENT_STATUS;

typedef struct {
    unsigned MAUASRT : 1;
    unsigned SYSASRT : 1;
    unsigned STRVASRT : 1;
    unsigned rsv_3   : 29;

} GMC2_IRQ_STATUS, *PGMC2_IRQ_STATUS;



// MPU:  70081100  @P96
// MAU1: 70082400  @P135
// MAU2: 700B1100  @P216

typedef struct {
    MAU_REG_RANGE_START  ENT0_RANGE_STR;   //00
    unsigned int         ENT0_RANGE_END;   //04
    unsigned int         ENT0_INVAL_LMST;   //08
    unsigned int         ENT0_INVAL_HMST;   //0c

    MAU_REG_RANGE_START  ENT1_RANGE_STR;   //10
    unsigned int         ENT1_RANGE_END;   //14
    unsigned int         ENT1_INVAL_LMST;   //08
    unsigned int         ENT1_INVAL_HMST;   //0c

    MAU_REG_RANGE_START  ENT2_RANGE_STR;   //20
    unsigned int         ENT2_RANGE_END;   //24
    unsigned int         ENT2_INVAL_LMST;   //08
    unsigned int         ENT2_INVAL_HMST;   //0c

    MAU_REG_ENT_STATUS   ENT0_STATUS;      //30
    MAU_REG_ENT_STATUS   ENT1_STATUS;      //34
    MAU_REG_ENT_STATUS   ENT2_STATUS;      //38

    unsigned int         rsv_3C;           //3C

    unsigned int         INTERRUPT;       //40
} volatile MAU_REGS, *PMAU_REGS;



extern PMAU_REGS const MPU_REG;
extern PMAU_REGS const MAU1_REG;
extern PMAU_REGS const MAU2_REG;


#ifdef __cplusplus
}
#endif

#endif // __MTK_MAU_REG_H__



#define REG_GMC1_MMUEN0                 0xC00 // one bit for each port, 1:enable M4U
#define REG_GMC1_MMUEN1                 0xC04
#define REG_GMC1_SECURITY_CON0          0x510 //port security control bit
#define REG_GMC1_SECURITY_CON1          0x514
#define REG_GMC1_SECURITY_CON2          0x518
#define REG_GMC1_SECURITY_CON3          0x51c
#define REG_GMC1_SECURITY_CON4          0x520


//@p117
#define MAU_REG_GMC2_CON_RD 0xF70B1010
#define MAU_REG_GMC2_CON_WT 0xF70B1014
#define MAU_REG_GMC2_CON_CLR 0xF70B1018




