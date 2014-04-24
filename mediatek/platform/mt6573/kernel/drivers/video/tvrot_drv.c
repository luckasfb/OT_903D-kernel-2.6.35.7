

#include <mach/mt6573_typedefs.h>
#include <mach/mt6573_reg_base.h>
#include <mach/mt6573_pll.h>
#include <mach/mt6573_sysram.h>

#include "tvrot_reg.h"
#include "tvrot_drv.h"

#include "mtkfb.h"


#if defined(MTK_TVOUT_SUPPORT)

PTVR_REGS const TVR_REG = (PTVR_REGS)(TV_ROT_BASE);

static BOOL s_isTvrPowerOn = FALSE;
static BOOL s_isSramAllocated = FALSE;
const UINT32 RESAMPLE = 0;

// ---------------------------------------------------------------------------
//  TVR Register Backup/Restore in suspend/resume
// ---------------------------------------------------------------------------

static TVR_REGS regBackup;

#define TVR_REG_OFFSET(r)       offsetof(TVR_REGS, r)
#define REG_ADDR(base, offset)  (((BYTE *)(base)) + (offset))

const UINT32 BACKUP_TVR_REG_OFFSETS[] =
{
    TVR_REG_OFFSET(IRQ_FLAG),
    TVR_REG_OFFSET(CFG),
    TVR_REG_OFFSET(DROP_INPUT),
    TVR_REG_OFFSET(STOP),
    TVR_REG_OFFSET(ENABLE),
    TVR_REG_OFFSET(RD_BASE),
    TVR_REG_OFFSET(WR_BASE),
    TVR_REG_OFFSET(QUEUE_BASE),
    TVR_REG_OFFSET(EXEC_CNT),
    TVR_REG_OFFSET(SLOW_DOWN),
    TVR_REG_OFFSET(BUF_BASE_ADDR0),
    TVR_REG_OFFSET(BUF_BASE_ADDR1),
    TVR_REG_OFFSET(Y_DST_STR_ADDR),
    TVR_REG_OFFSET(SRC_SIZE),
    TVR_REG_OFFSET(CLIP_SIZE),
    TVR_REG_OFFSET(CLIP_OFFSET),
    TVR_REG_OFFSET(DST_WIDTH_IN_BYTE),
    TVR_REG_OFFSET(CON),
    TVR_REG_OFFSET(PERF),
    TVR_REG_OFFSET(MAIN_BUF_SIZE),
    TVR_REG_OFFSET(SUB_BUF_SIZE),
    TVR_REG_OFFSET(BUF_BASE_ADDR2),
    TVR_REG_OFFSET(BUF_BASE_ADDR3)
};

#if 0
static void _BackupTVRRegisters(void)
{
    TVR_REGS *reg = &regBackup;
    UINT32 i;
    
    for (i = 0; i < ARY_SIZE(BACKUP_TVR_REG_OFFSETS); ++ i)
    {
        OUTREG32(REG_ADDR(reg, BACKUP_TVR_REG_OFFSETS[i]),
                 AS_UINT32(REG_ADDR(TVR_REG, BACKUP_TVR_REG_OFFSETS[i])));
    }
}

static void _RestoreTVRRegisters(void)
{
    TVR_REGS *reg = &regBackup;
    UINT32 i;

    for (i = 0; i < ARY_SIZE(BACKUP_TVR_REG_OFFSETS); ++ i)
    {
        OUTREG32(REG_ADDR(TVR_REG, BACKUP_TVR_REG_OFFSETS[i]),
                 AS_UINT32(REG_ADDR(reg, BACKUP_TVR_REG_OFFSETS[i])));
    }
}
#endif

// ---------------------------------------------------------------------------
//  Private TVR functions
// ---------------------------------------------------------------------------

static BOOL _reset_tvrot(void)
{
    UINT32 timeout = 0;
    
    TVR_REG_RESET reset = {0};
    reset.WARN_RESET = 1;
    OUTREG32(&TVR_REG->RESET, AS_UINT32(&reset));
    
    while (TVR_REG->RESET.WARN_RESET) {
        ++ timeout;
        if (timeout > 100000) {
            printk("[TVR][ERROR] Reset timeout\n");
            return FALSE;
        }
    }

    return TRUE;
}


#define ALIGN_TO_POW_OF_2(x, n)  \
    (((x) + ((n) - 1)) & ~((n) - 1))

extern unsigned long MT6573_SYSRAM_ALLOC(MT6573_SYSRAM_USR eOwner,
                                         unsigned long u4Size,
                                         unsigned long u4Alignment);

extern void MT6573_SYSRAM_FREE(MT6573_SYSRAM_USR eOwner);


static BOOL _alloc_and_set_sram(const TVR_PARAM *param)
{
    UINT32 main_lb_s_in_line = 0;
    UINT32 sub_lb_s_in_line = 0;

    UINT32 main_buf_line_size = 0;
    UINT32 main_blk_w = 0;
    UINT32 sub_buf_line_size = 0;
    UINT32 sub_blk_w = 0;

    UINT32 main_size = 0;
    UINT32 sub_size = 0;
    UINT32 desc_size = 4 * 2;

    UINT32 lb_bpp = 0;
    UINT32 src_width = param->srcWidth;

    UINT32 main_buf_addr = 0;
    UINT32 sub_buf_addr = 0;
    UINT32 desc_addr = 0;

    BOOL need_sub_buffer = FALSE;

    switch(param->outputFormat)
    {
    case TVR_RGB565  :
        main_lb_s_in_line = 4;
        lb_bpp = 2;
        need_sub_buffer = RESAMPLE ? TRUE : FALSE;
        break;
    case TVR_YUYV422 : 
        main_lb_s_in_line = 8;
        lb_bpp = 1;
        need_sub_buffer = TRUE;
        break;
    }

    // calculate main buffer

    main_blk_w = (src_width + main_lb_s_in_line - 1) / main_lb_s_in_line;

    main_buf_line_size = main_blk_w * (main_lb_s_in_line + 1);          // FIFO mode
    main_size = lb_bpp * main_buf_line_size * (main_lb_s_in_line + 1);  // FIFO mode

    // calculate sub buffer

    if (need_sub_buffer)
    {
        sub_lb_s_in_line = (main_lb_s_in_line / 2);
        
        if (RESAMPLE) src_width >>= 1;
        
        sub_blk_w = (src_width + sub_lb_s_in_line - 1) / sub_lb_s_in_line;
        
        sub_buf_line_size = sub_blk_w * (sub_lb_s_in_line + 1);     // FIFO mode
        sub_size = 2 * sub_lb_s_in_line * (sub_lb_s_in_line + 1);   // FIFO mode
    }

    main_size = ALIGN_TO_POW_OF_2(main_size, 8);
    sub_size  = ALIGN_TO_POW_OF_2(sub_size,  8);

    printk("[TVR] Allocate internal SRAM, main_size: %d, sub_size: %d\n",
           main_size, sub_size);

    // try to allocate internal SRAM
    
    main_buf_addr = MT6573_SYSRAM_ALLOC(MT6573SYSRAMUSR_TVROT,
                                        main_size + sub_size + desc_size, 8);

    if (0 == main_buf_addr) {
        printk("[TVR][ERROR] allocate internal SRAM failed\n");
        return FALSE;
    }

    s_isSramAllocated = TRUE;
    
    sub_buf_addr = main_buf_addr + main_size;
    desc_addr    = sub_buf_addr + sub_size;

    if (!need_sub_buffer) sub_buf_addr = 0;

    OUTREG32(&TVR_REG->BUF_BASE_ADDR0, main_buf_addr);
    OUTREG32(&TVR_REG->BUF_BASE_ADDR2, sub_buf_addr);
    {
        TVR_REG_BUF_SIZE buf_size;
        
        buf_size.LINE_SIZE   = main_buf_line_size;
        buf_size.BLOCK_WIDTH = main_blk_w;
        OUTREG32(&TVR_REG->MAIN_BUF_SIZE, AS_UINT32(&buf_size));
        
        buf_size.LINE_SIZE    = sub_buf_line_size;
        buf_size.BLOCK_WIDTH  = sub_blk_w;
        OUTREG32(&TVR_REG->SUB_BUF_SIZE, AS_UINT32(&buf_size));
    }
    {
        TVR_REG_PERF perf;
        perf.FIFO_MODE = 1;
        perf.MAIN_LB_S_IN_LINE = main_lb_s_in_line;
        perf.THRESHOLD = 7;
        OUTREG32(&TVR_REG->PERF, AS_UINT32(&perf));
    }

    // set descriptor base address
#if 1
    OUTREG32(&TVR_REG->QUEUE_BASE, desc_addr);
#else
    // Fixed descriptor internals SRAM address for debug
    OUTREG32(&TVR_REG->QUEUE_BASE, 0x40043E80);
#endif

    return TRUE;
}


static UINT32 _cal_dst_buffer_pitch(const TVR_PARAM *param)
{
    switch(param->rotation)
    {
    case TVR_ROT_0 :
    case TVR_ROT_180 :
        return param->srcWidth * 2;
        
    case TVR_ROT_90  :
    case TVR_ROT_270 :
        return param->srcHeight * 2;
        
    default :
        return 0;
    }
}


static UINT32 _cal_dst_buffer_offset(const TVR_PARAM *param,
                                     UINT32 dstPitchInBytes)
{
    switch(param->rotation)
    {
    case TVR_ROT_180 :
        return dstPitchInBytes * (param->srcHeight - 1);
    case TVR_ROT_270 :
        return dstPitchInBytes * (param->srcWidth - 1);
    default :
        return 0;
    }
}


static BOOL _config_tvrot_reg(const TVR_PARAM *param)
{
    UINT32 dstPitchInBytes = _cal_dst_buffer_pitch(param);
    UINT32 dstBufOffset    = _cal_dst_buffer_offset(param, dstPitchInBytes);

    {
        TVR_REG_CFG config = {0};

        // descriptor mode
        {
            config.AUTO_LOOP   = 1;
            config.DOUBLE_BUF  = 0;
            config.MODE        = 1;
            config.QUEUE_DEPTH = 2 - 1; // double DST buffers
            config.SEG1EN      = 1;     // enable DST buffer address field only
        }
        OUTREG32(&TVR_REG->CFG, AS_UINT32(&config));
    }
    {
        TVR_REG_SIZE size;
        size.WIDTH  = param->srcWidth;
        size.HEIGHT = param->srcHeight;
        OUTREG32(&TVR_REG->SRC_SIZE, AS_UINT32(&size));
        OUTREG32(&TVR_REG->CLIP_SIZE, AS_UINT32(&size));
        OUTREG32(&TVR_REG->CLIP_OFFSET, 0);
    }
    {
        TVR_REG_CON control   = TVR_REG->CON;
        control.OUTPUT_FORMAT = param->outputFormat;
        control.ROT_ANGLE     = param->rotation;
        control.FLIP          = param->flip ? 1 : 0;
        control.RESAMPLE      = RESAMPLE;
        control.ROUND         = 0;
        OUTREG32(&TVR_REG->CON, AS_UINT32(&control));
    }

    OUTREG32(&TVR_REG->DST_WIDTH_IN_BYTE, dstPitchInBytes);

    // mingchen: query the OK bit firstly, then write data to queue.
    // descriptor mode
    {
        UINT32 i, timeout;

        for (i = 0; i < ARY_SIZE(param->dstBufAddr); ++ i) {
            //OUTREG32(&TVR_REG->QUEUE_DATA, param->dstBufAddr[i] + dstBufOffset);

            timeout = 0;
            while (0 == TVR_REG->QUEUE_WSTA.OK) {
                ++ timeout;
                if (timeout > 100000) {
                    printk("[TVR][ERROR] QUEUE_DATA timeout\n");
                    return FALSE;
                }
            }
            OUTREG32(&TVR_REG->QUEUE_DATA, param->dstBufAddr[i] + dstBufOffset);
        }
    }

    return TRUE;
}

// ---------------------------------------------------------------------------
//  Interrupt Handler
// ---------------------------------------------------------------------------

static __tcmfunc irqreturn_t _TVR_InterruptHandler(int irq, void *dev_id)
{   
    TVR_REG_IRQ_FLAG flag = TVR_REG->IRQ_FLAG;
    TVR_REG_IRQ_FLAG_CLR clr = {0};
    
    if (flag.FLAG0) {
        printk("[TVR][IRQ] FLAG0: descriptor is finished\n");
        clr.FLAG0_CLR = 1;
    }
    if (flag.FLAG1) {
        printk("[TVR][IRQ] FLAG1: SW configuration error\n");
        clr.FLAG1_CLR = 1;
    }
    if (flag.FLAG5) {
        printk("[TVR][IRQ] FLAG5: SW configuration error\n");
        clr.FLAG5_CLR = 1;
    }

    OUTREG32(&TVR_REG->IRQ_FLAG_CLR, AS_UINT32(&clr));

    return IRQ_HANDLED;
}


// ---------------------------------------------------------------------------
//  Public TVR functions
// ---------------------------------------------------------------------------

TVR_STATUS TVR_Init(void)
{
    if (request_irq(MT6573_TV_ROT_IRQ_LINE,
        (irq_handler_t)_TVR_InterruptHandler, 0, "mt6573-tvrot", NULL) < 0)
    {
        printk("[TVR][ERROR] fail to request TVR irq\n"); 
        return TVR_STATUS_ERROR;
    }

    return TVR_STATUS_OK;
}


TVR_STATUS TVR_Deinit(void)
{
    TVR_Stop();
    TVR_PowerOff();

    return TVR_STATUS_OK;
}


TVR_STATUS TVR_PowerOn()
{
    if (!s_isTvrPowerOn)
    {
        BOOL ret = hwEnablePLL(MT65XX_3G_PLL, "TVR");
        ASSERT(ret);
        ret = hwEnableClock(MT65XX_PDN_MM_TV_ROT, "TVR");
        ASSERT(ret);
#if 0
        _RestoreTVRRegisters();
#endif
        s_isTvrPowerOn = TRUE;
    }

    return TVR_STATUS_OK;
}


TVR_STATUS TVR_PowerOff()
{
    if (s_isTvrPowerOn)
    {
        BOOL ret = TRUE;
#if 0
        _BackupTVRRegisters();
#endif
        ret = hwDisableClock(MT65XX_PDN_MM_TV_ROT, "TVR");
        ASSERT(ret);
        ret = hwDisablePLL(MT65XX_3G_PLL, "TVR");
        ASSERT(ret);

        s_isTvrPowerOn = FALSE;
    }

    return TVR_STATUS_OK;
}


TVR_STATUS TVR_Start(void)
{
    if (!s_isSramAllocated) {
        printk("[TVR][ERROR] working SRAM is not allocated!!\n");
        return TVR_STATUS_ERROR;
    }
    OUTREG32(&TVR_REG->ENABLE, 1);
    
    return TVR_STATUS_OK;
}


TVR_STATUS TVR_Stop(void)
{
    OUTREG32(&TVR_REG->STOP, 1);

    if (s_isSramAllocated) {
        MT6573_SYSRAM_FREE(MT6573SYSRAMUSR_TVROT);
        s_isSramAllocated = FALSE;
    }

    return TVR_STATUS_OK;
}


TVR_STATUS TVR_Config(const TVR_PARAM *param)
{
    // Enable All Interrupts
    {
        TVR_REG_IRQ_FLAG flag = {0};
        flag.FLAG0_IRQ_EN = 1;
        flag.FLAG1_IRQ_EN = 1;
        flag.FLAG5_IRQ_EN = 1;
        OUTREG32(&TVR_REG->IRQ_FLAG, AS_UINT32(&flag));
    }

    if (!_reset_tvrot()) {
        return TVR_STATUS_ERROR;
    }

    if (!_alloc_and_set_sram(param)) {
        return TVR_STATUS_INSUFFICIENT_SRAM;
    }

    if (!_config_tvrot_reg(param)) {
        return TVR_STATUS_ERROR;
    }

    return TVR_STATUS_OK;
}


TVR_STATUS TVR_DumpRegisters(void)
{
    UINT32 i;

    printk("---------- Start dump TVR registers ----------\n"
           "TVR_BASE: 0x%08x\n", TV_ROT_BASE);
    
    for (i = 0; i < sizeof(TVR_REGS); i += 4)
    {
        printk("TVR+%04x : 0x%08x\n", i, INREG32(TV_ROT_BASE + i));
    }

    return TVR_STATUS_OK;
}

#endif
