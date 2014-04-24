


#ifndef _AUDDRV_REGISTER_H
#define _AUDDRV_REGISTER_H


#include <mach/mt6573_reg_base.h>
#include "../power/pmu6573_hw.h"






#define MASK_ALL          (0xFFFFFFFF)
#define AFE_REGION        (0x3CC)

#ifndef AFE_BASE
#define AFE_BASE           0xFD000000
#endif

#ifndef AFE_DATA_BASE
#define AFE_DATA_BASE      0xFD400000
#endif

#define IRQ1_MCU_CLR				(1<<0)
#define IRQ2_MCU_CLR				(1<<1)
#define IRQ_MCU_DAI_SET_CLR	(1<<2)
#define IRQ_MCU_DAI_RST_CLR	(1<<3)

// AudioSys AFE base address
#ifndef AUDIO_AFE_REGS_BASE
#define AUDIO_AFE_REGS_BASE             0xFD000000
#endif

//Audio Slave Mode Buffer
#define AFE_DL1_SYSRAM_BASE (AFE_DATA_BASE + 0x0000)
#define AFE_DL1_SYSRAM_END  (AFE_DATA_BASE + 0x2700)
#define AFE_VUL_SYSRAM_BASE (AFE_DATA_BASE + 0x2700)
#define AFE_VUL_SYSRAM_END  (AFE_DATA_BASE + 0x2880)
#define AFE_DAI_SYSRAM_BASE (AFE_DATA_BASE + 0x2880)
#define AFE_DAI_SYSRAM_END  (AFE_DATA_BASE + 0x2940)
// Analog AFE Base Address
#define ANA_MASK_ALL          (0xffffffff)
#ifndef MIXEDSYS0_BASE
#define MIXEDSYS0_BASE          (0xF702E000)
#endif
#ifndef MIXEDSYS1_BASE
#define MIXEDSYS1_BASE          (0xF702F000)
#endif

#define AUDIO_TOP_CON0    (0x0000)
#define AFE_DAC_CON0      (0x0010)
#define AFE_DAC_CON1      (0x0014)
#define AFE_I2S_IN_CON    (0x0018)
#define AFE_FOC_CON       (0x0170)
#define AFE_DAIBT_CON     (0x001c)
#define AFE_CONN0         (0x0020)
#define AFE_CONN1         (0x0024)
#define AFE_CONN2         (0x0028)
#define AFE_CONN3         (0x002C)
#define AFE_CONN4         (0x0030)
#define AFE_I2S_OUT_CON   (0x0034)
#define AFE_DL1_BASE      (0x0040)
#define AFE_DL1_CUR       (0x0044)
#define AFE_DL1_END       (0x0048)
#define AFE_DL2_BASE      (0x0050)
#define AFE_DL2_CUR       (0x0054)
#define AFE_DL2_END       (0x0058)
#define AFE_I2S_BASE      (0x0060)
#define AFE_I2S_CUR       (0x0064)
#define AFE_I2S_END       (0x0068)
#define AFE_AWB_BASE      (0x0070)
#define AFE_AWB_CUR       (0x0074)
#define AFE_AWB_END       (0x0078)
#define AFE_VUL_CUR       (0x0084)
#define AFE_DAI_CUR       (0x0094)

#define AFE_DL_SRC1_1     (0x0100) //reserved
#define AFE_DL_SRC1_2     (0x0104) //reserved
#define AFE_DL_SRC2_1     (0x0108)
#define AFE_DL_SRC2_2     (0x010C)
#define AFE_DL_SDM_CON0   (0x0110)
#define AFE_UL_SRC_0      (0x0114)
#define AFE_UL_SRC_1      (0x0118)

#define AFE_IRQ_CON       (0x03A0)
#define AFE_IR_STATUS     (0x03A4)
#define AFE_IR_CLR        (0x03A8)
#define AFE_IRQ_CNT1      (0x03AC)
#define AFE_IRQ_CNT2      (0x03B0)
#define AFE_IRQ_MON       (0x03B8)

// IRQ for Modem part
#define AFE_MODEM_IRQ_CON       (0x00A0)
#define AFE_MODEM_IR_STATUS     (0x00A4)
#define AFE_MODEM_IR_CLR        (0x00A8)
#define AFE_MODEM_IRQ_CNT1      (0x00AC)
#define AFE_MODEM_IRQ_CNT2      (0x00B0)


//Register : AGC
#define AFE_UL_AGC0        (0x020c)
#define AFE_UL_AGC1        (0x0120)
#define AFE_UL_AGC2        (0x0124)
#define AFE_UL_AGC3        (0x0128)
#define AFE_UL_AGC4        (0x012C)
#define AFE_UL_AGC5        (0x0130)
#define AFE_UL_AGC6        (0x0134)
#define AFE_UL_AGC7        (0x0138)
#define AFE_UL_AGC8        (0x013C)
#define AFE_UL_AGC9        (0x0140)
#define AFE_UL_AGC10       (0x0144)
#define AFE_UL_AGC11       (0x0148)
#define AFE_UL_AGC12       (0x014C)
#define AFE_UL_AGC13       (0x0150)
#define AFE_UL_AGC14       (0x0154)
#define AFE_UL_AGC15       (0x0158)
#define AFE_UL_AGC16       (0x015C)
#define AFE_UL_AGC17       (0x0160)
#define AFE_UL_AGC18       (0x0164)

#define AFE_SDM_GAIN_STAGE (0x0168) //[31]: 0, [5:0]: gain :default:0x10 -6dB

//SIDETONE
#define AFE_SIDETONE_CON0  (0x01E0)
#define AFE_SIDETONE_CON1  (0x01E4)

//Register : TOP CONTROL
#define AFE_TOP_CONTROL_0  (0x0200)


#define BIT_00	0x00000001        /* ---- ---- ---- ---- ---- ---- ---- ---1 */
#define BIT_01	0x00000002        /* ---- ---- ---- ---- ---- ---- ---- --1- */
#define BIT_02	0x00000004        /* ---- ---- ---- ---- ---- ---- ---- -1-- */
#define BIT_03	0x00000008        /* ---- ---- ---- ---- ---- ---- ---- 1--- */
#define BIT_04	0x00000010        /* ---- ---- ---- ---- ---- ---- ---1 ---- */
#define BIT_05	0x00000020        /* ---- ---- ---- ---- ---- ---- --1- ---- */
#define BIT_06	0x00000040        /* ---- ---- ---- ---- ---- ---- -1-- ---- */
#define BIT_07	0x00000080        /* ---- ---- ---- ---- ---- ---- 1--- ---- */
#define BIT_08	0x00000100        /* ---- ---- ---- ---- ---- ---1 ---- ---- */
#define BIT_09	0x00000200        /* ---- ---- ---- ---- ---- --1- ---- ---- */
#define BIT_10	0x00000400        /* ---- ---- ---- ---- ---- -1-- ---- ---- */
#define BIT_11	0x00000800        /* ---- ---- ---- ---- ---- 1--- ---- ---- */
#define BIT_12	0x00001000        /* ---- ---- ---- ---- ---1 ---- ---- ---- */
#define BIT_13	0x00002000        /* ---- ---- ---- ---- --1- ---- ---- ---- */
#define BIT_14	0x00004000        /* ---- ---- ---- ---- -1-- ---- ---- ---- */
#define BIT_15	0x00008000        /* ---- ---- ---- ---- 1--- ---- ---- ---- */
#define BIT_16	0x00010000        /* ---- ---- ---- ---1 ---- ---- ---- ---- */
#define BIT_17	0x00020000        /* ---- ---- ---- --1- ---- ---- ---- ---- */
#define BIT_18	0x00040000        /* ---- ---- ---- -1-- ---- ---- ---- ---- */
#define BIT_19	0x00080000        /* ---- ---- ---- 1--- ---- ---- ---- ---- */
#define BIT_20	0x00100000        /* ---- ---- ---1 ---- ---- ---- ---- ---- */
#define BIT_21	0x00200000        /* ---- ---- --1- ---- ---- ---- ---- ---- */
#define BIT_22	0x00400000        /* ---- ---- -1-- ---- ---- ---- ---- ---- */
#define BIT_23	0x00800000        /* ---- ---- 1--- ---- ---- ---- ---- ---- */
#define BIT_24	0x01000000        /* ---- ---1 ---- ---- ---- ---- ---- ---- */
#define BIT_25	0x02000000        /* ---- --1- ---- ---- ---- ---- ---- ---- */
#define BIT_26	0x04000000        /* ---- -1-- ---- ---- ---- ---- ---- ---- */
#define BIT_27	0x08000000        /* ---- 1--- ---- ---- ---- ---- ---- ---- */
#define BIT_28	0x10000000        /* ---1 ---- ---- ---- ---- ---- ---- ---- */
#define BIT_29	0x20000000        /* --1- ---- ---- ---- ---- ---- ---- ---- */
#define BIT_30	0x40000000        /* -1-- ---- ---- ---- ---- ---- ---- ---- */
#define BIT_31	0x80000000        /* 1--- ---- ---- ---- ---- ---- ---- ---- */


// MIX_ABB
#define PLL_CON2     (MIXEDSYS0_BASE+0x0108)
#define AUDIO_CON0   (MIXEDSYS0_BASE+0x0900)
#define AUDIO_CON1   (MIXEDSYS0_BASE+0x0904)
#define AUDIO_CON2   (MIXEDSYS0_BASE+0x0908)
#define AUDIO_CON3   (MIXEDSYS0_BASE+0x090C)
#define AUDIO_CON4   (MIXEDSYS0_BASE+0x0910)
#define AUDIO_CON5   (MIXEDSYS0_BASE+0x0914)
#define AUDIO_CON6   (MIXEDSYS0_BASE+0x0918)
#define AUDIO_CON7   (MIXEDSYS0_BASE+0x091C)
#define AUDIO_CON8   (MIXEDSYS0_BASE+0x0920)
#define AUDIO_CON9   (MIXEDSYS0_BASE+0x0924)
#define AUDIO_CON10  (MIXEDSYS0_BASE+0x0928)
#define AUDIO_CON20  (MIXEDSYS0_BASE+0x0980)
#define AUDIO_CON21  (MIXEDSYS0_BASE+0x0984)
#define AUDIO_CON22  (MIXEDSYS0_BASE+0x0988)
#define AUDIO_CON23  (MIXEDSYS0_BASE+0x098C)
#define AUDIO_CON24  (MIXEDSYS0_BASE+0x0990)
#define AUDIO_CON25  (MIXEDSYS0_BASE+0x0994)
#define AUDIO_CON26  (MIXEDSYS0_BASE+0x0998)
#define AUDIO_CON27  (MIXEDSYS0_BASE+0x099C)
#define AUDIO_CON28  (MIXEDSYS0_BASE+0x09A0)
#define AUDIO_CON29  (MIXEDSYS0_BASE+0x09A4)
#define AUDIO_CON30  (MIXEDSYS0_BASE+0x09A8)
#define AUDIO_CON31  (MIXEDSYS0_BASE+0x09AC)
#define AUDIO_CON32  (MIXEDSYS0_BASE+0x09B0)

#define ACIF_WR_PATH (MIXEDSYS0_BASE+0x0800)

// MIX_PMU
#define VAUDP_CON1    (VAUDP_CON0+0x4)
#define VAUDP_CON2    (VAUDP_CON0+0x8)


void Afe_Set_Reg(kal_uint32 offset,kal_uint32 value,kal_uint32 mask);
kal_uint32 Afe_Get_Reg(kal_uint32 offset);

void   Ana_Set_Reg(kal_uint32 offset,kal_uint32 value,kal_uint32 mask);
kal_uint32 Ana_Get_Reg(kal_uint32 offset);

void Afe_Enable_Memory_Power(void);
void Afe_Disable_Memory_Power(void);

#endif


