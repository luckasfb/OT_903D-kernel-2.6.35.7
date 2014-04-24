
#include <common.h>
#include <asm/arch/mt65xx.h>
#include <asm/arch/mt65xx_typedefs.h>
//#include <asm/arch/mt6573_pdn_sw.h>

#define MS_TO_US            1000

#define GPT_100HZ_DIV       8125
#define GPT_1000HZ_DIV      1625

/* macro to read the 32 bit timer */
#if (CFG_HZ == 1000)
	#define GPT_HZ_DIV	    GPT_1000HZ_DIV
#elif (CFG_HZ == 100)
	#define GPT_HZ_DIV	    GPT_100HZ_DIV
    #define GPT_GET_MS(x)   ( ((x) >> 13) * 10 )  /* 2 ^ 13 = 8192 ~ 8125 */
#else
	#error "Unsupported Timer Tick"
#endif

#define TIMER_LOAD_VAL      0xffffffff
#define MAX_REG_MS          GPT_GET_MS(TIMER_LOAD_VAL)

/* macro to read the 32 bit timer */
/* the value in this register is updated regularly by HW */
/* every 8125 equals to 10ms, and to avoid division, we use shift instead */
#define READ_TIMER	    GPT_GET_MS(DRV_Reg32(MT6573_XGPT1_COUNT))


static volatile U32 timestamp;
static volatile U32 lastinc;


// TODO : remove this temp solution
//======================================================================

#define GPT4_MAX_TICK_CNT   ((U32)0xFFFFFFFF)
    // 26MHz setting 
    #define GPT4_MAX_US_TIMEOUT ((U32)164926744)    
    #define GPT4_MAX_MS_TIMEOUT ((U32)164926)       
    #define GPT4_1US_TICK       ((U32)26)           
    #define GPT4_1MS_TICK       ((U32)26041)        
    // 26MHz: 1us = 26.042 ticks
    #define TIME_TO_TICK_US(us) ((us)*GPT4_1US_TICK + ((us)*42 + (1000-1))/1000)
    // 26MHz: 1ms = 26041.666 ticks
    #define TIME_TO_TICK_MS(ms) ((ms)*GPT4_1MS_TICK + ((ms)*666 + (1000-1))/1000)


#define GPT4_CON                    ((P_U32)(APMCU_GPTIMER_BASE+0x0028))
#define GPT4_DAT                    ((P_U32)(APMCU_GPTIMER_BASE+0x002C))

#define GPT4_EN                     0x0001
#define GPT4_LOCK                   0x0002

//===========================================================================
// GPT4 fixed 26MHz counter
//===========================================================================
static void gpt_power_on(bool bPowerOn)
{
}

static void gpt4_start(bool bLock)
{
    if(bLock)
    {
        *GPT4_CON = (GPT4_EN|GPT4_LOCK);
    }
    else
    {
        *GPT4_CON = (GPT4_EN);
    }
    //dsb();
}

static void gpt4_stop(void)
{
    *GPT4_CON = 0;
    //dsb();
}

static void gpt4_init(bool bStart)
{
    // power on GPT 
    gpt_power_on(TRUE);

    // clear GPT4 first 
    gpt4_stop();

    // enable GPT4 without lock 
    if(bStart)
    {
        gpt4_start(FALSE);
    }
}

static U32  gpt4_get_current_tick(void)
{
    return (*GPT4_DAT);
}

static bool gpt4_timeout_tick(U32 start_tick, U32 timeout_tick)
{
    register U32 cur_tick;
    register U32 elapse_tick;

    // get current tick 
    cur_tick = gpt4_get_current_tick();

    // check elapse time 
    if( start_tick <= cur_tick )
    {
        elapse_tick = cur_tick - start_tick;
    }
    else
    {
        elapse_tick = (GPT4_MAX_TICK_CNT-start_tick) + cur_tick;
    }

    // check if timeout 
    if( timeout_tick <= elapse_tick )
    {
        // timeout 
        return TRUE;
    }

    return FALSE;
}

//===========================================================================
// us interface 
//===========================================================================
static U32  gpt4_tick2time_us(U32 tick)
{
    return ((tick + (GPT4_1US_TICK-1))/GPT4_1US_TICK);
}

static U32  gpt4_time2tick_us(U32 time_us)
{
    if( GPT4_MAX_US_TIMEOUT <= time_us )
    {
        return GPT4_MAX_US_TIMEOUT;
    }
    else
    {
        return TIME_TO_TICK_US(time_us);
    }
}

//===========================================================================
// ms interface 
//===========================================================================
static U32  gpt4_tick2time_ms(U32 tick)
{
    return ((tick + (GPT4_1MS_TICK-1))/GPT4_1MS_TICK);
}

static U32  gpt4_time2tick_ms(U32 time_ms)
{
    if( GPT4_MAX_MS_TIMEOUT <= time_ms )
    {
        return GPT4_MAX_MS_TIMEOUT;
    }
    else
    {
        return TIME_TO_TICK_MS(time_ms);
    }
}

//===========================================================================
// bust wait 
//===========================================================================
void gpt4_busy_wait_us(U32 timeout_us)
{
    U32 start_tick, timeout_tick;
    
    // get timeout tick 
    timeout_tick  = gpt4_time2tick_us(timeout_us);
    start_tick    = gpt4_get_current_tick();
    
    // wait for timeout 
    while(!gpt4_timeout_tick(start_tick, timeout_tick));
}

void gpt4_busy_wait_ms(U32 timeout_ms)
{
    U32 start_tick, timeout_tick;
    
    // get timeout tick 
    timeout_tick  = gpt4_time2tick_ms(timeout_ms);
    start_tick    = gpt4_get_current_tick();
    
    // wait for timeout 
    while(!gpt4_timeout_tick(start_tick, timeout_tick));
}

//======================================================================


void reset_timer_masked (void)
{
	//lastinc = READ_TIMER;
	lastinc = gpt4_tick2time_ms(*GPT4_DAT);
	timestamp = 0;
}

ulong get_timer_masked (void)
{
	volatile U32 now;

	//now = READ_TIMER;
	now = gpt4_tick2time_ms(*GPT4_DAT);

	if (now >= lastinc) {
		timestamp = timestamp + now - lastinc; 			/* normal */
	} else { 
		timestamp = timestamp + MAX_REG_MS - lastinc + now;	/* overflow */
	}
	lastinc = now;

	return timestamp;
}

void reset_timer (void)
{
	reset_timer_masked();
}

#define MAX_TIMESTAMP_US  0xffffffff
ulong get_timer (ulong base)
{
	ulong current_timestamp = 0 ;	
	ulong temp = 0;

	current_timestamp = get_timer_masked ();

	if(current_timestamp >= base)
	{	/* timestamp normal */					
		return (current_timestamp - base);
	}
	/* timestamp overflow */
	//dbg_print("return = 0x%x\n",MAX_TIMESTAMP_US - ( base - current_timestamp ));	
	temp = base - current_timestamp;
	
	return (MAX_TIMESTAMP_US - temp);
}

void set_timer (ulong ticks)
{
	timestamp = ticks;
}

/* delay msec mseconds */
void mdelay (unsigned long msec)
{
	ulong start_time = 0;

	start_time = get_timer(0);
	while(get_timer(start_time) < msec);
}

/* delay usec useconds */
void udelay (unsigned long usec)
{
	ulong tmo, tmp;

	if (usec >= 1000) {		/* if "big" number, spread normalization to seconds */
		tmo = usec / 1000;	/* start to normalize for usec to ticks per sec */
		tmo *= MS_TO_US;	/* find number of "ticks" to wait to achieve target */
		tmo /= 1000;		/* finish normalize. */
	} else {				/* else small number, don't kill it prior to HZ multiply */
		tmo = usec * MS_TO_US;
		tmo /= (1000*1000);
	}

	tmp = get_timer (0);		/* get current timestamp */
	if ((tmo + tmp + 1) < tmp)	/* if setting this fordward will roll time stamp */
		reset_timer_masked();	/* reset "advancing" timestamp to 0, set lastdec value */
	else
		tmo += tmp;		/* else, set advancing stamp wake up time */

	while (get_timer_masked() < tmo)/* loop till event */
		/*NOP*/;
}

unsigned long long get_ticks(void)
{
	return (unsigned long long)get_timer(0);
}

ulong get_tbclk (void)
{
	ulong tbclk;

	tbclk = CFG_HZ;
	return tbclk;
}

void mt6573_timer_init(void)
{
#if 0
    PDN_Power_CONA_DOWN(PDN_PERI_XGPT, KAL_FALSE); // Jau

    /*
     *  Disable the GTP1 to avoid the hardware runing and ack the old interrupt
     *  indication.            */
    DRV_WriteReg32(MT6573_XGPT1_CON, 0x32);
    DRV_WriteReg32(MT6573_XGPT_IRQACK, 0x01);

    /*
     *  Disable Interrupt of GPT1
     */
    DRV_SetReg32(MT6573_XGPT_IRQEN, 0x00);

    /*
     *  Specify the resolution
     */
    DRV_WriteReg32(MT6573_XGPT1_PRESCALE, 0x4);

    /*
     *  Set timeout count of GPT0.     
     *  We expected GPT interrupt arise every 10ms. (HZ = 100Hz)     
     */	
    /* MT6573 812500/100=8125   */
    //DRV_WriteReg32(MT6573_XGPT1_COMPARE, 8125);

    DRV_WriteReg32(MT6573_XGPT1_CON, 0x33);
#else
    gpt4_init(TRUE);
#endif

    // init timer system
    reset_timer();
}
