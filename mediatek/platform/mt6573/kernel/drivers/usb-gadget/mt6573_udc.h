
#ifndef MT6573_USBD_H
#define MT6573_USBD_H

#include <linux/device.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/usb/ch9.h>
#include <asm/byteorder.h>
#include <mach/mt6573_reg_base.h>
#include <linux/usb/gadget.h>
#include <asm/atomic.h>
#include <mt6573_battery.h>

/* ============= */
/* hardware spec */
/* ============= */

#define MT_EP_NUM 9
#define MT_CHAN_NUM 8
#define MT_EP0_FIFOSIZE 64

#define FIFO_START_ADDR  512

#define MT_BULK_MAXP 512
#define MT_INT_MAXP  64

/* =================== */
/* USB common register */
/* =================== */

#define FADDR    (USB_BASE + 0x0000)  /* Function Address Register */
#define POWER    (USB_BASE + 0x0001)  /* Power Management Register */
#define INTRTX   (USB_BASE + 0x0002)  /* TX Interrupt Status Register */
#define INTRRX   (USB_BASE + 0X0004)  /* RX Interrupt Status Register */
#define INTRTXE  (USB_BASE + 0x0006)  /* TX Interrupt Status Enable Register */
#define INTRRXE  (USB_BASE + 0x0008)  /* RX Interrupt Status Enable Register */
#define INTRUSB  (USB_BASE + 0x000a)  /* Common USB Interrupt Register */
#define INTRUSBE (USB_BASE + 0x000b)  /* Common USB Interrupt Enable Register */
#define FRAME    (USB_BASE + 0x000c)  /* Frame Number Register */
#define INDEX    (USB_BASE + 0x000e)  /* Endpoint Selecting Index Register */
#define TESTMODE (USB_BASE + 0x000f)  /* Test Mode Enable Register */

/* ============ */
/* POWER fields */
/* ============ */

#define PWR_ISO_UPDATE       (1<<7)
#define PWR_SOFT_CONN        (1<<6)
#define PWR_HS_ENAB          (1<<5)
#define PWR_HS_MODE          (1<<4)
#define PWR_RESET            (1<<3)
#define PWR_RESUME           (1<<2)
#define PWR_SUSPEND_MODE     (1<<1)
#define PWR_ENABLE_SUSPENDM  (1<<0)

/* ============== */
/* INTRUSB fields */
/* ============== */

#define INTRUSB_VBUS_ERROR (1<<7)
#define INTRUSB_SESS_REQ   (1<<6)
#define INTRUSB_DISCON     (1<<5)
#define INTRUSB_CONN       (1<<4)
#define INTRUSB_SOF        (1<<3)
#define INTRUSB_RESET      (1<<2)
#define INTRUSB_RESUME     (1<<1)
#define INTRUSB_SUSPEND    (1<<0)

/* ========== */
/* Test Modes */
/* ========== */

#define USB_TST_FORCE_HOST     (1<<7)
#define USB_TST_FIFO_ACCESS    (1<<6)
#define USB_TST_FORCE_FS       (1<<5)
#define USB_TST_FORCE_HS       (1<<4)
#define USB_TST_TEST_PACKET    (1<<3)
#define USB_TST_TEST_K         (1<<2)
#define USB_TST_TEST_J         (1<<1)
#define USB_TST_SE0_NAK        (1<<0)

/* ===================== */
/* DMA control registers */
/* ===================== */

#define DMA_INTR (USB_BASE + 0x0200)

#define USB_DMA_CNTL(chan)  (USB_BASE + 0x0204 + 0x10*(chan-1))
#define USB_DMA_ADDR(chan)  (USB_BASE + 0x0208 + 0x10*(chan-1))
#define USB_DMA_COUNT(chan) (USB_BASE + 0x020c + 0x10*(chan-1))
#define USB_DMA_REALCOUNT(chan) (USB_BASE + 0x0400 + 0x10*(chan-1))

/* ================================= */
/* Endpoint Control/Status Registers */
/* ================================= */

#define IECSR (USB_BASE + 0x0010)
/* for EP0 */
#define CSR0         0x2  /* EP0 Control Status Register */
                          /* For Host Mode, it would be 0x2 */
#define COUNT0       0x8  /* EP0 Received Bytes Register */
#define NAKLIMIT0    0xB  /* NAK Limit Register */
#define CONFIGDATA   0xF  /* Core Configuration Register */
/* for other endpoints */
#define TXMAP        0x0  /* TXMAP Register: Max Packet Size for TX */
#define TXCSR        0x2  /* TXCSR Register: TX Control Status Register */
#define RXMAP        0x4  /* RXMAP Register: Max Packet Size for RX */
#define RXCSR        0x6  /* RXCSR Register: RX Control Status Register */
#define RXCOUNT      0x8  /* RXCOUNT Register */
#define TXTYPE       0xa  /* TX Type Register */
#define TXINTERVAL   0xb  /* TX Interval Register */
#define RXTYPE       0xc  /* RX Type Register */
#define RXINTERVAL   0xd  /* RX Interval Register */
#define FIFOSIZE     0xf  /* configured FIFO size register */

/* ============================== */
/* control status register fields */
/* ============================== */

/* CSR0_DEV */
#define EP0_FLUSH_FIFO           (1<<8)
#define EP0_SERVICE_SETUP_END    (1<<7)
#define EP0_SERVICED_RXPKTRDY    (1<<6)
#define EP0_SENDSTALL            (1<<5)
#define EP0_SETUPEND             (1<<4)
#define EP0_DATAEND              (1<<3)
#define EP0_SENTSTALL            (1<<2)
#define EP0_TXPKTRDY             (1<<1)
#define EP0_RXPKTRDY             (1<<0)

/* TXCSR_DEV */
#define EPX_TX_AUTOSET           (1<<15)
#define EPX_TX_ISO               (1<<14)
#define EPX_TX_MODE              (1<<13)
#define EPX_TX_DMAREQEN          (1<<12)
#define EPX_TX_FRCDATATOG        (1<<11)
#define EPX_TX_DMAREQMODE        (1<<10)
#define EPX_TX_AUTOSETEN_SPKT    (1<<9)
#define EPX_TX_INCOMPTX          (1<<7)
#define EPX_TX_CLRDATATOG        (1<<6)
#define EPX_TX_SENTSTALL         (1<<5)
#define EPX_TX_SENDSTALL         (1<<4)
#define EPX_TX_FLUSHFIFO         (1<<3)
#define EPX_TX_UNDERRUN          (1<<2)
#define EPX_TX_FIFONOTEMPTY      (1<<1)
#define EPX_TX_TXPKTRDY          (1<<0)

#define EPX_TX_WZC_BITS  (EPX_TX_INCOMPTX | EPX_TX_SENTSTALL | EPX_TX_UNDERRUN \
| EPX_TX_FIFONOTEMPTY)

/* RXCSR_DEV */
#define EPX_RX_AUTOCLEAR         (1<<15)
#define EPX_RX_ISO               (1<<14)
#define EPX_RX_DMAREQEN          (1<<13)
#define EPX_RX_DISNYET           (1<<12)
#define EPX_RX_PIDERR            (1<<12)
#define EPX_RX_DMAREQMODE        (1<<11)
#define EPX_RX_AUTOCLRENSPKT     (1<<10)
#define EPX_RX_INCOMPRXINTREN    (1<<9)
#define EPX_RX_INCOMPRX          (1<<8)
#define EPX_RX_CLRDATATOG        (1<<7)
#define EPX_RX_SENTSTALL         (1<<6)
#define EPX_RX_SENDSTALL         (1<<5)
#define EPX_RX_FLUSHFIFO         (1<<4)
#define EPX_RX_DATAERR           (1<<3)
#define EPX_RX_OVERRUN           (1<<2)
#define EPX_RX_FIFOFULL          (1<<1)
#define EPX_RX_RXPKTRDY          (1<<0)

#define EPX_RX_WZC_BITS  (EPX_RX_SENTSTALL | EPX_RX_OVERRUN | EPX_RX_RXPKTRDY)

/* ================= */
/* CONFIGDATA fields */
/* ================= */

#define MP_RXE         (1<<7)
#define MP_TXE         (1<<6)
#define BIGENDIAN      (1<<5)
#define HBRXE          (1<<4)
#define HBTXE          (1<<3)
#define DYNFIFOSIZING  (1<<2)
#define SOFTCONE       (1<<1)
#define UTMIDATAWIDTH  (1<<0)

/* ============= */
/* FIFO register */
/* ============= */

/* for endpint 1 ~ 4, writing to these addresses = writing to the */
/* corresponding TX FIFO, reading from these addresses = reading from */ 
/* corresponding RX FIFO */

#define FIFO(ep_num)     (USB_BASE + 0x0020 + ep_num*0x0004)

/* ============================ */
/* additional control registers */
/* ============================ */

#define DEVCTL       (USB_BASE + 0x0060)  /* OTG Device Control Register */
#define PWRUPCNT     (USB_BASE + 0x0061)  /* Power Up Counter Register */
#define TXFIFOSZ     (USB_BASE + 0x0062)  /* TX FIFO Size Register */
#define RXFIFOSZ     (USB_BASE + 0x0063)  /* RX FIFO Size Register */
#define TXFIFOADD    (USB_BASE + 0x0064)  /* TX FIFO Address Register */
#define RXFIFOADD    (USB_BASE + 0x0066)  /* RX FIFO Address Register */
#define HWVERS       (USB_BASE + 0x006c)  /* H/W Version Register */
#define SWRST        (USB_BASE + 0x0070)  /* Software Reset Register */
#define EPINFO       (USB_BASE + 0x0078)  /* TX and RX Information Register */
#define RAM_DMAINFO  (USB_BASE + 0x0079)  /* RAM and DMA Information Register */
#define LINKINFO     (USB_BASE + 0x007a)  /* Delay Time Information Register */
#define VPLEN        (USB_BASE + 0x007b)  /* VBUS Pulse Charge Time Register */
#define HSEOF1       (USB_BASE + 0x007c)  /* High Speed EOF1 Register */
#define FSEOF1       (USB_BASE + 0x007d)  /* Full Speed EOF1 Register */
#define LSEOF1       (USB_BASE + 0x007e)  /* Low Speed EOF1 Register */
#define RSTINFO      (USB_BASE + 0x007f)  /* Reset Information Register */

/* ========================================================== */
/* FIFO size register fields and available packet size values */
/* ========================================================== */
#define DPB        0x10
#define PKTSZ      0x0f

#define PKTSZ_8    (1<<3)
#define PKTSZ_16   (1<<4)
#define PKTSZ_32   (1<<5)
#define PKTSZ_64   (1<<6)
#define PKTSZ_128  (1<<7)
#define PKTSZ_256  (1<<8)
#define PKTSZ_512  (1<<9)
#define PKTSZ_1024 (1<<10)

#define FIFOSZ_8      (0x0)
#define FIFOSZ_16     (0x1)
#define FIFOSZ_32     (0x2)
#define FIFOSZ_64     (0x3)
#define FIFOSZ_128    (0x4)
#define FIFOSZ_256    (0x5)
#define FIFOSZ_512    (0x6)
#define FIFOSZ_1024   (0x7)
#define FIFOSZ_2048   (0x8)
#define FIFOSZ_4096   (0x9)
#define FIFOSZ_3072   (0xF)

/* ============ */
/* SWRST fields */
/* ============ */

#define SWRST_PHY_RST         (1<<7)
#define SWRST_PHYSIG_GATE_HS  (1<<6)
#define SWRST_PHYSIG_GATE_EN  (1<<5)
#define SWRST_REDUCE_DLY      (1<<4)
#define SWRST_UNDO_SRPFIX     (1<<3)
#define SWRST_FRC_VBUSVALID   (1<<2)
#define SWRST_SWRST           (1<<1)
#define SWRST_DISUSBRESET     (1<<0)

/* ============= */
/* DMA Registers */
/* ============= */

/* DMA_CNTL */
#define USB_DMA_CNTL_ENDMAMODE2             (1 << 13)
#define USB_DMA_CNTL_PP_RST                 (1 << 12)
#define USB_DMA_CNTL_PP_EN                  (1 << 11)
#define USB_DMA_CNTL_BURST_MODE_MASK        (0x3 << 9)
#define USB_DMA_CNTL_BURST_MODE_OFFSET      (9)
#define USB_DMA_CNTL_BURST_MODE0            (0x0 << 9)
#define USB_DMA_CNTL_BURST_MODE1            (0x1 << 9)
#define USB_DMA_CNTL_BURST_MODE2            (0x2 << 9)
#define USB_DMA_CNTL_BURST_MODE3            (0x3 << 9)
#define USB_DMA_CNTL_BUSERR                 (1 << 8)
#define USB_DMA_CNTL_ENDPOINT_MASK          (0xf << 4)
#define USB_DMA_CNTL_ENDPOINT_OFFSET        (4)
#define USB_DMA_CNTL_INTEN                  (1 << 3)
#define USB_DMA_CNTL_DMAMODE                (1 << 2)
#define USB_DMA_CNTL_DMAMODE_OFFSET         (2)
#define USB_DMA_CNTL_DMADIR                 (1 << 1)
#define USB_DMA_CNTL_DMADIR_OFFSET          (1)
#define USB_DMA_CNTL_DMAEN                  (1 << 0)

/* DMA_LIMITER */
#define USB_DMA_LIMITER_MASK                (0xff00)
#define USB_DMA_LIMITER_OFFSET              (8)

/* ======= */
/* typedef */
/* ======= */

typedef enum
{
    USB_FALSE = 0,
    USB_TRUE,
}USB_BOOL;

typedef enum
{
    USB_RX = 0,
    USB_TX,
    USB_CTRL,
    USB_DIR_NUM = USB_CTRL,
}USB_DIR;

typedef enum
{
    EP0_IDLE = 0,
    EP0_RX,
    EP0_TX,
    EP0_RX_STATUS,
}EP0_STATE;

typedef enum
{
    EP0_STATE_READ_END = 0,
    EP0_STATE_WRITE_RDY,
    EP0_STATE_TRANSACTION_END,
    EP0_STATE_CLEAR_SENT_STALL,
}EP0_DRV_STATE;

typedef enum
{
    DEVSTATE_DEFAULT = 0,
    DEVSTATE_SET_ADDRESS,
    DEVSTATE_ADDRESS,
    DEVSTATE_CONFIG,
}DEVICE_STATE;

#if 0
typedef enum
{
    CHARGER_UNKNOWN = 0,
    STANDARD_HOST,
    NONSTANDARD_CHARGER,
    STANDARD_CHARGER,
    CHARGING_HOST,
}CHARGER_TYPE;
#endif

typedef enum
{
    
    DMA_MODE_0 = 0, /*map to DMA MODE0*/
    DMA_MODE_1 = 1, /*map to DMA MODE1*/
    DMA_MODE_PIO,
    
    DMA_MODE_MAX
} DMA_MODE;

typedef enum 
{
    BURST_MODE_0 = 0,
    BURST_MODE_1,
    BURST_MODE_2,
    BURST_MODE_3,

    BURST_MODE_MAX
} BURST_MODE;

typedef enum
{
    CABLE_MODE_CHRG_ONLY = 0,
    CABLE_MODE_NORMAL,

    CABLE_MODE_MAX
} CABLE_MODE;
/* =========== */
/* some macros */
/* =========== */

#define EP_IS_IN(EP)      ((EP)->desc->bEndpointAddress & USB_DIR_IN)
#define EP_INDEX(EP)      ((EP)->desc->bEndpointAddress & 0xF)
#define EP_MAXPACKET(EP)  ((EP)->ep.maxpacket)

#define EPMASK(x) (1<<x)
#define CHANMASK(x) (1<<(x-1))

/* ========== */
/* structures */
/* ========== */

struct mt_ep_dma 
{
    u16 chan;    
    u16 burst;
    atomic_t active; 
    atomic_t allocated;
    atomic_t count;
};

struct mt_ep
{
    unsigned int ep_num;
    unsigned int wedge;
    struct usb_ep ep;
    struct list_head queue;
    const struct usb_endpoint_descriptor *desc;
    struct mt_ep_dma *dma;
    atomic_t enabled;
    
    unsigned int busycompleting:1;
    unsigned int dir:1;
    unsigned int dma_mode:2;
    unsigned int _padding:28;    
};

struct mt_req
{
    struct usb_request req;
    struct list_head queue;
    int queued;
    u32 signature;
    atomic_t zlp_sent;
};

struct mt_udc_dbg 
{
    int (*usblog)(const char *fmt, ...);
    int (*usbmsg)(const char *fmt, ...);
    int (*usberr)(const char *fmt, ...);
    int (*check_req)(struct mt_ep *ep, struct mt_req *req, int line);
    int (*check_dma_map)(int chan, int ep_num, int dir);
    int (*check_dma_unmap)(struct mt_ep *ep, struct mt_req *req, int count, int line);
    unsigned long opt;
    atomic_t dma_sta_retry_max;
};

struct mt_udc
{
    u8                         faddr;
    struct usb_gadget          gadget;
    struct usb_gadget_driver   *driver;
    spinlock_t                 lock;
    EP0_STATE                  ep0_state;
    struct mt_ep              *eps[MT_EP_NUM][USB_DIR_NUM];
    u16                        fifo_addr;
    unsigned long flags;
    u8 set_address;
    u8 test_mode;
    u8 test_mode_nr;
    u8                         power;
    u8                         ready;
    CHARGER_TYPE               charger_type;
    struct mt_req              *udc_req;

    atomic_t                   cmode;   /*cable mode*/
    struct mt_udc_dbg          *dbg;
};

void mt_usb_connect(void);
void mt_usb_disconnect(void);
void mt6573_usb_charger_event_for_evb(int charger_in);

#endif //MT6573_USBD_H

