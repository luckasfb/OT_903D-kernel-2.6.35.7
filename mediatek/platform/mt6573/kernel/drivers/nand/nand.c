


#include <linux/slab.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/dma-mapping.h>
#include <linux/jiffies.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/time.h>
#include <linux/semaphore.h>

#include <asm/io.h>
#include <asm/cacheflush.h>
#include <asm/uaccess.h>

#include <linux/miscdevice.h>

//#include <mach/mt6573.h>
// #include <mach/mtk_nand_device.h>
#include <mach/dma.h>
#include <mach/mt6573_devs.h>
#include <mach/mt6573_reg_base.h>
#include <mach/mt6573_typedefs.h>
/* Koshi for mt6573 porting */
#include <mach/mt6573_pll.h>
#include <mach/mt6573_nand.h>
/* Koshi for mt6573 porting */
#include "bmt.h"
// #include <mach/partition.h>

/* Added for TCM used */
#include <asm/tcm.h>
#include <asm/system.h>
#include "partition_define.h"
#include "nand_device_list.h"
#include "nand_customer.h"

#define VERSION  	"v2.0"
#define MODULE_NAME	"# MT6573 NAND #"
#define PROCNAME    "driver/nand"
#define PMT 1
#define _MTK_NAND_DUMMY_DRIVER_

#ifndef NAND_OTP_SUPPORT
#define NAND_OTP_SUPPORT 1
#endif

#if NAND_OTP_SUPPORT

#define SAMSUNG_OTP_SUPPORT     1
#define OTP_MAGIC_NUM           0x4E3AF28B
#define SAMSUNG_OTP_PAGE_NUM    0x20
//libin

//static const unsigned int Samsung_OTP_Page[SAMSUNG_OTP_PAGE_NUM] = {0x15, 0x16, 0x17, 0x18, 0x19, 0x1b};//libin

static struct mt6573_otp_config g_mt6573_otp_fuc;
// static spinlock_t g_OTPLock;
DECLARE_MUTEX(g_OTPLock);

#define OTP_MAGIC           'k'

/* NAND OTP IO control number */
#define OTP_GET_LENGTH 		_IOW(OTP_MAGIC, 1, int)
#define OTP_READ 	        _IOW(OTP_MAGIC, 2, int)
#define OTP_WRITE 			_IOW(OTP_MAGIC, 3, int)

#define FS_OTP_READ         0
#define FS_OTP_WRITE        1

/* NAND OTP Error codes */
#define OTP_SUCCESS                   0
#define OTP_ERROR_OVERSCOPE          -1
#define OTP_ERROR_TIMEOUT            -2
#define OTP_ERROR_BUSY               -3
#define OTP_ERROR_NOMEM              -4
#define OTP_ERROR_RESET              -5
#define OTP_INVALID_OP               -6

#endif

#if NAND_OTP_SUPPORT
struct mt6573_otp_config
{
	u32 (* OTPRead)         (u32 PageAddr, void *BufferPtr, void *SparePtr);
	u32 (* OTPWrite)        (u32 PageAddr, void *BufferPtr, void *SparePtr);
	u32 (* OTPQueryLength)  (u32 *Length);
};

struct otp_ctl
{
    unsigned int QLength;
    unsigned int Offset;
    unsigned int Length;
    char *BufferPtr;
    unsigned int status;
};
typedef struct
{
	unsigned char imei[8];
	unsigned char svn;
	unsigned char pad;
} IMEI_SVN;

typedef struct
{
	unsigned char imei[8];
	unsigned char imei_checksum[8];
	unsigned char res[16];
} OTP_LAYOUT;


typedef enum {
	E_READ = 0,
	E_WRITE
} E_RW;

#define IMEI_SVN_LEN (sizeof (IMEI_SVN))
#define OTP_LAYOUT_LEN (sizeof (OTP_LAYOUT))
#endif

//#define NFI_SET_REG32(reg, value)   (DRV_WriteReg32(reg, DRV_Reg32(reg) | (value))) 
//#define NFI_SET_REG16(reg, value)   (DRV_WriteReg16(reg, DRV_Reg16(reg) | (value)))
//#define NFI_CLN_REG32(reg, value)   (DRV_WriteReg32(reg, DRV_Reg32(reg) & (~(value))))
//#define NFI_CLN_REG16(reg, value)   (DRV_WriteReg16(reg, DRV_Reg16(reg) & (~(value))))

#define NFI_SET_REG32(reg, value) \
do {	\
	g_value = (DRV_Reg32(reg) | (value));\
	DRV_WriteReg32(reg, g_value); \
} while(0)

#define NFI_SET_REG16(reg, value) \
do {	\
	g_value = (DRV_Reg16(reg) | (value));\
	DRV_WriteReg16(reg, g_value); \
} while(0)

#define NFI_CLN_REG32(reg, value) \
do {	\
	g_value = (DRV_Reg32(reg) & (~(value)));\
	DRV_WriteReg32(reg, g_value); \
} while(0)

#define NFI_CLN_REG16(reg, value) \
do {	\
	g_value = (DRV_Reg16(reg) & (~(value)));\
	DRV_WriteReg16(reg, g_value); \
} while(0)


#define NFI_WAIT_STATE_DONE(state) do{;}while (__raw_readl(NFI_STA_REG32) & state)
#define NFI_WAIT_TO_READY()  do{;}while (!(__raw_readl(NFI_STA_REG32) & STA_BUSY2READY))


#define NAND_SECTOR_SIZE (512)
#define OOB_PER_SECTOR      (16)
#define OOB_AVAI_PER_SECTOR (8)

#ifndef PART_SIZE_BMTPOOL
#define BMT_POOL_SIZE       (80)
#else
#define BMT_POOL_SIZE (PART_SIZE_BMTPOOL)
#endif

#define PMT_POOL_SIZE	(2)
#ifdef NAND_PFM
static suseconds_t g_PFM_R = 0;
static suseconds_t g_PFM_W = 0;
static suseconds_t g_PFM_E = 0;
static u32 g_PFM_RNum = 0;
static u32 g_PFM_RD = 0;
static u32 g_PFM_WD = 0;
static struct timeval g_now;

#define PFM_BEGIN(time) \
do_gettimeofday(&g_now); \
(time) = g_now;

#define PFM_END_R(time, n) \
do_gettimeofday(&g_now); \
g_PFM_R += (g_now.tv_sec * 1000000 + g_now.tv_usec) - (time.tv_sec * 1000000 + time.tv_usec); \
g_PFM_RNum += 1; \
g_PFM_RD += n; \
MSG(PERFORMANCE, "%s - Read PFM: %lu, data: %d, ReadOOB: %d (%d, %d)\n", MODULE_NAME , g_PFM_R, g_PFM_RD, g_kCMD.pureReadOOB, g_kCMD.pureReadOOBNum, g_PFM_RNum);

#define PFM_END_W(time, n) \
do_gettimeofday(&g_now); \
g_PFM_W += (g_now.tv_sec * 1000000 + g_now.tv_usec) - (time.tv_sec * 1000000 + time.tv_usec); \
g_PFM_WD += n; \
MSG(PERFORMANCE, "%s - Write PFM: %lu, data: %d\n", MODULE_NAME, g_PFM_W, g_PFM_WD);

#define PFM_END_E(time) \
do_gettimeofday(&g_now); \
g_PFM_E += (g_now.tv_sec * 1000000 + g_now.tv_usec) - (time.tv_sec * 1000000 + time.tv_usec); \
MSG(PERFORMANCE, "%s - Erase PFM: %lu\n", MODULE_NAME, g_PFM_E);
#else
#define PFM_BEGIN(time)
#define PFM_END_R(time, n)
#define PFM_END_W(time, n)
#define PFM_END_E(time)
#endif

#define TIMEOUT_1   0x1fff
#define TIMEOUT_2   0x8ff
#define TIMEOUT_3   0xffff
#define TIMEOUT_4   0xffff//5000   //PIO

#define NFI_ISSUE_COMMAND(cmd, col_addr, row_addr, col_num, row_num) \
   do { \
      DRV_WriteReg(NFI_CMD_REG16,cmd);\
      while (DRV_Reg32(NFI_STA_REG32) & STA_CMD_STATE);\
      DRV_WriteReg32(NFI_COLADDR_REG32, col_addr);\
      DRV_WriteReg32(NFI_ROWADDR_REG32, row_addr);\
      DRV_WriteReg(NFI_ADDRNOB_REG16, col_num | (row_num<<ADDR_ROW_NOB_SHIFT));\
      while (DRV_Reg32(NFI_STA_REG32) & STA_ADDR_STATE);\
   }while(0);

//-------------------------------------------------------------------------------
static struct completion g_comp_AHB_Done;
static struct mt6573_CMD g_kCMD;
static u32 g_u4ChipVer;
bool g_bInitDone;
static int g_i4Interrupt;
static bool g_bcmdstatus;
static u32 g_value = 0;
static int g_page_size;

static u8 *local_buffer_16_align;       // 16 byte aligned buffer, for HW issue
static u8 local_buffer[4096+16];

extern void nand_release_device(struct mtd_info *mtd);
extern int nand_get_device(struct nand_chip *chip, struct mtd_info *mtd, int new_state);

static bmt_struct *g_bmt;
struct mt6573_nand_host *host;

extern struct mtd_partition g_pasStatic_Partition[] ;
extern int part_num;
#ifdef PMT
extern void part_init_pmt(struct mtd_info *mtd, u8 *buf);
extern struct mtd_partition g_exist_Partition[] ;
#endif
int manu_id;
int dev_id;

static u8 local_oob_buf[128];

#ifdef _MTK_NAND_DUMMY_DRIVER_
int dummy_driver_debug;
#endif

void nand_enable_clock(void)
{
    (void)hwEnableClock(MT65XX_PDN_PERI_NFI, "NAND");
}

void nand_disable_clock(void)
{
    (void)hwDisableClock(MT65XX_PDN_PERI_NFI, "NAND");
}

static struct nand_ecclayout nand_oob_16 = {
	.eccbytes = 8,
	.eccpos = {8, 9, 10, 11, 12, 13, 14, 15},
	.oobfree = {{1,6}, {0, 0}}
};

struct nand_ecclayout nand_oob_64 = {
	.eccbytes = 32,
	.eccpos = {32, 33, 34, 35, 36, 37, 38, 39,
        40, 41, 42, 43, 44, 45, 46, 47,
        48, 49, 50, 51, 52, 53, 54, 55,
        56, 57, 58, 59, 60, 61, 62, 63},
	.oobfree = {{1, 7}, {9, 7}, {17, 7}, {25, 6}, {0, 0}}
};

struct nand_ecclayout nand_oob_128 = {
	.eccbytes = 64,
	.eccpos = {
        64, 65, 66, 67, 68, 69, 70, 71, 
        72, 73, 74, 75, 76, 77, 78, 79,
        80, 81, 82, 83, 84, 85, 86, 86,
        88, 89, 90, 91, 92, 93, 94, 95,
        96, 97, 98, 99, 100, 101, 102, 103,
        104, 105, 106, 107, 108, 109, 110, 111,
        112, 113, 114, 115, 116, 117, 118, 119,
        120, 121, 122, 123, 124, 125, 126, 127},
	.oobfree = {{1, 7}, {9, 7}, {17, 7}, {25, 7}, {33, 7}, {41, 7}, {49, 7}, {57, 6}}
};

flashdev_info devinfo;

void dump_nfi(void)
{
#if __DEBUG_NAND
    printk(KERN_INFO "NFI_ACCCON: 0x%x\n",  DRV_Reg32(NFI_ACCCON_REG32));
    printk(KERN_INFO "NFI_PAGEFMT: 0x%x\n", DRV_Reg16(NFI_PAGEFMT_REG16));
    printk(KERN_INFO "NFI_CNFG: 0x%x\n", DRV_Reg16(NFI_CNFG_REG16));
    printk(KERN_INFO "NFI_CON: 0x%x\n", DRV_Reg16(NFI_CON_REG16));
    printk(KERN_INFO "NFI_STRDATA: 0x%x\n", DRV_Reg16(NFI_STRDATA_REG16));
    printk(KERN_INFO "NFI_ADDRCNTR: 0x%x\n", DRV_Reg16(NFI_ADDRCNTR_REG16));
    printk(KERN_INFO "NFI_FIFOSTA: 0x%x\n", DRV_Reg16(NFI_FIFOSTA_REG16));
    printk(KERN_INFO "NFI_ADDRNOB: 0x%x\n", DRV_Reg16(NFI_ADDRNOB_REG16));
    printk(KERN_INFO "NFI_FDM_0L: 0x%x\n", DRV_Reg32(NFI_FDM0L_REG32));
    printk(KERN_INFO "NFI_STA: 0x%x\n", DRV_Reg32(NFI_STA_REG32));
    printk(KERN_INFO "NFI_FDM_0M: 0x%x\n", DRV_Reg32(NFI_FDM0M_REG32));
    printk(KERN_INFO "NFI_IOCON: 0x%x\n", DRV_Reg16(NFI_IOCON_REG16));
    printk(KERN_INFO "NFI_BYTELEN: 0x%x\n", DRV_Reg16(NFI_BYTELEN_REG16));
    printk(KERN_INFO "NFI_COLADDR: 0x%x\n", DRV_Reg32(NFI_COLADDR_REG32));
    printk(KERN_INFO "NFI_ROWADDR: 0x%x\n", DRV_Reg32(NFI_ROWADDR_REG32));
    printk(KERN_INFO "ECC_ENCCNFG: 0x%x\n", DRV_Reg32(ECC_ENCCNFG_REG32));
    printk(KERN_INFO "ECC_ENCCON: 0x%x\n", DRV_Reg16(ECC_ENCCON_REG16));
    printk(KERN_INFO "ECC_DECCNFG: 0x%x\n", DRV_Reg32(ECC_DECCNFG_REG32));
    printk(KERN_INFO "ECC_DECCON: 0x%x\n", DRV_Reg16(ECC_DECCON_REG16));
    printk(KERN_INFO "NFI_CSEL: 0x%x\n", DRV_Reg16(NFI_CSEL_REG16));
    // printk(KERN_INFO "NFI clock register: 0x%x: %s\n", DRV_Reg32((volatile u32 *)0x00000000),
    //         (DRV_Reg32((volatile u32 *)0xF0039300) & (1 << 17)) ? "miss" : "OK");
#endif
}


bool get_device_info(u16 id, u32 ext_id, flashdev_info *pdevinfo)
{
	u32 index;
	for(index=0;gen_FlashTable[index].id!=0;index++)
	{
	    if(id==gen_FlashTable[index].id && ext_id == gen_FlashTable[index].ext_id)
	    {
	    	pdevinfo->id = gen_FlashTable[index].id;
	    	pdevinfo->ext_id = gen_FlashTable[index].ext_id;
			pdevinfo->blocksize = gen_FlashTable[index].blocksize;
			pdevinfo->addr_cycle = gen_FlashTable[index].addr_cycle;
			pdevinfo->iowidth = gen_FlashTable[index].iowidth;
			pdevinfo->timmingsetting = gen_FlashTable[index].timmingsetting;
			pdevinfo->advancedmode = gen_FlashTable[index].advancedmode;
			pdevinfo->pagesize = gen_FlashTable[index].pagesize;
	        pdevinfo->totalsize = gen_FlashTable[index].totalsize;
			memcpy(pdevinfo->devciename,gen_FlashTable[index].devciename,sizeof(pdevinfo->devciename));
            printk(KERN_INFO "Device found in MTK table, ID: %x\n", id);
	
	        goto find;		
		}
	}

    for (index = 0; cust_FlashTable[index].id != 0; index++)
    {
        if (id == cust_FlashTable[index].id)
        {
            pdevinfo->id = cust_FlashTable[index].id;
            pdevinfo->blocksize = cust_FlashTable[index].blocksize;
            pdevinfo->addr_cycle = cust_FlashTable[index].addr_cycle;
            pdevinfo->iowidth = cust_FlashTable[index].iowidth;
            pdevinfo->timmingsetting = cust_FlashTable[index].timmingsetting;
            pdevinfo->advancedmode = cust_FlashTable[index].advancedmode;
            pdevinfo->pagesize = cust_FlashTable[index].pagesize;
            pdevinfo->totalsize = cust_FlashTable[index].totalsize;
            memcpy(pdevinfo->devciename, cust_FlashTable[index].devciename, sizeof(pdevinfo->devciename));
            printk(KERN_INFO "Device found in customer table, ID: %x\n", id);

            goto find;
        }
    }


find:
	if(0==pdevinfo->id)
	{
        printk(KERN_INFO "Device not found, ID: %x\n", id);
	    return false;
	}
	else
	{
		return true;
	}
}
/* Modified for TCM used */
static __tcmfunc irqreturn_t mt6573_nand_irq_handler(int irqno, void *dev_id)
//static irqreturn_t mt6573_nand_irq_handler(int irqno, void *dev_id)
{
   u16 u16IntStatus = DRV_Reg16(NFI_INTR_REG16);
   	(void)irqno;
//	MSG(INIT, "mt6573_nand_irq_handler @ [%s]: %d\n", __FUNCTION__, __LINE__);
    if (u16IntStatus & (u16)INTR_AHB_DONE_EN)
	{
    	complete(&g_comp_AHB_Done);
    } 
    return IRQ_HANDLED;
}

static void ECC_Config(struct mt6573_nand_host_hw *hw)
{
	u32 u4ENCODESize;
	u32 u4DECODESize;

    DRV_WriteReg16(ECC_DECCON_REG16, DEC_DE);
    do{;}while (!DRV_Reg16(ECC_DECIDLE_REG16));
 
    DRV_WriteReg16(ECC_ENCCON_REG16, ENC_DE);
    do{;}while (!DRV_Reg32(ECC_ENCIDLE_REG32));

	/* setup FDM register base */
	DRV_WriteReg32(ECC_FDMADDR_REG32, NFI_FDM0L_REG32);

    /* Sector + FDM */
    u4ENCODESize = (hw->nand_sec_size + 8) << 3;
    /* Sector + FDM + YAFFS2 meta data bits */
	u4DECODESize = ((hw->nand_sec_size + 8) << 3) + 4 * 13; 

	/* configure ECC decoder && encoder*/
	DRV_WriteReg32(ECC_DECCNFG_REG32,
		ECC_CNFG_ECC4|DEC_CNFG_NFI|DEC_CNFG_EMPTY_EN|
		(u4DECODESize << DEC_CNFG_CODE_SHIFT));

	DRV_WriteReg32(ECC_ENCCNFG_REG32, 
		ECC_CNFG_ECC4|ENC_CNFG_NFI|
		(u4ENCODESize << ENC_CNFG_MSG_SHIFT));

#if USE_AHB_MODE
	NFI_SET_REG32(ECC_DECCNFG_REG32, DEC_CNFG_CORRECT);
#else
	NFI_SET_REG32(ECC_DECCNFG_REG32, DEC_CNFG_EL);
#endif
}

static void ECC_Decode_Start(void)
{
   	/* wait for device returning idle */
	while(!(DRV_Reg16(ECC_DECIDLE_REG16) & DEC_IDLE));
	DRV_WriteReg16(ECC_DECCON_REG16, DEC_EN);
}

static void ECC_Decode_End(void)
{
   /* wait for device returning idle */
	while(!(DRV_Reg16(ECC_DECIDLE_REG16) & DEC_IDLE));
	DRV_WriteReg16(ECC_DECCON_REG16, DEC_DE);
}

static void ECC_Encode_Start(void)
{
   /* wait for device returning idle */
	while(!(DRV_Reg32(ECC_ENCIDLE_REG32) & ENC_IDLE));
	DRV_WriteReg16(ECC_ENCCON_REG16, ENC_EN);
}

static void ECC_Encode_End(void)
{
   /* wait for device returning idle */
	while(!(DRV_Reg32(ECC_ENCIDLE_REG32) & ENC_IDLE));
	DRV_WriteReg16(ECC_ENCCON_REG16, ENC_DE);
}

static bool mt6573_nand_check_bch_error(
	struct mtd_info *mtd, u8* pDataBuf, u32 u4SecIndex, u32 u4PageAddr)
{
	bool bRet = true;
	u16 u2SectorDoneMask = 1 << u4SecIndex;
	u32 u4ErrorNumDebug, i, u4ErrNum;
	u32 timeout = 0xFFFF;
    u32 correct_count = 0;
	// int el;
#if !USE_AHB_MODE
	u32 au4ErrBitLoc[6];
	u32 u4ErrByteLoc, u4BitOffset;
	u32 u4ErrBitLoc1th, u4ErrBitLoc2nd;
#endif

	//4 // Wait for Decode Done
	while (0 == (u2SectorDoneMask & DRV_Reg16(ECC_DECDONE_REG16)))
    {       
		timeout--;
		if (0 == timeout)
        {
			return false;
		}
	}
#if (USE_AHB_MODE)
	u4ErrorNumDebug = DRV_Reg32(ECC_DECENUM_REG32);
	if (0 != (u4ErrorNumDebug & 0xFFFF))
    {
		for (i = 0; i <= u4SecIndex; ++i)
        {
			u4ErrNum = DRV_Reg32(ECC_DECENUM_REG32) >> (i << 2);
			u4ErrNum &= 0xF;
            correct_count += u4ErrNum;
            
			if (0xF == u4ErrNum)
            {
				mtd->ecc_stats.failed++;
				bRet = false;
				printk(KERN_ERR"UnCorrectable at PageAddr=%d, Sector=%d\n", u4PageAddr, i);
			} 
            else 
            {
                if (u4ErrNum)
                {
				    printk(KERN_ERR"Correct %d at PageAddr=%d, Sector=%d\n", u4ErrNum, u4PageAddr, i);
                    // int el = DRV_Reg32(ECC_DECEL0_REG32);
                    // printk(KERN_INFO "EL: %x , Data in buf: %x\n", el, pDataBuf[i * 512 + el >> 3]);
                }
			}
		}
        if ((correct_count > 2) && bRet)
        {
				mtd->ecc_stats.corrected++;
		}
        else
        {
            // printk(KERN_INFO "Less than 2 bit error, ignore\n");
		}
	}
#else
	memset(au4ErrBitLoc, 0x0, sizeof(au4ErrBitLoc));
	u4ErrorNumDebug = DRV_Reg32(ECC_DECENUM_REG32);
	u4ErrNum = DRV_Reg32(ECC_DECENUM_REG32) >> (u4SecIndex << 2);
	u4ErrNum &= 0xF;
    
	if (u4ErrNum)
    {
		if (0xF == u4ErrNum)
        {
			mtd->ecc_stats.failed++;
			bRet = false;
			//printk(KERN_ERR"UnCorrectable at PageAddr=%d\n", u4PageAddr);
		} 
        else 
        {
			for (i = 0; i < ((u4ErrNum+1)>>1); ++i)
            {
				au4ErrBitLoc[i] = DRV_Reg32(ECC_DECEL0_REG32 + i);
				u4ErrBitLoc1th = au4ErrBitLoc[i] & 0x1FFF;
                
				if (u4ErrBitLoc1th < 0x1000)
                {
					u4ErrByteLoc = u4ErrBitLoc1th/8;
					u4BitOffset = u4ErrBitLoc1th%8;
					pDataBuf[u4ErrByteLoc] = pDataBuf[u4ErrByteLoc]^(1<<u4BitOffset);
					mtd->ecc_stats.corrected++;
				} 
                else 
                {
					mtd->ecc_stats.failed++;
					//printk(KERN_ERR"UnCorrectable ErrLoc=%d\n", au4ErrBitLoc[i]);
				}
				u4ErrBitLoc2nd = (au4ErrBitLoc[i] >> 16) & 0x1FFF;
				if (0 != u4ErrBitLoc2nd)
                {
					if (u4ErrBitLoc2nd < 0x1000)
                    {
						u4ErrByteLoc = u4ErrBitLoc2nd/8;
						u4BitOffset = u4ErrBitLoc2nd%8;
						pDataBuf[u4ErrByteLoc] = pDataBuf[u4ErrByteLoc]^(1<<u4BitOffset);
						mtd->ecc_stats.corrected++;
					} 
                    else 
                    {
						mtd->ecc_stats.failed++;
						//printk(KERN_ERR"UnCorrectable High ErrLoc=%d\n", au4ErrBitLoc[i]);
					}
				}
			}
		}
		if (0 == (DRV_Reg16(ECC_DECFER_REG16) & (1 << u4SecIndex)))
        {
			bRet = false;
		}
	}
#endif
	return bRet;
}

static bool mt6573_nand_RFIFOValidSize(u16 u2Size)
{
	u32 timeout = 0xFFFF;
	while (FIFO_RD_REMAIN(DRV_Reg16(NFI_FIFOSTA_REG16)) < u2Size)
    {
		timeout--;
		if (0 == timeout){
			return false;
		}
	}
	return true;
}

static bool mt6573_nand_WFIFOValidSize(u16 u2Size)
{
	u32 timeout = 0xFFFF;
	while (FIFO_WR_REMAIN(DRV_Reg16(NFI_FIFOSTA_REG16)) > u2Size)
    {
		timeout--;
		if (0 == timeout)
        {
			return false;
		}
	}
	return true;
}

static bool mt6573_nand_status_ready(u32 u4Status)
{
	u32 timeout = 0xFFFF;
	while ((DRV_Reg32(NFI_STA_REG32) & u4Status) != 0)
    {
		timeout--;
		if (0 == timeout)
        {
			return false;
		}
	}
	return true;
}

static bool mt6573_nand_reset(void)
{
    // HW recommended reset flow
    int timeout = 0xFFFF;
    if (DRV_Reg16(NFI_MASTERSTA_REG16)) // master is busy
    {
        DRV_WriteReg16(NFI_CON_REG16, CON_FIFO_FLUSH | CON_NFI_RST);
        while (DRV_Reg16(NFI_MASTERSTA_REG16))
        {
            timeout--;
            if (!timeout)
            {
                MSG(INIT, "Wait for NFI_MASTERSTA timeout\n");
            }
        }
    }
	/* issue reset operation */
	DRV_WriteReg16(NFI_CON_REG16, CON_FIFO_FLUSH | CON_NFI_RST);

	return mt6573_nand_status_ready(STA_NFI_FSM_MASK|STA_NAND_BUSY) &&
		   mt6573_nand_RFIFOValidSize(0) &&
		   mt6573_nand_WFIFOValidSize(0);
}

static void mt6573_nand_set_mode(u16 u2OpMode)
{
	u16 u2Mode = DRV_Reg16(NFI_CNFG_REG16);
	u2Mode &= ~CNFG_OP_MODE_MASK;
	u2Mode |= u2OpMode;
	DRV_WriteReg16(NFI_CNFG_REG16, u2Mode);
}

static void mt6573_nand_set_autoformat(bool bEnable)
{
	if (bEnable)
    {
		NFI_SET_REG16(NFI_CNFG_REG16, CNFG_AUTO_FMT_EN);
	}
    else
    {
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AUTO_FMT_EN);
	}
}

static void mt6573_nand_configure_fdm(u16 u2FDMSize)
{
	NFI_CLN_REG16(NFI_PAGEFMT_REG16, PAGEFMT_FDM_MASK | PAGEFMT_FDM_ECC_MASK);
	NFI_SET_REG16(NFI_PAGEFMT_REG16, u2FDMSize << PAGEFMT_FDM_SHIFT);
	NFI_SET_REG16(NFI_PAGEFMT_REG16, u2FDMSize << PAGEFMT_FDM_ECC_SHIFT);
}

static void mt6573_nand_configure_lock(void)
{
	u32 u4WriteColNOB = 2;
	u32 u4WriteRowNOB = 3;
	u32 u4EraseColNOB = 0;
	u32 u4EraseRowNOB = 3;
	DRV_WriteReg16(NFI_LOCKANOB_REG16, 
		(u4WriteColNOB << PROG_CADD_NOB_SHIFT)  |
		(u4WriteRowNOB << PROG_RADD_NOB_SHIFT)  |
		(u4EraseColNOB << ERASE_CADD_NOB_SHIFT) |
		(u4EraseRowNOB << ERASE_RADD_NOB_SHIFT));

	if (CHIPVER_ECO_1 == g_u4ChipVer)
    {
		int i;
		for (i = 0; i < 16; ++i)
        {
			DRV_WriteReg32(NFI_LOCK00ADD_REG32 + (i << 1), 0xFFFFFFFF);
			DRV_WriteReg32(NFI_LOCK00FMT_REG32 + (i << 1), 0xFFFFFFFF);
		}
		//DRV_WriteReg16(NFI_LOCKANOB_REG16, 0x0);
		DRV_WriteReg32(NFI_LOCKCON_REG32, 0xFFFFFFFF);
		DRV_WriteReg16(NFI_LOCK_REG16, NFI_LOCK_ON);
	}	
}

static bool mt6573_nand_pio_ready(void)
{
    int count = 0;
    while ( !(DRV_Reg16(NFI_PIO_DIRDY_REG16) & 1) )
    {
        count++;
        if (count > 0xffff)
        {
            printk("PIO_DIRDY timeout\n");
            return false;
        }
    }

    return true;
}

static bool mt6573_nand_set_command(u16 command)
{
	/* Write command to device */
	DRV_WriteReg16(NFI_CMD_REG16, command);
	return mt6573_nand_status_ready(STA_CMD_STATE);
}

static bool mt6573_nand_set_address(u32 u4ColAddr, u32 u4RowAddr, u16 u2ColNOB, u16 u2RowNOB)
{
	/* fill cycle addr */
	DRV_WriteReg32(NFI_COLADDR_REG32, u4ColAddr);
	DRV_WriteReg32(NFI_ROWADDR_REG32, u4RowAddr);
	DRV_WriteReg16(NFI_ADDRNOB_REG16, u2ColNOB|(u2RowNOB << ADDR_ROW_NOB_SHIFT));
	return mt6573_nand_status_ready(STA_ADDR_STATE);
}

static bool mt6573_nand_check_RW_count(u16 u2WriteSize)
{
	u32 timeout = 0xFFFF;
	u16 u2SecNum = u2WriteSize >> 9;
    
	while (ADDRCNTR_CNTR(DRV_Reg16(NFI_ADDRCNTR_REG16)) < u2SecNum)
    {
		timeout--;
		if (0 == timeout)
        {
            printk(KERN_INFO "[%s] timeout\n", __FUNCTION__);
			return false;
		}
	}
	return true;
}

static bool mt6573_nand_ready_for_read(struct nand_chip *nand, u32 u4RowAddr, u32 u4ColAddr, bool full, u8 *buf)
{
	/* Reset NFI HW internal state machine and flush NFI in/out FIFO */	
	bool bRet = false;
	u16 sec_num = 1 << (nand->page_shift - 9);
    u32 col_addr = u4ColAddr;
	u32 colnob=2, rownob=devinfo.addr_cycle-2;	
    if (nand->options & NAND_BUSWIDTH_16)
        col_addr /= 2;

	if (!mt6573_nand_reset())
    {
		goto cleanup;
	}
	
    /* Enable HW ECC */
    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);

	mt6573_nand_set_mode(CNFG_OP_READ);
	NFI_SET_REG16(NFI_CNFG_REG16, CNFG_READ_EN);
	DRV_WriteReg16(NFI_CON_REG16, sec_num << CON_NFI_SEC_SHIFT);

    if (full)
    {
#if USE_AHB_MODE
	    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_AHB);
        DRV_WriteReg32(NFI_STRADDR_REG32, __virt_to_phys(buf));
#else
    	NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);
#endif
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    }
    else
    {
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);
    }

	mt6573_nand_set_autoformat(full);
    if (full)
	    ECC_Decode_Start();

	if (!mt6573_nand_set_command(NAND_CMD_READ0))
    {
		goto cleanup;
	}

	//1 FIXED ME: For Any Kind of AddrCycle
	if (!mt6573_nand_set_address(col_addr, u4RowAddr, colnob, rownob))
    {
		goto cleanup;
	}

	if (!mt6573_nand_set_command(NAND_CMD_READSTART))
    {
		goto cleanup;
	}

	if (!mt6573_nand_status_ready(STA_NAND_BUSY))
    {
		goto cleanup;
	}

	bRet = true;
	
cleanup:
	return bRet;
}

static bool mt6573_nand_ready_for_write(
	struct nand_chip *nand, u32 u4RowAddr, u32 col_addr, bool full, u8 *buf)
{
	bool bRet = false;
	u32 sec_num = 1 << (nand->page_shift - 9);
	u32 colnob=2, rownob=devinfo.addr_cycle-2;
	
    if (nand->options & NAND_BUSWIDTH_16)
        col_addr /= 2;


	/* Reset NFI HW internal state machine and flush NFI in/out FIFO */	
	if (!mt6573_nand_reset())
    {
		return false;
	}

	mt6573_nand_set_mode(CNFG_OP_PRGM);
	
	NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_READ_EN);
	
	DRV_WriteReg16(NFI_CON_REG16, sec_num << CON_NFI_SEC_SHIFT);

    if (full)
    {
#if USE_AHB_MODE
	NFI_SET_REG16(NFI_CNFG_REG16, CNFG_AHB);
    DRV_WriteReg32(NFI_STRADDR_REG32, __virt_to_phys(buf));
#else
	NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);
#endif

	NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    }
    else
    {
	    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
	    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);
    }

	mt6573_nand_set_autoformat(full);

    if (full)
	ECC_Encode_Start();

	if (!mt6573_nand_set_command(NAND_CMD_SEQIN)){
		goto cleanup;
	}

	//1 FIXED ME: For Any Kind of AddrCycle
	if (!mt6573_nand_set_address(col_addr, u4RowAddr, colnob, rownob)){
		goto cleanup;
	}

	if (!mt6573_nand_status_ready(STA_NAND_BUSY)){
		goto cleanup;
	}

	bRet = true;
cleanup:

	return bRet;
}

static bool mt6573_nand_check_dececc_done(u32 u4SecNum)
{
    u32 timeout, dec_mask;
    timeout = 0xffff;
    dec_mask = (1<<u4SecNum)-1;
    while((dec_mask != DRV_Reg(ECC_DECDONE_REG16)) && timeout>0)
        timeout--;
    if(timeout == 0){
        MSG(VERIFY, "ECC_DECDONE: timeout\n");
        return false;
    }
    return true;
}

static bool mt6573_nand_dma_read_data(struct mtd_info *mtd, u8 *buf, u32 length)
{
    int interrupt_en = g_i4Interrupt;
    int timeout = 0xffff;
    struct scatterlist sg;
    enum dma_data_direction dir = DMA_FROM_DEVICE;
    
    sg_init_one(&sg, buf, length);
	dma_map_sg(&(mtd->dev), &sg, 1, dir);

	NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);
	// DRV_WriteReg32(NFI_STRADDR_REG32, __virt_to_phys(pDataBuf));
 
    if ((unsigned int)buf % 16)		// TODO: can not use AHB mode here
    {
        printk(KERN_INFO "Un-16-aligned address\n");
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_DMA_BURST_EN);
    }
    else
    {
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_DMA_BURST_EN);
    }

    DRV_Reg16(NFI_INTR_REG16);
    DRV_WriteReg16(NFI_INTR_EN_REG16, INTR_AHB_DONE_EN);

    if (interrupt_en) 
    {
	    init_completion(&g_comp_AHB_Done);
	}
	//dmac_inv_range(pDataBuf, pDataBuf + u4Size);
	
	NFI_SET_REG16(NFI_CON_REG16, CON_NFI_BRD);
   
	if (interrupt_en) 
    {
	    if (!wait_for_completion_timeout(&g_comp_AHB_Done, 2))
        {
    //        MSG(INIT, "wait for completion timeout happened @ [%s]: %d\n", __FUNCTION__, __LINE__);
            dump_nfi();
            return false;
        }
        while ( (length >> 9) > ((DRV_Reg16(NFI_BYTELEN_REG16) & 0xf000) >> 12) )
        {
		    timeout--;
		    if (0 == timeout)
            {
                printk(KERN_ERR "[%s] poll BYTELEN error\n", __FUNCTION__);
			    return false; //4  // AHB Mode Time Out!
			}
		}	
    } 
    else 
    {
        while (!DRV_Reg16(NFI_INTR_REG16))
        {
		    timeout--;
		    if (0 == timeout)
            {
                printk(KERN_ERR "[%s] poll nfi_intr error\n", __FUNCTION__);
                dump_nfi();
			    return false; //4  // AHB Mode Time Out!
            }
        }
        while ( (length >> 9) > ((DRV_Reg16(NFI_BYTELEN_REG16) & 0xf000) >> 12) )
        {
		    timeout--;
		    if (0 == timeout)
            {
                printk(KERN_ERR "[%s] poll BYTELEN error\n", __FUNCTION__);
                dump_nfi();
			    return false; //4  // AHB Mode Time Out!
			}
		}
	}
	dma_unmap_sg(&(mtd->dev), &sg, 1, dir);
	return true;
}

static bool mt6573_nand_mcu_read_data(u8 *buf, u32 length)
{
    int timeout = 0xffff;
	u32 i;
	u32* buf32 = (u32 *)buf;
#ifdef TESTTIME		
	unsigned long long time1,time2;
    time1 = sched_clock();
#endif
    if ((u32)buf % 4 || length % 4)
	    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);
    else
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);

    DRV_WriteReg32(NFI_STRADDR_REG32, 0);
	NFI_SET_REG16(NFI_CON_REG16, CON_NFI_BRD);
	
    if ((u32)buf % 4 || length % 4)
    {
        for (i = 0; (i < (length))&&(timeout > 0);)
        {
    		if (FIFO_RD_REMAIN(DRV_Reg16(NFI_FIFOSTA_REG16)) >= 4)
            {
    			*buf++ = (u8)DRV_Reg32(NFI_DATAR_REG32);
    			i++;
    		} 
            else 
            {
    			timeout--;
    		}
    		if (0 == timeout)
            {
                printk(KERN_ERR "[%s] timeout\n", __FUNCTION__);
                dump_nfi();
    			return false;
    		}
    	}
    }
    else
    {
        for (i = 0; (i < (length >> 2))&&(timeout > 0);)
        {
    		if (FIFO_RD_REMAIN(DRV_Reg16(NFI_FIFOSTA_REG16)) >= 4)
            {
    			*buf32++ = DRV_Reg32(NFI_DATAR_REG32);
    			i++;
    		} 
            else 
            {
    			timeout--;
    		}
    		if (0 == timeout)
            {
                printk(KERN_ERR "[%s] timeout\n", __FUNCTION__);
                dump_nfi();
    			return false;
    		}
    	}
     }
#ifdef TESTTIME		
	time2 = sched_clock()-time1;
	if(!readdatatime)
	{
	   readdatatime=(time2);
	}
#endif
    return true;
}

static bool mt6573_nand_read_page_data(struct mtd_info *mtd, u8* pDataBuf, u32 u4Size)
{
#if (USE_AHB_MODE)
    return mt6573_nand_dma_read_data(mtd, pDataBuf, u4Size);
#else
    return mt6573_nand_mcu_read_data(mtd, pDataBuf, u4Size);
#endif
}	

static bool mt6573_nand_dma_write_data(struct mtd_info *mtd, u8 *pDataBuf, u32 u4Size)
{
	int i4Interrupt = 0;	//g_i4Interrupt;
	u32 timeout = 0xFFFF;
	struct scatterlist sg;
	enum dma_data_direction dir = DMA_TO_DEVICE;

    sg_init_one(&sg, pDataBuf, u4Size);
	dma_map_sg(&(mtd->dev), &sg, 1, dir);
	
	NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);
	DRV_Reg16(NFI_INTR_REG16);
	DRV_WriteReg16(NFI_INTR_EN_REG16, 0);
	// DRV_WriteReg32(NFI_STRADDR_REG32, (u32*)virt_to_phys(pDataBuf));
    
    if ((unsigned int)pDataBuf % 16)		// TODO: can not use AHB mode here
    {
        printk(KERN_INFO "Un-16-aligned address\n");
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_DMA_BURST_EN);
    }
    else
    {
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_DMA_BURST_EN);
    }
//	MSG(INIT, "mt6573_nand_dma_write_data @ [%s]: %d\n", __FUNCTION__, __LINE__);
	if (i4Interrupt) 
    {
		init_completion(&g_comp_AHB_Done);
		DRV_Reg16(NFI_INTR_REG16);
		DRV_WriteReg16(NFI_INTR_EN_REG16, INTR_AHB_DONE_EN);
	}    
	//dmac_clean_range(pDataBuf, pDataBuf + u4Size);
	NFI_SET_REG16(NFI_CON_REG16, CON_NFI_BWR);
    
	if (i4Interrupt) 
    {
	    if (!wait_for_completion_timeout(&g_comp_AHB_Done, 2))
        {
            MSG(READ, "wait for completion timeout happened @ [%s]: %d\n", __FUNCTION__, __LINE__);
            dump_nfi();
            return false;
        }
		// wait_for_completion(&g_comp_AHB_Done);
	} 
    else 
    {
        while ( (u4Size >> 9) > ((DRV_Reg16(NFI_BYTELEN_REG16) & 0xf000) >> 12) )
        {
			timeout--;
			if (0 == timeout)
            {
                printk(KERN_ERR "[%s] poll BYTELEN error\n", __FUNCTION__);
				return false; //4  // AHB Mode Time Out!
			}
		}
	}	
	
	dma_unmap_sg(&(mtd->dev), &sg, 1, dir);

    return true;
}

static bool mt6573_nand_mcu_write_data(struct mtd_info *mtd, const u8 *buf, u32 length)
{
	u32 timeout = 0xFFFF;
	u32 i;	
	u32* pBuf32;
	NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);
	NFI_SET_REG16(NFI_CON_REG16, CON_NFI_BWR);
	pBuf32 = (u32*)buf;
    
    if ((u32)buf % 4 || length % 4)
	    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);
    else
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);

    if ((u32)buf % 4 || length % 4)
    {
        for (i = 0; (i < (length))&&(timeout > 0);)
        {
            if (DRV_Reg16(NFI_PIO_DIRDY_REG16) & 1)
            {
			    DRV_WriteReg32(NFI_DATAW_REG32, *buf++);
    			i++;
    		} 
            else 
            {
    			timeout--;
    		}
    		if (0 == timeout)
            {
                printk(KERN_ERR "[%s] timeout\n", __FUNCTION__);
                dump_nfi();
    			return false;
    		}
    	}
    }
    else
    {
        for (i = 0; (i < (length >> 2)) && (timeout > 0); )
        {
		    // if (FIFO_WR_REMAIN(DRV_Reg16(NFI_FIFOSTA_REG16)) <= 12)
            if (DRV_Reg16(NFI_PIO_DIRDY_REG16) & 1)
            {
			    DRV_WriteReg32(NFI_DATAW_REG32, *pBuf32++);
			    i++;
		    } 
            else 
            {
			    timeout--;
		    }
		    if (0 == timeout)
            {
                    printk(KERN_ERR "[%s] timeout\n", __FUNCTION__);
                    dump_nfi();
        			return false;
		    }
	    }
    }

	return true;
}


static bool mt6573_nand_write_page_data(struct mtd_info *mtd, u8* buf, u32 size)
{
#if (USE_AHB_MODE)
    return mt6573_nand_dma_write_data(mtd, buf, size);
#else
    return mt6573_nand_mcu_write_data(mtd, buf, size);
#endif
}

static void mt6573_nand_read_fdm_data(u8* pDataBuf, u32 u4SecNum)
{
	u32 i;
	u32* pBuf32 = (u32*)pDataBuf;

    if (pBuf32)
    {
    	for (i = 0; i < u4SecNum; ++i)
    	{
    		*pBuf32++ = DRV_Reg32(NFI_FDM0L_REG32 + (i<<1));
    		*pBuf32++ = DRV_Reg32(NFI_FDM0M_REG32 + (i<<1));
    		//*pBuf32++ = DRV_Reg32((u32)NFI_FDM0L_REG32 + (i<<3));
    		//*pBuf32++ = DRV_Reg32((u32)NFI_FDM0M_REG32 + (i<<3));
    	}
    }
}

static u8 fdm_buf[64];
static void mt6573_nand_write_fdm_data(struct nand_chip *chip, u8* pDataBuf, u32 u4SecNum)
{
	u32 i, j;
    u8 checksum = 0;
    bool empty = true;
    struct nand_oobfree *free_entry;
    u32* pBuf32;

    memcpy(fdm_buf, pDataBuf, u4SecNum * 8);

    free_entry = chip->ecc.layout->oobfree;
    for (i = 0; i < MTD_MAX_OOBFREE_ENTRIES && free_entry[i].length; i++)
    {
        for (j = 0; j < free_entry[i].length; j++)
        {
            if (pDataBuf[free_entry[i].offset + j] != 0xFF)
                empty = false;
            checksum ^= pDataBuf[free_entry[i].offset + j];
        }
    }

    if (!empty)
    {
        fdm_buf[free_entry[i-1].offset + free_entry[i-1].length] = checksum;
    }

	
    pBuf32 = (u32*)fdm_buf;
	for (i = 0; i < u4SecNum; ++i)
	{
		DRV_WriteReg32(NFI_FDM0L_REG32 + (i<<1), *pBuf32++);
		DRV_WriteReg32(NFI_FDM0M_REG32 + (i<<1), *pBuf32++);
		//DRV_WriteReg32((u32)NFI_FDM0L_REG32 + (i<<3), *pBuf32++);
		//DRV_WriteReg32((u32)NFI_FDM0M_REG32 + (i<<3), *pBuf32++);
	}
}

static void mt6573_nand_stop_read(void)
{
	NFI_CLN_REG16(NFI_CON_REG16, CON_NFI_BRD);
    mt6573_nand_reset();
	ECC_Decode_End();
    DRV_WriteReg16(NFI_INTR_EN_REG16, 0);
}

static void mt6573_nand_stop_write(void)
{
	NFI_CLN_REG16(NFI_CON_REG16, CON_NFI_BWR);
	ECC_Encode_End();
    DRV_WriteReg16(NFI_INTR_EN_REG16, 0);
}

bool mt6573_nand_exec_read_page(
	struct mtd_info *mtd, u32 u4RowAddr, u32 u4PageSize, u8* pPageBuf, u8* pFDMBuf)
{
    u8 *buf;
	bool bRet = true;
	struct nand_chip *nand = mtd->priv;
	u32 u4SecNum = u4PageSize >> 9;
#ifdef NAND_PFM
	struct timeval pfm_time_read;
#endif
	PFM_BEGIN(pfm_time_read);
    if (((u32)pPageBuf % 16) && local_buffer_16_align)
	{
        // printk(KERN_INFO "Data buffer not 16 bytes aligned: %p\n", pPageBuf);
        buf = local_buffer_16_align;
    }
    else
        buf = pPageBuf;

	if (mt6573_nand_ready_for_read(nand, u4RowAddr, 0, true, buf))
	{
		if (!mt6573_nand_read_page_data(mtd, buf, u4PageSize))
		{
			bRet = false;
		}
        
		if (!mt6573_nand_status_ready(STA_NAND_BUSY))
		{
			bRet = false;
		}
		
		if(!mt6573_nand_check_dececc_done(u4SecNum))
        {
            bRet = false;
        }       
        
		mt6573_nand_read_fdm_data(pFDMBuf, u4SecNum);
        
		if (!mt6573_nand_check_bch_error(mtd, buf, u4SecNum - 1, u4RowAddr))
		{
			bRet = false;
		}
		mt6573_nand_stop_read();
	}

    if (buf == local_buffer_16_align)
        memcpy(pPageBuf, buf, u4PageSize);

	PFM_END_R(pfm_time_read, u4PageSize + 32);
	return bRet;
}

int mt6573_nand_exec_write_page(
	struct mtd_info *mtd, u32 u4RowAddr, u32 u4PageSize, u8* pPageBuf, u8* pFDMBuf)
{
	struct nand_chip *chip = mtd->priv;
	u32 u4SecNum = u4PageSize >> 9;
    u8 *buf;
    u8 status;

    MSG(WRITE, "mt6573_nand_exec_write_page, page: 0x%x\n", u4RowAddr);

#ifdef _MTK_NAND_DUMMY_DRIVER_
    if (dummy_driver_debug)
    {
	    unsigned long long time = sched_clock();
        if (!((time * 123 + 59 ) % 32768))
        {
            printk(KERN_INFO "[NAND_DUMMY_DRIVER] Simulate write error at page: 0x%x\n", u4RowAddr);
            return -EIO;
        }
    }
#endif
    
#ifdef NAND_PFM
	struct timeval pfm_time_write;
#endif
	PFM_BEGIN(pfm_time_write);
    if (((u32)pPageBuf % 16) && local_buffer_16_align)
    {
        printk(KERN_INFO "Data buffer not 16 bytes aligned: %p\n", pPageBuf);
        memcpy(local_buffer_16_align, pPageBuf, mtd->writesize);
        buf = local_buffer_16_align;
    }
    else
        buf = pPageBuf;

    if (mt6573_nand_ready_for_write(chip, u4RowAddr, 0, true, buf))
	{
		mt6573_nand_write_fdm_data(chip, pFDMBuf, u4SecNum);
		(void)mt6573_nand_write_page_data(mtd, buf, u4PageSize);
		(void)mt6573_nand_check_RW_count(u4PageSize);
		mt6573_nand_stop_write();
        (void)mt6573_nand_set_command(NAND_CMD_PAGEPROG);
		while(DRV_Reg32(NFI_STA_REG32) & STA_NAND_BUSY);		
	}
	PFM_END_W(pfm_time_write, u4PageSize + 32);

    status = chip->waitfunc(mtd, chip);
    if (status & NAND_STATUS_FAIL)
        return -EIO;
    else
        return 0;
}


static int mt6573_nand_write_page(struct mtd_info *mtd, struct nand_chip *chip, 
        const u8 *buf, int page, int cached, int raw)
{
    int page_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);
    int block = page / page_per_block;
    u16 page_in_block = page % page_per_block;
    int mapped_block = get_mapping_block_index(block);

    // write bad index into oob
    if (mapped_block != block)
    {
        set_bad_index_to_oob(chip->oob_poi, block);
    }
    else 
    {
        set_bad_index_to_oob(chip->oob_poi, FAKE_INDEX);
    }

    if (mt6573_nand_exec_write_page(mtd, page_in_block + mapped_block * page_per_block, mtd->writesize, (u8 *)buf, chip->oob_poi))
    {
        MSG(INIT, "write fail at block: 0x%x, page: 0x%x\n", mapped_block, page_in_block);
        if (update_bmt((page_in_block + mapped_block * page_per_block) << chip->page_shift, 
                    UPDATE_WRITE_FAIL, (u8 *)buf, chip->oob_poi))
        {
            MSG(INIT, "Update BMT success\n");
            return 0;
        }
        else
        {
            MSG(INIT, "Update BMT fail\n");
            return -EIO;
        }
    }

    return 0;
}


//-------------------------------------------------------------------------------

static void mt6573_nand_command_bp(struct mtd_info *mtd, unsigned int command,
			 int column, int page_addr)
{
	struct nand_chip* nand = mtd->priv;
#ifdef NAND_PFM
	struct timeval pfm_time_erase;
#endif
    switch (command) 
    {
        case NAND_CMD_SEQIN:
		    /* Reset g_kCMD */
		//if (g_kCMD.u4RowAddr != page_addr) {
			memset(g_kCMD.au1OOB, 0xFF, sizeof(g_kCMD.au1OOB));
			g_kCMD.pDataBuf = NULL;
        //}
		    g_kCMD.u4RowAddr = page_addr;
		    g_kCMD.u4ColAddr = column;
            break;

        case NAND_CMD_PAGEPROG:
           	if (g_kCMD.pDataBuf || (0xFF != g_kCMD.au1OOB[0])) 
    		{
           		u8* pDataBuf = g_kCMD.pDataBuf ? g_kCMD.pDataBuf : nand->buffers->databuf;
    			mt6573_nand_exec_write_page(mtd, g_kCMD.u4RowAddr, mtd->writesize, pDataBuf, g_kCMD.au1OOB);
    			g_kCMD.u4RowAddr = (u32)-1;
    			g_kCMD.u4OOBRowAddr = (u32)-1;
            }
            break;

        case NAND_CMD_READOOB:
    		g_kCMD.u4RowAddr = page_addr;        	
    		g_kCMD.u4ColAddr = column + mtd->writesize;
    		#ifdef NAND_PFM
    		g_kCMD.pureReadOOB = 1;
    		g_kCMD.pureReadOOBNum += 1;
    		#endif
			break;
			
        case NAND_CMD_READ0:
    		g_kCMD.u4RowAddr = page_addr;        	
    		g_kCMD.u4ColAddr = column;
    		#ifdef NAND_PFM
    		g_kCMD.pureReadOOB = 0;
    		#endif		
			break;

        case NAND_CMD_ERASE1:
    		PFM_BEGIN(pfm_time_erase);
    		(void)mt6573_nand_reset();
            mt6573_nand_set_mode(CNFG_OP_ERASE);
    		(void)mt6573_nand_set_command(NAND_CMD_ERASE1);
    		(void)mt6573_nand_set_address(0,page_addr,0,devinfo.addr_cycle-2);
            break;
            
        case NAND_CMD_ERASE2:
       	    (void)mt6573_nand_set_command(NAND_CMD_ERASE2);
			while(DRV_Reg32(NFI_STA_REG32) & STA_NAND_BUSY);
		    PFM_END_E(pfm_time_erase);
            break;
            
        case NAND_CMD_STATUS:
            (void)mt6573_nand_reset(); 
            NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);		           	
			mt6573_nand_set_mode(CNFG_OP_SRD);
            mt6573_nand_set_mode(CNFG_READ_EN);
            NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);
            NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
		    (void)mt6573_nand_set_command(NAND_CMD_STATUS);
        	NFI_CLN_REG16(NFI_CON_REG16, CON_NFI_NOB_MASK);
			DRV_WriteReg16(NFI_CON_REG16, CON_NFI_SRD|(1 << CON_NFI_NOB_SHIFT));
            g_bcmdstatus = true;            
            break;
            
        case NAND_CMD_RESET:
       	    (void)mt6573_nand_reset();
			//mt6573_nand_exec_reset_device();
            break;

		case NAND_CMD_READID: 
            mt6573_nand_reset();
            /* Disable HW ECC */
            NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
            NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);
    
            /* Disable 16-bit I/O */
            //NFI_CLN_REG16(NFI_PAGEFMT_REG16, PAGEFMT_DBYTE_EN);
		
			NFI_SET_REG16(NFI_CNFG_REG16, CNFG_READ_EN|CNFG_BYTE_RW);
		    (void)mt6573_nand_reset();
			mt6573_nand_set_mode(CNFG_OP_SRD);
		    (void)mt6573_nand_set_command(NAND_CMD_READID);
		    (void)mt6573_nand_set_address(0,0,1,0);
			DRV_WriteReg16(NFI_CON_REG16, CON_NFI_SRD);
			while(DRV_Reg32(NFI_STA_REG32) & STA_DATAR_STATE);
			break;
            
        default:
            BUG();        
            break;
    }
 }

static void mt6573_nand_select_chip(struct mtd_info *mtd, int chip)
{
	if (chip == -1 && false == g_bInitDone)
	{
		struct nand_chip *nand = mtd->priv;
		/* Setup PageFormat */
	if (4096 == mtd->writesize) {
       		NFI_SET_REG16(NFI_PAGEFMT_REG16, (PAGEFMT_SPARE_16 << PAGEFMT_SPARE_SHIFT) | PAGEFMT_4K);
                nand->cmdfunc = mt6573_nand_command_bp;		
	} else if (2048 == mtd->writesize) {
       		NFI_SET_REG16(NFI_PAGEFMT_REG16, (PAGEFMT_SPARE_16 << PAGEFMT_SPARE_SHIFT) | PAGEFMT_2K);
                nand->cmdfunc = mt6573_nand_command_bp;
        }/* else if (512 == mtd->writesize) {
       		NFI_SET_REG16(NFI_PAGEFMT_REG16, (PAGEFMT_SPARE_16 << PAGEFMT_SPARE_SHIFT) | PAGEFMT_512);
	       	nand->cmdfunc = mt6573_nand_command_sp;
    	}*/
		g_bInitDone = true;
	}
    switch(chip)
    {
	case -1:
		break;
	case 0: 
	case 1:
		DRV_WriteReg16(NFI_CSEL_REG16, chip);
		break;
    }
}

static uint8_t mt6573_nand_read_byte(struct mtd_info *mtd)
{
#if 0
	//while(0 == FIFO_RD_REMAIN(DRV_Reg16(NFI_FIFOSTA_REG16)));
	/* Check the PIO bit is ready or not */
    u32 timeout = TIMEOUT_4;
    uint8_t retval = 0;
    WAIT_NFI_PIO_READY(timeout);   

    retval = DRV_Reg8(NFI_DATAR_REG32);
    MSG(INIT, "mt6573_nand_read_byte (0x%x)\n", retval);
    
    if(g_bcmdstatus)
    {
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_AHB);
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
        g_bcmdstatus = false;
    }
    
    return retval;
#endif
    uint8_t retval = 0;
    
    if (!mt6573_nand_pio_ready())
    {
        printk("pio ready timeout\n");
        retval = false;
    }

    if(g_bcmdstatus)
    {
        retval = DRV_Reg8(NFI_DATAR_REG32);
        NFI_CLN_REG16(NFI_CON_REG16, CON_NFI_NOB_MASK); 
        mt6573_nand_reset();
#if (USE_AHB_MODE)        
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_AHB);
#endif
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
        g_bcmdstatus = false;
    }
    else
        retval = DRV_Reg8(NFI_DATAR_REG32);

    return retval;
}

static void mt6573_nand_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	struct nand_chip* nand = (struct nand_chip*)mtd->priv;
	struct mt6573_CMD* pkCMD = &g_kCMD;
	u32 u4ColAddr = pkCMD->u4ColAddr;
	u32 u4PageSize = mtd->writesize;
		
	if (u4ColAddr < u4PageSize) 
	{
		if ((u4ColAddr == 0) && (len >= u4PageSize)) 
		{
			mt6573_nand_exec_read_page(mtd, pkCMD->u4RowAddr, u4PageSize, 
									   buf, pkCMD->au1OOB);
			if (len > u4PageSize) 
			{
				u32 u4Size = min(len - u4PageSize, sizeof(pkCMD->au1OOB));
				memcpy(buf + u4PageSize, pkCMD->au1OOB, u4Size);
			}
		} 
		else 
		{
			mt6573_nand_exec_read_page(mtd, pkCMD->u4RowAddr, u4PageSize, 
									   nand->buffers->databuf, pkCMD->au1OOB);
			memcpy(buf, nand->buffers->databuf + u4ColAddr, len);
		}
		pkCMD->u4OOBRowAddr = pkCMD->u4RowAddr;
	} 
	else 
	{
		u32 u4Offset = u4ColAddr - u4PageSize;
		u32 u4Size = min(len - u4Offset, sizeof(pkCMD->au1OOB));
		if (pkCMD->u4OOBRowAddr != pkCMD->u4RowAddr) 
		{
			mt6573_nand_exec_read_page(mtd, pkCMD->u4RowAddr, u4PageSize,
									   nand->buffers->databuf, pkCMD->au1OOB);
			pkCMD->u4OOBRowAddr = pkCMD->u4RowAddr;
		}
		memcpy(buf, pkCMD->au1OOB + u4Offset, u4Size);
	}
	pkCMD->u4ColAddr += len;	
}

static void mt6573_nand_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	struct mt6573_CMD* pkCMD = &g_kCMD;
	u32 u4ColAddr = pkCMD->u4ColAddr;
	u32 u4PageSize = mtd->writesize;
    int i4Size, i;

	if (u4ColAddr >= u4PageSize) 
    {
	    u32 u4Offset = u4ColAddr - u4PageSize;
		u8* pOOB = pkCMD->au1OOB + u4Offset;
		i4Size = min(len, (int)(sizeof(pkCMD->au1OOB) - u4Offset));
        
		for (i = 0; i < i4Size; i++) 
        {
			pOOB[i] &= buf[i];
		}
	} 
    else 
    {
		pkCMD->pDataBuf = (u8*)buf;
    }
    
	pkCMD->u4ColAddr += len;	
}	

static void mt6573_nand_write_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip, const uint8_t *buf)
{
	mt6573_nand_write_buf(mtd, buf, mtd->writesize);
	mt6573_nand_write_buf(mtd, chip->oob_poi, mtd->oobsize);
}

static int mt6573_nand_read_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip, uint8_t *buf, int page)
{
#if 0
	mt6573_nand_read_buf(mtd, buf, mtd->writesize);
	mt6573_nand_read_buf(mtd, chip->oob_poi, mtd->oobsize);
#else
	struct mt6573_CMD* pkCMD = &g_kCMD;
	u32 u4ColAddr = pkCMD->u4ColAddr;
	u32 u4PageSize = mtd->writesize;
		
	if (u4ColAddr == 0) 
    {
        mt6573_nand_exec_read_page(mtd, pkCMD->u4RowAddr, u4PageSize, buf, chip->oob_poi);
        pkCMD->u4ColAddr += u4PageSize + mtd->oobsize;
	}
#endif
	return 0;
}

static int mt6573_nand_read_page(struct mtd_info *mtd, struct nand_chip *chip, u8 *buf, int page)
{
    int page_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);
    int block = page / page_per_block;
    u16 page_in_block = page % page_per_block;
    int mapped_block = get_mapping_block_index(block);

    if (mt6573_nand_exec_read_page(mtd, page_in_block + mapped_block * page_per_block,
                mtd->writesize, buf, chip->oob_poi))
        return 0;
    else
        return -EIO;
}

int mt6573_nand_erase_hw(struct mtd_info *mtd, int page)
{
    struct nand_chip *chip = (struct nand_chip *)mtd->priv;
    
#ifdef _MTK_NAND_DUMMY_DRIVER_
    if (dummy_driver_debug)
    {
	    unsigned long long time = sched_clock();
        if (!((time * 123 + 59 ) % 1024))
        {
            printk(KERN_INFO "[NAND_DUMMY_DRIVER] Simulate erase error at page: 0x%x\n", page);
            return NAND_STATUS_FAIL;
        }
    }
#endif

    chip->erase_cmd(mtd, page);

    return chip->waitfunc(mtd, chip);
}

static int mt6573_nand_erase(struct mtd_info *mtd, int page)
{
    // get mapping 
    struct nand_chip *chip = mtd->priv;
    int page_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);
    int page_in_block = page % page_per_block;
    int block = page / page_per_block;
 
    int mapped_block = get_mapping_block_index(block);

    int status = mt6573_nand_erase_hw(mtd, page_in_block + page_per_block * mapped_block);

    if (status & NAND_STATUS_FAIL)
    {
        if (update_bmt( (page_in_block + mapped_block * page_per_block) << chip->page_shift, 
                    UPDATE_ERASE_FAIL, NULL, NULL))
        {
            MSG(INIT, "Erase fail at block: 0x%x, update BMT success\n", mapped_block);
            return 0;
        }
        else
        {
            MSG(INIT, "Erase fail at block: 0x%x, update BMT fail\n", mapped_block);
            return NAND_STATUS_FAIL;
        }
    }

    return 0;
}



#if 0
static int mt6573_nand_read_multi_page_cache(struct mtd_info *mtd, struct nand_chip *chip,
        int page, struct mtd_oob_ops *ops)
{
    int res = -EIO;
    int len = ops->len;
    struct mtd_ecc_stats stat = mtd->ecc_stats;
    uint8_t *buf = ops->datbuf;

    if (!mt6573_nand_ready_for_read(chip, page, 0, true, buf))
        return -EIO;

    while (len > 0)
    {
        mt6573_nand_set_mode(CNFG_OP_CUST);
        DRV_WriteReg16(NFI_CON_REG16, 8 << CON_NFI_SEC_SHIFT);

        if (len > mtd->writesize)               // remained more than one page
        {
            if (!mt6573_nand_set_command(0x31))      // todo: add cache read command
                goto ret;
        }
        else
        {
            if (!mt6573_nand_set_command(0x3f))      // last page remained
                goto ret;
        }

        mt6573_nand_status_ready(STA_NAND_BUSY);

#ifdef USE_AHB_MODE
        //if (!mt6573_nand_dma_read_data(buf, mtd->writesize))
        if (!mt6573_nand_read_page_data(mtd, buf, mtd->writesize))
            goto ret;
#else
        if (!mt6573_nand_mcu_read_data(buf, mtd->writesize))
            goto ret;
#endif

        // get ecc error info
        mt6573_nand_check_bch_error(mtd, buf, 3, page);
        ECC_Decode_End();

        page++;
        len -= mtd->writesize;
        buf += mtd->writesize;
        ops->retlen += mtd->writesize;

        if (len > 0)
        {
            ECC_Decode_Start();
            mt6573_nand_reset();
        }

    }

    res = 0;

ret:
    mt6573_nand_stop_read();

    if (res)
        return res;

    if (mtd->ecc_stats.failed > stat.failed)
    {
        printk(KERN_INFO "ecc fail happened\n");
        return -EBADMSG;
    }

	return  mtd->ecc_stats.corrected - stat.corrected ? -EUCLEAN: 0;
}
#endif

static int mt6573_nand_read_oob_raw(struct mtd_info *mtd, uint8_t *buf, int page_addr, int len)
{
    struct nand_chip *chip = (struct nand_chip *)mtd->priv;
    u32 col_addr = 0;
    u32 sector = 0;
    int res = 0;
    u32 colnob=2, rawnob=devinfo.addr_cycle-2;
	int randomread =0;
    int read_len = 0;

    if (len > 128 || len % OOB_AVAI_PER_SECTOR || !buf)
    {
        printk(KERN_WARNING "[%s] invalid parameter, len: %d, buf: %p\n",
                __FUNCTION__, len, buf);
        return -EINVAL;
    }
    if(len>16)
    {
        randomread=1;
    }
	if(!randomread||!(devinfo.advancedmode & RAMDOM_READ))
	{
	    while (len > 0)
	    {
	        read_len = min(len, OOB_PER_SECTOR);
	        col_addr = NAND_SECTOR_SIZE + sector * (NAND_SECTOR_SIZE + OOB_PER_SECTOR); // TODO: Fix this hard-code 16
	        if (!mt6573_nand_ready_for_read(chip, page_addr, col_addr, false, NULL))
	        {
	            printk(KERN_WARNING "mt6573_nand_ready_for_read return failed\n");
	            res = -EIO;
	            goto error;
	        }
	        if (!mt6573_nand_mcu_read_data(buf + OOB_PER_SECTOR * sector, read_len))    // TODO: and this 8
	        {
	            printk(KERN_WARNING "mt6573_nand_mcu_read_data return failed\n");
	            res = -EIO;
	            goto error;
	        }
	        mt6573_nand_stop_read();
			//dump_data(buf + 16 * sector,16);
	        sector++;
	        len -= read_len;
			
	    }
	}
    else  //should be 64
	{
	    col_addr = NAND_SECTOR_SIZE;
	    if (chip->options & NAND_BUSWIDTH_16)
	    {
	        col_addr /= 2;
	    }
		
		if (!mt6573_nand_reset())
	    {
			goto error;
		}

		mt6573_nand_set_mode(0x6000);
		NFI_SET_REG16(NFI_CNFG_REG16, CNFG_READ_EN);
	    DRV_WriteReg16(NFI_CON_REG16, 4 << CON_NFI_SEC_SHIFT);


	    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);
	    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);


		mt6573_nand_set_autoformat(false);
		

		if (!mt6573_nand_set_command(NAND_CMD_READ0))
	    {
			goto error;
		}		

		//1 FIXED ME: For Any Kind of AddrCycle
		if (!mt6573_nand_set_address(col_addr, page_addr, colnob, rawnob))
	    {
			goto error;
		}

		if (!mt6573_nand_set_command(NAND_CMD_READSTART))
	    {
			goto error;
		}
		if (!mt6573_nand_status_ready(STA_NAND_BUSY))
	    {
			goto error;
		}
		

		read_len = min(len, OOB_PER_SECTOR);
		if (!mt6573_nand_mcu_read_data(buf + OOB_PER_SECTOR * sector, read_len))    // TODO: and this 8
		{
			printk(KERN_WARNING "mt6573_nand_mcu_read_data return failed first 16\n");
			res = -EIO;
			goto error;
		}
		sector++;
		len -= read_len;
		mt6573_nand_stop_read();
		while(len>0)
		{
		    read_len = min(len, 16);
			if (!mt6573_nand_set_command(0x05))
			{
				goto error;
			}

			col_addr =  NAND_SECTOR_SIZE + sector * (NAND_SECTOR_SIZE + 16);
			if (chip->options & NAND_BUSWIDTH_16)
		    {
		        col_addr /= 2;
		    }
			DRV_WriteReg32(NFI_COLADDR_REG32, col_addr);
			DRV_WriteReg16(NFI_ADDRNOB_REG16, 2);
	        DRV_WriteReg16(NFI_CON_REG16, 4 << CON_NFI_SEC_SHIFT);

			if( !mt6573_nand_status_ready(STA_ADDR_STATE))
			{
				goto error;
			}

			if (!mt6573_nand_set_command(0xE0))
		    {
				goto error;
			}
			if (!mt6573_nand_status_ready(STA_NAND_BUSY))
		    {
				goto error;
			}
			if (!mt6573_nand_mcu_read_data(buf + OOB_PER_SECTOR * sector, read_len))    // TODO: and this 8
			{
				printk(KERN_WARNING "mt6573_nand_mcu_read_data return failed first 16\n");
				res = -EIO;
				goto error;
			}
			mt6573_nand_stop_read();
			sector++;
			len -= read_len;
		}
		//dump_data(&testbuf[16],16);
		//printk(KERN_ERR "\n");
	}
error:
    NFI_CLN_REG16(NFI_CON_REG16, CON_NFI_BRD);
    return res;
}

static int mt6573_nand_write_oob_raw(struct mtd_info *mtd, const uint8_t *buf, int page_addr, int len)
{
    struct nand_chip *chip = mtd->priv;
    // int i;
    u32 col_addr = 0;
    u32 sector = 0;
    // int res = 0;
    // u32 colnob=2, rawnob=devinfo.addr_cycle-2;
	// int randomread =0;
    int write_len = 0;
    int status;

    if (len > 64 || len % OOB_AVAI_PER_SECTOR || !buf)
    {
        printk(KERN_WARNING "[%s] invalid parameter, len: %d, buf: %p\n",
                __FUNCTION__, len, buf);
        return -EINVAL;
    }

    while (len > 0)
    {
        write_len = min(len, OOB_PER_SECTOR);
        col_addr = sector * (NAND_SECTOR_SIZE + OOB_PER_SECTOR) + NAND_SECTOR_SIZE;
        if (!mt6573_nand_ready_for_write(chip, page_addr, col_addr, false, NULL))
        {
            return -EIO;
        }
        
        if (!mt6573_nand_mcu_write_data(mtd, buf + sector * OOB_PER_SECTOR, write_len))
        {
            return -EIO;
        }
        
		(void)mt6573_nand_check_RW_count(write_len);
	    NFI_CLN_REG16(NFI_CON_REG16, CON_NFI_BWR);
        (void)mt6573_nand_set_command(NAND_CMD_PAGEPROG);
		
        while(DRV_Reg32(NFI_STA_REG32) & STA_NAND_BUSY);		
	
        status = chip->waitfunc(mtd, chip);
        if (status & NAND_STATUS_FAIL)
        {
            printk(KERN_INFO "status: %d\n", status);
            return -EIO;
        }

        len -= write_len;
        sector++;
    }

    return 0;
}

static int mt6573_nand_write_oob_hw(struct mtd_info *mtd, struct nand_chip *chip, int page)
{
    // u8 *buf = chip->oob_poi;
    int i, iter;

    memcpy(local_oob_buf, chip->oob_poi, mtd->oobsize);

    // copy ecc data
    for (i = 0; i < chip->ecc.layout->eccbytes; i++)
    {
        iter = (i / OOB_AVAI_PER_SECTOR) * OOB_PER_SECTOR + OOB_AVAI_PER_SECTOR + i % OOB_AVAI_PER_SECTOR;
        local_oob_buf[iter] = chip->oob_poi[chip->ecc.layout->eccpos[i]];
        // chip->oob_poi[chip->ecc.layout->eccpos[i]] = local_oob_buf[iter];
    }

    // copy FDM data
    for (i = 0; i < 4; i++)
    {
        memcpy(&local_oob_buf[i * OOB_PER_SECTOR], &chip->oob_poi[i * OOB_AVAI_PER_SECTOR], OOB_AVAI_PER_SECTOR);
    }

    return mt6573_nand_write_oob_raw(mtd, local_oob_buf, page, mtd->oobsize);
}

static int mt6573_nand_write_oob(struct mtd_info *mtd, struct nand_chip *chip, int page)
{
    int page_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);
    int block = page / page_per_block;
    u16 page_in_block = page % page_per_block;
    int mapped_block = get_mapping_block_index(block);

    // write bad index into oob
    if (mapped_block != block)
    {
        set_bad_index_to_oob(chip->oob_poi, block);
    }
    else 
    {
        set_bad_index_to_oob(chip->oob_poi, FAKE_INDEX);
    }

    if (mt6573_nand_write_oob_hw(mtd, chip, page_in_block + mapped_block * page_per_block /* page */))
    {
        MSG(INIT, "write oob fail at block: 0x%x, page: 0x%x\n", mapped_block, page_in_block);
        if (update_bmt((page_in_block + mapped_block * page_per_block) << chip->page_shift, 
                    UPDATE_WRITE_FAIL, NULL, chip->oob_poi))
        {
            MSG(INIT, "Update BMT success\n");
            return 0;
        }
        else
        {
            MSG(INIT, "Update BMT fail\n");
            return -EIO;
        }
    }

    return 0;
}

int mt6573_nand_block_markbad_hw(struct mtd_info *mtd, loff_t offset)
{
    struct nand_chip *chip = mtd->priv;
    int block = (int)offset >> chip->phys_erase_shift;
    int page = block * (1 << (chip->phys_erase_shift - chip->page_shift));
    int ret;

    u8 buf[8];
    memset(buf, 0xFF, 8);
    buf[0] = 0;

    ret = mt6573_nand_write_oob_raw(mtd, buf, page, 8);
    return ret;
}

static int mt6573_nand_block_markbad(struct mtd_info *mtd, loff_t offset)
{
    struct nand_chip *chip = mtd->priv;
    int block = (int)offset >> chip->phys_erase_shift;
    int mapped_block;
    int ret;

    nand_get_device(chip, mtd, FL_WRITING);

    mapped_block = get_mapping_block_index(block);
    ret = mt6573_nand_block_markbad_hw(mtd, mapped_block << chip->phys_erase_shift);

    nand_release_device(mtd);

    return ret;
}

int mt6573_nand_read_oob_hw(struct mtd_info *mtd,struct nand_chip *chip, int page)
{
    int i;
    u8 iter = 0;
#ifdef TESTTIME
	unsigned long long time1,time2;

	time1 = sched_clock();
#endif
    
    if (mt6573_nand_read_oob_raw(mtd, chip->oob_poi, page, mtd->oobsize))
    {
        // printk(KERN_ERR "[%s]mt6573_nand_read_oob_raw return failed\n", __FUNCTION__);
        return -EIO;
    }
#ifdef TESTTIME
    time2= sched_clock()-time1;
	if(!readoobflag)
	{  
	   readoobflag=1;
	   printk(KERN_ERR "[%s] time is %llu",__FUNCTION__,time2);
	}
#endif	

    // adjust to ecc physical layout to memory layout
    /*********************************************************/
    /* FDM0 | ECC0 | FDM1 | ECC1 | FDM2 | ECC2 | FDM3 | ECC3 */
    /*  8B  |  8B  |  8B  |  8B  |  8B  |  8B  |  8B  |  8B  */
    /*********************************************************/
    
    memcpy(local_oob_buf, chip->oob_poi, mtd->oobsize);

    // copy ecc data
    for (i = 0; i < chip->ecc.layout->eccbytes; i++)
    {
        iter = (i / OOB_AVAI_PER_SECTOR) * OOB_PER_SECTOR + OOB_AVAI_PER_SECTOR + i % OOB_AVAI_PER_SECTOR;
        chip->oob_poi[chip->ecc.layout->eccpos[i]] = local_oob_buf[iter];
    }

    // copy FDM data
    for (i = 0; i < 4; i++)
    {
        memcpy(&chip->oob_poi[i * OOB_AVAI_PER_SECTOR], &local_oob_buf[i * OOB_PER_SECTOR], OOB_AVAI_PER_SECTOR);
    }

    return 0;
}

static int mt6573_nand_read_oob(struct mtd_info *mtd,struct nand_chip *chip, int page, int sndcmd)
{
    int page_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);
    int block = page / page_per_block;
    u16 page_in_block = page % page_per_block;
    int mapped_block = get_mapping_block_index(block);

    mt6573_nand_read_oob_hw(mtd, chip, page_in_block + mapped_block * page_per_block);

    return 0;       // the return value is sndcmd
}


int mt6573_nand_block_bad_hw(struct mtd_info *mtd, loff_t ofs)
{
    struct nand_chip *chip = (struct nand_chip *)mtd->priv;
	int page_addr = (int)(ofs >> chip->page_shift);
    unsigned int page_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);


    unsigned char oob_buf[8];
    page_addr &= ~(page_per_block - 1);

    if (mt6573_nand_read_oob_raw(mtd, oob_buf, page_addr, sizeof(oob_buf)))
    {
        printk(KERN_WARNING "mt6573_nand_read_oob_raw return error\n");
        return 1;
    }
    
    if (oob_buf[0] != 0xff)
    {
        printk(KERN_WARNING "Bad block detected at 0x%x, oob_buf[0] is 0x%x\n", page_addr, oob_buf[0]);
        // dump_nfi();
        return 1;
    }

    return 0;        // everything is OK, good block
}


static int mt6573_nand_block_bad(struct mtd_info *mtd, loff_t ofs, int getchip)
{
    int chipnr = 0;
    
    struct nand_chip *chip = (struct nand_chip *)mtd->priv;
    int block = (int)ofs >> chip->phys_erase_shift;
    int mapped_block;

    int ret;

    if (getchip) {
		chipnr = (int)(ofs >> chip->chip_shift);
		nand_get_device(chip, mtd, FL_READING);
		/* Select the NAND device */
		chip->select_chip(mtd, chipnr);
	}

    mapped_block = get_mapping_block_index(block);

    ret = mt6573_nand_block_bad_hw(mtd, mapped_block << chip->phys_erase_shift);

    if (ret)
    {
        MSG(INIT, "Unmapped bad block: 0x%x\n", mapped_block);
        if (update_bmt(mapped_block << chip->phys_erase_shift, UPDATE_UNMAPPED_BLOCK, NULL, NULL))
        {
            MSG(INIT, "Update BMT success\n");
            ret = 0;
        }
        else
        {
            MSG(INIT, "Update BMT fail\n");
            ret = 1;
        }
    }

    if (getchip)
    {
        nand_release_device(mtd);
    }

    return ret;
}


#ifdef CONFIG_MTD_NAND_VERIFY_WRITE

char gacBuf[4096 + 128];

static int mt6573_nand_verify_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
#if 1
	struct nand_chip* chip = (struct nand_chip*)mtd->priv;
	struct mt6573_CMD* pkCMD = &g_kCMD;
	u32 u4PageSize = mtd->writesize;
	u32 *pSrc, *pDst;
	int i;

    mt6573_nand_exec_read_page(mtd, pkCMD->u4RowAddr, u4PageSize, gacBuf, gacBuf + u4PageSize);

	pSrc = (u32*)buf;
	pDst = (u32*)gacBuf;
	len = len/sizeof(u32);
	for (i = 0; i < len; ++i) 
    {
		if (*pSrc != *pDst) 
        {
			MSG(VERIFY, "mt6573_nand_verify_buf page fail at page %d\n", pkCMD->u4RowAddr);
            return -1;
		}
		pSrc++;
		pDst++;
	}
    
	pSrc = (u32*)chip->oob_poi;
	pDst = (u32*)(gacBuf + u4PageSize);
    
	if ((pSrc[0] != pDst[0]) || (pSrc[1] != pDst[1]) ||
	    (pSrc[2] != pDst[2]) || (pSrc[3] != pDst[3]) ||
	    (pSrc[4] != pDst[4]) || (pSrc[5] != pDst[5]))
	    // TODO: Ask Designer Why?
	    //(pSrc[6] != pDst[6]) || (pSrc[7] != pDst[7])) 
    {
        MSG(VERIFY, "mt6573_nand_verify_buf oob fail at page %d\n", pkCMD->u4RowAddr);
		MSG(VERIFY, "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", 
		    pSrc[0], pSrc[1], pSrc[2], pSrc[3], pSrc[4], pSrc[5], pSrc[6], pSrc[7]);
		MSG(VERIFY, "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", 
		    pDst[0], pDst[1], pDst[2], pDst[3], pDst[4], pDst[5], pDst[6], pDst[7]);
		return -1;		
    }
	/*
	for (i = 0; i < len; ++i) {
		if (*pSrc != *pDst) {
			printk(KERN_ERR"mt6573_nand_verify_buf oob fail at page %d\n", g_kCMD.u4RowAddr);
			return -1;
		}
		pSrc++;
		pDst++;
	}
	*/
	//printk(KERN_INFO"mt6573_nand_verify_buf OK at page %d\n", g_kCMD.u4RowAddr);
	
	return 0;
#else
    return 0;
#endif
}
#endif

static void mt6573_nand_init_hw(struct mt6573_nand_host *host)
{
	struct mt6573_nand_host_hw *hw = host->hw;
	

    MSG(INIT, "Enable NFI Clock\n");
    nand_enable_clock();

	g_bInitDone = false;
    /* Get the HW_VER */
    //g_u4ChipVer = DRV_Reg32(CONFIG_BASE);
	g_kCMD.u4OOBRowAddr  = (u32)-1;

    /* Set default NFI access timing control */
	DRV_WriteReg32(NFI_ACCCON_REG32, hw->nfi_access_timing);
	DRV_WriteReg16(NFI_CNFG_REG16, 0);
	DRV_WriteReg16(NFI_PAGEFMT_REG16, 0);
	
    /* Reset the state machine and data FIFO, because flushing FIFO */
	(void)mt6573_nand_reset();
	
    /* Set the ECC engine */
    if(hw->nand_ecc_mode == NAND_ECC_HW)
	{
		MSG(INIT, "%s : Use HW ECC\n", MODULE_NAME);
		NFI_SET_REG32(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
		ECC_Config(host->hw);
   		mt6573_nand_configure_fdm(8);
		mt6573_nand_configure_lock();
	}

	/* Initilize interrupt. Clear interrupt, read clear. */
    DRV_Reg16(NFI_INTR_REG16);
	
    /* Interrupt arise when read data or program data to/from AHB is done. */
	DRV_WriteReg16(NFI_INTR_EN_REG16, 0);
}

//-------------------------------------------------------------------------------
static int mt6573_nand_dev_ready(struct mtd_info *mtd)
{	
    return !(DRV_Reg32(NFI_STA_REG32) & STA_NAND_BUSY);
}

static int mt6573_nand_proc_read(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
    int len;
	if (off > 0) 
    {
		return 0;
	}
	// return sprintf(page, "Interrupt-Scheme is %d\n", g_i4Interrupt);
    len = sprintf(page, "ID: 0x%x, total size: %dMiB\n", devinfo.id, devinfo.totalsize);
    len += sprintf(page + len, "Current working in %s mode\n", g_i4Interrupt ? "interrupt" : "polling");

    return len;
}

static int mt6573_nand_proc_write(struct file* file, const char* buffer,
	unsigned long count, void *data)
{
    struct mtd_info *mtd = &host->mtd;
	char buf[16];
	int len = count, n;

	if (len >= sizeof(buf)) 
    {
		len = sizeof(buf) - 1;
	}

	if (copy_from_user(buf, buffer, len)) 
    {
		return -EFAULT;
	}

	buf[len] = '\0';
	if (buf[0] == 'I') 
    {
        // sync before switching between polling and interrupt, 
		n = simple_strtol(buf+1, NULL, 10);
 
        if ((n > 0 && !g_i4Interrupt) || 
            (n == 0 && g_i4Interrupt))
        {
            nand_get_device((struct nand_chip *)mtd->priv, mtd, FL_READING);

            g_i4Interrupt = n;
            
            if (g_i4Interrupt)
			{
				DRV_Reg16(NFI_INTR_REG16);
                enable_irq(MT6573_NFI_IRQ_LINE);
            }
            else
                disable_irq(MT6573_NFI_IRQ_LINE);
            
            nand_release_device(mtd);
    	} 
    } 

    if (buf[0] == 'D')
    {
#ifdef _MTK_NAND_DUMMY_DRIVER_
        printk(KERN_INFO "Enable dummy driver\n");
        dummy_driver_debug = 1;
#endif
    } 
	
#ifdef NAND_PFM
	if (buf[0] == 'P') 
    {
        /* Reset values */
		g_PFM_R = 0;
		g_PFM_W = 0;
		g_PFM_E = 0;
		g_PFM_RD = 0;
		g_PFM_WD = 0;
		g_kCMD.pureReadOOBNum = 0;
	}
#endif

	return len;
}

static int mt6573_nand_probe(struct platform_device *pdev)
{
	
	struct mt6573_nand_host_hw *hw;	
	struct mtd_info *mtd;
	struct nand_chip *nand_chip;
    struct resource *res = pdev->resource;	
	int err = 0;
    int id;
    u32 ext_id;
    u8 ext_id1, ext_id2, ext_id3;
   
    hw = (struct mt6573_nand_host_hw*)pdev->dev.platform_data;
    BUG_ON(!hw);

	if (pdev->num_resources != 4 ||
	    res[0].flags != IORESOURCE_MEM || 
	    res[1].flags != IORESOURCE_MEM ||
	    res[2].flags != IORESOURCE_IRQ ||
   	    res[3].flags != IORESOURCE_IRQ)
   	{
		MSG(INIT, "%s: invalid resource type\n", __FUNCTION__);
		return -ENODEV;
	}

	/* Request IO memory */
	if (!request_mem_region(res[0].start,
				            res[0].end - res[0].start + 1, 
				            pdev->name)) 
	{
		return -EBUSY;
	}
	if (!request_mem_region(res[1].start,
				            res[1].end - res[1].start + 1, 
				            pdev->name)) 
	{
		return -EBUSY;
	}

	/* Allocate memory for the device structure (and zero it) */
	host = kzalloc(sizeof(struct mt6573_nand_host), GFP_KERNEL);	
	if (!host) 
	{
		MSG(INIT, "mt6573_nand: failed to allocate device structure.\n");
		return -ENOMEM;
	}

	/* Allocate memory for 16 byte aligned buffer */
    local_buffer_16_align = local_buffer + 16 - ((u32)local_buffer % 16);
    printk(KERN_INFO "Allocate 16 byte aligned buffer: %p\n", local_buffer_16_align);

    host->hw = hw;

	/* init mtd data structure */
	nand_chip  = &host->nand_chip;
	nand_chip->priv = host;		/* link the private data structures */
	
	mtd        = &host->mtd;	
	mtd->priv  = nand_chip;
	mtd->owner = THIS_MODULE;
	mtd->name  = "MT6573-Nand";

    hw->nand_ecc_mode = NAND_ECC_HW;

	/* Set address of NAND IO lines */
	nand_chip->IO_ADDR_R 	    = (void __iomem*)NFI_DATAR_REG32;
	nand_chip->IO_ADDR_W 	    = (void __iomem*)NFI_DATAW_REG32;
	nand_chip->chip_delay 	    = 20;			/* 20us command delay time */
	nand_chip->ecc.mode 	    = hw->nand_ecc_mode;	/* enable ECC */

	nand_chip->read_byte        = mt6573_nand_read_byte;
	nand_chip->read_buf		    = mt6573_nand_read_buf;
	nand_chip->write_buf	    = mt6573_nand_write_buf;
#ifdef CONFIG_MTD_NAND_VERIFY_WRITE	
	nand_chip->verify_buf       = mt6573_nand_verify_buf;
#endif
    nand_chip->select_chip      = mt6573_nand_select_chip;
    nand_chip->dev_ready 	    = mt6573_nand_dev_ready;
	nand_chip->cmdfunc 		    = mt6573_nand_command_bp;	
   	nand_chip->ecc.read_page    = mt6573_nand_read_page_hwecc;
	nand_chip->ecc.write_page   = mt6573_nand_write_page_hwecc;


    nand_chip->ecc.layout	    = &nand_oob_64;
    nand_chip->ecc.size		    = hw->nand_ecc_size;	//2048
    nand_chip->ecc.bytes	    = hw->nand_ecc_bytes;	//32
	//nand_chip->options		    = NAND_USE_FLASH_BBT;
	nand_chip->options		 = NAND_SKIP_BBTSCAN;
	//nand_chip->options		 = NAND_USE_FLASH_BBT | NAND_NO_AUTOINCR;
								/*
							   BBT_AUTO_REFRESH      | 
		                       NAND_NO_SUBPAGE_WRITE | 
		                       NAND_NO_AUTOINCR;
		                       */

    // For BMT, we need to revise driver architecture
    nand_chip->write_page       = mt6573_nand_write_page;
    nand_chip->ecc.write_oob    = mt6573_nand_write_oob;
    nand_chip->read_page        = mt6573_nand_read_page;
    nand_chip->ecc.read_oob     = mt6573_nand_read_oob;
    nand_chip->block_markbad    = mt6573_nand_block_markbad;   // need to add nand_get_device()/nand_release_device().
    nand_chip->erase            = mt6573_nand_erase;
    nand_chip->block_bad        = mt6573_nand_block_bad;


	mt6573_nand_init_hw(host);
	/* Select the device */
	nand_chip->select_chip(mtd, 0);

	/*
	 * Reset the chip, required by some chips (e.g. Micron MT29FxGxxxxx)
	 * after power-up
	 */
	nand_chip->cmdfunc(mtd, NAND_CMD_RESET, -1, -1);

	/* Send the command for reading device ID */
	nand_chip->cmdfunc(mtd, NAND_CMD_READID, 0x00, -1);

	/* Read manufacturer and device IDs */
	manu_id = nand_chip->read_byte(mtd);
	dev_id = nand_chip->read_byte(mtd);

	ext_id1 = nand_chip->read_byte(mtd);
	ext_id2 = nand_chip->read_byte(mtd);
	ext_id3 = nand_chip->read_byte(mtd);
    ext_id = ext_id1 << 16 | ext_id2 << 8 | ext_id3;

	//Check NAND Info
	// id = (dev_id<<8)|manu_id;
	id = dev_id | (manu_id << 8);
	if(!get_device_info(id, ext_id, &devinfo))
	{
        MSG(INIT, "Not Support this Device! \r\n");
	}

    if (devinfo.pagesize == 4096) {
        nand_chip->ecc.layout = &nand_oob_128;
    } else if (devinfo.pagesize == 2048) {
        nand_chip->ecc.layout = &nand_oob_64;
    } else if (devinfo.pagesize == 512) {
        nand_chip->ecc.layout = &nand_oob_16;	
    }
    MSG(INIT, "[NAND] pagesz:%d eccsz: %d, oobsz: %d\n", 
        nand_chip->ecc.size, nand_chip->ecc.bytes, sizeof(g_kCMD.au1OOB));

	MSG(INIT, "Support this Device in MTK table! %x \r\n",id);
    hw->nfi_bus_width = devinfo.iowidth;
	DRV_WriteReg32(NFI_ACCCON_REG32, devinfo.timmingsetting);

	/* 16-bit bus width */
	if (hw->nfi_bus_width == 16)
	{
	    MSG(INIT, "%s : Set the 16-bit I/O settings!\n", MODULE_NAME);
		nand_chip->options |= NAND_BUSWIDTH_16;
	}

    /*  register NFI IRQ handler. */
    err = request_irq(MT6573_NFI_IRQ_LINE, (irq_handler_t)mt6573_nand_irq_handler, 0, 
                     "mt6573-nand", NULL);
    if (0 != err) 
	{
        MSG(INIT, "%s : Request IRQ fail: err = %d\n", MODULE_NAME, err);
        goto out;
    }

    if (g_i4Interrupt)
        enable_irq(MT6573_NFI_IRQ_LINE);
    else
        disable_irq(MT6573_NFI_IRQ_LINE);

#if 0
    if (devinfo.advancedmode & CACHE_READ)
    {
        nand_chip->ecc.read_multi_page_cache = NULL;
        // nand_chip->ecc.read_multi_page_cache = mt6573_nand_read_multi_page_cache;
    	// MSG(INIT, "Device %x support cache read \r\n",id);
    }
    else
        nand_chip->ecc.read_multi_page_cache = NULL;
#endif

	/* Scan to find existance of the device */
	if (nand_scan(mtd, hw->nfi_cs_num)) 
	{
		MSG(INIT, "%s : nand_scan fail.\n", MODULE_NAME);
		err = -ENXIO;
		goto out;
	}

	g_page_size = mtd->writesize;
	
	platform_set_drvdata(pdev, host);
    
    if (hw->nfi_bus_width == 16)
	{
		NFI_SET_REG16(NFI_PAGEFMT_REG16, PAGEFMT_DBYTE_EN);
	}

	nand_chip->select_chip(mtd, 0);
  
    nand_chip->chipsize -= (BMT_POOL_SIZE) << nand_chip->phys_erase_shift;
    mtd->size = nand_chip->chipsize;

    if (!g_bmt)
    {
        if ( !(g_bmt = init_bmt(nand_chip, BMT_POOL_SIZE)) )
        {
            MSG(INIT, "Error: init bmt failed\n");
            // ASSERT(0);
            return 0;
        }
    }
	
   nand_chip->chipsize -= (PMT_POOL_SIZE) << nand_chip->phys_erase_shift;
   mtd->size = nand_chip->chipsize;

#ifdef CONFIG_MTD_PARTITIONS

#ifdef PMT

	part_init_pmt(mtd,(u8 *)&g_exist_Partition[0]);
    set_bmt_reserve_region(SET_RESERVE_SET, g_exist_Partition[0].size +  g_exist_Partition[1].size);
	err = add_mtd_partitions(mtd, g_exist_Partition, part_num);

#else

	err = add_mtd_partitions(mtd, g_pasStatic_Partition, part_num);
#endif	


#ifdef CONFIG_MTD_NAND_NVRAM

#endif

#else

	err = add_mtd_device(mtd);

#endif

#ifdef _MTK_NAND_DUMMY_DRIVER_
    dummy_driver_debug = 0;
#endif


	/* Successfully!! */
	if (!err)
	{
        MSG(INIT, "[mt6573_nand] probe successfully!\n");
        nand_disable_clock();
		return err;
	}

	/* Fail!! */
out:
	MSG(INIT, "[NFI] mt6573_nand_probe fail, err = %d!\n", err);
	
	nand_release(mtd);
	
	platform_set_drvdata(pdev, NULL);
	
	kfree(host);

    nand_disable_clock();
	return err;
}
static int mt6573_nand_suspend(struct platform_device *pdev, pm_message_t state)
{
	if (DRV_Reg32(NFI_STA_REG32) & STA_NAND_BUSY)
	{
		MSG(POWERCTL, "[NFI] Busy, Suspend Fail !\n");		
		return 1; // BUSY
	}	

	MSG(POWERCTL, "[NFI] Suspend !\n");
    return 0;
}
static int mt6573_nand_resume(struct platform_device *pdev)
{
	MSG(POWERCTL, "[NFI] Resume !\n");
    return 0;
}

static int __devexit mt6573_nand_remove(struct platform_device *pdev)
{
	struct mt6573_nand_host *host = platform_get_drvdata(pdev);
	struct mtd_info *mtd = &host->mtd;

	nand_release(mtd);

	kfree(host);

    nand_disable_clock();
	
	return 0;
}

#if (NAND_OTP_SUPPORT && SAMSUNG_OTP_SUPPORT)
unsigned int samsung_OTPQueryLength(unsigned int *QLength)
{
    *QLength = SAMSUNG_OTP_PAGE_NUM * g_page_size;
	
    MSG(INIT, "[%s]: libin query here area %d\n", __func__,*QLength);
	return 0;
}

void OTP_IMEI_Read (void *BufferPtr, int num)
{
	int i, j;
	BOOL bValid = true;
	OTP_LAYOUT *otp_layout[2];
	IMEI_SVN imei_svn[2];

	memset(imei_svn[i].imei, 0xff, 2*sizeof(IMEI_SVN));
	
	//MSG(INIT, "[%s]:num=%d\n", __func__, num);
	
	for (i = 0; i<num; i++)
	{
		otp_layout[i] = (OTP_LAYOUT*)((unsigned char*)BufferPtr + i*sizeof(OTP_LAYOUT));

		for (j=0, bValid = true; j<8; j++)
		{
			if (otp_layout[i]->imei[j]+otp_layout[i]->imei_checksum[j] != 0xFF)
			{
				bValid = false;
				MSG(INIT, "[%s]:imei[%d] =%x, imei_checksum =%x \n", __func__, j,otp_layout[i]->imei[j],otp_layout[i]->imei_checksum[j]);
				break;
			}
		}
		if (bValid == true)
		{
			memcpy(imei_svn[i].imei, otp_layout[i]->imei, 8);
			imei_svn[i].svn = 0x01;
			imei_svn[i].pad = 0x00;
		}
	}
	memset(BufferPtr, 0xff, 2*sizeof(OTP_LAYOUT));
	memcpy(BufferPtr, imei_svn, 2*sizeof(IMEI_SVN));
}

void OTP_IMEI_Write (void *BufferPtr, int num)
{
	int i, j;
	OTP_LAYOUT otp_layout[2];
	IMEI_SVN *imei_svn[2];
	BOOL bSkip = true;

	memset(otp_layout, 0xff, 2*sizeof(OTP_LAYOUT));  //no SVN+PAD
	
	//MSG(INIT, "[%s]:num=%d\n", __func__, num);
	
	for (i = 0; i<num; i++)
	{
		imei_svn[i] = (IMEI_SVN*)((unsigned char*)BufferPtr + i*sizeof(IMEI_SVN));

		for (j=0, bSkip=true; j<8; j++)
		{
			if (0xFF != imei_svn[i]->imei[j])
			{
				memcpy(otp_layout[i].imei, imei_svn[i]->imei, 8);
				bSkip = false;              //modem will write imei1, 2 twice
				break;
			}
		}

		if (false == bSkip)
		{
			for (j=0; j<8; j++)
			{
			
			   otp_layout[i].imei_checksum[j] = 0xFF-otp_layout[i].imei[j];
			}
		}
	}

	memcpy(BufferPtr, otp_layout, 2*sizeof(OTP_LAYOUT));
}
/*MICRON_HYNIX_COMPATIBILITY start*/
#define NFI_Wait_Adddress(timeout)              while ( (*NFI_STA_REG32 & STA_ADDR_STATE) && (timeout--) );
#define NFI_FIFO_Empty(timeout)                         while ( !(*NFI_FIFOSTA_REG16 & FIFO_WR_EMPTY) && (timeout--) );
#define PAGEFMT_8BITS        (0x0000)
#define PAGEFMT_16BITS       (0x0008)
/*MICRON_HYNIX_COMPATIBILITY end*/

void OTP_PostProcess (unsigned int PageAddr, void *BufferPtr, E_RW bRW)
{
	//MSG(INIT, "[%s]:PageAddr=%x, bRW=%d\n", __func__, PageAddr, bRW);
	
	if (0 != PageAddr) return;
	
	if (E_READ == bRW)
	{
		OTP_IMEI_Read (BufferPtr, 2);

	} else
	{
		OTP_IMEI_Write (BufferPtr, 2);
	}
}

/*MICRON_HYNIX_COMPATIBILITY start*/
enum OTP_OP
{
OTP_OP_MIN = 0x0,
OTP_EXIT = OTP_OP_MIN,
OTP_ENTER = 0x1,
OTP_OP_MAX = OTP_ENTER
};
/*MICRON_HYNIX_COMPATIBILITY end*/

/******porting form DA src codes*******/
/*MICRON_HYNIX_COMPATIBILITY start*/
#define STATUS_READY                    (0x40)
#define STATUS_FAIL                             (0x01)
#define STATUS_WR_ALLOW                 (0x80)

#define CON_NOB_SHIFT   (5)

#define FIFO_PIO_READY(x)  (0x1 & x)

#define WAIT_NFI_PIO_READY(timeout) \
    do {\
    while( (!FIFO_PIO_READY(DRV_Reg(NFI_PIO_DIRDY_REG16))) && (--timeout) );\
    if(timeout == 0)\
        {\
                MSG(INIT, "Error: PIO_READY timeout at line=%d\n", __LINE__);\
        }\
    } while(0);

//------------------------------------------------------------------------------
// Read Status Callback Function                                                
//------------------------------------------------------------------------------
static bool mt6573_nand_read_status(const uint32_t con_timeout)
{
    int status, i;
    mt6573_nand_reset();
    unsigned int timeout;

    mt6573_nand_reset();

    /* Disable HW ECC */
    NFI_CLN_REG16 (NFI_CNFG_REG16, CNFG_HW_ECC_EN);

    /* Disable 16-bit I/O */
    NFI_CLN_REG16 (NFI_PAGEFMT_REG16, PAGEFMT_DBYTE_EN);
    NFI_SET_REG16 (NFI_CNFG_REG16, CNFG_OP_SRD | CNFG_READ_EN | CNFG_BYTE_RW);

    DRV_WriteReg16(NFI_CON_REG16, CON_NFI_SRD | (1 << CON_NOB_SHIFT));

    DRV_WriteReg16(NFI_CON_REG16, 0x3);
    mt6573_nand_set_mode(CNFG_OP_SRD);
    DRV_WriteReg16(NFI_CNFG_REG16, 0x2042);
    mt6573_nand_set_command(NAND_CMD_STATUS);
    DRV_WriteReg16(NFI_CON_REG16, 0x90);

    timeout = con_timeout;
    WAIT_NFI_PIO_READY(timeout);

    if(timeout)
    {
        status = (DRV_Reg16(NFI_DATAR_REG32));
    }

    //~  clear NOB
    DRV_WriteReg16(NFI_CON_REG16, 0);

    if (devinfo.iowidth == NAND_BUSWIDTH_16)
    {
        NFI_SET_REG16 (NFI_PAGEFMT_REG16, PAGEFMT_DBYTE_EN);
        NFI_CLN_REG16 (NFI_CNFG_REG16, CNFG_BYTE_RW);
    }

    // check READY/BUSY status first 
    if( !(STATUS_READY&status) ) {
        // MSG(ERR, "status is not ready\n");  
    }

    // flash is ready now, check status code
    if( STATUS_FAIL & status ) {
        if( !(STATUS_WR_ALLOW&status) ) {
            // MSG(INIT, "status locked\n");
            return FALSE;
        }
        else {
            // MSG(INIT, "status unknown\n");
            return FALSE;
        }
    }
    else {
        return TRUE;
    }
}

void micron_get_features(const uint32_t timeout)
{
        uint32_t u4_timeout = timeout;
        uint16_t nfi_pagefmt;
        uint32_t Readback = 0;
        uint8_t read_data[4], i = 0;

        mt6573_nand_reset();

        DRV_WriteReg16(NFI_CNFG_REG16, CNFG_READ_EN | CNFG_OP_SRD | CNFG_BYTE_RW);

        nfi_pagefmt = DRV_Reg16(NFI_PAGEFMT_REG16);
        DRV_WriteReg16(NFI_PAGEFMT_REG16,((nfi_pagefmt&(~PAGEFMT_16BITS))|PAGEFMT_8BITS));

        (void)mt6573_nand_set_command(0xEE);

        DRV_WriteReg32(NFI_ROWADDR_REG32,0x0);
        DRV_WriteReg32(NFI_COLADDR_REG32,0x90);
        DRV_WriteReg16(NFI_ADDRNOB_REG16,0x1);

        u4_timeout = timeout;
        NFI_Wait_Adddress(u4_timeout);
        if(!u4_timeout) {return OTP_ERROR_TIMEOUT;}

        DRV_WriteReg16(NFI_CON_REG16,(CON_NFI_SRD | (4 << CON_NFI_NOB_SHIFT))); /*DA codes*/

        u4_timeout = timeout;
        while((FIFO_RD_REMAIN(DRV_Reg16(NFI_FIFOSTA_REG16)) <= 4) && (--u4_timeout))    /*4  10?*/
          {
           if(timeout){
                read_data[i] = DRV_Reg16(NFI_DATAR_REG32);
//              MSG(INIT, "jrd_enter %s: read_data[%d]: %d!\n",__func__, i, read_data[i]);
                i++;
                if(i == 4)
                        break;
                }
          }

//      mt6573_nand_read_status(0xffff);        
}

uint32_t OTP_Enter_Exit(const uint32_t timeout, enum OTP_OP op)
{
        uint32_t u4_timeout = timeout;
        uint16_t nfi_pagefmt, i=0;
        //uint8 otp_op = op;
        uint32_t readback = 0;  /*micron get feature P1--P4*/
        const uint8_t data[4] = {op, 0x00, 0x00, 0x00}; //enter OTP mode parameters.

        if(op < OTP_OP_MIN || op > OTP_OP_MAX)
        {
                return OTP_INVALID_OP;
        }

        mt6573_nand_reset();

        /*disable HW ECC copy from read id*/
            NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
            NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);

        nfi_pagefmt = DRV_Reg16(NFI_PAGEFMT_REG16);
        DRV_WriteReg16(NFI_PAGEFMT_REG16,((nfi_pagefmt&(~PAGEFMT_16BITS))|PAGEFMT_8BITS));      /*0x00 every other byte*/

 /*read id setting*/
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_READ_EN);
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);
        mt6573_nand_reset();

        mt6573_nand_set_mode(CNFG_OP_CUST);

        DRV_WriteReg16(NFI_CON_REG16,0x0001);
        NFI_FIFO_Empty(u4_timeout);
        if(!u4_timeout) {return OTP_ERROR_TIMEOUT;}

        DRV_WriteReg16(NFI_CON_REG16,(1 << CON_NFI_SEC_SHIFT));
        DRV_WriteReg16(NFI_STRDATA_REG16,0x0001);

//        MSG(INIT, "jrd_enter 1 [%s]: call dump_nfi() before set command\n",__func__, op);
//      dump_nfi();
        (void)mt6573_nand_set_command(0xEF);

        DRV_WriteReg32(NFI_ROWADDR_REG32,0x0);
        DRV_WriteReg32(NFI_COLADDR_REG32,0x90);
        DRV_WriteReg16(NFI_ADDRNOB_REG16,0x1);

        u4_timeout = timeout;
        NFI_Wait_Adddress(u4_timeout);
        if(!u4_timeout) {return OTP_ERROR_TIMEOUT;}

        DRV_WriteReg16(NFI_CON_REG16,(CON_NFI_BWR | (1 << CON_NFI_SEC_SHIFT)));
        DRV_WriteReg16(NFI_STRDATA_REG16,0x1);

        u4_timeout = timeout;
        while((FIFO_WR_REMAIN( DRV_Reg16(NFI_FIFOSTA_REG16)) <= 12) && (--u4_timeout) )
        {
//      MSG(INIT, "jrd_enter %s: write data[%d] :0x%x!\n",__func__,i, data[i]);
                DRV_WriteReg32(NFI_DATAW_REG32,data[i++]);
                if(i == 4)
                {
                        break;
                }
        }
#if 0   /*original check status func*/
        if (!mt6573_nand_status_ready(STA_NAND_BUSY))
    {
        return OTP_ERROR_BUSY;
    }

#else   /*porting form DA src code*/
        if(!mt6573_nand_read_status(0xffff))
                return OTP_ERROR_TIMEOUT;

        /*run get feature, i am not sure it's necessory or not */
        micron_get_features(0xffff);
#endif

        DRV_WriteReg16(NFI_PAGEFMT_REG16,nfi_pagefmt);

        return OTP_SUCCESS;

}

/*****porting form DA codes *****/
void NUTL_FIFO_Read_V3(const uint32_t c_timeout, uint32_t *p_data32, const uint32_t data_len)
{
        uint32_t        timeout = c_timeout;
        uint32_t        i;
        uint32_t        sectors;
        uint32_t        sector_page_size32;
        uint32_t        sector_spare_size32;
        uint32_t        sector_pagespare_size32;
        uint32_t        dec_mask;
        uint32_t        ErrNum;
        uint32_t        ErrBitLoc;

        uint32_t        sector;

        uint32_t        nand_sector_size = 512;
        uint32_t        nand_spare_size = 64;

        sectors = g_page_size/nand_sector_size; /*4. DA:                        = NUTL_PAGE_SIZE()/NAND_SECTOR_SIZE;*/
        sector_page_size32 = (g_page_size/sectors) >> 2;        /*128. DA:      = (NUTL_PAGE_SIZE()/sectors)>>2;*/
        sector_spare_size32 = (nand_spare_size/sectors)>>2;             /*4. ???*/
        sector_pagespare_size32 = sector_page_size32 + sector_spare_size32;


        MSG(INIT, "jrd_enter %s now !\n",__func__);
//      dump_nfi();

        /*NFI mode  -- dword access mode*/
        /*read page data*/
        for(sector = 0; sector < sectors; sector++);    /*4 */
        {
                /*1. Read main area per sector*/
            for(i = 0; i < sector_page_size32; i++)
                {
                    timeout = c_timeout;
                    WAIT_NFI_PIO_READY(timeout);
                    if(!timeout)
                        return OTP_ERROR_TIMEOUT;

                    *(p_data32 + sector_pagespare_size32*sector + i) = DRV_Reg32(NFI_DATAR_REG32);
                }

           /*2. read spare area per sector*/
            for(i = 0; i < sector_spare_size32; i++)
                {
                    timeout = c_timeout;
                    WAIT_NFI_PIO_READY(timeout);
                    if(!timeout)
                        return OTP_ERROR_TIMEOUT;

                    *(p_data32 + sector_pagespare_size32*sector + sector_page_size32 +i) = DRV_Reg32(NFI_DATAR_REG32);
                }

#if 0   /*DA codes */
            dec_mask = 1 <<  sector;
            while(!(dec_mask & (DRV_Reg(ECC_DECDONE_REG16))))   
                ;
        
            ErrNum = ((DRV_Reg(ECC_DECENUM_REG32)) >> (4 * sector)) & 0xF;
            if(ErrNum && ErrNum != 0xF) 
                {
                  for(i = 0; i < ErrNum; i++)
                        {
                          ErrBitLoc = DRV_Reg(ECC_DECELO_REG32 + i/2);
                        }
                }

            ECC_Decode_end();   
#else   /*replace with this func ??*/
            mt6573_nand_check_dececc_done(sector);
#endif
        }
}
/*MICRON_HYNIX_COMPATIBILITY end*/

unsigned int samsung_OTPRead(unsigned int PageAddr, void *BufferPtr, void *SparePtr)
{
    struct mtd_info *mtd = &host->mtd;
    unsigned int rowaddr, coladdr;
    unsigned int u4Size = g_page_size;
    unsigned int timeout = 0xFFFF;
    unsigned int bRet;
    unsigned int sec_num = mtd->writesize >> 9;

/*MICRON_HYNIX_COMPATIBILITY start*/
    uint8_t *buf;

    uint16_t nfi_pagefmt;               /*jrd_add for debug page format*/
    bool custom_mode = 0;               /*jrd_add for read otp in custom mode*/
    bool bUsingDMA = 1;

    uint32_t i = 0;
    int manufacture_id, device_id;
/*MICRON_HYNIX_COMPATIBILITY end*/

    if(PageAddr >= SAMSUNG_OTP_PAGE_NUM)
    {
        return OTP_ERROR_OVERSCOPE;
    }

/*MICRON_HYNIX_COMPATIBILITY start*/
  if((manu_id == 0xad) && (dev_id == 0xbc))	/*Hynix flash*/
   {	
/*MICRON_HYNIX_COMPATIBILITY end*/
    /* Col -> Row; LSB first */
    coladdr = 0x00000000;
  //  rowaddr = Samsung_OTP_Page[PageAddr];
    rowaddr = 0x20 + PageAddr;//libin changed
    MSG(INIT, "[%s]:(COLADDR) [0x%08x]/(ROWADDR)[0x%08x]\n", __func__, coladdr, rowaddr);

	nand_get_device((struct nand_chip *)mtd->priv, mtd, FL_READING);
    /* Power on NFI HW component. */
    //nand_enable_clock();	/*del for enter deep idle mode when playing mp3*/

	mt6573_nand_reset();
    (void)mt6573_nand_set_command(0x04);//lbin changed
    // mt6573_nand_reset();
    (void)mt6573_nand_set_command(0x19);//llibin changed

    MSG(INIT, "[%s]: Start to read data from OTP area\n", __func__);

    mt6573_nand_set_mode(CNFG_OP_READ);
    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_READ_EN);
    DRV_WriteReg16(NFI_CON_REG16, sec_num << CON_NFI_SEC_SHIFT);

    DRV_WriteReg32(NFI_STRADDR_REG32, __virt_to_phys(BufferPtr)); 
    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_AHB);

    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    mt6573_nand_set_autoformat(true);
    ECC_Decode_Start();

    if (!mt6573_nand_set_command(NAND_CMD_READ0))
    {
        bRet = OTP_ERROR_BUSY;
        goto cleanup;
    }

    if (!mt6573_nand_set_address(coladdr, rowaddr, 2, 3))
    {
        bRet = OTP_ERROR_BUSY;
        goto cleanup;
    }

    if (!mt6573_nand_set_command(NAND_CMD_READSTART))
    {
        bRet = OTP_ERROR_BUSY;
        goto cleanup;
    }

    if (!mt6573_nand_status_ready(STA_NAND_BUSY))
    {
        bRet = OTP_ERROR_BUSY;
        goto cleanup;
    }

    MSG(INIT, "[%s]:mt6573_nand_read_page_data \n", __func__);
    if (!mt6573_nand_read_page_data(mtd, BufferPtr, u4Size))
    {
    MSG(INIT, "[%s]:mt6573_nand_read_page_data fails \n", __func__);
        bRet = OTP_ERROR_BUSY;
        goto cleanup;
    }

    if (!mt6573_nand_status_ready(STA_NAND_BUSY))
    {
        bRet = OTP_ERROR_BUSY;
        goto cleanup;
    }

    mt6573_nand_read_fdm_data(SparePtr, sec_num);

    mt6573_nand_stop_read();

    MSG(INIT, "[%s]: End to read data from OTP area\n", __func__);
/*MICRON_HYNIX_COMPATIBILITY start*/
  }else if((manu_id == 0x2c)&&(dev_id == 0xbc))	/*MT29F4G16ABBDA*/
  {
    /* Col -> Row; LSB first */
    coladdr = 0x00000000;
  //  rowaddr = Samsung_OTP_Page[PageAddr];
    rowaddr = 0x02 + PageAddr;//libin changed
    MSG(INIT, "[%s]:(h2-COLADDR) [0x%08x]/(ROWADDR)[0x%08x]\n", __func__, coladdr, rowaddr);


    if (((u32)BufferPtr % 16) && local_buffer_16_align)
        {
         printk(KERN_INFO "jrd_enter Data buffer not 16 bytes aligned: %p..............\n", BufferPtr);
        buf = local_buffer_16_align;
    }
    else
        buf = (uint8_t *)BufferPtr;

        nand_get_device((struct nand_chip *)mtd->priv, mtd, FL_READING);
    /* Power on NFI HW component. */

    mt6573_nand_reset();
    OTP_Enter_Exit(0xFFFF,OTP_ENTER);

//   nand_enable_clock();       /*jrd_del*/

    mt6573_nand_reset();

//    MSG(INIT, "[%s]: jrd_enter Start to read data from OTP area,sec_num:%d!\n", __func__,sec_num);

//    nfi_pagefmt = DRV_Reg16(NFI_PAGEFMT_REG16);
//    DRV_WriteReg16(NFI_PAGEFMT_REG16,((nfi_pagefmt&(~PAGEFMT_16BITS))|PAGEFMT_8BITS));        /*0x00 every other byte ?*/

    /* Enable HW ECC */
    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);        /*disable byte access*/

    mt6573_nand_set_mode(CNFG_OP_READ);
    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_READ_EN);

//      DRV_WriteReg16(NFI_CNFG_REG16, (CNFG_READ_EN| CNFG_OP_READ | CNFG_HW_ECC_EN));  /*or CNFG_OP_READ--> CNFG_OP_CUST ????*/

    if(bUsingDMA)
        {

        /*jrd_del for compatible with DA codes , enable when dma read*/
            DRV_WriteReg16(NFI_CON_REG16, sec_num << CON_NFI_SEC_SHIFT);                /*?????????*/
//          DRV_WriteReg16(NFI_INTR_EN_REG16, INTR_BSY_RTN_EN);         /*jrd_add NFI_INTR_EN_REG16 = 0x0010*/
        }

    NFI_SET_REG16(NFI_CNFG_REG16, (bUsingDMA)?CNFG_AHB:0);
        DRV_WriteReg32(NFI_STRADDR_REG32, __virt_to_phys(buf));   /*compatible with DA codes*/
    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    mt6573_nand_set_autoformat(true);       /*CNFG_AUTO_FMT_EN*/

    ECC_Encode_End();                               /*jrd_add make sure encoder is close*/
    ECC_Decode_Start();                             /*jrd_add enable decoder*/

#if 0   /*jrd_add for debug*/
        MSG(INIT, "jrd_enter %s call dump_nfi before set cmd!\n",__func__);
        dump_nfi();
#endif

    if (!mt6573_nand_set_command(NAND_CMD_READ0))
    {
        bRet = OTP_ERROR_BUSY;
        goto cleanup;
    }

    if (!mt6573_nand_set_address(coladdr, rowaddr, 2, 3))
    {
        bRet = OTP_ERROR_BUSY;
        goto cleanup;
    }

    if (!mt6573_nand_set_command(NAND_CMD_READSTART))
    {
        bRet = OTP_ERROR_BUSY;
        goto cleanup;
    }

#if 1   /*for compatible with DA codes*/    
//    if (!mt6573_nand_status_ready(STA_NAND_BUSY_RETURN))
    if (!mt6573_nand_status_ready(STA_NAND_BUSY))
    {
        bRet = OTP_ERROR_BUSY;
        goto cleanup;
    }
#endif

#if 0
    DRV_WriteReg16(NFI_CMD_REG16, NAND_CMD_STATUS);     

    if(!mt6573_nand_status_ready(NAND_CMD_STATUS))
    {
        bRet = OTP_ERROR_BUSY;
        goto cleanup;
    }   

#endif
//    DRV_WriteReg16(NFI_CNRNB_REG16, 0xf1);      /*jrd_add NFI check nand ready/busy register*/

    if(!bUsingDMA)
        {
        /*disable when use dma read*/
          DRV_WriteReg16(NFI_CON_REG16, CON_NFI_BRD | (sec_num << CON_NFI_SEC_SHIFT));  /*mt6573 p495*/

            if (!mt6573_nand_status_ready(STA_NAND_BUSY))
            {
                bRet = OTP_ERROR_BUSY;
                goto cleanup;
            }
        }

//        DRV_WriteReg16(NFI_PAGEFMT_REG16,nfi_pagefmt);                /*jrd_add */

    MSG(INIT, "[%s]:mt6573_nand_read_page_data \n", __func__);
    if(bUsingDMA)
        {
//          if (!mt6573_nand_read_page_data(mtd, BufferPtr, u4Size))
            if (!mt6573_nand_read_page_data(mtd, buf, u4Size))
            {
            MSG(INIT, "[%s]:mt6573_nand_read_page_data fails \n", __func__);
                bRet = OTP_ERROR_BUSY;
                goto cleanup;
            }
        }
   else
        NUTL_FIFO_Read_V3(timeout, BufferPtr, g_page_size+64);

    if (!mt6573_nand_status_ready(STA_NAND_BUSY))
    {
        bRet = OTP_ERROR_BUSY;
        goto cleanup;
    }

#if 0    
    if(!mt6573_nand_check_dececc_done(sec_num))
        {
            bRet = false;
        }
#endif
//    nfi_pagefmt = DRV_Reg16(NFI_PAGEFMT_REG16);
//    DRV_WriteReg16(NFI_PAGEFMT_REG16,((nfi_pagefmt&(~PAGEFMT_8BITS))|PAGEFMT_16BITS));

    mt6573_nand_read_fdm_data(SparePtr, sec_num);

#if 0
        for(i = 0; i < 128; i= i + 4)
           printk(KERN_INFO "jrd_enter %s: buf[%d]--[%d]:0x%x,0x%x,0x%x,0x%x!\n",
                                        __func__,i,i+3, buf[i], buf[i+1], buf[i+2], buf[i+3]);
#endif

#if 0
    if(!mt6573_nand_check_bch_error(mtd, buf, sec_num - 1, rowaddr))
        {
         bRet = false;
        }
#endif
    mt6573_nand_stop_read();

    OTP_Enter_Exit(0xFFFF,OTP_EXIT);
    MSG(INIT, "[%s]: End to read data from OTP area\n", __func__);

    if (buf == local_buffer_16_align)
        memcpy(BufferPtr, buf, u4Size);
  }
/*MICRON_HYNIX_COMPATIBILITY end*/

    bRet = OTP_SUCCESS;

	OTP_PostProcess (PageAddr, BufferPtr, E_READ);
cleanup:

	mt6573_nand_reset();
    (void)mt6573_nand_set_command(0xFF);

	nand_release_device(mtd);
	return bRet;
}

unsigned int samsung_OTPWrite(unsigned int PageAddr, void *BufferPtr, void *SparePtr)
{
    struct mtd_info *mtd = &host->mtd;
    unsigned int rowaddr, coladdr;
    unsigned int u4Size = g_page_size;
    unsigned int timeout = 0xFFFF;
    unsigned int bRet;
    unsigned int sec_num = mtd->writesize >> 9;

    if(PageAddr >= SAMSUNG_OTP_PAGE_NUM)
    {
        return OTP_ERROR_OVERSCOPE;
    }
/*MICRON_HYNIX_COMPATIBILITY start*/
 if((manu_id == 0xad)&&(dev_id == 0xbc))	/*Hynix flash*/
  { 
/*MICRON_HYNIX_COMPATIBILITY end*/
	OTP_PostProcess (PageAddr, BufferPtr, E_WRITE);
    /* Col -> Row; LSB first */
    coladdr = 0x00000000;
   // rowaddr = Samsung_OTP_Page[PageAddr];
    rowaddr = 0x20+ PageAddr;//libin changed

    MSG(INIT, "[%s]:(COLADDR) [0x%08x]/(ROWADDR)[0x%08x]\n", __func__, coladdr, rowaddr);
	nand_get_device((struct nand_chip *)mtd->priv, mtd, FL_READING);
	mt6573_nand_reset();
    (void)mt6573_nand_set_command(0x04);//libin
    mt6573_nand_reset();
    (void)mt6573_nand_set_command(0x19);//libin changed for hynix

    MSG(INIT, "[%s]: Start to write data to OTP area\n", __func__);

    /* Power on NFI HW component. */
    //nand_enable_clock();	/*del for enter deep idle mode when playing mp3*/

    if (!mt6573_nand_reset())
    {
		bRet = OTP_ERROR_RESET;
        goto cleanup;
	}

	mt6573_nand_set_mode(CNFG_OP_PRGM);

	NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_READ_EN);

	DRV_WriteReg16(NFI_CON_REG16, sec_num << CON_NFI_SEC_SHIFT);

    DRV_WriteReg32(NFI_STRADDR_REG32, __virt_to_phys(BufferPtr)); 
	NFI_SET_REG16(NFI_CNFG_REG16, CNFG_AHB);

	NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);

	mt6573_nand_set_autoformat(true);

	ECC_Encode_Start();

	if (!mt6573_nand_set_command(NAND_CMD_SEQIN))
    {
        bRet = OTP_ERROR_BUSY;
		goto cleanup;
	}

	if (!mt6573_nand_set_address(coladdr, rowaddr, 2, 3))
    {
        bRet = OTP_ERROR_BUSY;
		goto cleanup;
	}

	if (!mt6573_nand_status_ready(STA_NAND_BUSY))
    {
        bRet = OTP_ERROR_BUSY;
		goto cleanup;
	}

	mt6573_nand_write_fdm_data((struct nand_chip *)mtd->priv, BufferPtr, sec_num);
	(void)mt6573_nand_write_page_data(mtd, BufferPtr, u4Size);
	if(!mt6573_nand_check_RW_count(u4Size))
    {
        MSG(INIT, "[%s]: Check RW count timeout !\n", __func__);
        bRet = OTP_ERROR_TIMEOUT;
        goto cleanup;
    }

	mt6573_nand_stop_write();
    (void)mt6573_nand_set_command(NAND_CMD_PAGEPROG);
	while(DRV_Reg32(NFI_STA_REG32) & STA_NAND_BUSY);

    bRet = OTP_SUCCESS;
  }else if((manu_id == 0x2c)&&(dev_id == 0xbc))	/*Micro flash*/
  {
        OTP_PostProcess (PageAddr, BufferPtr, E_WRITE);
    /* Col -> Row; LSB first */
    coladdr = 0x00000000;
   // rowaddr = Samsung_OTP_Page[PageAddr];
//    rowaddr = 0x02 + PageAddr;//libin changed
    rowaddr = 0x02 + PageAddr;//libin changed

    MSG(INIT, "[%s]:(w02-COLADDR) [0x%08x]/(ROWADDR)[0x%08x]\n", __func__, coladdr, rowaddr);
        nand_get_device((struct nand_chip *)mtd->priv, mtd, FL_READING);
        mt6573_nand_reset();
    //(void)mt6573_nand_set_command(0x04);//libin
    //mt6573_nand_reset();
    //(void)mt6573_nand_set_command(0x19);//libin changed for hynix
        OTP_Enter_Exit(0xFFFF,OTP_ENTER);

    MSG(INIT, "[%s]: Start to write data to OTP area\n", __func__);

    /* Power on NFI HW component. */
    //nand_enable_clock();      /*del for enter deep idle mode when playing mp3*/

    if (!mt6573_nand_reset())
    {
                bRet = OTP_ERROR_RESET;
        goto cleanup;
        }
        //mt6573_nand_set_mode(CNFG_OP_PRGM);
        DRV_WriteReg16(NFI_CNFG_REG16, CNFG_OP_PRGM | CNFG_HW_ECC_EN | CNFG_AUTO_FMT_EN);

        //NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_READ_EN);

        DRV_WriteReg16(NFI_CON_REG16, sec_num << CON_NFI_SEC_SHIFT);

    DRV_WriteReg32(NFI_STRADDR_REG32, __virt_to_phys(BufferPtr));
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_AHB);

        //NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);

        mt6573_nand_set_autoformat(true);

        ECC_Encode_Start();

        if (!mt6573_nand_set_command(NAND_CMD_SEQIN))
    {
        bRet = OTP_ERROR_BUSY;
                goto cleanup;
        }

        if (!mt6573_nand_set_address(coladdr, rowaddr, 2, 3))
    {
        bRet = OTP_ERROR_BUSY;
                goto cleanup;
        }
        if (!mt6573_nand_status_ready(STA_NAND_BUSY))
    {
        bRet = OTP_ERROR_BUSY;
                goto cleanup;
        }

        mt6573_nand_write_fdm_data((struct nand_chip *)mtd->priv, BufferPtr, sec_num);
        (void)mt6573_nand_write_page_data(mtd, BufferPtr, u4Size);
        if(!mt6573_nand_check_RW_count(u4Size))
    {
        MSG(INIT, "[%s]: Check RW count timeout !\n", __func__);
        bRet = OTP_ERROR_TIMEOUT;
        goto cleanup;
    }

        mt6573_nand_stop_write();
    (void)mt6573_nand_set_command(NAND_CMD_PAGEPROG);
        while(DRV_Reg32(NFI_STA_REG32) & STA_NAND_BUSY);

    bRet = OTP_SUCCESS;
        OTP_Enter_Exit(0xFFFF,OTP_EXIT);
  }

    MSG(INIT, "[%s]: End to write data to OTP area\n", __func__);

cleanup:
    mt6573_nand_reset();
    (void)mt6573_nand_set_command(0xFF);
	nand_release_device(mtd);
    return bRet;
}
#endif

#if NAND_OTP_SUPPORT
static int mt_otp_open(struct inode *inode, struct file *filp)
{
	MSG(INIT, "[%s]:(MAJOR)%d:(MINOR)%d\n", __func__, MAJOR(inode->i_rdev), MINOR(inode->i_rdev));
	filp->private_data = (int*)OTP_MAGIC_NUM;
	return 0;
}

static int mt_otp_release(struct inode *inode, struct file *filp)
{
	MSG(INIT, "[%s]:(MAJOR)%d:(MINOR)%d\n", __func__, MAJOR(inode->i_rdev), MINOR(inode->i_rdev));
	return 0;
}

static int mt_otp_access(unsigned int access_type, unsigned int offset, void *buff_ptr, unsigned int length, unsigned int *status)
{
    unsigned int i = 0, ret = 0;
    char *BufAddr = (char *)buff_ptr;
    unsigned int PageAddr, AccessLength=0;
    int Status = 0;

    static char *p_D_Buff = NULL;
    char S_Buff[64];

    if (!(p_D_Buff = kmalloc(g_page_size, GFP_KERNEL)))
    {
        ret = -ENOMEM;
        *status = OTP_ERROR_NOMEM;
        goto exit;
    }

    MSG(INIT, "[%s]: %s (0x%x) length:(%d bytes) !\n", __func__, access_type?"WRITE":"READ", offset, length);

    while(1)
    {
        PageAddr = offset/g_page_size;
        if(FS_OTP_READ == access_type)
        {
            memset(p_D_Buff, 0xff, g_page_size);
            memset(S_Buff, 0xff, (sizeof(char)*64));

            MSG(INIT, "[%s]: Read Access of page (%d)\n",__func__, PageAddr);

            Status = g_mt6573_otp_fuc.OTPRead(PageAddr, p_D_Buff, &S_Buff);
            *status = Status;

            if( OTP_SUCCESS != Status)
            {
                MSG(INIT, "[%s]: Read status (%d)\n", __func__, Status);
                break;
            }

            AccessLength = g_page_size - (offset % g_page_size);

            if(length >= AccessLength)
            {
                memcpy(BufAddr, (p_D_Buff+(offset % g_page_size)), AccessLength);
            }
            else
            {
                //last time
                memcpy(BufAddr, (p_D_Buff+(offset % g_page_size)), length);
            }
        }
        else if(FS_OTP_WRITE == access_type)
        {
            AccessLength = g_page_size - (offset % g_page_size);
            memset(p_D_Buff, 0xff, g_page_size);
            memset(S_Buff, 0xff, (sizeof(char)*64));

            if(length >= AccessLength)
            {
                memcpy((p_D_Buff+(offset % g_page_size)), BufAddr, AccessLength);
            }
            else
            {
                //last time
                memcpy((p_D_Buff+(offset % g_page_size)), BufAddr, length);
            }

            Status = g_mt6573_otp_fuc.OTPWrite(PageAddr, p_D_Buff, &S_Buff);
            *status = Status;

            if( OTP_SUCCESS != Status)
            {
                MSG(INIT, "[%s]: Write status (%d)\n",__func__, Status);
                break;
            }
        }
        else
        {
            MSG(INIT, "[%s]: Error, not either read nor write operations !\n",__func__);
            break;
        }

        offset += AccessLength;
        BufAddr += AccessLength;
        if(length <= AccessLength)
        {
            length = 0;
            break;
        }
        else
        {
            length -= AccessLength;
            MSG(INIT, "[%s]: Remaining %s (%d) !\n",__func__, access_type?"WRITE":"READ", length);
        }
    }
error:
    kfree(p_D_Buff);
exit:
    return ret;
}

static long mt_otp_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0, i=0;
	static char *pbuf = NULL;

	void __user *uarg = (void __user *)arg;
    struct otp_ctl otpctl;

    /* Lock */
    // spin_lock(&g_OTPLock);
    if(down_interruptible(&g_OTPLock))
    {
		MSG(INIT, "ERROR: get mutex failed\n");
		return -EFAULT;;
    }
	if (copy_from_user(&otpctl, uarg, sizeof(struct otp_ctl)))
	{
        ret = -EFAULT;
        goto exit;
    }

    if(false == g_bInitDone)
    {
        MSG(INIT, "ERROR: NAND Flash Not initialized !!\n");
        ret = -EFAULT;
        goto exit;
    }

    if (!(pbuf = kmalloc(sizeof(char)*otpctl.Length, GFP_KERNEL)))
    {
        ret = -ENOMEM;
        goto exit;
    }

	switch (cmd)
    {
	case OTP_GET_LENGTH:
        MSG(INIT, "OTP IOCTL: OTP_GET_LENGTH\n");
        g_mt6573_otp_fuc.OTPQueryLength(&otpctl.QLength);
        otpctl.status = OTP_SUCCESS;
        MSG(INIT, "OTP IOCTL: The Length is %d\n", otpctl.QLength);
        break;
    case OTP_READ:
        MSG(INIT, "OTP IOCTL: OTP_READ Offset(0x%x), Length(0x%x) \n", otpctl.Offset, otpctl.Length);
        memset(pbuf, 0xff, sizeof(char)*otpctl.Length);

        mt_otp_access(FS_OTP_READ, otpctl.Offset, pbuf, otpctl.Length, &otpctl.status);

        if (copy_to_user(otpctl.BufferPtr, pbuf, (sizeof(char)*otpctl.Length)))
        {
            MSG(INIT, "OTP IOCTL: Copy to user buffer Error !\n");
            goto error;
        }
        break;
    case OTP_WRITE:
        MSG(INIT, "OTP IOCTL: OTP_WRITE Offset(0x%x), Length(0x%x) \n", otpctl.Offset, otpctl.Length);
        if (copy_from_user(pbuf, otpctl.BufferPtr, (sizeof(char)*otpctl.Length)))
        {
            MSG(INIT, "OTP IOCTL: Copy from user buffer Error !\n");
            goto error;
        }
        mt_otp_access(FS_OTP_WRITE, otpctl.Offset , pbuf, otpctl.Length, &otpctl.status);
        break;
	default:
		ret = -EINVAL;
	}

    ret = copy_to_user(uarg, &otpctl, sizeof(struct otp_ctl));

error:
    kfree(pbuf);
exit:
    up(&g_OTPLock);
    return ret;
}

static struct file_operations nand_otp_fops = {
    .owner=      THIS_MODULE,
    .ioctl=      mt_otp_ioctl,
    .open=       mt_otp_open,
    .release=    mt_otp_release,
};

static struct miscdevice nand_otp_dev = {
    .minor   = MISC_DYNAMIC_MINOR,
    .name    = "otp",
    .fops    = &nand_otp_fops,
};
#endif

static struct platform_driver mt6573_nand_driver = {
	.probe		= mt6573_nand_probe,
	.remove		= mt6573_nand_remove,
	.suspend	= mt6573_nand_suspend,
	.resume	    = mt6573_nand_resume,
	.driver		= {
		.name	= "mt6573-nand",
		.owner	= THIS_MODULE,
	},
};

static int __init mt6573_nand_init(void)
{
	struct proc_dir_entry *entry;
	
//#ifdef CONFIG_MTK_MTD_NAND_INTERRUPT_SCHEME	
    // boot up using polling mode
	g_i4Interrupt = 0;      
//#else
//	g_i4Interrupt = 0;
//#endif

#if NAND_OTP_SUPPORT
    int err = 0;
    MSG(INIT, "OTP: register NAND OTP device ...\n");
	err = misc_register(&nand_otp_dev);
	if (unlikely(err))
    {
		MSG(INIT, "OTP: failed to register NAND OTP device!\n");
		return err;
	}
	// spin_lock_init(&g_OTPLock);
#endif

#if (NAND_OTP_SUPPORT && SAMSUNG_OTP_SUPPORT)
    g_mt6573_otp_fuc.OTPQueryLength = samsung_OTPQueryLength;
    g_mt6573_otp_fuc.OTPRead = samsung_OTPRead;
    g_mt6573_otp_fuc.OTPWrite = samsung_OTPWrite;
#endif



	entry = create_proc_entry(PROCNAME, 0666, NULL);
	if (entry == NULL) 
	{
		MSG(INIT, "MT6573 Nand : unable to create /proc entry\n");
		return -ENOMEM;
	}
	entry->read_proc = mt6573_nand_proc_read;
	entry->write_proc = mt6573_nand_proc_write;	
	//entry->owner = THIS_MODULE;

	MSG(INIT, "MediaTek MT6573 Nand driver init, version %s\n", VERSION);

	return platform_driver_register(&mt6573_nand_driver);
}

static void __exit mt6573_nand_exit(void)
{
	MSG(INIT, "MediaTek MT6573 Nand driver exit, version %s\n", VERSION);
#if NAND_OTP_SUPPORT
	misc_deregister(&nand_otp_dev);
#endif

#ifdef SAMSUNG_OTP_SUPPORT
    g_mt6573_otp_fuc.OTPQueryLength = NULL;
    g_mt6573_otp_fuc.OTPRead = NULL;
    g_mt6573_otp_fuc.OTPWrite = NULL;
#endif

	platform_driver_unregister(&mt6573_nand_driver);
	remove_proc_entry(PROCNAME, NULL);
}

module_init(mt6573_nand_init);
module_exit(mt6573_nand_exit);
MODULE_LICENSE("GPL");
