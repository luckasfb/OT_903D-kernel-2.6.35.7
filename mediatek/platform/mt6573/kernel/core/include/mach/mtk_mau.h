

#ifndef _MTK_MAU_H_
#define _MTK_MAU_H_



#define MTK_MAU_MAJOR_NUMBER 190

typedef enum {
    MAU1_MASK_RSV = 0,
    MAU1_MASK_DEFECT,
    MAU1_MASK_OVL_MSK,
    MAU1_MASK_OVL_DCP,
    MAU1_MASK_JPG_ENC,
    MAU1_MASK_DPI,
    MAU1_MASK_ROTDMA0_OUT0,
    MAU1_MASK_ROTDMA1_OUT0,
    MAU1_MASK_ROTDMA2_OUT0,
    MAU1_MASK_ROTDMA3_OUT0,
    MAU1_MASK_TVROT_OUT0,
    MAU1_MASK_TVC,
    MAU1_MASK_CAM,
    MAU1_MASK_JPG_DEC_FDVT,
    MAU1_MASK_FDVT_OUT2,
    MAU1_MASK_LCD_R,
    MAU1_MASK_LCD_W,
    MAU1_MASK_GCMQ,
    MAU1_MASK_G2D_WR,
    MAU1_MASK_G2D_RD,
    MAU1_MASK_RDMA0_YUV,
    MAU1_MASK_RDMA1_YUV,
    MAU1_MASK_FDVT_OUT1,
    MAU1_MASK_DPI_HWC,
    MAU1_MASK_GIF1,
    MAU1_MASK_GIF2,
    MAU1_MASK_GIF3,
    MAU1_MASK_PNG1,
    MAU1_MASK_PNG2,
    MAU1_MASK_PNG3,
    MAU1_MASK_VRZ,
    MAU1_MASK_PCA,
    MAU1_MASK_ALL,

    MAU2_MASK_SPI,
    MAU2_MASK_RISC,
    MAU2_MASK_DMA,
    MAU2_MASK_BS,
    MAU2_MASK_POST,
    MAU2_MASK_CDMA,
    MAU2_MASK_PRED,
    MAU2_MASK_RESI,
    MAU2_MASK_VLCSAD,
    MAU_MASK_ALL
}MTK_MAU_MASK;



typedef enum {
    MPU_H_MASK_JPGDMA_R,
    MPU_H_MASK_JPGDMA_W,
    MPU_H_MASK_ROTDMA0_OUT1,
    MPU_H_MASK_ROTDMA0_OUT2,
    MPU_H_MASK_ROTDMA1_OUT1,
    MPU_H_MASK_ROTDMA1_OUT2,
    MPU_H_MASK_ROTDMA2_OUT1_OUT2,
    MPU_H_MASK_ROTDMA3_OUT1_OUT2,
    MPU_H_MASK_TVROT_OUT1_OUT2,
    MPU_H_MASK_GREQ_BLKW,
    MPU_H_MASK_GREQ_BLKR,
    MPU_H_MASK_RDMA_G2D,
    MPU_H_MASK_JPG_DEC_RDMA_EIS,
    MPU_H_MASK_TVC_PFH,
    MPU_H_MASK_TVC_RESZ,
    MPU_H_MASK_ROT_DMA_PT,
    MPU_H_MASK_TVROT_RDES_EIS,
    MPU_H_MASK_ROT_DMA_DES,
    MPU_H_MASK_ALL,
}MTK_MPU_H_MASK;




typedef enum
{
    ENABLE_MAU_TAG,
    DISABLE_MAU_TAG,
    ENABLE_MPU_TAG_FOR_INT_MEM,
    ENABLE_MPU_TAG_FOR_EXT_MEM,
    DISABLE_MPU_TAG,
} MTK_MAU_MODE;

typedef enum
{
    MAU_ENTRY_0 = 0,
    MAU_ENTRY_1,
    MAU_ENTRY_2,
    MAU_ENTRY_ALL
} MTK_MAU_ENTRY;



typedef struct
{
    MTK_MAU_ENTRY EntryID;						    // Entry ID 0~2
    bool         Enable;
    unsigned int InvalidMasterGMC1 ;	// one bit represent one master, 0: allow, 1: not allow, usd by MAU and MPU
	unsigned int InvalidMasterGMC2;     // one bit represent one master, 0: allow, 1: not allow, only used by MPU
	unsigned int ReadEn; 					    // check read transaction, 1:enable, 0:disable
	unsigned int WriteEn;				      // check write transaction, 1:enable, 0:disable
	unsigned int StartAddr;					  // start address
	unsigned int EndAddr;					    // end address
} MTK_MAU_CONFIG;


int  MAU_Config(MTK_MAU_CONFIG* pMauConf);
void MAU_PrintStatus(char* buf, unsigned int buf_len, unsigned int* num);
void MAU_Mau1DumpReg(void);
void MAU_Mau2DumpReg(void);
void MAU_LogSwitch(bool enable);
void MAU_Mau2BackupReg(void);
void MAU_Mau2RestoreReg(void);

int  MAU_Mau2PowerOn(void);
int  MAU_Mau2PowerOff(void);




#endif

