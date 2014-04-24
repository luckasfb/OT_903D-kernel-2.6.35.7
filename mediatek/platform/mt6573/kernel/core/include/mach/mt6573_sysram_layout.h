
#ifndef _MT6573_SYSRAM_LAYOUT_H
#define _MT6573_SYSRAM_LAYOUT_H

// Constrain
// MFlexVideo needs continous memory space larger than any single bank.
// use smallest bank if possible to save power.
// use less bank if possible to save power.
// ROTDMA occupies much bandwidth.

#define MM_SYSRAM_BASE_PA       (0x40000000)
#define Base_Bank0              (MM_SYSRAM_BASE_PA)
#define Size_Bank0              (0x8000)        //32KB
#define Base_Bank1              (Base_Bank0+Size_Bank0)
#define Size_Bank1              (0x8000)        //32KB
#define Base_Bank2              (Base_Bank1+Size_Bank1)
#define Size_Bank2              (0x10000)       //64KB
#define Base_Bank3              (Base_Bank2+Size_Bank2)
#define Size_Bank3              (0x8000)        //32KB
#define Base_Bank4              (Base_Bank3+Size_Bank3)
#define Size_Bank4              (0x18000)       //96KB
#define Base_Bank5              (Base_Bank4+Size_Bank4)
#define Size_Bank5              (0x4000)        //16KB
#define MM_SYSRAM_SIZE          (Size_Bank0+Size_Bank1+Size_Bank2+Size_Bank3+Size_Bank4+Size_Bank5)

typedef struct MemNode
{
    ESysramUser_T eOwner;
    unsigned long u4Offset;
    unsigned long u4Length;
    unsigned long u4Index;
    struct MemNode* pNext;
    struct MemNode* pPrev;
} MemNode_T;

//------------------------------------------------------------------------------
//  Memory Pool Bank
typedef enum {
    MEM_POOL_BANK012 = 0,           //  bank 0,1,2
    MEM_POOL_BANK34,                //  bank 3,4
    MEM_POOL_CNT,                   //  memory pool count.
    MEM_POOL_BAD     = 0x80000000,  //  bad memory pool
    MEM_POOL_STATIC,                //  static map.
} MEM_POOL_BANK_NO_T;

#define MT6573Bank012StartAddr  (Base_Bank0+4096)               //SPI DMA
#define MT6573Bank012Size       (Size_Bank0+Size_Bank1+Size_Bank2-4096)
                                                        //128KB-4KB //SPI DMA
#define MT6573Bank34StartAddr   (Base_Bank3)
#define MT6573Bank34Size        (Size_Bank3+Size_Bank4) //128KB

#define MEM_NODE_CNT_PER_POOL   (MT6573SYSRAMUSR_CNT*2 + 2)

//------------------------------------------------------------------------------
//  Memory Pool Info
typedef struct MemPoolInfo
{
    MemNode_T*const         paNodes;
    unsigned long const     u4OwnerCnt;
    unsigned long const     u4StartAddr;
    unsigned long const     u4Size;

    unsigned long           u4IndexTbl;
    unsigned long           u4UserCount;
} MemPoolInfo_T;

static MemNode_T g_aMemPoolBank012Tbl[MEM_NODE_CNT_PER_POOL];
static MemNode_T g_aMemPoolBank34Tbl [MEM_NODE_CNT_PER_POOL];

static MemPoolInfo_T g_aMemPoolInfo[MEM_POOL_CNT] = 
{
    [MEM_POOL_BANK012] = {
        .paNodes    = &g_aMemPoolBank012Tbl[0], 
        .u4OwnerCnt = MEM_NODE_CNT_PER_POOL, 
        .u4StartAddr= MT6573Bank012StartAddr, 
        .u4Size     = MT6573Bank012Size, 
        .u4IndexTbl = (~0x1), 
        .u4UserCount= 0, 
    }, 
    [MEM_POOL_BANK34]  = {
        .paNodes    = &g_aMemPoolBank34Tbl[0], 
        .u4OwnerCnt = MEM_NODE_CNT_PER_POOL, 
        .u4StartAddr= MT6573Bank34StartAddr, 
        .u4Size     = MT6573Bank34Size, 
        .u4IndexTbl = (~0x1), 
        .u4UserCount= 0, 
    }, 
};

static MemPoolInfo_T* GetMemPoolInfo(MEM_POOL_BANK_NO_T const eMemPoolBankNo)
{
    if  ( MEM_POOL_CNT > eMemPoolBankNo )
        return  &g_aMemPoolInfo[eMemPoolBankNo];
    return  NULL;
}

enum
{
    EMPoolBank012OwnersMask =
                         (1<<MT6573SYSRAMUSR_ROTDMA0)
                        |(1<<MT6573SYSRAMUSR_ROTDMA1)
                        |(1<<MT6573SYSRAMUSR_ROTDMA2)
                        |(1<<MT6573SYSRAMUSR_ROTDMA3)
                        |(1<<MT6573SYSRAMUSR_TVROT)
                        |(1<<MT6573SYSRAMUSR_TVC)
                        |(1<<MT6573SYSRAMUSR_RDMA0)
                        |(1<<MT6573SYSRAMUSR_RDMA1)
                        |(1<<MT6573SYSRAMUSR_CAM_SHADING_PREVIEW)
                        |(1<<MT6573SYSRAMUSR_CAM_SHADING_CAPTURE)
                        , 

    EMPoolBank34OwnersMask =
                         (1<<MT6573SYSRAMUSR_MFLEXVIDEO)
                        |(1<<MT6573SYSRAMUSR_BRZ)
                        |(1<<MT6573SYSRAMUSR_JPGDMA)
                        |(1<<MT6573SYSRAMUSR_JPGDECODER)
                        |(1<<MT6573SYSRAMUSR_CAM_FLICKER)
                        |(1<<MT6573SYSRAMUSR_CAM_DEFECT)
                        , 

    EStaticOwnersMask       =
                         (1<<MT6573SYSRAMUSR_CAM_PCA)
                        |(1<<MT6573SYSRAMUSR_EIS)
                        |(1<<MT6573SYSRAMUSR_CAM_LCE0)
                        |(1<<MT6573SYSRAMUSR_CAM_LCE1)
                        |(1<<MT6573SYSRAMUSR_VRZ)
                        |(1<<MT6573SYSRAMUSR_G2D_RMMU)
                        |(1<<MT6573SYSRAMUSR_SPI_DMA)
                        , 

    EDynamicOwnersMask      =
                        (~EStaticOwnersMask)
                        , 

    ELogOwnersMask          =
                         EMPoolBank34OwnersMask
                        |(1<<MT6573SYSRAMUSR_TVC)
                        |(1<<MT6573SYSRAMUSR_CAM_PCA)
                        |(1<<MT6573SYSRAMUSR_EIS)
                        |(1<<MT6573SYSRAMUSR_CAM_LCE0)
                        |(1<<MT6573SYSRAMUSR_CAM_LCE1)
                        |(1<<MT6573SYSRAMUSR_VRZ)
                        |(1<<MT6573SYSRAMUSR_G2D_RMMU)
                        , 
};

static MEM_POOL_BANK_NO_T GetMemPoolNo(ESysramUser_T const eOwner)
{
    unsigned long const u4OwnerMask = (1<<eOwner);
    if  ( u4OwnerMask & EStaticOwnersMask )
    {
        return  MEM_POOL_STATIC;
    }

    if  ( u4OwnerMask & EMPoolBank012OwnersMask )
    {
        return  MEM_POOL_BANK012;   //Bank 1+2
    }

    if  ( u4OwnerMask & EMPoolBank34OwnersMask )
    {
        return  MEM_POOL_BANK34;    //Bank3 + 4
    }

    return  MEM_POOL_BAD;
}


static char const*const g_apszOwnerName[MT6573SYSRAMUSR_CNT] = 
{
    [MT6573SYSRAMUSR_ROTDMA0            ] = "ROTDMA0", 
    [MT6573SYSRAMUSR_ROTDMA1            ] = "ROTDMA1", 
    [MT6573SYSRAMUSR_ROTDMA2            ] = "ROTDMA2", 
    [MT6573SYSRAMUSR_ROTDMA3            ] = "ROTDMA3", 
    [MT6573SYSRAMUSR_TVROT              ] = "TVROT", 
    [MT6573SYSRAMUSR_TVC                ] = "TVC", 
    [MT6573SYSRAMUSR_RDMA0              ] = "RDMA0", 
    [MT6573SYSRAMUSR_RDMA1              ] = "RDMA1", 
    [MT6573SYSRAMUSR_JPGDMA             ] = "JPGDMA", 
    [MT6573SYSRAMUSR_JPGDECODER         ] = "JPGDEC", 
    [MT6573SYSRAMUSR_MFLEXVIDEO         ] = "MFLEX Video", 
    [MT6573SYSRAMUSR_BRZ                ] = "BRZ", 
    [MT6573SYSRAMUSR_CAM_FLICKER        ] = "Flicker", 
    [MT6573SYSRAMUSR_CAM_DEFECT         ] = "Defect", 
    [MT6573SYSRAMUSR_CAM_SHADING_PREVIEW] = "Shading Pre", 
    [MT6573SYSRAMUSR_CAM_SHADING_CAPTURE] = "Shading Cap", 
    [MT6573SYSRAMUSR_CAM_LCE0           ] = "LCE0", 
    [MT6573SYSRAMUSR_CAM_LCE1           ] = "LCE1", 
    [MT6573SYSRAMUSR_CAM_PCA            ] = "PCA", 
    [MT6573SYSRAMUSR_EIS                ] = "EIS", 
    [MT6573SYSRAMUSR_VRZ                ] = "VRZ", 
    [MT6573SYSRAMUSR_G2D_RMMU           ] = "G2D RMMU", 
    [MT6573SYSRAMUSR_SPI_DMA            ] = "SPI DMA", 
};

static unsigned long const g_au4StaticUserAddr[MT6573SYSRAMUSR_CNT] = 
{
    [MT6573SYSRAMUSR_ROTDMA0            ] = 0, 
    [MT6573SYSRAMUSR_ROTDMA1            ] = 0, 
    [MT6573SYSRAMUSR_ROTDMA2            ] = 0, 
    [MT6573SYSRAMUSR_ROTDMA3            ] = 0, 
    [MT6573SYSRAMUSR_TVROT              ] = 0, 
    [MT6573SYSRAMUSR_TVC                ] = 0, 
    [MT6573SYSRAMUSR_RDMA0              ] = 0, 
    [MT6573SYSRAMUSR_RDMA1              ] = 0, 
    [MT6573SYSRAMUSR_JPGDMA             ] = 0, 
    [MT6573SYSRAMUSR_JPGDECODER         ] = 0, 
    [MT6573SYSRAMUSR_MFLEXVIDEO         ] = 0, 
    [MT6573SYSRAMUSR_BRZ                ] = 0, 
    [MT6573SYSRAMUSR_CAM_FLICKER        ] = 0, 
    [MT6573SYSRAMUSR_CAM_DEFECT         ] = 0, 
    [MT6573SYSRAMUSR_CAM_SHADING_PREVIEW] = 0, 
    [MT6573SYSRAMUSR_CAM_SHADING_CAPTURE] = 0, 
    [MT6573SYSRAMUSR_CAM_LCE0           ] = (Base_Bank5 + 0x1000), 
    [MT6573SYSRAMUSR_CAM_LCE1           ] = (Base_Bank5 + 0x1C00), 
    [MT6573SYSRAMUSR_CAM_PCA            ] = (Base_Bank5 + 0x0C00), 
    [MT6573SYSRAMUSR_EIS                ] = (Base_Bank5 + 0x0B00), 
    [MT6573SYSRAMUSR_VRZ                ] = (Base_Bank5 + 0x0000), 
    [MT6573SYSRAMUSR_G2D_RMMU           ] = (Base_Bank5 + 0x2800), 
    [MT6573SYSRAMUSR_SPI_DMA            ] = (Base_Bank0), 
};

static unsigned long const g_au4MaxUserSize[MT6573SYSRAMUSR_CNT] = 
{
    [MT6573SYSRAMUSR_ROTDMA0            ] = (3 + 63998 ) / 4 * 4, 
    [MT6573SYSRAMUSR_ROTDMA1            ] = (3 + 4096/*63998*/ ) / 4 * 4, 
    [MT6573SYSRAMUSR_ROTDMA2            ] = (3 + 4096  ) / 4 * 4, 
    [MT6573SYSRAMUSR_ROTDMA3            ] = (3 + 0     ) / 4 * 4, 
    [MT6573SYSRAMUSR_TVROT              ] = (3 + 25600 ) / 4 * 4, 
    [MT6573SYSRAMUSR_TVC                ] = (3 + 24064/*23096*/ ) / 4 * 4, 
    [MT6573SYSRAMUSR_RDMA0              ] = (3 + 22528 ) / 4 * 4, 
    [MT6573SYSRAMUSR_RDMA1              ] = (3 + 22528 ) / 4 * 4, 
    [MT6573SYSRAMUSR_JPGDMA             ] = (3 + 112640) / 4 * 4, 
    [MT6573SYSRAMUSR_JPGDECODER         ] = (3 + 4096  ) / 4 * 4, 
    [MT6573SYSRAMUSR_MFLEXVIDEO         ] = (3 + 131072) / 4 * 4, 
    [MT6573SYSRAMUSR_BRZ                ] = (3 + 82944 ) / 4 * 4, 
    [MT6573SYSRAMUSR_CAM_FLICKER        ] = (3 + 24576 ) / 4 * 4, 
    [MT6573SYSRAMUSR_CAM_DEFECT         ] = (3 + 4096  ) / 4 * 4, 
    [MT6573SYSRAMUSR_CAM_SHADING_PREVIEW] = (3 + 19200 ) / 4 * 4, 
    [MT6573SYSRAMUSR_CAM_SHADING_CAPTURE] = (3 + 16384 ) / 4 * 4, 
    [MT6573SYSRAMUSR_CAM_LCE0           ] = (3 + 3072  ) / 4 * 4, 
    [MT6573SYSRAMUSR_CAM_LCE1           ] = (3 + 3072  ) / 4 * 4, 
    [MT6573SYSRAMUSR_CAM_PCA            ] = (3 + 720   ) / 4 * 4, 
    [MT6573SYSRAMUSR_EIS                ] = (3 + 184   ) / 4 * 4, 
    [MT6573SYSRAMUSR_VRZ                ] = (3 + 2816  ) / 4 * 4, 
    [MT6573SYSRAMUSR_G2D_RMMU           ] = (3 + 4096  ) / 4 * 4, 
    [MT6573SYSRAMUSR_SPI_DMA            ] = (3 + 4096  ) / 4 * 4, 
};

typedef unsigned long const (*P_MaxUsrSizeTbl_T)[MT6573SYSRAMUSR_CNT];


#else   //_MT6573_SYSRAM_LAYOUT_H
#endif  //_MT6573_SYSRAM_LAYOUT_H
