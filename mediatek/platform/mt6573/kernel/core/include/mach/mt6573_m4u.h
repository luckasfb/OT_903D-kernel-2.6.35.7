
#ifndef _MT6573_M4U_KERNEL_H
#define _MT6573_M4U_KERNEL_H

#include <linux/ioctl.h>

#ifdef MTK_M4U_SUPPORT
  #define MTK_TVOUT_ENABLE_M4U
  #define MTK_LCDC_ENABLE_M4U
  #define MTK_DEBUG_ENABLE_M4U
  // #define MTK_MDP_ENABLE_M4U
#endif


#define MODULE_WITH_INDEPENDENT_PORT_ID 31
#define M4U_CLIENT_MODULE_NUM M4U_CLNTMOD_MAX
#define TOTAL_MVA_RANGE 0x40000000                              //total virtual address range: 1GB

//----------------------------------------------------------------------
/// macros to handle M4u Page Table processing
#define M4U_MVA_MAX 0x3fffffff   // 1G 
#define M4U_PAGE_MASK 0xfff
#define M4U_PAGE_SIZE   0x1000 //4KB
#define DEFAULT_PAGE_SIZE   0x1000 //4KB
#define M4U_GET_PTE_OFST_TO_PT_SA(MVA)    (((MVA) >> 12) << 2)
#define M4U_PTE_MAX (M4U_GET_PTE_OFST_TO_PT_SA(TOTAL_MVA_RANGE-1))
#define mva_pteAddr(mva) ((unsigned int *)pPageTableVA_nonCache+((mva) >> 12))
#define mva_pageOffset(mva) ((mva)&0xfff)

#define ENABLE_KEENE_API

#define ACCESS_TYPE_TRANSLATION_FAULT  0
#define ACCESS_TYPE_64K_PAGE           1
#define ACCESS_TYPE_4K_PAGE            2
#define ACCESS_TYPE_4K_EXTEND_PAGE     3

#define PT_TOTAL_ENTRY_NUM    TOTAL_MVA_RANGE/M4U_PAGE_SIZE//total page table entries
//#define MODULE_MVA_RANGE      TOTAL_MVA_RANGE/M4U_CLIENT_MODULE_NUM     //the virtual address range per port
//#define PT_MODULE_ENTRY_NUM   MODULE_MVA_RANGE/DEFAULT_PAGE_SIZE            //number of page table entries for each port
//#define PT_MODULE_PA_SZ       PT_MODULE_ENTRY_NUM*4                      //the physical memory size of page table per port
#define MMU_TAG_HW_NUM        24
#define MASTER_PORT_NUM       31
#define RT_RANGE_NUM          4
#define SEQ_RANGE_NUM         4
#define TOTAL_RANGE_NUM       (RT_RANGE_NUM+SEQ_RANGE_NUM)


typedef enum
{
	M4U_PORT_DEFECT = 0,	   //0
	M4U_PORT_OVL_MSK,
	M4U_PORT_OVL_DCP,
	M4U_PORT_JPG_ENC,
	M4U_PORT_DPI,
	M4U_PORT_ROT_DMA0_OUT0,   //5
	M4U_PORT_ROT_DMA1_OUT0,
	M4U_PORT_ROT_DMA2_OUT0,
	M4U_PORT_ROT_DMA3_OUT0,
	M4U_PORT_TV_ROT_OUT0,
	M4U_PORT_TVC,			   //10
	M4U_PORT_CAM,
	M4U_PORT_JPG_DEC_FDVT,
	M4U_PORT_FDVT_OUT2,
	M4U_PORT_LCD_R,
	M4U_PORT_LCD_W,		   //15
	M4U_PORT_GCMQ,
	M4U_PORT_G2D_WR,
	M4U_PORT_G2D_RD,
	M4U_PORT_R_DMA0_YUV,
	M4U_PORT_R_DMA1_YUV,	   //20
	M4U_PORT_FDVT_OUT1,
	M4U_PORT_DPI_HWC,
	M4U_PORT_GIF1,
	M4U_PORT_GIF2,
	M4U_PORT_GIF3, 		   //25
	M4U_PORT_PNG1,
	M4U_PORT_PNG2,
	M4U_PORT_PNG3,
	M4U_PORT_VRZ,
	M4U_PORT_PCA,			   //30
	M4U_PORT_JPGDMA_R, 	   //31 - 31
	M4U_PORT_JPGDMA_W,
	M4U_PORT_ROT_DMA0_OUT1,
	M4U_PORT_ROT_DMA0_OUT2,
	M4U_PORT_ROT_DMA1_OUT1,   //35 - 31
	M4U_PORT_ROT_DMA1_OUT2,
	M4U_PORT_ROT_DMA2_OUT1_2,
	M4U_PORT_ROT_DMA3_OUT1_2,
	M4U_PORT_TV_ROT_OUT1_2,
	M4U_PORT_GREQ_BLKW,	   //40 - 31	
	M4U_PORT_GREQ_BLKR,
	M4U_PORT_RDMA_G2D,
	M4U_PORT_JPG_DEC_RDMA_EIS,
	M4U_PORT_TVC_PFH,
	M4U_PORT_TVC_RESZ, 	   //45 - 31
	M4U_PORT_ROT_DMA_PT,
	M4U_PORT_TVROT_RDES_EIS,
	M4U_PORT_ROT_DMA_DES		//48 - 31
} M4U_PORT_ID_ENUM;

typedef enum
{
	M4U_CLNTMOD_DEFECT,  //0
	M4U_CLNTMOD_OVL,
	M4U_CLNTMOD_DPI,
	M4U_CLNTMOD_ROT0,
	M4U_CLNTMOD_ROT1,
	M4U_CLNTMOD_ROT2,  //5
	M4U_CLNTMOD_ROT3,
	M4U_CLNTMOD_TVROT,
	M4U_CLNTMOD_TVC,
	M4U_CLNTMOD_RDMA0,
	M4U_CLNTMOD_RDMA1, //10
	M4U_CLNTMOD_CAM,
	M4U_CLNTMOD_G2D,			
	M4U_CLNTMOD_JPEG_DEC,
	M4U_CLNTMOD_JPEG_ENC,
	M4U_CLNTMOD_LCDC,  //15
	M4U_CLNTMOD_PCA,
	M4U_CLNTMOD_GIF,
	M4U_CLNTMOD_PNG,
	M4U_CLNTMOD_LCDC_UI, 
	M4U_CLNTMOD_FD,   //20 
	M4U_CLNTMOD_FD_INPUT,
	M4U_CLNTMOD_RESERVED,
	M4U_CLNTMOD_MAX
} M4U_MODULE_ID_ENUM;

#define M4U_CLIENTMODULE_SZ_DEFECT     64 * 0x00100000
#define M4U_CLIENTMODULE_SZ_OVL        32 * 0x00100000
#define M4U_CLIENTMODULE_SZ_DPI        32 * 0x00100000
#define M4U_CLIENTMODULE_SZ_ROT0       32 * 0x00100000
#define M4U_CLIENTMODULE_SZ_ROT1       32 * 0x00100000
#define M4U_CLIENTMODULE_SZ_ROT2       32 * 0x00100000
#define M4U_CLIENTMODULE_SZ_ROT3       32 * 0x00100000
#define M4U_CLIENTMODULE_SZ_TVROT      8  * 0x00100000
#define M4U_CLIENTMODULE_SZ_TVC        8  * 0x00100000
#define M4U_CLIENTMODULE_SZ_RDMA0      64 * 0x00100000
#define M4U_CLIENTMODULE_SZ_RDMA1      64 * 0x00100000
#define M4U_CLIENTMODULE_SZ_CAM        32 * 0x00100000
#define M4U_CLIENTMODULE_SZ_G2D        16 * 0x00100000
#define M4U_CLIENTMODULE_SZ_JPEG_DEC   128* 0x00100000
#define M4U_CLIENTMODULE_SZ_JPEG_ENC   128* 0x00100000
#define M4U_CLIENTMODULE_SZ_LCDC       32 * 0x00100000
#define M4U_CLIENTMODULE_SZ_PCA        64 * 0x00100000
#define M4U_CLIENTMODULE_SZ_PNG        8  * 0x00100000
#define M4U_CLIENTMODULE_SZ_GIF        8  * 0x00100000
#define M4U_CLIENTMODULE_SZ_LCDC_UI    16 * 0x00100000
#define M4U_CLIENTMODULE_SZ_FD         8  * 0x00100000
#define M4U_CLIENTMODULE_SZ_FD_INPUT   8  * 0x00100000

// total current: 848 MB

typedef struct _M4U_RANGE_DES  //sequential entry range
{
    unsigned int Enabled;
    M4U_MODULE_ID_ENUM eModuleID;
    unsigned int MVAStart;
    unsigned int MVAEnd;
} M4U_RANGE_DES_T;

typedef struct _M4U_MVA_SLOT
{
    unsigned int BaseAddr;      //slot MVA start address
    unsigned int Size;          //slot size
    unsigned int Offset;        //current offset of the slot
    unsigned int BufCnt;        //how many buffer has been allocated from this slot
} M4U_MVA_SLOT_T;

typedef enum
{
	M4U_DESC_MAIN_TLB=0,
	M4U_DESC_PRE_TLB_LSB,
	M4U_DESC_PRE_TLB_MSB
} M4U_DESC_TLB_SELECT_ENUM;


typedef enum
{
	RT_RANGE_HIGH_PRIORITY=0,
	SEQ_RANGE_LOW_PRIORITY=1
} M4U_RANGE_PRIORITY_ENUM;

typedef enum
{
	M4U_DMA_READ_WRITE = 0,
	M4U_DMA_READ = 1,
	M4U_DMA_WRITE = 2,
	M4U_DMA_NONE_OP = 3,

} M4U_DMA_DIR_ENUM;


// native logic
// port related: virtuality, security, distance
typedef struct _M4U_PORT
{  
	M4U_PORT_ID_ENUM ePortID;		   //hardware port ID, defined in M4U_PORT_ID_ENUM
	unsigned int Virtuality;						   
	unsigned int Security;
	unsigned int Distance;
	unsigned int Direction;
}M4U_PORT_STRUCT;


// module related:  alloc/dealloc MVA buffer
typedef struct _M4U_MOUDLE
{
	// MVA alloc / dealloc
	M4U_MODULE_ID_ENUM eModuleID;	// module ID used inside M4U driver, defined in M4U_MODULE_ID_ENUM
	unsigned int BufAddr;				// buffer virtual address
	unsigned int BufSize;				// buffer size in byte

	// TLB range invalidate or set uni-upadte range
	unsigned int MVAStart;						 // MVA start address
	unsigned int MVAEnd;							 // MVA end address
	M4U_RANGE_PRIORITY_ENUM ePriority;						 // range priority, 0:high, 1:normal

    // manually insert page entry
	unsigned int EntryMVA;						 // manual insert entry MVA
	unsigned int Lock;							 // manual insert lock or not
}M4U_MOUDLE_STRUCT;

//------------------------------------------------
// RMMU Related Start
//------------------------------------------------
#define RMMU_ROT_G2D_BASE_PA  0x70084000
#define RMMU_ROT_DMA0_BASE_PA 0x70088000
#define RMMU_ROT_DMA1_BASE_PA 0x70089000
#define RMMU_ROT_DMA2_BASE_PA 0x7008A000
#define RMMU_ROT_DMA3_BASE_PA 0x7008B000
#define RMMU_ROT_TV_BASE_PA   0x7009F000

typedef enum
{
    RMMU_PORT_G2D_W,
    RMMU_PORT_ROTDMA0,
    RMMU_PORT_ROTDMA1,
    RMMU_PORT_ROTDMA2,
    RMMU_PORT_ROTDMA3,
    RMMU_PORT_ROTDMA_TV,
    RMMU_PORT_MAX
}RMMU_PORT_ENUM;

typedef enum
{
    M2D_ROTATE_HFLIP_90  = 0,
    M2D_ROTATE_90        = 1,
    M2D_ROTATE_270       = 2,
    M2D_ROTATE_HFLIP_270 = 3,
    M2D_ROTATE_180       = 4,
    M2D_ROTATE_HFLIP_0   = 5,
    M2D_ROTATE_HFLIP_180 = 6,
    M2D_ROTATE_0         = 7,

    M2D_ROTATE_FORCE_DWORD = 0xFFFFFFFF,
} M2D_ROTATE;

typedef enum
{
	MDP_ROTATE_0 = 0,
	MDP_ROTATE_90,
	MDP_ROTATE_180,
	MDP_ROTATE_270
} MDP_ROTATE;

typedef struct _RMMU_PAGE_TABLE
{
    RMMU_PORT_ENUM PortID;
    unsigned int BufVirAddr;
    unsigned int BufLength; 
    unsigned int PageTablePhyAddr;
} RMMU_PAGE_TABLE_STRUCT;

typedef struct _RMMU_CONFIG
{
    RMMU_PORT_ENUM port_id;
    bool enable;
    unsigned int intermem_addr;  // allocate from internal memory, used to prefetch page table
    unsigned int intermem_size;  // size=4K 
    unsigned int buf_vir_addr;
    unsigned int buf_size;       // width*height*bpp  
    union
    {
        M2D_ROTATE m2d_rot_angle;
        MDP_ROTATE mdp_rot_angle;
    };
    
    //private, config inside rmmu_config()
    bool _rmmu_fwd_dir;          // 0: engine write address is from low to high, 1: engine write address is from high to low
    bool _rmmu_wrap;			         // enable when internal working memory is smaller than virtual memory table
    unsigned int _rmmu_virt_base_addr;	 // dependent on rmmu_wrap
    unsigned int _rmmu_pageTb_addr; 	 // dependent on rmmu_wrap
    unsigned int _rmmu_pageTb_size;
} RMMU_CONFIG_STRUCT;
//------------------------------------------------
// RMMU Related End
//------------------------------------------------

typedef enum
{
    M4U_CACHE_FLUSH_BEFORE_HW_READ_MEM = 0,  // optimized, recommand to use
    M4U_CACHE_FLUSH_BEFORE_HW_WRITE_MEM = 1, // optimized, recommand to use
    M4U_CACHE_CLEAN_BEFORE_HW_READ_MEM = 2,
    M4U_CACHE_INVALID_AFTER_HW_WRITE_MEM = 3,
    M4U_NONE_OP = 4,
} M4U_CACHE_SYNC_ENUM;

typedef struct _M4U_CACHE
{
    // MVA alloc / dealloc
    M4U_MODULE_ID_ENUM eModuleID;             // module ID used inside M4U driver, defined in M4U_MODULE_ID_ENUM
    M4U_CACHE_SYNC_ENUM eCacheSync;
    unsigned int BufAddr;                  // buffer virtual address
    unsigned int BufSize;                     // buffer size in byte
}M4U_CACHE_STRUCT;

//IOCTL commnad
#define MT6573_M4UMAGICNO 'g'
#define MT6573M4U_T_POWER_ON               _IOW(MT6573_M4UMAGICNO, 0, int)
#define MT6573M4U_T_POWER_OFF              _IOW(MT6573_M4UMAGICNO, 1, int)
#define MT6573M4U_T_DUMP_REG               _IOW(MT6573_M4UMAGICNO, 2, int)
#define MT6573M4U_T_DUMP_INFO              _IOW(MT6573_M4UMAGICNO, 3, int)
#define MT6573M4U_T_ALLOC_MVA              _IOWR(MT6573_M4UMAGICNO, 4, int)
#define MT6573M4U_T_DEALLOC_MVA            _IOW(MT6573_M4UMAGICNO, 5, int)
#define MT6573M4U_T_INSERT_TLB_RANGE       _IOW(MT6573_M4UMAGICNO, 6, int)
#define MT6573M4U_T_INVALID_TLB_RANGE      _IOW(MT6573_M4UMAGICNO, 7, int)
#define MT6573M4U_T_INVALID_TLB_ALL        _IOW(MT6573_M4UMAGICNO, 8, int)
#define MT6573M4U_T_MANUAL_INSERT_ENTRY    _IOW(MT6573_M4UMAGICNO, 9, int)
#define MT6573M4U_T_CACHE_SYNC             _IOW(MT6573_M4UMAGICNO, 10, int)
#define MT6573M4U_T_RESET_MVA_RELEASE_TLB  _IOW(MT6573_M4UMAGICNO, 11, int)
#define MT6573M4U_T_POWER_ON_WITH_ID       _IOW(MT6573_M4UMAGICNO, 12, int)
#define MT6573M4U_T_POWER_OFF_WITH_ID      _IOW(MT6573_M4UMAGICNO, 13, int)
#define MT6573M4U_T_REG_RESTORE            _IOW(MT6573_M4UMAGICNO, 14, int)
// RMMU
#define MT6573RMMU_T_FILL_PAGE_TABLE _IOWR(MT6573_M4UMAGICNO, 30, int)
#define MT6573RMMU_T_RELEASE_PAGE_TABLE _IOW(MT6573_M4UMAGICNO, 31, int)


// Function pointer for kernel user
// M4U is .ko module, can not export function to .o module
// So kernel user should export functin pointer, M4U will 
// fill those pointers as callback for kernel user
// TVR, TVC, LCDC, DPI
typedef struct
{
	bool isInit;
  int (*m4u_dump_reg)(void);
  int (*m4u_dump_info)(void);
  int (*m4u_power_on)(M4U_MODULE_ID_ENUM eModuleID);
  int (*m4u_power_off)(M4U_MODULE_ID_ENUM eModuleID);
  int (*m4u_alloc_mva)(M4U_MODULE_ID_ENUM eModuleID, 
                  const unsigned int BufAddr, 
                  const unsigned int BufSize, 
                  unsigned int *pRetMVABuf);

  int (*m4u_alloc_mva_lcd)(M4U_MODULE_ID_ENUM eModuleID, 
                  const unsigned int BufAddr, 
                  const unsigned int BufSize, 
                  unsigned int *pRetMVABuf);

  int (*m4u_dealloc_mva)(M4U_MODULE_ID_ENUM eModuleID, 
                  const unsigned int BufAddr, 
                  const unsigned int BufSize, 
									const unsigned int MVA);		
                  							
  int (*m4u_insert_tlb_range)(M4U_MODULE_ID_ENUM eModuleID, 
                  unsigned int MVAStart, 
                  const unsigned int MVAEnd, 
                  M4U_RANGE_PRIORITY_ENUM ePriority);
                        
  int (*m4u_invalid_tlb_range)(M4U_MODULE_ID_ENUM eModuleID, unsigned int MVAStart, unsigned int MVAEnd);                  
  int (*m4u_invalid_tlb_all)(void);  
  int (*m4u_manual_insert_entry)(M4U_MODULE_ID_ENUM eModuleID,
                  unsigned int EntryMVA, 
                  bool Lock); 
  
  int (*m4u_config_port)(M4U_PORT_STRUCT* pM4uPort); //native
  int (*m4u_monitor_start)(void);
  int (*m4u_monitor_stop)(void);  
#ifdef ENABLE_KEENE_API
  int (*m4u_dma_cache_maint)(M4U_MODULE_ID_ENUM eModuleID, const void *start, size_t size, int direction);
#endif
  void (*m4u_reg_restore)(void);
  int (*m4u_reset_mva_release_tlb)(M4U_MODULE_ID_ENUM eModuleID);
} M4U_EXPORT_FUNCTION_STRUCT;

typedef struct
{
	bool isInit;
  int (*m4u_perf_timer_on)(void);
  int (*m4u_perf_timer_off)(void);
  int (*m4u_log_on)(void);
  int (*m4u_log_off)(void);
} M4U_DEBUG_FUNCTION_STRUCT;

// for kernel direct call --------------------------------------------
int m4u_dump_reg(void);
int m4u_dump_info(void);
int m4u_dump_pagetable(M4U_MODULE_ID_ENUM eModuleID);

int m4u_power_on(void);
int m4u_power_off(void);
int m4u_power_on_with_id(M4U_MODULE_ID_ENUM eModuleID); // with module ID, can locate who call power on/off not in pairs
int m4u_power_off_with_id(M4U_MODULE_ID_ENUM eModuleID);
int m4u_alloc_mva(M4U_MODULE_ID_ENUM eModuleID, 
                  const unsigned int BufAddr, 
								  const unsigned int BufSize, 
								  unsigned int *pRetMVABuf);
// just for LCD UI layer
int m4u_alloc_mva_lcd(M4U_MODULE_ID_ENUM eModuleID, 
                  const unsigned int BufAddr, 
								  const unsigned int BufSize, 
								  unsigned int *pRetMVABuf);

int m4u_dealloc_mva(M4U_MODULE_ID_ENUM eModuleID, 
									const unsigned int BufAddr, 
									const unsigned int BufSize, 
									const unsigned int MVA);
																		
int m4u_insert_tlb_range(M4U_MODULE_ID_ENUM eModuleID, 
									unsigned int MVAStart, 
									const unsigned int MVAEnd, 
									M4U_RANGE_PRIORITY_ENUM ePriority);
                      
int m4u_invalid_tlb_range(M4U_MODULE_ID_ENUM eModuleID,
                          unsigned int MVAStart, 
                          unsigned int MVAEnd);
                          
int m4u_invalid_tlb_all(void);

int m4u_manual_insert_entry(M4U_MODULE_ID_ENUM eModuleID,
                  unsigned int EntryMVA, 
                  bool Lock); 

int m4u_config_port(M4U_PORT_STRUCT* pM4uPort); //native
int m4u_monitor_start(void);
int m4u_monitor_stop(void);
int rmmu_config(RMMU_CONFIG_STRUCT *pRmmuConfig);

#ifdef ENABLE_KEENE_API
int m4u_dma_cache_maint(M4U_MODULE_ID_ENUM eModuleID, 
                        const void *start, 
                        size_t size, 
                        int direction);
#endif

// internal use ---------------------------------------------------
bool m4u_struct_init(void);
bool m4u_hw_init(void);
bool m4u_hw_reinit(void);
                     
unsigned int m4u_get_descriptor(M4U_DESC_TLB_SELECT_ENUM tlbSelect, 
                             unsigned int tlbIndex);
                             
unsigned int m4u_get_pages(M4U_MODULE_ID_ENUM eModuleID,
                        unsigned int BufAddr, 
                        unsigned int BufSize, 
                        unsigned int* pPageTableAddr);
// just for LCD UI layer      
unsigned int m4u_get_pages_lcd(unsigned int BufAddr, 
                        unsigned int BufSize, 
                        unsigned int* pPageTableAddr);  

int m4u_release_pages(M4U_MODULE_ID_ENUM eModuleID,
                        unsigned int BufAddr, 
                        unsigned int BufSize,
                        unsigned int MVA);
int m4u_release_pages_by_pagetable(M4U_MODULE_ID_ENUM eModuleID, 
                                   unsigned int BufAddr, 
                                   unsigned int BufSize, 
                                   unsigned int MVA);

unsigned int m4u_user_v2p(unsigned int va);
M4U_DMA_DIR_ENUM m4u_get_dir_by_module(M4U_MODULE_ID_ENUM eModuleID);
M4U_DMA_DIR_ENUM m4u_get_dir_by_port(M4U_PORT_ID_ENUM portID);
M4U_MODULE_ID_ENUM m4u_get_module_by_MVA(unsigned int MVA);
void m4u_clear_intr(void);
void m4u_reg_backup(void);
void m4u_reg_restore(void);
void m4u_memory_usage(bool bPrintAll);
unsigned int m4u_make_vmalloc_uncacheable(unsigned int BufAddr, unsigned int BufSize);
int m4u_reset_mva_release_tlb(M4U_MODULE_ID_ENUM eModuleID); 
void m4u_print_active_port(void);
unsigned int m4u_get_write_mode_by_module(M4U_MODULE_ID_ENUM eModuleID);

int m4u_print_range_info(void);
void m4u_get_performance(unsigned long data);
int m4u_perf_timer_on(void);
int m4u_perf_timer_off(void);
int m4u_log_on(void);
int m4u_log_off(void);
void m4u_get_power_status(void);
#endif
