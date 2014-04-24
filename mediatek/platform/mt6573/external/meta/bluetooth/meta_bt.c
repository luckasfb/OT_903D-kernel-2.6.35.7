/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */


#include "meta_bt.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <linux/serial.h>

/* for RX thread */
#include <pthread.h>

#include <dlfcn.h>

#include <stdbool.h>
#include <cutils/log.h>
#include <cutils/sockets.h>
#include <cutils/properties.h>
#ifdef HAVE_LIBC_SYSTEM_PROPERTIES
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>
#endif

#include "cutils/misc.h"

/* Support NVRAM */
#include "libnvram.h"
#include "CFG_BT_File.h"
#include "CFG_file_lid.h"
#include "CFG_BT_Default.h"

/* custom */
#include "cust_bt.h"

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG                 "BT_META "

#ifndef BT_DRV_MOD_NAME
#define BT_DRV_MOD_NAME         "bluetooth"
#endif


#define  BT_META_DEBUG  1

#define  ERR(f, ...)   LOGE("%s: " f, __func__, ##__VA_ARGS__)
#define  WAN(f, ...)   LOGW("%s: " f, __func__, ##__VA_ARGS__)
#if BT_META_DEBUG
#define  DBG(f, ...)   LOGD("%s: " f, __func__, ##__VA_ARGS__)
#define  TRC(f)        LOGW("%s #%d", __func__, __LINE__)
#define  CONSOLE_PRINT(f, ...) printf(LOG_TAG " %s: " f, __func__, ##__VA_ARGS__)
#else
#define  DBG(...)      ((void)0)
#define  TRC(f)        ((void)0)
#define  CONSOLE_PRINT(...)   ((void)0)
#endif

#define  EVTTYPE_NONE    0
#define  EVTTYPE_EVENT   1
#define  EVTTYPE_SCO     2
#define  EVTTYPE_ACL     3

#define FREEIF(p)   do { if(p) free(p); p = NULL; } while(0)

#define INVALID_HANDLE_VALUE (-1)


typedef unsigned long DWORD;
typedef unsigned long* PDWORD;
typedef unsigned long* LPDWORD;
typedef unsigned short USHORT;
typedef unsigned long HANDLE;
typedef unsigned char BOOLEAN;
typedef void VOID;
typedef void* LPCVOID;
typedef void* LPVOID;
typedef void* LPOVERLAPPED;
typedef unsigned char* PUCHAR;
typedef unsigned char* PBYTE;
typedef unsigned char* LPBYTE;
typedef unsigned char UCHAR;
typedef unsigned char UINT8;
typedef unsigned short UINT16;


static int   bt_init = 0;
static int   bt_fd = -1;
static int   bt_rfkill_id = -1;
static char *bt_rfkill_state_path = NULL;
static BT_CNF_CB cnf_cb = NULL;
static BT_CNF bt_cnf;

/* Used to read serial port */
static pthread_t rxThread;
static BOOL bKillThread = FALSE;

/* mtk init library */
static void *glib_handle = NULL;
typedef int (*SETUP_UART_PARAM)(unsigned long hComPort, int iBaudrate, int iFlowControl);
typedef BOOL (*BT_INIT)(HANDLE hComPortFile,
	                      PBYTE  bAddr,
	                      DWORD  dwBaud,
	                      DWORD  dwHostBaud,
	                      DWORD  dwUseFlowControl,
	                      DWORD  dwSleep,
	                      DWORD  dwCap,
	                      DWORD  dwCodec,
	                      PBYTE  pbExtraInit,
	                      DWORD  dwExtraInitLen,
	                      PBYTE  ucNvRamData,
	                      SETUP_UART_PARAM  setup_uart_param);
typedef BOOL (*BT_DEINIT)(HANDLE hComPortFile,
	                        SETUP_UART_PARAM  setup_uart_param);
typedef BOOL (*BT_WRITE_FD)(HANDLE hComPortFile, unsigned char *pBuffer, DWORD dwLen);
typedef BOOL (*BT_READ_FD)(HANDLE hComPortFile, unsigned char *pBuffer, DWORD dwLen, PDWORD pdwReadLen);

static BT_INIT   BT_InitDevice = NULL;
static BT_DEINIT BT_DeinitDevice = NULL;
static BT_WRITE_FD BT_WriteCommPort = NULL;
static BT_READ_FD  BT_ReadCommPort = NULL;


//Local function
static BOOL BT_Send_HciCmd(BT_HCI_CMD *hci);
static BOOL BT_Recv_HciEvent(BT_HCI_EVENT *hci_event);
static BOOL BT_Send_AclData(BT_HCI_BUFFER *pAclData);
static BOOL BT_Recv_AclData(BT_HCI_BUFFER *pRevAclData);


static void* BT_MetaThread(void* pContext);

static void bt_send_resp(BT_CNF *cnf, size_t len, void *buf, unsigned int size)
{
    if (cnf_cb)
        cnf_cb(cnf, buf, size);
    else
        WriteDataToPC(cnf, sizeof(BT_CNF), buf, size);

}

#ifndef MTK_MT6620
static int bt_init_rfkill(void) 
{
    char path[128];
    char buf[32];
    int fd, id;
    ssize_t sz;
    
    TRC();
    
    for (id = 0; id < 10 ; id++) {
        snprintf(path, sizeof(path), "/sys/class/rfkill/rfkill%d/type", id);
        fd = open(path, O_RDONLY);
        if (fd < 0) {
            CONSOLE_PRINT("open(%s) failed: %s (%d)\n", path, strerror(errno), errno);
            return -1;
        }
        sz = read(fd, &buf, sizeof(buf));
        close(fd);
        if (sz >= (ssize_t)strlen(BT_DRV_MOD_NAME) && 
            memcmp(buf, BT_DRV_MOD_NAME, strlen(BT_DRV_MOD_NAME)) == 0) {
            bt_rfkill_id = id;
            break;
        }
    }

    if (id == 10)
        return -1;

    asprintf(&bt_rfkill_state_path, "/sys/class/rfkill/rfkill%d/state", 
        bt_rfkill_id);
    
    return 0;
}

static int bt_set_power(int on) 
{
    int sz;
    int fd = -1;
    int ret = -1;
    const char buf = (on ? '1' : '0');

    TRC();

    if (bt_rfkill_id == -1) {
        if (bt_init_rfkill()) goto out;
    }

    fd = open(bt_rfkill_state_path, O_WRONLY);
    if (fd < 0) {
        ERR("unable to open BT RFKILL");
        CONSOLE_PRINT("open(%s) for write failed: %s (%d)", bt_rfkill_state_path,
             strerror(errno), errno);
        goto out;
    }
    sz = write(fd, &buf, 1);
    if (sz < 0) {
        ERR("unable to write BT RFKILL");
        CONSOLE_PRINT("write(%s) failed: %s (%d)", bt_rfkill_state_path, strerror(errno),
             errno);
        goto out;
    }
    ret = 0;

out:
    if (fd >= 0) close(fd);
    return ret;
}
#endif


/* Initialize UART driver */
static int init_uart(char *dev)
{
    struct termios ti;
    struct serial_struct ss;
    int fd, i;
	  
    TRC();
    
    fd = open(dev, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        perror("Can't open serial port");
        return -1;
    }
#ifndef MTK_MT6620
    tcflush(fd, TCIOFLUSH);
    
    /* Clear the cust flag */
    if((ioctl(fd, TIOCGSERIAL, &ss))<0){
        ERR("BAUD: error to get the serial_struct info:%s\n", strerror(errno));
        return -1;
    }
    
    if (ss.flags & ASYNC_SPD_CUST) {
        DBG("clear ASYNC_SPD_CUST\r\n");
        ss.flags &= ~ASYNC_SPD_CUST;
    }
    if((ioctl(fd, TIOCSSERIAL, &ss))<0){
    	ERR("BAUD: error to set serial_struct:%s\n", strerror(errno));
    	return -1;
    }
    
    if (tcgetattr(fd, &ti) < 0) {
        ERR("unable to get UART port setting");
        perror("Can't get port settings");
        return -1;
    }
    
    cfmakeraw(&ti);
    
    ti.c_cflag |= CLOCAL;
    ti.c_cflag &= ~CRTSCTS;
    ti.c_lflag = 0;
    ti.c_cc[VTIME]    = 5; /* 0.5 sec */
    ti.c_cc[VMIN]     = 0;
    
    /* Set initial baudrate */
    cfsetospeed(&ti, B115200);
    cfsetispeed(&ti, B115200);
    
    if (tcsetattr(fd, TCSANOW, &ti) < 0) {
        ERR("unable to set UART port setting");
        perror("Can't set port settings");
        return -1;
    }
    
    tcflush(fd, TCIOFLUSH);
#endif
    return fd;
}

#ifndef MTK_MT6620
static int uart_speed(int s)
{
    switch (s) {
    case 9600:
	  return B9600;
    case 19200:
	  return B19200;
    case 38400:
	  return B38400;
    case 57600:
	  return B57600;
    case 115200:
	  return B115200;
    case 230400:
	  return B230400;
    case 460800:
	  return B460800;
    case 500000:
	  return B500000;
    case 576000:
	  return B576000;
    case 921600:
	  return B921600;
    case 1000000:
	  return B1000000;
    case 1152000:
	  return B1152000;
    case 1500000:
	  return B1500000;
    case 2000000:
	  return B2000000;
#ifdef B2500000
    case 2500000:
      return B2500000;
#endif
#ifdef B3000000
    case 3000000:
	  return B3000000;
#endif
#ifdef B3500000
    case 3500000:
      return B3500000;
#endif
#ifdef B4000000
    case 4000000:
      return B4000000;
#endif
    default:
	  return B57600;
	}
}

static int set_speed(int fd, struct termios *ti, int speed)
{
    struct serial_struct ss;
    int baudenum = uart_speed(speed);

    if ((baudenum == B57600) && (speed != 57600)) {
        DBG("non-standard baudrate: %d\n", speed);
        cfsetispeed(ti, B38400);  
        cfsetospeed(ti, B38400);  
        tcflush(fd, TCIFLUSH);/*handle unrecevie char*/  
        tcsetattr(fd, TCSANOW, ti);  
        if((ioctl(fd, TIOCGSERIAL, &ss))<0){  
            ERR("error to get the serial_struct info:%s\n", strerror(errno));  
            return -1;  
        }  
        ss.flags |= ASYNC_SPD_CUST;  
        ss.custom_divisor = ss.baud_base / speed;  
        if((ioctl(fd, TIOCSSERIAL, &ss))<0){  
            ERR("error to set serial_struct:%s\n", strerror(errno));  
            return -2;  
        }
        return 0;
    }
    else {
        DBG("standard baudrate: %d -> 0x%08x\n", speed, baudenum);
        if((ioctl(fd, TIOCGSERIAL, &ss))<0){  
            ERR("error to get the serial_struct info:%s\n", strerror(errno));  
            return -1;  
        }  
        ss.flags &= ~ASYNC_SPD_CUST;    
        if((ioctl(fd, TIOCSSERIAL, &ss))<0){  
            ERR("error to set serial_struct:%s\n", strerror(errno));  
            return -2;  
        }
        cfsetospeed(ti, baudenum);
        cfsetispeed(ti, baudenum);
        return tcsetattr(fd, TCSANOW, ti);
    }
}

/* Used as host uart param setup callback */
int setup_uart_param(
    unsigned long hComPort, 
    int iBaudrate, 
    int iFlowControl)
{
    struct termios ti;
    int  fd;
	
    DBG("setup_uart_param %d %d\n", iBaudrate, iFlowControl);
    
    fd = (int)hComPort;
    if (fd < 0) {
        ERR("Invalid serial port");
        return -1;
    }
    
    tcflush(fd, TCIOFLUSH);
    
    if (tcgetattr(fd, &ti) < 0) {
        ERR("Can't get port settings");
        return -1;
    }
    
    cfmakeraw(&ti);
    
    ti.c_cflag |= CLOCAL;
    
    ti.c_cflag &= ~CRTSCTS;
    ti.c_iflag &= ~(IXON | IXOFF | IXANY | 0x80000000);

    /* HW flow control */
    if (iFlowControl == 1){
	  	ti.c_cflag |= CRTSCTS;
	  }
	  else if (iFlowControl == 2){
        /* SW flow control */
        ti.c_iflag |= (IXON | IXOFF | IXANY);
        ti.c_iflag |= 0x80000000;
        ti.c_cflag |= CRTSCTS;
    }
    
    if (tcsetattr(fd, TCSANOW, &ti) < 0) {
        ERR("Can't set port settings");
        return -1;
    }
    
    /* Set baudrate */
    if (set_speed(fd, &ti, iBaudrate) < 0) {
        ERR("Can't set initial baud rate");
        return -1;
    }
    
    tcflush(fd, TCIOFLUSH);
    
    return fd;
}
#endif

static int bt_read_nvram(unsigned char* ucBtAddr, unsigned long* pdwCap, unsigned long* pdwCodec, unsigned char *ucNvRamData)
{
    int bt_nvram_fd = -1;
    int rec_size = 0;
    int rec_num = 0;
    int capId, codec;
    ap_nvram_btradio_mt6610_struct bt_nvram;
    
    TRC();
    
    /* read NVRAM */
    bt_nvram_fd = NVM_GetFileDesc(AP_CFG_RDEB_FILE_BT_ADDR_LID, &rec_size, &rec_num, ISWRITE);
    if(bt_nvram_fd < 0){
        ERR("Read BT NVRAM errno %d\n", errno);
        return -1;
    }
    
    NVRAM_LOG("FD %d rec_size %d rec_num %d\n", bt_nvram_fd, rec_size, rec_num);
    
    if(rec_num != 1){
        ERR("Unexpected record num %d", rec_num);
        NVM_CloseFileDesc(bt_nvram_fd);
        return -1;
    }
    
    if(rec_size != sizeof(ap_nvram_btradio_mt6610_struct)){
        ERR("Unexpected record size %d ap_nvram_btradio_mt6610_struct %d", 
			rec_size, sizeof(ap_nvram_btradio_mt6610_struct));
	    NVM_CloseFileDesc(bt_nvram_fd);
	    return -1;
    }
    	
    if(read(bt_nvram_fd, &bt_nvram , rec_num*rec_size) < 0){
	    ERR("Read NVRAM fails %d\n", errno);
	    NVM_CloseFileDesc(bt_nvram_fd);
	    return -1;
    }

#ifdef MTK_MT6620
    NVRAM_LOG("Read NVRAM: [BD address %02x-%02x-%02x-%02x-%02x-%02x] [Voice %02x %02x] [Codec %02x %02x %02x %02x] \
              [Radio %02x %02x %02x %02x %02x %02x] [Sleep %02x %02x %02x %02x %02x %02x %02x] [BtFTR %02x %02x] [TxPWOffset %02x %02x %02x]\n",
              bt_nvram.addr[0], bt_nvram.addr[1], bt_nvram.addr[2], bt_nvram.addr[3], bt_nvram.addr[4], bt_nvram.addr[5], 
              bt_nvram.Voice[0], bt_nvram.Voice[1], 
              bt_nvram.Codec[0], bt_nvram.Codec[1], bt_nvram.Codec[2], bt_nvram.Codec[3], 
              bt_nvram.Radio[0], bt_nvram.Radio[1], bt_nvram.Radio[2], bt_nvram.Radio[3], bt_nvram.Radio[4], bt_nvram.Radio[5], 
              bt_nvram.Sleep[0], bt_nvram.Sleep[1], bt_nvram.Sleep[2], bt_nvram.Sleep[3], bt_nvram.Sleep[4], bt_nvram.Sleep[5], bt_nvram.Sleep[6], 
              bt_nvram.BtFTR[0], bt_nvram.BtFTR[1], 
              bt_nvram.TxPWOffset[0], bt_nvram.TxPWOffset[1], bt_nvram.TxPWOffset[2]);
#else
    NVRAM_LOG("Read NVRAM: BD address %02x-%02x-%02x-%02x-%02x-%02x Cap %02x Codec %02x\n",
              bt_nvram.addr[0], bt_nvram.addr[1], bt_nvram.addr[2], bt_nvram.addr[3], bt_nvram.addr[4], bt_nvram.addr[5], 
              bt_nvram.CapId[0], bt_nvram.Codec[0]);
#endif

    ucBtAddr[0] = bt_nvram.addr[0];
    ucBtAddr[1] = bt_nvram.addr[1];
    ucBtAddr[2] = bt_nvram.addr[2];
    ucBtAddr[3] = bt_nvram.addr[3];
    ucBtAddr[4] = bt_nvram.addr[4];
    ucBtAddr[5] = bt_nvram.addr[5];

#ifdef MTK_MT6620
    memcpy(ucNvRamData, &bt_nvram.Voice[0], sizeof(ap_nvram_btradio_mt6610_struct)-6);
#else
    *pdwCap = bt_nvram.CapId[0];
    *pdwCodec = bt_nvram.Codec[0];
#endif

    NVM_CloseFileDesc(bt_nvram_fd);
    bt_nvram_fd = -1;
    
    return 0;
}

void META_BT_Register(BT_CNF_CB callback)
{
    cnf_cb = callback;
}

BOOL META_BT_init(void)
{
    unsigned char ucBTAddr[] = {0x00, 0x0C, 0xE7, 0x00, 0x11, 0x22};
    int speed = 0;
    int iUseFlowControl = 0, iSleep = 0; 
    unsigned long dwCap = 0, dwCodec = 0;
    unsigned char ucNvRamData[] = {0x60, 0x00,
    	                           0x23, 0x10, 0x00, 0x00,
    	                           0x06, 0x80, 0x00, 0x06, 0x03, 0x06,
    	                           0x03, 0x40, 0x1F, 0x40, 0x1F, 0x00, 0x04,
    	                           0x80, 0x00,
    	                           0xFF, 0xFF, 0xFF};
    SETUP_UART_PARAM pSetupUartParam = NULL;

    TRC();
#ifndef MTK_MT6620
    /* in case BT is powered on before test */
    bt_set_power(0);
    
    bt_set_power(1);
#endif
    sleep(1);

    /* Create COM port */
    bt_fd = init_uart(CUST_BT_SERIAL_PORT);
    if (bt_fd < 0){
        ERR("unable to initilize UART" CUST_BT_SERIAL_PORT);
        /* error handling */
        goto error;
    }
    
    /* Load libbluetoothinit_mtk.so, modified by tingting.lei
        */
    glib_handle = dlopen("libbluetoothinit_mtk.so", RTLD_LAZY);
    if (!glib_handle) 
    {
        ERR("%s\n", dlerror());
        goto error;
    }
    
    BT_InitDevice = dlsym(glib_handle, "BT_InitDevice");
    BT_DeinitDevice = dlsym(glib_handle, "BT_DeinitDevice");
    BT_WriteCommPort = dlsym(glib_handle, "BT_WriteCommPort");
    BT_ReadCommPort = dlsym(glib_handle, "BT_ReadCommPort");

    if ((!BT_InitDevice) || (!BT_DeinitDevice) || (!BT_WriteCommPort) || (!BT_ReadCommPort)) 
    {
        ERR("can not find functions %s\n", dlerror());
        goto error;
    }
    
    DBG("function address 0x%08x 0x%08x 0x%08x 0x%08x\n", 
        (int)BT_InitDevice, (int)BT_DeinitDevice, (int)BT_WriteCommPort, (int)BT_ReadCommPort);

    if(bt_read_nvram(ucBTAddr, &dwCap, &dwCodec, ucNvRamData)){
#ifndef MTK_MT6620
        // Use default values
        dwCap = 0x40;
        dwCodec = 0x23;
#endif
    }

#ifndef MTK_MT6620
    iSleep = 1;
    iUseFlowControl = 2; /* FLOW_CTL_SW */
    speed = CUST_BT_BAUD_RATE;
    pSetupUartParam = setup_uart_param;
#endif

    if(BT_InitDevice(
        bt_fd,
        ucBTAddr,
        speed,
        speed,
        iUseFlowControl,
        iSleep,
        dwCap,
        dwCodec,
        NULL,
        0,
        ucNvRamData,
        pSetupUartParam) == FALSE){

        ERR("Initialize BT device fails\n");
        goto error;
    }


    /* Create RX thread */
    pthread_create( &rxThread, NULL, BT_MetaThread, (void*) &bt_cnf);
    
    
    bt_init = 1;
    sched_yield();

    return TRUE;

error:
    if (glib_handle){
        dlclose(glib_handle);
        glib_handle = NULL;
    }
    if (bt_fd > 0){
        close(bt_fd);
        bt_fd = -1;
    }
#ifndef MTK_MT6620
    bt_set_power(0);
#endif
    return FALSE;
}

void META_BT_deinit(void)
{
    SETUP_UART_PARAM pSetupUartParam = NULL;
#ifndef MTK_MT6620
    pSetupUartParam = setup_uart_param;
#endif

    TRC();
    if(glib_handle && BT_DeinitDevice && (bt_fd > 0)){
        BT_DeinitDevice(bt_fd, pSetupUartParam);
    }
    
    /* stop RX thread */
    bKillThread = TRUE;
    
    /* wait until thread exist */
    pthread_join(rxThread, NULL);
	
    if (glib_handle){
        dlclose(glib_handle);
        glib_handle = NULL;
    }
    
    /* Close COM port */
    if (bt_fd > 0){
        close (bt_fd);
        bt_fd = -1;
    }

#ifndef MTK_MT6620    
    bt_set_power(0); /* shutdown BT */
#endif

    bt_init = 0;
    return;
}

void META_BT_OP(BT_REQ *req, 
    char *peer_buf, 
    unsigned short peer_len)
{
    unsigned int i;
    int ret = -1;
    unsigned long avail_sz;

    TRC();

    CONSOLE_PRINT("[CCC HHH META_BT] %s:%d\n", __FUNCTION__, __LINE__);

//    if (NULL == req || NULL == peer_buf || wifi_skfd < 0 || !bt_init) {
    /* need check COM port handle */
    if (NULL == req || !bt_init) {
        CONSOLE_PRINT("[META_BT] Invalid arguments or operation\n");
//        goto exit;
    }
    
    memset(&bt_cnf, 0, sizeof(BT_CNF));
    bt_cnf.header.id = FT_BT_CNF_ID;
    bt_cnf.header.token = req->header.token;
    bt_cnf.op = req->op;
    
    /* purse com port? */
    
    switch(req->op){
	    case BT_OP_HCI_SEND_COMMAND:
	        DBG("BT_OP_HCI_SEND_COMMAND");
	        if(!BT_Send_HciCmd(&req->cmd.hci))
	        {
	            CONSOLE_PRINT("Can't send HCI command \r\n");
	        }
	        break;
	        
	    case BT_OP_HCI_CLEAN_COMMAND:
	        DBG("BT_OP_HCI_CLEAN_COMMAND");
#ifndef MTK_MT6620
	        if(bt_fd != INVALID_HANDLE_VALUE)
	            tcflush(bt_fd, TCIOFLUSH);
#endif
	        bt_cnf.status = META_SUCCESS;		    
	        break;
		      
	    case BT_OP_HCI_SEND_DATA:	
	        DBG("BT_OP_HCI_SEND_DATA");
	        BT_Send_AclData(&req->cmd.buf);
	        break;
	        
	    case BT_OP_HCI_TX_PURE_TEST_V2:		    
	    case BT_OP_HCI_TX_PURE_TEST:		  
	    case BT_OP_HCI_RX_TEST_START_V2:	  
	    case BT_OP_HCI_RX_TEST_START:
	    case BT_OP_HCI_RX_TEST_END:		   
	    case BT_OP_ENABLE_NVRAM_ONLINE_UPDATE:
	    case BT_OP_DISABLE_NVRAM_ONLINE_UPDATE:
	        ERR("Unexpected BT meta command");
	        CONSOLE_PRINT("[META_BT] Unexpected command %d\n", req->op);
	        
	        bt_cnf.header.id = FT_BT_CNF_ID; //remember to add FT_BT_CNF_ID to ft_msg
	        bt_cnf.op = req->op;
	        bt_cnf.status = META_FAILED;
	        bt_send_resp(&bt_cnf, sizeof(BT_CNF), NULL, 0);
	        break;

	    case BT_OP_ENABLE_PCM_CLK_SYNC_SIGNAL:
	        /* need to confirm w. CCCI driver buddy */
	        CONSOLE_PRINT("[META_BT] Not implemented command %d\n", req->op);
	             
	        bt_cnf.header.id = FT_BT_CNF_ID; //remember to add FT_BT_CNF_ID to ft_msg
	        bt_cnf.op = req->op;
	        bt_cnf.status = META_FAILED;
	        bt_send_resp(&bt_cnf, sizeof(BT_CNF), NULL, 0);
	        break;
	    case BT_OP_DISABLE_PCM_CLK_SYNC_SIGNAL:
	        /* need to confirm w. CCCI driver buddy */
	        CONSOLE_PRINT("[META_BT] Not implemented command %d\n", req->op);
	        
	        bt_cnf.header.id = FT_BT_CNF_ID; //remember to add FT_BT_CNF_ID to ft_msg
	        bt_cnf.op = req->op;
	        bt_cnf.status = META_FAILED;
	        bt_send_resp(&bt_cnf, sizeof(BT_CNF), NULL, 0);
	        break;
	    default:
	        ERR("unknown BT meta command");
	        CONSOLE_PRINT("[META_BT] Unknown command %d\n", req->op);
	        
	        bt_cnf.header.id = FT_BT_CNF_ID; //remember to add FT_BT_CNF_ID to ft_msg
	        bt_cnf.op = req->op;
	        bt_cnf.status = META_FAILED;
	        bt_send_resp(&bt_cnf, sizeof(BT_CNF), NULL, 0);
	        break;
    }
    return;
}

static BOOL BT_Send_HciCmd(BT_HCI_CMD *hci)
{
    UINT8 pHCI[260];
    DWORD dwLen = 0;
    
    pHCI[0] = 0x01;
    pHCI[1] = (hci->opcode)&0xff;
    pHCI[2] = (hci->opcode>>8)&0xff;
    pHCI[3] = hci->len;        
    memcpy(&pHCI[4], hci->cmd, hci->len);
    
    dwLen = 4 + hci->len;

    TRC();
    
    if(!glib_handle || !BT_WriteCommPort || (bt_fd < 0)){
    	  ERR("Invalid serial port or write function\n");
    	  return FALSE;
    }
    
    return BT_WriteCommPort(bt_fd, pHCI, dwLen);
}

static BOOL BT_Recv_HciEvent(BT_HCI_EVENT *hci_event)
{
    UINT8 header = 0;
    DWORD dwReadLen = 0;
    DWORD dwParaLen = 0;
    hci_event->status = FALSE;
    
    TRC();
    
    if(!glib_handle || !BT_ReadCommPort || (bt_fd < 0)){
        ERR("Invalid serial port or read function\n");
        return FALSE;
    }
    
    if(!BT_ReadCommPort(bt_fd, &hci_event->event, sizeof(unsigned char), &dwReadLen) || (dwReadLen == 0)){
        return FALSE;
    }
    DBG("read event code: 0x%x\n", hci_event->event);
    
    dwReadLen = 0;
    if(!BT_ReadCommPort(bt_fd, &hci_event->len, sizeof(unsigned char), &dwReadLen) || (dwReadLen == 0)){
        return FALSE;
    }
    DBG("read event len: 0x%x\n", hci_event->len);
	  
    if(hci_event->len){
        if(!BT_ReadCommPort(bt_fd, hci_event->parms, hci_event->len, &dwParaLen) || (dwParaLen < (hci_event->len))){
            return FALSE;  
        }
    }
    
    hci_event->status = TRUE;
    
    return TRUE;
}

static BOOL BT_Send_AclData(BT_HCI_BUFFER *pAclData)
{
    UINT8 AclData[1029];
    DWORD dwLen = 0;
    
    AclData[0] = 0x02;
    AclData[1] = (pAclData->con_hdl)&0xff;
    AclData[2] = (pAclData->con_hdl>>8)&0xff;
    AclData[3] = (pAclData->len)&0xff;
    AclData[4] = (pAclData->len>>8)&0xff; 
         
    memcpy(&AclData[5], pAclData->buffer, pAclData->len);
    
    dwLen = 5 + pAclData->len;
    
    TRC();
    
    if(!glib_handle || !BT_WriteCommPort || (bt_fd < 0)){
        ERR("Invalid serial port or write function\n");
        return FALSE;
    }
    
    return BT_WriteCommPort(bt_fd, AclData, dwLen);
}

static BOOL BT_Recv_AclData(BT_HCI_BUFFER *pRevAclData)
{
    UINT8 header = 0;
    DWORD dwReadLen = 0;
    UINT16 dwDatalen = 0;

    TRC();

    if(!glib_handle || !BT_ReadCommPort || (bt_fd < 0)){
		ERR("Invalid serial port or read function\n");
		return FALSE;
    }
	
    if(!BT_ReadCommPort(bt_fd, (UCHAR*)&pRevAclData->con_hdl, 2, &dwReadLen) || (dwReadLen < 2)){
        return FALSE;
    }
    
    pRevAclData->con_hdl = ((pRevAclData->con_hdl>>8)&&0xff)|((pRevAclData->con_hdl&&0xff)<<8);
    
    dwReadLen = 0;    
    if(!BT_ReadCommPort(bt_fd, (UCHAR*)&pRevAclData->len, 2, &dwReadLen) || (dwReadLen < 2)){
        return FALSE;
    }
    
    dwDatalen = ((pRevAclData->len&0xff)<<8)|((pRevAclData->len>>8)&0xff); 
    pRevAclData->len = dwDatalen;
	
    if(pRevAclData->len){
        dwReadLen = 0;
        if(!BT_ReadCommPort(bt_fd, pRevAclData->buffer, dwDatalen, &dwReadLen) || (dwReadLen < dwDatalen)){
            return FALSE;
        }
    }
    return TRUE;    
}

static void *BT_MetaThread( void *ptr )
{
    BT_HCI_EVENT hci_event;
    BOOL  RetVal = TRUE;
    UINT8 ucHeader = 0;
    DWORD dwReadLength = 0;
    BT_CNF *pBt_CNF = (BT_CNF*)ptr;
    BT_HCI_BUFFER temp_acl_data;

    TRC();
    
    while(!bKillThread)
    {
        if(!glib_handle || !BT_ReadCommPort || (bt_fd < 0)){
            ERR("Invalid serial port or read function\n");
            break;
        }
        
        dwReadLength = 0;
        RetVal = BT_ReadCommPort(bt_fd, &ucHeader, sizeof(ucHeader), &dwReadLength);
        if(!RetVal || (dwReadLength == 0)){
            ERR("zero byte read\n");
            continue;
        }
        
        switch (ucHeader)
        {
            case 0x04:
                DBG("HCI Event\n");
                if(BT_Recv_HciEvent(&hci_event))
                {     
                    pBt_CNF->bt_status = TRUE;
                    pBt_CNF->eventtype = EVTTYPE_EVENT;
                    memcpy(&pBt_CNF->bt_result.hcievent,  &hci_event, sizeof(hci_event));
                    pBt_CNF->status = META_SUCCESS;   
                    bt_send_resp(&bt_cnf, sizeof(BT_CNF), NULL, 0);
                }
                else{
                    pBt_CNF->bt_status = FALSE;
                    pBt_CNF->eventtype = EVTTYPE_EVENT;
                    pBt_CNF->status = META_FAILED;                  
                    bt_send_resp(&bt_cnf, sizeof(BT_CNF), NULL, 0);
                }
                break;
                
            case 0x02:
                DBG("HCI ACL\n");
                if(BT_Recv_AclData(&temp_acl_data))
                {
                    pBt_CNF->bt_status = TRUE;
                    pBt_CNF->eventtype = EVTTYPE_ACL;
                    memcpy(&pBt_CNF->bt_result.hcibuffer,  &temp_acl_data, sizeof(temp_acl_data));
                    pBt_CNF->status = META_SUCCESS;
                    bt_send_resp(&bt_cnf, sizeof(BT_CNF), NULL, 0);
                }
                else{
                    pBt_CNF->bt_status = FALSE;
                    pBt_CNF->eventtype = EVTTYPE_ACL;
                    pBt_CNF->status = META_FAILED;      
                    bt_send_resp(&bt_cnf, sizeof(BT_CNF), NULL, 0);
                }
                break;                
                
            default:
                ERR("Unexpected BT packet header");
                ERR("Packet:%d is not process now \r\n", ucHeader);
                break; 
        }        
    }    

    return 0;
}
