









#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio.h>
//#include <linux/mmc/protocol.h>

#include <linux/mmc/sdio_func.h> 
#include <linux/mmc/sdio_ids.h> 

#include <asm/memory.h>
#include <linux/mm.h>

#include <linux/firmware.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/platform_device.h>

#include "gl_typedef.h"
#include "mt592x_reg.h"
#include "platform.h"

#define BW_COMODULE

static const struct sdio_device_id mtk_sdio_ids[] = {
	{ SDIO_DEVICE(0x037a, 0x5921) }, /* Not an SDIO standard class device */
	{ /* end: all zeroes */ },
};

//static const int MAX_IOREQ_NUM = 15;

MODULE_DEVICE_TABLE(sdio, mtk_sdio_ids);

typedef void(*isr)(unsigned long u4Data);
typedef	int (*inthandler)(void *);

typedef struct _mtk_sdio_drv_t {
    struct net_device *prNetDevice;
    isr pfnIsr;
    void * prDevInfo;
} mtk_sdio_drv_t, *p_mtk_sdio_drv_t;


enum _ENUM_FUNC_IDX{
    FUNC_IDX_DS = 0, //Daisy chain
    FUNC_IDX_PLUGIN,   // plug-in func
    FUNC_IDX_NUM
};

void * pvSdio = NULL;
static probe_card pfPluginProbe = NULL;
static remove_card pfPluginRemove = NULL;
static inthandler pfPluginInthandler = NULL;
static struct sdio_driver mtk_sdio_driver = {
	.name		= "MTKSD", /* "MTK SDIO WLAN Driver" */
	.id_table		= mtk_sdio_ids,
	.probe		= NULL,
	.remove		= NULL,
};

static int gRefCount;
#define DRV_NAME "[SDIO Client]"
extern int platform_init(void);
extern void platform_deinit(void);
extern int register_comodule_cb(void * cb);
extern int deregister_comodule_cb(void);
//extern void mmc_power_up(struct mmc_host *host);
//extern void mmc_power_off(struct mmc_host *host);
extern void mmc_power_up_ext(struct mmc_host *host);
extern void mmc_power_off_ext(struct mmc_host *host);

#ifdef CFG_EINT_HANDLED_IN_WLAN
extern void mt5921_wifi_request_irq(sdio_irq_handler_t irq_handler, void *data);
extern void mt5921_wifi_enable_irq(void);
extern void mt5921_wifi_disable_irq(void);
#endif

int comodule_status_handler(int funcIdx, int op);
BOOL EnableDaisyChain( void * pvData );
BOOL mt5921SetSysOff( void * pvData	);
BOOL
hifBusInit (
    void * pvData
    );

static BOOL
hifDevRegRead (
       void *          pHif,
       unsigned long         u4Register,
      unsigned long *       pu4Value
    )
{
    struct sdio_func *prSdioFunc = (struct sdio_func *)pHif;
    int ret = 0;

    //printk(KERN_INFO DRV_NAME"++hifDevRegRead++ buf:0x%p, reg:0x%x\n", pu4Value, u4Register);

    sdio_claim_host(prSdioFunc);
    *pu4Value = sdio_readl(prSdioFunc, u4Register, &ret);
    sdio_release_host(prSdioFunc);

    //printk(KERN_INFO DRV_NAME"--hifDevRegRead-- buf:0x%p, reg:0x%x, val:0x%x\n", pu4Value, u4Register, *pu4Value);

    return (ret) ? FALSE : TRUE;
} /* end of hifDevRegRead() */

static BOOL
hifDevRegWrite (
       void *          pHif,
      unsigned long        u4Register,
      unsigned long        u4Value
    )
{
    struct sdio_func *prSdioFunc = (struct sdio_func *)pHif;
    int ret = 0;
    
    //printk(KERN_INFO DRV_NAME"++hifDevRegWrite++ reg:0x%x, val:0x%x\n", u4Register, u4Value);

    sdio_claim_host(prSdioFunc);
    sdio_writel(prSdioFunc, u4Value, u4Register, &ret);
    sdio_release_host(prSdioFunc);
    //printk(KERN_INFO DRV_NAME"hifDevRegWrite MCR(%#x) = %#x, %d\n", u4Register, u4Value, ret);

    //printk(KERN_INFO DRV_NAME"--hifDevRegWrite-- ret=%d\n", ret);

    return (ret) ? FALSE : TRUE;
} /* end of hifDevRegWrite() */

static BOOL
hifDevPortRead (
       void *          pHif,
       unsigned short         u2Port,
       unsigned short         u2Len,
      unsigned char *        pucBuf,
       unsigned short         u2ValidOutBufSize
    )
{
    struct sdio_func *prSdioFunc = NULL;
    unsigned char * pucDst = NULL;
    int count = u2Len;
    int ret = 0;

    //printk(KERN_INFO DRV_NAME"++hifDevPortRead++ buf:0x%p, port:0x%x, length:%d\n", pucBuf, u2Port, u2Len);
    prSdioFunc = (struct sdio_func *)pHif;

    pucDst = pucBuf;

    sdio_claim_host(prSdioFunc);
#if 0
    ret = sdio_readsb(prSdioFunc, pucBuf, u2Port, u2Len);
#else
    /* Split buffer into multiple single block to workaround s3c-hsmmc driver */
    while (!ret && count >= prSdioFunc->cur_blksize) {
        ret = sdio_readsb(prSdioFunc, pucDst, u2Port, prSdioFunc->cur_blksize);
        pucDst += prSdioFunc->cur_blksize;
        count -= prSdioFunc->cur_blksize;
    }

    if (!ret && count > 0) {
        ret = sdio_readsb(prSdioFunc, pucDst, u2Port, count);
    }

    if (ret) {
        goto release_host;
    }
release_host:
#endif

    sdio_release_host(prSdioFunc);

    //printk(KERN_INFO DRV_NAME"-- hifDevPortRead-- ret=%d\n", ret);

    return (ret) ? FALSE : TRUE;
} /* end of hifDevPortRead() */


static BOOL
hifDevPortWrite (
      void *         pHif,
      unsigned short        u2Port,
      unsigned short        u2Len,
      unsigned char *       pucBuf,
      unsigned short        u2ValidInBufSize
    )
{
    struct sdio_func *prSdioFunc = NULL;
    unsigned char * pucSrc = NULL;
    int count = u2Len;
    int ret = 0;

    //printk(KERN_INFO DRV_NAME"++hifDevPortWrite++ buf:0x%p, port:0x%x, length:%d\n", pucBuf, u2Port, u2Len);
    prSdioFunc = (struct sdio_func *)pHif;

    pucSrc = pucBuf;

    sdio_claim_host(prSdioFunc);

    /* Split buffer into multiple single block to workaround s3c-hsmmc driver */
    while (!ret && count >= prSdioFunc->cur_blksize) {
        ret = sdio_writesb(prSdioFunc, u2Port, pucSrc, prSdioFunc->cur_blksize);
        pucSrc += prSdioFunc->cur_blksize;
        count -= prSdioFunc->cur_blksize;
    }

    if (!ret && count > 0) {
        ret = sdio_writesb(prSdioFunc, u2Port, pucSrc, count);
    }

    sdio_release_host(prSdioFunc);

    //printk(KERN_INFO DRV_NAME"-- hifDevPortWrite-- ret=%d\n", ret);

    return (ret) ? FALSE : TRUE;
} /* end of hifDevPortWrite() */


#ifdef CFG_EINT_HANDLED_IN_WLAN
static void hifDevEnableEint()
{
    mt5921_wifi_enable_irq();
}

static void hifDevDisableEint()
{
    mt5921_wifi_enable_irq();
}
#endif

/*******************************************************************/
/* SDIO callbacks                                                  */
/*******************************************************************/
void sdio_irq_func(void * pHif)
{
    struct sdio_func *prSdioFunc = ( struct sdio_func *)pHif;
    int ret = 0;

    //printk(KERN_INFO DRV_NAME"++ sdio_irq_func++\n");
    if(pfPluginInthandler == NULL)
    {
    	/*sdio_claim_host(prSdioFunc);
    	sdio_release_irq(prSdioFunc);
    	sdio_release_host(prSdioFunc);*/
		return;
    }
#ifndef CFG_EINT_HANDLED_IN_WLAN
    sdio_writel(prSdioFunc, 0x20, 0x3C, &ret);
#endif
    pfPluginInthandler((void *)prSdioFunc);
    //printk(KERN_INFO DRV_NAME"-- sdio_irq_func--\n");
	
}

/***************************************/
#define DRIVER_NAME	"mt5921"

static int wlan_release(struct device *dev)
{
	printk(KERN_INFO DRV_NAME"wlan_release ok.\n");
}

#if 0
static struct platform_device  mt5921_dev = {
	.name = DRIVER_NAME,
	.id = -1,
	.dev = {
		.release = wlan_release,
	},
};
#else
static struct platform_device  *mt5921_dev = NULL;
#endif 

static int mtk_sdio_probe (
    struct sdio_func *func,
    const struct sdio_device_id *id
    )
{
    int ret = 0;
    int i = 0;
    BOOL bRet;

    printk(KERN_INFO DRV_NAME "mtk_sdio_probe()\n");

    printk(KERN_INFO DRV_NAME "Basic struct size checking...\n");
    printk(KERN_INFO DRV_NAME "sizeof(struct device) = %d\n", sizeof(struct device));
    printk(KERN_INFO DRV_NAME "sizeof(struct mmc_host) = %d\n", sizeof(struct mmc_host));
    printk(KERN_INFO DRV_NAME "sizeof(struct mmc_card) = %d\n", sizeof(struct mmc_card));
    printk(KERN_INFO DRV_NAME "sizeof(struct mmc_driver) = %d\n", sizeof(struct mmc_driver));
    printk(KERN_INFO DRV_NAME "sizeof(struct mmc_data) = %d\n", sizeof(struct mmc_data));
    printk(KERN_INFO DRV_NAME "sizeof(struct mmc_command) = %d\n", sizeof(struct mmc_command));
    printk(KERN_INFO DRV_NAME "sizeof(struct mmc_request) = %d\n", sizeof(struct mmc_request));
    printk(KERN_INFO DRV_NAME "sizeof(struct sdio_func) = %d\n", sizeof(struct sdio_func));

    printk(KERN_INFO DRV_NAME "Card information checking...\n");
    printk(KERN_INFO DRV_NAME "func = 0x%p\n", func);
    printk(KERN_INFO DRV_NAME "Number of info = %d:\n", func->card->num_info);

    for (i = 0;i < func->card->num_info;i++) {
        printk(KERN_INFO DRV_NAME "info[%d]: %s\n", i, func->card->info[i]);
    }

    sdio_claim_host(func);
    ret = sdio_enable_func(func);
    sdio_release_host(func);
	printk(KERN_INFO "sdio_enable_func ret=%d\n", ret);

    if (ret) {
        printk(KERN_INFO DRV_NAME"sdio_enable_func failed!\n");
        goto out;
    }
    printk(KERN_INFO DRV_NAME"sdio_enable_func done!\n");

	
    bRet = hifBusInit((void *)func);

    /* Cannot get IO address from interface */
    if (FALSE == bRet) {
		printk(KERN_INFO DRV_NAME " bus init failed!\n");
	    ret = -EIO;
		goto out;
    }
    
    platform_init();
    
    pvSdio = (void *)func;

#ifdef BW_COMODULE
		register_comodule_cb((void *)comodule_status_handler);
	
#else
		if (pfPluginProbe((PVOID)func) != WLAN_STATUS_SUCCESS) {
			printk(KERN_WARNING DRV_NAME"pfPluginProbe fail!call pfWlanRemove()\n");
			pfWlanRemove();
		}
#endif
 
   mt5921_dev = platform_device_alloc(DRIVER_NAME, 1);
   if(NULL == mt5921_dev)
	printk(KERN_INFO"platform_device_alloc ERORO\n");    
 
   ret = platform_device_add(mt5921_dev);

out:
    printk(KERN_INFO DRV_NAME"mtk_sdio_probe() done(%d)\n", ret);

    return ret;
}


static void
mtk_sdio_remove (
    struct sdio_func *func
    )
{
    int ret = 0;   
 
    printk(KERN_INFO DRV_NAME"mtk_sdio_remove()\n");

    platform_device_unregister(mt5921_dev);

    if(func == NULL){
        printk(KERN_WARNING DRV_NAME"mtk_sdio_remove, error,  invalid param\n");
        return;
    }

#ifdef BW_COMODULE
		comodule_status_handler(0,0);
		comodule_status_handler(1,0);
	
		deregister_comodule_cb();
#else
		pfPluginRemove();
		printk(KERN_INFO DRV_NAME"pfPluginRemove done\n");
#endif
    platform_deinit();
    
    pvSdio = NULL;

    sdio_claim_host(func);
    sdio_disable_func(func);
    printk(KERN_INFO DRV_NAME"sdio_disable_func() done\n");
    sdio_release_host(func);

    printk(KERN_INFO DRV_NAME"mtk_sdio_remove() done\n");
}

static int
mtk_sdio_suspend (
    struct device * pDev,
    pm_message_t state
    )
{
    printk(KERN_INFO "mtk_sdio: mtk_sdio_suspend dev(0x%p)\n", pDev);
    
    return 0;
}

int mtk_sdio_resume (
    struct device * pDev
    )
{
    printk(KERN_INFO "mtk_sdio: mtk_sdio_resume dev(0x%p)\n", pDev);

    return 0;
}


int
hifRegisterBus (
    struct MT5921_ops * mtops,
    struct SDIOBUS_ops * busops
    )
{
    int ret = 0;

    printk(KERN_INFO DRV_NAME"++hifRegisterBus++\n");
    printk(KERN_INFO DRV_NAME"mtk_sdio: Copyright MediaTek Inc.\n");

    pfPluginProbe = mtops->probe;
    pfPluginRemove = mtops->remove;
    pfPluginInthandler = mtops->inthandler;

    busops->DevPortRead = hifDevPortRead;
    busops->DevPortWrite = hifDevPortWrite;
    busops->DevRegRead = hifDevRegRead;
    busops->DevRegWrite = hifDevRegWrite;
    busops->DevIrq = sdio_irq_func;
	
#ifdef CFG_EINT_HANDLED_IN_WLAN
    busops->DevEnableEint = hifDevEnableEint;
    busops->DevDisableEint = hifDevDisableEint; 
    printk(KERN_INFO DRV_NAME"DevDisableEint and hifDevEnableEint join us.\n");
#endif
	
    mtk_sdio_driver.probe = mtk_sdio_probe;
    mtk_sdio_driver.remove = mtk_sdio_remove;

    mtk_sdio_driver.drv.suspend = mtk_sdio_suspend;
    mtk_sdio_driver.drv.resume = mtk_sdio_resume;

    ret = sdio_register_driver(&mtk_sdio_driver);
	printk(KERN_INFO DRV_NAME"sdio_reg_drv ret=%d\n", ret);

    gRefCount ++;
	
    printk(KERN_INFO DRV_NAME"--hifRegisterBus--\n");

    return ret;
} /* end of hifRegisterBus() */


void
hifUnregisterBus(
    remove_card pfRemove
    )
{
    printk(KERN_INFO DRV_NAME"++hifUnregisterBus++\n");

    pfPluginProbe = NULL;
    pfPluginRemove = NULL;
    pfPluginInthandler = NULL;
    
	gRefCount --;

    if (gRefCount<0)
    {
    	printk(KERN_INFO DRV_NAME"gRefCount<0\n");
    }

	if(!gRefCount){
		sdio_unregister_driver(&mtk_sdio_driver);
		printk(KERN_INFO DRV_NAME"sdio_unregister_driver(&mtk_sdio_driver)\n");
	}
	
    printk(KERN_INFO DRV_NAME"--hifUnregisterBus--\n");
    return;
} /* end of hifUnregisterBus() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
struct platform_driver  mt5921_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},	
};


int hifRegisterPlatformDriver (
	suspend_callback	wlanSuspend,
	resume_callback		wlanResume )
{
	int ret = 0;	

	if(NULL != wlanSuspend)
		mt5921_driver.suspend = wlanSuspend;
	else{
		printk(KERN_INFO"glRegisterPlatformDriver wlanSuspend ERROR.\n");
		ret = -1;
	}

	if(NULL != wlanResume)
		mt5921_driver.resume = wlanResume;
	else{
		printk(KERN_INFO"glRegisterPlatformDriver wlanResume ERROR.\n");
		ret = -1;
	}

	ret = platform_driver_register(&mt5921_driver);
	printk(KERN_INFO "platform_driver_register return %d\n", ret);
	return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
int hifUnregisterPlatformDriver (
    )
{
	int ret = 0;	

	platform_driver_unregister(&mt5921_driver);

	mt5921_driver.suspend = NULL;
	mt5921_driver.resume = NULL;

	return ret;
}


BOOL
hifBusInit (
    void * pvData
    )
{
    struct sdio_func *func = NULL;
    int ret = 0;

    printk(KERN_INFO DRV_NAME"%s pvData = %p\n", __FUNCTION__, pvData);

    func = (struct sdio_func *)pvData;

    sdio_claim_host(func);
    ret = sdio_set_block_size(func, 512);
    sdio_release_host(func);

    if (ret) {
        printk(KERN_INFO DRV_NAME"sdio_set_block_size 512 failed!\n");
    }
    else {
        printk(KERN_INFO DRV_NAME"sdio_set_block_size 512 done!\n");
    }

    printk(KERN_INFO DRV_NAME"param: func->cur_blksize(%d)\n", func->cur_blksize);
    printk(KERN_INFO DRV_NAME"param: func->max_blksize(%d)\n", func->max_blksize);
    printk(KERN_INFO DRV_NAME"param: func->card->host->max_blk_size(%d)\n", func->card->host->max_blk_size);
    printk(KERN_INFO DRV_NAME"param: func->card->host->max_blk_count(%d)\n", func->card->host->max_blk_count);

    return TRUE;
} /* end of hifBusInit() */


void
hifBusRelease (
    void * pvData
    )
{
    return;
} /* end of hifBusRelease() */

int
hifBusSetIrq (
    void * pvData,
    void * pfnIsr
    )
{
    struct sdio_func *prSdioFunc = (struct sdio_func *)pvData;
	int ret = -1;

    if (!pvData || !pfnIsr) {
        return -1;
    }

#ifndef CFG_EINT_HANDLED_IN_WLAN    
    sdio_claim_host(prSdioFunc);
    ret = sdio_claim_irq(prSdioFunc, pfnIsr);
    sdio_release_host(prSdioFunc);
    printk(KERN_INFO "sdio_claim_irq ret=%d\n", ret);
#else
    mt5921_wifi_request_irq(pfnIsr, prSdioFunc);
    printk(KERN_INFO "hifBusSetIrq\n");	
#endif

    return 0;
} /* end of hifBusSetIrq() */


void
hifBusFreeIrq (
    void * pvData
    )
{
    struct sdio_func *prSdioFunc = (struct sdio_func *)pvData;
    printk(KERN_INFO DRV_NAME"++ hifBusFreeIrq++\n");
    
#ifndef CFG_EINT_HANDLED_IN_WLAN 
    sdio_claim_host(prSdioFunc);
    sdio_release_irq(prSdioFunc);
    sdio_release_host(prSdioFunc);
#endif
    printk(KERN_INFO DRV_NAME"-- hifBusFreeIrq -- \n");

    return;
} /* end of hifBusFreeIrq() */



int comodule_status_handler(int funcIdx, int op)
{
    static int funcStatus[FUNC_IDX_NUM]= { 0,0 };
    printk(KERN_INFO DRV_NAME"++ comodule_status_handler ++  fucIdx=%d op=%d \n", funcIdx, op);


    if( (funcIdx != FUNC_IDX_DS && funcIdx != FUNC_IDX_PLUGIN) || 
        (op != 0 && op != 1) ) {
        printk(KERN_ALERT DRV_NAME"comodule_status_handler: invalid param funcIdx %d, op %d!\n",
        funcIdx, op);
        return -1;
    }

    if( funcStatus[funcIdx] == op) {
        printk(KERN_WARNING DRV_NAME"comodule_status_handler: already in that status!\n");
        return 0;
    }
	
	if( (funcStatus[FUNC_IDX_DS] != 0 && funcStatus[FUNC_IDX_DS] != 1)||
		(funcStatus[FUNC_IDX_PLUGIN] != 0 && funcStatus[FUNC_IDX_PLUGIN] != 1) ) {
	    printk(KERN_ALERT DRV_NAME"comodule_status_handler: local funcStatus error: DS %d, Plug-in %d!\n",
			funcStatus[FUNC_IDX_DS], funcStatus[FUNC_IDX_PLUGIN]);
        return -1;
	}
	
    if( funcIdx == FUNC_IDX_DS) {
		//just enable or disable daisy chain
        if( op == 1 ) {
				if( funcStatus[FUNC_IDX_PLUGIN] == 1 ) {
						printk(KERN_ALERT "[MT5921] comodule_status_handler: cann't enable DS func since plug-in func running!\n");
						return -1;
				}

				if ( pvSdio == NULL ) {
						printk(KERN_ALERT "[MT5921] comodule_status_handler: invalid Sdio func handle!\n");
						return -1;
				}
				if( EnableDaisyChain(pvSdio) == FALSE ) {
						printk(KERN_ALERT "[MT5921] comodule_status_handler: Failed to enable daisy chain!\n");
						return -1;
				}

				if( mt5921SetSysOff(pvSdio) == FALSE ) {
						printk(KERN_ALERT "[MT5921] comodule_status_handler: Failed to lay mt5921 into system off!\n");
						return -1;
				}
				//mmc_power_off( ((struct sdio_func *)pvSdio)->card->host );
            		        mmc_power_off_ext( ((struct sdio_func *)pvSdio)->card->host );
			       //add by mtk80743

				funcStatus[FUNC_IDX_DS] = 1;
				return 0;
		} else {
				//mmc_power_up( ((struct sdio_func *)pvSdio)->card->host );
	    		        mmc_power_up_ext( ((struct sdio_func *)pvSdio)->card->host );
			        //add by mtk80743			
				funcStatus[FUNC_IDX_DS] = 0;
				return 0;
		    }
    }else {

  // need init or deinit plug in function
		if( op == 1 ) {
				if( funcStatus[FUNC_IDX_DS] == 1 ) {
						printk(KERN_WARNING "[MT5921] comodule_status_handler: disenable DS func first since DS func running!\n");
						//mmc_power_up( ((struct sdio_func *)pvSdio)->card->host );
						funcStatus[FUNC_IDX_DS] = 0;
				} 

				if( pfPluginProbe == NULL ) {
						printk(KERN_ALERT "[MT5921] comodule_status_handler: pfPluginProbe = NULL!\n");
						return -1;
				}
				if (pfPluginProbe((void *)pvSdio) != 0) {
						printk(KERN_ALERT "[MT5921] comodule_status_handler: failed to call pfPluginProbe!\n");
						pfPluginRemove();
						return -1;
				}

				funcStatus[FUNC_IDX_PLUGIN] = 1;	
				return 0;
		} else {
			if (pfPluginRemove) {
					pfPluginRemove();
					funcStatus[FUNC_IDX_PLUGIN] = 0;
			}
			return 0;
	}

}



}


BOOL EnableDaisyChain( void * pvData )
{
    unsigned long u4Scr;

    printk(KERN_INFO DRV_NAME"++ EnableDaisyChain ++\n");

    /* Enable daisy chain before set MT5921 to system off */ 
    printk(KERN_INFO "[MT5921] EnableDaisyChain ++!\n");
	
    if (hifDevRegRead(pvData, MCR_SCR, &u4Scr) == FALSE) 
    {               
	    printk(KERN_ALERT "[MT5921] EnableDaisyChain: failed to read MCR_SCR!\n");
        return FALSE;
    } 
    else
    { 
        printk(KERN_ALERT "[MT5921] EnableDaisyChain: MCR_SCR = 0x%lx!\n",u4Scr);
        u4Scr &= ~(SCR_GPIO0_ENABLE_OUTPUT_MODE | 
                   SCR_GPIO0_CHAIN_SEL |
                   SCR_GPIO0_WDATA |
                   SCR_GPIO2_ENABLE_OUTPUT_MODE | 
                   SCR_GPIO2_CHAIN_SEL |
                   SCR_GPIO2_WDATA |
                   SCR_GPIO1_ENABLE_OUTPUT_MODE | 
                   SCR_GPIO1_CHAIN_SEL);/* gpi */
        u4Scr |= SCR_GPIO0_POLAR_HIGH | SCR_GPIO1_POLAR_HIGH|SCR_GPIO1_CHAIN_SEL; /* Input, daisy */

        if( hifDevRegWrite(pvData, MCR_SCR, u4Scr) == FALSE)
        {
		    printk(KERN_ALERT "[MT5921] EnableDaisyChain: failed to write MCR_SCR!\n");
            return FALSE;
        }
    }

    return TRUE; 
}


BOOL mt5921SetSysOff( void * pvData  )
{
    unsigned long u4Scr;
    printk(KERN_INFO DRV_NAME"++ mt5921SetSysOff ++\n");


    printk(KERN_INFO "[MT5921] mt5921SetSysOff ++!\n");

    /* Renbang :  lay the mt5921 to system off mode */
    if( hifDevRegWrite(pvData, MCR_HLPCR, HLPCR_DISABLE_GINT) == FALSE)
    {
		printk(KERN_ALERT "[MT5921] mt5921SetSysOff: failed to Disable INT!\n");		
        return FALSE;
    }
        
    if (hifDevRegRead(pvData, MCR_IOPCR, &u4Scr) == FALSE) 
    {               
		printk(KERN_ALERT "[MT5921] mt5921SetSysOff: failed to read MCR_IOPCR!\n");		
        return FALSE;
    } 
    else
    {
        u4Scr &= ~IOPCR_ALL_TRAP_PIN_OUTPUT_EN;
        if( hifDevRegWrite(pvData, MCR_IOPCR, u4Scr) == FALSE)
        {
			printk(KERN_ALERT "[MT5921] mt5921SetSysOff: failed to write MCR_IOPCR!\n"); 	
			return FALSE;
        }				  
    }

    if( hifDevRegWrite(pvData, MCR_HLPCR, HLPCR_RF_SX_PD | HLPCR_PLL_CLOCK_GATED) == FALSE)
    {
	    printk(KERN_ALERT "[MT5921] mt5921SetSysOff: failed to write MCR_HLPCR 1!\n"); 			
        return FALSE;
    }
    else 
    {
        if( hifDevRegWrite(pvData, MCR_HLPCR, HLPCR_PD_ALL | HLPCR_PLL_CLOCK_GATED) == FALSE)
        {
	        printk(KERN_ALERT "[MT5921] mt5921SetSysOff: failed to write MCR_HLPCR 2!\n"); 						
            return FALSE;
        }
    }
    
    return TRUE; 
}



void hifSetInfo(void * pHif, void *prGlueInfo)
{
    struct sdio_func *prSdioFunc = ( struct sdio_func *)pHif;
    
    printk(KERN_INFO DRV_NAME"++ hifSetInfo++\n");
    sdio_set_drvdata(prSdioFunc, prGlueInfo);
    printk(KERN_INFO DRV_NAME"-- hifSetInfo--\n");

}

static int __init initSdio(void)
{
    int ret = 0;
    printk(KERN_INFO DRV_NAME"++ initSdio++\n");
	
	gRefCount = 0;
#if 0	
    mtk_sdio_driver.probe = mtk_sdio_probe;
    mtk_sdio_driver.remove = mtk_sdio_remove;

    mtk_sdio_driver.drv.suspend = mtk_sdio_suspend;
    mtk_sdio_driver.drv.resume = mtk_sdio_resume;

    ret = sdio_register_driver(&mtk_sdio_driver);

    printk(KERN_INFO DRV_NAME"--initSdio(%d)--\n", ret);
#endif

	printk(KERN_INFO DRV_NAME"-- initSdio--\n");
	
    return ret;
}

static VOID __exit exitSdio(void)
{
    if (gRefCount > 0)
    {
    	return;
    }

    printk( KERN_INFO "exitSdio\n");

    return;
} /* end of exitWlan() */


module_init(initSdio);
module_exit(exitSdio);

MODULE_LICENSE("GPL");

EXPORT_SYMBOL(hifRegisterBus);
EXPORT_SYMBOL(hifUnregisterBus);
EXPORT_SYMBOL(hifRegisterPlatformDriver);
EXPORT_SYMBOL(hifUnregisterPlatformDriver);

EXPORT_SYMBOL(hifBusInit);
EXPORT_SYMBOL(hifBusRelease);
EXPORT_SYMBOL(hifBusSetIrq);
EXPORT_SYMBOL(hifBusFreeIrq);
//EXPORT_SYMBOL(hifDevRegRead);
//EXPORT_SYMBOL(hifDevRegWrite);
//EXPORT_SYMBOL(hifDevPortRead);
//EXPORT_SYMBOL(hifDevPortWrite);
EXPORT_SYMBOL(comodule_status_handler);
EXPORT_SYMBOL(sdio_irq_func);
EXPORT_SYMBOL(hifSetInfo);

