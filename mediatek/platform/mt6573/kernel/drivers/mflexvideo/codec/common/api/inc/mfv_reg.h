
#ifndef __mfv_reg_h__
#define __mfv_reg_h__
#ifdef MFV_REGC

#include "register_based_cmodel_framework.h"
#include "mfv_adapter.h"

unsigned int *MFV_RegMap(unsigned int const address);

// ---------------------------------------------------------------------------

#define MFV_REG_COMM_DECL(NAME)                                                           \
  NAME& Bits;                                                                             \
  NAME() : Bits(*this) {}                                                                 \
  NAME(unsigned int address,                                                              \
       unsigned int *(*FUN_RegMap)(unsigned int const) = MFV_RegMap,                      \
       unsigned int reset_val = 0,                                                        \
       RegWriter writer = MFV_HW_WRITE,                                                   \
       RegReader reader = MFV_HW_READ) : Bits(*this)                                      \
  {                                                                                       \
    Init(address, FUN_RegMap, reset_val, writer, reader);                                 \
  }                                                                                       \
                                                                                          \
  NAME &operator=(unsigned int const value) {                                             \
    Raw = value;                                                                          \
    return *this;                                                                         \
  }                                                                                       \
  bool operator==(unsigned int const value) const {                                       \
    return (Raw==value);                                                                  \
  }                                                                                       \
  bool operator!=(unsigned int const value) const {                                       \
    return (Raw!=value);                                                                  \
  }                                                                                       \
                                                                                          \
  void Init(unsigned int address,                                                         \
            unsigned int *(*FUN_RegMap)(unsigned int const) = MFV_RegMap,                 \
            unsigned int reset_val = 0,                                                   \
            RegWriter writer = MFV_HW_WRITE,                                              \
            RegReader reader = MFV_HW_READ)

// ---------------------------------------------------------------------------

class MFV_RESET_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_RESET_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    RESET.Init(this, 0, 0);
  }

  Field RESET;
};


class MFV_START_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_START_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    START.Init(this, 0, 0);
  }

  Field START;
};


class MFV_CLOCK_EN_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_CLOCK_EN_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    CLOCK_EN.Init(this, 15, 0);
  }

  Field CLOCK_EN;
};


class MFV_MEM_SWITCH_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_MEM_SWITCH_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    SWITCH0.Init(this, 1, 0);
    SWITCH1.Init(this, 5, 4);
  }

  Field SWITCH0;
  Field SWITCH1;
};


class MFV_IRQ_MASK_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_IRQ_MASK_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    RISC_DONE.Init(this, 0, 0);
    IDMA_DONE.Init(this, 1, 1);
    RISC_ERROR.Init(this, 2, 2);
    RISC_INT.Init(this, 3, 3);
  }

  Field RISC_DONE;
  Field IDMA_DONE;
  Field RISC_ERROR;
  Field RISC_INT;
};


class MFV_IRQ_ACK_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_IRQ_ACK_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    RISC_DONE.Init(this, 0, 0);
    IDMA_DONE.Init(this, 1, 1);
    RISC_ERROR.Init(this, 2, 2);
    RISC_INT.Init(this, 3, 3);
  }

  Field RISC_DONE;
  Field IDMA_DONE;
  Field RISC_ERROR;
  Field RISC_INT;
};


class MFV_IRQ_STS_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_IRQ_STS_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    RISC_DONE.Init(this, 0, 0);
    IDMA_DONE.Init(this, 1, 1);
    RISC_ERROR.Init(this, 2, 2);
    RISC_INT.Init(this, 3, 3);
  }

  Field RISC_DONE;
  Field IDMA_DONE;
  Field RISC_ERROR;
  Field RISC_INT;
};


class MFV_BS_LIMIT_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_BS_LIMIT_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    BS_LIMIT.Init(this, 15, 0);
  }

  Field BS_LIMIT;
};


class MFV_PIC_CONFIG_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_PIC_CONFIG_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    FRAME_WIDTH.Init(this, 10, 0);
    FRAME_HEIGHT.Init(this, 26, 16);
    PFH_HEIGHT_DIV8.Init(this, 31, 28);
  }

  Field FRAME_WIDTH;
  Field FRAME_HEIGHT;
  Field PFH_HEIGHT_DIV8;
};


class MFV_ENC_CONFIG_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_ENC_CONFIG_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    MB_X_LIMIT_SRC.Init(this, 6, 0);
    MB_Y_LIMIT_SRC.Init(this, 14, 8);
  }

  Field MB_X_LIMIT_SRC;
  Field MB_Y_LIMIT_SRC;
};


class MFV_EIS_CONFIG_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_EIS_CONFIG_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    GMV_X.Init(this, 9, 0);
    GMV_Y.Init(this, 25, 16);
  }

  Field GMV_X;
  Field GMV_Y;
};


class MFV_TVLD_ADDR_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_TVLD_ADDR_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    TVLD_ADDR.Init(this, 31, 0);
  }

  Field TVLD_ADDR;
};


class MFV_SVLD_HEADER_ADDR_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_SVLD_HEADER_ADDR_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    SVLD_HEADER_ADDR.Init(this, 31, 6);
  }

  Field SVLD_HEADER_ADDR;
};


class MFV_SVLD_BODY_ADDR_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_SVLD_BODY_ADDR_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    SVLD_BODY_ADDR.Init(this, 31, 6);
  }

  Field SVLD_BODY_ADDR;
};


class MFV_VLE_ADDR_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_VLE_ADDR_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    VLE_ADDR.Init(this, 31, 6);
  }

  Field VLE_ADDR;
};


class MFV_REC_ADDR_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_REC_ADDR_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    REC_ADDR.Init(this, 31, 4);
  }

  Field REC_ADDR;
};


class MFV_PFH_ADDR_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_PFH_ADDR_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    PFH_ADDR.Init(this, 31, 4);
  }

  Field PFH_ADDR;
};


class MFV_PFH_INFO_ADDR_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_PFH_INFO_ADDR_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    PFH_INFO_ADDR.Init(this, 31, 4);
  }

  Field PFH_INFO_ADDR;
};


class MFV_REF_PTR_ADDR_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_REF_PTR_ADDR_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    REF_PTR_ADDR.Init(this, 31, 0);
  }

  Field REF_PTR_ADDR;
};


class MFV_VOP_ADDR_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_VOP_ADDR_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    VOP_ADDR.Init(this, 31, 3);
  }

  Field VOP_ADDR;
};


class MFV_PFH_MB_NUM_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_PFH_MB_NUM_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    PFH_MB_NUM.Init(this, 8, 0);
  }

  Field PFH_MB_NUM;
};


class MFV_FVLD_ADDR_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_FVLD_ADDR_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    FVLD_ADDR.Init(this, 31, 0);
  }

  Field FVLD_ADDR;
};


class MFV_FVLD_ADDR1_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_FVLD_ADDR1_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    FVLD_ADDR1.Init(this, 31, 0);
  }

  Field FVLD_ADDR1;
};


class MFV_CONTROL_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_CONTROL_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    STEP.Init(this, 0, 0);
    GO.Init(this, 1, 1);
    RUN.Init(this, 2, 2);
    HST_IRQ.Init(this, 3, 3);
    NORMAL_EN.Init(this, 30, 30);
    DEBUG_EN.Init(this, 31, 31);
  }

  Field STEP;
  Field GO;
  Field RUN;
  Field HST_IRQ;
  Field NORMAL_EN;
  Field DEBUG_EN;
};


class MFV_GO_CYCLE_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_GO_CYCLE_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    GO_CYCLE.Init(this, 31, 0);
  }

  Field GO_CYCLE;
};


class MFV_BREAKPOINT_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_BREAKPOINT_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    BREAK_POINT_0.Init(this, 12, 0);
    BREAK_POINT_1.Init(this, 28, 16);
  }

  Field BREAK_POINT_0;
  Field BREAK_POINT_1;
};


class MFV_IDMA_RST_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_IDMA_RST_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    RESET.Init(this, 0, 0);
  }

  Field RESET;
};


class MFV_IDMA_STR_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_IDMA_STR_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    START.Init(this, 0, 0);
  }

  Field START;
};


class MFV_IDMA_PAC_BASE_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_IDMA_PAC_BASE_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    PAC_ADDR.Init(this, 31, 3);
  }

  Field PAC_ADDR;
};


class MFV_IDMA_GMC_BASE_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_IDMA_GMC_BASE_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    GMC_BASE.Init(this, 31, 18);
  }

  Field GMC_BASE;
};


class MFV_IDMA_STATUS_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_IDMA_STATUS_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    STATUS.Init(this, 5, 0);
    BUSY.Init(this, 8, 8);
    GREQ.Init(this, 9, 9);
    RINT.Init(this, 10, 10);
    DATACNT.Init(this, 15, 12);
    CURSIZE.Init(this, 26, 16);
    CHX.Init(this, 28, 28);
    CCX.Init(this, 29, 29);
    CGX.Init(this, 30, 30);
    CPX.Init(this, 31, 31);
  }

  Field STATUS;
  Field BUSY;
  Field GREQ;
  Field RINT;
  Field DATACNT;
  Field CURSIZE;
  Field CHX;
  Field CCX;
  Field CGX;
  Field CPX;
};


class MFV_IDMA_CH_INFO_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_IDMA_CH_INFO_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    TAR_ADDR.Init(this, 14, 0);
    DST_ID.Init(this, 19, 16);
    ACT.Init(this, 20, 20);
  }

  Field TAR_ADDR;
  Field DST_ID;
  Field ACT;
};


class MFV_IDMA_MEM_ADDR_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_IDMA_MEM_ADDR_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    WADDR.Init(this, 15, 0);
    RADDR.Init(this, 31, 16);
  }

  Field WADDR;
  Field RADDR;
};


class MFV_IDMA_GMC_STA_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_IDMA_GMC_STA_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    GADDR.Init(this, 31, 0);
  }

  Field GADDR;
};


class MFV_IFME_KMB_ADDR0_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_IFME_KMB_ADDR0_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    MFV_IFME_KMB_ADDR0.Init(this, 31, 0);
  }

  Field MFV_IFME_KMB_ADDR0;
};


class MFV_IFME_KMB_ADDR1_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_IFME_KMB_ADDR1_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    MFV_IFME_KMB_ADDR1.Init(this, 31, 0);
  }

  Field MFV_IFME_KMB_ADDR1;
};


class MFV_IFME_KMB_NUM_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_IFME_KMB_NUM_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    MFV_IFME_KMB_NUM.Init(this, 31, 0);
  }

  Field MFV_IFME_KMB_NUM;
};


class MFV_CUR_NMB_ADDR_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_CUR_NMB_ADDR_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    MFV_CUR_NMB_ADDR.Init(this, 31, 0);
  }

  Field MFV_CUR_NMB_ADDR;
};


class MFV_CUR_NMB_NUM_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_CUR_NMB_NUM_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    MFV_CUR_NMB_NUM.Init(this, 31, 0);
  }

  Field MFV_CUR_NMB_NUM;
};


class MFV_CONFIG_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_CONFIG_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    MB_X_LIMIT.Init(this, 6, 0);
    MB_Y_LIMIT.Init(this, 14, 8);
    PFH_HEIGHT_DIV8.Init(this, 19, 16);
  }

  Field MB_X_LIMIT;
  Field MB_Y_LIMIT;
  Field PFH_HEIGHT_DIV8;
};


class MFV_HDDEC_SHARE_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_HDDEC_SHARE_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    MFV_USE_NBM.Init(this, 0, 0);
    MFV_USE_VDSL.Init(this, 1, 1);
  }

  Field MFV_USE_NBM;
  Field MFV_USE_VDSL;
};


class MFV_VERSION_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_VERSION_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    VERSION.Init(this, 31, 0);
  }

  Field VERSION;
};


class MFV_RISC_DEBUG_0_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_RISC_DEBUG_0_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    PC_EX.Init(this, 11, 0);
    PERI_IRQ.Init(this, 12, 12);
    VLD_RDY.Init(this, 13, 13);
    WAIT_STATE.Init(this, 25, 14);
    EX_STATE.Init(this, 31, 27);
  }

  Field PC_EX;
  Field PERI_IRQ;
  Field VLD_RDY;
  Field WAIT_STATE;
  Field EX_STATE;
};


class MFV_RISC_DEBUG_1_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_RISC_DEBUG_1_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    A00.Init(this, 0, 0);
    A01.Init(this, 1, 1);
    A02.Init(this, 2, 2);
    A03.Init(this, 3, 3);
    A04.Init(this, 4, 4);
    A05.Init(this, 5, 5);
    A06.Init(this, 6, 6);
    A07.Init(this, 7, 7);
    A08.Init(this, 8, 8);
    A09.Init(this, 9, 9);
    A10.Init(this, 10, 10);
    A11.Init(this, 11, 11);
    A12.Init(this, 12, 12);
    A13.Init(this, 13, 13);
    A14.Init(this, 14, 14);
    A15.Init(this, 15, 15);
  }

  Field A00;
  Field A01;
  Field A02;
  Field A03;
  Field A04;
  Field A05;
  Field A06;
  Field A07;
  Field A08;
  Field A09;
  Field A10;
  Field A11;
  Field A12;
  Field A13;
  Field A14;
  Field A15;
};


class MFV_RISC_DEBUG_2_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_RISC_DEBUG_2_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    D00.Init(this, 7, 0);
    D01.Init(this, 15, 8);
    D02.Init(this, 23, 16);
    D03.Init(this, 31, 24);
  }

  Field D00;
  Field D01;
  Field D02;
  Field D03;
};


class MFV_RISC_DEBUG_3_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_RISC_DEBUG_3_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    E00.Init(this, 15, 0);
    E01.Init(this, 31, 16);
  }

  Field E00;
  Field E01;
};


class MFV_RISC_DEBUG_4_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_RISC_DEBUG_4_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    F00.Init(this, 31, 0);
  }

  Field F00;
};


class MFV_RISC_DEBUG_5_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_RISC_DEBUG_5_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    RISC_COUNTER.Init(this, 31, 0);
  }

  Field RISC_COUNTER;
};


class MFV_RISC_DEBUG_6_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_RISC_DEBUG_6_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    OP2_EX.Init(this, 31, 0);
  }

  Field OP2_EX;
};


class MFV_RISC_DEBUG_7_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_RISC_DEBUG_7_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    OP3_EX.Init(this, 31, 0);
  }

  Field OP3_EX;
};


class MFV_RISC_DEBUG_8_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_RISC_DEBUG_8_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    SET_VAL.Init(this, 31, 0);
  }

  Field SET_VAL;
};


class MFV_RISC_DEBUG_9_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_RISC_DEBUG_9_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    VLD_CODEWORD.Init(this, 31, 0);
  }

  Field VLD_CODEWORD;
};


class MFV_RISC_DEBUG_10_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_RISC_DEBUG_10_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    TOTAL_BITCNT.Init(this, 15, 0);
  }

  Field TOTAL_BITCNT;
};


class MFV_RISC_DEBUG_11_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_RISC_DEBUG_11_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    MB_Y_P0.Init(this, 6, 0);
    MB_X_P0.Init(this, 14, 8);
    BLOCK_P0.Init(this, 20, 16);
  }

  Field MB_Y_P0;
  Field MB_X_P0;
  Field BLOCK_P0;
};


class MFV_PRED_DEBUG_0_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_PRED_DEBUG_0_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    PC_EX.Init(this, 12, 0);
    START_EN.Init(this, 16, 16);
    BUSY.Init(this, 17, 17);
    GREQ.Init(this, 18, 18);
    MEM_STATE.Init(this, 19, 19);
  }

  Field PC_EX;
  Field START_EN;
  Field BUSY;
  Field GREQ;
  Field MEM_STATE;
};


class MFV_PRED_DEBUG_1_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_PRED_DEBUG_1_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    A00.Init(this, 0, 0);
    A01.Init(this, 1, 1);
    A02.Init(this, 2, 2);
    A03.Init(this, 3, 3);
    A04.Init(this, 4, 4);
    A05.Init(this, 5, 5);
    A06.Init(this, 6, 6);
    A07.Init(this, 7, 7);
    A08.Init(this, 8, 8);
    A09.Init(this, 9, 9);
    A10.Init(this, 10, 10);
    A11.Init(this, 11, 11);
    A12.Init(this, 12, 12);
    A13.Init(this, 13, 13);
    A14.Init(this, 14, 14);
    A15.Init(this, 15, 15);
    A16.Init(this, 16, 16);
    A17.Init(this, 17, 17);
    A18.Init(this, 18, 18);
    A19.Init(this, 19, 19);
    A20.Init(this, 20, 20);
    A21.Init(this, 21, 21);
    A22.Init(this, 22, 22);
    A23.Init(this, 23, 23);
    A24.Init(this, 24, 24);
    A25.Init(this, 25, 25);
    A26.Init(this, 26, 26);
    A27.Init(this, 27, 27);
    A28.Init(this, 28, 28);
    A29.Init(this, 29, 29);
    A30.Init(this, 30, 30);
    A31.Init(this, 31, 31);
  }

  Field A00;
  Field A01;
  Field A02;
  Field A03;
  Field A04;
  Field A05;
  Field A06;
  Field A07;
  Field A08;
  Field A09;
  Field A10;
  Field A11;
  Field A12;
  Field A13;
  Field A14;
  Field A15;
  Field A16;
  Field A17;
  Field A18;
  Field A19;
  Field A20;
  Field A21;
  Field A22;
  Field A23;
  Field A24;
  Field A25;
  Field A26;
  Field A27;
  Field A28;
  Field A29;
  Field A30;
  Field A31;
};


class MFV_PRED_DEBUG_2_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_PRED_DEBUG_2_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    D00.Init(this, 7, 0);
    D01.Init(this, 15, 8);
    D02.Init(this, 23, 16);
    D03.Init(this, 31, 24);
  }

  Field D00;
  Field D01;
  Field D02;
  Field D03;
};


class MFV_PRED_DEBUG_3_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_PRED_DEBUG_3_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    D00.Init(this, 15, 0);
    D01.Init(this, 31, 16);
  }

  Field D00;
  Field D01;
};


class MFV_POST_DEBUG_0_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_POST_DEBUG_0_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    PC.Init(this, 10, 0);
    BUSY.Init(this, 12, 12);
    WAIT.Init(this, 13, 13);
    WAIT_STATUS.Init(this, 22, 16);
  }

  Field PC;
  Field BUSY;
  Field WAIT;
  Field WAIT_STATUS;
};


class MFV_POST_DEBUG_1_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_POST_DEBUG_1_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    CSM.Init(this, 7, 0);
    BLK_LIMIT.Init(this, 10, 8);
    ROW_CNT.Init(this, 13, 11);
    ASM.Init(this, 24, 16);
    AUTO.Init(this, 25, 25);
    WRAP_CNT.Init(this, 31, 26);
  }

  Field CSM;
  Field BLK_LIMIT;
  Field ROW_CNT;
  Field ASM;
  Field AUTO;
  Field WRAP_CNT;
};


class MFV_POST_DEBUG_2_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_POST_DEBUG_2_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    G0.Init(this, 15, 0);
    G1.Init(this, 31, 16);
  }

  Field G0;
  Field G1;
};


class MFV_POST_GSTA0_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_POST_GSTA0_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    GADDR.Init(this, 31, 0);
  }

  Field GADDR;
};


class MFV_POST_GSTA1_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_POST_GSTA1_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    GBURST.Init(this, 3, 0);
    GREQ.Init(this, 4, 4);
    GWRITE.Init(this, 5, 5);
    GLCOMD.Init(this, 6, 6);
    GDRDY.Init(this, 7, 7);
    GLWDATA.Init(this, 11, 8);
  }

  Field GBURST;
  Field GREQ;
  Field GWRITE;
  Field GLCOMD;
  Field GDRDY;
  Field GLWDATA;
};


class MFV_RESI_DEBUG_0_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_RESI_DEBUG_0_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    FETCH.Init(this, 17, 0);
    DECODE.Init(this, 22, 18);
    REG.Init(this, 28, 23);
  }

  Field FETCH;
  Field DECODE;
  Field REG;
};


class MFV_RESI_DEBUG_1_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_RESI_DEBUG_1_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    GMC_DEBUG.Init(this, 14, 0);
    INOUT.Init(this, 18, 15);
  }

  Field GMC_DEBUG;
  Field INOUT;
};


class MFV_FVLD_DEBUG_0_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_FVLD_DEBUG_0_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    MEM_DO_L.Init(this, 31, 0);
  }

  Field MEM_DO_L;
};


class MFV_FVLD_DEBUG_1_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_FVLD_DEBUG_1_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    MEM_DO_H.Init(this, 31, 0);
  }

  Field MEM_DO_H;
};


class MFV_FVLD_DEBUG_2_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_FVLD_DEBUG_2_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    RISC_FVLD_TABLE_NUM.Init(this, 14, 0);
    RISC_FVLD_REQ.Init(this, 15, 15);
    MEM_A.Init(this, 27, 16);
    MEM_CS.Init(this, 28, 28);
  }

  Field RISC_FVLD_TABLE_NUM;
  Field RISC_FVLD_REQ;
  Field MEM_A;
  Field MEM_CS;
};


class MFV_FVLD_DEBUG_3_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_FVLD_DEBUG_3_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    FVLD_ERROR_IRQ_ACK.Init(this, 9, 9);
    FVLD_ERROR_IRQ.Init(this, 10, 10);
    FVLD_ERROR.Init(this, 11, 11);
    FVLD_REQ_RDY.Init(this, 12, 12);
    REACH_LEAF.Init(this, 13, 13);
    SKIP.Init(this, 14, 14);
    HEADER_HIT.Init(this, 15, 15);
    STEP2.Init(this, 19, 16);
    STEP1.Init(this, 23, 20);
    STEP0.Init(this, 27, 24);
    STATE.Init(this, 30, 28);
    ENTRY_SIZE.Init(this, 31, 31);
  }

  Field FVLD_ERROR_IRQ_ACK;
  Field FVLD_ERROR_IRQ;
  Field FVLD_ERROR;
  Field FVLD_REQ_RDY;
  Field REACH_LEAF;
  Field SKIP;
  Field HEADER_HIT;
  Field STEP2;
  Field STEP1;
  Field STEP0;
  Field STATE;
  Field ENTRY_SIZE;
};


#define MFV_DMEM_SIZE 256
class MFV_DMEM_REGC : public Reg
{
public:
  MFV_REG_COMM_DECL(MFV_DMEM_REGC)
  {
    Reg::Init(address, FUN_RegMap, reset_val, writer, reader);
    MFV_DMEM.Init(this, 31, 0);
  }

  Field MFV_DMEM;
};


class MFV_REGS
{
public:
  MFV_RESET_REGC MFV_RESET_REG;
  MFV_START_REGC MFV_START_REG;
  MFV_CLOCK_EN_REGC MFV_CLOCK_EN_REG;
  MFV_MEM_SWITCH_REGC MFV_MEM_SWITCH_REG;
  MFV_IRQ_MASK_REGC MFV_IRQ_MASK_REG;
  MFV_IRQ_ACK_REGC MFV_IRQ_ACK_REG;
  MFV_IRQ_STS_REGC MFV_IRQ_STS_REG;
  MFV_BS_LIMIT_REGC MFV_BS_LIMIT_REG;
  MFV_PIC_CONFIG_REGC MFV_PIC_CONFIG_REG;
  MFV_ENC_CONFIG_REGC MFV_ENC_CONFIG_REG;
  MFV_EIS_CONFIG_REGC MFV_EIS_CONFIG_REG;
  MFV_TVLD_ADDR_REGC MFV_TVLD_ADDR_REG;
  MFV_SVLD_HEADER_ADDR_REGC MFV_SVLD_HEADER_ADDR_REG;
  MFV_SVLD_BODY_ADDR_REGC MFV_SVLD_BODY_ADDR_REG;
  MFV_VLE_ADDR_REGC MFV_VLE_ADDR_REG;
  MFV_REC_ADDR_REGC MFV_REC_ADDR_REG;
  MFV_PFH_ADDR_REGC MFV_PFH_ADDR_REG;
  MFV_PFH_INFO_ADDR_REGC MFV_PFH_INFO_ADDR_REG;
  MFV_REF_PTR_ADDR_REGC MFV_REF_PTR_ADDR_REG;
  MFV_VOP_ADDR_REGC MFV_VOP_ADDR_REG;
  MFV_PFH_MB_NUM_REGC MFV_PFH_MB_NUM_REG;
  MFV_FVLD_ADDR_REGC MFV_FVLD_ADDR_REG;
  MFV_FVLD_ADDR1_REGC MFV_FVLD_ADDR1_REG;
  MFV_CONTROL_REGC MFV_CONTROL_REG;
  MFV_GO_CYCLE_REGC MFV_GO_CYCLE_REG;
  MFV_BREAKPOINT_REGC MFV_BREAKPOINT_REG;
  MFV_IDMA_RST_REGC MFV_IDMA_RST_REG;
  MFV_IDMA_STR_REGC MFV_IDMA_STR_REG;
  MFV_IDMA_PAC_BASE_REGC MFV_IDMA_PAC_BASE_REG;
  MFV_IDMA_GMC_BASE_REGC MFV_IDMA_GMC_BASE_REG;
  MFV_IDMA_STATUS_REGC MFV_IDMA_STATUS_REG;
  MFV_IDMA_CH_INFO_REGC MFV_IDMA_CH_INFO_REG;
  MFV_IDMA_MEM_ADDR_REGC MFV_IDMA_MEM_ADDR_REG;
  MFV_IDMA_GMC_STA_REGC MFV_IDMA_GMC_STA_REG;
  MFV_IFME_KMB_ADDR0_REGC MFV_IFME_KMB_ADDR0_REG;
  MFV_IFME_KMB_ADDR1_REGC MFV_IFME_KMB_ADDR1_REG;
  MFV_IFME_KMB_NUM_REGC MFV_IFME_KMB_NUM_REG;
  MFV_CUR_NMB_ADDR_REGC MFV_CUR_NMB_ADDR_REG;
  MFV_CUR_NMB_NUM_REGC MFV_CUR_NMB_NUM_REG;
  MFV_CONFIG_REGC MFV_CONFIG_REG;
  MFV_HDDEC_SHARE_REGC MFV_HDDEC_SHARE_REG;
  MFV_VERSION_REGC MFV_VERSION_REG;
  MFV_RISC_DEBUG_0_REGC MFV_RISC_DEBUG_0_REG;
  MFV_RISC_DEBUG_1_REGC MFV_RISC_DEBUG_1_REG;
  MFV_RISC_DEBUG_2_REGC MFV_RISC_DEBUG_2_REG;
  MFV_RISC_DEBUG_3_REGC MFV_RISC_DEBUG_3_REG;
  MFV_RISC_DEBUG_4_REGC MFV_RISC_DEBUG_4_REG;
  MFV_RISC_DEBUG_5_REGC MFV_RISC_DEBUG_5_REG;
  MFV_RISC_DEBUG_6_REGC MFV_RISC_DEBUG_6_REG;
  MFV_RISC_DEBUG_7_REGC MFV_RISC_DEBUG_7_REG;
  MFV_RISC_DEBUG_8_REGC MFV_RISC_DEBUG_8_REG;
  MFV_RISC_DEBUG_9_REGC MFV_RISC_DEBUG_9_REG;
  MFV_RISC_DEBUG_10_REGC MFV_RISC_DEBUG_10_REG;
  MFV_RISC_DEBUG_11_REGC MFV_RISC_DEBUG_11_REG;
  MFV_PRED_DEBUG_0_REGC MFV_PRED_DEBUG_0_REG;
  MFV_PRED_DEBUG_1_REGC MFV_PRED_DEBUG_1_REG;
  MFV_PRED_DEBUG_2_REGC MFV_PRED_DEBUG_2_REG;
  MFV_PRED_DEBUG_3_REGC MFV_PRED_DEBUG_3_REG;
  MFV_POST_DEBUG_0_REGC MFV_POST_DEBUG_0_REG;
  MFV_POST_DEBUG_1_REGC MFV_POST_DEBUG_1_REG;
  MFV_POST_DEBUG_2_REGC MFV_POST_DEBUG_2_REG;
  MFV_POST_GSTA0_REGC MFV_POST_GSTA0_REG;
  MFV_POST_GSTA1_REGC MFV_POST_GSTA1_REG;
  MFV_RESI_DEBUG_0_REGC MFV_RESI_DEBUG_0_REG;
  MFV_RESI_DEBUG_1_REGC MFV_RESI_DEBUG_1_REG;
  MFV_FVLD_DEBUG_0_REGC MFV_FVLD_DEBUG_0_REG;
  MFV_FVLD_DEBUG_1_REGC MFV_FVLD_DEBUG_1_REG;
  MFV_FVLD_DEBUG_2_REGC MFV_FVLD_DEBUG_2_REG;
  MFV_FVLD_DEBUG_3_REGC MFV_FVLD_DEBUG_3_REG;
  MFV_DMEM_REGC MFV_DMEM_REG[MFV_DMEM_SIZE];
};

// ---------------------------------------------------------------------------

extern class MFV_REGS* P_MFV_REG;
void MFV_INIT(FUN_POINT callback);

extern MFV_RESET_REGC MFV_RESET_REG;
extern MFV_START_REGC MFV_START_REG;
extern MFV_CLOCK_EN_REGC MFV_CLOCK_EN_REG;
extern MFV_MEM_SWITCH_REGC MFV_MEM_SWITCH_REG;
extern MFV_IRQ_MASK_REGC MFV_IRQ_MASK_REG;
extern MFV_IRQ_ACK_REGC MFV_IRQ_ACK_REG;
extern MFV_IRQ_STS_REGC MFV_IRQ_STS_REG;
extern MFV_BS_LIMIT_REGC MFV_BS_LIMIT_REG;
extern MFV_PIC_CONFIG_REGC MFV_PIC_CONFIG_REG;
extern MFV_ENC_CONFIG_REGC MFV_ENC_CONFIG_REG;
extern MFV_EIS_CONFIG_REGC MFV_EIS_CONFIG_REG;
extern MFV_TVLD_ADDR_REGC MFV_TVLD_ADDR_REG;
extern MFV_SVLD_HEADER_ADDR_REGC MFV_SVLD_HEADER_ADDR_REG;
extern MFV_SVLD_BODY_ADDR_REGC MFV_SVLD_BODY_ADDR_REG;
extern MFV_VLE_ADDR_REGC MFV_VLE_ADDR_REG;
extern MFV_REC_ADDR_REGC MFV_REC_ADDR_REG;
extern MFV_PFH_ADDR_REGC MFV_PFH_ADDR_REG;
extern MFV_PFH_INFO_ADDR_REGC MFV_PFH_INFO_ADDR_REG;
extern MFV_REF_PTR_ADDR_REGC MFV_REF_PTR_ADDR_REG;
extern MFV_VOP_ADDR_REGC MFV_VOP_ADDR_REG;
extern MFV_PFH_MB_NUM_REGC MFV_PFH_MB_NUM_REG;
extern MFV_FVLD_ADDR_REGC MFV_FVLD_ADDR_REG;
extern MFV_FVLD_ADDR1_REGC MFV_FVLD_ADDR1_REG;
extern MFV_CONTROL_REGC MFV_CONTROL_REG;
extern MFV_GO_CYCLE_REGC MFV_GO_CYCLE_REG;
extern MFV_BREAKPOINT_REGC MFV_BREAKPOINT_REG;
extern MFV_IDMA_RST_REGC MFV_IDMA_RST_REG;
extern MFV_IDMA_STR_REGC MFV_IDMA_STR_REG;
extern MFV_IDMA_PAC_BASE_REGC MFV_IDMA_PAC_BASE_REG;
extern MFV_IDMA_GMC_BASE_REGC MFV_IDMA_GMC_BASE_REG;
extern MFV_IDMA_STATUS_REGC MFV_IDMA_STATUS_REG;
extern MFV_IDMA_CH_INFO_REGC MFV_IDMA_CH_INFO_REG;
extern MFV_IDMA_MEM_ADDR_REGC MFV_IDMA_MEM_ADDR_REG;
extern MFV_IDMA_GMC_STA_REGC MFV_IDMA_GMC_STA_REG;
extern MFV_IFME_KMB_ADDR0_REGC MFV_IFME_KMB_ADDR0_REG;
extern MFV_IFME_KMB_ADDR1_REGC MFV_IFME_KMB_ADDR1_REG;
extern MFV_IFME_KMB_NUM_REGC MFV_IFME_KMB_NUM_REG;
extern MFV_CUR_NMB_ADDR_REGC MFV_CUR_NMB_ADDR_REG;
extern MFV_CUR_NMB_NUM_REGC MFV_CUR_NMB_NUM_REG;
extern MFV_CONFIG_REGC MFV_CONFIG_REG;
extern MFV_HDDEC_SHARE_REGC MFV_HDDEC_SHARE_REG;
extern MFV_VERSION_REGC MFV_VERSION_REG;
extern MFV_RISC_DEBUG_0_REGC MFV_RISC_DEBUG_0_REG;
extern MFV_RISC_DEBUG_1_REGC MFV_RISC_DEBUG_1_REG;
extern MFV_RISC_DEBUG_2_REGC MFV_RISC_DEBUG_2_REG;
extern MFV_RISC_DEBUG_3_REGC MFV_RISC_DEBUG_3_REG;
extern MFV_RISC_DEBUG_4_REGC MFV_RISC_DEBUG_4_REG;
extern MFV_RISC_DEBUG_5_REGC MFV_RISC_DEBUG_5_REG;
extern MFV_RISC_DEBUG_6_REGC MFV_RISC_DEBUG_6_REG;
extern MFV_RISC_DEBUG_7_REGC MFV_RISC_DEBUG_7_REG;
extern MFV_RISC_DEBUG_8_REGC MFV_RISC_DEBUG_8_REG;
extern MFV_RISC_DEBUG_9_REGC MFV_RISC_DEBUG_9_REG;
extern MFV_RISC_DEBUG_10_REGC MFV_RISC_DEBUG_10_REG;
extern MFV_RISC_DEBUG_11_REGC MFV_RISC_DEBUG_11_REG;
extern MFV_PRED_DEBUG_0_REGC MFV_PRED_DEBUG_0_REG;
extern MFV_PRED_DEBUG_1_REGC MFV_PRED_DEBUG_1_REG;
extern MFV_PRED_DEBUG_2_REGC MFV_PRED_DEBUG_2_REG;
extern MFV_PRED_DEBUG_3_REGC MFV_PRED_DEBUG_3_REG;
extern MFV_POST_DEBUG_0_REGC MFV_POST_DEBUG_0_REG;
extern MFV_POST_DEBUG_1_REGC MFV_POST_DEBUG_1_REG;
extern MFV_POST_DEBUG_2_REGC MFV_POST_DEBUG_2_REG;
extern MFV_POST_GSTA0_REGC MFV_POST_GSTA0_REG;
extern MFV_POST_GSTA1_REGC MFV_POST_GSTA1_REG;
extern MFV_RESI_DEBUG_0_REGC MFV_RESI_DEBUG_0_REG;
extern MFV_RESI_DEBUG_1_REGC MFV_RESI_DEBUG_1_REG;
extern MFV_FVLD_DEBUG_0_REGC MFV_FVLD_DEBUG_0_REG;
extern MFV_FVLD_DEBUG_1_REGC MFV_FVLD_DEBUG_1_REG;
extern MFV_FVLD_DEBUG_2_REGC MFV_FVLD_DEBUG_2_REG;
extern MFV_FVLD_DEBUG_3_REGC MFV_FVLD_DEBUG_3_REG;
extern MFV_DMEM_REGC MFV_DMEM_REG[MFV_DMEM_SIZE];

#define MFV_RESET &MFV_RESET_REG
#define MFV_START &MFV_START_REG
#define MFV_CLOCK_EN &MFV_CLOCK_EN_REG
#define MFV_MEM_SWITCH &MFV_MEM_SWITCH_REG
#define MFV_IRQ_MASK &MFV_IRQ_MASK_REG
#define MFV_IRQ_ACK &MFV_IRQ_ACK_REG
#define MFV_IRQ_STS &MFV_IRQ_STS_REG
#define MFV_BS_LIMIT &MFV_BS_LIMIT_REG
#define MFV_PIC_CONFIG &MFV_PIC_CONFIG_REG
#define MFV_ENC_CONFIG &MFV_ENC_CONFIG_REG
#define MFV_EIS_CONFIG &MFV_EIS_CONFIG_REG
#define MFV_TVLD_ADDR &MFV_TVLD_ADDR_REG
#define MFV_SVLD_HEADER_ADDR &MFV_SVLD_HEADER_ADDR_REG
#define MFV_SVLD_BODY_ADDR &MFV_SVLD_BODY_ADDR_REG
#define MFV_VLE_ADDR &MFV_VLE_ADDR_REG
#define MFV_REC_ADDR &MFV_REC_ADDR_REG
#define MFV_PFH_ADDR &MFV_PFH_ADDR_REG
#define MFV_PFH_INFO_ADDR &MFV_PFH_INFO_ADDR_REG
#define MFV_REF_PTR_ADDR &MFV_REF_PTR_ADDR_REG
#define MFV_VOP_ADDR &MFV_VOP_ADDR_REG
#define MFV_PFH_MB_NUM &MFV_PFH_MB_NUM_REG
#define MFV_FVLD_ADDR &MFV_FVLD_ADDR_REG
#define MFV_FVLD_ADDR1 &MFV_FVLD_ADDR1_REG
#define MFV_CONTROL &MFV_CONTROL_REG
#define MFV_GO_CYCLE &MFV_GO_CYCLE_REG
#define MFV_BREAKPOINT &MFV_BREAKPOINT_REG
#define MFV_IDMA_RST &MFV_IDMA_RST_REG
#define MFV_IDMA_STR &MFV_IDMA_STR_REG
#define MFV_IDMA_PAC_BASE &MFV_IDMA_PAC_BASE_REG
#define MFV_IDMA_GMC_BASE &MFV_IDMA_GMC_BASE_REG
#define MFV_IDMA_STATUS &MFV_IDMA_STATUS_REG
#define MFV_IDMA_CH_INFO &MFV_IDMA_CH_INFO_REG
#define MFV_IDMA_MEM_ADDR &MFV_IDMA_MEM_ADDR_REG
#define MFV_IDMA_GMC_STA &MFV_IDMA_GMC_STA_REG
#define MFV_IFME_KMB_ADDR0 &MFV_IFME_KMB_ADDR0_REG
#define MFV_IFME_KMB_ADDR1 &MFV_IFME_KMB_ADDR1_REG
#define MFV_IFME_KMB_NUM &MFV_IFME_KMB_NUM_REG
#define MFV_CUR_NMB_ADDR &MFV_CUR_NMB_ADDR_REG
#define MFV_CUR_NMB_NUM &MFV_CUR_NMB_NUM_REG
#define MFV_CONFIG &MFV_CONFIG_REG
#define MFV_HDDEC_SHARE &MFV_HDDEC_SHARE_REG
#define MFV_VERSION &MFV_VERSION_REG
#define MFV_RISC_DEBUG_0 &MFV_RISC_DEBUG_0_REG
#define MFV_RISC_DEBUG_1 &MFV_RISC_DEBUG_1_REG
#define MFV_RISC_DEBUG_2 &MFV_RISC_DEBUG_2_REG
#define MFV_RISC_DEBUG_3 &MFV_RISC_DEBUG_3_REG
#define MFV_RISC_DEBUG_4 &MFV_RISC_DEBUG_4_REG
#define MFV_RISC_DEBUG_5 &MFV_RISC_DEBUG_5_REG
#define MFV_RISC_DEBUG_6 &MFV_RISC_DEBUG_6_REG
#define MFV_RISC_DEBUG_7 &MFV_RISC_DEBUG_7_REG
#define MFV_RISC_DEBUG_8 &MFV_RISC_DEBUG_8_REG
#define MFV_RISC_DEBUG_9 &MFV_RISC_DEBUG_9_REG
#define MFV_RISC_DEBUG_10 &MFV_RISC_DEBUG_10_REG
#define MFV_RISC_DEBUG_11 &MFV_RISC_DEBUG_11_REG
#define MFV_PRED_DEBUG_0 &MFV_PRED_DEBUG_0_REG
#define MFV_PRED_DEBUG_1 &MFV_PRED_DEBUG_1_REG
#define MFV_PRED_DEBUG_2 &MFV_PRED_DEBUG_2_REG
#define MFV_PRED_DEBUG_3 &MFV_PRED_DEBUG_3_REG
#define MFV_POST_DEBUG_0 &MFV_POST_DEBUG_0_REG
#define MFV_POST_DEBUG_1 &MFV_POST_DEBUG_1_REG
#define MFV_POST_DEBUG_2 &MFV_POST_DEBUG_2_REG
#define MFV_POST_GSTA0 &MFV_POST_GSTA0_REG
#define MFV_POST_GSTA1 &MFV_POST_GSTA1_REG
#define MFV_RESI_DEBUG_0 &MFV_RESI_DEBUG_0_REG
#define MFV_RESI_DEBUG_1 &MFV_RESI_DEBUG_1_REG
#define MFV_FVLD_DEBUG_0 &MFV_FVLD_DEBUG_0_REG
#define MFV_FVLD_DEBUG_1 &MFV_FVLD_DEBUG_1_REG
#define MFV_FVLD_DEBUG_2 &MFV_FVLD_DEBUG_2_REG
#define MFV_FVLD_DEBUG_3 &MFV_FVLD_DEBUG_3_REG
#define MFV_DMEM &MFV_DMEM_REG

#else

#include "mfv_config.h"

#ifdef _6573_FPGA
#define MFV_REG_BASE                   0xC0180000
#else /* _6573_REAL_CHIP E1 address */
#ifdef _6573_LINUX
#include "mt6573_reg_base.h"
#define MFV_REG_BASE                   MFV_BASE
#else
#define MFV_REG_BASE                   0x700b3000
#endif
#endif

#define MFV_RESET                      (unsigned int)(MFV_REG_BASE+0x0)
#define MFV_START                      (unsigned int)(MFV_REG_BASE+0x4)
#define MFV_CLOCK_EN                   (unsigned int)(MFV_REG_BASE+0x8)
#define MFV_MEM_SWITCH                 (unsigned int)(MFV_REG_BASE+0xc)
#define MFV_IRQ_MASK                   (unsigned int)(MFV_REG_BASE+0x10)
#define MFV_IRQ_ACK                    (unsigned int)(MFV_REG_BASE+0x14)
#define MFV_IRQ_STS                    (unsigned int)(MFV_REG_BASE+0x18)
#define MFV_BS_LIMIT                   (unsigned int)(MFV_REG_BASE+0x20)
#define MFV_PIC_CONFIG                 (unsigned int)(MFV_REG_BASE+0x24)
#define MFV_ENC_CONFIG                 (unsigned int)(MFV_REG_BASE+0x28)
#define MFV_EIS_CONFIG                 (unsigned int)(MFV_REG_BASE+0x2c)
#define MFV_TVLD_ADDR                  (unsigned int)(MFV_REG_BASE+0x30)
#define MFV_SVLD_HEADER_ADDR           (unsigned int)(MFV_REG_BASE+0x34)
#define MFV_SVLD_BODY_ADDR             (unsigned int)(MFV_REG_BASE+0x38)
#define MFV_VLE_ADDR                   (unsigned int)(MFV_REG_BASE+0x3c)
#define MFV_REC_ADDR                   (unsigned int)(MFV_REG_BASE+0x40)
#define MFV_PFH_ADDR                   (unsigned int)(MFV_REG_BASE+0x44)
#define MFV_PFH_INFO_ADDR              (unsigned int)(MFV_REG_BASE+0x48)
#define MFV_REF_PTR_ADDR               (unsigned int)(MFV_REG_BASE+0x4c)
#define MFV_VOP_ADDR                   (unsigned int)(MFV_REG_BASE+0x50)
#define MFV_PFH_MB_NUM                 (unsigned int)(MFV_REG_BASE+0x54)
#define MFV_FVLD_ADDR                  (unsigned int)(MFV_REG_BASE+0x58)
#define MFV_FVLD_ADDR1                 (unsigned int)(MFV_REG_BASE+0x5c)
#define MFV_CONTROL                    (unsigned int)(MFV_REG_BASE+0x60)
#define MFV_GO_CYCLE                   (unsigned int)(MFV_REG_BASE+0x64)
#define MFV_BREAKPOINT                 (unsigned int)(MFV_REG_BASE+0x68)
#define MFV_IDMA_RST                   (unsigned int)(MFV_REG_BASE+0x80)
#define MFV_IDMA_STR                   (unsigned int)(MFV_REG_BASE+0x84)
#define MFV_IDMA_PAC_BASE              (unsigned int)(MFV_REG_BASE+0x88)
#define MFV_IDMA_GMC_BASE              (unsigned int)(MFV_REG_BASE+0x8c)
#define MFV_IDMA_STATUS                (unsigned int)(MFV_REG_BASE+0x90)
#define MFV_IDMA_CH_INFO               (unsigned int)(MFV_REG_BASE+0x94)
#define MFV_IDMA_MEM_ADDR              (unsigned int)(MFV_REG_BASE+0x98)
#define MFV_IDMA_GMC_STA               (unsigned int)(MFV_REG_BASE+0x9c)
#define MFV_IFME_KMB_ADDR0             (unsigned int)(MFV_REG_BASE+0xa0)
#define MFV_IFME_KMB_ADDR1             (unsigned int)(MFV_REG_BASE+0xa4)
#define MFV_IFME_KMB_NUM               (unsigned int)(MFV_REG_BASE+0xa8)
#define MFV_CUR_NMB_ADDR               (unsigned int)(MFV_REG_BASE+0xac)
#define MFV_CUR_NMB_NUM                (unsigned int)(MFV_REG_BASE+0xb0)
#define MFV_CONFIG                     (unsigned int)(MFV_REG_BASE+0xc0)
#define MFV_HDDEC_SHARE                (unsigned int)(MFV_REG_BASE+0xc4)
#define MFV_VERSION                    (unsigned int)(MFV_REG_BASE+0x100)
#define MFV_RISC_DEBUG_0               (unsigned int)(MFV_REG_BASE+0x104)
#define MFV_RISC_DEBUG_1               (unsigned int)(MFV_REG_BASE+0x108)
#define MFV_RISC_DEBUG_2               (unsigned int)(MFV_REG_BASE+0x10c)
#define MFV_RISC_DEBUG_3               (unsigned int)(MFV_REG_BASE+0x110)
#define MFV_RISC_DEBUG_4               (unsigned int)(MFV_REG_BASE+0x114)
#define MFV_RISC_DEBUG_5               (unsigned int)(MFV_REG_BASE+0x118)
#define MFV_RISC_DEBUG_6               (unsigned int)(MFV_REG_BASE+0x11c)
#define MFV_RISC_DEBUG_7               (unsigned int)(MFV_REG_BASE+0x120)
#define MFV_RISC_DEBUG_8               (unsigned int)(MFV_REG_BASE+0x124)
#define MFV_RISC_DEBUG_9               (unsigned int)(MFV_REG_BASE+0x128)
#define MFV_RISC_DEBUG_10              (unsigned int)(MFV_REG_BASE+0x12C)
#define MFV_RISC_DEBUG_11              (unsigned int)(MFV_REG_BASE+0x130)
#define MFV_PRED_DEBUG_0               (unsigned int)(MFV_REG_BASE+0x140)
#define MFV_PRED_DEBUG_1               (unsigned int)(MFV_REG_BASE+0x144)
#define MFV_PRED_DEBUG_2               (unsigned int)(MFV_REG_BASE+0x148)
#define MFV_PRED_DEBUG_3               (unsigned int)(MFV_REG_BASE+0x14c)
#define MFV_POST_DEBUG_0               (unsigned int)(MFV_REG_BASE+0x150)
#define MFV_POST_DEBUG_1               (unsigned int)(MFV_REG_BASE+0x154)
#define MFV_POST_DEBUG_2               (unsigned int)(MFV_REG_BASE+0x158)
#define MFV_POST_GSTA0                 (unsigned int)(MFV_REG_BASE+0x160)
#define MFV_POST_GSTA1                 (unsigned int)(MFV_REG_BASE+0x164)
#define MFV_RESI_DEBUG_0               (unsigned int)(MFV_REG_BASE+0x170)
#define MFV_RESI_DEBUG_1               (unsigned int)(MFV_REG_BASE+0x174)
#define MFV_FVLD_DEBUG_0               (unsigned int)(MFV_REG_BASE+0x190)
#define MFV_FVLD_DEBUG_1               (unsigned int)(MFV_REG_BASE+0x194)
#define MFV_FVLD_DEBUG_2               (unsigned int)(MFV_REG_BASE+0x198)
#define MFV_FVLD_DEBUG_3               (unsigned int)(MFV_REG_BASE+0x19C)
#define MFV_DMEM                       (unsigned int)&((unsigned int *)(MFV_REG_BASE+0x400))
#endif  // #ifdef MFV_REGC
#endif  // __mfv_reg_h__
