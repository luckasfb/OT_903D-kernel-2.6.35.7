#define __UDC_LOG_C__
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/time.h>
#include <linux/device.h>
#include "mt6573_udc.h"
#include "udc_log.h"

/*---------------------------------------------------------------------------*/
#if defined(ENABLE_USB_LOGGER)
/*---------------------------------------------------------------------------*/
enum {
    ULG_TRC_ERR = 0x0001,
    ULG_TRC_DBG = 0x0002,    
    ULG_TRC_FUC = 0x0004,
    ULG_TRC_RD  = 0x0008,
    ULG_TRC_WR  = 0x0010,
    ULG_TRC_DUMP= 0x1000,
};
/*---------------------------------------------------------------------------*/
#define ULG_PREFIX "[ULG] "
/*---------------------------------------------------------------------------*/
#define ULGD(fmt, args...) \
    do {    \
        if(atomic_read(&s_ulg_priv.trc) & ULG_TRC_DBG)  \
            printk(fmt, ##args);  \
    } while(0)
/*---------------------------------------------------------------------------*/
#define ULGE(fmt, args...) \
    do {    \
        if(atomic_read(&s_ulg_priv.trc) & ULG_TRC_ERR) \
            printk(ULG_PREFIX fmt, ##args);  \
    } while(0)
/*---------------------------------------------------------------------------*/
#define ULGF(fmt, args...) \
    do {    \
        if(atomic_read(&s_ulg_priv.trc) & ULG_TRC_FUC) \
            printk(ULG_PREFIX fmt, ##args);  \
    } while(0)
/*---------------------------------------------------------------------------*/
#define C_ULG_DEFAULT_SIZE  (64*1024)
/*---------------------------------------------------------------------------*/
#define C_ULG_SIGNATURE     (0x544D)
/*---------------------------------------------------------------------------*/
#define get_buf_off(n)	    ((n) & (atomic_read(&obj->size) - 1))
/*---------------------------------------------------------------------------*/
struct ulg_evt {
    unsigned long *filter;
    __u32   bits;
    __u32   bytes;  
    char*   (*str)(int code);
};
/*---------------------------------------------------------------------------*/
struct ulg_priv{
    spinlock_t          lock;	
	struct list_head    reader_list;	/* entry in logger_log's list */
    wait_queue_head_t   read_wait;    
    char*               buffer;
	atomic_t		    w_off;	/* current write head offset */
	atomic_t			head;	/* new readers start here */
	atomic_t			size;	/* size of the log */
    /*attribute*/
    atomic_t            len;
    atomic_t            evt;    /* event trace mask */
    atomic_t            trc;    /* debug trace mask */
    
	struct ulg_evt      ev_int;
    struct ulg_evt      ev_fuc;   
    struct ulg_evt      ev_buf;
    unsigned long       ep_filter;    /*MT_EP_NUM X USB_DIR_NUM*/
};
/*----------------------------------------------------------------------------*/
static unsigned long s_evtint_filter[BITS_TO_LONGS(ULG_INT_NUM)] = {0};
static unsigned long s_evtfuc_filter[BITS_TO_LONGS(ULG_FUC_NUM)] = {0};
static unsigned long s_evtbuf_filter[BITS_TO_LONGS(ULG_BUF_NUM)] = {0};
/*---------------------------------------------------------------------------*/
#define DEC(X) #X
static char *s_evtint[] = {
    ULG_INT_LIST            
};
#undef  DEC
/*---------------------------------------------------------------------------*/
inline char *get_evtint_str(int code) 
{
    return (code >= ARRAY_SIZE(s_evtint)) ? (NULL) : (s_evtint[code]); 
}
/*---------------------------------------------------------------------------*/
#define DEC(X) #X
static char *s_evtfuc[] = {
    ULG_FUC_LIST
};
#undef  DEC
/*---------------------------------------------------------------------------*/
inline char *get_evtfuc_str(int code) 
{ 
    return (code >= ARRAY_SIZE(s_evtfuc)) ? (NULL) : (s_evtfuc[code]); 
}
/*---------------------------------------------------------------------------*/
#define DEC(X) #X
static char *s_evtbuf[] = {
    ULG_BUF_LIST
};
#undef  DEC
/*---------------------------------------------------------------------------*/
inline char *get_evtbuf_str(int code) 
{ 
    return (code >= ARRAY_SIZE(s_evtbuf)) ? (NULL) : (s_evtbuf[code]); 
}
/*----------------------------------------------------------------------------*/
struct ulg_reader {
	struct ulg_priv	    *obj;	/* associated log */
	struct list_head    list;	/* entry in logger_log's list */
	atomic_t			r_off;	/* current read head offset */
};
/*----------------------------------------------------------------------------*/
static struct ulg_priv s_ulg_priv = {
    .len = ATOMIC_INIT(0),
    .evt = ATOMIC_INIT(0),
    .trc = ATOMIC_INIT(ULG_TRC_ERR|ULG_TRC_FUC),
    .ev_int = 
    {
        (unsigned long*)s_evtint_filter, 
        ULG_INT_NUM, 
        sizeof(s_evtint_filter),
        get_evtint_str,
    },
    .ev_fuc = 
    {
        (unsigned long*)s_evtfuc_filter, 
        ULG_FUC_NUM, 
        sizeof(s_evtfuc_filter),
        get_evtfuc_str,        
    },
    .ev_buf = 
    {
        (unsigned long*)s_evtbuf_filter, 
        ULG_BUF_NUM, 
        sizeof(s_evtbuf_filter),
        get_evtbuf_str,        
    },
};
/*---------------------------------------------------------------------------*/
static void n_log_intr(int ep, int state, int intr, const char *fmt, ...) {}
static void n_log_func(int ep, int state, int func, const char *fmt, ...) {}
static void n_log_str(int ep, int state, const char *fmt, ...) {}
static void n_log_buf(int ep, int state, int type, int size, char *buf) {}
/*---------------------------------------------------------------------------*/
log_intr_t e_log_intr = n_log_intr;
log_func_t e_log_func = n_log_func;
log_str_t  e_log_str = n_log_str;
log_buf_t  e_log_buf = n_log_buf;
/*---------------------------------------------------------------------------*/
static void udc_log_write(struct ulg_priv *obj, char* buf, size_t count)
{
    size_t len;
    size_t w_off = atomic_read(&obj->w_off);
    size_t room = atomic_read(&obj->size) - w_off;
    if (!obj || !buf) {
        ULGE("null ptr: %p %p \n", obj, buf);
        return;
    }
        
    len = min(count, room);
	memcpy(obj->buffer + w_off, buf, len);
	if (count != len)
		memcpy(obj->buffer, buf + len, count - len);
	atomic_set(&obj->w_off, get_buf_off(w_off + count));
	wake_up_interruptible(&obj->read_wait);    

    if (atomic_read(&obj->trc) & ULG_TRC_WR) {
        int idx;
        printk("wr %2d bytes:\n", count);
        for (idx = 0; idx < count; idx++) {
            printk("%02X ", buf[idx]);
            if ((idx % 32) == 31)
                printk("\n");
        }
        if (idx % 32)
            printk("\n");
    }        
}
/*---------------------------------------------------------------------------*/
static int udc_get_entry_len(struct ulg_priv *obj, size_t off)
{
	__u16 val;

	switch (atomic_read(&obj->size) - off) {
	case 1:
		memcpy(&val, obj->buffer + off, 1);
		memcpy(((char *) &val) + 1, obj->buffer, 1);
		break;
	default:
		memcpy(&val, obj->buffer + off, 2);
	}

	return val; /*the whole size is recorded in the structure*/
}
/*---------------------------------------------------------------------------*/
static size_t udc_find_next(struct ulg_priv *obj, size_t off, size_t len)
{
	size_t count = 0;

	do {
		size_t nr = udc_get_entry_len(obj, off);
		off = get_buf_off(off + nr);
		count += nr;
	} while (count < len);

	return off;
}
/*---------------------------------------------------------------------------*/
static inline int in_interval(size_t a, size_t b, size_t c)
{
	if (b < a) {
		if (a < c || b >= c)
			return 1;
	} else {
		if (a < c && b >= c)
			return 1;
	}

	return 0;
}
/*---------------------------------------------------------------------------*/
static void udc_update_reader(struct ulg_priv *obj, size_t len)
{
	size_t w_off = atomic_read(&obj->w_off);
    size_t r_off;
    size_t head  = atomic_read(&obj->head);
	size_t new = get_buf_off(w_off + len);
	struct ulg_reader *reader;

	if (in_interval(w_off, new, head))
		atomic_set(&obj->head, udc_find_next(obj, head, len));

	list_for_each_entry(reader, &obj->reader_list, list) {
        r_off = atomic_read(&reader->r_off);
		if (in_interval(w_off, new, r_off))
            atomic_set(&reader->r_off, udc_find_next(obj, r_off, len));
    }
}
/*---------------------------------------------------------------------------*/
static ssize_t udc_log_read(struct ulg_priv *obj, struct ulg_reader *reader,
				                char __user *buf, size_t count)
{
	size_t len;
    size_t r_off = atomic_read(&reader->r_off);
    size_t room = atomic_read(&obj->size) - r_off;

	len = min(count, room);
	if (copy_to_user(buf, obj->buffer + r_off, len))
		return -EFAULT;

	/*
	 * Second, we read any remaining bytes, starting back at the head of
	 * the log.
	 */
	if (count != len)
		if (copy_to_user(buf + len, obj->buffer, count - len))
			return -EFAULT;

	atomic_set(&reader->r_off, get_buf_off(r_off + count));

    if (atomic_read(&obj->trc) & ULG_TRC_RD) {
        static char tmp[1024];        
        int idx;
        if (!copy_from_user(tmp, buf, count)) {            
            printk("rd %2d bytes:\n", count);
            for (idx = 0; idx < count; idx++) {
                printk("%02X ", tmp[idx]);
                if ((idx % 32) == 31)
                    printk("\n");
            }
            if (idx % 32)
                printk("\n");
        }
    }

	return count;
}
/*---------------------------------------------------------------------------*/
size_t udc_log_size(struct ulg_priv *obj)
{
    return atomic_read(&obj->size);
}
/*---------------------------------------------------------------------------*/
size_t udc_log_len_locked(struct ulg_reader *reader)
{
    struct ulg_priv* obj = reader->obj;
    size_t ret;
    
	if (atomic_read(&obj->w_off) >= atomic_read(&reader->r_off))
        ret = atomic_read(&obj->w_off) - atomic_read(&reader->r_off);
	else
        ret = atomic_read(&obj->size) - atomic_read(&reader->r_off) + atomic_read(&obj->w_off);
	return ret;
}
/*---------------------------------------------------------------------------*/
size_t udc_log_flush(struct ulg_priv *obj)
{
	struct ulg_reader *reader;    
    unsigned long flags;
    spin_lock_irqsave(&obj->lock, flags);
	list_for_each_entry(reader, &obj->reader_list, list)
		atomic_set(&reader->r_off, atomic_read(&obj->w_off));
    atomic_set(&obj->head, atomic_read(&obj->w_off));
    spin_unlock_irqrestore(&obj->lock, flags);
    return 0;
}
/*---------------------------------------------------------------------------*/
size_t udc_log_reset(struct ulg_priv *obj)
{
	struct ulg_reader *reader; 
    int reader_active = 0;
    if (atomic_read(&obj->evt)) {
        ULGE("fail: the log is active");
        return 0;
    }
	list_for_each_entry(reader, &obj->reader_list, list)
        reader_active = 1;
    
    if (reader_active == 0) {
        atomic_set(&obj->w_off, 0);
        atomic_set(&obj->head, 0);
    }
    return 0;
}
/*---------------------------------------------------------------------------*/
size_t udc_log_resize(struct ulg_priv *obj, size_t size)
{
	struct ulg_reader *reader;    
    unsigned long flags;

    if (atomic_read(&obj->evt)) {
        ULGE("fail: the log is active");
        return 0;
    }
    spin_lock_irqsave(&obj->lock, flags);
    if (size != atomic_read(&obj->size)) {
        char *ptr = NULL;
        if ((ptr = kzalloc(size, GFP_KERNEL))) {
            kfree(obj->buffer);
            obj->buffer = ptr;
            ptr = NULL;
            atomic_set(&obj->size, size);
            atomic_set(&obj->w_off, 0);
            atomic_set(&obj->head, 0);
        	list_for_each_entry(reader, &obj->reader_list, list)
        		atomic_set(&reader->r_off, 0);            
        }
    }
    spin_unlock_irqrestore(&obj->lock, flags);
    return 0;
}
/*---------------------------------------------------------------------------*/
void udc_log_intr(int ep, int state, int code, const char* fmt, ...)
{
    struct ulg_priv *obj = &s_ulg_priv;
    struct udc_log_ent_int item;
	struct timespec now;
    unsigned long flags;

    if (!test_bit(code, obj->ev_int.filter))
        return;
    if (!test_bit(ep, &obj->ep_filter))
        return;

    now = current_kernel_time();
    memset(&item, 0x00, sizeof(item));
    item.header.signature = C_ULG_SIGNATURE;
    item.header.len = sizeof(item.header);
	item.header.sec = now.tv_sec;
	item.header.nsec = now.tv_nsec;
    item.header.type = ULG_EVT_INT;
    item.header.code = code;
    item.header.ep_num = GET_EPN(ep);
    item.header.ep_dir = GET_DIR(ep);
    item.header.ep_state = state;
    if (fmt != NULL) {
        int err = 0, num = 0;    
        va_list ap;

        va_start(ap, fmt);
        while (*fmt && num < ARRAY_SIZE(item.args)) {
            if (*fmt == 'd') {
                item.args[num++] = va_arg(ap, __u32);
            } else {
                err = -EINVAL;
                break;
            }
            fmt++;
        }
        va_end(ap);
        if (err)
            ULGE("parsing error\n");
        else
            item.header.len = sizeof(item);
    }

    spin_lock_irqsave(&obj->lock, flags);
    udc_update_reader(obj, item.header.len);
    udc_log_write(obj, (char*)&item, item.header.len);
    spin_unlock_irqrestore(&obj->lock, flags);
}
/*---------------------------------------------------------------------------*/
void udc_log_func(int ep, int state, int code, const char *fmt, ...)
{
    #define C_LOG_BUF_SIZE 1024
    struct ulg_priv *obj = &s_ulg_priv;
    struct udc_log_ent_fuc item;
    struct timespec now;
    unsigned long flags;

    if (!test_bit(code, obj->ev_fuc.filter))
        return ;
    if (!test_bit(ep, &obj->ep_filter))
        return;

	now = current_kernel_time();    

    memset(&item, 0x00, sizeof(item));
    item.header.signature = C_ULG_SIGNATURE;
    item.header.len = sizeof(item.header);
	item.header.sec = now.tv_sec;
	item.header.nsec = now.tv_nsec;
    item.header.type = ULG_EVT_FUC;
    item.header.code = code;
    item.header.ep_num = GET_EPN(ep);
    item.header.ep_dir = GET_DIR(ep);
    item.header.ep_state = state;

    if (fmt != NULL) {
        int err = 0, num = 0;    
        va_list ap;
        va_start(ap, fmt);
        while (*fmt && num < ARRAY_SIZE(item.args)) {
            if (*fmt == 'd') {
                item.args[num++] = va_arg(ap, __u32);
            } else {
                err = -EINVAL;
                break;
            }
            fmt++;
        }
        va_end(ap);
        if (err)
            ULGE("parsing error\n");
        else
            item.header.len = sizeof(item);
    }

    spin_lock_irqsave(&obj->lock, flags);
    udc_update_reader(obj, item.header.len);    
    udc_log_write(obj, (char*)&item, item.header.len);
    spin_unlock_irqrestore(&obj->lock, flags);
}
/*---------------------------------------------------------------------------*/
void udc_log_str(int ep, int state, const char *fmt, ...)
{
    #define C_LOG_BUF_SIZE 1024
    struct ulg_priv *obj = &s_ulg_priv;
    struct udc_log_entry header;
    size_t len;
    char *buf = NULL;
    struct timespec now;
    unsigned long flags;

    if (!test_bit(ep, &obj->ep_filter))
        return;

    buf = kzalloc(C_LOG_BUF_SIZE, GFP_KERNEL);
	now = current_kernel_time();    

    if (!buf) {
        ULGE("null ptr: %p\n", buf);
        return;
    } else {
	    va_list args;
    	va_start(args, fmt);
    	len = vscnprintf(buf, C_LOG_BUF_SIZE, fmt, args);
    	va_end(args);
    }
    memset(&header, 0x00, sizeof(header));
    header.signature = C_ULG_SIGNATURE;
    header.len = sizeof(header)+len;
	header.sec = now.tv_sec;
	header.nsec = now.tv_nsec;
    header.type = ULG_EVT_STR;
    header.code = 0x00;
    header.ep_num = GET_EPN(ep);
    header.ep_dir = GET_DIR(ep);
    header.ep_state = state;

    spin_lock_irqsave(&obj->lock, flags);
    udc_update_reader(obj, header.len);    
    udc_log_write(obj, (char*)&header, sizeof(header));
    udc_log_write(obj, buf, len);
    spin_unlock_irqrestore(&obj->lock, flags);
}
/*---------------------------------------------------------------------------*/
void udc_log_buf(int ep, int state, int code, int size, char *buf)
{
    struct ulg_priv *obj = &s_ulg_priv;
    struct udc_log_entry header;
    struct timespec now;
    unsigned long flags;

    if (!test_bit(code, obj->ev_buf.filter))
        return ;
    if (!test_bit(ep, &obj->ep_filter))
        return;

	now = current_kernel_time();    

    memset(&header, 0x00, sizeof(header));
    header.signature = C_ULG_SIGNATURE;
    header.len = sizeof(header)+size;
	header.sec = now.tv_sec;
	header.nsec = now.tv_nsec;
    header.type = ULG_EVT_BUF;
    header.code = code;
    header.ep_num = GET_EPN(ep);
    header.ep_dir = GET_DIR(ep);
    header.ep_state = state;

    spin_lock_irqsave(&obj->lock, flags);
    udc_update_reader(obj, header.len);    
    udc_log_write(obj, (char*)&header, sizeof(header));
    udc_log_write(obj, buf, size);
    spin_unlock_irqrestore(&obj->lock, flags);
}
/*----------------------------------------------------------------------------*/
static ssize_t ulg_show_dbg(struct device* dev, 
                                 struct device_attribute *attr, char *buf)
{
    struct ulg_priv *obj;
    ssize_t res;
    if (!dev) {
        ULGD("dev is null!!\n");
        return 0;
    } else if (!(obj = (struct ulg_priv*)dev_get_drvdata(dev))) {
        ULGD("drv data is null!!\n");
        return 0;
    }
    res = scnprintf(buf, PAGE_SIZE, "evt[0x%04x], trc[0x%04x]\n", 
                   atomic_read(&obj->evt), atomic_read(&obj->trc)); 
    return res;
}
/*----------------------------------------------------------------------------*/
static ssize_t ulg_store_dbg(struct device* dev, struct device_attribute *attr,
                                  const char *buf, size_t count)
{
    struct ulg_priv *obj;
    int evt, trc;
    if (!dev) {
        ULGD("dev is null!!\n");
        return count;
    } else if (!(obj = (struct ulg_priv*)dev_get_drvdata(dev))) {
        ULGD("drv data is null!!\n");
        return count;
    } else if (2 != sscanf(buf, "0x%x 0x%x", &evt, &trc)) {
        ULGE("invalid format!!\n");
        return count;
    }
    atomic_set(&obj->evt, evt);
    atomic_set(&obj->trc, trc);
    e_log_intr = (evt & ULG_EVT_INT) ? (udc_log_intr) : (n_log_intr);
    e_log_func = (evt & ULG_EVT_FUC) ? (udc_log_func) : (n_log_func);
    e_log_str  = (evt & ULG_EVT_STR) ? (udc_log_str) :  (n_log_str);
    e_log_buf  = (evt & ULG_EVT_BUF) ? (udc_log_buf) :  (n_log_buf);
    return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t ulg_show_cnf(struct device* dev, 
                                 struct device_attribute *attr, char *buf)
{
    struct ulg_priv *obj;
    ssize_t res;
    if (!dev) {
        ULGD("dev is null!!\n");
        return 0;
    } else if (!(obj = (struct ulg_priv*)dev_get_drvdata(dev))) {
        ULGD("drv data is null!!\n");
        return 0;
    }
    res = scnprintf(buf, PAGE_SIZE, "%4d %4d\n", ULG_INT_NUM, ULG_FUC_NUM); 
    return res;
}
/*----------------------------------------------------------------------------*/
static ssize_t ulg_show_buf(struct device* dev, 
                                 struct device_attribute *attr, char *buf)
{
    struct ulg_priv *obj;
	struct ulg_reader *reader;    
    size_t len = 0;
    unsigned long flags;
    if (!dev) {
        ULGD("dev is null!!\n");
        return 0;
    } else if (!(obj = (struct ulg_priv*)dev_get_drvdata(dev))) {
        ULGD("drv data is null!!\n");
        return 0;
    }

    
    len += scnprintf(buf+len, PAGE_SIZE-len, "Log: %5d %5d %5d\n", 
                     atomic_read(&obj->size), atomic_read(&obj->head), atomic_read(&obj->w_off));
    spin_lock_irqsave(&obj->lock, flags);
	list_for_each_entry(reader, &obj->reader_list, list)
        len += scnprintf(buf+len, PAGE_SIZE-len, "Reader[%p] %5d %5d\n", reader,
                         atomic_read(&reader->r_off), udc_log_len_locked(reader));    
    spin_unlock_irqrestore(&obj->lock, flags);
    return (ssize_t)len;
}
/*----------------------------------------------------------------------------*/
static ssize_t ulg_store_buf(struct device* dev, struct device_attribute *attr,
                                  const char *buf, size_t count)
{
    struct ulg_priv *obj;
    int size, idx;
    if (!dev) {
        ULGD("dev is null!!\n");
        return count;
    } else if (!(obj = (struct ulg_priv*)dev_get_drvdata(dev))) {
        ULGD("drv data is null!!\n");
        return count;
    } else if (!strncmp("-c", buf, 2)) {
        ULGD("clear buffer\n");
        udc_log_flush(obj);
        return count;
    } else if (!strncmp("-r", buf, 2)) {
        ULGD("reset buffer\n");
        udc_log_reset(obj);
        return count;
    } else if (1 == sscanf(buf,"-r=%d", &size)) {
        ULGD("size change: %5d -> %5d\n", udc_log_size(obj), size);
        udc_log_resize(obj, size);
        return count;
    } else if (!strncmp("-t", buf, 2)) {
        for (idx = ULG_INT_NONE; idx < ULG_INT_NUM; idx++)
            ULG_INT(0x00, 0x01, idx, NULL);
        for (idx = ULG_FUC_NONE; idx < ULG_FUC_NUM; idx++) {
            if ((idx % 5) == 0)
                ULG_FUC(0x00, 0x02, idx, "");
            if ((idx % 5) == 1)
                ULG_FUC(0x00, 0x02, idx, "d", 55);
            if ((idx % 5) == 2)
                ULG_FUC(0x00, 0x02, idx, "dd", 77, 99);
            if ((idx % 5) == 3)
                ULG_FUC(0x00, 0x02, idx, "ddd", 11, 22, 33);
            if ((idx % 5) == 4)
                ULG_FUC(0x00, 0x02, idx, "dddd", 11, 22, 33, 44);
        }            
        ULG_STR(0x00, 0x03, "2 args: %d %d", 55, 77);
        ULG_STR(0x00, 0x03, "3 args: %d %d %d", 55, 77, 99);
        ULG_STR(0x00, 0x03, "4 args: %d %d %d %p", 55, 77, 99, obj);
        ULG_BUF(0x00, 0x03, 0x01, 10, "hellostore");
        ULG_BUF(0x00, 0x03, 0x02, 20, "hello, googleandroid");        
        ULGD("unit test finished\n");
    } else {
        ULGD("invalid format\n");
        return count;
    }
    return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t ulg_show_evtint(struct device* dev, 
                                 struct device_attribute *attr, char *buf)
{
    struct ulg_priv *obj;
    struct ulg_evt *ptr;
    int bit;
    size_t len = 0;
    
    if (!dev) {
        ULGD("dev is null!!\n");
        return 0;
    } else if (!(obj = (struct ulg_priv*)dev_get_drvdata(dev))) {
        ULGD("drv data is null!!\n");
        return 0;
    } else if (!(ptr = &obj->ev_int)) {
        ULGD("ptr null\n");
        return 0; 
    }
    for (bit = 0; bit < ptr->bits; bit++)
        len += scnprintf(buf+len, PAGE_SIZE-len, "%2d %2d %-20s\n", bit, test_bit(bit, ptr->filter), ptr->str(bit));
    return (ssize_t)len;
}
/*----------------------------------------------------------------------------*/
static ssize_t ulg_store_evtint(struct device* dev, struct device_attribute *attr,
                                  const char *buf, size_t count)
{
    struct ulg_priv *obj;
    struct ulg_evt *ptr;
    int evt, bit;
    if (!dev) {
        ULGD("dev is null!!\n");
        return count;
    } else if (!(obj = (struct ulg_priv*)dev_get_drvdata(dev))) {
        ULGD("drv data is null!!\n");
        return count;
    } else if (!(ptr = &obj->ev_int)) {
        ULGD("ptr null\n");
        return count;    
    } else if (1 == sscanf(buf, "ALL %d", &evt)) {
        for (bit = 0; bit < ptr->bits; bit++) {
            if (evt)
                set_bit(bit, ptr->filter);
            else
                clear_bit(bit, ptr->filter);
        }
    } else if (2 == sscanf(buf, "%d %d", &bit, &evt)) {
        if (bit < ptr->bits) {
            if (evt)
                set_bit(bit, ptr->filter);
            else
                clear_bit(bit, ptr->filter);
        }
    } else {
        ULGE("invalid format!!\n");
        return count;
    }
    return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t ulg_show_evtfuc(struct device* dev, 
                                 struct device_attribute *attr, char *buf)
{
    struct ulg_priv *obj;
    struct ulg_evt *ptr ;
    int bit;
    size_t len = 0;
    
    if (!dev) {
        ULGD("dev is null!!\n");
        return 0;
    } else if (!(obj = (struct ulg_priv*)dev_get_drvdata(dev))) {
        ULGD("drv data is null!!\n");
        return 0;
    } else if (!(ptr = &obj->ev_fuc)) {
        ULGD("ptr null\n");
        return 0; 
    }
    for (bit = 0; bit < ptr->bits; bit++)
        len += scnprintf(buf+len, PAGE_SIZE-len, "%2d %2d %-20s\n", bit, test_bit(bit, ptr->filter), ptr->str(bit));
    return (ssize_t)len;
}
/*----------------------------------------------------------------------------*/
static ssize_t ulg_store_evtfuc(struct device* dev, struct device_attribute *attr,
                                  const char *buf, size_t count)
{
    struct ulg_priv *obj;
    struct ulg_evt *ptr;
    int evt, bit;
    if (!dev) {
        ULGD("dev is null!!\n");
        return count;
    } else if (!(obj = (struct ulg_priv*)dev_get_drvdata(dev))) {
        ULGD("drv data is null!!\n");
        return count;
    } else if (!(ptr = &obj->ev_fuc)) {
        ULGD("ptr null\n");
        return count;    
    } else if (1 == sscanf(buf, "ALL %d", &evt)) {
        for (bit = 0; bit < ptr->bits; bit++) {
            if (evt)
                set_bit(bit, ptr->filter);
            else
                clear_bit(bit, ptr->filter);
        }
    } else if (2 == sscanf(buf, "%d %d", &bit, &evt)) {
        if (bit < ptr->bits) {
            if (evt)
                set_bit(bit, ptr->filter);
            else
                clear_bit(bit, ptr->filter);
        }
    } else {
        ULGE("invalid format!!\n");
        return count;
    }
    return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t ulg_show_evtbuf(struct device* dev, 
                                 struct device_attribute *attr, char *buf)
{
    struct ulg_priv *obj;
    struct ulg_evt *ptr ;
    int bit;
    size_t len = 0;
    
    if (!dev) {
        ULGD("dev is null!!\n");
        return 0;
    } else if (!(obj = (struct ulg_priv*)dev_get_drvdata(dev))) {
        ULGD("drv data is null!!\n");
        return 0;
    } else if (!(ptr = &obj->ev_buf)) {
        ULGD("ptr null\n");
        return 0; 
    }
    for (bit = 0; bit < ptr->bits; bit++)
        len += scnprintf(buf+len, PAGE_SIZE-len, "%2d %2d %-20s\n", bit, test_bit(bit, ptr->filter), ptr->str(bit));
    return (ssize_t)len;
}
/*----------------------------------------------------------------------------*/
static ssize_t ulg_store_evtbuf(struct device* dev, struct device_attribute *attr,
                                  const char *buf, size_t count)
{
    struct ulg_priv *obj;
    struct ulg_evt *ptr;
    int evt, bit;
    if (!dev) {
        ULGD("dev is null!!\n");
        return count;
    } else if (!(obj = (struct ulg_priv*)dev_get_drvdata(dev))) {
        ULGD("drv data is null!!\n");
        return count;
    } else if (!(ptr = &obj->ev_buf)) {
        ULGD("ptr null\n");
        return count;    
    } else if (1 == sscanf(buf, "ALL %d", &evt)) {
        for (bit = 0; bit < ptr->bits; bit++) {
            if (evt)
                set_bit(bit, ptr->filter);
            else
                clear_bit(bit, ptr->filter);
        }
    } else if (2 == sscanf(buf, "%d %d", &bit, &evt)) {
        if (bit < ptr->bits) {
            if (evt)
                set_bit(bit, ptr->filter);
            else
                clear_bit(bit, ptr->filter);
        }
    } else {
        ULGE("invalid format!!\n");
        return count;
    }
    return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t ulg_show_ep(struct device* dev, 
                                 struct device_attribute *attr, char *buf)
{
    struct ulg_priv *obj;
    int ep, dir, bit;
    size_t len = 0;
    
    if (!dev) {
        ULGD("dev is null!!\n");
        return 0;
    } else if (!(obj = (struct ulg_priv*)dev_get_drvdata(dev))) {
        ULGD("drv data is null!!\n");
        return 0;
    }
    len += scnprintf(buf+len, PAGE_SIZE-len, "ep filter: %8lx\n", obj->ep_filter);
    for (ep = 0; ep < MT_EP_NUM; ep++) {
        for (dir = 0; dir < USB_DIR_NUM; dir++) {
            bit = MAKE_EP(ep, dir);
            len += scnprintf(buf+len, PAGE_SIZE-len, "EP%d %s: %d\n", ep, (dir == USB_RX)?("RX"):("TX"), test_bit(bit, &obj->ep_filter));
        }
    }
    return (ssize_t)len;
}
/*----------------------------------------------------------------------------*/
static ssize_t ulg_store_ep(struct device* dev, struct device_attribute *attr,
                                  const char *buf, size_t count)
{
    struct ulg_priv *obj;
    struct ulg_evt *ptr;
    unsigned long mask;
    if (!dev) {
        ULGD("dev is null!!\n");
        return count;
    } else if (!(obj = (struct ulg_priv*)dev_get_drvdata(dev))) {
        ULGD("drv data is null!!\n");
        return count;
    } else if (!(ptr = &obj->ev_buf)) {
        ULGD("ptr null\n");
        return count;    
    } else if (1 == sscanf(buf, "%lx", &mask)) {
        obj->ep_filter = mask & 0x3ffff; /*18 bits*/
    } else {
        ULGE("invalid format!!\n");
        return count;
    }
    return count;
}
/*----------------------------------------------------------------------------*/
DEVICE_ATTR(cnf,        0664, ulg_show_cnf, NULL);
DEVICE_ATTR(dbg,        0664, ulg_show_dbg, ulg_store_dbg);
DEVICE_ATTR(buf,        0664, ulg_show_buf, ulg_store_buf);
DEVICE_ATTR(evtint,     0664, ulg_show_evtint, ulg_store_evtint);
DEVICE_ATTR(evtfuc,     0664, ulg_show_evtfuc, ulg_store_evtfuc);
DEVICE_ATTR(evtbuf,     0664, ulg_show_evtbuf, ulg_store_evtbuf);
DEVICE_ATTR(ep,         0664, ulg_show_ep, ulg_store_ep);
/*----------------------------------------------------------------------------*/
static struct device_attribute *ulg_attr_list[] = {
    &dev_attr_cnf,  /*configuration*/
    &dev_attr_dbg,  /*debug  control*/
    &dev_attr_buf,  /*buffer control*/
    &dev_attr_evtint, /*log filter*/
    &dev_attr_evtfuc, /*log filter*/
    &dev_attr_evtbuf, /*log filter*/
    &dev_attr_ep,   /*ep filter*/
};
/*----------------------------------------------------------------------------*/
static int ulg_create_attr(struct device *dev) 
{
    int idx, err = 0;
    int num = (int)(sizeof(ulg_attr_list)/sizeof(ulg_attr_list[0]));

    //ULGF("%s \n",__FUNCTION__);
    if (!dev)
        return -EINVAL;

    for (idx = 0; idx < num; idx++) {
        if ((err = device_create_file(dev, ulg_attr_list[idx]))) {            
            ULGD("device_create_file (%s) = %d\n", ulg_attr_list[idx]->attr.name, err);        
            break;
        }
    }
    
    return err;
}
/*----------------------------------------------------------------------------*/
static int ulg_delete_attr(struct device *dev) 
{
    int idx ,err = 0;
    int num = (int)(sizeof(ulg_attr_list)/sizeof(ulg_attr_list[0]));
    
    ULGF("%s\n",__FUNCTION__);
    if (!dev)
        return -EINVAL;

    for (idx = 0; idx < num; idx++) 
        device_remove_file(dev, ulg_attr_list[idx]);

    return err;
}
/*---------------------------------------------------------------------------*/
static int ulg_open(struct inode *inode, struct file *file)
{
    struct ulg_priv *obj = &s_ulg_priv;
    unsigned long flags;

    ULGF("%s\n",__FUNCTION__);
	if (file->f_mode & FMODE_READ) {
		struct ulg_reader *reader;

		reader = kmalloc(sizeof(*reader), GFP_KERNEL);
		if (!reader)
			return -ENOMEM;

		reader->obj = obj;
		INIT_LIST_HEAD(&reader->list);

		spin_lock_irqsave(&obj->lock, flags);
        atomic_set(&reader->r_off, atomic_read(&obj->head));
		list_add_tail(&reader->list, &obj->reader_list);
		spin_unlock_irqrestore(&obj->lock, flags);
		file->private_data = reader;
	} else {
		file->private_data = obj;
	}

	return nonseekable_open(inode, file);
}
/*----------------------------------------------------------------------------*/
static int ulg_release(struct inode *inode, struct file *file)
{
    ULGF("%s\n",__FUNCTION__);
	if (file->f_mode & FMODE_READ) {
		struct ulg_reader *reader = file->private_data;
		list_del(&reader->list);
		kfree(reader);
	}
	file->private_data = NULL;
	return 0;
}
/*----------------------------------------------------------------------------*/
static int ulg_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
       unsigned long arg)
{
    ULGF("%s\n",__FUNCTION__);
    return -ENOIOCTLCMD;
}
/*----------------------------------------------------------------------------*/
static unsigned int ulg_poll(struct file *file, poll_table *wait)
{
	struct ulg_reader *reader;
	struct ulg_priv *obj;
	unsigned int ret = POLLOUT | POLLWRNORM;

	if (!(file->f_mode & FMODE_READ))
		return ret;

	reader = file->private_data;
	obj = reader->obj;

	poll_wait(file, &obj->read_wait, wait);

	if (atomic_read(&obj->w_off) != atomic_read(&reader->r_off))
		ret |= POLLIN | POLLRDNORM;

	return ret;
}
/*----------------------------------------------------------------------------*/
static ssize_t ulg_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	struct ulg_reader *reader = file->private_data;
	struct ulg_priv *obj = reader->obj;
	ssize_t ret;
    unsigned long flags;
	DEFINE_WAIT(wait);

start:
	while (1) {
		prepare_to_wait(&obj->read_wait, &wait, TASK_INTERRUPTIBLE);

        ret = (atomic_read(&obj->w_off) == atomic_read(&reader->r_off));
		if (!ret)
			break;

		if (file->f_flags & O_NONBLOCK) {
			ret = -EAGAIN;
			break;
		}

		if (signal_pending(current)) {
			ret = -EINTR;
			break;
		}

		schedule();
	}

	finish_wait(&obj->read_wait, &wait);
	if (ret)
		return ret;

	/* is there still something to read or did we race? */
	if (unlikely(atomic_read(&obj->w_off) == atomic_read(&reader->r_off))) {
		goto start;
	}

    spin_lock_irqsave(&obj->lock, flags);
	/* get the size of the next entry */
	ret = udc_get_entry_len(obj, atomic_read(&reader->r_off));
	if (count < ret) {
		ret = -EINVAL;
		goto out;
	}

	/* get exactly one entry from the log */
	ret = udc_log_read(obj, reader, buf, ret);

out:
	spin_unlock_irqrestore(&obj->lock, flags);

	return ret;
}
/*----------------------------------------------------------------------------*/
static struct file_operations ulg_fops = {
	.owner = THIS_MODULE,
	.open = ulg_open,
	.release = ulg_release,
	.ioctl = ulg_ioctl,
	.poll = ulg_poll,
	.read = ulg_read,
};
/*---------------------------------------------------------------------------*/
static struct ulg_priv* udc_alloc(size_t len)
{
    int err = 0;
    struct ulg_priv *obj;
#if 0
	if (!(obj = kzalloc(sizeof(*obj), GFP_KERNEL))) {
		err = -ENOMEM;
		goto exit;
	}	
	memset(obj, 0, sizeof(*obj));
#else
    obj = &s_ulg_priv;
#endif

    if (!(obj->buffer = kzalloc(len, GFP_KERNEL))) {
        err = -ENOMEM;
        goto exit;
    }
    atomic_set(&obj->size, len);
    atomic_set(&obj->w_off, 0);
    atomic_set(&obj->head, 0);
    init_waitqueue_head(&obj->read_wait);    
	INIT_LIST_HEAD(&obj->reader_list);
	spin_lock_init(&obj->lock);
    return obj;
exit:    
    if (obj) {
        if (obj->buffer)
            kfree(obj->buffer);
        //kfree(obj);
    }
    return obj;
}
/*---------------------------------------------------------------------------*/
static struct miscdevice ulg_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "ulg",
	.fops = &ulg_fops,
};
/*---------------------------------------------------------------------------*/
static int __init udc_log_init(void)
{
    int err = 0;
    struct ulg_priv *obj = udc_alloc(C_ULG_DEFAULT_SIZE);

    if (!obj)
        return -ENOMEM;
    
    ULGF("%s \n",__FUNCTION__);
	if ((err = misc_register(&ulg_device)))
	{
		ULGE("udc_log register failed\n");
		goto exit_misc_device_register_failed;
	}
    dev_set_drvdata(ulg_device.this_device, obj);

	if ((err = ulg_create_attr(ulg_device.this_device)))
	{
		ULGE("create attribute err = %d\n", err);
		goto exit_create_attr_failed;
	}
    ULGF("%s: '%s' '%s' %p %p %d\n", __FUNCTION__, ulg_device.name, ulg_device.nodename,
            ulg_device.parent, ulg_device.this_device, ulg_device.minor);
    return 0;

exit_create_attr_failed:
    ulg_delete_attr(ulg_device.this_device);
exit_misc_device_register_failed:
	misc_deregister(&ulg_device);    
    
    return err;
}
/*---------------------------------------------------------------------------*/
static void __exit udc_log_exit(void)
{
    ULGF("%s",__FUNCTION__);
}
/*****************************************************************************/
module_init(udc_log_init);
module_exit(udc_log_exit);
/*****************************************************************************/
MODULE_AUTHOR("MingHsien Hsieh <MingHsien.Hsieh@mediatek.com>");
MODULE_DESCRIPTION("USB Logger");
MODULE_LICENSE("GPL");
/*---------------------------------------------------------------------------*/
#endif /*ENABLE_USB_LOGGER*/
