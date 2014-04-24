 
 
#include <linux/string.h> 
#include <linux/time.h> 
#include <linux/uaccess.h> 
#include <linux/fb.h> 
 
#include <linux/debugfs.h> 
 
#if defined(CONFIG_ARCH_MT6516) 
    #include <mach/mt6516_typedefs.h> 
#elif defined(CONFIG_ARCH_MT6573) 
    #include <mach/mt6573_typedefs.h> 
#else 
    #error "unknown arch" 
#endif 
 
 
 
#include "mtk_mau_debug.h" 
#include <mach/mtk_mau.h> 
#include <mach/mt6573_m4u.h>

 
#define MAU_DBG_DEBUG 
#ifdef MAU_DBG_DEBUG 
#define DDBG printk 
#else 
#define DDBG(x,...) 
#endif 
 
#define MAU_DBG_MSG 
#ifdef MAU_DBG_MSG 
#define DMSG printk 
#else 
#define DMSG(x,...) 
#endif 
 
 
 
 
//extern char const* const mau1PortName[MAU1_MASK_ALL]; 
//extern char const* const mpuHPortName[MPU_H_MASK_ALL]; 
extern char const* const mauPortName[MAU1_MASK_ALL]; 
 
 
 
extern unsigned int MAU1_PORT_MASK; 
extern unsigned int MAU2_PORT_MASK; 
extern unsigned int MPUL_PORT_MASK; 
extern unsigned int MPUH_PORT_MASK; 
 
static char debug_buffer[4096]; 
static MTK_MAU_CONFIG mauConfig = {0}; 
M4U_DEBUG_FUNCTION_STRUCT _m4u_debug_func; 
EXPORT_SYMBOL(_m4u_debug_func); 
 
 
static char STR_HELP[] = 
    "\n" 
    "USAGE\n" 
    "        echo [ACTION]... > mau_dbg\n" 
    "\n" 
    "ACTION\n" 
    "        module1|module2?R/W/RW(startPhyAddr,endPhyAddr)@MAU_Enty_ID\n" 
    "             MAU will monitor specified module whether R/W specified range of memory\n" 
    "             example: echo tvc|lcd_r?R(0,0x1000)@1 > mau_dbg\n" 
    "             you can use [all] to specify all modules\n" 
    "             example: echo all?W(0x2000,0x9000)@2 > mau_dbg\n" 
    "\n" 
    "        module1|module2@MAU_Enty_ID:off\n" 
    "             Turn off specified module on specified MAU Entry\n" 
    "             example: echo tvc|lcd_r@1:off > mau_dbg\n" 
    "\n" 
    "\n" 
    "        all:off\n" 
    "             Turn off all of modules\n" 
    "             example: echo all:off > mau_dbg\n" 
    "\n" 
    "        list modules\n" 
    "             list all module names MAU could monitor\n" 
    "\n" 
    "        reg:[MPU|MAU1|MAU2]\n" 
    "             dump hw register values\n" 
    "\n"
    "        regw:addr=val\n"
    "             write hw register\n"
    "\n"
    "        regr:addr\n"
    "             read hw register\n"
    "\n"
    "        m4u_log:on\n"
    "             start to print m4u translate miss rate every second \n"
    "\n"
    "        m4u_log:off\n"
    "             stop to print m4u translate miss rate every second \n"
    "\n"
    "        m4u_monitor:on\n"
    "             start to print m4u translate miss rate every second \n"
    "\n"
    "        m4u_monitor:off\n"
    "             stop to print m4u translate miss rate every second \n";    
// --------------------------------------------------------------------------- 
//  Information Dump Routines
// --------------------------------------------------------------------------- 



#if 0
static long int get_current_time_us(void) 
{ 
    struct timeval t; 
    do_gettimeofday(&t); 
    return (t.tv_sec & 0xFFF) * 1000000 + t.tv_usec; 
}

//Find a char in string, return its last position. -1 means no char in string 
static int find_char(const char * str, char tar) 
{ 
    int length = 0; 
    int i = 0; 
    if (str == NULL) return -1; 
    length = strlen(str); 
    DDBG("[MAU_DBG]%s str:%s len: %d tar:%c\n", __func__, str,length,tar ); 
 
    for (i=length-1; i>=0; i--) { 
        if (str[i] == tar) { 
            DDBG("[MAU_DBG]%s find str @ %d\n", __func__, i ); 
            return i; 
        } 
    } 
    return -1; 
}
#endif 
 
 
static bool parse_cmd(const char *name) 
{ 
    int i = 0; 
    int n = 0; 
    char strCmd[256]; 
    unsigned int _mau1_port_mask = 0; 
    unsigned int _mau2_port_mask = 0; 
    bool check_r = false; 
    bool check_w = false; 
    unsigned long startAddr; 
    unsigned long endAddr; 
    unsigned long entryId = 0; 
 
    const int cmdLendth = sizeof(strCmd) - 1; 
    memset(strCmd, 0, sizeof(strCmd)); 
    scnprintf(strCmd, cmdLendth, name); 
 
 
    DDBG("[MAU_DBG]%s, %s\n", __func__, strCmd); 
    if (0 == strncmp(strCmd, "all", 3)) { 
        _mau1_port_mask = 0xffffffff; 
        _mau2_port_mask = 0xffffffff; 
       n += 4; 
    } 
    else { 
        do { 
            for (i = 1; i < MAU_MASK_ALL; i++) { 
                if (i == MAU1_MASK_ALL) continue; 
                if (0 == strncmp(strCmd+n, mauPortName[i], strlen(mauPortName[i]))) { 
                    if (i < MAU1_MASK_ALL) { 
                        _mau1_port_mask |= 1<<(i-1); 
                    } 
                    else if (i > MAU1_MASK_ALL) { 
                        _mau2_port_mask |= 1<<(i-MAU1_MASK_ALL-1); 
                    } 
                    n += strlen(mauPortName[i])+1; 
                    DDBG("[MAU_DBG]module math %s\n", mauPortName[i]); 
                    break; 
                } 
 
            } 
        } while (strCmd[n-1] == ','); 
    } 
 
    if (strCmd[n-1] == ':') { 
        if (strCmd[n] == 'R' || strCmd[n] == 'r') { 
            check_r = true; 
            n++; 
            if (strCmd[n] == 'W' || strCmd[n] == 'w') { 
                check_w = true; 
                n++; 
            } 
        } else if (strCmd[n] == 'W' || strCmd[n] == 'w') { 
            check_w =true; 
            n++; 
            if (strCmd[n] == 'R' || strCmd[n] == 'r') { 
                check_r = true; 
                n++; 
            } 
 
        } else { 
            goto Error; 
        } 
 
    } else { 
        goto Error; 
    } 
 
    if (strCmd[n] == '[') { 
        char *p = (char *)strCmd + n + 1; 
        startAddr = simple_strtoul(p, &p, 16); 
        endAddr  = simple_strtoul(p + 1, &p, 16); 
        entryId = simple_strtoul(p + 1, &p, 16); 
    } else { 
        goto Error; 
    } 
 
    DDBG("[MAU_DBG] Parse result: port mask(%x), R(%d), W(%d), S(0x%x), E(0x%x)\n", 
        _mau1_port_mask, check_r, check_w, (unsigned int)startAddr, (unsigned int)endAddr); 
    mauConfig.InvalidMasterGMC1 = _mau1_port_mask; 
    mauConfig.InvalidMasterGMC2 = _mau2_port_mask; 
    mauConfig.ReadEn = check_r; 
    mauConfig.WriteEn = check_w; 
    mauConfig.StartAddr = startAddr; 
    mauConfig.EndAddr = endAddr; 
    mauConfig.Enable = true; 
    if ( entryId == 0 || entryId == 1 || entryId == 2) { 
        mauConfig.EntryID = entryId; 
    } 
    else { 
        mauConfig.EntryID = 0; 
    } 
    return true; 
 
 
Error: 
    DDBG("[MAU_DBG]parse command error! %s\n", strCmd); 
    return false; 
 
} 
 
// ---------------------------------------------------------------------------
//  Command Processor
// ---------------------------------------------------------------------------

static void process_dbg_opt(const char *opt)
{
    if (0 == strncmp(opt, "m4u_monitor:", 12))
    {
        if (0 == strncmp(opt + 12, "on", 2)) {
            if(_m4u_debug_func.isInit)
            {
            	 _m4u_debug_func.m4u_perf_timer_on();
            }
            else
            {
            	 printk("[M4U][MAU] error: m4u debug struct is invalid !\n");
            }
        } else if (0 == strncmp(opt + 12, "off", 3)) {
            if(_m4u_debug_func.isInit)
            {
            	 _m4u_debug_func.m4u_perf_timer_off();
            }
            else
            {
            	 printk("[M4U][MAU] error: m4u debug struct is invalid !\n");
            }
        } else {
            goto Error;
        }
    }
    else if (0 == strncmp(opt, "m4u_log:", 8))
    {
        if (0 == strncmp(opt + 8, "on", 2)) {
            if(_m4u_debug_func.isInit)
            {
            	 _m4u_debug_func.m4u_log_on();
            }
            else
            {
            	 printk("[M4U][MAU] error: m4u debug struct is invalid !\n");
            }
        } else if (0 == strncmp(opt + 8, "off", 3)) {
            if(_m4u_debug_func.isInit)
            {
            	 _m4u_debug_func.m4u_log_off();
            }
            else
            {
            	 printk("[M4U][MAU] error: m4u debug struct is invalid !\n");
            }
        } else {
            goto Error;
        }
    }     
    else if (0 == strncmp(opt, "log:", 4))
    {
        if (0 == strncmp(opt + 4, "on", 2)) {
            MAU_LogSwitch(true);
        } else if (0 == strncmp(opt + 4, "off", 3)) {
            MAU_LogSwitch(false);
        } else {
            goto Error;
        }
    }
    else if (0 == strncmp(opt, "reg:", 4)) 
    {
        if (0 == strncmp(opt + 4, "mau1", 4)) { 
            MAU_Mau1DumpReg(); 
        } else if (0 == strncmp(opt + 4, "mau2", 4)) { 
            MAU_Mau2PowerOn(); 
            MAU_Mau2DumpReg(); 
            MAU_Mau2PowerOff(); 
        } else { 
            goto Error;
        }
    } 
    else if (0 == strncmp(opt, "all:stop", 8 )) 
    { 
        //memset(&mauConfig, 0, sizeof(mauConfig)); 
        mauConfig.EntryID = MAU_ENTRY_ALL; 
        mauConfig.Enable = false; 
        MAU_Config(&mauConfig); 
 
        printk("[MAU_DBG]to do all\n"); 
    } 
    else if (parse_cmd(opt)) 
    { 
        MAU_Config(&mauConfig); 
        printk("[MAU_DBG]parse success\n"); 
    } 
 
    else goto Error; 
 
    return; 
 
Error: 
    printk("[MAU_DBG] parse command error!\n\n%s", STR_HELP); 
} 
 
 
static void process_dbg_cmd(char *cmd) 
{ 
    char *tok; 
 
    printk("[MAU_DBG] %s\n", cmd); 
 
    while ((tok = strsep(&cmd, " ")) != NULL) 
    { 
        process_dbg_opt(tok); 
    } 
} 
 
 
// --------------------------------------------------------------------------- 
//  Debug FileSystem Routines 
// --------------------------------------------------------------------------- 
 
struct dentry *mau_dbgfs = NULL; 
 
 
static ssize_t debug_open(struct inode *inode, struct file *file) 
{ 
    file->private_data = inode->i_private; 
    return 0; 
} 
 
 
 
 
static ssize_t debug_read(struct file *file, 
                          char __user *ubuf, size_t count, loff_t *ppos) 
{ 
    const int debug_bufmax = sizeof(debug_buffer) - 1; 
    int n = 0; 
#if 0 
    n += scnprintf(debug_buffer + n, debug_bufmax - n, STR_HELP); 
    n += scnprintf(debug_buffer + n, debug_bufmax - n, "        MAU module list\n"); 
    n += scnprintf(debug_buffer + n, debug_bufmax - n, "        ----------------"); 
    for (i=0; i < MAU1_MASK_ALL; i++ ) { 
            n += scnprintf(debug_buffer + n, debug_bufmax - n, "        ID:%02d ", i); 
            n += scnprintf(debug_buffer + n, debug_bufmax - n, mau1PortName[i]); 
            n += scnprintf(debug_buffer + n, debug_bufmax - n, "\n"); 
    } 
    debug_buffer[n++] = 0; 
#endif 
 
 	MAU_PrintStatus(debug_buffer, debug_bufmax, &n); 
 
    return simple_read_from_buffer(ubuf, count, ppos, debug_buffer, n); 
} 
 
 
static ssize_t debug_write(struct file *file, 
                           const char __user *ubuf, size_t count, loff_t *ppos) 
{ 
    const int debug_bufmax = sizeof(debug_buffer) - 1; 
	size_t ret; 
 
	ret = count; 
 
	if (count > debug_bufmax) 
        count = debug_bufmax; 
 
	if (copy_from_user(&debug_buffer, ubuf, count)) 
		return -EFAULT; 
 
	debug_buffer[count] = 0; 
 
    process_dbg_cmd(debug_buffer); 
 
    return ret; 
} 
 
 
static struct file_operations debug_fops = { 
	.read  = debug_read, 
    .write = debug_write, 
	.open  = debug_open, 
}; 
 
 
void MAU_DBG_Init(void) 
{ 
    mau_dbgfs = debugfs_create_file("mau", 
        S_IFREG|S_IRUGO, NULL, (void *)0, &debug_fops); 
    printk("[MAU_DBG]%s\n", __func__); 
} 
 
 
void MAU_DBG_Deinit(void) 
{ 
    debugfs_remove(mau_dbgfs); 
    printk("[MAU_DBG]%s\n", __func__); 
} 
 
 
