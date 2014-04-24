
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/cdc.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/wait.h>
#include <linux/platform_device.h>

#include <linux/wakelock.h>
/* architecture dependent header files */
#include <mach/mt6573_pll.h>
#include <mach/mt6573_typedefs.h>
#include <mach/mt6573_reg_base.h>
#include <mach/irqs.h>
#include <asm/io.h>
#include <linux/dma-mapping.h>
#include <asm/cacheflush.h>
#include "mt6573_udc.h"
#include <mtk_usb_custom.h>
#include <asm/tcm.h>
#include <mach/pmic_features.h>
#include <mach/pmu6573_sw.h>
#include <mach/upmu_sw.h>
#include "udc_log.h"
/* --------------------------------------------------------------------------*/
/* Function configuration                                                    */
/* --------------------------------------------------------------------------*/
/* Enable debug message with care!! */
/* Note that printk() will affect the transmission */
//#define USB_USE_DMA_MODE1
#define USB_USE_DMA_MODE0
//#define USB_USE_PIO_MODE
//#define CONFIG_USB_DPB  /*double packet buffer*/

#define CONFIG_USB_SMART_REGRW 
/* --------------------------------------------------------------------------*/
#define USB_TAG "[USB] "
#define USB_LOG_ENABLE  0
/* --------------------------------------------------------------------------*/
#define USB_VALID_EP(ep_num, dir) ((ep_num < MT_EP_NUM) && (dir < USB_DIR_NUM))
/* --------------------------------------------------------------------------*/
#define IRQ_LOG_SIZE    65536
/* --------------------------------------------------------------------------*/
/* Local variable declaration                                                */
/* --------------------------------------------------------------------------*/
static const char driver_name[] = "mt_udc";
static bool msc_test = FALSE;
static struct mt_udc *udc;
struct mt_udc udc_global;
//extern UINT32 g_USBStatus;

struct mt_ep_dma ep_dma[MT_CHAN_NUM];
struct wake_lock usb_lock;

static const u8 mt_usb_test_packet[53] = {
        /* implicit SYNC then DATA0 to start */

        /* JKJKJKJK x9 */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* JJKKJJKK x8 */
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        /* JJJJKKKK x8 */
        0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
        /* JJJJJJJKKKKKKK x8 */
        0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        /* JJJJJJJK x8 */
        0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd,
        /* JKKKKKKK x10, JK */
        0xfc, 0x7e, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0x7e

        /* implicit CRC16 then EOP to end */
};

#if USB_LOG_ENABLE
int irq_log[IRQ_LOG_SIZE];
unsigned int log_w = 0;
unsigned int log_r = 0;
#endif

extern void mt6573_irq_set_sens(unsigned int irq_line, unsigned int sens);
extern void BATTERY_SetUSBState(int usb_state_value);
extern void wake_up_bat(void);
extern kal_bool upmu_is_chr_det(upmu_chr_list_enum chr);

/* --------------------------------------------------------------------------*/
/* Function prototype                                                        */
/* --------------------------------------------------------------------------*/
static void irq_logging(int value);
static void irq_log_dump(void);

static inline int mt_udc_read_fifo(struct mt_ep *ep);
static inline int mt_udc_write_fifo(struct mt_ep *ep);

static void done(struct mt_ep *ep, struct mt_req *req, int status);

static void mt_ep0_handler(void);
static void mt_ep0_idle(void);
static void mt_ep0_rx(void);
static void mt_ep0_tx(void);
static void mt_epx_handler(int ep_num, USB_DIR dir);
static void mt_usb_reset(void);
static void mt_dev_connect(void);
static void mt_dev_disconnect(void);
static void mt_usb_load_testpacket(void);

static void mt_ep_fifo_flush(struct usb_ep *_ep);
static void mt_ep_fifo_flush_internal(struct usb_ep *_ep);

static irqreturn_t mt_udc_dma_handler(int irq_src);
#define mt_usb_config_dma(ep, req, addr, count) \
        __mt_usb_config_dma(ep, req, addr, count, __LINE__)
static void __mt_usb_config_dma(struct mt_ep* ep, struct mt_req* req, u32 addr, u32 count, int line);

extern void mt6573_usb_phy_recover(struct mt_udc *udc);
extern void mt6573_usb_phy_savecurrent(struct mt_udc *udc);
CHARGER_TYPE mt_charger_type_detection(void);
extern void mt6573_usb_charger_event_for_evb(int charger_in);
extern BOOL mt6573_usb_enable_clock(BOOL enable);
/* --------------------------------------------------------------------------*/
/* read/write register debug function                                        */
/* --------------------------------------------------------------------------*/
#if !defined(CONFIG_USB_SMART_REGRW)
#define __udc_readb     __raw_readb
#define __udc_readw     __raw_readw
#define __udc_readl     __raw_readl
#define __udc_writeb    __raw_writeb
#define __udc_writew    __raw_writew
#define __udc_writel    __raw_writel
/*don't turn off clock*/
#define UDC_ENABLE_CLOCK(on) do { if (on) mt6573_usb_enable_clock(on); } while (0)
#else
struct udc_reg {
    u8      (*_readb)(u32 addr, u32 line);
    u16     (*_readw)(u32 addr, u32 line);
    u32     (*_readl)(u32 addr, u32 line);
    void    (*_writeb)(u8  b, u32 addr, u32 line);
    void    (*_writew)(u16 w, u32 addr, u32 line);
    void    (*_writel)(u32 l, u32 addr, u32 line); 
};
static inline void udc_reg_enable_clock(u8 on, u32 line);
#define UDC_ENABLE_CLOCK(on) udc_reg_enable_clock(on, __LINE__)
static inline u8   __pof_readb(u32 addr, u32 line)   {printk("usb is power off, readb error\n\n");return 0;}//{ udc_reg_enable_clock(TRUE, line); return __raw_readb(addr); }
static inline u16  __pof_readw(u32 addr, u32 line)   {printk("usb is power off, readw error\n\n");return 0;}//{ udc_reg_enable_clock(TRUE, line); return __raw_readw(addr); }
static inline u32  __pof_readl(u32 addr, u32 line)   {printk("usb is power off, readl error\n\n");return 0;}//{ udc_reg_enable_clock(TRUE, line); return __raw_readl(addr); }
static inline void __pof_writeb(u8  b, u32 addr, u32 line)  {printk("usb is power off, writeb error\n\n");}//{ udc_reg_enable_clock(TRUE, line); __raw_writeb(b, addr); }
static inline void __pof_writew(u16 w, u32 addr, u32 line)  {printk("usb is power off, writew error\n\n");}//{ udc_reg_enable_clock(TRUE, line); __raw_writew(w, addr); }
static inline void __pof_writel(u32 l, u32 addr, u32 line)  {printk("usb is power off, writel error\n\n");}//{ udc_reg_enable_clock(TRUE, line); __raw_writel(l, addr); }

static inline u8   __pon_readb(u32 addr, u32 line)  { return __raw_readb(addr); }
static inline u16  __pon_readw(u32 addr, u32 line)  { return __raw_readw(addr); }
static inline u32  __pon_readl(u32 addr, u32 line)  { return __raw_readl(addr); }
static inline void __pon_writeb(u8  b, u32 addr, u32 line) { __raw_writeb(b, addr); }
static inline void __pon_writew(u16 w, u32 addr, u32 line) { __raw_writew(w, addr); }
static inline void __pon_writel(u32 l, u32 addr, u32 line) { __raw_writel(l, addr); }

static struct udc_reg g_udc_reg = {
    ._readb  = __pof_readb,
    ._readw  = __pof_readw,
    ._readl  = __pof_readl,
    ._writeb = __pof_writeb,
    ._writew = __pof_writew,
    ._writel = __pof_writel,
};
static inline void udc_reg_enable_clock(u8 on, u32 line)
{
    if (on) {
        printk(USB_TAG "enable_clock (%4d)\n", line);
        mt6573_usb_enable_clock(TRUE);
        g_udc_reg._readb = __pon_readb;
        g_udc_reg._readw = __pon_readw;
        g_udc_reg._readl = __pon_readl;
        g_udc_reg._writeb= __pon_writeb;
        g_udc_reg._writew= __pon_writew;
        g_udc_reg._writel= __pon_writel;
    } else {
        printk(USB_TAG "disable_clock(%4d)\n", line);
       
        g_udc_reg._readb = __pof_readb;
        g_udc_reg._readw = __pof_readw;
        g_udc_reg._readl = __pof_readl;
        g_udc_reg._writeb= __pof_writeb;
        g_udc_reg._writew= __pof_writew;
        g_udc_reg._writel= __pof_writel;
        mt6573_usb_enable_clock(FALSE);
    }
}

#define __udc_readb(addr)       g_udc_reg._readb(addr, __LINE__)
#define __udc_readw(addr)       g_udc_reg._readw(addr, __LINE__)
#define __udc_readl(addr)       g_udc_reg._readl(addr, __LINE__)
#define __udc_writeb(b,addr)    g_udc_reg._writeb(b, addr, __LINE__)
#define __udc_writew(w,addr)    g_udc_reg._writew(w, addr, __LINE__)
#define __udc_writel(l,addr)    g_udc_reg._writel(l, addr, __LINE__)
#endif
/* --------------------------------------------------------------------------*/
/* debug function implementation                                             */
/* --------------------------------------------------------------------------*/
int dummy_log(const char* fmt, ...) {return 0;}
int dummy_check_req(struct mt_ep *ep, struct mt_req *req, int line) {return 0;}
int dummy_check_dma_map(int chan, int ep_num, int dir) {return 0;}
int dummy_check_dma_unmap(struct mt_ep *ep, struct mt_req *req, int count, int line) {return 0;}
/* --------------------------------------------------------------------------*/
enum
{
    DEBUG_USB_LOG,
    DEBUG_USB_MSG,
    DEBUG_USB_ERR,
    DEBUG_CHECK_REQ,
    DEBUG_CHECK_DMA_MAP,
    DEBUG_CHECK_DMA_UNMAP,

    DEBUG_MAX_NUM,
};
/* --------------------------------------------------------------------------*/
static struct mt_udc_dbg g_udc_dbg = {
    .usblog = dummy_log,
    .usbmsg = dummy_log,
    .usberr = dummy_log,
    .check_req = dummy_check_req,
    .check_dma_map = dummy_check_dma_map,
    .check_dma_unmap = dummy_check_dma_unmap,
    .opt = (1 << DEBUG_USB_ERR),
    .dma_sta_retry_max = ATOMIC_INIT(5),
};
/* --------------------------------------------------------------------------*/
#define USB_LOG(fmt, args...)   g_udc_dbg.usblog(KERN_INFO USB_TAG fmt, ##args)
#define USB_MSG(fmt, args...)   g_udc_dbg.usbmsg(KERN_INFO USB_TAG fmt, ##args)
#define USB_ERR(fmt, args...)   g_udc_dbg.usberr(KERN_ERR USB_TAG"%s:%4d: "fmt, __FUNCTION__, __LINE__, ##args)
#define CHECKREQ(ep, req)       g_udc_dbg.check_req(ep, req, __LINE__) 
#define CHECK_DMA_MAP(ch,ep,d)  g_udc_dbg.check_dma_map(ch, ep, d)
#define CHECK_DMA_UNMAP(e,r,c)  g_udc_dbg.check_dma_unmap(e, r, c, __LINE__)
/* --------------------------------------------------------------------------*/
#define USB_REQ_SIGNATURE 0x042455AA
/* --------------------------------------------------------------------------*/
int check_req(struct mt_ep *ep, struct mt_req *req, int line)
{
    if ((req->signature != USB_REQ_SIGNATURE) || (req->queued < 0)) {
        USB_ERR("[%4d] %p/%p: %x %d\n", line, req, &req->req, req->signature, req->queued);
        USB_ERR("%d: %p %p %p %p\n", list_empty(&ep->queue), &ep->queue, ep->queue.next, ep->queue.next->next, ep->queue.next->next->next);
        ULG_FUC(MAKE_EP(ep->ep_num, ep->dir), 0x00, ULG_FUC_CHECK_REQ, "ddd", req, &req->req, req->signature);
    }        
    return 0;
}
/* --------------------------------------------------------------------------*/
int check_dma_map(int chan, int ep_num, int dir) 
{
    struct mt_ep_dma *dma = &ep_dma[chan - 1];
    if (chan < 1 || !dma)
        USB_ERR("invalid chan:%d, %p", chan, dma);
    if (atomic_inc_return(&dma->count) != 1) {
        ULG_FUC(MAKE_EP(ep_num, dir), 0x00, ULG_FUC_DMA_MAP_FAIL, "");
        USB_ERR("invalid: %d, %x\n", atomic_read(&dma->count), __udc_readl(USB_DMA_CNTL(chan)));
        atomic_set(&dma->count, 1); /*try to recover*/
    } else {
        ULG_FUC(MAKE_EP(ep_num, dir), 0x00, ULG_FUC_DMA_MAP_OKAY, "");
    }
    return 0;
}
/* --------------------------------------------------------------------------*/
int check_dma_unmap(struct mt_ep *ep, struct mt_req *req, int count, int line)
{
    int chan = (ep && ep->dma) ? (ep->dma->chan) : 0;
    struct mt_ep_dma *dma = &ep_dma[chan - 1];
    if (chan < 1 || !dma)
        USB_ERR("invalid chan:%d, %p", chan, dma);
    if (atomic_dec_return(&dma->count) != 0) {
        ULG_FUC(MAKE_EP(ep->ep_num, ep->dir), 0x00, ULG_FUC_DMA_UNMAP_FAIL, "");
        USB_ERR("invalid: %d, %d-%x [%d %d %d] %d\n", atomic_read(&dma->count), chan, __udc_readl(USB_DMA_CNTL(chan)), count, req->req.actual, req->req.length, line);
        atomic_set(&dma->count, 0); /*try to recover*/        
    } else {
        ULG_FUC(MAKE_EP(ep->ep_num, ep->dir), 0x00, ULG_FUC_DMA_UNMAP_OKAY, "");
    }
    return 0;
}
/*----------------------------------------------------------------------------*/
static ssize_t mt_udc_show_debug(struct device* dev, 
                                 struct device_attribute *attr, char *buf)
{
    struct mt_udc *obj;
    struct mt_udc_dbg *dbg;
    
    if (!dev) {
        printk(KERN_ERR "dev is null!!\n");
        return 0;
    } else if (!(obj = (struct mt_udc*)dev_get_drvdata(dev))) {
        printk(KERN_ERR "drv data is null!!\n");
        return 0;
    } else if (!(dbg = obj->dbg)) {
        printk(KERN_ERR "dbg is null!!\n");
        return 0;    
    }
    return scnprintf(buf, PAGE_SIZE, "0x%08lx %d\n", dbg->opt, atomic_read(&dbg->dma_sta_retry_max));
}
/*----------------------------------------------------------------------------*/
void mt_udc_setup_debug_callback(struct mt_udc_dbg *dbg)
{
    if (!dbg)
        return;

    dbg->usblog = test_bit(DEBUG_USB_LOG, &dbg->opt) ? (printk) : (dummy_log);
    dbg->usbmsg = test_bit(DEBUG_USB_MSG, &dbg->opt) ? (printk) : (dummy_log);
    dbg->usberr = test_bit(DEBUG_USB_ERR, &dbg->opt) ? (printk) : (dummy_log);
    dbg->check_req = test_bit(DEBUG_CHECK_REQ, &dbg->opt) ? (check_req) : (dummy_check_req);
    dbg->check_dma_map = test_bit(DEBUG_CHECK_DMA_MAP, &dbg->opt) ? (check_dma_map) : (dummy_check_dma_map);
    dbg->check_dma_unmap = test_bit(DEBUG_CHECK_DMA_UNMAP, &dbg->opt) ? (check_dma_unmap) : (dummy_check_dma_unmap);
}
/*----------------------------------------------------------------------------*/
static ssize_t mt_udc_store_debug(struct device* dev, struct device_attribute *attr,
                                  const char *buf, size_t count)
{
    struct mt_udc *obj;
    struct mt_udc_dbg *dbg;
    unsigned int mask, retry;

    if (!dev) {
        printk(KERN_ERR "dev is null!!\n");
        return count;
    } else if (!(obj = (struct mt_udc*)dev_get_drvdata(dev))) {
        printk(KERN_ERR "drv data is null!!\n");
        return count;
    } else if (!(dbg = obj->dbg)) {
        printk(KERN_ERR "dbg is null!!\n");
        return count;    
    } else if (2 == sscanf(buf, "0x%x %d", &mask, &retry)) {        
        dbg->opt = mask;
        mt_udc_setup_debug_callback(dbg);
        atomic_set(&dbg->dma_sta_retry_max, retry);
    } else {
        printk(KERN_ERR "invalid format!!\n");
        return count;
    }
    return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t mt_udc_show_cmode(struct device* dev, 
                                 struct device_attribute *attr, char *buf)
{
    struct mt_udc *obj;
    
    if (!dev) {
        printk(KERN_ERR "dev is null!!\n");
        return 0;
    } else if (!(obj = (struct mt_udc*)dev_get_drvdata(dev))) {
        printk(KERN_ERR "drv data is null!!\n");
        return 0;
    }
    return scnprintf(buf, PAGE_SIZE, "%d\n", atomic_read(&obj->cmode));
}
/*----------------------------------------------------------------------------*/
static ssize_t mt_udc_store_cmode(struct device* dev, struct device_attribute *attr,
                                  const char *buf, size_t count)
{
    struct mt_udc *obj;
    unsigned int cmode;

    if (!dev) {
        printk(KERN_ERR "dev is null!!\n");
        return count;
    } else if (!(obj = (struct mt_udc*)dev_get_drvdata(dev))) {
        printk(KERN_ERR "drv data is null!!\n");
        return count;
    } else if (1 == sscanf(buf, "%d", &cmode)) {   
        if (cmode >= CABLE_MODE_MAX)
            cmode = CABLE_MODE_NORMAL;
        if ((atomic_read(&obj->cmode) != cmode) && (udc->power)) {
            mt_usb_disconnect();
            atomic_set(&obj->cmode, cmode);
            msleep(10);
            mt_usb_connect();
        } else {
            USB_MSG("mode:%d->%d, power(%d)\n", atomic_read(&obj->cmode), cmode, udc->power);
            atomic_set(&obj->cmode, cmode);
        }
    } else {
        printk(KERN_ERR "invalid format!!\n");
        return count;
    }
    return count;
}
/*----------------------------------------------------------------------------*/
DEVICE_ATTR(debug, 0664, mt_udc_show_debug, mt_udc_store_debug);
DEVICE_ATTR(cmode, 0664, mt_udc_show_cmode, mt_udc_store_cmode);
/*----------------------------------------------------------------------------*/
static struct device_attribute *mt_udc_attrs[] = {
    &dev_attr_debug,    /*debug configuration*/
    &dev_attr_cmode,    /*0: charging only. disconnect if cable is connected */
                        /*1: normal mode*/
};
/*----------------------------------------------------------------------------*/
static int mt_udc_create_attr(struct device *dev) 
{
    int idx, err = 0;
    int num = (int)(sizeof(mt_udc_attrs)/sizeof(mt_udc_attrs[0]));

    if (!dev)
        return -EINVAL;

    for (idx = 0; idx < num; idx++) {
        if ((err = device_create_file(dev, mt_udc_attrs[idx]))) {            
            printk(KERN_ERR "device_create_file (%s) = %d\n", mt_udc_attrs[idx]->attr.name, err);        
            break;
        }
    }
    
    return err;
}
/*----------------------------------------------------------------------------*/
static int mt_udc_delete_attr(struct device *dev) 
{
    int idx ,err = 0;
    int num = (int)(sizeof(mt_udc_attrs)/sizeof(mt_udc_attrs[0]));
    
    if (!dev)
        return -EINVAL;

    for (idx = 0; idx < num; idx++) 
        device_remove_file(dev, mt_udc_attrs[idx]);

    return err;
}
/* --------------------------------------------------------------------------*/
/* Function implementation                                                   */
/* --------------------------------------------------------------------------*/
void mt6573_usb_charger_event_for_evb(int charger_in)
{
    if(charger_in)
    {
        if(STANDARD_HOST == mt_charger_type_detection())
        {
            mt_usb_connect();
        }
    }
    else
    {
        mt_usb_disconnect();
    }
  
    return;
}

#if USB_LOG_ENABLE
static void irq_logging(int value){

    *(irq_log + log_w) = value;
    
    log_w++;
    
    if(log_w == IRQ_LOG_SIZE)
        log_w = 0;
    
    if(log_w == log_r)
        log_r++;

    if(log_r == IRQ_LOG_SIZE)
        log_r = 0;

    return;
}

static void irq_log_dump(){

    while(log_w != log_r){
        printk("%d", *(irq_log + log_r));
        log_r++;
        if(log_r == IRQ_LOG_SIZE)
            log_r = 0;
    }

    printk("\nirq log dump complete\n");
    
    return;
}
#else
void irq_logging(int value){
    return;
}

static void irq_log_dump(){
    return;
}
#endif

static void mt_usb_load_testpacket(void){
    u8 index;
    u8 i;

    index = __udc_readb(INDEX);
    __udc_writeb(0, INDEX);

    for(i = 0; i < sizeof(mt_usb_test_packet); i++){
        __udc_writeb(mt_usb_test_packet[i], FIFO(0));
    }

    __udc_writew(EP0_TXPKTRDY, IECSR + CSR0);

    __udc_writeb(index, INDEX);

    return;
}

/* must be called when udc lock is obtained */
static inline int mt_udc_read_fifo(struct mt_ep *ep){
    int count = 0;
    u8 ep_num;
    u8 index = 0;
    int len = 0;
    unsigned char *bufp = NULL;
    struct mt_req *req;

    if(!ep){
        USB_ERR("mt_udc_read_fifo, *ep null\n");
        return 0;
    }

    ep_num = ep->ep_num;

    req = container_of(ep->queue.next, struct mt_req, queue);
    if(list_empty(&ep->queue)){
        USB_ERR("SW buffer is not ready!!\n");
        return 0;
    }
    CHECKREQ(ep, req);
    
    if(req){
        index = __udc_readb(INDEX);
        __udc_writeb(ep_num, INDEX);

        count = len = min((unsigned)__udc_readw(IECSR + RXCOUNT), req->req.length - req->req.actual);
        bufp = req->req.buf + req->req.actual;

        while(len > 0){
            *bufp = __udc_readb(FIFO(ep_num));
            bufp++;
            len--;
        }

        req->req.actual += count;

        __udc_writeb(index, INDEX);
    }
    else{// should not go here
         USB_ERR("NO REQUEST\n");
    }

    return count;
}

/* must be called when udc lock is obtained */
static inline int mt_udc_write_fifo(struct mt_ep *ep){
    int count = 0;
    u8 ep_num;
    u8 index = 0;
    int len = 0;
    unsigned char *bufp = NULL;
    unsigned int maxpacket;
    struct mt_req *req;
   
    if(!ep){
        USB_ERR("*ep null\n");
        return 0;
    }

    ep_num = ep->ep_num;
 
    req = container_of(ep->queue.next, struct mt_req, queue);
    if(list_empty(&ep->queue)){
        USB_ERR("SW buffer is not ready!!\n");
        return 0;
    }
    CHECKREQ(ep, req);

    if(req){
        index = __udc_readb(INDEX);
        __udc_writeb(ep_num, INDEX);
        
        maxpacket = ep->ep.maxpacket;

        count = len = min(maxpacket, req->req.length - req->req.actual);
        bufp = req->req.buf + req->req.actual;

        while(len > 0){
            __udc_writeb(*bufp, FIFO(ep_num));
            bufp++;
            len--;
        }
        req->req.actual += count;
        
        __udc_writeb(index, INDEX);
    }
    else{// should not go here
        USB_ERR("NO REQUEST\n");
    }

    return count;
}

/* retire a request, removing it from the queue */
/* must be called when udc lock is obtained */
static void done(struct mt_ep *ep, struct mt_req *req, int status)
{   
    int locking;
    #if 0
    if(ep->ep_num == 1 || ep->ep_num == 3 || ep->ep_num == 5)
        printk("[USB] done, ep %d, req = %x, req->req.actual = %d\n", ep->ep_num, &req->req, req->req.actual);
    #endif
    if(!ep || !req){
        USB_ERR("invalid *ep or *req pointer!!\n");
        return;
    }

    if(req->queued <= 0){
        USB_ERR("EP%d, REQ_NOT_QUEUED, REQ_STAT = %d\n", ep->ep_num, req->req.status);
        return;
    }
    
    list_del(&req->queue);
    ep->busycompleting = 1;
    req->queued = 0;
    
    if (likely(req->req.status == -EINPROGRESS))
		req->req.status = status;

    locking = spin_is_locked(&udc->lock);
    if(locking){
        spin_unlock_irqrestore(&udc->lock, udc->flags);
    }
    
    if(req->req.complete)
        req->req.complete(&ep->ep, &req->req);
    
    if(locking)
        spin_lock_irqsave(&udc->lock, udc->flags);
    ep->busycompleting = 0;
    /*
    if(ep->ep_num == 1 || ep->ep_num == 3 || ep->ep_num == 5)
        printk("[USB] done(complete), ep%d, req->req.status = %d\n", ep->ep_num, req->req.status);
    */
    
    return;
}

static int mt_ep_dma_alloc_locked(struct mt_ep *ep)
{
    int i, max = ARRAY_SIZE(ep_dma);

    if (ep->dma) {
        USB_ERR("dma already allocated!!\n");
        return -EINVAL;
    }
    //if (strstr(ep->ep.name, "-int")) { /*don't alloc dma for interrupt ep*/
    //    ep->dma_mode = DMA_MODE_PIO;
    //    ep->dma = NULL;
    //} else {
    {
        for (i = 0; i < max; i++) {
            if (atomic_cmpxchg(&ep_dma[i].allocated, 0, 1) == 0)
                break;
        }
        if (i == max) {
            ep->dma = NULL;
            ep->dma_mode = DMA_MODE_PIO;
            USB_ERR("alloc dma for ep[%s] fail\n", ep->ep.name);
        } else {
            ep->dma = &ep_dma[i];
            ep->dma_mode = DMA_MODE_0;
            ep->dma->burst = BURST_MODE_3;
            atomic_set(&ep->dma->active, 0);
        }    
    }
    USB_MSG("alloc dma : %d\n",  ep->dma ? (ep->dma->chan) : (0x00));
    ULG_FUC(MAKE_EP(ep->ep_num, ep->dir), udc->ep0_state, ULG_FUC_DMA_ALLOC, "d", ep->dma ? (ep->dma->chan) : (0x00));
    return 0;
        
}

static int mt_ep_dma_free_locked(struct mt_ep *ep)
{
    if (!ep->dma) {
        /*doing nothing*/
    } else if (atomic_read(&ep->dma->active)) {
        USB_ERR("dma used\n");
        return -EINVAL;
    } else {
        atomic_set(&ep->dma->allocated, 0);
        atomic_set(&ep->dma->active, 0);
    }
    USB_MSG("free dma : %d\n",  ep->dma ? (ep->dma->chan) : (0x00)); 
    ULG_FUC(MAKE_EP(ep->ep_num, ep->dir), udc->ep0_state, ULG_FUC_DMA_FREE, "d", ep->dma ? (ep->dma->chan) : (0x00));    
    ep->dma = NULL;
    return 0;
}

/* ========================================== */
/* callback functions registered to mt_ep_eps */
/* ========================================== */

/* configure endpoint, making it usable */
static int mt_ep_enable(struct usb_ep *_ep, 
                        const struct usb_endpoint_descriptor *desc)
{
    struct mt_ep *ep;
    u16 maxp;
    u16 tmp;

    if(!_ep || !desc || !udc){
        USB_ERR("invalid *_ep or *desc or *udc!!\n");
        return 0;
    }

    spin_lock_irqsave(&udc->lock, udc->flags);
    
    ep = container_of(_ep, struct mt_ep, ep);
    maxp = le16_to_cpu(desc->wMaxPacketSize);
    mt_ep_dma_alloc_locked(ep);
    //printk("[USB] mt_ep_enable, ep%d, start\n", ep->ep_num);

    ep->desc = desc;
    ep->ep.maxpacket = maxp;

    __udc_writeb(ep->ep_num, INDEX);
    
    if(EP_IS_IN(ep)) /* TX */
    {
        /* flush fifo and clear data toggle */
        tmp = __udc_readw(IECSR + TXCSR);
        tmp |= EPX_TX_CLRDATATOG;
        __udc_writew(tmp, IECSR + TXCSR);

        /* udpate max packet size to TXMAP */
        __udc_writew(maxp, IECSR + TXMAP);

        /* assign fifo address */
        __udc_writew((udc->fifo_addr) >> 3, TXFIFOADD);
        
        /* assign fifo size */
        tmp = desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
        if(tmp == USB_ENDPOINT_XFER_BULK)
        {
            if(udc->gadget.speed == USB_SPEED_FULL){
            #if defined(CONFIG_USB_DPB)    
                __udc_writeb(FIFOSZ_64 | DPB, TXFIFOSZ);
            #else
                __udc_writeb(FIFOSZ_64, TXFIFOSZ);
            #endif
            }
            else if(udc->gadget.speed == USB_SPEED_HIGH){
            #if defined(CONFIG_USB_DPB)    
                __udc_writeb(FIFOSZ_512 | DPB, TXFIFOSZ);
            #else
                __udc_writeb(FIFOSZ_512, TXFIFOSZ);
            #endif
            }
            else{
                USB_ERR("Not Supported speed\n");
            }
            
            /* update global fifo address */
        #if defined(CONFIG_USB_DPB)    
      		udc->fifo_addr += 2 * MT_BULK_MAXP;
        #else
            udc->fifo_addr += MT_BULK_MAXP;
        #endif
        }
        else if(tmp == USB_ENDPOINT_XFER_INT)
        {
            /* for full speed */
        #if defined(CONFIG_USB_DPB)            
            __udc_writeb(FIFOSZ_64 | DPB, TXFIFOSZ);
            /* update global fifo address */
            udc->fifo_addr += 2 * MT_INT_MAXP;
        #else
             __udc_writeb(FIFOSZ_64, TXFIFOSZ);
            /* update global fifo address */
            udc->fifo_addr += MT_INT_MAXP;
       #endif
        }
		
		/* enable the interrupt */
		tmp = __udc_readw(INTRTXE);
		tmp |= EPMASK(ep->ep_num);
		__udc_writew(tmp, INTRTXE);
		
    }
    else /* RX */
    {
        /* flush fifo and clear data toggle */
        tmp = __udc_readw(IECSR + RXCSR);
        tmp |= EPX_RX_CLRDATATOG;
        __udc_writew(tmp, IECSR + RXCSR);

        /* udpate max packet size to RXMAP */
        __udc_writew(maxp, IECSR + RXMAP);

        /* assign fifo address */
        __udc_writew((udc->fifo_addr >> 3), RXFIFOADD);

        /* assign fifo size */
        tmp = desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
        if(tmp == USB_ENDPOINT_XFER_BULK)
        {
            if(udc->gadget.speed == USB_SPEED_FULL){
            #if defined(CONFIG_USB_DPB)    
                __udc_writeb(FIFOSZ_64 | DPB, RXFIFOSZ);
            #else
                __udc_writeb(FIFOSZ_64, RXFIFOSZ);
            #endif
            }
            else if(udc->gadget.speed == USB_SPEED_HIGH){
            #if defined(CONFIG_USB_DPB)    
                __udc_writeb(FIFOSZ_512 | DPB, RXFIFOSZ);
            #else
                __udc_writeb(FIFOSZ_512, RXFIFOSZ);
            #endif
            }
            else{
                USB_ERR("Not supported speed\n");
            }
            /* update global fifo address */
        #if defined(CONFIG_USB_DPB)    
    		udc->fifo_addr += 2 * MT_BULK_MAXP;
        #else
    		udc->fifo_addr += MT_BULK_MAXP;
        #endif
        }
        else if(tmp == USB_ENDPOINT_XFER_INT)
        {
            /* for full speed */
        #if defined(CONFIG_USB_DPB)    
            __udc_writeb(FIFOSZ_64 | DPB, RXFIFOSZ);
            /* update global fifo address */
		    udc->fifo_addr += 2 * MT_INT_MAXP;
        #else
            __udc_writeb(FIFOSZ_64, RXFIFOSZ);
            /* update global fifo address */
		    udc->fifo_addr += MT_INT_MAXP;            
        #endif
        }
		
		/* enable the interrupt */
		tmp = __udc_readw(INTRRXE);
		tmp |= EPMASK(ep->ep_num);
		__udc_writew(tmp, INTRRXE);

    }

    USB_MSG("mt_ep_enable, ep%d, %d\n", ep->ep_num, udc->fifo_addr);
    atomic_set(&ep->enabled, 1);
    spin_unlock_irqrestore(&udc->lock, udc->flags);
    ULG_FUC(MAKE_EP(ep->ep_num, ep->dir), udc->ep0_state, ULG_FUC_EP_ENABLE, "dd", EP_IS_IN(ep), maxp);
    return 0;
}

/* endpoint is no longer usable */
/* reset struct mt_ep */
static int mt_ep_disable(struct usb_ep *_ep)
{
    struct mt_ep *ep;
    struct mt_req *req = NULL;
    u16 tmp;

    if(!_ep || !udc){
        USB_ERR("Invalid *_ep, *udc\n");
        return 0;
    }

    spin_lock_irqsave(&udc->lock, udc->flags);

    ep = container_of(_ep, struct mt_ep, ep);

    //printk("[USB] mt_ep_disable, ep%d, start\n", ep->ep_num);

    if(udc && (udc->power == USB_TRUE)){
        tmp = __udc_readw(INTRTXE);
        tmp &= ~(EPMASK(ep->ep_num));
        __udc_writew(tmp, INTRTXE);

        tmp = __udc_readw(INTRRXE);
        tmp &= ~(EPMASK(ep->ep_num));
        __udc_writew(tmp, INTRRXE);
    }

    while(!list_empty(&ep->queue)){
        req = container_of(ep->queue.next, struct mt_req, queue);
        CHECKREQ(ep, req);
        if(!req){
            USB_ERR("invalid *req\n");
            spin_unlock_irqrestore(&udc->lock, udc->flags);
            return 0;
        }
        done(ep, req, -ECONNRESET);
    }

    USB_MSG("mt_ep_disable, ep%d, end\n", ep->ep_num);
    ep->desc = NULL;
    mt_ep_dma_free_locked(ep);
    atomic_set(&ep->enabled, 0);    
    spin_unlock_irqrestore(&udc->lock, udc->flags);
    ULG_FUC(MAKE_EP(ep->ep_num, ep->dir), udc->ep0_state, ULG_FUC_EP_DISABLE, NULL);
    return 0;
}

/* allocate a request object to use with this endpoint */
static struct usb_request *mt_ep_alloc_request(struct usb_ep *_ep, unsigned 
gfp_flags)
{
    struct mt_req *req;

    //printk("mt_alloc_request\n");

    if(!(req = kmalloc(sizeof(struct mt_req), gfp_flags)))
    {
        USB_ERR("Request Allocation Failed\n");
        return NULL;
    }
    
    memset(req, 0, sizeof(struct mt_req));
    req->signature = USB_REQ_SIGNATURE;

    //USB_LOG("[USB] mt_ep_alloc_request, ep %s, req = %p/%p\n", _ep->name, req, &req->req);

    return &req->req;
}

/* frees a request object */
static void mt_ep_free_request(struct usb_ep *_ep, struct usb_request *_req)
{
    /* if the request is being transferred, then we must stop the activity */
    /* and remove the request from the queue of the endpoint, start the transfer */
    /* of the next request */
    struct mt_req *req;

    //printk("mt_free_request\n");

    if(!_ep || !_req){
        USB_ERR("invalid *_ep or !_req\n");
        return;
    }
    
    req = container_of(_req, struct mt_req, req);
    memset(req, 0x00, sizeof(*req));
    kfree(req);
    
    return;
}

/* queues(submits) an I/O request to an endpoint */
static int mt_ep_enqueue(struct usb_ep *_ep, struct usb_request *_req, unsigned gfp_flags)
{
    struct mt_ep *ep;
    struct mt_req *req;
    int ep_idle;
    int ep_num = 0;
    u16 csr;
    u8 index;

    if(!_req || !_ep || !udc){
        USB_ERR("invalid USB Enqueue Operation\n");
        return -1;
    }
    
    spin_lock_irqsave(&udc->lock, udc->flags);

    ep = container_of(_ep, struct mt_ep, ep);
    req = container_of(_req, struct mt_req, req);

    ep_num = ep->ep_num;

    ep_idle = list_empty(&ep->queue);
    /*
    if(ep->busycompleting)
        printk("[USB] ep %d is busy completing a transfer\n", ep_num);
    }
    */
    if(req->queued > 0){
        USB_ERR("ep%d is already queued, req->queued = %d\n", ep_num, req->queued);
        if(ep_num != 0) 
            done(ep, req, -ECONNRESET);            
        spin_unlock_irqrestore(&udc->lock, udc->flags);
        ULG_FUC(MAKE_EP(ep->ep_num, ep->dir), udc->ep0_state, ULG_FUC_ENQUEUE_ALREADY, NULL);
        return 0;
    }

    if((ep_num != 0) && !(ep->desc)){
        USB_ERR("aborted, connection in reset\n");
        irq_logging(-6);
        spin_unlock_irqrestore(&udc->lock, udc->flags);
        ULG_FUC(MAKE_EP(ep->ep_num, ep->dir), udc->ep0_state, ULG_FUC_ENQUEUE_ABORT, NULL);
        return -ESHUTDOWN;
    }
    
    req->req.status = -EINPROGRESS;
    req->req.actual = 0;
    list_add_tail(&req->queue, &ep->queue);
    req->queued = 1;

    if((ep_num == 0) && (req->req.length == 0)){

        if(msc_test)
        {
        	printk("[USB] zero length control transfer\n");
        	index = __udc_readb(INDEX);
  				__udc_writeb(0, INDEX);
			 	 csr = __udc_readw(IECSR + CSR0);
       	 csr |= (EP0_SERVICED_RXPKTRDY | EP0_DATAEND);
		   	 __udc_writew(csr, IECSR + CSR0);		
		   	 msc_test=FALSE;
        }
        done(ep, req, 0);
        udc->ep0_state = EP0_IDLE;
        spin_unlock_irqrestore(&udc->lock, udc->flags);
        ULG_FUC(MAKE_EP(ep->ep_num, ep->dir), udc->ep0_state, ULG_FUC_ENQUEUE_ZLP, NULL);
        return 0;
    }

    if((ep_num == 0) && udc->ep0_state == EP0_TX){
        ULG_FUC(MAKE_EP(ep->ep_num, ep->dir), udc->ep0_state, ULG_FUC_ENQUEUE_TX, "d", ep_idle);
        if(!ep_idle){
            spin_unlock_irqrestore(&udc->lock, udc->flags);
            return 0;
        }
        
        mt_ep0_tx();
    }

    if((ep_num != 0) && (ep->desc->bEndpointAddress & USB_DIR_IN)){
        /*printk("mt_ep_enqueue, tx, ep_num = %d, req = %x, req->length = %d, req->actual = %d\n", \
        ep_num, &req->req, req->req.length, req->req.actual); */
        ULG_FUC(MAKE_EP(ep->ep_num, ep->dir), udc->ep0_state, ULG_FUC_ENQUEUE_TX, "dddd", ep_idle, ep->busycompleting, req->req.length, req->req.actual);
        if(ep_idle && (!ep->busycompleting)){
            
            index = __udc_readb(INDEX);
            __udc_writeb(ep_num, INDEX);

            if (ep->dma) { /*DMA mode*/
            #if defined USB_USE_DMA_MODE0
                mt_usb_config_dma(ep, req, (u32)req->req.buf + req->req.actual, 
                min((unsigned int)(ep->ep.maxpacket), req->req.length - req->req.actual));       
            #elif defined USB_USE_DMA_MODE1
                csr = __udc_readw(IECSR + TXCSR);
                csr |= EPX_TX_AUTOSET | EPX_TX_DMAREQEN | EPX_TX_DMAREQMODE;
                __udc_writew(csr, IECSR + TXCSR);
                mt_usb_config_dma(ep, req, (u32)req->req.buf, req->req.length);
            #endif    
            } else { /*PIO mode*/           
                mt_udc_write_fifo(ep);
                csr = __udc_readw(IECSR + TXCSR);
                csr |= EPX_TX_TXPKTRDY;
                __udc_writew(csr, IECSR + TXCSR);
            }
            __udc_writeb(index, INDEX);
        }
    }

    if((ep_num != 0) && !(ep->desc->bEndpointAddress & USB_DIR_IN)){
        //USB_LOG("mt_ep_enqueue, rx\n");
        ULG_FUC(MAKE_EP(ep->ep_num, ep->dir), udc->ep0_state, ULG_FUC_ENQUEUE_RX, "dddd", ep_idle, ep->busycompleting, req->req.length, req->req.actual);        
        if(ep_idle  && (!ep->busycompleting)){
            index = __udc_readb(INDEX);
            __udc_writeb(ep_num, INDEX);

            csr = __udc_readw(IECSR + RXCSR);
            if(csr & EPX_RX_RXPKTRDY){
                if (ep->dma) { /*DMA mode*/
                    mt_usb_config_dma(ep, req, (u32)req->req.buf + req->req.actual, \
                    min((unsigned)__udc_readw(IECSR + RXCOUNT), req->req.length - req->req.actual));
                } else { /*PIO mode*/
                    mt_udc_read_fifo(ep);
                    csr &= ~EPX_RX_RXPKTRDY;
                    __udc_writew(csr, IECSR + RXCSR);                    
                }
            }

            __udc_writeb(index, INDEX);
        }
    }
    
    spin_unlock_irqrestore(&udc->lock, udc->flags);
    
    return 0;
}

/* dequeues(cancels, unlinks) an I/O request from an endoint */
static int mt_ep_dequeue(struct usb_ep *_ep, struct usb_request *_req)
{
    struct mt_ep *ep;
    struct mt_req *req;

    if(!udc){
        USB_ERR("invalid *udc!!\n");
        return 0;
    }

    spin_lock_irqsave(&udc->lock, udc->flags);

    if(!_ep || !_req){
        USB_ERR("invalid dequeue operation, _ep = %x, _req = %x!!\n", (unsigned int)_ep, (unsigned int)_req);

        spin_unlock_irqrestore(&udc->lock, udc->flags);
        return 0;
    }

    ep = container_of(_ep, struct mt_ep, ep);

    //printk("mt_ep_dequeue, ep %d, start\n", ep->ep_num);

    list_for_each_entry(req, &ep->queue, queue)
    {
        if(&req->req == _req)
            break;
    }

    if(&req->req != _req)
    {
        spin_unlock_irqrestore(&udc->lock, udc->flags);
        return -EINVAL;
    }
    done(ep, req, -ECONNRESET);

    //printk("mt_ep_dequeue, ep %d, end\n", ep->ep_num);

    spin_unlock_irqrestore(&udc->lock, udc->flags);
    ULG_FUC(MAKE_EP(ep->ep_num, ep->dir), udc->ep0_state, ULG_FUC_DEQUEUE, NULL);        

    return 0;
}

/* sets the endpoint halt feature or */
/* clears endpoint halt, and resets toggle */
static int mt_ep_set_halt(struct usb_ep *_ep, int value)
{
    struct mt_ep *ep;
    u8 index;
    u16 csr;

    if (!_ep)
		return -EINVAL;

    spin_lock_irqsave(&udc->lock, udc->flags);

    ep = container_of(_ep, struct mt_ep, ep);

    //printk("mt_ep_set_halt, ep %d, start\n", ep->ep_num);
  //  ULG_FUC(MAKE_EP(ep->ep_num, ep->dir), udc->ep0_state, ULG_FUC_SET_HALT, "d", EP_IS_IN(ep));

    index = __udc_readb(INDEX);
    __udc_writeb(ep->ep_num, INDEX);
    
    if(value && (ep->dir == USB_TX)){
        csr = __udc_readw(IECSR + TXCSR);

        if(csr & EPX_TX_FIFONOTEMPTY){
            spin_unlock_irqrestore(&udc->lock, udc->flags);
            return -EAGAIN;
        }
     //   __udc_writeb(index, INDEX);
    }

    if(ep->dir == USB_TX){
        csr = __udc_readw(IECSR + TXCSR);
        if(csr & EPX_TX_FIFONOTEMPTY)
            csr |= EPX_TX_FLUSHFIFO;
        csr |= EPX_TX_WZC_BITS;
        csr |= EPX_TX_CLRDATATOG;
        if(value){
            csr |= EPX_TX_SENDSTALL;
        }
        else{
            csr &= ~(EPX_TX_SENDSTALL | EPX_TX_SENTSTALL);
            csr &= ~EPX_TX_TXPKTRDY;
            ep -> wedge = 0;
        }
        __udc_writew(csr, IECSR + TXCSR);
    }
    else{
        csr = __udc_readw(IECSR + RXCSR);
        csr |= EPX_RX_WZC_BITS;
        csr |= EPX_RX_FLUSHFIFO;
        csr |= EPX_RX_CLRDATATOG;

        if(value){
            csr |= EPX_RX_SENDSTALL;
        }
        else{
            csr &= ~(EPX_RX_SENDSTALL | EPX_RX_SENTSTALL);
            ep -> wedge = 0;
        }

        __udc_writew(csr, IECSR + RXCSR);
    }

    if(!list_empty(&ep->queue) && !ep->busycompleting){
        if(ep->dir == USB_TX)
            mt_epx_handler(ep->ep_num, USB_TX);
        else
            mt_epx_handler(ep->ep_num, USB_RX);        
    }

    __udc_writeb(index, INDEX);

    //printk("mt_ep_set_halt, ep %d, end\n", ep->ep_num);

    spin_unlock_irqrestore(&udc->lock, udc->flags);

    return 0;
}

/* returns number of bytes in fifo, or error */
static int mt_ep_fifo_status(struct usb_ep *_ep)
{

    struct mt_ep *ep;
    int count;
    u8 index;

    if(!_ep){
        USB_ERR("invalid *_ep\n");
        return 0;
    }

    spin_lock_irqsave(&udc->lock, udc->flags);

    //printk("mt_ep_fifo_status, start\n");

    count = 0;
    
    ep = container_of(_ep, struct mt_ep, ep);

    index = __udc_readb(INDEX);
    __udc_writeb(ep->ep_num, INDEX);
    
    if(ep->ep_num == 0)
    {
        count = __udc_readb(IECSR + COUNT0);
    }
    else
    {
        count = __udc_readw(IECSR + RXCOUNT);
    }

    __udc_writeb(index, INDEX);

    //printk("mt_ep_fifo_status, end\n");

    spin_unlock_irqrestore(&udc->lock, udc->flags);
    ULG_FUC(MAKE_EP(ep->ep_num, ep->dir), udc->ep0_state, ULG_FUC_FIFO_STATUS, "d", count);
    return count;
}

static void mt_ep_fifo_flush(struct usb_ep *_ep)
{
    if(!_ep){
        USB_ERR("invalid *_ep\n");
        return;
    }

    spin_lock_irqsave(&udc->lock, udc->flags);
    mt_ep_fifo_flush_internal(_ep);
    spin_unlock_irqrestore(&udc->lock, udc->flags);
    return;
}

/* flushes contents of a fifo, doe */
static void mt_ep_fifo_flush_internal(struct usb_ep *_ep)
{
    struct mt_ep *ep;
    u16 csr;
    u8 index;

    if(!_ep){
        USB_ERR("invalid *_ep\n");
        return;
    }

    if(!udc || (udc->power == USB_FALSE))
        return;

    //printk("mt_ep_fifo_flush_internal, start\n");

    ep = container_of(_ep, struct mt_ep, ep);
    ULG_FUC(MAKE_EP(ep->ep_num, ep->dir), udc->ep0_state, ULG_FUC_FIFO_FLUSH, NULL); 
    //printk("mt_ep_fifo_flush_internal, ep%d\n", ep->ep_num);
    if (!ep->desc && atomic_read(&ep->enabled)) {
        USB_ERR("ep%d: invalid ep->desc\n", ep->ep_num);
        return;
    }
        

    /* write into INDEX register to select the endpoint mapped */
    index = __udc_readb(INDEX);
    __udc_writeb(ep->ep_num, INDEX);

    /* the structure of different types of endpiont are different */
    if(ep->ep_num == 0)
    {
        //csr = __udc_readw(IECSR + CSR0);
        csr = EP0_FLUSH_FIFO;
        __udc_writew(csr, IECSR + CSR0);
        __udc_writew(csr, IECSR + CSR0);
    } else {
        int dir = ep->dir;

        if (atomic_read(&ep->enabled)) {
            if (EP_IS_IN(ep) && (dir == USB_TX))
                ;
            else if (!EP_IS_IN(ep) && (dir == USB_RX))
                ;
            else /*should not go here*/
                USB_ERR("EP%d: check dir fail\n", ep->ep_num);
        }
        if (dir == USB_TX)
        {
            csr = EPX_TX_FLUSHFIFO;
            __udc_writew(csr, IECSR + TXCSR);
            __udc_writew(csr, IECSR + TXCSR);
        }
        else
        {
            csr = EPX_RX_FLUSHFIFO;
            __udc_writew(csr, IECSR + RXCSR);
            __udc_writew(csr, IECSR + RXCSR);
        }
    }

    __udc_writeb(index, INDEX);

    //printk("mt_ep_fifo_flush_internal, end\n");

    return;
}

static int mt_gadget_set_wedge(struct usb_ep *_ep)
{
    struct mt_ep *ep;
    
    if (!_ep)
			return -EINVAL;		
		ep = container_of(_ep, struct mt_ep, ep);


    ep -> wedge = 1;
		return usb_ep_set_halt(_ep);
}
/* ========= */
/* mt_ep_ops */
/* ========= */

static struct usb_ep_ops mt_ep_ops = 
{
    .enable = mt_ep_enable,
    .disable = mt_ep_disable,

    .alloc_request = mt_ep_alloc_request,
    .free_request = mt_ep_free_request,

    .queue = mt_ep_enqueue,
    .dequeue = mt_ep_dequeue,

    .set_halt = mt_ep_set_halt,
    .fifo_status = mt_ep_fifo_status,
    .fifo_flush = mt_ep_fifo_flush,
    .set_wedge	= mt_gadget_set_wedge,
    
};

/* ============================================== */
/* callback functions registered to mt_gadget_eps */
/* ============================================== */

/* returns the current frame number */
static int mt_get_frame(struct usb_gadget *gadget)
{
    //printk("mt_get_frame\n");

    return __udc_readw(FRAME);
}

/* tries to wake up the host connected to this gadget */
//static int mt_wakeup(struct usb_gadget *gadget)
//{
//    return -ENOTSUPP;
//}

/* sets or cleares the device selpowered feature */
/* called by usb_gadget_set_selfpowered and usb_gadget_clear_selfpowered */
static int mt_set_selfpowered(struct usb_gadget *gadget, int is_selfpowered)
{
    /* not supported now */

    return -ENOTSUPP;
}

/* notify the controller that VBUS is powered or VBUS session end */
/* called by usb_gadget_vbus_connect and usb_gadget_vbus_disconnect */
static int mt_vbus_session(struct usb_gadget *gadget, int is_active)
{
    /* not supported now */
    return -ENOTSUPP;
}

/* constrain controller's VBUS power usage */
static int mt_vbus_draw(struct usb_gadget *gadget, unsigned mA)
{
    /* not supported now */
    return -ENOTSUPP;
}

/* software-controlled connect/disconnect from USB host */
/* called by usb_gadget_connect and usb_gadget_disconnect_gadget */
static int mt_pullup(struct usb_gadget *gadget, int is_on)
{
    if(is_on){        
        if(upmu_is_chr_det(CHR)){
            switch(mt_charger_type_detection()){
                case STANDARD_HOST:
                    mt_dev_connect();
                    break;
                case CHARGER_UNKNOWN:
                case STANDARD_CHARGER:
                case NONSTANDARD_CHARGER:
                case CHARGING_HOST:
                    break;
            }
        }        
    }
    else{
        mt_dev_disconnect();
    }
    
    return 0;
}

/* ============= */
/* mt_gadget_ops */
/* ============= */

/* except for get_frame, most of the callback functions are not implemented */
struct usb_gadget_ops mt_gadget_ops = 
{
    .get_frame = mt_get_frame,
    .wakeup = NULL, /*android composite will set remote-wakeup attribute bit if the function is not NULL*/
    .set_selfpowered = mt_set_selfpowered,
    .vbus_session = mt_vbus_session,
    .vbus_draw = mt_vbus_draw,
    .pullup = mt_pullup,
    .ioctl = NULL,
};

/* =================================================================== */
/* These are the functions that bind peripheral controller drivers and */
/* gadget drivers together                                             */
/* =================================================================== */

int usb_gadget_register_driver(struct usb_gadget_driver *driver)
{
    /* declaration is in usb_gadget.h, but implementation is here */
    /* binds gadget driver and peripheral controller driver, and then calls */
    /* bind() callback function registered by gadget driver */
    
    int retval;
    
    if(!driver || !driver->bind || !driver->disconnect || !driver->setup)
        return -EINVAL;

    spin_lock_irqsave(&udc->lock, udc->flags);

    if(!udc){
        spin_unlock_irqrestore(&udc->lock, udc->flags);
        return -ENODEV;
    }

    if(udc->driver){
        USB_ERR("A driver has been associated with MT_UDC\n");
        spin_unlock_irqrestore(&udc->lock, udc->flags);
        return -EBUSY;
    }

    /* hook up gadget driver and peripheral controller driver */
    udc->driver = driver;
    udc->gadget.dev.driver = &driver->driver;
    //udc->gadget.dev.driver = &driver->driver;
    spin_unlock_irqrestore(&udc->lock, udc->flags);
    retval = device_add(&udc->gadget.dev);
    
    /* may call udc related functions in external callback functions */
   
    retval = driver->bind(&udc->gadget);
    
    spin_lock_irqsave(&udc->lock, udc->flags);
    
    if(retval)
    {
        USB_ERR("%s binds to %s => FAIL\n", udc->gadget.name, 
        driver->driver.name);
        device_del(&udc->gadget.dev);

        udc->driver = NULL;
        udc->gadget.dev.driver = NULL;

        spin_unlock_irqrestore(&udc->lock, udc->flags);
        return retval;
    }

    spin_unlock_irqrestore(&udc->lock, udc->flags);

    printk("USB REGISTER SUCCESS\n");
    ULG_FUC(MAKE_EP(0,0), udc->ep0_state, ULG_FUC_GADGET_REG, NULL);   
    UDC_ENABLE_CLOCK(FALSE);
    return 0;
}

int usb_gadget_unregister_driver(struct usb_gadget_driver *driver)
{
    /* declaration is in usb_gadget.h, but implementation is here */
    /* unbinds gadget driver and peripheral controller driver, and then calls */
    /* unbind() callback function registered by gadget driver */
    //int i;

    if(!udc)
        return -ENODEV;

    if(!driver || driver != udc->driver)
        return -EINVAL;

    spin_lock_irqsave(&udc->lock, udc->flags);

    mt_dev_disconnect();
   
    udc->driver = NULL;
    
    spin_unlock_irqrestore(&udc->lock, udc->flags);
 
    driver->unbind(&udc->gadget);
    mt_udc_delete_attr(&udc->gadget.dev);
    device_del(&udc->gadget.dev);
    USB_MSG("Unregistered gadget driver '%s'\n", driver->driver.name);
    ULG_FUC(MAKE_EP(0,0), udc->ep0_state, ULG_FUC_GADGET_UNREG, NULL);   
    return 0;
}

/* ==================================================== */
/* interrut service routine when interrupts are issued  */
/* there are two interrupt sources in mt3351 USB module */
/* one for USB, one for USB_DMA                             */
/* ==================================================== */

static __tcmfunc irqreturn_t mt_udc_irq(int irq, void *dev_id)
{
    irqreturn_t status;
    unsigned int ep;

    u16 intrtx, intrrx; 
    u8 intrusb;

    //printk("mt_udc_irq\n");
    irq_logging(1);

    ep = 0;
    status = IRQ_HANDLED;

    if(!udc->driver){
        return status;
    }

    //printk("USB: INTERRUPT, START\n");
    
    /* find the interrupt source and call the corresonding handler */
    /* RX interrupts must be stored and INTRRX cleared immediately */
    /* RX interrupts will be lost if INTRRX not cleared and RX interrupt is 
    issued */
    
    intrtx =  __udc_readw(INTRTX);
    intrrx =  __udc_readw(INTRRX);
    mb();
    __udc_writew(~intrrx, INTRRX);
    intrusb = __udc_readb(INTRUSB);

    intrtx &= __udc_readw(INTRTXE);
    intrrx &= __udc_readw(INTRRXE);
    intrusb &= __udc_readb(INTRUSBE);

    ULG_INT(MAKE_EP(0,0), udc->ep0_state, ULG_INT_UDC_IRQ_S, "ddd", intrtx, intrrx, intrusb);
    
    /* print error message and enter an infinite while loop */
    if(intrusb & INTRUSB_VBUS_ERROR)
    {
        ULG_INT(MAKE_EP(0,0), udc->ep0_state, ULG_INT_VBUS_ERROR, NULL);
        USB_MSG("INTRUSB_VBUS_ERROR\n");
    }

    /* the device receives reset signal and call reset function */
    if(intrusb & INTRUSB_RESET)
    {
        ULG_INT(MAKE_EP(0,0), udc->ep0_state, ULG_INT_RESET, NULL);
        wake_lock(&usb_lock);
        mt_usb_reset();
        status = IRQ_HANDLED;
        USB_MSG("INTRUSB_RESET\n");
    }
   
    if(intrusb & INTRUSB_SESS_REQ)
    {
        ULG_INT(MAKE_EP(0,0), udc->ep0_state, ULG_INT_SESSIONREQ, NULL);
        status = IRQ_HANDLED;
        USB_MSG("INTRUSB_SESS_REQ\n");
    }
    if(intrusb & INTRUSB_DISCON)
    {
        ULG_INT(MAKE_EP(0,0), udc->ep0_state, ULG_INT_DISCONNECT, NULL);
        wake_unlock(&usb_lock);
        status = IRQ_HANDLED;
        USB_MSG("INTRUSB_DISCON\n");
    }
    if(intrusb & INTRUSB_CONN) /* only valid in host mode */
    {
        ULG_INT(MAKE_EP(0,0), udc->ep0_state, ULG_INT_CONNECT, NULL);
        status = IRQ_HANDLED;
        USB_MSG("INTRUSB_CONN\n");
    }
    if(intrusb & INTRUSB_SOF)
    {
        ULG_INT(MAKE_EP(0,0), udc->ep0_state, ULG_INT_SOF, NULL);
        status = IRQ_HANDLED;
        USB_MSG("INTRUSB_SOF\n");
    }
    if(intrusb & INTRUSB_RESUME)
    {
        ULG_INT(MAKE_EP(0,0), udc->ep0_state, ULG_INT_RESUME, NULL);
        status = IRQ_HANDLED;
        USB_MSG("INTRUSB_RESUME\n");
    }
    if(intrusb & INTRUSB_SUSPEND)
    { 
        ULG_INT(MAKE_EP(0,0), udc->ep0_state, ULG_INT_SUSPEND, NULL);
        irq_log_dump();

        BATTERY_SetUSBState(USB_SUSPEND);
        wake_up_bat();
        
        wake_unlock(&usb_lock);
		wake_lock_timeout(&usb_lock, 5 * HZ);	   
        
        status = IRQ_HANDLED;
        USB_MSG("INTRUSB_SUSPEND\n");
    }
    
    /* the interrupt source is endpoint 0 and calls the state handler */
    if(intrtx)
    { 
        if(intrtx & EPMASK(0))
        {
            mt_ep0_handler();
            status = IRQ_HANDLED;
        }
        
        for(ep = 1; ep < MT_EP_NUM; ep++)
        {
            if(intrtx & EPMASK(ep))
            {
                mt_epx_handler(ep, USB_TX);
                status = IRQ_HANDLED;
            }
        }
    }

    if(intrrx)
    {
        for(ep = 1; ep < MT_EP_NUM; ep++)
        {
            if(intrrx & EPMASK(ep))
            {
                mt_epx_handler(ep, USB_RX);
                status = IRQ_HANDLED;
            }
        }
    }

    ULG_INT(MAKE_EP(0,0), udc->ep0_state, ULG_INT_UDC_IRQ_E, NULL);

    irq_logging(-1);
    return status;
} 

static __tcmfunc irqreturn_t mt_udc_dma_irq(int irq, void *_mt_udc)
{
    irqreturn_t status;
    unsigned int chan;
    u32 dma_intr;
    u32 cnt = 0, retry_max = (udc && udc->dbg) ? atomic_read(&udc->dbg->dma_sta_retry_max) : (5);

    status = IRQ_NONE;
    chan = 0;
    if(!udc){
        USB_ERR("invalid *udc\n");
        return status;
    }

    //printk("mt_udc_dma_irq\n");
    irq_logging(2);
    /* find the interrupt source and call the associated handler */
    dma_intr = __udc_readb(DMA_INTR);
  	mb();
   	__udc_writeb(~dma_intr, DMA_INTR);
 //   mt65xx_reg_sync_writeb(~dma_intr, DMA_INTR);

    while ( (0 != (dma_intr & __udc_readb(DMA_INTR))) && (cnt++ < retry_max)) 
    	printk("DMA interrupt don't clear, cnt is %d, retry_max is %d\n",cnt,retry_max);
        
    if ( (cnt) || (dma_intr & __udc_readb(DMA_INTR))) {
        u32 mask = dma_intr | __udc_readb(DMA_INTR);
        USB_MSG("intr not cleared[%2d]: %x, %x\n", cnt, dma_intr, __udc_readb(DMA_INTR));
        for(chan = 1; chan <= MT_CHAN_NUM; chan++){
            if(mask & CHANMASK(chan))
                USB_MSG("   dma[%d] = 0x%4x\n", chan, __udc_readl(USB_DMA_CNTL(chan)));
        }    
    }
    ULG_INT(MAKE_EP(0,0), udc->ep0_state, ULG_INT_DMA_IRQ_S, "d", __udc_readb(DMA_INTR));
    
    for(chan = 1; chan <= MT_CHAN_NUM; chan++){
        if(dma_intr & CHANMASK(chan)){
           //USB_LOG("mt_udc_dma_irq: chan %d\n", chan);
           status = mt_udc_dma_handler(chan);
        }
    }

    ULG_INT(MAKE_EP(0,0), udc->ep0_state, ULG_INT_DMA_IRQ_E, NULL);
    irq_logging(-2);
    return status;
}

/*---------------------------------------------------------------------------*/
static char *ep_str[MT_EP_NUM][USB_DIR_NUM] = {
#if 0
    {"ep0-control", NULL            },  /*ep0 rx/tx share the same endpoint*/
    {"ep1out-bulk", NULL            },
    {"ep2out-bulk", NULL            },
    {"ep3out-bulk", NULL            },
    {NULL,          "ep4in-bulk"    },
    {NULL,          "ep5in-bulk"    },
    {NULL,          "ep6in-bulk"    },
    {NULL,          "ep7in-int"     },
    {NULL,          "ep8in-int"     },
#else
    {"ep0-control", NULL            },  /*ep0 rx/tx share the same endpoint*/
    {"ep1out-bulk", "ep1in-bulk"    },  /*rndis*/
    {"ep2out-bulk", "ep2in-bulk"    },  /*usb mass storage*/
    {"ep3out-bulk", "ep3in-bulk"    },  /*adb*/
    {"ep4out-bulk", "ep4in-bulk"    },  /*cdc-acm*/
    {NULL,          NULL            },
    {NULL,          NULL            },
    {NULL,          "ep7in-int"     },
    {NULL,          "ep8in-int"     },
#endif
};
/*---------------------------------------------------------------------------*/
static struct mt_ep ep_res[MT_EP_NUM][USB_DIR_NUM];
/*---------------------------------------------------------------------------*/

/* initialize all data structures */
static void mt_gadget_init(struct device *dev)
{
    int i, j, err = 0;
    struct mt_ep* ep;

    if(!udc){
        USB_ERR("invalid *udc\n");
        return;
    }
    memset(udc->eps, 0x00, sizeof(udc->eps));  
    memset(ep_res, 0x00, sizeof(ep_res));  
    for (i = 0; i < MT_EP_NUM; i++) {
        for (j = 0; j < USB_DIR_NUM; j++) {
            ep = &ep_res[i][j];
            if (ep_str[i][j] == NULL)
                continue;
                        
            ep->ep_num = i;
            /* initialize struct usb_ep part */
            ep->ep.driver_data = udc;
            ep->ep.ops = &mt_ep_ops;
            INIT_LIST_HEAD(&ep->ep.ep_list);
            
            INIT_LIST_HEAD(&ep->queue); /* initialize request queue */
            ep->desc = NULL;
            ep->busycompleting = 0;
            ep->dir = j;
            ep->ep.name = ep_str[i][j]; 
            if (i == 0)
                ep->ep.maxpacket = MT_EP0_FIFOSIZE;
            else if (strstr(ep->ep.name, "-int")) 
                ep->ep.maxpacket = MT_INT_MAXP;
            else if (strstr(ep->ep.name, "-bulk"))
                ep->ep.maxpacket = MT_BULK_MAXP;
            else {
                USB_ERR("unknown ep type: '%s'\n", ep->ep.name);
                err = -EINVAL;
            }

            if (!err) {
                if (i == 0)
                    udc->eps[i][USB_RX] = udc->eps[i][USB_TX] = ep;
                else
                    udc->eps[i][j] = ep;
            }
        }
    }
#if 0
    /* initialize endpoint data structures */
    /* ==================================================== */
    /* Endpoint 0(default control endpoint)                 */
    /* ==================================================== */
    udc->ep[0].ep_num = 0;
    /* initialize struct usb_ep part */
    udc->ep[0].ep.driver_data = udc;
    udc->ep[0].ep.ops = &mt_ep_ops;
    INIT_LIST_HEAD(&udc->ep[0].ep.ep_list);
    udc->ep[0].ep.maxpacket = MT_EP0_FIFOSIZE;
    INIT_LIST_HEAD(&udc->ep[0].queue); /* initialize request queue */
    udc->ep[0].desc = NULL;
    udc->ep[0].busycompleting = 0;

    /* ==================================================== */
    /* Endpoint 1 ~ 8                                       */
    /* ==================================================== */
    for(i = 1; i < MT_EP_NUM; i++){
        udc->ep[i].ep_num = i;
        /* initialize struct usb_ep part */
        udc->ep[i].ep.driver_data = udc;
        udc->ep[i].ep.ops = &mt_ep_ops;
        INIT_LIST_HEAD(&udc->ep[i].ep.ep_list);
        udc->ep[i].ep.maxpacket = MT_BULK_MAXP;
        INIT_LIST_HEAD(&udc->ep[i].queue); /* initialize request queue */
        //printk("ep %d, queue = %x\n", i, &udc->ep[i].queue);
        udc->ep[i].desc = NULL;
        udc->ep[i].busycompleting = 0;
    }
    udc->ep[MT_EP_NUM - 2].ep.maxpacket = MT_INT_MAXP;
    udc->ep[MT_EP_NUM - 1].ep.maxpacket = MT_INT_MAXP;
    udc->ep[0].ep.name = "ep0-control";
    udc->ep[1].ep.name = "ep1out-bulk";
    udc->ep[2].ep.name = "ep2out-bulk";
    udc->ep[3].ep.name = "ep3out-bulk";
    udc->ep[4].ep.name = "ep4in-bulk";
    udc->ep[5].ep.name = "ep5in-bulk";
    udc->ep[6].ep.name = "ep6in-bulk";
    udc->ep[7].ep.name = "ep7in-int";
    udc->ep[8].ep.name = "ep8in-int";
    memset(udc->eps, 0x00, sizeof(udc->eps));
    udc->eps[0][USB_RX] = udc->eps[0][USB_TX] = &udc->ep[0];
    udc->eps[1][USB_RX] = &udc->ep[1];
    udc->eps[2][USB_RX] = &udc->ep[2];
    udc->eps[3][USB_RX] = &udc->ep[3];
    udc->eps[4][USB_TX] = &udc->ep[4];
    udc->eps[5][USB_TX] = &udc->ep[5];
    udc->eps[6][USB_TX] = &udc->ep[6];
    udc->eps[7][USB_TX] = &udc->ep[7];
    udc->eps[8][USB_TX] = &udc->ep[8];
#endif
    
    /* initialize udc data structure */
    udc->gadget.ops = &mt_gadget_ops;
    udc->gadget.ep0 = &udc->eps[0][USB_RX]->ep;
    INIT_LIST_HEAD(&udc->gadget.ep_list);
    udc->gadget.speed = USB_SPEED_UNKNOWN;
    udc->gadget.is_dualspeed = 1;
    udc->gadget.is_otg = 0;
    udc->gadget.is_a_peripheral = 1;
    udc->gadget.b_hnp_enable = 0;
    udc->gadget.a_hnp_support = 0;
    udc->gadget.a_alt_hnp_support = 0;
    udc->gadget.name = driver_name;

    udc->ep0_state = EP0_IDLE;
    udc->faddr = 0;
    udc->fifo_addr = FIFO_START_ADDR;
    udc->driver = NULL;
    udc->set_address = 0;
    udc->test_mode = 0;
    udc->test_mode_nr = 0;
    udc->power = USB_FALSE;
    udc->ready = USB_FALSE;
    udc->charger_type = CHARGER_UNKNOWN;
    udc->udc_req = kmalloc(sizeof(struct mt_req), GFP_KERNEL);
    memset(udc->udc_req, 0, sizeof(struct mt_req));
    udc->udc_req->signature = USB_REQ_SIGNATURE;
    udc->udc_req->req.buf = kmalloc(2, GFP_KERNEL);

    device_initialize(&udc->gadget.dev);
    dev_set_name(&udc->gadget.dev, "mtk_usb");

    spin_lock_init(&udc->lock);

    /* build ep_list structures */
    for (i = 0; i < MT_EP_NUM; i++) {
        for (j = 0; j < USB_DIR_NUM; j++) {
            if (!ep_res[i][j].ep.name)
                continue;
            ep = &ep_res[i][j];
            list_add_tail(&ep->ep.ep_list, &udc->gadget.ep_list);
            //USB_LOG("[USB]: ep[%d][%d].name='%s', max=%d\n", i, j, ep->ep.name, ep->ep.maxpacket);
        }
    }
    atomic_set(&udc->cmode, CABLE_MODE_NORMAL);
    udc->dbg = &g_udc_dbg;
    mt_udc_setup_debug_callback(udc->dbg);
    mt_udc_create_attr(dev);
    dev_set_drvdata(dev, udc);


    return;
}

/* ============================================== */
/* callback functions registered to mt_udc_driver */
/* ============================================== */

static int mt_udc_probe(struct device *dev)
{
    int status;


    udc = &udc_global;
   // udc_gadget = &udc->gadget;
   // udc = kmalloc(sizeof(struct mt_udc), GFP_KERNEL);
    if(!udc){
        USB_ERR("udc alloation failed\n");
        return -1;
    }
    printk("USB UDC PROBE\n\n");
    UDC_ENABLE_CLOCK(TRUE);

    memset(udc, 0, sizeof(struct mt_udc));

    mt_gadget_init(dev);

    __udc_readw(INTRTX);
    __udc_writew(0, INTRRX);
    __udc_readb(INTRUSB);
    __udc_writew(0, INTRTXE);
    __udc_writew(0, INTRRXE);
    __udc_writeb(0, INTRUSBE);
    __udc_writeb(0, DMA_INTR);

    /* request irq line for both USB and USB_DMA */
    status = request_irq(MT6573_USB_MC_IRQ_LINE, (irq_handler_t)mt_udc_irq, IRQF_DISABLED, "MT6573_USB", NULL);

    if(status)
    {
        kfree(udc);
        udc = NULL;
        return -1;
    }
    
    status = request_irq(MT6573_USB_DMA_IRQ_LINE, (irq_handler_t)mt_udc_dma_irq, IRQF_DISABLED, "MT6573_USB_DMA", \
    NULL);

    if(status)
    {
        kfree(udc);
        udc = NULL;
        return -1;
    }

    udc->ready = USB_TRUE;

    wake_lock_init(&usb_lock, WAKE_LOCK_SUSPEND, "USB suspend lock");

    return status;
}

static void mt_dev_connect()
{
    int i;
    u8  tmpReg8;

    printk("mt_dev_connect(try)\n");
    ULG_FUC(MAKE_EP(0,0), 0x00, ULG_FUC_DEV_CONN_TRY, NULL);
    if (!udc || (udc->power == USB_TRUE))
        return;
    if (atomic_read(&udc->cmode) == CABLE_MODE_CHRG_ONLY) {
        BATTERY_SetUSBState(USB_CONFIGURED);
        wake_up_bat();
        //if (!wake_lock_active(&usb_lock))
        //    wake_lock(&usb_lock);            
        udc->power = TRUE;    
        USB_MSG("conn:chgr(%d)\n", wake_lock_active(&usb_lock));
        return;
    } else {
        USB_MSG("conn(%d)\n", wake_lock_active(&usb_lock));
    }

    printk("mt_dev_connect(success)\n");

		__udc_readb(USB_BASE+0xff); // read 0xff reguster before read phy register due to hardware issue
    mt6573_usb_phy_recover(udc);
		UDC_ENABLE_CLOCK(TRUE);
    __udc_writew(0x0, INTRTXE);
    __udc_writew(0x0, INTRRXE);
    __udc_writeb(0x0, INTRUSBE);
    
    __udc_readw(INTRTX);
    __udc_writew(0, INTRRX);
    __udc_readb(INTRUSB);

    /* reset INTRUSBE */
    __udc_writeb(INTRUSB_RESET, INTRUSBE);

    /* connect */
    tmpReg8 = __udc_readb(POWER);
    tmpReg8 |= PWR_HS_ENAB;
    #ifdef USB_FORCE_FULL_SPEED
    tmpReg8 &= ~PWR_HS_ENAB;
    #else
    tmpReg8 |= PWR_HS_ENAB;
    #endif
    tmpReg8 |= PWR_SOFT_CONN;
    tmpReg8 |= PWR_ENABLE_SUSPENDM;
    __udc_writeb(tmpReg8, POWER);

    for(i = 0; i < MT_CHAN_NUM; i++){
        memset(&ep_dma[i], 0x00, sizeof(ep_dma[i]));
        ep_dma[i].chan = i+1;
    }
    ULG_FUC(MAKE_EP(0,0), 0x00, ULG_FUC_DEV_CONN_OK, NULL);
    return;
}

static void mt_dev_disconnect()
{
    int i, j;
    struct mt_ep *ep0;
    u8 tmpReg8;

    printk("mt_dev_disconnect(try)\n");
    ULG_FUC(MAKE_EP(0,0), 0x00, ULG_FUC_DEV_DISC_TRY, NULL);
    if(!udc || (udc->power == USB_FALSE))
        return;
    
    if (atomic_read(&udc->cmode) == CABLE_MODE_CHRG_ONLY) {
        BATTERY_SetUSBState(USB_SUSPEND);
        wake_up_bat();
        if (wake_lock_active(&usb_lock))
            wake_unlock(&usb_lock);            
        udc->power = FALSE;    
        USB_MSG("disc:chgr:(%d)\n", wake_lock_active(&usb_lock));
        return;
    } else {
        USB_MSG("disc(%d)\n", wake_lock_active(&usb_lock));
    }

    printk("mt_dev_disconnect(success)\n");

    ep0 = udc->eps[0][USB_RX];

    /* reinitialize ep0 */
    mt_ep_fifo_flush_internal(&ep0->ep);
    
    while(!list_empty(&ep0->queue)){
        struct mt_req *req0 = container_of(ep0->queue.next, struct mt_req, queue);
        CHECKREQ(ep0, req0);
        list_del(&req0->queue);
        req0->queued--;
    }

    /* disconnect */
    tmpReg8 = __udc_readb(POWER);
    tmpReg8 &= ~PWR_SOFT_CONN;
    __udc_writeb(tmpReg8, POWER);
    
    if(udc->driver)
        udc->driver->disconnect(&udc->gadget);
    
    udc->faddr = 0;
    udc->fifo_addr = FIFO_START_ADDR;
    udc->ep0_state = EP0_IDLE;
    //printk("3\n");
    mt_ep_fifo_flush_internal(&udc->eps[0][USB_RX]->ep);
    for (i = 1; i < MT_EP_NUM; i++)
        for (j = USB_RX; j < USB_DIR_NUM; j++)
            if (udc->eps[i][j])
                mt_ep_fifo_flush_internal(&udc->eps[i][j]->ep);

    __udc_readw(INTRTX);
    __udc_writew(0, INTRRX);
    __udc_readb(INTRUSB);
    
     /* diable interrupts */
    __udc_writew(0, INTRTXE);
    __udc_writew(0, INTRRXE);
    __udc_writeb(INTRUSB_RESET, INTRUSBE);

    /* reset function address to 0 */
    __udc_writeb(0, FADDR);
    //printk("4\n");

    /* clear all dma channels */
    for(i = 1; i <= MT_CHAN_NUM; i++){
        __udc_writew(0, USB_DMA_CNTL(i));
        __udc_writel(0, USB_DMA_ADDR(i));
        __udc_writel(0, USB_DMA_COUNT(i));
    }
		UDC_ENABLE_CLOCK(FALSE);
		__udc_readb(USB_BASE+0xff); // read 0xff reguster before read phy register due to hardware issue
    mt6573_usb_phy_savecurrent(udc);
    udc->gadget.speed = USB_SPEED_UNKNOWN;
    ULG_FUC(MAKE_EP(0,0), 0x00, ULG_FUC_DEV_DISC_OK, NULL);
    if (wake_lock_active(&usb_lock))
    	{
    			wake_unlock(&usb_lock); 
    		wake_lock_timeout(&usb_lock, 5 * HZ);	
    		}
   
    return;
}

static int mt_udc_remove(struct device *_dev)
{
    if(!_dev){
        USB_ERR("invalid *_dev\n");
        return -1;
    }
    
    if(!udc){
        USB_ERR("invalid *udc\n");
        return -1;
    }

    /* reset the function address to zero */
    __udc_writeb(0, FADDR);

    if(udc->driver){
        usb_gadget_unregister_driver(udc->driver);
    }
    else{
        USB_ERR("invalid udc->driver\n");
        return -1;
    }

    free_irq(MT6573_USB_MC_IRQ_LINE, udc);
    free_irq(MT6573_USB_DMA_IRQ_LINE, udc);

    dev_set_drvdata(_dev, NULL);

    udc = NULL;

    return 0;
}

void mt_udc_shutdown(struct device *dev){

    mt_usb_disconnect();
    
    return;
}

/* do nothing due to usb power management is done by other way */
static int mt_udc_suspend(struct device *dev, pm_message_t state)
{
    return 0;
}

/* do nothing due to usb power management is done by other way */
static int mt_udc_resume(struct device *dev)
{
    return 0;
}

/* ================================ */
/* device driver for the mt_udc */
/* ================================ */

static struct platform_driver mt_udc_driver =
{
    .driver     = {
        .name = (char *)driver_name,
        .bus = &platform_bus_type,
        .probe = mt_udc_probe,
        .remove = mt_udc_remove,
        .shutdown = mt_udc_shutdown,
        .suspend = mt_udc_suspend,
        .resume = mt_udc_resume,
    }    
};

/* ===================================================== */
/* handlers which service interrupts originated from USB */
/* ===================================================== */

/* if bit 0 of INTRTX is set when USB issues an interrupt, this function will */
/* be called to handler it. */
static void mt_ep0_handler()
{
    u16 csr0;
    u8 index;
    u8 has_error = 0;

    if(!udc){
        USB_ERR("invalid *udc\n");
        return;
    }

    index = __udc_readb(INDEX);
    __udc_writeb(0, INDEX);

    csr0 = __udc_readw(IECSR + CSR0);

    ULG_FUC(MAKE_EP(0,0), udc->ep0_state, ULG_FUC_EP0_HANDLER, "d", csr0);
    if(csr0 & EP0_SENTSTALL){
        USB_ERR("EP0: STALL\n");
        /* needs to implement exception handling here */
        __udc_writew(csr0 &~ EP0_SENTSTALL, IECSR + CSR0);
        udc->ep0_state = EP0_IDLE;
        has_error = 1;
        ULG_FUC(MAKE_EP(0,0), udc->ep0_state, ULG_FUC_EP0_SENTSTALL, NULL);
    }

    if(csr0 & EP0_SETUPEND){
        USB_ERR("EP0: SETUPEND\n");
        __udc_writew(csr0 | EP0_SERVICE_SETUP_END, IECSR + CSR0);
        udc->ep0_state = EP0_IDLE;
        has_error = 1;
        ULG_FUC(MAKE_EP(0,0), udc->ep0_state, ULG_FUC_EP0_SETUPEND, NULL);
    }

    if(has_error){
        if(!(csr0 & EP0_RXPKTRDY)){
            ULG_FUC(MAKE_EP(0,0), udc->ep0_state, ULG_FUC_EP0_NORXPKT, NULL);
            return;
        }
    }

    switch(udc->ep0_state){
        case EP0_IDLE:
            if(udc->set_address){
                udc->set_address = 0;
                __udc_writeb(udc->faddr, FADDR);
                __udc_writeb(index, INDEX);
                ULG_FUC(MAKE_EP(0,0), udc->ep0_state, ULG_FUC_SET_FADDR, "d", udc->faddr);
                return;
            }

            if(udc->test_mode){

                BATTERY_SetUSBState(USB_SUSPEND);
                wake_up_bat();
                
                if(udc->test_mode_nr == USB_TST_TEST_PACKET);
                    mt_usb_load_testpacket();

                __udc_writeb(udc->test_mode_nr, TESTMODE);
                USB_MSG("Enter test mode: %d\n", udc->test_mode_nr);
                ULG_FUC(MAKE_EP(0,0), udc->ep0_state, ULG_FUC_TESTMODE_S, "d", udc->test_mode_nr);
                return;
            }

            if(__udc_readw(IECSR + RXCOUNT) == 0){
                /* the transfer has completed, this is a confirmation */
                __udc_writeb(index, INDEX);
                ULG_FUC(MAKE_EP(0,0), udc->ep0_state, ULG_FUC_EP0_COMPLETE, NULL);                
                return;
            }
            
            mt_ep0_idle();
            break;
        case EP0_TX:
            mt_ep0_tx();
            break;
        case EP0_RX:
            mt_ep0_rx();
            break;
        default:
            USB_ERR("Unknown EP0 State\n");
            break;
    }
    __udc_writeb(index, INDEX);

    return;
}

/* called when an interrupt is issued and the state of endpoint 0 is EP0_IDLE.*/
/* this function is called when a packet is received in setup stage.          */
/* must decode the command and prepare for the request sent, and change ep0   */
/* state accordingly */
static void mt_ep0_idle()
{
    int count;//, i;
    int retval = -1;
    u8 type;
    u16 csr0;
    u8 index;
    u8 recip; // the recipient maybe a device, interface or endpoint
    u8 ep_num;
    struct mt_ep *ep = NULL;
    struct mt_req *req = NULL;
    u8 is_in;
    u16 csr;
    u8 tmp, result[2] = {0, 0};
    u8 udc_handled = 0;
    
    union u{
        u8    word[8];
        struct usb_ctrlrequest r;
    }ctrlreq;

    if(!udc){
        USB_ERR("invalid *udc\n");
        return;
    }

    /* read the request from fifo */
    for(count = 0; count < 8; count++)
    {
        ctrlreq.word[count] = __udc_readb(FIFO(0));
    }
    ULG_BUF(0x00, udc->ep0_state, ULG_BUF_EP_RX, 8, ctrlreq.word);
#define	w_value		le16_to_cpup (&ctrlreq.r.wValue)
#define	w_index		le16_to_cpup (&ctrlreq.r.wIndex)
#define	w_length	le16_to_cpup (&ctrlreq.r.wLength)
    
    if(ctrlreq.r.bRequestType & USB_DIR_IN){
        udc->ep0_state = EP0_TX;
    }
    else{
        udc->ep0_state = EP0_RX; // request without data stage will be picked 
        // out later
    }

    type = ctrlreq.r.bRequestType;
    type &= USB_TYPE_MASK;

    recip = ctrlreq.r.bRequestType & USB_RECIP_MASK;
    ULG_FUC(MAKE_EP(0,0), udc->ep0_state, ULG_FUC_EP0_IDLE_S, "dddd", ctrlreq.r.bRequestType << 8 | ctrlreq.r.bRequest , w_index, w_value, w_length);
    switch(type){
        case USB_TYPE_STANDARD:
            /* update ep0 state according to the request */
            switch(ctrlreq.r.bRequest){
                case USB_REQ_SET_ADDRESS:
                    udc->faddr = w_value;
                    udc->set_address = 1;
                    ULG_FUC(MAKE_EP(0,0), udc->ep0_state, ULG_FUC_GET_FADDR, "d", udc->faddr);
                    retval = 1;
                case USB_REQ_CLEAR_FEATURE:
                    switch(recip){
                        case USB_RECIP_DEVICE:
                            break;
                        case USB_RECIP_INTERFACE:
                            break;
                        case USB_RECIP_ENDPOINT:
                            ep_num = w_index & 0x0f;
                            if((ep_num == 0) || (ep_num > MT_EP_NUM) || (w_value != USB_ENDPOINT_HALT))
                                break;

                            is_in = (u8)w_index & USB_DIR_IN;
                            if(is_in)
                                ep = udc->eps[ep_num][USB_TX];
                            else
                                ep = udc->eps[ep_num][USB_RX];
                            if (!ep) {
                                retval = -EINVAL;
                                break;
                            }
                            if(ep -> wedge == 1)
                          	{
                          		 retval = 1;
                          		 printk("ep is sswedge %d\n",__udc_readw(IECSR + CSR0));
                          		 __udc_writeb(0, INDEX);
                          		 break;
                          		}
                            mt_ep_set_halt(&ep->ep,0);

                            __udc_writeb(0, INDEX);
                            retval = 1;
                                
                            break;
                    }
                case USB_REQ_SET_CONFIGURATION:
                case USB_REQ_SET_INTERFACE:
                    udc->ep0_state = EP0_IDLE; /* standard requests with no 
                    data stage */
                    break;
                case USB_REQ_SET_FEATURE:
                    udc->ep0_state = EP0_IDLE;
                    switch(recip){
                        case USB_RECIP_DEVICE:
                            if(udc->gadget.speed != USB_SPEED_HIGH)
                                break;
                            if(w_index & 0xff)
                                break;
                            switch(w_index >> 8){
                                case 1:
                                    USB_MSG("Try to enter test mode: TEST_J\n");
                                    udc->test_mode_nr = USB_TST_TEST_J;
                                    retval = 1;
                                    break;
                                case 2:
                                    USB_MSG("Try to enter test mode: TEST_K\n");
                                    udc->test_mode_nr = USB_TST_TEST_K;
                                    retval = 1;
                                    break;
                                case 3:
                                    USB_MSG("Try to enter test mode: SE0_NAK\n");
                                    udc->test_mode_nr = USB_TST_SE0_NAK;
                                    retval = 1;
                                    break;
                                case 4:
                                    USB_MSG("Try to enter test mode: TEST_PACKET\n");
                                    udc->test_mode_nr = USB_TST_TEST_PACKET;
                                    retval = 1;
                                    break;
                                default:
                                    break;
                            }
                            if(retval >= 0)
                                udc->test_mode = 1;

                            break;
                        case USB_RECIP_INTERFACE:
                            break;
                        case USB_RECIP_ENDPOINT:
                            // Only handle USB_ENDPOINT_HALT for now

                            ep_num = w_index & 0x0f;
                            
                            if((ep_num == 0) || (ep_num > MT_EP_NUM) || (w_value != USB_ENDPOINT_HALT))
                                break;
                                
                            is_in = w_index & USB_DIR_IN;
                            
                            __udc_writeb(ep_num, INDEX);

                            if(is_in){
                                csr = __udc_readw(IECSR + TXCSR);
                                if(csr & EPX_TX_FIFONOTEMPTY)
                                    csr |= EPX_TX_FLUSHFIFO;
                                csr |= EPX_TX_SENDSTALL;
                                csr |= EPX_TX_CLRDATATOG;
                                /* write 1 ignore, write 0 clear bits */
                                csr |= EPX_TX_WZC_BITS;
                                
                                __udc_writew(csr, IECSR + TXCSR);
                            }
                            else{
                                csr = __udc_readw(IECSR + RXCSR);

                                csr |= EPX_RX_SENDSTALL;
                                csr |= EPX_RX_FLUSHFIFO;
                                csr |= EPX_RX_CLRDATATOG;
                                csr |= EPX_RX_WZC_BITS;

                                __udc_writew(csr, IECSR + RXCSR);
                            }
                            
                            __udc_writeb(0, INDEX);
                            
                            retval = 1;
                            break;
                        default:
                            break;
                    }
                case USB_REQ_GET_STATUS:
                        req = udc->udc_req;
                        switch(recip){
                            case USB_RECIP_DEVICE:
                                result[0] = 1 << USB_DEVICE_SELF_POWERED;
                                // WE DON'T SUPPORT REMOTE WAKEUP
                                retval = 1;
                                break;
                            case USB_RECIP_INTERFACE:
                                result[0] = 0;
                                retval = 1;
                                break;
                            case USB_RECIP_ENDPOINT:
                                ep_num = (u8)w_index & 0x0f;
                                if(!ep_num){
                                    result[0] = 0;
                                    retval = 1;
                                    break;
                                }

                                is_in = (u8)w_index & USB_DIR_IN;
                                if(is_in)
                                    ep = udc->eps[ep_num][USB_TX];
                                else
                                    ep = udc->eps[ep_num][USB_RX];
 
                                if(ep_num > MT_EP_NUM || !ep || !ep->desc){
                                    retval = -EINVAL;
                                    break;
                                } 
                                                                
                                __udc_writeb(ep_num, INDEX);

                                if(is_in){
                                    tmp = __udc_readw(IECSR + TXCSR) & EPX_TX_SENDSTALL;
                                }
                                else{
                                    tmp = __udc_readw(IECSR + RXCSR) & EPX_RX_SENDSTALL;
                                }

                                result[0] = tmp ? 1 : 0;


                                __udc_writeb(0, INDEX);
                                 
                                retval = 1;
                                break;
                            default:
                                break;
                        }

                        if(retval > 0){
                            req->req.length = w_length;

                            if(req->req.length > 2)
                                req->req.length = 2;

                            memcpy(req->req.buf, result, w_length);
                            udc_handled = 1;                            
                        }
                        
                        break;

                default:
                    break;
            }
            break;
        case USB_TYPE_CLASS:
            switch(ctrlreq.r.bRequest){
                case USB_CDC_REQ_SET_CONTROL_LINE_STATE: /* no data stage */
                case 0xff:
                	msc_test = TRUE;
                  udc->ep0_state = EP0_RX_STATUS;
                default:
                    break;
            }
            break;
        case USB_TYPE_VENDOR:
            USB_MSG("Vendor specific command\n");
            break;
        case USB_TYPE_RESERVED:
            USB_MSG("Reserved command\n");
            break;
        default:
            break;
    }
		    index = __udc_readb(INDEX);
    __udc_writeb(0, INDEX);

    csr0 = __udc_readw(IECSR + CSR0);
    /* if no data stage */
    if(udc->ep0_state == EP0_IDLE){
        csr0 |= (EP0_SERVICED_RXPKTRDY | EP0_DATAEND);
    }
    else if(udc->ep0_state != EP0_RX_STATUS){
        csr0 |= EP0_SERVICED_RXPKTRDY;
    }

    __udc_writew(csr0, IECSR + CSR0);	


    ULG_FUC(MAKE_EP(0,0), udc->ep0_state, ULG_FUC_EP0_IDLE_E, "dd", csr0, udc_handled);

    if(retval >= 0){
        // do not forward to gadget driver if the request is already handled
        if(udc_handled)
            mt_ep_enqueue(&udc->eps[0][USB_RX]->ep, &req->req, GFP_ATOMIC); 

        udc_handled = 0;
    }
    else if(!((type == USB_TYPE_STANDARD) && (ctrlreq.r.bRequest == USB_REQ_SET_ADDRESS))){
        retval = udc->driver->setup(&udc->gadget, &ctrlreq.r);
    }

    if(ctrlreq.r.bRequest == USB_REQ_SET_CONFIGURATION){
        if(ctrlreq.r.wValue & 0xff){
            BATTERY_SetUSBState(USB_CONFIGURED);
        }
        else{
            BATTERY_SetUSBState(USB_UNCONFIGURED);
        }
        wake_up_bat();
    }
    
    if(retval < 0){
        //not supported by gadget driver;
        USB_MSG("Request from Host is not supported, retval = %d!!\n", retval);
        USB_MSG("bRequestType = %x\n", ctrlreq.r.bRequestType);
        USB_MSG("bRequest = %x\n", ctrlreq.r.bRequest);
        USB_MSG("w_value = %x\n", w_value);
        USB_MSG("w_index = %x\n", w_index);
        USB_MSG("w_length = %x\n", w_length);
        #if 0
        for(i = 0; i < MT_EP_NUM; i++)
        {
            mt_ep_fifo_flush_internal(&udc->ep[i].ep);
        }
        #endif
        mt_ep_fifo_flush_internal(&udc->eps[0][USB_RX]->ep);

        __udc_writew(EP0_SENDSTALL, IECSR + CSR0);
        USB_MSG("EP0 SENDSTALL\n");
    }
    __udc_writeb(index, INDEX);
   
    return;
}

/* called when an interrupt is issued and the state of endpoint 0 is EP0_RX */
static void mt_ep0_rx()
{
    struct mt_ep *ep;
    struct mt_req *req;
    int count = 0;
    u8 index;
    u16 csr0;

    //printk("EP0_RX\n");
    if(!udc){
        USB_ERR("invalid *udc\n");
        return;
    }
    
    ep = udc->eps[0][USB_RX];

    if(!ep){
        USB_ERR("invalid *ep\n");
        return;
    }

    if(list_empty(&ep->queue)){
        USB_ERR("SW buffer is not ready!!\n");
        return;
    }

    req = container_of(ep->queue.next, struct mt_req, queue);
    CHECKREQ(ep, req);
    if(req){
        index = __udc_readb(INDEX);
        __udc_writeb(0, INDEX);

        csr0 = __udc_readw(IECSR + CSR0);
        
        count = mt_udc_read_fifo(ep);
        if(count < ep->ep.maxpacket){   
            done(ep, req, 0);
            udc->ep0_state = EP0_IDLE;
            csr0 |= EP0_DATAEND;
        }
        csr0 |= EP0_SERVICED_RXPKTRDY;
        __udc_writew(csr0, IECSR + CSR0);
        
        __udc_writeb(index, INDEX);
        ULG_FUC(MAKE_EP(0,0), udc->ep0_state, ULG_FUC_EP0_RX, "dd", count, csr0);        
    }
    else{
        USB_ERR("NO REQUEST\n");
        ULG_FUC(MAKE_EP(0,0), udc->ep0_state, ULG_FUC_EP0_RX, "d", 0);        
    }

    return;
}

/* called when an interrupt is issued and the state of endpoint 0 is EP0_TX */
static void mt_ep0_tx()
{
    struct mt_ep *ep;
    struct mt_req *req;
    int count = 0;
    u8 index;
    u16 csr0;
    
    //printk("EP0_TX\n");
    if(!udc){
        USB_ERR("invalid *udc\n");
        return;
    }

    ep = udc->eps[0][USB_RX];

    if(!ep){
        USB_ERR("invalid *ep\n");
        return;
    }

    if(list_empty(&ep->queue)){
        USB_ERR("SW buffer is not ready!!\n");
        return;
    }
    
    
    req = container_of(ep->queue.next, struct mt_req, queue);
    CHECKREQ(ep, req);
    if(req){
        index = __udc_readb(INDEX);
        __udc_writeb(0, INDEX);

        csr0 = __udc_readw(IECSR + CSR0);  
        count = mt_udc_write_fifo(ep);
        if(count < ep->ep.maxpacket){
            done(ep, req, 0);
            csr0 |= EP0_DATAEND;
            udc->ep0_state = EP0_IDLE;
        }
        csr0 |= EP0_TXPKTRDY;
       
        __udc_writew(csr0, IECSR + CSR0);
        
        __udc_writeb(index, INDEX);
        ULG_FUC(MAKE_EP(0,0), udc->ep0_state, ULG_FUC_EP0_TX, "dd", count, csr0);
    }
    else{
        USB_ERR("NO REQUEST\n");
        ULG_FUC(MAKE_EP(0,0), udc->ep0_state, ULG_FUC_EP0_TX, "d", 0);        
    }
    
    return;
}

/* if any bit of INTRTX except for bit 0 is set, then this function will be */
/* called with the second parameter being USB_TX. if any bit of INTRRX is set */
/* then this function will be called with the second parameter being USB_RX */
static void mt_epx_handler(int ep_num, USB_DIR dir)
{
    struct mt_req *req;
    struct mt_ep *ep;
    int count;
    u8 index;
    u16 csr;
    
    req = NULL;
  
    if(!udc){
        USB_ERR("invalid *udc\n");
        return;
    } else if (!USB_VALID_EP(ep_num, dir)) {
        USB_ERR("invalid argument: %d %d\n", ep_num, dir);
        return;
    } else if (!udc->eps[ep_num][dir]) {
        USB_ERR("null: %d %d\n", ep_num, dir); 
        return;
    }

    ep = udc->eps[ep_num][dir];
    req = container_of(ep->queue.next, struct mt_req, queue);
    if(!ep || !req){
        USB_ERR("invalid *ep or *req\n");
        return;
    }

    if(!list_empty(&ep->queue)){
    	CHECKREQ(ep, req);
        index = __udc_readb(INDEX);
        __udc_writeb(ep_num, INDEX);

        if(dir == USB_TX){
            csr = __udc_readw(IECSR + TXCSR);
            if(csr & EPX_TX_TXPKTRDY){
                USB_ERR("EP%d: TX Packet not transferred: %d %d %d %d\n", ep_num, req->req.length, req->req.actual, 
                        req->req.zero, atomic_read(&req->zlp_sent));
                ULG_FUC(MAKE_EP(ep_num, dir), udc->ep0_state, ULG_FUC_TX_NOT_TRANS, "dddd",
                        req->req.length, req->req.actual, req->req.zero, atomic_read(&req->zlp_sent));
            }
            
            if (ep->dma) { /*dma mode*/
            #if defined USB_USE_DMA_MODE0
                count = min((unsigned int)ep->ep.maxpacket, req->req.length - req->req.actual);
                ULG_FUC(MAKE_EP(ep->ep_num, ep->dir), udc->ep0_state, ULG_FUC_EP_TX, "dddd", csr, req->req.length, req->req.actual, count);
                if((0 < count)  && (count <= ep->ep.maxpacket)){
                    mt_usb_config_dma(ep, req, (u32)(req->req.buf + req->req.actual), count);
                } else if ((req->req.length == 0) && (req->req.zero)) {
                    mt_usb_config_dma(ep, req, (u32)(req->req.buf + req->req.actual), count);
                    USB_MSG("EP%d: zero request\n", ep_num);
                } else if ((0 == (req->req.length % ep->ep.maxpacket)) &&
                           (1 == atomic_inc_return(&req->zlp_sent)) && (req->req.zero)) {
                    mt_usb_config_dma(ep, req, (u32)(req->req.buf + req->req.actual), count);
                } else if(count == 0){
                    /* this is the confirmation of the tx packet, nothing needs to 
                    be done */
                    atomic_set(&req->zlp_sent, 0);
                    done(ep, req, 0);
                    if(!list_empty(&ep->queue)){
                        mt_epx_handler(ep_num, USB_TX);
                    }
                }
            #elif defined USB_USE_DMA_MODE1
                done(ep, req, 0);
                
                if(!list_empty(&ep->queue)){
                    req = container_of(ep->queue.next, struct mt_req, queue);
                    CHECKREQ(ep, req);
                    csr |= EPX_TX_AUTOSET | EPX_TX_DMAREQEN | EPX_TX_DMAREQMODE;
                    __udc_writew(csr, IECSR + TXCSR);
                    mt_usb_config_dma(ep, req, (u32)req->req.buf, req->req.length);
                }
            #endif    
            }else { /*PIO mode*/
                count = mt_udc_write_fifo(ep);
                if((0 < count)  && (count <= ep->ep.maxpacket)){
                    csr |= EPX_TX_TXPKTRDY;
                    
                    __udc_writew(csr, IECSR + TXCSR);
                }
                else if(count == 0){
                    /* this is the confirmation of the tx packet, nothing needs to 
                    be done */
                    
                    done(ep, req, 0);
                    
                    __udc_writeb(index, INDEX);
                    
                    if(!list_empty(&ep->queue)){
                        mt_epx_handler(ep_num, USB_TX);
                    }

                }
            }
        }
        else if(dir == USB_RX){

            csr = __udc_readw(IECSR + RXCSR);                 
            if(csr & EPX_RX_RXPKTRDY){

                if (ep->dma) { /*dma mode*/
                #if defined USB_USE_DMA_MODE0
                    if(!list_empty(&ep->queue)){
                        count = min((u32)__udc_readw(IECSR + RXCOUNT), req->req.length - req->req.actual);
                        ULG_FUC(MAKE_EP(ep->ep_num, ep->dir), udc->ep0_state, ULG_FUC_EP_RX, "dddd", csr, req->req.length, req->req.actual, count);
                        mt_usb_config_dma(ep, req, (u32)req->req.buf + req->req.actual, count);
                    }
                    else{
                        USB_ERR("SW buffer is not ready for rx transfer!!\n");
                    }
                #elif defined USB_USE_DMA_MODE1
                    if(!list_empty(&ep->queue)){
                        mt_usb_config_dma(ep, req, (u32)req->req.buf + req->req.actual, min((u32)__udc_readw(IECSR + RXCOUNT), req->req.length - req->req.actual));
                    }
                    else{
                        USB_ERR("SW buffer is not ready for rx transfer!!\n");
                    }
                #endif
                } else { /*PIO mode*/
                    count = mt_udc_read_fifo(ep);
                    
                    if((count < ep->ep.maxpacket) || (req->req.length == req->req.actual)){
                        done(ep, req, 0);
                        if(!list_empty(&ep->queue)){
                            csr &= ~EPX_RX_RXPKTRDY;
                            __udc_writew(csr, IECSR + RXCSR);
                        }
                    }
                    else{
                        csr &= ~EPX_RX_RXPKTRDY;
                        __udc_writew(csr, IECSR + RXCSR);
                    }
                }
            }
        }

        __udc_writeb(index, INDEX);
    }
    
    return;
}

/* reset the device */
static void mt_usb_reset()
{    
    int i, j;
    u8 tmp8;
    u16 swrst;
    struct mt_ep *ep;
        
    if(!udc){
        USB_ERR("invalid *udc\n");
        return;
    }

    if(!udc->test_mode){
        BATTERY_SetUSBState(USB_UNCONFIGURED);
        wake_up_bat();
    }
    //printk("USB RESET\n");
    /* enable all system interrupts, but disable all endpoint interrupts */
    __udc_writew(0, INTRTXE);
    __udc_writew(0, INTRRXE);
    __udc_writeb(0, INTRUSBE);

    mt_ep_fifo_flush_internal(&udc->eps[0][USB_RX]->ep);
    for (i = 1; i < MT_EP_NUM; i++)
        for (j = USB_RX; j < USB_DIR_NUM; j++)
            if (udc->eps[i][j])
                mt_ep_fifo_flush_internal(&udc->eps[i][j]->ep);

     /* clear all dma channels */
    for(i = 1; i <= MT_CHAN_NUM; i++){
        __udc_writew(0, USB_DMA_CNTL(i));
        __udc_writel(0, USB_DMA_ADDR(i));
        __udc_writel(0, USB_DMA_COUNT(i));
    }

    __udc_readw(INTRTX);
    __udc_writew(0, INTRRX);
    __udc_readb(INTRUSB);
    __udc_writeb(0, DMA_INTR);
    /* ip software reset */
    swrst = __udc_readw(SWRST);
    swrst |= (SWRST_DISUSBRESET | SWRST_SWRST);
    __udc_writew(swrst, SWRST);
    __udc_writeb((INTRUSB_SUSPEND | INTRUSB_RESUME | INTRUSB_RESET | 
    INTRUSB_DISCON), INTRUSBE);

    /* flush endpoint 0 fifo */
    __udc_writeb(0, INDEX);
    __udc_writew(EP0_FLUSH_FIFO, IECSR + CSR0);

    udc->ep0_state = EP0_IDLE;
    udc->faddr = 0;
    udc->fifo_addr = FIFO_START_ADDR;
    //udc->test_mode = 0;

    for (i = 1; i < MT_EP_NUM; i++) {
        for (j = USB_RX; j < USB_DIR_NUM; j++) {
            if (!udc->eps[i][j])
                continue;
            ep = udc->eps[i][j];
            ep->busycompleting = 0;
        }
    }
    
    tmp8 = __udc_readb(POWER);
    printk("USB POWER is %x\n",tmp8);
    if(tmp8 & PWR_HS_MODE){
        udc->gadget.speed = USB_SPEED_HIGH;
    }
    else{
        udc->gadget.speed = USB_SPEED_FULL;
    }

    __udc_writew(0x1, INTRTXE);
    
    ULG_FUC(MAKE_EP(0,0), udc->ep0_state, ULG_FUC_RESET, "dd", udc->fifo_addr, udc->gadget.speed);
    return;
}

/* ========================================================= */
/* the handler which service interrupts generated by USB_DMA */ 
/* ========================================================= */

static irqreturn_t mt_udc_dma_handler(int chan)
{
    struct mt_req *req = NULL;
    struct mt_ep *ep;
    int count;
    u8 index;
    u16 csr;
    u8 ep_num;
    u8 ep_dir;
    u32 usb_dma_cntl;
    //dma_addr_t dma_addr = 0;
    if(!udc){
        USB_ERR("invalid *udc\n");
        return IRQ_NONE;
    }

    usb_dma_cntl = __udc_readl(USB_DMA_CNTL(chan));
    /* for dma mode 0 temporarily */
    
    ep_num = (u8)((usb_dma_cntl & USB_DMA_CNTL_ENDPOINT_MASK) >> USB_DMA_CNTL_ENDPOINT_OFFSET);
    ep_dir = (usb_dma_cntl & USB_DMA_CNTL_DMADIR) ? (USB_TX) : (USB_RX);
    ep = udc->eps[ep_num][ep_dir];
    if (!ep) {
        USB_ERR("null ep: %d %d\n", ep_num, ep_dir);
        return IRQ_NONE;
    }
    req = container_of(ep->queue.next, struct mt_req, queue);
    count = __udc_readl(USB_DMA_REALCOUNT(chan));
    if(!ep || !req || !ep->dma){
        USB_ERR("[USB] mt_udc_dma_handler, ep[%p], req[%p], ep->dma[%p]", ep, req, ep ? (ep->dma) : (NULL));
        return IRQ_NONE;
    } else if (list_empty(&ep->queue)) {
        USB_MSG("queue empty: ep%d, dma%d, %x\n", ep->ep_num, ep->dma ? ep->dma->chan : 0x00, usb_dma_cntl);
        return IRQ_HANDLED;
    }
    CHECKREQ(ep, req);

    index = __udc_readb(INDEX);
    __udc_writeb(ep_num, INDEX);
    
    if(usb_dma_cntl & USB_DMA_CNTL_DMADIR){ /* tx dma */
        #if defined USB_USE_DMA_MODE0
            CHECK_DMA_UNMAP(ep, req, count);
            dma_unmap_single(NULL, virt_to_dma(NULL, req->req.buf + \
            req->req.actual), count, DMA_TO_DEVICE);
            req->req.actual += count;

            csr = __udc_readw(IECSR + TXCSR);
            csr |= EPX_TX_TXPKTRDY;
            __udc_writew(csr, IECSR + TXCSR);
            ULG_FUC(MAKE_EP(ep->ep_num, ep->dir), udc->ep0_state, ULG_FUC_DMA_TX, "ddd", req->req.actual-count, count, csr);
        #elif defined USB_USE_DMA_MODE1
           
            dma_addr = virt_to_dma(NULL, req->req.buf);
            count = __udc_readl(USB_DMA_REALCOUNT(chan));
            CHECK_DMA_UNMAP(ep, req);
            dma_unmap_single(NULL, dma_addr, req->req.length, DMA_TO_DEVICE);
            req->req.actual = count;

            if(count % (ep->ep.maxpacket) != 0){
                csr = __udc_readw(IECSR + TXCSR);
                csr &= ~EPX_TX_DMAREQEN;
                __udc_writew(csr, IECSR + TXCSR);
                csr |= EPX_TX_TXPKTRDY;
                __udc_writew(csr, IECSR + TXCSR);
            }
            else{    
                
                done(ep, req, 0);
                 
                if(!list_empty(&ep->queue)){
                    req = container_of(ep->queue.next, struct mt_req, queue);
                    CHECKREQ(ep, req);
                    csr = __udc_readw(IECSR + TXCSR);
                    csr |= EPX_TX_AUTOSET | EPX_TX_DMAREQEN | EPX_TX_DMAREQMODE;
                    __udc_writew(csr, IECSR + TXCSR);
                    mt_usb_config_dma(ep, req, (u32)req->req.buf, req->req.length);
                }
            }

        #endif

        atomic_set(&ep_dma[chan - 1].active, 0);
    }
    else{ /* rx dma */
        CHECK_DMA_UNMAP(ep, req, count);
        dma_unmap_single(NULL, virt_to_dma(NULL, req->req.buf + \
        req->req.actual), count, DMA_FROM_DEVICE);

        /* if the transfer is done, then complete it  */
        req->req.actual += count;
         
        if((count < ep->ep.maxpacket) || (req->req.length == req->req.actual)){
            done(ep, req, 0);
        }

        /* enable the current fifo for the next packet */
        csr = __udc_readw(IECSR + RXCSR);
        csr &= ~EPX_RX_RXPKTRDY;
        __udc_writew(csr, IECSR + RXCSR);

        ULG_FUC(MAKE_EP(ep->ep_num, ep->dir), udc->ep0_state, ULG_FUC_DMA_RX, "ddd", req->req.actual-count, count, csr);

        atomic_set(&ep_dma[chan - 1].active, 0);

        /* if fifo is loaded and sw buffer is ready, then trigger the dma transfer(for double packet buffering) */
        if(!list_empty(&ep->queue)){
            csr = __udc_readw(IECSR + RXCSR);
            if(csr & EPX_RX_RXPKTRDY){
                req = container_of(ep->queue.next, struct mt_req, queue);
                CHECKREQ(ep, req);
                count = __udc_readw(IECSR + RXCOUNT);

                if(count > 0){
                    mt_usb_config_dma(ep, req, (u32)req->req.buf + (u32)req->req.actual, 
                    min((unsigned)count, req->req.length - req->req.actual));
                } else {
                    printk("%s:%d catched\n", __func__, __LINE__);
                }
            }
        }
    }

    __udc_writeb(index, INDEX);

    return IRQ_HANDLED;
}

static void __mt_usb_config_dma(struct mt_ep* ep, struct mt_req* req, u32 addr, u32 count, int line){
    
    u16 usb_dma_cntl = 0, csr = 0; 
    dma_addr_t dma_addr;
    int channel, burst_mode, ep_num, dma_mode, dir;
    
    if (!ep->dma || !ep || !req) {
        USB_ERR("null ptr: %p %p %p\n", ep, ep->dma, req);
        return;
    }
    channel = ep->dma->chan;
    burst_mode = ep->dma->burst;
    ep_num = ep->ep_num;
    dma_mode = ep->dma_mode;
    dir = ep->dir;
    if (atomic_read(&ep_dma[channel - 1].active)) {
        //USB_MSG("dma%d still active: %d\n", channel, line);
        //MUL_LOG_FUC(MAKE_EP(ep_num, dir), 0x00, ULG_FUC_DMA_OCCUPIED, "dd", channel, line);
        return;
    } else if (count > 0) { /*non-zero tx & rx*/
        atomic_set(&ep_dma[channel - 1].active, 1);
        
        CHECK_DMA_MAP(channel, ep_num, dir);
        dma_addr = dma_map_single(NULL, (void *)addr, count, dir ? DMA_TO_DEVICE : DMA_FROM_DEVICE);
     
        __udc_writel(dma_addr, USB_DMA_ADDR(channel));
        __udc_writel(count, USB_DMA_COUNT(channel));
        
        usb_dma_cntl = (burst_mode << USB_DMA_CNTL_BURST_MODE_OFFSET) | (ep_num << USB_DMA_CNTL_ENDPOINT_OFFSET) | USB_DMA_CNTL_INTEN 
        | (dma_mode << USB_DMA_CNTL_DMAMODE_OFFSET) | (dir << USB_DMA_CNTL_DMADIR_OFFSET) | USB_DMA_CNTL_DMAEN;

        __udc_writew(usb_dma_cntl, USB_DMA_CNTL(channel));
    } else if (dir == 1) { /*count = 0, dma tx*/ 

        csr = __udc_readw(IECSR + TXCSR);
        csr |= EPX_TX_TXPKTRDY;
        __udc_writew(csr, IECSR + TXCSR);
    } else if (dir == 0) { /*count = 0, dma rx*/
        done(ep, req, 0);
        /* enable the current fifo for the next packet */
        csr = __udc_readw(IECSR + RXCSR);
        csr &= ~EPX_RX_RXPKTRDY;
        __udc_writew(csr, IECSR + RXCSR);

        /* if fifo is loaded and sw buffer is ready, then trigger the dma transfer(for double packet buffering) */
        if(!list_empty(&ep->queue)){
            csr = __udc_readw(IECSR + RXCSR);
            if(csr & EPX_RX_RXPKTRDY){
                req = container_of(ep->queue.next, struct mt_req, queue);
                CHECKREQ(ep, req);
                count = __udc_readw(IECSR + RXCOUNT);

                if(count > 0){
                    mt_usb_config_dma(ep, req, (u32)req->req.buf + (u32)req->req.actual, 
                    min((unsigned)count, req->req.length - req->req.actual));
                } else {
                    printk("%s:%d catched\n", __func__, __LINE__);
                }
            }
        }        
    }

    return;
}

extern CHARGER_TYPE mt_charger_type_detection(void);

#if 0
CHARGER_TYPE mt_charger_type_detection(void){

    return STANDARD_HOST;
}
#endif

/* ================================ */
/* connect and disconnect functions */
/* ================================ */

void mt_usb_connect(void){

    ULG_FUC(MAKE_EP(0,0), 0x00, ULG_FUC_CONNECT, NULL);
    if(!udc || !udc->ready)
        return;

    mt_dev_connect();
}

void mt_usb_disconnect(void){
    ULG_FUC(MAKE_EP(0,0), 0x00, ULG_FUC_DISCONN, NULL);
    if(!udc || !udc->ready)
        return;
    
    mt_dev_disconnect();
}

void usb_check_connect(void)
{
   if(upmu_is_chr_det(CHR)){
        switch(mt_charger_type_detection()){
            case STANDARD_HOST:
                mt_dev_connect();
                printk("connect success\n");
                return;
            case CHARGER_UNKNOWN:
            case STANDARD_CHARGER:
            case NONSTANDARD_CHARGER:
            case CHARGING_HOST:
                break;
        }
    }	
}

/* =============================================== */
/* module initialization and destruction functions */
/* =============================================== */

static int __init mt_udc_init(void)
{
    /* the USB clock will be turned off during PM initialization, 
     * for preventing phone hang, USB MAC register should be left untouched until probe.
     */
    printk("%s:0x%8x\n", __func__, __raw_readl(APMCU_CG_CON0));
    mt6573_irq_set_sens(MT6573_USB_MC_IRQ_LINE, MT65xx_LEVEL_SENSITIVE);
    mt6573_irq_set_sens(MT6573_USB_DMA_IRQ_LINE, MT65xx_LEVEL_SENSITIVE);
    return platform_driver_register(&mt_udc_driver);
}

static void __exit mt_udc_exit(void)
{
    platform_driver_unregister(&mt_udc_driver);

    return;
}

module_init(mt_udc_init); 
module_exit(mt_udc_exit);

EXPORT_SYMBOL(usb_gadget_register_driver);
EXPORT_SYMBOL(usb_gadget_unregister_driver);

MODULE_AUTHOR("MediaTek");
