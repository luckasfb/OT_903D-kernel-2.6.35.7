






#include "gl_os.h"
#include "gl_kal.h"

#include "wlan_lib.h"
#include "debug.h"


#define PROC_MCR_ACCESS                         "mcr"
#define PROC_DRV_STATUS                         "status"
#define PROC_RX_STATISTICS                      "rx_statistics"
#define PROC_TX_STATISTICS                      "tx_statistics"
#define PROC_DBG_LEVEL                          "dbg_level"
#define PROC_DBG_OID							"dbg_oid"

#define PROC_MCR_ACCESS_MAX_USER_INPUT_LEN      20
#define PROC_RX_STATISTICS_MAX_USER_INPUT_LEN   10
#define PROC_TX_STATISTICS_MAX_USER_INPUT_LEN   10
#define PROC_DBG_LEVEL_MAX_USER_INPUT_LEN       20
#define PROC_DBG_LEVEL_MAX_DISPLAY_STR_LEN      30
#define PROC_OID_MAX_USER_INPUT_LEN 			50
#define PROC_OID_MAX_MAX_DISPLAY_LEN 			50




static UINT_32 u4McrOffset = 0;


extern struct proc_dir_entry * proc_net_mkdir_wrapper(
    struct net *net, 
    const char *name,
    struct proc_dir_entry *parent);
	
extern struct proc_dir_entry *create_proc_entry_wrapper(
    const char *name,
    mode_t mode, 
    struct proc_dir_entry *parent);
	
extern struct proc_dir_entry *create_proc_read_entry_wrapper(
    const char *name,
    mode_t mode, 
    struct proc_dir_entry *base, 
    read_proc_t *read_proc, 
    void * data) ;
	
extern void remove_proc_entry_wrapper(
    const char *name, 
    struct proc_dir_entry *parent);
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
procMCRRead (
    char *page,
    char **start,
    off_t off,
    int count,
    int *eof,
    void *data
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    PARAM_CUSTOM_MCR_RW_STRUC_T rMcrInfo;
    UINT_32 u4BufLen;
    char *p = page;
    UINT_32 u4Count;

    GLUE_SPIN_LOCK_DECLARATION();

    // Kevin: Apply PROC read method 1.
    if (off != 0) {
        return 0; // To indicate end of file.
    }

    ASSERT(page && eof && data);
    if (!data || !eof || !data) {
        printk(KERN_WARNING DRV_NAME"%s Invalid input data\n", __FUNCTION__);
        return -EINVAL;
    }

    prGlueInfo = (P_GLUE_INFO_T)netdev_priv((struct net_device *)data);
    if (!prGlueInfo) {
        printk(KERN_WARNING DRV_NAME"%s No glue info\n", __FUNCTION__);
        return -EINVAL;
    }

#if !defined(_HIF_SDIO)
    GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);
#endif

    rMcrInfo.u4McrOffset = u4McrOffset;

    //printk("Read MCR 0x%lx: 0x%lx\n",
        //rMcrInfo.u4McrOffset, rMcrInfo.mcrData);

#if defined(_HIF_SDIO)
   sdio_io_ctrl(prGlueInfo,
                     wlanoidQueryMcrRead,
                     (PVOID)&rMcrInfo,
                     sizeof(rMcrInfo),
                     TRUE,
                     TRUE,
                     &u4BufLen);
#else
    wlanQueryInformation(prGlueInfo->prAdapter,
                         wlanoidQueryMcrRead,
                         (PVOID)&rMcrInfo,
                         sizeof(rMcrInfo),
                         &u4BufLen);
#endif

#if !defined(_HIF_SDIO)
    GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);
#endif

    SPRINTF(p, ("MCR (%#04lxh): %#08lx\n",
        rMcrInfo.u4McrOffset, rMcrInfo.u4McrData));

    u4Count = (UINT_32)(p - page);

    *eof = 1;

    return (int)u4Count;

} /* end of procMCRRead() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
procMCRWrite (
    struct file *file,
    const char *buffer,
    unsigned long count,
    void *data
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    char acBuf[PROC_MCR_ACCESS_MAX_USER_INPUT_LEN + 1]; // + 1 for "\0"
    UINT_32 u4CopySize;
    PARAM_CUSTOM_MCR_RW_STRUC_T rMcrInfo;
    UINT_32 u4BufLen;

    GLUE_SPIN_LOCK_DECLARATION();

    ASSERT(data);
    if (!data) {
        printk(KERN_WARNING DRV_NAME"%s Invalid input data\n", __FUNCTION__);
        return -EINVAL;
    }

    prGlueInfo = (P_GLUE_INFO_T)netdev_priv((struct net_device *)data);
    if (!prGlueInfo) {
        printk(KERN_WARNING DRV_NAME"%s No glue info\n", __FUNCTION__);
        return -EINVAL;
    }

    u4CopySize = (count < PROC_RX_STATISTICS_MAX_USER_INPUT_LEN)
        ? count : PROC_RX_STATISTICS_MAX_USER_INPUT_LEN;
    if (copy_from_user(acBuf, buffer, u4CopySize)) {
        printk(KERN_WARNING DRV_NAME"%s copy_from_user() fail\n", __FUNCTION__);
        return -EFAULT;
    }
    acBuf[u4CopySize] = '\0';

    switch (sscanf(acBuf, "0x%lx 0x%lx",
                   &rMcrInfo.u4McrOffset, &rMcrInfo.u4McrData)) {
    case 2:
        /* NOTE: Sometimes we want to test if bus will still be ok, after accessing
         * the MCR which is not align to DW boundary.
         */
        //if (IS_ALIGN_4(rMcrInfo.u4McrOffset))
        {
            u4McrOffset = rMcrInfo.u4McrOffset;

            //printk("Write 0x%lx to MCR 0x%04lx\n",
                //rMcrInfo.u4McrOffset, rMcrInfo.u4McrData);

#if defined(_HIF_SDIO)
            sdio_io_ctrl(prGlueInfo,
                               wlanoidSetMcrWrite,
                               (PVOID)&rMcrInfo,
                               sizeof(rMcrInfo),
                               FALSE,
                               TRUE,
                               &u4BufLen);
#else
            GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);

            wlanSetInformation(prGlueInfo->prAdapter,
                               wlanoidSetMcrWrite,
                               (PVOID)&rMcrInfo,
                               sizeof(rMcrInfo),
                               &u4BufLen);

            GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);
#endif
        }
        break;

    case 1:
        //if (IS_ALIGN_4(rMcrInfo.u4McrOffset))
        {
            u4McrOffset = rMcrInfo.u4McrOffset;
        }
        break;

    default:
        break;
    }

    return count;

} /* end of procMCRWrite() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
procDrvStatusRead (
    char *page,
    char **start,
    off_t off,
    int count,
    int *eof,
    void *data
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    char *p = page;
    UINT_32 u4Count;

    GLUE_SPIN_LOCK_DECLARATION();

    // Kevin: Apply PROC read method 1.
    if (off != 0) {
        return 0; // To indicate end of file.
    }

    ASSERT(page || eof || data);
    if (!data || !eof || !data) {
        printk(KERN_WARNING DRV_NAME"%s Invalid input data\n", __FUNCTION__);
        return -EINVAL;
    }

    prGlueInfo = (P_GLUE_INFO_T)netdev_priv((struct net_device *)data);
    if (!prGlueInfo) {
        printk(KERN_WARNING DRV_NAME"%s No glue info\n", __FUNCTION__);
        return -EINVAL;
    }

    SPRINTF(p, ("GLUE LAYER STATUS:"));
    SPRINTF(p, ("\n=================="));

    SPRINTF(p, ("\n* Number of Pending Frames: %ld\n",
        prGlueInfo->u4TxPendingFrameNum));

    GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);

    wlanoidQueryDrvStatusForLinuxProc(prGlueInfo->prAdapter, p, &u4Count);

    GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);

    u4Count += (UINT_32)(p - page);

    *eof = 1;

    return (int)u4Count;

} /* end of procDrvStatusRead() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
procRxStatisticsRead (
    char *page,
    char **start,
    off_t off,
    int count,
    int *eof,
    void *data
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    char *p = page;
    UINT_32 u4Count;

    GLUE_SPIN_LOCK_DECLARATION();

    // Kevin: Apply PROC read method 1.
    if (off != 0) {
        return 0; // To indicate end of file.
    }

    ASSERT(page && eof && data);
    if (!data || !eof || !data) {
        printk(KERN_WARNING DRV_NAME"%s Invalid input data\n", __FUNCTION__);
        return -EINVAL;
    }

    prGlueInfo = (P_GLUE_INFO_T)netdev_priv((struct net_device *)data);
    if (!prGlueInfo) {
        printk(KERN_WARNING DRV_NAME"%s No glue info\n", __FUNCTION__);
        return -EINVAL;
    }

    SPRINTF(p, ("RX STATISTICS (Write 1 to clear):"));
    SPRINTF(p, ("\n=================================\n"));

    GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);

    wlanoidQueryRxStatisticsForLinuxProc(prGlueInfo->prAdapter, p, &u4Count);

    GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);

    u4Count += (UINT_32)(p - page);

    *eof = 1;

    return (int)u4Count;

} /* end of procRxStatisticsRead() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
procRxStatisticsWrite (
    struct file *file,
    const char *buffer,
    unsigned long count,
    void *data
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    char acBuf[PROC_RX_STATISTICS_MAX_USER_INPUT_LEN + 1]; // + 1 for "\0"
    UINT_32 u4CopySize;
    UINT_32 u4ClearCounter;

    GLUE_SPIN_LOCK_DECLARATION();

    ASSERT(data);
    if (!data) {
        printk(KERN_WARNING DRV_NAME"%s Invalid input data\n", __FUNCTION__);
        return -EINVAL;
    }

    prGlueInfo = (P_GLUE_INFO_T)netdev_priv((struct net_device *)data);
    if (!prGlueInfo) {
        printk(KERN_WARNING DRV_NAME"%s No glue info\n", __FUNCTION__);
        return -EINVAL;
    }

    u4CopySize = (count < PROC_RX_STATISTICS_MAX_USER_INPUT_LEN)
        ? count : PROC_RX_STATISTICS_MAX_USER_INPUT_LEN;
    if (copy_from_user(acBuf, buffer, u4CopySize)) {
        printk(KERN_WARNING DRV_NAME"%s copy_from_user() fail\n", __FUNCTION__);
        return -EFAULT;
    }
    acBuf[u4CopySize] = '\0';

    if (sscanf(acBuf, "%ld", &u4ClearCounter) == 1) {
        if (u4ClearCounter == 1) {
            GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);

            wlanoidSetRxStatisticsForLinuxProc(prGlueInfo->prAdapter);

            GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);
        }
    }

    return count;

} /* end of procRxStatisticsWrite() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
procTxStatisticsRead (
    char *page,
    char **start,
    off_t off,
    int count,
    int *eof,
    void *data
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    char *p = page;
    UINT_32 u4Count;

    GLUE_SPIN_LOCK_DECLARATION();

    // Kevin: Apply PROC read method 1.
    if (off != 0) {
        return 0; // To indicate end of file.
    }

    ASSERT(page && eof && data);
    if (!data || !eof || !data) {
        printk(KERN_WARNING DRV_NAME"%s Invalid input data\n", __FUNCTION__);
        return -EINVAL;
    }

    prGlueInfo = (P_GLUE_INFO_T)netdev_priv((struct net_device *)data);
    if (!prGlueInfo) {
        printk(KERN_WARNING DRV_NAME"%s No glue info\n", __FUNCTION__);
        return -EINVAL;
    }

    SPRINTF(p, ("TX STATISTICS (Write 1 to clear):"));
    SPRINTF(p, ("\n=================================\n"));

    GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);

    wlanoidQueryTxStatisticsForLinuxProc(prGlueInfo->prAdapter, p, &u4Count);

    GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);

    u4Count += (UINT_32)(p - page);

    *eof = 1;

    return (int)u4Count;

} /* end of procTxStatisticsRead() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
procTxStatisticsWrite (
    struct file *file,
    const char *buffer,
    unsigned long count,
    void *data
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    char acBuf[PROC_RX_STATISTICS_MAX_USER_INPUT_LEN + 1]; // + 1 for "\0"
    UINT_32 u4CopySize;
    UINT_32 u4ClearCounter;

    GLUE_SPIN_LOCK_DECLARATION();

    ASSERT(data);
    if (!data) {
        printk(KERN_WARNING DRV_NAME"%s Invalid input data\n", __FUNCTION__);
        return -EINVAL;
    }

    prGlueInfo = (P_GLUE_INFO_T)netdev_priv((struct net_device *)data);
    if (!prGlueInfo) {
        printk(KERN_WARNING DRV_NAME"%s No glue info\n", __FUNCTION__);
        return -EINVAL;
    }

    u4CopySize = (count < PROC_RX_STATISTICS_MAX_USER_INPUT_LEN)
        ? count : PROC_RX_STATISTICS_MAX_USER_INPUT_LEN;
    if (copy_from_user(acBuf, buffer, u4CopySize)) {
        printk(KERN_WARNING DRV_NAME"%s copy_from_user() fail\n", __FUNCTION__);
        return -EFAULT;
    }
    acBuf[u4CopySize] = '\0';

    if (sscanf(acBuf, "%ld", &u4ClearCounter) == 1) {
        if (u4ClearCounter == 1) {
            GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);

            wlanoidSetTxStatisticsForLinuxProc(prGlueInfo->prAdapter);

            GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);
        }
    }

    return count;

} /* end of procTxStatisticsWrite() */


#if DBG
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
procDbgLevelRead (
    char *page,
    char **start,
    off_t off,
    int count,
    int *eof,
    void *data
    )
{
    char *p = page;
    int i;

    UINT_8 aucDbModuleName[][PROC_DBG_LEVEL_MAX_DISPLAY_STR_LEN] = {
        "DBG_INIT_IDX",
        "DBG_ARB_IDX",
        "DBG_TEST_IDX",
        "DBG_SCAN_IDX",
        "DBG_JOIN_IDX",
        "DBG_ROAMING_IDX",
        "DBG_NIC_IDX",
        "DBG_PHY_IDX",
        "DBG_HAL_IDX",
        "DBG_INTR_IDX",
        "DBG_TX_IDX",
        "DBG_RX_IDX",
        "DBG_REQ_IDX",
        "DBG_MGT_IDX",
        "DBG_RSN_IDX",
        "DBG_LP_IDX",
        "DBG_RFTEST_IDX",
        "DBG_LP_IOT_IDX",
        "DBG_RCPI_MEASURE_IDX",
        "DBG_DOMAIN_IDX",

        "DBG_WH_IDX",
        "DBG_KEVIN_IDX",
        "DBG_MIKE_IDX",
        "DBG_KENT_IDX",
        "DBG_GEORGE_IDX",
        "DBG_CMC_IDX"
        };


    // Kevin: Apply PROC read method 1.
    if (off != 0) {
        return 0; // To indicate end of file.
    }

    for (i = 0; i < (sizeof(aucDbModuleName)/PROC_DBG_LEVEL_MAX_DISPLAY_STR_LEN); i++) {
        SPRINTF(p, ("%c %-15s(0x%02x): %02x\n",
            ((i == u4DebugModule) ? '*' : ' '),
            &aucDbModuleName[i][0],
            i,
            aucDebugModule[i]));
    }

    *eof = 1;
    return (int)(p - page);
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
procDbgLevelWrite (
    struct file *file,
    const char *buffer,
    unsigned long count,
    void *data
    )
{
    char acBuf[PROC_DBG_LEVEL_MAX_USER_INPUT_LEN + 1]; // + 1 for "\0"
    UINT_32 u4CopySize;
    UINT_32 u4NewDbgModule, u4NewDbgLevel;


    u4CopySize = (count < (sizeof(acBuf) - 1)) ? count : (sizeof(acBuf) - 1);
    if (copy_from_user(acBuf, buffer, u4CopySize)) {
        printk(KERN_INFO DRV_NAME"procDbgLevelWrite copy_from_user() fail\n");
    }

    acBuf[u4CopySize] = '\0';

    if (sscanf(acBuf, "0x%lx 0x%lx", &u4NewDbgModule, &u4NewDbgLevel) == 2) {
        if (u4NewDbgModule < DBG_MODULE_NUM) {
            u4DebugModule = u4NewDbgModule;
            u4NewDbgLevel &= DBG_CLASS_MASK;
            aucDebugModule[u4DebugModule] = (UINT_8)u4NewDbgLevel;
        }
    }

    return count;
}
#endif /* DBG */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
procOidWrite(
	struct file *file,
	const char *buffer,
	unsigned long count,
	void *data
	)
{
	P_GLUE_INFO_T prGlueInfo = NULL;	
	char acBuf[PROC_OID_MAX_USER_INPUT_LEN + 1];//+1 for "\0"
	UINT_32 u4CopySize;
	UINT_8 u4Ip[4];
	UINT_32 u4BufLen;
	int i = 0;

	GLUE_SPIN_LOCK_DECLARATION();

	printk(KERN_INFO DRV_NAME "%s ++\n", __FUNCTION__);
	for(i = 0; i < count; i++)
		printk("%c", buffer[i]);
	printk(KERN_INFO DRV_NAME "\n");

	ASSERT(data);
	if(!data){
		printk(KERN_WARNING DRV_NAME"%s Invalid input data\n", __FUNCTION__);
		return -EINVAL;
	}

	prGlueInfo = (P_GLUE_INFO_T)netdev_priv((struct net_device *)data);
	
	if(!prGlueInfo){
		printk(KERN_WARNING DRV_NAME"%s NO glue info\n", __FUNCTION__);
		return -EINVAL;
	}
	
	u4CopySize = (count < (sizeof(acBuf) - 1))? count : (sizeof(acBuf) - 1);
    
	if(copy_from_user(acBuf, buffer, u4CopySize)){
		printk(KERN_INFO DRV_NAME"procOidWrite copy_from_user() failed\n");
	}

	acBuf[u4CopySize] = "\0";
	
	printk(KERN_INFO DRV_NAME "%s: %s\n", __FUNCTION__, acBuf);

#if 0
	if(4 == sscanf(acBuf, "%d.%d.%d.%d", u4Ip, u4Ip + 1, u4Ip + 2, u4Ip + 3)){

#if defined(_HIF_SDIO)
					sdio_io_ctrl(prGlueInfo,
								 wlanoidSetIpAddress,
								 (PVOID)&u4Ip,
								  sizeof(u4Ip),
								  FALSE,
								  TRUE,
								  &u4BufLen);
#else
					GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);
		
					wlanSetInformation(prGlueInfo->prAdapter,
									   wlanoidSetIpAddress,
									   (PVOID)&u4Ip,
									   sizeof(u4Ip),
									   &u4BufLen);
		
					GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);
#endif
	}
#endif


#if defined(_HIF_SDIO)
						sdio_io_ctrl(prGlueInfo,
									 wlanoidOidTest,
									 (PVOID)acBuf,
									  count,
									  FALSE,
									  TRUE,
									  &u4BufLen);
#else
						GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);
			
						wlanSetInformation(prGlueInfo->prAdapter,
										   wlanoidOidTest,
										   (PVOID)acBuf,
										   count,
										   &u4BufLen);
			
						GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);
#endif

	return count;	

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
INT_32
procInitProcfs (
    struct net_device *prDev,
    char *pucDevName
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    struct proc_dir_entry *prEntry;

    struct proc_dir_entry *prKernelProcNet = init_net.proc_net;

    ASSERT(prDev);
    if (!prDev) {
        printk(KERN_WARNING DRV_NAME"%s Invalid input data\n", __FUNCTION__);
        return -EINVAL;
    }

    if (!prKernelProcNet) {
        printk("init proc fs fail: proc_net == NULL\n");
        return -ENOENT;
    }

    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prDev);
    if (!prGlueInfo) {
        printk(KERN_WARNING DRV_NAME"%s No glue info\n", __FUNCTION__);
        return -EINVAL;
    }

    /*
    /proc/net/wlan0
               |-- mcr              (PROC_MCR_ACCESS)
               |-- status           (PROC_DRV_STATUS)
               |-- rx_statistics    (PROC_RX_STATISTICS)
               |-- tx_statistics    (PROC_TX_STATISTICS)
               |-- dbg_level        (PROC_DBG_LEVEL)
               |-- (end)
     */

    /*
    * Directory: Root (/proc/net/wlan0)
    */

    prGlueInfo->pProcRoot = proc_mkdir(pucDevName, prKernelProcNet);

    if (prGlueInfo->pProcRoot == NULL) {
        return -ENOENT;
    }

    /* File Root/mcr (RW) */
    prEntry = create_proc_entry(PROC_MCR_ACCESS, 0, prGlueInfo->pProcRoot);
    if (prEntry) {
        prEntry->read_proc = procMCRRead;
        prEntry->write_proc = procMCRWrite;
        prEntry->data = (void *)prDev;
    }

    /* File Root/status (RW) */
    prEntry = create_proc_read_entry(PROC_DRV_STATUS, 0, prGlueInfo->pProcRoot,
                                     procDrvStatusRead, prDev);

    /* File Root/rx_statistics (RW) */
    prEntry = create_proc_entry(PROC_RX_STATISTICS, 0, prGlueInfo->pProcRoot);
    if (prEntry) {
        prEntry->read_proc = procRxStatisticsRead;
        prEntry->write_proc = procRxStatisticsWrite;
        prEntry->data = (void *)prDev;
    }

    /* File Root/tx_statistics (RW) */
    prEntry = create_proc_entry(PROC_TX_STATISTICS, 0, prGlueInfo->pProcRoot);
    if (prEntry) {
        prEntry->read_proc = procTxStatisticsRead;
        prEntry->write_proc = procTxStatisticsWrite;
        prEntry->data = (void *)prDev;
    }

#if DBG
    /* File Root/dbg_level (RW) */
    prEntry = create_proc_entry(PROC_DBG_LEVEL, 0644, prGlueInfo->pProcRoot);
    if (prEntry) {
        prEntry->read_proc = procDbgLevelRead;
        prEntry->write_proc = procDbgLevelWrite;
    }
#endif /* DBG */

	prEntry = create_proc_entry(PROC_DBG_OID, 0, prGlueInfo->pProcRoot);
	if(prEntry) {
		prEntry->write_proc = procOidWrite;
		prEntry->data = (void *)prDev;
	}

    return 0;

} /* end of procInitProcfs() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
INT_32
procRemoveProcfs (
    struct net_device *prDev,
    char *pucDevName
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24)
    struct proc_dir_entry *prKernelProcNet = init_net.proc_net;
#else
    struct proc_dir_entry *prKernelProcNet = proc_net;
#endif

    ASSERT(prDev);

    if (!prDev) {
        printk(KERN_WARNING DRV_NAME"%s Invalid input data\n", __FUNCTION__);
        return -EINVAL;
    }

    if (!prKernelProcNet) {
        printk(KERN_ALERT "remove proc fs fail: proc_net == NULL\n");
        return -ENOENT;
    }

    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prDev);
    if (!prGlueInfo || !prGlueInfo->pProcRoot) {
        printk(KERN_ALERT "The procfs root is NULL\n");
        return -ENOENT;
    }

#if DBG
    remove_proc_entry(PROC_DBG_LEVEL,       prGlueInfo->pProcRoot);
#endif /* DBG */
    remove_proc_entry(PROC_TX_STATISTICS,   prGlueInfo->pProcRoot);
    remove_proc_entry(PROC_RX_STATISTICS,   prGlueInfo->pProcRoot);
    remove_proc_entry(PROC_DRV_STATUS,      prGlueInfo->pProcRoot);
    remove_proc_entry(PROC_MCR_ACCESS,      prGlueInfo->pProcRoot);
    remove_proc_entry(PROC_DBG_OID, 	    prGlueInfo->pProcRoot);   

    /* remove root directory (proc/net/wlan0) */
    remove_proc_entry(pucDevName, prKernelProcNet);

    return 0;

} /* end of procRemoveProcfs() */


