

#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/earlysuspend.h>
#include <linux/mm.h>
#include <linux/slab.h>


#include <mach/irqs.h>
#include <asm/tcm.h>
#include <asm/io.h>
//#include <mach/sync_write.h>


#if defined(CONFIG_ARCH_MT6516)
	#include <mach/mt6516_typedefs.h>
	#include <mach/mt6516_boot.h>
    #include <mach/mt6516_reg_base.h>
    #include <mach/mt6516_pll.h>
	#include "tvc_drv.h"
	#include "tve_drv.h"
	#include "tvrot_drv.h"
#elif defined(CONFIG_ARCH_MT6573)
	#include <mach/mt6573_typedefs.h>
	#include <mach/mt6573_boot.h>
    #include <mach/mt6573_reg_base.h>
    #include <mach/mt6573_pll.h>
	#include <mach/mt6573_m4u.h>
	#include <linux/vmalloc.h>
	#include "tvc_drv.h"
	#include "tve_drv.h"
	#include "tvrot_drv.h"
#else
	#error "unknown arch"
#endif    



#include "tv_out.h"
#include "disp_drv.h"
#include "lcd_drv.h"
#include "lcm_drv.h"

#if defined(MTK_TVOUT_ENABLE_M4U)
M4U_EXPORT_FUNCTION_STRUCT _m4u_tvout_func = {0};
EXPORT_SYMBOL(_m4u_tvout_func);
#endif

#if defined (MTK_TVOUT_SUPPORT)


//--------------------------------------------Define-----------------------------------------------//


DECLARE_MUTEX(sem_tvout_screen_update);
#if 1
#define SCREEN_UPDATE_LOCK() \
    do { if (down_interruptible(&sem_tvout_screen_update)) { \
            printk("[TV] ERROR: Can't get sem_tvout_screen_update" \
                   " in %s()\n", __func__); \
            return TVOUT_STATUS_ERROR; \
        }} while (0)
#define SCREEN_UPDATE_RELEASE() \
    do { up(&sem_tvout_screen_update); } while (0)
#else
#define SCREEN_UPDATE_LOCK() \
    do { printk("[TV]%s: lock \n",__func__ ); \
        if (down_interruptible(&sem_tvout_screen_update)) { \
            printk("[TV] ERROR: Can't get sem_tvout_screen_update" \
                   " in %s()\n", __func__); \
            return TVOUT_STATUS_ERROR; \
         } \
	} while (0)
#define SCREEN_UPDATE_RELEASE() \
    do { up(&sem_tvout_screen_update); printk("[TV]%s: unlock\n", __func__); } while (0)
#endif

// This mutex is used to reslove wait check line inter time out issue when disable TV-out
// during video playback.
// When video playback, TVC will update registers each frame, 
// and wait check line inter, if we disable TV-out at this time,
// TVC will not generate any inter, so time out occurs.
// So, use this mutex to let 'disable' take effect after TVC update registers.
DECLARE_MUTEX(sem_video_update);
#define VIDEO_UPDATE_LOCK() \
    do { if (down_interruptible(&sem_video_update)) { \
            printk("[TV] ERROR: Can't get sem_video_update" \
                   " in %s()\n", __func__); \
            return TVOUT_STATUS_ERROR; \
        }} while (0)
#define VIDEO_UPDATE_RELEASE() \
    do { up(&sem_video_update); } while (0)


DECLARE_MUTEX(sem_tvout_status);
#define TVOUT_STATUS_LOCK() \
        do { if (down_interruptible(&sem_tvout_status)) { \
                printk("[TV] ERROR: Can't get sem_tvout_status" \
                       " in %s()\n", __func__); \
                return TVOUT_STATUS_ERROR; \
            }} while (0)
#define TVOUT_STATUS_UNLOCK() \
        do { up(&sem_tvout_status); } while (0)


#define TVOUT_DEBUG
#ifdef TVOUT_DEBUG
#define TVDBG printk
#else
#define TVDBG(x,...)
#endif

#define TVOUT_MSG
#ifdef TVOUT_MSG
#define TVMSG printk
#else
#define TVMSG(x,...)
#endif


#define TVOUT_LOG(fmt, arg...) \
    do { \
        if (tv_out_log_on) printk(fmt, ##arg); \
    }while (0)

#define TVOUT_USER_ENABLE_BUTTON
#define TVOUT_BUFFERS (2)

#if defined(MTK_TVOUT_ENABLE_M4U)
#define TVOUT_ENABLE_M4U
#else
#define TVOUT_USE_KMALLOC
#endif

#if defined(MTK_TVOUT_ENABLE_M4U)
	//M4U_EXPORT_FUNCTION_STRUCT _m4u_tvout_func = {0};
	//EXPORT_SYMBOL(_m4u_tvout_func);
	static bool is_m4u_tvc_port_closed = false;
#endif

#if defined(TVOUT_ENABLE_M4U)
	static unsigned char * tmpBufVAForM4U;
	static UINT32 mva_tvc=0;
	static UINT32 mva_tvr=0;
    static UINT32 mva_size=0;
#endif


typedef enum
{
    CONFIG_SRC_SIZE     = (1 << 0),
    CONFIG_SRC_FORMAT   = (1 << 1),
	CONFIG_SRC_RGBADDR  = (1 << 2),
	CONFIG_TV_TYPE      = (1 << 3),
	CONFIG_ALL		    =  0xF,
} TVC_CONFIG;


typedef struct
{
    UINT32 pa;
    UINT32 pitchInBytes;
} TempBuffer;

typedef struct
{
    BOOL         	isEnabled;
    TVOUT_MODE   	tvMode;
    TVOUT_SYSTEM 	tvSystem;
    TVOUT_ROT    	orientation;

    UINT32 			targetWidth, targetHeight;

    TempBuffer     	tmpBuffers[TVOUT_BUFFERS];
#if defined(TVOUT_ENABLE_M4U)
    TempBuffer	   	tmpBuffersForTVR[TVOUT_BUFFERS]; //TVR and TVC use the same buffer, but they
#endif												//have different MVA seperately.
    UINT32         	activeTmpBufferIdx;

    // Video playback mode parameters
    UINT32 			videoPa;
    TVOUT_SRC_FORMAT 	videoFormat;
    UINT32 			videoWidth, videoHeight;
} _TVOUT_CONTEXT;

static _TVOUT_CONTEXT tvout_context = {0};


//static bool is_tv_cable_plug_in = false;
static bool is_tv_enabled = false;
static bool is_tv_enabled_before_suspend = false;
static bool is_engine_in_suspend_mode = false;
static bool is_tv_out_turned_on    = false;
static bool is_tv_cable_plugged_in = false;
static bool is_tv_display_colorbar = false;
static bool is_tv_initialized_done = false;

static bool is_tv_out_closed = false;

static int is_disable_video_mode = 0;
static bool tv_out_log_on = false;

#define TVOUT_BUFFER_BPP (2)
#define TVOUT_BUFFER_NUM (2)
#define ALIGN_TO_POW_OF_2(x, n)  \
	(((x) + ((n) - 1)) & ~((n) - 1))
UINT32 tvout_get_vram_size(void)
{
	UINT32 vramsize;
	vramsize = DISP_GetScreenWidth() *
			   DISP_GetScreenHeight() *
			   TVOUT_BUFFER_BPP * TVOUT_BUFFER_NUM;
#if !defined(TVOUT_ENABLE_M4U) 
	vramsize = ALIGN_TO_POW_OF_2(vramsize, 0x100000);
#else
	vramsize += 1;
#endif	
	TVOUT_LOG("[TV]tvout_get_vram_size: %u bytes\n", vramsize);
	
	return vramsize;
}

// ---------------------------------------------------------------------------
//  TVOut Driver Private Functions
// ---------------------------------------------------------------------------

static unsigned int _user_v2p(unsigned int va)
{
    unsigned int pageOffset = (va & (PAGE_SIZE - 1));
    pgd_t *pgd;
    pmd_t *pmd;
    pte_t *pte;
    unsigned int pa;

    pgd = pgd_offset(current->mm, va); /* what is tsk->mm */
    pmd = pmd_offset(pgd, va);
    pte = pte_offset_map(pmd, va);
    
    pa = (pte_val(*pte) & (PAGE_MASK)) | pageOffset;

    return pa;
}


static __inline UINT32 get_active_tmp_buffer(void)
{
    return tvout_context.tmpBuffers[tvout_context.activeTmpBufferIdx].pa;
}

static __inline void increase_active_tmp_buffer_index(void)
{
    tvout_context.activeTmpBufferIdx = 
        (tvout_context.activeTmpBufferIdx + 1) % TVOUT_BUFFERS;
}


static TVOUT_STATUS Config_TVR(TVOUT_ROT rot, BOOL lockScreenUpdateMutex)
{
	TVDBG("[TV] rot(%d), %s\n",  rot, (lockScreenUpdateMutex?"Block":"Nonblock"));
    // Prevent from race condition with screen update process
    if (lockScreenUpdateMutex) {
        SCREEN_UPDATE_LOCK();
    }

    tvout_context.orientation = rot;

    if (!tvout_context.isEnabled || TVOUT_MODE_MIRROR != tvout_context.tvMode)
        goto End;
   
	{
        TVR_PARAM param;
        UINT32 i;

        // 1. Config LCD output to TVRot

        //LCD_CHECK_RET(LCD_SetOutputMode(LCD_OUTPUT_TO_LCM | LCD_OUTPUT_TO_TVROT));

        // 2. Config TVRot
        // Temp solution: Prevent error from TV-out rotating multi-times
		TVR_Stop();

        param.srcWidth = DISP_GetScreenWidth();
        param.srcHeight = DISP_GetScreenHeight();
        param.outputFormat = TVR_RGB565;
        param.rotation = (TVR_ROT)rot;
        param.flip = FALSE;

        // if activeTmpBufferIdx is not zero, shift the dstBufAddrs
#if defined (TVOUT_ENABLE_M4U)
        for (i = 0; i < TVR_BUFFERS; ++ i) {
            param.dstBufAddr[i] = tvout_context.tmpBuffersForTVR[
                (tvout_context.activeTmpBufferIdx + i) % TVOUT_BUFFERS].pa;
        }
#else
        for (i = 0; i < TVR_BUFFERS; ++ i) {
            param.dstBufAddr[i] = tvout_context.tmpBuffers[
                (tvout_context.activeTmpBufferIdx + i) % TVOUT_BUFFERS].pa;
        }
#endif

        TVR_Config(&param);
		TVR_Start();
    }

    // Update tvout target size
    
    switch(rot)
    {
    case TVOUT_ROT_90  : 
    case TVOUT_ROT_270 : 
        tvout_context.targetWidth = DISP_GetScreenHeight();
        tvout_context.targetHeight = DISP_GetScreenWidth();
        break;

    case TVOUT_ROT_0   : 
    case TVOUT_ROT_180 : 
    default :
        tvout_context.targetWidth = DISP_GetScreenWidth();
        tvout_context.targetHeight = DISP_GetScreenHeight();
        break;
    }

End:
    if (lockScreenUpdateMutex) {
        SCREEN_UPDATE_RELEASE();
    }

    return TVOUT_STATUS_OK;
}


static TVOUT_STATUS Config_TVC(TVC_CONFIG config)
{    
	if (config & CONFIG_SRC_SIZE) {
  	    TVC_CHECK_RET(TVC_SetSrcSize(tvout_context.targetWidth, tvout_context.targetHeight));
	}
	
	if (config & CONFIG_SRC_FORMAT) {
		TVC_CHECK_RET(TVC_SetSrcFormat(TVC_RGB565));
	}
	
	if (config & CONFIG_SRC_RGBADDR) {
	    TVC_CHECK_RET(TVC_SetSrcRGBAddr(get_active_tmp_buffer()));
	}
	
	if (config & CONFIG_TV_TYPE) {
	    TVC_CHECK_RET(TVC_SetTvType((TVC_TV_TYPE)tvout_context.tvSystem));
        TVE_CHECK_RET(TVE_SetTvType((TVE_TV_TYPE)tvout_context.tvSystem));
	}
        
    TVC_CHECK_RET(TVC_CommitChanges(TRUE));

	return TVOUT_STATUS_OK;
}

#if defined(TVOUT_ENABLE_M4U)
TVOUT_STATUS tvout_init_m4u(void)
{
	int i=0;
	//UINT32 size = 0;
	M4U_PORT_STRUCT M4uPort;
    UINT32 pa;
	//UINT32 tmpFbStartVA=0;

	TVOUT_LOG("[TV] %s\n", __func__);

	if (!_m4u_tvout_func.isInit)
	{
		TVMSG("[TV][Error] M4U has not init func for TV-out\n");
		return TVOUT_STATUS_ERROR;
	}

	mva_size = tvout_get_vram_size();
	tmpBufVAForM4U = (unsigned char*)vmalloc(mva_size);
	if (tmpBufVAForM4U == NULL) {
		TVMSG("[TV][Error] %s, vmalloc failed\n",__func__ );
		return TVOUT_STATUS_ERROR;
	}
	
	TVOUT_LOG("[TV]%s, %0x\n", __func__, (UINT32)tmpBufVAForM4U );
	memset(tmpBufVAForM4U, 0, mva_size);

	//Config TVROT&M4U	

	if (_m4u_tvout_func.m4u_alloc_mva(M4U_CLNTMOD_TVROT, (unsigned int)tmpBufVAForM4U, mva_size, &mva_tvr) != 0)
    {
        return TVOUT_STATUS_ERROR;
    } 
	TVOUT_LOG("[M4U] return TVR MVA is: 0x%x\n", mva_tvr);
	_m4u_tvout_func.m4u_insert_tlb_range(M4U_CLNTMOD_TVROT, 
                                  		 (unsigned int)mva_tvr, 
                      					 (unsigned int)(mva_tvr+mva_size-1), 
                      					 SEQ_RANGE_LOW_PRIORITY);

    pa = mva_tvr;
	for ( i=0; i < TVOUT_BUFFERS; i++)
	{
		TempBuffer* b = &tvout_context.tmpBuffersForTVR[i];

		b->pa = pa;
		b->pitchInBytes = DISP_GetScreenWidth() * TVOUT_BUFFER_BPP;
		//ASSERT((tmpFbStartPA & 0x7) == 0);
		pa += DISP_GetScreenWidth() * DISP_GetScreenHeight() * TVOUT_BUFFER_BPP;
		TVOUT_LOG("[TV] TVR buffer addr: (%d): 0x%x, Pitch:0x%x\n",
			   	i, b->pa, b->pitchInBytes);
	}

	M4uPort.ePortID = M4U_PORT_TV_ROT_OUT0;
	M4uPort.Virtuality = 1;
	M4uPort.Security = 0;
	M4uPort.Distance = 1;
	M4uPort.Direction = 0;
	_m4u_tvout_func.m4u_config_port(&M4uPort);
	//_m4u_tvout_func.m4u_dump_reg();

    //Config TVC&M4U
	if (_m4u_tvout_func.m4u_alloc_mva(M4U_CLNTMOD_TVC, (unsigned int)tmpBufVAForM4U, mva_size, &mva_tvc) !=0 )
    {
        return TVOUT_STATUS_ERROR;
    } 
	TVOUT_LOG("[M4U] return TVC MVA is: 0x%x\n", mva_tvc);
	_m4u_tvout_func.m4u_insert_tlb_range(M4U_CLNTMOD_TVC, 
                                  		 (unsigned int)mva_tvc, 
                      					 (unsigned int)(mva_tvc+mva_size-1), 
                      					 SEQ_RANGE_LOW_PRIORITY);

    pa = mva_tvc;
	for ( i=0; i < TVOUT_BUFFERS; i++)
	{
		TempBuffer* b = &tvout_context.tmpBuffers[i];
		b->pa = pa;
		b->pitchInBytes = DISP_GetScreenWidth() * TVOUT_BUFFER_BPP;
		//ASSERT((tmpFbStartPA & 0x7) == 0);
		pa += DISP_GetScreenWidth() * DISP_GetScreenHeight() * TVOUT_BUFFER_BPP;
		TVOUT_LOG("[TV]TVC buffer addr(%d): 0x%x, Pitch:0x%x\n",
			   	i, b->pa, b->pitchInBytes);
	}

    M4uPort.ePortID = M4U_PORT_TVC;
	M4uPort.Virtuality = 1;
	M4uPort.Security = 0;
	M4uPort.Distance = 1;
	M4uPort.Direction = 0;
	_m4u_tvout_func.m4u_config_port(&M4uPort);
	//_m4u_tvout_func.m4u_dump_reg();

	return TVOUT_STATUS_OK;
}

void tvout_deinit_m4u(void)
{
	M4U_PORT_STRUCT M4uPort;
	
	if (!_m4u_tvout_func.isInit)
	{
		printk("[TV]Error, M4U has not init func for TV-out\n");
		return;
	}

	TVOUT_LOG("[TV]: Disable M4U for TVC\n");
    M4uPort.ePortID = M4U_PORT_TV_ROT_OUT0;
	M4uPort.Virtuality = 0;
	M4uPort.Security = 0;
	M4uPort.Distance = 1;
	M4uPort.Direction = 0;
	_m4u_tvout_func.m4u_config_port(&M4uPort);
    
	TVOUT_LOG("[TV]: Disable M4U for TVR\n");
	M4uPort.ePortID = M4U_PORT_TVC;
	M4uPort.Virtuality = 0;
	M4uPort.Security = 0;
	M4uPort.Distance = 1;
	M4uPort.Direction = 0;
	_m4u_tvout_func.m4u_config_port(&M4uPort);

    _m4u_tvout_func.m4u_invalid_tlb_range(M4U_CLNTMOD_TVROT, 
                                  		 (unsigned int)mva_tvr, 
                      					 (unsigned int)(mva_tvr+mva_size-1));
    _m4u_tvout_func.m4u_invalid_tlb_range(M4U_CLNTMOD_TVC, 
                                  		 (unsigned int)mva_tvc, 
                      					 (unsigned int)(mva_tvc+mva_size-1));
    
    _m4u_tvout_func.m4u_dealloc_mva(M4U_CLNTMOD_TVROT, (UINT32)tmpBufVAForM4U, tvout_get_vram_size(), mva_tvr);	
    _m4u_tvout_func.m4u_dealloc_mva(M4U_CLNTMOD_TVC, (UINT32)tmpBufVAForM4U, tvout_get_vram_size(), mva_tvc);

	if (tmpBufVAForM4U) {vfree(tmpBufVAForM4U);}	
}
#endif

static __inline TVOUT_STATUS tvout_enterMirrorMode(void)
{
	LCD_OUTPUT_MODE outputMode;

	TVDBG("[TV]%s\n", __func__);

	tvout_context.activeTmpBufferIdx = 0;

	// 1. Set LCD output mode
	
	SCREEN_UPDATE_LOCK();	
	outputMode = LCD_GetOutputMode();
    //LCD_CHECK_RET(LCD_WaitForNotBusy());
	LCD_CHECK_RET(LCD_SetOutputMode(outputMode | LCD_OUTPUT_TO_TVROT));
	TVMSG("[TV] Set LCD output mode : (0x%x) -> (0x%x)\n", outputMode, LCD_GetOutputMode());

	// 2. Config TV Rotator

    Config_TVR(tvout_context.orientation, FALSE);
	SCREEN_UPDATE_RELEASE();	
 
 	// 3. Config TV controller

	//Config_TVC(CONFIG_ALL);
	TVC_CHECK_RET(TVC_SetSrcFormat(TVC_RGB565));
    TVC_CHECK_RET(TVC_SetSrcRGBAddr(get_active_tmp_buffer()));

	return TVOUT_STATUS_OK;	

}

static TVOUT_STATUS tvout_leaveMirrorMode(void)
{
	TVDBG("[TV]%s\n", __func__);
	return TVOUT_STATUS_OK;
}


static __inline TVOUT_STATUS tvout_enterVideoPlaybackMode(void)
{
	//TVDBG("[TV]%s\n", __func__);

#if defined(TVOUT_ENABLE_M4U)
	// Turn off TVC M4U port
	if (_m4u_tvout_func.isInit && !is_m4u_tvc_port_closed)
	{
		M4U_PORT_STRUCT M4uPort;
		
        TVC_Disable();
    	//TVC_CHECK_RET(TVC_CommitChanges(TRUE));

    	M4uPort.ePortID = M4U_PORT_TVC;
		M4uPort.Virtuality = 0;
		//M4uPort.Security = 0;
		M4uPort.Distance = 1;
		//M4uPort.Direction = 0;
		_m4u_tvout_func.m4u_config_port(&M4uPort);
		is_m4u_tvc_port_closed = true;
		TVMSG("[TV] Enter Video mode, turn off m4u tvc port\n");
		TVC_Enable();
	}

#endif

	if (TVOUT_FMT_YUV420_SEQ == tvout_context.videoFormat ||
        TVOUT_FMT_YUV420_BLK == tvout_context.videoFormat) 
	{
		UINT32 Y = tvout_context.videoPa;
    	UINT32 U = Y + tvout_context.videoWidth * tvout_context.videoHeight;
    	UINT32 V = U + tvout_context.videoWidth * tvout_context.videoHeight / 4;

    	TVOUT_LOG("[TV], Y: %x, U: %x, V: %x\n", Y, U, V);
    
    	TVC_SetSrcYUVAddr(Y, U, V);
    	TVC_SetSrcFormat((TVC_SRC_FORMAT)tvout_context.videoFormat);
    	TVC_SetSrcSize(tvout_context.videoWidth, tvout_context.videoHeight);
    } 
	else if (TVOUT_FMT_RGB565 == tvout_context.videoFormat ||
             TVOUT_FMT_UYVY422 == tvout_context.videoFormat)
	{
    	TVOUT_LOG("[TV], RGB addr: %x\n",tvout_context.videoPa);
        TVC_SetSrcRGBAddr(tvout_context.videoPa);
    	TVC_SetSrcFormat((TVC_SRC_FORMAT)tvout_context.videoFormat);
    	TVC_SetSrcSize(tvout_context.videoWidth, tvout_context.videoHeight);
	}

	VIDEO_UPDATE_LOCK();
    TVC_CHECK_RET(TVC_CommitChanges(TRUE));
	VIDEO_UPDATE_RELEASE();

	return TVOUT_STATUS_OK;
}

static __inline void tvout_leaveVideoPlaybackMode(void)
{
	TVMSG("[TV]%s\n", __func__);

#if defined(TVOUT_ENABLE_M4U)
	// Turn on TVC M4U port
	if (_m4u_tvout_func.isInit && is_m4u_tvc_port_closed)
	{
		M4U_PORT_STRUCT M4uPort;
    	M4uPort.ePortID = M4U_PORT_TVC;
		M4uPort.Virtuality = 1;
		M4uPort.Security = 0;
		M4uPort.Distance = 1;
		M4uPort.Direction = 0;
		_m4u_tvout_func.m4u_config_port(&M4uPort);
		is_m4u_tvc_port_closed = false;
		TVMSG("[TV] Leave Video mode, turn on m4u tvc port\n");
	}
#endif
}


static void tvout_enable_power(BOOL enable)
{
	TVMSG("[TV]TVout Power (%s)\n",(enable?"on":"off"));
    if (enable) {
        TVC_CHECK_RET(TVC_PowerOn());
        TVE_CHECK_RET(TVE_PowerOn());
        TVE_CHECK_RET(TVE_ResetDefaultSettings());
        TVR_CHECK_RET(TVR_PowerOn());
    } else {
        //LCD_CHECK_RET(LCD_WaitForNotBusy());
        TVR_CHECK_RET(TVR_PowerOff());
        TVE_CHECK_RET(TVE_PowerOff());
        TVC_CHECK_RET(TVC_PowerOff());
    }
}

TVOUT_STATUS tvout_enable_mode(BOOL enable)
{
	TVMSG("[TV]TvOut mode (%s)\n",(enable?"Enable":"Disable"));

    if (tvout_context.isEnabled == enable) 
        return TVOUT_STATUS_ALREADY_SET;

    tvout_context.isEnabled = enable;

    if (enable)
    {

#if defined (TVOUT_ENABLE_M4U)
		if (tvout_init_m4u() != TVOUT_STATUS_OK)
		{
			TVMSG("[TV][Error]: M4U is invalid (%s)@(%d)\n", __func__, __LINE__);
			return TVOUT_STATUS_ERROR;
		}
#endif
#if 0
		if (is_tv_display_colorbar) {
			TVOUT_EnableColorBar(false);
		}
#endif
        if (TVOUT_MODE_MIRROR == tvout_context.tvMode) {
            tvout_enterMirrorMode();
        } else {
            tvout_enterVideoPlaybackMode();
        }


        TVC_CHECK_RET(TVC_SetTvType(tvout_context.tvSystem));
        TVE_CHECK_RET(TVE_SetTvType(tvout_context.tvSystem));

        TVE_CHECK_RET(TVE_Enable());
        TVC_CHECK_RET(TVC_Enable());
        TVC_CHECK_RET(TVC_CommitChanges(TRUE));

        is_tv_initialized_done = true;

        DISP_UpdateScreen(0, 0, DISP_GetScreenWidth(), DISP_GetScreenHeight());
    }
    else
    {
#if 1
		LCD_OUTPUT_MODE outputMode = LCD_GetOutputMode();
		if (outputMode & LCD_OUTPUT_TO_TVROT) {
			SCREEN_UPDATE_LOCK();	
    		//LCD_CHECK_RET(LCD_WaitForNotBusy());
    		LCD_CHECK_RET(LCD_SetOutputMode(outputMode & ~LCD_OUTPUT_TO_TVROT));
			TVMSG("[TV] Set LCD output mode : (0x%x) -> (0x%x)\n", outputMode, LCD_GetOutputMode());
			SCREEN_UPDATE_RELEASE();
		}
#endif

		TVR_Stop();

		VIDEO_UPDATE_LOCK();
        TVC_CHECK_RET(TVC_Disable());
		VIDEO_UPDATE_RELEASE();

        //TVC_CHECK_RET(TVC_CommitChanges(TRUE));
        
		TVE_CHECK_RET(TVE_Disable());


        if (TVOUT_MODE_MIRROR == tvout_context.tvMode) {
            tvout_leaveMirrorMode();
        } else {
            tvout_leaveVideoPlaybackMode();
        }


#if defined (TVOUT_ENABLE_M4U)
		tvout_deinit_m4u();
#endif
		is_tv_initialized_done = false;
    }

	return TVOUT_STATUS_OK;
}

static TVOUT_STATUS tvout_enable(BOOL enable)
{
    TVOUT_STATUS r = TVOUT_STATUS_OK;

	TVMSG("[TV] TvOut: (%s)\n", (enable?"Enable":"Disable"));

    if (is_tv_enabled == enable) {
		TVMSG("[TV][Error] TvOut already set \n");
        return TVOUT_STATUS_ALREADY_SET;
	}


    if (enable) tvout_enable_power(TRUE);
    r = tvout_enable_mode(enable);
    if (!enable) tvout_enable_power(FALSE);
	
	is_tv_enabled = enable;

    return r;
}

// ---------------------------------------------------------------------------
//  TVOut Driver Public Functions
// ---------------------------------------------------------------------------
TVOUT_STATUS TVOUT_Capture_Tvrotbuffer(unsigned int pvbuf, unsigned int bpp)
{
    unsigned int i = 0;
    unsigned char *fbv;
    unsigned int fbsize = 0;
    unsigned int fb_bpp = 16;
    unsigned int w,h;

    printk("%s\n", __func__);

    // TV-out is not enabled, return

    if ( !tvout_context.isEnabled )
    {
        printk("TVOUT_Capture_Tvrotbuffer, TVout is not enabled\n");
        return TVOUT_STATUS_OK; 
    }


    // M4u is not enabled, not implement
#if !defined TVOUT_ENABLE_M4U
    printk("TVOUT_Capture_Tvrotbuffer, M4U is not enabled in TVout\n");
    return TVOUT_STATUS_OK;
#endif


    if(pvbuf == 0 || bpp == 0)
    {
        printk("TVOUT_Capture_Tvrotbuffer, ERROR, parameters wrong: pvbuf=0x%08x, bpp=%d\n", pvbuf, bpp);
        return TVOUT_STATUS_OK;
    }


    w = DISP_GetScreenWidth();
    h = DISP_GetScreenHeight();
    fbsize = w*h*fb_bpp/8;

    //Get active buffer

    fbv = tmpBufVAForM4U + tvout_context.activeTmpBufferIdx * fbsize;

    if (!fbv)
    {
        printk("TVOUT_Capture_Tvrotbuffer, fbv is NULL\n");
        return TVOUT_STATUS_ERROR;
    }
 

    if(bpp == 32 && fb_bpp == 16)
    {
        unsigned int t;
	    unsigned short* fbvt = (unsigned short*)fbv;
        for(i = 0;i < w*h; i++)
    	{
	        t = fbvt[i];
            *(unsigned int*)(pvbuf+i*4) = 0xff000000|((t&0x001F)<<3)|((t&0x07E0)<<5)|((t&0xF800)<<8);
    	}
    }
    else if(bpp == 16 && fb_bpp == 16)
    {
    	memcpy((void*)pvbuf, (void*)fbv, fbsize);
    }
    else
    {
    	printk("TVOUT_Capture_Tvrotbuffer, bpp:%d & fb_bpp:%d is not supported now\n", bpp, fb_bpp);
    }
    
    return TVOUT_STATUS_OK;    
}

void TVOUT_EnableLog(bool enable)
{
    tv_out_log_on = enable;
    TVMSG("[TV] debug log is %s\n", (enable?"ON":"OFF"));
}

bool TVOUT_IsCablePlugIn(void)
{
    return is_tv_cable_plugged_in;
}

bool TVOUT_IsTvoutEnabled(void)
{
	return tvout_context.isEnabled;
}

bool TVOUT_IsUserEnabled(void)
{
    return is_tv_out_turned_on;
}
EXPORT_SYMBOL(TVOUT_IsUserEnabled);

void TVOUT_TurnOn(bool en)
{
    TVMSG("[TV]%s() %d\n", __func__, en);
    is_tv_out_turned_on = en;
}

TVOUT_STATUS TVOUT_PowerEnable(bool enable)
{
	TVMSG("[TV] TvOut Power: (%s)\n", (enable?"Resume":"Suspend"));
#if 1
    TVOUT_STATUS_LOCK();

	if (!enable && (is_tv_enabled || is_tv_display_colorbar)) {
		is_tv_enabled_before_suspend = TRUE;
        //if (is_tv_out_turned_on) {
        if (is_tv_out_turned_on && !is_tv_display_colorbar) {
            tvout_enable(FALSE);
        }else{
            TVOUT_EnableColorBar(false);
        }
	}

	is_engine_in_suspend_mode = enable ? false : true;

	if (enable && is_tv_enabled_before_suspend) {
		is_tv_enabled_before_suspend = FALSE;
		//if (is_tv_out_turned_on) {
        if (is_tv_out_turned_on && !is_tv_out_closed) {
            tvout_enable(TRUE);
        }else{
            TVOUT_EnableColorBar(true);
        }
	}

    TVOUT_STATUS_UNLOCK();
#else
	if (!enable && is_tv_enabled) {
		is_tv_enabled_before_suspend = TRUE;
		tvout_enable(FALSE);
	}

	is_engine_in_suspend_mode = enable ? false : true;
	
	if (enable && is_tv_enabled_before_suspend) {
		is_tv_enabled_before_suspend = FALSE;
		tvout_enable(TRUE);
	}
#endif

	return TVOUT_STATUS_OK;
}

TVOUT_STATUS TVOUT_EnableColorBar(bool enable)
{
	TVMSG("[TV] ColorBar: (%s) pluged_in: (%d)\n", (enable?"Enable":"Disable"), 
			is_tv_cable_plugged_in);

    if (is_tv_display_colorbar == enable) {
		TVMSG("[TV][Error] TvOut already set \n");
        return TVOUT_STATUS_ALREADY_SET;
	}
	is_tv_display_colorbar = enable;


    if (enable) {
        tvout_enable_power(TRUE);
        TVE_CHECK_RET(TVE_Enable());
        TVE_CHECK_RET(TVE_EnableColorBar(TRUE));
    } else {
        TVE_CHECK_RET(TVE_EnableColorBar(FALSE));
        TVE_CHECK_RET(TVE_Disable());
        tvout_enable_power(FALSE);
    }

    is_tv_display_colorbar = enable;

	return TVOUT_STATUS_OK;
}

TVOUT_STATUS TVOUT_SetOrientation(TVOUT_ROT rot)
{

 	LCD_OUTPUT_MODE outputMode;
    printk("%s(%d)\n", __func__, rot);

    if (tvout_context.isEnabled && TVOUT_MODE_MIRROR == tvout_context.tvMode)
    {
        //SCREEN_UPDATE_LOCK();
        //Disable LCD output mode to TVROT
	    outputMode = LCD_GetOutputMode();
	    LCD_CHECK_RET(LCD_SetOutputMode(outputMode & ~LCD_OUTPUT_TO_TVROT));
        TVMSG("[TV] Set LCD output mode : (0x%x) -> (0x%x)\n", outputMode, LCD_GetOutputMode());
    
        Config_TVR(rot, FALSE);

        //Enable LCD output mode to TVROT
	    outputMode = LCD_GetOutputMode();
	    LCD_CHECK_RET(LCD_SetOutputMode(outputMode | LCD_OUTPUT_TO_TVROT));
        TVMSG("[TV] Set LCD output mode : (0x%x) -> (0x%x)\n", outputMode, LCD_GetOutputMode());
        //SCREEN_UPDATE_RELEASE();
    }
    else
    {
        Config_TVR(rot, FALSE);
    }

	return TVOUT_STATUS_OK;
}
extern void switch_NTSC_to_PAL(int mode);
TVOUT_STATUS TVOUT_SetTvSystem(TVOUT_SYSTEM tvSystem)
{
	TVMSG("[TV]System(%d)\n", tvSystem);

    tvout_context.tvSystem = tvSystem;
	switch_NTSC_to_PAL(tvSystem);

    if (tvout_context.isEnabled) {
		return Config_TVC(CONFIG_TV_TYPE);	
    }

    return TVOUT_STATUS_OK;
}

TVOUT_STATUS TVOUT_DisableVideoMode(bool disable)
{
    printk("[TV]%s video mode (%d)\n", (disable?"DISABLED":"ENABLED"), is_disable_video_mode);
    if (disable) is_disable_video_mode++;
    else is_disable_video_mode--;
    if (is_disable_video_mode < 0)
    {
        printk("[TV][Warning] disable video mode\n");
        is_disable_video_mode = 0;
    }
    return TVOUT_STATUS_OK;
}


extern int is_pmem_range(unsigned long* base, unsigned long size);
TVOUT_STATUS TVOUT_PostVideoBuffer(UINT32 va, TVOUT_SRC_FORMAT format,
		UINT32 width, UINT32 height)
{

    unsigned int pa;
    unsigned int bufSize;

    //printk("%s(), VA: %x, format: %x, width: %d, height: %d\n", __func__,
    //    va, format, width, height);
    if (is_disable_video_mode) {
        TVOUT_LOG("[TV]post video buffer using image mode\n");
        return TVOUT_STATUS_OK;
    }
#if defined(CONFIG_ARCH_MT6573)
    //YUV 420 Scan line is not supported in 6573 TVC
    if (format == TVOUT_FMT_YUV420_SEQ)
    {
        TVOUT_LOG("[TV]Unsupported format using image mode\n");
        return TVOUT_STATUS_OK;
    }
#endif
        
	pa = _user_v2p(va);
	switch (format)
	{	
		case TVOUT_FMT_RGB565:
        case TVOUT_FMT_UYVY422:
			bufSize = width * height * 2;
			break;
		case TVOUT_FMT_YUV420_SEQ:
		case TVOUT_FMT_YUV420_BLK:
			bufSize = width * height * 3 / 2;
			break;
		default:
			TVMSG("[TV][Warning]%s unsupport format\n", __func__);
			return TVOUT_STATUS_OK;
	}	

    if(!is_pmem_range((unsigned long*)pa, bufSize))
    {
		TVMSG("[TV][Error]%s video buffer addr is not in PMEM, PA:%x, VA:%x\n", __func__, pa, va);
		return TVOUT_STATUS_ERROR;
    }    


    //SCREEN_UPDATE_LOCK();

    tvout_context.tvMode = TVOUT_MODE_VIDEO;
    tvout_context.videoPa = pa;
    tvout_context.videoFormat = format;
    tvout_context.videoWidth = width;
    tvout_context.videoHeight = height;

    // Force align video width to 16-pixels if format is YUV420
    if (TVOUT_FMT_YUV420_SEQ == format ||
        TVOUT_FMT_YUV420_BLK == format) {
        tvout_context.videoWidth = (tvout_context.videoWidth + 15) & 0xFFFFFFF0;
    }

    if (!tvout_context.isEnabled || !is_tv_initialized_done) goto End;

    tvout_enterVideoPlaybackMode();

    //TVC_CHECK_RET(TVC_CommitChanges(FALSE));
    
End:
    //SCREEN_UPDATE_RELEASE();

	return TVOUT_STATUS_OK;
}


TVOUT_STATUS TVOUT_LeaveVideoBuffer(void)
{


	TVMSG("%s()\n", __func__);


#if defined(TVOUT_ENABLE_M4U)
	if (tvout_context.isEnabled && tvout_context.tvMode == TVOUT_MODE_VIDEO ) 
    {
		TVC_Disable();
    	//TVC_CHECK_RET(TVC_CommitChanges(TRUE));
		tvout_leaveVideoPlaybackMode();
		tvout_context.tvMode = TVOUT_MODE_MIRROR;
		tvout_enterMirrorMode();
		//TVC_CHECK_RET(TVC_SetSrcRGBAddr(get_active_tmp_buffer()));
		TVC_CHECK_RET(TVC_CommitChanges(TRUE));
		TVC_Enable();
        DISP_UpdateScreen(0, 0, DISP_GetScreenWidth(), DISP_GetScreenHeight());
	}
    // special case for video live wall paper 
    else if (tvout_context.isEnabled && tvout_context.tvMode == TVOUT_MODE_MIRROR)
    {
        TVOUT_LOG("[TV]leave video buffer in image mode\n");
	}
    else // TV-out is disabled
    {
        tvout_context.tvMode = TVOUT_MODE_MIRROR;
	    Config_TVR(tvout_context.orientation, FALSE);
		TVC_CHECK_RET(TVC_SetSrcFormat(TVC_RGB565));
    }
#else
		tvout_context.tvMode = TVOUT_MODE_MIRROR;
	    Config_TVR(tvout_context.orientation, FALSE);
		TVC_CHECK_RET(TVC_SetSrcFormat(TVC_RGB565));
#endif	
    
	return TVOUT_STATUS_OK;
}

extern void accdet_detect(void);
TVOUT_STATUS TVOUT_TvTurnOn(bool on)
{

    TVMSG("%s() %d\n", __func__, on);

    if (is_tv_out_turned_on == on)
    {
        TVMSG("[TV][Warning] already set\n");
        return TVOUT_STATUS_ALREADY_SET;
    }

    is_tv_out_turned_on = on;
    if (is_tv_out_turned_on && !is_tv_cable_plugged_in)
    {
        TVMSG("[TV]%s(): Turn On, Let Accdet detect cable\n", __func__);
        accdet_detect();
    }
    else if (is_tv_out_turned_on && is_tv_display_colorbar)
    {
    		TVOUT_EnableColorBar(false);
    		tvout_enable(true);
    }
    else if (!is_tv_out_turned_on && is_tv_cable_plugged_in)
    {
        TVMSG("[TV]%s(): Turn Off, Let Accdet detect cable\n", __func__);
		tvout_enable(false);
		TVOUT_EnableColorBar(true);

    }
    else
    {
        TVMSG("[TV]%s(): Unkown status\n", __func__);
        TVMSG("[TV]%s(): status: %d %d %d %d %d %d %d %d %d\n", __func__,
            is_tv_enabled,
            is_tv_enabled_before_suspend,
            is_engine_in_suspend_mode,
            is_tv_out_turned_on,
            is_tv_cable_plugged_in,
            is_tv_display_colorbar,
            is_tv_initialized_done,
            is_tv_out_closed,
            is_disable_video_mode);
    }
    return TVOUT_STATUS_OK;

}


TVOUT_STATUS TVOUT_TvCablePlugIn(void)
{

    TVMSG("%s()\n", __func__);

    if (FACTORY_BOOT == get_boot_mode()) {
        is_tv_cable_plugged_in = TRUE;
        return TVOUT_EnableColorBar(TRUE);
    }
   
	if (is_tv_cable_plugged_in) { 
		TVMSG("[TV][Warning] TV cable has pluged in.\n");
		return TVOUT_STATUS_ALREADY_SET;
	}

	is_tv_cable_plugged_in = TRUE;

	if (is_tv_out_turned_on) {
		if (is_engine_in_suspend_mode) {
			is_tv_enabled_before_suspend = TRUE;
		} else {
		    if (is_tv_out_closed) {
                TVMSG("[TV]%s(): tv out is closed, enable color bar\n", __func__);
                TVOUT_EnableColorBar(true);
            } else {
			    if (tvout_enable(TRUE) != TVOUT_STATUS_OK) {
				    TVMSG("[TV][Error] TV-out enable failed.\n");
				    return TVOUT_STATUS_ERROR;
			    }// tvout_enable
            }//is_tv_out_closed
		}//is_engine_in_suspend_mode
	}

	return TVOUT_STATUS_OK;
}

TVOUT_STATUS TVOUT_TvCablePlugOut(void)
{
	TVMSG("[TV]%s()\n", __func__);
    if (FACTORY_BOOT == get_boot_mode()) {
        is_tv_cable_plugged_in = FALSE;
        return TVOUT_EnableColorBar(FALSE);
    }

	if (!is_tv_cable_plugged_in)
		return TVOUT_STATUS_ALREADY_SET;

	is_tv_cable_plugged_in = FALSE;

	if (is_tv_out_turned_on)
	{
			if (is_engine_in_suspend_mode) {
				is_tv_enabled_before_suspend = FALSE;
			} else {
	    if (is_tv_out_closed) {
            TVMSG("[TV]%s(): tv out is closed, disable color bar\n", __func__);
            TVOUT_EnableColorBar(false);
        } else {
		    tvout_enable(FALSE);
        }
			}
	}
	else
	{
			TVOUT_EnableColorBar(false);
	}

	return TVOUT_STATUS_OK;

}



TVOUT_STATUS TVOUT_ScreenUpdateLock(void)
{
	// Release semaphore in dbi_tvout_on_lcd_done()
    SCREEN_UPDATE_LOCK();
	return TVOUT_STATUS_OK;
}

TVOUT_STATUS TVOUT_On_LCD_Done(void)
{
    // Software control LCD/TVC frame buffer addresses

    if (tvout_context.isEnabled && TVOUT_MODE_MIRROR == tvout_context.tvMode)
    {
#if defined (MTK_TVOUT_ENABLE_M4U)
	/*	
		static int isStart = 0;
		int ret = ((isStart)?_m4u_tvout_func.m4u_monitor_stop():_m4u_tvout_func.m4u_monitor_start());
		isStart++;
		isStart &= 0x1;
	*/	
#endif
        TVC_CHECK_RET(TVC_SetSrcSize(tvout_context.targetWidth, tvout_context.targetHeight));
		TVC_CHECK_RET(TVC_SetSrcRGBAddr(get_active_tmp_buffer()));
        TVC_CHECK_RET(TVC_CommitChanges(TRUE));

		//Add by GMC, when capture video buffer, INTR will increase index, but LCDC did not trans to TVR
		if (LCD_GetOutputMode() & LCD_OUTPUT_TO_TVROT)
		{
            increase_active_tmp_buffer_index();
		}
    }

    SCREEN_UPDATE_RELEASE();

    return TVOUT_STATUS_OK;
}

void TVOUT_ForceClose(void)
{
    TVMSG("[TV]%s() cable: %s \n",
        __func__, (is_tv_cable_plugged_in?"plug-in":"plug-out"));

    if (is_tv_out_closed)
    {
        TVMSG("[TV][warning]%s(): tv out has been closed, but close again\n", __func__);
        return;
    }
    if (is_engine_in_suspend_mode)
    {
        TVMSG("[TV][warning]force close after suspend\n");
        return;
    }
    
    TVOUT_STATUS_LOCK();
    if (is_tv_cable_plugged_in && is_tv_out_turned_on)
    {
        TVMSG("[TV]%s() cable plug-in, close tv out, enable color bar \n", __func__);
        // turn off TV-out
        tvout_enable(false);
        // enable colorbar
        TVOUT_EnableColorBar(true);
    }


    is_tv_out_closed = true;
    TVOUT_STATUS_UNLOCK();

}

void TVOUT_RestoreOpen(void)
{
    TVMSG("[TV]%s() cable: %s \n",
        __func__, (is_tv_cable_plugged_in?"plug-in":"plug-out"));


    if (!is_tv_out_closed)
    {
        TVMSG("[TV][warning]%s(): tv out has been opened, but open again\n", __func__);
        return;
    }

    if (is_engine_in_suspend_mode)
    {
        TVMSG("[TV][warning]restore open after suspend\n");
        return;
    }
    
    TVOUT_STATUS_LOCK();


    if (is_tv_cable_plugged_in && is_tv_out_turned_on)
    {
        TVMSG("[TV]%s() cable plug-in, open tv out, enable tv out \n", __func__);
        TVOUT_EnableColorBar(false);
        tvout_enable(true);
    }

	TVOUT_STATUS_UNLOCK();

    is_tv_out_closed = false;

}


/*****************************************************************************/

//file operations

static int Tvout_Drv_Ioctl(struct inode * a_pstInode,
						 struct file * a_pstFile,
						 unsigned int a_u4Command,
						 unsigned long a_u4Param)
{
    int ret = 0;
    void __user *argp = (void __user *)a_u4Param;

	switch (a_u4Command)
	{
        case TVOUT_IS_TV_CABLE_PLUG_IN:
        {
            BOOL isTvCableEnabled = TVOUT_IsCablePlugIn();
            return copy_to_user(argp, &isTvCableEnabled,
                            sizeof(isTvCableEnabled)) ? -EFAULT : 0;
        }
    
		case TVOUT_TURN_ON:
    	{
        	printk("[TV] ioctl: set TV Enable: %lu\n", a_u4Param);
            TVOUT_TvTurnOn(a_u4Param);
        	return 0;
    	}

    	case TVOUT_SET_TV_SYSTEM:
    	{
        	printk("[TV] ioctl: set TV System: %lu\n", a_u4Param);
        	TVOUT_SetTvSystem((TVOUT_SYSTEM)a_u4Param);
        	return 0;
    	}

    	case TVOUT_ISSHOW_TVOUTBUTTON:
    	{
        	bool isShow = true;
#if defined TVOUT_USER_ENABLE_BUTTON
            isShow = true;
#else
        	isShow = false;
#endif

        	printk("[TV] ioctl: show tvout button(%d)\n", isShow);
        	return isShow;
    	}

		case TVOUT_CTL_SWITCH_TO_HQA_MODE:
		{
		    printk("[TV] ioctl: swith to HQA mode(%d)\n", (UINT32)a_u4Param);
        	
			if (a_u4Param & 0x1) 
				tvout_context.tvMode = TVOUT_MODE_VIDEO;
			else
				tvout_context.tvMode = TVOUT_MODE_VIDEO;
       	 	return 0;
		}

	    case TVOUT_CTL_POST_HQA_BUFFER:
    	{
        	TVOUT_HQA_BUF b;
        	printk("[TV] ioctl: post HQA buffer\n");

       		if (copy_from_user(&b, (void __user *)a_u4Param, sizeof(b)))
        	{
            	printk("[TV]: copy_from_user failed! line:%d \n", __LINE__);
            	ret = -EFAULT;
        	} else {

				//TVC_SetTarSizeForHQA(TRUE);
            	TVOUT_PostVideoBuffer((UINT32)b.vir_addr, 
                                      (TVOUT_SRC_FORMAT)b.format,
                                      b.width, b.height);
			}
        	return ret;
    	}

    	case TVOUT_CTL_LEAVE_HQA_MODE:
    	{
        	printk("[TV] ioctl: leave HQA mode\n");

			//tvout_dumpreg();
			TVOUT_LeaveVideoBuffer();
			//TVC_SetTarSizeForHQA(FALSE);

			return 0;
    	}

    	case TVOUT_FORCE_CLOSE:
    	{
        	printk("[TV] ioctl: force close tv-out\n");
            TVOUT_ForceClose();
    		return 0;
    	}

        case TVOUT_RESTORE_OPEN:
    	{
        	printk("[TV] ioctl: restore open tv-out\n");
            TVOUT_RestoreOpen();
			return 0;
    	}

        case TVOUT_DISABLE_VIDEO_MODE:
        {
            printk("[TV] ioctl: video mode is %s\n", (a_u4Param?"DISABLED":"ENABLED"));
            TVOUT_DisableVideoMode(a_u4Param); 
        }
	}

	return ret;
}


static const struct file_operations g_stMTKTVout_fops =
{
	.owner = THIS_MODULE,
	.ioctl = Tvout_Drv_Ioctl
};

static struct cdev * g_pMTKTVout_CharDrv = NULL;
static dev_t g_MTKTVoutdevno = MKDEV(MTK_TVOUT_MAJOR_NUMBER,0);
static inline int Tvout_Drv_Register(void)
{
    printk("[TVOUT] %s\n",__func__);
    if( alloc_chrdev_region(&g_MTKTVoutdevno, 0, 1,"TV-out") )
    {
        TVDBG("[TVOUT] Allocate device no failed\n");
        return -EAGAIN;
    }

    //Allocate driver
    g_pMTKTVout_CharDrv = cdev_alloc();

    if(NULL == g_pMTKTVout_CharDrv)
    {
        unregister_chrdev_region(g_MTKTVoutdevno, 1);
        TVDBG("[TVOUT] Allocate mem for kobject failed\n");
        return -ENOMEM;
    }

    //Attatch file operation.
    cdev_init(g_pMTKTVout_CharDrv, &g_stMTKTVout_fops);
    g_pMTKTVout_CharDrv->owner = THIS_MODULE;


    //Add to system
    if(cdev_add(g_pMTKTVout_CharDrv, g_MTKTVoutdevno, 1))
    {
        printk("[TVOUT] Attatch file operation failed\n");
        unregister_chrdev_region(g_MTKTVoutdevno, 1);
        return -EAGAIN;
    }

    return 0;
}

static inline void Tvout_Drv_Unregister(void)
{
    //Release char driver
    cdev_del(g_pMTKTVout_CharDrv);
    unregister_chrdev_region(g_MTKTVoutdevno, 1);
}

static struct class *pTVoutClass = NULL;
// Called to probe if the device really exists. and create the semaphores
static int Tvout_Drv_Probe(struct platform_device *pdev)
{
    struct device* tvout_device = NULL;
    printk("[TV] %s\n",__func__);

    //Check platform_device parameters
    if(NULL == pdev)
    {
        TVMSG("[TVOUT] platform data missed\n");
        return -ENXIO;
    }

    //register char driver
    //Allocate major no
    if(Tvout_Drv_Register())
    {
        dev_err(&pdev->dev,"register char failed\n");
        return -EAGAIN;
    }

    pTVoutClass = class_create(THIS_MODULE, "tvoutdrv");
    if (IS_ERR(pTVoutClass)) {
        int ret = PTR_ERR(pTVoutClass);
        printk("Unable to create class, err = %d\n", ret);
        return ret;            
    }    
    tvout_device = device_create(pTVoutClass, NULL, g_MTKTVoutdevno, NULL, "TV-out");


#if !defined(TVOUT_ENABLE_M4U)
    //Init temp buffer address
    {
#if defined(TVOUT_USE_KMALLOC)
		printk("[TV] Buffer:using Kmalloc!\n");
		UINT32 tmpFbStartPA;
		UINT32 tmpFbStartVA = kmalloc(tvout_get_vram_size(), GFP_KERNEL);
		if (tmpFbStartVA == NULL)
		{
			printk("unable to allocate memory for TV-out\n");
			return -ENOMEM;
		}
		tmpFbStartPA = virt_to_phys(tmpFbStartVA);
		printk("[TV] kmalloc framebuffer PA=0x%08x, VA=0x%08x\n", tmpFbStartPA, tmpFbStartVA);


#else
		printk("[TV] Buffer:reserved!\n");
        struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 3);
        //fbdev->fb_pa_base = res->start;
        //fbdev->fb_va_base = ioremap_nocache(res->start, res->end - res->start + 1);
        ASSERT(tvout_get_vram_size() <= (res->end - res->start + 1));
        //memset(fbdev->fb_va_base, 0, (res->end - res->start + 1));
		UINT32 tmpFbStartPA = res->start;
#endif
		UINT32 tmpFbPitchInBytes = DISP_GetScreenWidth() * TVOUT_BUFFER_BPP;
		UINT32 tmpFbSizeInBytes = tmpFbPitchInBytes * DISP_GetScreenHeight();
        int i;

		for ( i=0; i < TVOUT_BUFFERS; i++)
		{
			TempBuffer* b = &tvout_context.tmpBuffers[i];
			b->pa = tmpFbStartPA;
			b->pitchInBytes = tmpFbPitchInBytes;
			ASSERT((tmpFbStartPA & 0x7) == 0);
			tmpFbStartPA += tmpFbSizeInBytes;
			printk("[TV]TmpBufferPA(%d): 0x%x, Size: 0x%x, Pitch:0x%x\n",
				   	i, b->pa, tmpFbSizeInBytes, b->pitchInBytes);
		}
	}
#else
	TVDBG("[TV] Buffer: Enable M4U!\n");
	memset(&_m4u_tvout_func, 0, sizeof(_m4u_tvout_func));
#endif

	TVDBG("[TV]%s, TVC TVE TVR Init\n",__func__);
	//Init TV-out engines
	TVC_CHECK_RET(TVC_Init());
    TVE_CHECK_RET(TVE_Init());
    TVR_CHECK_RET(TVR_Init());

#if defined TVOUT_USER_ENABLE_BUTTON
    is_tv_out_turned_on = false;
    if (FACTORY_BOOT == get_boot_mode()) {
        printk("[TV]Factory mode, set user config on\n");
        is_tv_out_turned_on = true;
    }
#else
    is_tv_out_turned_on = true; // no enable button, always be ture.
#endif
    return 0;
}



// Called when the device is being detached from the driver
static int Tvout_Drv_Remove(struct platform_device *pdev)
{
    //unregister char driver.
    Tvout_Drv_Unregister();
    device_destroy(pTVoutClass, g_MTKTVoutdevno);

    class_destroy(pTVoutClass);

    TVDBG("TV-out driver is removed\n");



    return 0;
}


static int Tvout_Drv_Suspend(struct platform_device *pdev, pm_message_t mesg)
{
    return 0;
}

static int Tvout_Drv_Resume(struct platform_device *pdev)
{
    return 0;
}




static struct platform_driver g_stMTKTVout_Platform_Driver = {
    .probe	= Tvout_Drv_Probe,
    .remove	= Tvout_Drv_Remove,
    .suspend= Tvout_Drv_Suspend,
    .resume	= Tvout_Drv_Resume,
    .driver	= {
    .name	= "TV-out",
    .owner	= THIS_MODULE,
    }
};


static int __init Tvout_Drv_Init(void)
{
    if(platform_driver_register(&g_stMTKTVout_Platform_Driver)){
        TVDBG("[TVOUT]failed to register TV out driver\n");
        return -ENODEV;
    }
    printk("[TV] %s\n",__func__);

    memset(&tvout_context, 0, sizeof(tvout_context));

    return 0;
}

static void __exit Tvout_Drv_Exit(void)
{
    TVDBG("[TVOUT] %s\n",__func__);
    platform_driver_unregister(&g_stMTKTVout_Platform_Driver);
}

module_init(Tvout_Drv_Init);
module_exit(Tvout_Drv_Exit);

MODULE_DESCRIPTION("MTK TV-out driver");
MODULE_AUTHOR("Mingchen <Mingchen.gao@Mediatek.com>");
MODULE_LICENSE("GPL");

#endif

