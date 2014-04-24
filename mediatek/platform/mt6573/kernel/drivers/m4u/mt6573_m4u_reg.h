 
#ifndef _MT6573_M4U_REG_H
#define _MT6573_M4U_REG_H

#include <asm/io.h>
	 
#define REG_MMU_PT_BASE_ADDR            0x0
#define REG_MMU_INVALIDATE              0x004
#define INVALIDATE_ALL         0x2
#define INVALIDATE_RANGE       0x1
#define REG_MMU_INVLD_SA                0x008

#define REG_MMU_INVLD_EA                0x00C

#define REG_MMU_PROG_EN                 0x010

#define ENABLE_MANUAL_MMU   0x1
#define REG_MMU_PROG_VA                 0x014

#define DSC_LOCK_BIT (0x1<<11)
#define REG_MMU_PROG_DSC                0x018

#define REG_MMU_RT_START_A0             0x020

#define RANGE_ENABLE (0x1<<11)
#define REG_MMU_RT_END_A0               0x024

#define REG_MMU_RT_START_A1             0x028
#define REG_MMU_RT_END_A1               0x02C
#define REG_MMU_RT_START_A2             0x030
#define REG_MMU_RT_END_A2               0x034
#define REG_MMU_RT_START_A3             0x038
#define REG_MMU_RT_END_A3               0x03C
#define REG_MMU_SQ_START_A0             0x040
#define REG_MMU_SQ_END_A0               0x044
#define REG_MMU_SQ_START_A1             0x048
#define REG_MMU_SQ_END_A1               0x04C
#define REG_MMU_SQ_START_A2             0x050
#define REG_MMU_SQ_END_A2               0x054
#define REG_MMU_SQ_START_A3             0x058
#define REG_MMU_SQ_END_A3               0x05C
#define REG_MMU_PFH_DIST0               0x080

#define REG_MMU_PFH_DIST1               0x084

#define REG_MMU_PFH_DIST2               0x088

#define REG_MMU_PFH_DIST3               0x08C

#define REG_MMU_PFH_DIST4               0x090

#define REG_MMU_PFH_DIR0                0x0F0

#define REG_MMU_MAIN_TAG0               0x100

#define REG_MMU_MAIN_TAG1               0x104
#define REG_MMU_MAIN_TAG2               0x108
#define REG_MMU_MAIN_TAG3               0x10C
#define REG_MMU_MAIN_TAG4               0x110
#define REG_MMU_MAIN_TAG5               0x114
#define REG_MMU_MAIN_TAG6               0x118
#define REG_MMU_MAIN_TAG7               0x11C
#define REG_MMU_MAIN_TAG8               0x120
#define REG_MMU_MAIN_TAG9               0x124
#define REG_MMU_MAIN_TAG10              0x128
#define REG_MMU_MAIN_TAG11              0x12C
#define REG_MMU_MAIN_TAG12              0x130
#define REG_MMU_MAIN_TAG13              0x134
#define REG_MMU_MAIN_TAG14              0x138
#define REG_MMU_MAIN_TAG15              0x13C
#define REG_MMU_MAIN_TAG16              0x140
#define REG_MMU_MAIN_TAG17              0x144
#define REG_MMU_MAIN_TAG18              0x148
#define REG_MMU_MAIN_TAG19              0x14C
#define REG_MMU_MAIN_TAG20              0x150
#define REG_MMU_MAIN_TAG21              0x154
#define REG_MMU_MAIN_TAG22              0x158
#define REG_MMU_MAIN_TAG23              0x15C
#define REG_MMU_MAIN_TAG24              0x160
#define REG_MMU_MAIN_TAG25              0x164
#define REG_MMU_MAIN_TAG26              0x168
#define REG_MMU_MAIN_TAG27              0x16C
#define REG_MMU_MAIN_TAG28              0x170
#define REG_MMU_MAIN_TAG29              0x174
#define REG_MMU_MAIN_TAG30              0x178
#define REG_MMU_MAIN_TAG31              0x17C
#define REG_MMU_PRE_TAG0                0x180

#define REG_MMU_PRE_TAG1                0x184
#define REG_MMU_PRE_TAG2                0x188
#define REG_MMU_PRE_TAG3                0x18C
#define REG_MMU_PRE_TAG4                0x190
#define REG_MMU_PRE_TAG5                0x194
#define REG_MMU_PRE_TAG6                0x198
#define REG_MMU_PRE_TAG7                0x19C
#define REG_MMU_PRE_TAG8                0x1A0
#define REG_MMU_PRE_TAG9                0x1A4
#define REG_MMU_PRE_TAG10               0x1A8
#define REG_MMU_PRE_TAG11               0x1AC
#define REG_MMU_PRE_TAG12               0x1B0
#define REG_MMU_PRE_TAG13               0x1B4
#define REG_MMU_PRE_TAG14               0x1B8
#define REG_MMU_PRE_TAG15               0x1BC
#define REG_MMU_PRE_TAG16               0x1C0
#define REG_MMU_PRE_TAG17               0x1C4
#define REG_MMU_PRE_TAG18               0x1C8
#define REG_MMU_PRE_TAG19               0x1CC
#define REG_MMU_PRE_TAG20               0x1D0
#define REG_MMU_PRE_TAG21               0x1D4
#define REG_MMU_PRE_TAG22               0x1D8
#define REG_MMU_PRE_TAG23               0x1DC
#define REG_MMU_PRE_TAG24               0x1E0
#define REG_MMU_PRE_TAG25               0x1E4
#define REG_MMU_PRE_TAG26               0x1E8
#define REG_MMU_PRE_TAG27               0x1EC
#define REG_MMU_PRE_TAG28               0x1F0
#define REG_MMU_PRE_TAG29               0x1F4
#define REG_MMU_PRE_TAG30               0x1F8
#define REG_MMU_PRE_TAG31               0x1FC
#define REG_MMU_READ_ENTRY              0x200

#define REG_MMU_DES_RDATA               0x204

#define REG_MMU_CTRL_REG                0x210

#define REG_MMU_IVRP_PADDR              0x214

#define REG_MMU_INT_CONTROL             0x220

#define INT_CLR_BIT (0x1<<12)
#define REG_MMU_FAULT_ST                0x224

#define INT_TRANSLATION_FAULT               0x01
#define INT_TLB_MULTI_HIT_FAULT             0x02
#define INT_INVALID_PHYSICAL_ADDRESS_FAULT  0x04
#define INT_ENTRY_REPLACEMENT_FAULT         0x08
#define INT_TABLE_WALK_FAULT                0x10
#define INT_TLB_MISS_FAULT                  0x20
#define INT_PRE_FETCH_DMA_FIFO_OVERFLOW     0x40
#define INT_VA_TO_PA_MAPPING_FAULT          0x80
	 
#define REG_MMU_FAULT_VA                0x228

#define REG_MMU_INVLD_PA                0x22C

#define REG_MMU_ACC_CNT                 0x230

#define REG_MMU_MAIN_MSCNT              0x234

#define REG_MMU_PF_MSCNT                0x238
#define REG_MMU_PF_CNT                  0x23C
	 

	 /// MAU
#define REG_MMU_MAU_ENTR0_START         0x400

#define RANGE_CHECK_READ_BIT            (1<<30)
#define RANGE_CHECK_WRITE_BIT           (1<<31)
#define REG_MMU_MAU_ENTR0_END           0x404

#define REG_MMU_MAU_ENTR0_GID           0x408

#define REG_MMU_MAU_ENTR1_START         0x410
#define REG_MMU_MAU_ENTR1_END           0x414
#define REG_MMU_MAU_ENTR1_GID           0x418
#define REG_MMU_MAU_ENTR2_START         0x420
#define REG_MMU_MAU_ENTR2_END           0x424
#define REG_MMU_MAU_ENTR2_GID           0x428
#define REG_MMU_MAU_ENTR0_STA           0x430  //status

#define MAU_RANGE_ASSERTION_BIT         0x80
#define MAU_RANGE_ASSERTION_ID_MASK     0x7F
#define REG_MMU_MAU_ENTR1_STA           0x434
#define REG_MMU_MAU_ENTR2_STA           0x438
#define REG_MMU_MAU_IRQ_EN              0x440
#define IRQ_MAU_ASSERT_EN               0x1
	 
#define REG_MMU_GMC1_MON_AXI_ENA        0x500
#define REG_MMU_GMC1_MON_AXI_CLR        0x504
#define REG_MMU_GMC1_MON_AXI_TYPE       0x508
#define REG_MMU_GMC1_MON_AXI_CON        0x50C
#define REG_MMU_GMC1_MON_AXI_ACT_CNT    0x510
#define REG_MMU_GMC1_MON_AXI_IDL_CNT    0x514
#define REG_MMU_GMC1_MON_AXI_REQ_CNT    0x518
#define REG_MMU_GMC1_MON_AXI_BEA_CNT    0x51C
#define REG_MMU_GMC1_MON_AXI_BYT_CNT    0x520
#define REG_MMU_GMC1_MON_AXI_CP_CNT     0x524
#define REG_MMU_GMC1_MON_AXI_DP_CNT     0x528
#define REG_MMU_GMC1_MON_AXI_CDP_MAX    0x52C
#define REG_MMU_GMC1_MON_AXI_COS_MAX    0x530
	 
 /* ===============================================================
  * 					  GMC definition
  * =============================================================== */
	 /// GMC registers
#define REG_GMC1_CTRL_STATUS            0x010
#define REG_GMC1_CTRL_SET               0x014
#define MPU_EN_BIT                      0x4

	 /// MPU
#define REG_GMC1_RANGE0_SA              0x100

#define REG_GMC1_RANGE0_EA              0x104

#define RANGE_VIRTUAL_ADDR_BIT          (1<<30)
#define RANGE_IN_IRAM_STATUS_BIT        (1<<31)
#define REG_GMC1_RANGE0_INV_MASTERS_LO  0x108

#define REG_GMC1_RANGE0_INV_MASTERS_HI  0x10C
#define REG_GMC1_RANGE1_SA              0x110
#define REG_GMC1_RANGE1_EA              0x114
#define REG_GMC1_RANGE1_INV_MASTERS_LO  0x118
#define REG_GMC1_RANGE1_INV_MASTERS_HI  0x11C
#define REG_GMC1_RANGE2_SA              0x120
#define REG_GMC1_RANGE2_EA              0x124
#define REG_GMC1_RANGE2_INV_MASTERS_LO  0x128
#define REG_GMC1_RANGE2_INV_MASTERS_HI  0x12C
#define REG_GMC1_RANGE0_STATUS          0x130
#define REG_GMC1_RANGE1_STATUS          0x134
#define REG_GMC1_RANGE2_STATUS          0x138
#define REG_GMC1_IRQ_STATUS             0x140
#define REG_GMC1_MMUEN0                 0xC00 // one bit for each port, 1:enable M4U
#define REG_GMC1_MMUEN1                 0xC04
#define REG_GMC1_SECURITY_CON0          0x510 //port security control bit
#define REG_GMC1_SECURITY_CON1          0x514	 
#define REG_GMC1_SECURITY_CON2          0x518
#define REG_GMC1_SECURITY_CON3          0x51c	
#define REG_GMC1_SECURITY_CON4          0x520

#define REG_RMMU_MMU_CON          0x0 
#define REG_RMMU_VIR_BASE         0x04
#define REG_RMMU_PAGE_BASE        0x08
#define REG_RMMU_PAGE_SIZE        0x0C
#define REG_RMMU_INT_BASE         0x10
#define REG_RMMU_INT_SIZE         0x14
#define REG_RMMU_RESET            0x20
#define REG_RMMU_STA              0x24
#define REG_RMMU_IRQ_FLAG         0x30
#define REG_RMMU_IRQ_CLR          0x34


#define GMC1_BASE 0xF7081000
#define GMC1_MMU_BASE 0xF7082000

static inline unsigned int M4U_ReadReg32(unsigned int Offset) 
{
  return ioread32(GMC1_MMU_BASE+Offset);
}
static inline void M4U_WriteReg32(unsigned int Offset, unsigned int VAL) 
{                   
  iowrite32(VAL, GMC1_MMU_BASE+Offset);  
}

static inline unsigned int GMC_ReadReg32(unsigned int Offset) 
{
  return ioread32(GMC1_BASE+Offset);
}
static inline void GMC_WriteReg32(unsigned int Offset, unsigned int VAL)
{                   
  iowrite32(VAL, GMC1_BASE+Offset);
}

static inline unsigned int RMMU_ReadReg32(unsigned int Base, unsigned int Offset) 
{
  return ioread32(Base+Offset);
}
static inline void RMMU_WriteReg32(unsigned int Base, unsigned int Offset, unsigned int VAL)
{                   
  iowrite32(VAL, Base+Offset);
}

#endif //_MMU_REG_H

