
#ifndef __UDC_LOG_H__
#define __UDC_LOG_H__
#include <linux/types.h>
#include <linux/ioctl.h>
#define ENABLE_USB_LOGGER
#define MAKE_EP(EPN,DIR) (((EPN & 0x0f) << 1) | (DIR & 0x01))
#define GET_EPN(EP)      ((EP & 0x1e) >> 1)
#define GET_DIR(EP)      ((EP & 0x01))
struct udc_log_entry {
	__u16		len;	/* length of the payload */
    __u16       signature;
	__s32		sec;	/* seconds since Epoch */
	__s32		nsec;	/* nanoseconds */
    __u32       ep_num:4;   /*endpoint num*/
    __u32       ep_dir:1;   /*endpoint dir*/
    __u32       ep_state:2; /*endpoint state*/
    __u32       _pad:1;
    __u32       type:8; /* event type */
    __u32       code:16; /* event code */
	char		msg[0];	/* the entry's payload */
};
/*---------------------------------------------------------------------------*/
struct udc_log_ent_int { /*log entry for interrupt*/
    struct udc_log_entry header;
    __u32       args[4];
};
/*---------------------------------------------------------------------------*/
struct udc_log_ent_fuc { /*log entry for interrupt*/
    struct udc_log_entry header;   
    __u32       args[4];
};
/*---------------------------------------------------------------------------*/
#define ULG_EVT_INT      0x01
#define ULG_EVT_DMA      0x02
#define ULG_EVT_FUC      0x04
#define ULG_EVT_STR      0x08
#define ULG_EVT_BUF      0x10
#define ULG_EVT_ERR      0x20
#define ULG_EVT_MASK     0xff
#define ULG_EVT_SHIFT    0x08
/*---------------------------------------------------------------------------*/
#define ULG_INT_LIST    \
    DEC(ULG_INT_NONE),       \
    DEC(ULG_INT_VBUS_ERROR), \
    DEC(ULG_INT_RESET),      \
    DEC(ULG_INT_SESSIONREQ), \
    DEC(ULG_INT_DISCONNECT), \
    DEC(ULG_INT_CONNECT),    \
    DEC(ULG_INT_SOF),        \
    DEC(ULG_INT_RESUME),     \
    DEC(ULG_INT_SUSPEND),    \
    DEC(ULG_INT_EP0),        \
    DEC(ULG_INT_EP_TX),      \
    DEC(ULG_INT_EP_RX),      \
    DEC(ULG_INT_UDC_IRQ_S),  /*udc_irq start*/   \
    DEC(ULG_INT_UDC_IRQ_E),  /*udc_irq end*/     \
    DEC(ULG_INT_DMA_IRQ_S),  /*mt_dma_irq start*/   \
    DEC(ULG_INT_DMA_IRQ_E),  /*mt_dma_irq end*/     \
    
/*---------------------------------------------------------------------------*/
#define DEC(X) X
/*---------------------------------------------------------------------------*/
typedef enum {  /*each event type owns 256 code*/
    ULG_INT_LIST 
    ULG_INT_NUM,
} ULG_INT_CODE;
/*---------------------------------------------------------------------------*/
#undef  DEC
/*---------------------------------------------------------------------------*/
#define ULG_FUC_LIST    \
    DEC(ULG_FUC_NONE),                                              \
    DEC(ULG_FUC_RESET),                                             \
    DEC(ULG_FUC_EP0_SENTSTALL),                                     \
    DEC(ULG_FUC_EP0_SETUPEND),                                      \
    DEC(ULG_FUC_EP0_NORXPKT),                                       \
    DEC(ULG_FUC_EP0_COMPLETE),                                      \
    DEC(ULG_FUC_EP0_IDLE),                                          \
    DEC(ULG_FUC_EP0_HANDLER),                                       \
    DEC(ULG_FUC_GET_FADDR),                                         \
    DEC(ULG_FUC_SET_FADDR),                                         \
    DEC(ULG_FUC_RST_FADDR),                                         \
    DEC(ULG_FUC_TESTMODE_S),     /*enter test mode*/                \
    DEC(ULG_FUC_SET_HALT),                                          \
    DEC(ULG_FUC_ENQUEUE_ALREADY),                                   \
    DEC(ULG_FUC_ENQUEUE_ABORT),                                     \
    DEC(ULG_FUC_ENQUEUE_ZLP),                                       \
    DEC(ULG_FUC_ENQUEUE_TX),                                        \
    DEC(ULG_FUC_ENQUEUE_RX),                                        \
    DEC(ULG_FUC_DEQUEUE),                                           \
    DEC(ULG_FUC_FIFO_STATUS),                                       \
    DEC(ULG_FUC_FIFO_FLUSH),                                        \
    DEC(ULG_FUC_EP0_TX),                                            \
    DEC(ULG_FUC_EP0_RX),                                            \
    DEC(ULG_FUC_EP_TX),                                             \
    DEC(ULG_FUC_EP_RX),                                             \
    DEC(ULG_FUC_EP_ENABLE),                                         \
    DEC(ULG_FUC_EP_DISABLE),                                        \
    DEC(ULG_FUC_GOP_GETFRAME),                                      \
    DEC(ULG_FUC_GOP_WAKEUP),                                        \
    DEC(ULG_FUC_GOP_SELFPWR),                                       \
    DEC(ULG_FUC_GOP_VBUSSES),                                       \
    DEC(ULG_FUC_GOP_DRAW),                                          \
    DEC(ULG_FUC_GOP_PULLUP),                                        \
    DEC(ULG_FUC_GADGET_REG),                                        \
    DEC(ULG_FUC_GADGET_UNREG),                                      \
    DEC(ULG_FUC_DEV_DISC_TRY),                                      \
    DEC(ULG_FUC_DEV_DISC_OK),                                       \
    DEC(ULG_FUC_DEV_CONN_TRY),                                      \
    DEC(ULG_FUC_DEV_CONN_OK),                                       \
    DEC(ULG_FUC_CONNECT),                                           \
    DEC(ULG_FUC_DISCONN),                                           \
    DEC(ULG_FUC_DMA_TX),                                            \
    DEC(ULG_FUC_DMA_RX),                                            \
    DEC(ULG_FUC_EP0_IDLE_S),                                        \
    DEC(ULG_FUC_EP0_IDLE_E),                                        \
    DEC(ULG_FUC_DMA_ALLOC),                                         \
    DEC(ULG_FUC_DMA_FREE),                                          \
    DEC(ULG_FUC_DMA_MAP_OKAY),                                      \
    DEC(ULG_FUC_DMA_MAP_FAIL),                                      \
    DEC(ULG_FUC_DMA_UNMAP_OKAY),                                    \
    DEC(ULG_FUC_DMA_UNMAP_FAIL),                                    \
    DEC(ULG_FUC_DMA_OCCUPIED),                                      \
    DEC(ULG_FUC_CHECK_REQ),                                         \
    DEC(ULG_FUC_TX_NOT_TRANS),                                      \
    
/*---------------------------------------------------------------------------*/
#define DEC(X) X
/*---------------------------------------------------------------------------*/
typedef enum {  /*each event type owns 256 code*/
    ULG_FUC_LIST
    ULG_FUC_NUM,
} ULG_FUC_CODE;
/*---------------------------------------------------------------------------*/
#undef  DEC
/*---------------------------------------------------------------------------*/
#define ULG_BUF_LIST    \
    DEC(ULG_BUF_EP_RX),      \
    DEC(ULG_BUF_EP_TX),      \
    
/*---------------------------------------------------------------------------*/
#define DEC(X) X
/*---------------------------------------------------------------------------*/
typedef enum {  /*each event type owns 256 code*/
    ULG_BUF_LIST 
    ULG_BUF_NUM,
} ULG_BUF_CODE;
/*---------------------------------------------------------------------------*/
#undef  DEC
/*---------------------------------------------------------------------------*/
#define ULG_ERR_LIST    \
    DEC(ULG_ERR_DMA_MAP),           \
    DEC(ULG_ERR_DMA_UNMAP),         \
    DEC(ULG_ERR_TX_NOT_TRANSFER),   \
    DEC(ULG_ERR_CONFIG_DMA),        \
    
/*---------------------------------------------------------------------------*/
#define DEC(X) X
/*---------------------------------------------------------------------------*/
typedef enum {  /*each event type owns 256 code*/
    ULG_ERR_LIST 
    ULG_ERR_NUM,
} ULG_ERR_CODE;
/*---------------------------------------------------------------------------*/
#undef  DEC
/*---------------------------------------------------------------------------*/
typedef void (*log_intr_t)(int ep, int state, int code, const char *fmt, ...);
typedef void (*log_func_t)(int ep, int state, int code, const char *fmt, ...);
typedef void (*log_str_t) (int ep, int state, const char *fmt, ...);
typedef void (*log_buf_t) (int ep, int state, int code, int size, char *buf);
typedef void (*log_err_t) (int ep, int state, int code, const char *fmt, ...);
/*---------------------------------------------------------------------------*/
#if defined(ENABLE_USB_LOGGER)
/*---------------------------------------------------------------------------*/
extern  log_intr_t e_log_intr;
extern  log_func_t e_log_func;
extern  log_str_t e_log_str;
extern  log_buf_t e_log_buf;
extern  log_buf_t e_log_err;
/*---------------------------------------------------------------------------*/
#define ULG_INT(ep, state, intr, fmt...)  e_log_intr(ep, state, intr, fmt)
#define ULG_FUC(ep, state, func, fmt...)  e_log_func(ep, state, func, fmt)
#define ULG_STR(ep, state, fmt...)        e_log_str(ep, state, fmt)
#define ULG_BUF(ep, state, type, sz, buf) e_log_buf(ep, state, type, sz, buf)
#define ULG_ERR(ep, state, type, sz, buf) e_log_err(ep, state, type, sz, buf)
/*---------------------------------------------------------------------------*/
#else   /*ENABLE_USB_LOGGER*/
/*---------------------------------------------------------------------------*/
#define ULG_INT(ep, state, intr, fmt...)  
#define ULG_FUC(ep, state, func, fmt...)  
#define ULG_STR(ep, state, fmt...)        
#define ULG_BUF(ep, state, type, sz, buf)       
#define ULG_ERR(ep, state, type, sz, buf)       
/*---------------------------------------------------------------------------*/
#endif  /*ENABLE_USB_LOGGER*/
/*---------------------------------------------------------------------------*/
#endif 
