





#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/completion.h>
#include <linux/mm.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/vmalloc.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/wakelock.h>
#include <linux/semaphore.h>
#include <linux/jiffies.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <mach/irqs.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/tcm.h>
#include <linux/pmic6326_sw.h>
//#include <mach/mt6573_gpio.h>
#include <mach/mt6573_irq.h>
#include <mach/mt6573_typedefs.h>
#include <mach/mt6573_pll.h>
#include <mach/mt6573_boot.h>

#include "auddrv_def.h"
#include "auddrv_ioctl_msg.h"
#include "auddrv_register.h"
#include "auddrv_headset.h"
#include "yusu_android_speaker.h"
#include <mach/mt6573_ost_sm.h>

#include "auddrv_kernel_driver.h"

#if defined(CONFIG_MTK_COMBO) || defined(CONFIG_MTK_COMBO_MODULE)
#include <mach/mt_combo.h>
#endif

#if defined(MTK_MT5192) || defined(MTK_MT5193)
#include <cust_matv.h>
#endif

#define AUDDRV_NAME   "MediaTek Audio Driver"
#define AUDDRV_AUTHOR "MediaTek WCX"

// internal memory usage
#define AFE_BUF_SLAVE_SIZE  0x2700   // 9984 bytes
#define AFE_BUF_LENGTH 0x3000   // 12k bytes
#define AWB_BUF_LENGTH 0x5000  //20K bytes
#define I2S_BUF_LENGTH 0x2400 //9k bytes
#define I2S_BUF_MAX    0x5000   // 20k bytes

#ifndef MT6573_AFE_MCU_IRQ_LINE
#define MT6573_AFE_MCU_IRQ_LINE    126
#endif

//#define AFE_INT_TIMEOUT       (200)  // 200 jiffies = 2000 ms
#define AFE_INT_TIMEOUT       (6)  // 6 jiffies = 60 ms
//#define I2S_INPUT_INT_TIMEOUT (500)  // jiffies
#define I2S_INPUT_INT_TIMEOUT (200)  // jiffies
#define AWB_TIMER_TIMEOUT (200)  // jiffies
#define AWB_TIMER_INTERVAL    (50)  // in ms

// ms to Jiffies--> (ms)* HZ / 1000;
// or
// ms to Jiffies--> ((ms)*HZ + 512) >> 10
#ifdef MM_SYSRAM_BASE_PA
#undef MM_SYSRAM_BASE_PA
#define MM_SYSRAM_BASE_PA			0x40000000
#endif
#define USE_PM_API

typedef enum
{
    DAI_STOP,
    DAI_INIT,
    DAI_ASSERTION,
    DAI_NEGATION
}DAIRST_T;


//if Flag_Aud_DL1_SlaveOn is false, the definition is valid
#define DL1_USE_FLEXL2 //Audio DL1 playback use master mode by FlexL2 memory
//#define DL1_USE_SYSRAM//Audio DL1 playback use slave mode by MMsys memory(only for test)

//#define I2SIN_USE_FLEXL2 //I2S Input playback use master mode by FlexL2 memory


typedef struct
{
   volatile UINT32 Suspend_AUDIO_TOP_CON0;
   volatile UINT32 Suspend_AFE_DAC_CON0;
   volatile UINT32 Suspend_AFE_DAC_CON1;
   volatile UINT32 Suspend_AFE_I2S_IN_CON;
   volatile UINT32 Suspend_AFE_FOC_CON;
   volatile UINT32 Suspend_AFE_DAIBT_CON;
   volatile UINT32 Suspend_AFE_CONN0;
   volatile UINT32 Suspend_AFE_CONN1;
   volatile UINT32 Suspend_AFE_CONN2;
   volatile UINT32 Suspend_AFE_CONN3;
   volatile UINT32 Suspend_AFE_CONN4;

   volatile UINT32 Suspend_AFE_I2S_OUT_CON;
   volatile UINT32 Suspend_AFE_DL1_BASE;
   volatile UINT32 Suspend_AFE_DL1_CUR;
   volatile UINT32 Suspend_AFE_DL1_END;
   volatile UINT32 Suspend_AFE_DL2_BASE;
   volatile UINT32 Suspend_AFE_DL2_CUR;
   volatile UINT32 Suspend_AFE_DL2_END;
   volatile UINT32 Suspend_AFE_I2S_BASE;
   volatile UINT32 Suspend_AFE_I2S_CUR;
   volatile UINT32 Suspend_AFE_I2S_END;

   volatile UINT32 Suspend_AFE_AWB_BASE;
   volatile UINT32 Suspend_AFE_AWB_END;

   volatile UINT32 Suspend_AFE_IRQ_CON;
   volatile UINT32 Suspend_AFE_IR_STATUS;
   volatile UINT32 Suspend_AFE_IR_CLR;
   volatile UINT32 Suspend_AFE_IRQ_CNT1;
   volatile UINT32 Suspend_AFE_IRQ_CNT2;
   volatile UINT32 Suspend_AFE_IRQ_MON;
   volatile UINT32 Suspend_AFE_DL_SDM_CON0;
   volatile UINT32 Suspend_AFE_DL_SRC2_1;
   volatile UINT32 Suspend_AFE_DL_SRC2_2;

   volatile UINT32 Suspend_AFE_SIDETONE_CON0;
   volatile UINT32 Suspend_AFE_SIDETONE_CON1;

   volatile UINT32 Suspend_AFE_UL_SRC_0;
   volatile UINT32 Suspend_AFE_UL_SRC_1;
   volatile UINT32 Suspend_AFE_TOP_CONTROL_0;

   volatile UINT32 Suspend_AFE_SDM_GAIN_STAGE;

   volatile UINT32 Suspend_PLL_CON2  ;
   volatile UINT32 Suspend_AUDIO_CON0;
   volatile UINT32 Suspend_AUDIO_CON1;
   volatile UINT32 Suspend_AUDIO_CON2;
   volatile UINT32 Suspend_AUDIO_CON3;
   volatile UINT32 Suspend_AUDIO_CON4;
   volatile UINT32 Suspend_AUDIO_CON5;
   volatile UINT32 Suspend_AUDIO_CON6;
   volatile UINT32 Suspend_AUDIO_CON7;
   volatile UINT32 Suspend_AUDIO_CON8;
   volatile UINT32 Suspend_AUDIO_CON9;
   volatile UINT32 Suspend_AUDIO_CON10;
   volatile UINT32 Suspend_AUDIO_CON20;
   volatile UINT32 Suspend_AUDIO_CON21;
   volatile UINT32 Suspend_AUDIO_CON22;
   volatile UINT32 Suspend_AUDIO_CON23;
   volatile UINT32 Suspend_AUDIO_CON24;
   volatile UINT32 Suspend_AUDIO_CON25;
   volatile UINT32 Suspend_AUDIO_CON26;
   volatile UINT32 Suspend_AUDIO_CON27;
   volatile UINT32 Suspend_AUDIO_CON28;
   volatile UINT32 Suspend_AUDIO_CON29;
   volatile UINT32 Suspend_AUDIO_CON30;
   volatile UINT32 Suspend_AUDIO_CON31;
   volatile UINT32 Suspend_AUDIO_CON32;
   volatile UINT32 Suspend_VAUDN_CON0;
   volatile UINT32 Suspend_VAUDP_CON0;
   volatile UINT32 Suspend_VAUDP_CON1;
   volatile UINT32 Suspend_VAUDP_CON2;
   volatile UINT32 Suspend_VA12_CON0 ;
   volatile UINT32 Suspend_VMIC_CON0 ;
   volatile UINT32 Suspend_VA25_CON0 ;

   volatile UINT32 Suspend_ACIF_WR_PATH;

}AudDrv_Suspend_Reg;

static struct fasync_struct  *AudDrv_async = NULL;
static AFE_DL_CONTROL_T      AFE_dl_Control_context;
static AFE_DL_CONTROL_T      *AFE_dl_Control = &AFE_dl_Control_context;   // main control of AFE
static I2S_INPUT_CONTROL_T   I2S_input_Control_context;
static I2S_INPUT_CONTROL_T   *I2S_input_Control = &I2S_input_Control_context;   // main control of I2S input
static DAI_OUTPUT_CONTROL_T  DAI_output_Control_context;
//static DAI_OUTPUT_CONTROL_T  *DAI_output_Control = &DAI_output_Control_context;   // main control of DAI output
static AWB_INPUT_CONTROL_T  AWB_input_Control_context;
static AWB_INPUT_CONTROL_T   *AWB_input_Control = &AWB_input_Control_context;   // main control of I2S input
static AudDrv_Suspend_Reg    Suspend_reg;
static SPH_Control           SPH_Ctrl_State;
static volatile kal_uint8    Afe_irq_status  = 0;
static int Aud_Flush_cntr =0;

static kal_uint32 wait_queue_flag     = 0;
static kal_uint32 I2S_wait_queue_flag = 0;
static kal_uint32 AWB_wait_queue_flag = 0;

static spinlock_t auddrv_lock         = SPIN_LOCK_UNLOCKED;
static int        Afe_Mem_Pwr_on      = 0;
static int        I2S_Pwr_on  =0;
static int        Aud_AFE_Clk_cntr          = 0;
static int        Aud_ADC_Clk_cntr          = 0;
static int        Aud_I2S_Clk_cntr          = 0;
static int        Aud_AWB_Clk_cntr          =0;
static kal_bool   AMP_Flag            = false; // AMP status
static kal_bool   AWB_Timer_Flag  = false; // AWB_timer_Flaer
static kal_bool   IsSuspen            = false; // is suspend flag
static kal_bool   Flag_AudDrv_ClkOn_1st        = false; // 1st AudDrv ClkOn
static kal_bool   Flag_Aud_26MClkOn            = false; // 1st AudDrv ClkOn
static kal_bool   Flag_Aud_DL1_SlaveOn            = false;
static kal_bool   b_reset_afe_pwr        = false;
static kal_bool   b_reset_i2s_pwr        = false;
static int        b_adc_clk_on           = 0;
static int        b_i2s_clk_on           = 0;
static int        b_afe_clk_on           = 0;
static int        b_afe_line_in_clk_on   = 0;

static kal_bool   b_SPKSound_Happen      = false;  // exception happened (AEE)

static kal_bool   b_FM_Analog_Enable     = false;

struct wake_lock  Audio_wake_lock;
struct wake_lock  Audio_record_wake_lock;

static kal_uint8 *AudSRAMVirtBufAddr;
static kal_uint8 *I2SInVirtBufAddr;
static kal_uint32 I2SInPhyBufAddr;

static char       auddrv_name[]       = "AudDrv_driver_device";
static u64        AudDrv_dmamask      = 0xffffffffUL;

static struct timer_list AWB_timer;

static int AudDrv_fasync(int fd, struct file *flip, int mode);
static int AudDrv_remap_mmap(struct file *flip, struct vm_area_struct *vma);
static int AudDrv_Read_Procmem(char *buf,char **start, off_t offset, int count , int *eof, void *data);
static void AudDrv_Store_reg(void);
static void AudDrv_Recover_reg(void);
static int AudDrv_Reset(void);
void AudDrv_Clk_On(void);
void AudDrv_Clk_Off(void);
void AudDrv_ADC_Clk_On(void);
void AudDrv_ADC_Clk_Off(void);

void Auddrv_AWB_timer_Routine(unsigned long data);
bool Auddrv_Check_Afe_Clock(void);
void AudDrv_allcate_AWB_buffer(void);
void AudDrv_Start_AWB_Stream(struct file *fp);
void AudDrv_Standby_AWB_Stream(struct file *fp);

void AudDrv_Init_I2S_InputStream(kal_uint32 I2S_Buffer_Length,struct file *fp);


static void AudDrv_magic_tasklet(unsigned long data );
DECLARE_TASKLET(magic_tasklet_handle, AudDrv_magic_tasklet, 0L);
DECLARE_WAIT_QUEUE_HEAD(DL_Wait_Queue);
DECLARE_WAIT_QUEUE_HEAD(I2Sin_Wait_Queue);
DECLARE_WAIT_QUEUE_HEAD(AWB_Wait_Queue);

ssize_t AudDrv_I2S_Read(struct file *fp,  char __user *data, size_t count, loff_t *offset);
ssize_t AudDrv_AWB_Read(struct file *fp,  char __user *data, size_t count, loff_t *offset);
void AudDrv_I2Sin_Tasklet(void);


bool AudDrv_Get_FM_Analog_Status(void)
{
   printk("AudDrv_Get_FM_Analog_Status (%d) \n",b_FM_Analog_Enable);
   return b_FM_Analog_Enable;
}

void AudDrv_Test_Print(void)
{
   // AudioSys Register Setting
   printk("AUDIO_TOP_CON0  = 0x%x\n",Afe_Get_Reg(AUDIO_TOP_CON0));
   printk("AFE_DAC_CON0    = 0x%x\n",Afe_Get_Reg(AFE_DAC_CON0));
   printk("AFE_DAC_CON1    = 0x%x\n",Afe_Get_Reg(AFE_DAC_CON1));

   printk("AFE_DL1_BASE    = 0x%x\n",Afe_Get_Reg(AFE_DL1_BASE));
   printk("AFE_DL1_CUR     = 0x%x\n",Afe_Get_Reg(AFE_DL1_CUR));
   printk("AFE_DL1_END     = 0x%x\n",Afe_Get_Reg(AFE_DL1_END));
   printk("AFE_IRQ_CON     = 0x%x\n",Afe_Get_Reg(AFE_IRQ_CON));
   printk("AFE_IR_STATUS   = 0x%x\n",Afe_Get_Reg(AFE_IR_STATUS));
   printk("AFE_IR_CLR      = 0x%x\n",Afe_Get_Reg(AFE_IR_CLR));
   printk("AFE_IRQ_CNT1    = 0x%x\n",Afe_Get_Reg(AFE_IRQ_CNT1));

   printk("AFE_DL_SRC2_1   = 0x%x\n",Afe_Get_Reg(AFE_DL_SRC2_1));
   printk("AFE_DL_SRC2_2   = 0x%x\n",Afe_Get_Reg(AFE_DL_SRC2_2));
   printk("AFE_DL_SRC1_1   = 0x%x\n",Afe_Get_Reg(AFE_DL_SRC1_1));

   // Audio Variable Setting
   printk("AMP_Flag         = %d\n",AMP_Flag);
   printk("Afe_Mem_Pwr_on   = %d\n",Afe_Mem_Pwr_on);
   printk("I2S_Pwr_on       = %d\n",I2S_Pwr_on);
   printk("Aud_AFE_Clk_cntr = %d\n",Aud_AFE_Clk_cntr);
   printk("Aud_ADC_Clk_cntr = %d\n",Aud_ADC_Clk_cntr);
   printk("Aud_I2S_Clk_cntr = %d\n",Aud_I2S_Clk_cntr);
   // Analog ABB Register Setting
   printk("PLL_CON2   = 0x%x\n",Ana_Get_Reg(PLL_CON2));
   printk("AUDIO_CON0 = 0x%x\n",Ana_Get_Reg(AUDIO_CON0));
   printk("AUDIO_CON1 = 0x%x\n",Ana_Get_Reg(AUDIO_CON1));
   printk("AUDIO_CON2 = 0x%x\n",Ana_Get_Reg(AUDIO_CON2));
   printk("AUDIO_CON3 = 0x%x\n",Ana_Get_Reg(AUDIO_CON3));
   printk("AUDIO_CON4 = 0x%x\n",Ana_Get_Reg(AUDIO_CON4));
   printk("AUDIO_CON5 = 0x%x\n",Ana_Get_Reg(AUDIO_CON5));
   printk("AUDIO_CON6 = 0x%x\n",Ana_Get_Reg(AUDIO_CON6));
   printk("AUDIO_CON7 = 0x%x\n",Ana_Get_Reg(AUDIO_CON7));
   printk("AUDIO_CON8 = 0x%x\n",Ana_Get_Reg(AUDIO_CON8));
   printk("AUDIO_CON9 = 0x%x\n",Ana_Get_Reg(AUDIO_CON9));
   printk("AUDIO_CON10 = 0x%x\n",Ana_Get_Reg(AUDIO_CON10));

    // Analog PMU Register Setting
    printk("VAUDN_CON0 = 0x%x\n",Ana_Get_Reg(VAUDN_CON0));
    printk("VAUDP_CON0 = 0x%x\n",Ana_Get_Reg(VAUDP_CON0));
}

void AudDrv_AudReg_Log_Print(void)
{
   // AudioSys Register Setting
   printk("AUDIO_TOP_CON0  = 0x%x\n",Afe_Get_Reg(AUDIO_TOP_CON0));
   printk("AFE_DAC_CON0    = 0x%x\n",Afe_Get_Reg(AFE_DAC_CON0));
   printk("AFE_DAC_CON1    = 0x%x\n",Afe_Get_Reg(AFE_DAC_CON1));
   printk("AFE_I2S_IN_CON  = 0x%x\n",Afe_Get_Reg(AFE_I2S_IN_CON));
   printk("AFE_FOC_CON     = 0x%x\n",Afe_Get_Reg(AFE_FOC_CON));
   printk("AFE_DAIBT_CON   = 0x%x\n",Afe_Get_Reg(AFE_DAIBT_CON));
   printk("AFE_CONN0       = 0x%x\n",Afe_Get_Reg(AFE_CONN0));
   printk("AFE_CONN1       = 0x%x\n",Afe_Get_Reg(AFE_CONN1));
   printk("AFE_CONN2       = 0x%x\n",Afe_Get_Reg(AFE_CONN2));
   printk("AFE_CONN3       = 0x%x\n",Afe_Get_Reg(AFE_CONN3));
   printk("AFE_CONN4       = 0x%x\n",Afe_Get_Reg(AFE_CONN4));
   printk("AFE_I2S_OUT_CON = 0x%x\n",Afe_Get_Reg(AFE_I2S_OUT_CON));
   printk("AFE_DL1_BASE    = 0x%x\n",Afe_Get_Reg(AFE_DL1_BASE));
   printk("AFE_DL1_CUR     = 0x%x\n",Afe_Get_Reg(AFE_DL1_CUR));
   printk("AFE_DL1_END     = 0x%x\n",Afe_Get_Reg(AFE_DL1_END));
   printk("AFE_IRQ_CON     = 0x%x\n",Afe_Get_Reg(AFE_IRQ_CON));
   printk("AFE_IR_STATUS   = 0x%x\n",Afe_Get_Reg(AFE_IR_STATUS));
   printk("AFE_IR_CLR      = 0x%x\n",Afe_Get_Reg(AFE_IR_CLR));
   printk("AFE_IRQ_CNT1    = 0x%x\n",Afe_Get_Reg(AFE_IRQ_CNT1));
   printk("AFE_IRQ_CNT2    = 0x%x\n",Afe_Get_Reg(AFE_IRQ_CNT2));
//   printk("AFE_IRQ_MON     = 0x%x\n",Afe_Get_Reg(AFE_IRQ_MON));
   printk("AFE_MODEM_IRQ_CON   = 0x%x\n",Afe_Get_Reg(AFE_MODEM_IRQ_CON));
   printk("AFE_MODEM_IR_STATUS = 0x%x\n",Afe_Get_Reg(AFE_MODEM_IR_STATUS));
   printk("AFE_MODEM_IR_CLR    = 0x%x\n",Afe_Get_Reg(AFE_MODEM_IR_CLR));
   printk("AFE_MODEM_IRQ_CNT1  = 0x%x\n",Afe_Get_Reg(AFE_MODEM_IRQ_CNT1));
   printk("AFE_MODEM_IRQ_CNT2  = 0x%x\n",Afe_Get_Reg(AFE_MODEM_IRQ_CNT2));

   printk("AFE_DL_SDM_CON0 = 0x%x\n",Afe_Get_Reg(AFE_DL_SDM_CON0));
   printk("AFE_DL_SRC2_1   = 0x%x\n",Afe_Get_Reg(AFE_DL_SRC2_1));
   printk("AFE_DL_SRC2_2   = 0x%x\n",Afe_Get_Reg(AFE_DL_SRC2_2));
   printk("AFE_DL_SRC1_1   = 0x%x\n",Afe_Get_Reg(AFE_DL_SRC1_1));

   // for Audio HQA
//   printk("AFE_UL_AGC5     = 0x%x\n",Afe_Get_Reg(AFE_UL_AGC5));
//   printk("AFE_UL_AGC13    = 0x%x\n",Afe_Get_Reg(AFE_UL_AGC13));

//   printk("AFE_SIDETONE_CON0 = 0x%x\n",Afe_Get_Reg(AFE_SIDETONE_CON0));
//   printk("AFE_SIDETONE_CON1 = 0x%x\n",Afe_Get_Reg(AFE_SIDETONE_CON1));
   printk("AFE_UL_SRC_0      = 0x%x\n",Afe_Get_Reg(AFE_UL_SRC_0));
   printk("AFE_UL_SRC_1      = 0x%x\n",Afe_Get_Reg(AFE_UL_SRC_1));
   printk("AFE_SDM_GAIN_STAGE  = 0x%x\n",Afe_Get_Reg(AFE_SDM_GAIN_STAGE));
   // Audio Variable Setting
   printk("AMP_Flag         = %d\n",AMP_Flag);
   printk("Afe_Mem_Pwr_on   = %d\n",Afe_Mem_Pwr_on);
   printk("I2S_Pwr_on       = %d\n",I2S_Pwr_on);
   printk("Aud_AFE_Clk_cntr = %d\n",Aud_AFE_Clk_cntr);
   printk("Aud_ADC_Clk_cntr = %d\n",Aud_ADC_Clk_cntr);
   printk("Aud_I2S_Clk_cntr = %d\n",Aud_I2S_Clk_cntr);
   // Analog ABB Register Setting
   printk("PLL_CON2   = 0x%x\n",Ana_Get_Reg(PLL_CON2));
   printk("AUDIO_CON0 = 0x%x\n",Ana_Get_Reg(AUDIO_CON0));
   printk("AUDIO_CON1 = 0x%x\n",Ana_Get_Reg(AUDIO_CON1));
   printk("AUDIO_CON2 = 0x%x\n",Ana_Get_Reg(AUDIO_CON2));
   printk("AUDIO_CON3 = 0x%x\n",Ana_Get_Reg(AUDIO_CON3));
   printk("AUDIO_CON4 = 0x%x\n",Ana_Get_Reg(AUDIO_CON4));
   printk("AUDIO_CON5 = 0x%x\n",Ana_Get_Reg(AUDIO_CON5));
   printk("AUDIO_CON6 = 0x%x\n",Ana_Get_Reg(AUDIO_CON6));
   printk("AUDIO_CON7 = 0x%x\n",Ana_Get_Reg(AUDIO_CON7));
   printk("AUDIO_CON8 = 0x%x\n",Ana_Get_Reg(AUDIO_CON8));
   printk("AUDIO_CON9 = 0x%x\n",Ana_Get_Reg(AUDIO_CON9));
   printk("AUDIO_CON10 = 0x%x\n",Ana_Get_Reg(AUDIO_CON10));
   printk("AUDIO_CON20 = 0x%x\n",Ana_Get_Reg(AUDIO_CON20));
   printk("AUDIO_CON21 = 0x%x\n",Ana_Get_Reg(AUDIO_CON21));
   printk("AUDIO_CON22 = 0x%x\n",Ana_Get_Reg(AUDIO_CON22));
   printk("AUDIO_CON23 = 0x%x\n",Ana_Get_Reg(AUDIO_CON23));
   printk("AUDIO_CON24 = 0x%x\n",Ana_Get_Reg(AUDIO_CON24));
   printk("AUDIO_CON25 = 0x%x\n",Ana_Get_Reg(AUDIO_CON25));
   printk("AUDIO_CON26 = 0x%x\n",Ana_Get_Reg(AUDIO_CON26));
   printk("AUDIO_CON27 = 0x%x\n",Ana_Get_Reg(AUDIO_CON27));
   printk("AUDIO_CON28 = 0x%x\n",Ana_Get_Reg(AUDIO_CON28));
   printk("AUDIO_CON29 = 0x%x\n",Ana_Get_Reg(AUDIO_CON29));
   printk("AUDIO_CON30 = 0x%x\n",Ana_Get_Reg(AUDIO_CON30));
   printk("AUDIO_CON31 = 0x%x\n",Ana_Get_Reg(AUDIO_CON31));
   printk("AUDIO_CON32 = 0x%x\n",Ana_Get_Reg(AUDIO_CON32));

    // Analog PMU Register Setting
    printk("VAUDN_CON0 = 0x%x\n",Ana_Get_Reg(VAUDN_CON0));
    printk("VAUDP_CON0 = 0x%x\n",Ana_Get_Reg(VAUDP_CON0));
    printk("VAUDP_CON1 = 0x%x\n",Ana_Get_Reg(VAUDP_CON1));
    printk("VAUDP_CON2 = 0x%x\n",Ana_Get_Reg(VAUDP_CON2));
    printk("VA12_CON0  = 0x%x\n",Ana_Get_Reg(VA12_CON0));
    printk("VMIC_CON0  = 0x%x\n",Ana_Get_Reg(VMIC_CON0));
    printk("VA25_CON0  = 0x%x\n",Ana_Get_Reg(VA25_CON0));
    printk("ACIF_WR_PATH = 0x%x\n",Ana_Get_Reg(ACIF_WR_PATH));
    // Speech/Record/BGS/TTY/VT Setting
}

static void AudDrv_Store_Reg_LouderSPKSound(void)
{
   printk("+AudDrv_Store_Reg_LouderSPKSound \n");
   // Digital register setting
   Suspend_reg.Suspend_AFE_DAC_CON0    = Afe_Get_Reg(AFE_DAC_CON0);
   Suspend_reg.Suspend_AFE_DL_SDM_CON0 = Afe_Get_Reg(AFE_DL_SDM_CON0);
   Suspend_reg.Suspend_AFE_DL_SRC2_1   = Afe_Get_Reg(AFE_DL_SRC2_1);
   Suspend_reg.Suspend_AFE_DL_SRC2_2   = Afe_Get_Reg(AFE_DL_SRC2_2);
   Suspend_reg.Suspend_AFE_UL_SRC_1      = Afe_Get_Reg(AFE_UL_SRC_1);
   Suspend_reg.Suspend_AFE_TOP_CONTROL_0 = Afe_Get_Reg(AFE_TOP_CONTROL_0);
   Suspend_reg.Suspend_AFE_SDM_GAIN_STAGE = Afe_Get_Reg(AFE_SDM_GAIN_STAGE);
   Suspend_reg.Suspend_AFE_I2S_BASE    = Afe_Get_Reg(AFE_I2S_BASE);
   Suspend_reg.Suspend_AFE_I2S_END     = Afe_Get_Reg(AFE_I2S_END);

   // Analog register setting
   Suspend_reg.Suspend_AUDIO_CON0 = Ana_Get_Reg(AUDIO_CON0);
   Suspend_reg.Suspend_AUDIO_CON1 = Ana_Get_Reg(AUDIO_CON1);
   Suspend_reg.Suspend_AUDIO_CON2 = Ana_Get_Reg(AUDIO_CON2);
   Suspend_reg.Suspend_AUDIO_CON3 = Ana_Get_Reg(AUDIO_CON3);
   Suspend_reg.Suspend_AUDIO_CON4 = Ana_Get_Reg(AUDIO_CON4);
   Suspend_reg.Suspend_AUDIO_CON5 = Ana_Get_Reg(AUDIO_CON5);
   Suspend_reg.Suspend_AUDIO_CON6 = Ana_Get_Reg(AUDIO_CON6);
   Suspend_reg.Suspend_AUDIO_CON7 = Ana_Get_Reg(AUDIO_CON7);
   Suspend_reg.Suspend_AUDIO_CON8 = Ana_Get_Reg(AUDIO_CON8);
   Suspend_reg.Suspend_AUDIO_CON9 = Ana_Get_Reg(AUDIO_CON9);
   Suspend_reg.Suspend_AUDIO_CON10 = Ana_Get_Reg(AUDIO_CON10);

   Afe_Set_Reg(AFE_IR_CLR,0x001f,0x00ff);    // clear interrupt
   mdelay(2);                                // wait for a shor latency
   Afe_Set_Reg(AFE_IR_CLR,0x001f,0x00ff);    // clear interrupt

   PRINTK_AUDDRV("-AudDrv_Store_Reg_LouderSPKSound \n");
}

static void AudDrv_Recover_Reg_LouderSPKSound(void)
{
   printk("+AudDrv_Recover_Reg_LouderSPKSound \n");

   // Digital register setting
   // Only recovery AFE_ON (because only modify "AFE_ON" for LouderSPKSound function. )
   // don't care the "I2S ON" buts
   Afe_Set_Reg(AFE_I2S_BASE,Suspend_reg.Suspend_AFE_I2S_BASE, MASK_ALL);
   Afe_Set_Reg(AFE_I2S_END,Suspend_reg.Suspend_AFE_I2S_END, MASK_ALL);
   Afe_Set_Reg(AFE_DAC_CON0,Suspend_reg.Suspend_AFE_DAC_CON0, 0x1);

   Afe_Set_Reg(AFE_DL_SDM_CON0,Suspend_reg.Suspend_AFE_DL_SDM_CON0, MASK_ALL);
   Afe_Set_Reg(AFE_DL_SRC2_1,Suspend_reg.Suspend_AFE_DL_SRC2_1, MASK_ALL);
   Afe_Set_Reg(AFE_DL_SRC2_2,Suspend_reg.Suspend_AFE_DL_SRC2_2, MASK_ALL);
   Afe_Set_Reg(AFE_UL_SRC_1,Suspend_reg.Suspend_AFE_UL_SRC_1, MASK_ALL);
   Afe_Set_Reg(AFE_TOP_CONTROL_0,Suspend_reg.Suspend_AFE_TOP_CONTROL_0, MASK_ALL);
   Afe_Set_Reg(AFE_SDM_GAIN_STAGE,Suspend_reg.Suspend_AFE_SDM_GAIN_STAGE, MASK_ALL);

   // Analog register setting
   Ana_Set_Reg(AUDIO_CON0,Suspend_reg.Suspend_AUDIO_CON0, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON1,Suspend_reg.Suspend_AUDIO_CON1, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON2,Suspend_reg.Suspend_AUDIO_CON2, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON3,Suspend_reg.Suspend_AUDIO_CON3, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON4,Suspend_reg.Suspend_AUDIO_CON4, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON5,Suspend_reg.Suspend_AUDIO_CON5, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON6,Suspend_reg.Suspend_AUDIO_CON6, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON7,Suspend_reg.Suspend_AUDIO_CON7, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON8,Suspend_reg.Suspend_AUDIO_CON8, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON9,Suspend_reg.Suspend_AUDIO_CON9, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON10,Suspend_reg.Suspend_AUDIO_CON10, MASK_ALL);

   PRINTK_AUDDRV("-AudDrv_Recover_Reg_LouderSPKSound \n");
}

int LouderSPKSound(kal_uint32 time)
{
   int prev_i2s_clk, prev_adc_clk;
   printk("+AudDrv LouderSPKSound time = %d ms \n",time);
   AudDrv_Clk_On();

   if(time > 250){
      printk("AudDrv LouderSPKSound +Dump Audio Reg \n");
//      AudDrv_AudReg_Log_Print();
      printk(". \n");
      printk(".. \n");
   }

   b_SPKSound_Happen = true;

   prev_i2s_clk = Aud_I2S_Clk_cntr;
   prev_adc_clk = Aud_ADC_Clk_cntr;
   //start SinWave
   {
      AudDrv_Store_Reg_LouderSPKSound();

      spin_lock_bh(&auddrv_lock);

      Ana_Set_Reg(AUDIO_CON0,0x0000,0xffffffff);
      Ana_Set_Reg(AUDIO_CON1,0x0C0C,0xffffffff);
      Ana_Set_Reg(AUDIO_CON2,0x000C,0xffffffff);
      Ana_Set_Reg(AUDIO_CON3,0x01B0,0xffffffff);
      Ana_Set_Reg(AUDIO_CON4,0x1818,0xffffffff);
      Ana_Set_Reg(AUDIO_CON5,0x0440,0xffffffff);
      Ana_Set_Reg(AUDIO_CON6,0x001B,0xffffffff);
      Ana_Set_Reg(AUDIO_CON7,0x0400,0xffffffff);
      Ana_Set_Reg(AUDIO_CON10,0x0020,0xffffffff);
      Ana_Set_Reg(AUDIO_CON7,0x0000,0xffffffff);
      Ana_Set_Reg(AUDIO_CON10,0x0000,0xffffffff);
      Ana_Set_Reg(AUDIO_CON8,0x7000,0xffffffff);
      Ana_Set_Reg(AUDIO_CON9,0x0003,0xffffffff);

      Ana_Set_Reg(VAUDN_CON0,0x0001,0x000f);
      Ana_Set_Reg(VAUDP_CON0,0x0001,0x000f);
      Ana_Set_Reg(VAUDP_CON1,0x0000,0xffff);
      Ana_Set_Reg(VAUDP_CON2,0x012B,0xffff);

      Afe_Set_Reg(AFE_DAC_CON0,0x1,0x1);
      Afe_Set_Reg(AFE_TOP_CONTROL_0,0x40000000,0xffffffff);
      Afe_Set_Reg(AFE_DL_SRC1_1,0x1,0x1);
      Afe_Set_Reg(AFE_DL_SRC2_1,0x73000003,0xf3000003);
      Afe_Set_Reg(AFE_DL_SRC2_2,0xf0000000,0xffff0000);  // gain setting twice  -66dBv:0x00400000 0dBv:0xf0000000 mute:0x00000000
      Afe_Set_Reg(AFE_UL_SRC_1,0x08828828,0xffffffff);   // freq setting 750Hz: 1.5KHz:0x08828828
      Afe_Set_Reg(AFE_SDM_GAIN_STAGE,0x1e,0xffff00ff);  // SDM gain (1e->1c)
      Afe_Set_Reg(AFE_DL_SDM_CON0,0x08800000,0xffffffff);  // SDM choose (2-Order 9-Bit Scrambler and No Dithering)
      Ana_Set_Reg(ACIF_WR_PATH,0x4020,0x4020);   // FIFO Clock Edge Control <--

      spin_unlock_bh(&auddrv_lock);
      // turn on speaker
      Sound_Speaker_Turnon(Channel_Stereo);
      msleep(time); // sleep

   }
   // stop SineWave
   {
      spin_lock_bh(&auddrv_lock);
      // Protect: Because during bee period, Audio scenario maybe change (Ex: FM off, audio play off)
      // So, Need to add dome judgement to check the Audio scenario change.
      // Original I2S/AFE On --> during sleep period I2S/AFE Off, recovery may be turn on it again.
      if(( (Aud_I2S_Clk_cntr == 0) && (Aud_I2S_Clk_cntr < prev_i2s_clk ) )||
         ( (Aud_ADC_Clk_cntr == 0) && (Aud_ADC_Clk_cntr < prev_adc_clk ))
        )
      {
         if(Suspend_reg.Suspend_AFE_DAC_CON0 != Afe_Get_Reg(AFE_DAC_CON0))
         {
            Suspend_reg.Suspend_AFE_DAC_CON0 = Afe_Get_Reg(AFE_DAC_CON0);
         }
         if( Suspend_reg.Suspend_AFE_DL_SRC2_1 != Afe_Get_Reg(AFE_DL_SRC2_1) )
         {
            Suspend_reg.Suspend_AFE_DL_SRC2_1 = Afe_Get_Reg(AFE_DL_SRC2_1);
         }
      }
      // Protect: Because during bee period, Audio scenario maybe change (Ex: FM on, audio play on)
      // So, Need to add dome judgement to check the Audio scenario change.
      // Original I2S/AFE Off --> during sleep period I2S/AFE On, recovery may be turn off it again.
      if(( (prev_i2s_clk == 0) && (Aud_I2S_Clk_cntr > prev_i2s_clk )) ||
         ( (prev_adc_clk == 0) && (Aud_ADC_Clk_cntr > prev_adc_clk ))
        )
      {
         if(Suspend_reg.Suspend_AFE_DAC_CON0 != Afe_Get_Reg(AFE_DAC_CON0))
         {
            Suspend_reg.Suspend_AFE_DAC_CON0 = Afe_Get_Reg(AFE_DAC_CON0);
         }
         if( Suspend_reg.Suspend_AFE_DL_SRC2_1 != Afe_Get_Reg(AFE_DL_SRC2_1) )
         {
            Suspend_reg.Suspend_AFE_DL_SRC2_1 = Afe_Get_Reg(AFE_DL_SRC2_1);
         }
      }

      AudDrv_Recover_Reg_LouderSPKSound();
      spin_unlock_bh(&auddrv_lock);

      if(AMP_Flag== false){
         Sound_Speaker_Turnoff(Channel_Stereo);
      }
   }

   AudDrv_Clk_Off();
   printk("-AudDrv LouderSPKSound \n");
   return true;
}
EXPORT_SYMBOL(LouderSPKSound);

static void AudDrv_Store_reg(void)
{
	printk("+AudDrv_Store_reg \n");

   AudDrv_Clk_On();
   // Digital register setting
   Suspend_reg.Suspend_AFE_DAC_CON0    = Afe_Get_Reg(AFE_DAC_CON0);
   Suspend_reg.Suspend_AFE_DAC_CON1    = Afe_Get_Reg(AFE_DAC_CON1);
   Suspend_reg.Suspend_AFE_I2S_IN_CON  = Afe_Get_Reg(AFE_I2S_IN_CON);
   Suspend_reg.Suspend_AFE_FOC_CON     = Afe_Get_Reg(AFE_FOC_CON);
   Suspend_reg.Suspend_AFE_IRQ_CNT2    = Afe_Get_Reg(AFE_IRQ_CNT2);
   Suspend_reg.Suspend_AFE_DAIBT_CON   = Afe_Get_Reg(AFE_DAIBT_CON);
   Suspend_reg.Suspend_AFE_CONN0       = Afe_Get_Reg(AFE_CONN0);
   Suspend_reg.Suspend_AFE_CONN1       = Afe_Get_Reg(AFE_CONN1);
   Suspend_reg.Suspend_AFE_CONN2       = Afe_Get_Reg(AFE_CONN2);
   Suspend_reg.Suspend_AFE_CONN3       = Afe_Get_Reg(AFE_CONN3);
   Suspend_reg.Suspend_AFE_CONN4       = Afe_Get_Reg(AFE_CONN4);
   Suspend_reg.Suspend_AFE_I2S_OUT_CON = Afe_Get_Reg(AFE_I2S_OUT_CON);
   Suspend_reg.Suspend_AFE_DL1_BASE    = Afe_Get_Reg(AFE_DL1_BASE);
//   Suspend_reg.Suspend_AFE_DL1_CUR     = Afe_Get_Reg(AFE_DL1_CUR);
   Suspend_reg.Suspend_AFE_DL1_END     = Afe_Get_Reg(AFE_DL1_END);
   Suspend_reg.Suspend_AFE_DL2_BASE    = Afe_Get_Reg(AFE_DL2_BASE);
//   Suspend_reg.Suspend_AFE_DL2_CUR     = Afe_Get_Reg(AFE_DL2_CUR);
   Suspend_reg.Suspend_AFE_DL2_END     = Afe_Get_Reg(AFE_DL2_END);
   Suspend_reg.Suspend_AFE_I2S_BASE    = Afe_Get_Reg(AFE_I2S_BASE);
//   Suspend_reg.Suspend_AFE_I2S_CUR     = Afe_Get_Reg(AFE_I2S_CUR);
   Suspend_reg.Suspend_AFE_I2S_END     = Afe_Get_Reg(AFE_I2S_END);
   Suspend_reg.Suspend_AFE_AWB_BASE   = Afe_Get_Reg(AFE_AWB_BASE);
   Suspend_reg.Suspend_AFE_AWB_END	   = Afe_Get_Reg(AFE_AWB_END);

   Suspend_reg.Suspend_AFE_DL_SDM_CON0 = Afe_Get_Reg(AFE_DL_SDM_CON0);
   Suspend_reg.Suspend_AFE_DL_SRC2_1   = Afe_Get_Reg(AFE_DL_SRC2_1);
   Suspend_reg.Suspend_AFE_DL_SRC2_2   = Afe_Get_Reg(AFE_DL_SRC2_2);
   Suspend_reg.Suspend_AFE_SIDETONE_CON0 = Afe_Get_Reg(AFE_SIDETONE_CON0);
   Suspend_reg.Suspend_AFE_SIDETONE_CON1 = Afe_Get_Reg(AFE_SIDETONE_CON1);

   Suspend_reg.Suspend_AFE_UL_SRC_0      = Afe_Get_Reg(AFE_UL_SRC_0);
   Suspend_reg.Suspend_AFE_UL_SRC_1      = Afe_Get_Reg(AFE_UL_SRC_1);
   Suspend_reg.Suspend_AFE_TOP_CONTROL_0 = Afe_Get_Reg(AFE_TOP_CONTROL_0);

   Suspend_reg.Suspend_AFE_SDM_GAIN_STAGE = Afe_Get_Reg(AFE_SDM_GAIN_STAGE);


   // Analog register setting
   Suspend_reg.Suspend_PLL_CON2   = Ana_Get_Reg(PLL_CON2);
   Suspend_reg.Suspend_AUDIO_CON0 = Ana_Get_Reg(AUDIO_CON0);
   Suspend_reg.Suspend_AUDIO_CON1 = Ana_Get_Reg(AUDIO_CON1);
   Suspend_reg.Suspend_AUDIO_CON2 = Ana_Get_Reg(AUDIO_CON2);
   Suspend_reg.Suspend_AUDIO_CON3 = Ana_Get_Reg(AUDIO_CON3);
   Suspend_reg.Suspend_AUDIO_CON4 = Ana_Get_Reg(AUDIO_CON4);
   Suspend_reg.Suspend_AUDIO_CON5 = Ana_Get_Reg(AUDIO_CON5);
   Suspend_reg.Suspend_AUDIO_CON6 = Ana_Get_Reg(AUDIO_CON6);
   Suspend_reg.Suspend_AUDIO_CON7 = Ana_Get_Reg(AUDIO_CON7);
   Suspend_reg.Suspend_AUDIO_CON8 = Ana_Get_Reg(AUDIO_CON8);
   Suspend_reg.Suspend_AUDIO_CON9 = Ana_Get_Reg(AUDIO_CON9);
   Suspend_reg.Suspend_AUDIO_CON10 = Ana_Get_Reg(AUDIO_CON10);

   Suspend_reg.Suspend_AUDIO_CON20 = Ana_Get_Reg(AUDIO_CON20);
   Suspend_reg.Suspend_AUDIO_CON21 = Ana_Get_Reg(AUDIO_CON21);
   Suspend_reg.Suspend_AUDIO_CON22 = Ana_Get_Reg(AUDIO_CON22);
   Suspend_reg.Suspend_AUDIO_CON23 = Ana_Get_Reg(AUDIO_CON23);
   Suspend_reg.Suspend_AUDIO_CON24 = Ana_Get_Reg(AUDIO_CON24);
   Suspend_reg.Suspend_AUDIO_CON25 = Ana_Get_Reg(AUDIO_CON25);
   Suspend_reg.Suspend_AUDIO_CON26 = Ana_Get_Reg(AUDIO_CON26);
   Suspend_reg.Suspend_AUDIO_CON27 = Ana_Get_Reg(AUDIO_CON27);
   Suspend_reg.Suspend_AUDIO_CON28 = Ana_Get_Reg(AUDIO_CON28);
   Suspend_reg.Suspend_AUDIO_CON29 = Ana_Get_Reg(AUDIO_CON29);
   Suspend_reg.Suspend_AUDIO_CON30 = Ana_Get_Reg(AUDIO_CON30);
   Suspend_reg.Suspend_AUDIO_CON31 = Ana_Get_Reg(AUDIO_CON31);
   Suspend_reg.Suspend_AUDIO_CON32 = Ana_Get_Reg(AUDIO_CON32);
//   Suspend_reg.Suspend_ACIF_WR_PATH = Ana_Get_Reg(ACIF_WR_PATH);

//   Afe_Set_Reg(AFE_DAC_CON0,0x0,0xffffffff); // turn off all module
   Afe_Set_Reg(AFE_IR_CLR,0x001f,0x00ff);    // clear interrupt
   mdelay(5);                                // wait for a shor latency
   Afe_Set_Reg(AFE_IR_CLR,0x001f,0x00ff);    // clear interrupt

   AudDrv_Clk_Off();
	PRINTK_AUDDRV("-AudDrv_Store_reg \n");
}

static void AudDrv_Store_reg_AFE(void)
{
	//printk("+AudDrv_Store_reg_AFE \n");

   // Digital register setting
   Suspend_reg.Suspend_AFE_DAC_CON0    = Afe_Get_Reg(AFE_DAC_CON0);
   Suspend_reg.Suspend_AFE_DAC_CON1    = Afe_Get_Reg(AFE_DAC_CON1);
   Suspend_reg.Suspend_AFE_I2S_IN_CON  = Afe_Get_Reg(AFE_I2S_IN_CON);
   Suspend_reg.Suspend_AFE_FOC_CON     = Afe_Get_Reg(AFE_FOC_CON);
   Suspend_reg.Suspend_AFE_IRQ_CNT2    = Afe_Get_Reg(AFE_IRQ_CNT2);
   Suspend_reg.Suspend_AFE_DAIBT_CON   = Afe_Get_Reg(AFE_DAIBT_CON);
   Suspend_reg.Suspend_AFE_CONN0       = Afe_Get_Reg(AFE_CONN0);
   Suspend_reg.Suspend_AFE_CONN1       = Afe_Get_Reg(AFE_CONN1);
   Suspend_reg.Suspend_AFE_CONN2       = Afe_Get_Reg(AFE_CONN2);
   Suspend_reg.Suspend_AFE_CONN3       = Afe_Get_Reg(AFE_CONN3);
   Suspend_reg.Suspend_AFE_CONN4       = Afe_Get_Reg(AFE_CONN4);
   Suspend_reg.Suspend_AFE_I2S_OUT_CON = Afe_Get_Reg(AFE_I2S_OUT_CON);
   Suspend_reg.Suspend_AFE_DL1_BASE    = Afe_Get_Reg(AFE_DL1_BASE);
//   Suspend_reg.Suspend_AFE_DL1_CUR     = Afe_Get_Reg(AFE_DL1_CUR);
   Suspend_reg.Suspend_AFE_DL1_END     = Afe_Get_Reg(AFE_DL1_END);
   Suspend_reg.Suspend_AFE_DL2_BASE    = Afe_Get_Reg(AFE_DL2_BASE);
//   Suspend_reg.Suspend_AFE_DL2_CUR     = Afe_Get_Reg(AFE_DL2_CUR);
   Suspend_reg.Suspend_AFE_DL2_END     = Afe_Get_Reg(AFE_DL2_END);
   Suspend_reg.Suspend_AFE_I2S_BASE    = Afe_Get_Reg(AFE_I2S_BASE);
//   Suspend_reg.Suspend_AFE_I2S_CUR     = Afe_Get_Reg(AFE_I2S_CUR);
   Suspend_reg.Suspend_AFE_I2S_END     = Afe_Get_Reg(AFE_I2S_END);
   Suspend_reg.Suspend_AFE_AWB_BASE    = Afe_Get_Reg(AFE_AWB_BASE);
   Suspend_reg.Suspend_AFE_AWB_END     = Afe_Get_Reg(AFE_AWB_END);
   Suspend_reg.Suspend_AFE_DL_SDM_CON0 = Afe_Get_Reg(AFE_DL_SDM_CON0);
   Suspend_reg.Suspend_AFE_DL_SRC2_1   = Afe_Get_Reg(AFE_DL_SRC2_1);
   Suspend_reg.Suspend_AFE_DL_SRC2_2   = Afe_Get_Reg(AFE_DL_SRC2_2);
   Suspend_reg.Suspend_AFE_SIDETONE_CON0 = Afe_Get_Reg(AFE_SIDETONE_CON0);
   Suspend_reg.Suspend_AFE_SIDETONE_CON1 = Afe_Get_Reg(AFE_SIDETONE_CON1);

   Suspend_reg.Suspend_AFE_UL_SRC_0      = Afe_Get_Reg(AFE_UL_SRC_0);
   Suspend_reg.Suspend_AFE_UL_SRC_1      = Afe_Get_Reg(AFE_UL_SRC_1);
   Suspend_reg.Suspend_AFE_TOP_CONTROL_0 = Afe_Get_Reg(AFE_TOP_CONTROL_0);

   Suspend_reg.Suspend_AFE_SDM_GAIN_STAGE = Afe_Get_Reg(AFE_SDM_GAIN_STAGE);

   Afe_Set_Reg(AFE_IR_CLR,0x001f,0x00ff);    // clear interrupt
   mdelay(5);                                // wait for a shor latency
   Afe_Set_Reg(AFE_IR_CLR,0x001f,0x00ff);    // clear interrupt

	PRINTK_AUDDRV("-AudDrv_Store_reg_AFE \n");
}

static void AudDrv_Recover_reg(void)
{
	printk("+AudDrv_Recover_reg \n");
   AudDrv_Clk_On();
   // Digital register setting
   Afe_Set_Reg(AFE_DL1_BASE,Suspend_reg.Suspend_AFE_DL1_BASE, MASK_ALL);
   Afe_Set_Reg(AFE_DL1_END,Suspend_reg.Suspend_AFE_DL1_END, MASK_ALL);
   Afe_Set_Reg(AFE_DL2_BASE,Suspend_reg.Suspend_AFE_DL2_BASE, MASK_ALL);
   Afe_Set_Reg(AFE_DL2_END,Suspend_reg.Suspend_AFE_DL2_END, MASK_ALL);
   Afe_Set_Reg(AFE_I2S_BASE,Suspend_reg.Suspend_AFE_I2S_BASE, MASK_ALL);
   Afe_Set_Reg(AFE_I2S_END,Suspend_reg.Suspend_AFE_I2S_END, MASK_ALL);
   Afe_Set_Reg(AFE_AWB_BASE,Suspend_reg.Suspend_AFE_AWB_BASE, MASK_ALL);
   Afe_Set_Reg(AFE_AWB_END,Suspend_reg.Suspend_AFE_AWB_END, MASK_ALL);


   Afe_Set_Reg(AFE_DAC_CON1,Suspend_reg.Suspend_AFE_DAC_CON1, MASK_ALL);
   Afe_Set_Reg(AFE_I2S_IN_CON,Suspend_reg.Suspend_AFE_I2S_IN_CON, MASK_ALL);
   Afe_Set_Reg(AFE_FOC_CON,Suspend_reg.Suspend_AFE_FOC_CON, MASK_ALL);
   Afe_Set_Reg(AFE_IRQ_CNT2,Suspend_reg.Suspend_AFE_IRQ_CNT2, MASK_ALL);
   Afe_Set_Reg(AFE_DAIBT_CON,Suspend_reg.Suspend_AFE_DAIBT_CON, MASK_ALL);
   Afe_Set_Reg(AFE_CONN0,Suspend_reg.Suspend_AFE_CONN0, MASK_ALL);
   Afe_Set_Reg(AFE_CONN1,Suspend_reg.Suspend_AFE_CONN1, MASK_ALL);
   Afe_Set_Reg(AFE_CONN2,Suspend_reg.Suspend_AFE_CONN2, MASK_ALL);
   Afe_Set_Reg(AFE_CONN3,Suspend_reg.Suspend_AFE_CONN3, MASK_ALL);
   Afe_Set_Reg(AFE_CONN4,Suspend_reg.Suspend_AFE_CONN4, MASK_ALL);
   Afe_Set_Reg(AFE_DAC_CON0,Suspend_reg.Suspend_AFE_DAC_CON0, MASK_ALL);
   Afe_Set_Reg(AFE_I2S_OUT_CON,Suspend_reg.Suspend_AFE_I2S_OUT_CON, MASK_ALL);

   Afe_Set_Reg(AFE_DL_SDM_CON0,Suspend_reg.Suspend_AFE_DL_SDM_CON0, MASK_ALL);
   Afe_Set_Reg(AFE_DL_SRC2_1,Suspend_reg.Suspend_AFE_DL_SRC2_1, MASK_ALL);
   Afe_Set_Reg(AFE_DL_SRC2_2,Suspend_reg.Suspend_AFE_DL_SRC2_2, MASK_ALL);
   Afe_Set_Reg(AFE_SIDETONE_CON0,Suspend_reg.Suspend_AFE_SIDETONE_CON0, MASK_ALL);
   Afe_Set_Reg(AFE_SIDETONE_CON1,Suspend_reg.Suspend_AFE_SIDETONE_CON1, MASK_ALL);

   Afe_Set_Reg(AFE_UL_SRC_0,Suspend_reg.Suspend_AFE_UL_SRC_0, MASK_ALL);
   Afe_Set_Reg(AFE_UL_SRC_1,Suspend_reg.Suspend_AFE_UL_SRC_1, MASK_ALL);
   Afe_Set_Reg(AFE_TOP_CONTROL_0,Suspend_reg.Suspend_AFE_TOP_CONTROL_0, MASK_ALL);

   Afe_Set_Reg(AFE_SDM_GAIN_STAGE,Suspend_reg.Suspend_AFE_SDM_GAIN_STAGE, MASK_ALL);

   // Analog register setting
   Ana_Set_Reg(PLL_CON2,Suspend_reg.Suspend_PLL_CON2, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON0,Suspend_reg.Suspend_AUDIO_CON0, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON1,Suspend_reg.Suspend_AUDIO_CON1, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON2,Suspend_reg.Suspend_AUDIO_CON2, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON3,Suspend_reg.Suspend_AUDIO_CON3, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON4,Suspend_reg.Suspend_AUDIO_CON4, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON5,Suspend_reg.Suspend_AUDIO_CON5, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON6,Suspend_reg.Suspend_AUDIO_CON6, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON7,Suspend_reg.Suspend_AUDIO_CON7, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON8,Suspend_reg.Suspend_AUDIO_CON8, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON9,Suspend_reg.Suspend_AUDIO_CON9, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON10,Suspend_reg.Suspend_AUDIO_CON10, MASK_ALL);

   Ana_Set_Reg(AUDIO_CON20,Suspend_reg.Suspend_AUDIO_CON20, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON21,Suspend_reg.Suspend_AUDIO_CON21, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON22,Suspend_reg.Suspend_AUDIO_CON22, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON23,Suspend_reg.Suspend_AUDIO_CON23, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON24,Suspend_reg.Suspend_AUDIO_CON24, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON25,Suspend_reg.Suspend_AUDIO_CON25, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON26,Suspend_reg.Suspend_AUDIO_CON26, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON27,Suspend_reg.Suspend_AUDIO_CON27, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON28,Suspend_reg.Suspend_AUDIO_CON28, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON29,Suspend_reg.Suspend_AUDIO_CON29, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON30,Suspend_reg.Suspend_AUDIO_CON30, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON31,Suspend_reg.Suspend_AUDIO_CON31, MASK_ALL);
   Ana_Set_Reg(AUDIO_CON32,Suspend_reg.Suspend_AUDIO_CON32, MASK_ALL);
//   Ana_Set_Reg(ACIF_WR_PATH,Suspend_reg.Suspend_ACIF_WR_PATH, MASK_ALL);

   AudDrv_Clk_Off();

	PRINTK_AUDDRV("-AudDrv_Recover_reg \n");
}

static void AudDrv_Recover_reg_AFE(void)
{
	//printk("+AudDrv_Recover_reg_AFE \n");
   // Digital register setting
   Afe_Set_Reg(AFE_DL1_BASE,Suspend_reg.Suspend_AFE_DL1_BASE, MASK_ALL);
   Afe_Set_Reg(AFE_DL1_END,Suspend_reg.Suspend_AFE_DL1_END, MASK_ALL);
   Afe_Set_Reg(AFE_DL2_BASE,Suspend_reg.Suspend_AFE_DL2_BASE, MASK_ALL);
   Afe_Set_Reg(AFE_DL2_END,Suspend_reg.Suspend_AFE_DL2_END, MASK_ALL);
   Afe_Set_Reg(AFE_I2S_BASE,Suspend_reg.Suspend_AFE_I2S_BASE, MASK_ALL);
   Afe_Set_Reg(AFE_I2S_END,Suspend_reg.Suspend_AFE_I2S_END, MASK_ALL);
   Afe_Set_Reg(AFE_AWB_BASE,Suspend_reg.Suspend_AFE_AWB_BASE, MASK_ALL);
   Afe_Set_Reg(AFE_AWB_END,Suspend_reg.Suspend_AFE_AWB_END, MASK_ALL);

   Afe_Set_Reg(AFE_DAC_CON1,Suspend_reg.Suspend_AFE_DAC_CON1, MASK_ALL);
   Afe_Set_Reg(AFE_I2S_IN_CON,Suspend_reg.Suspend_AFE_I2S_IN_CON, MASK_ALL);
   Afe_Set_Reg(AFE_FOC_CON,Suspend_reg.Suspend_AFE_FOC_CON, MASK_ALL);
   Afe_Set_Reg(AFE_IRQ_CNT2,Suspend_reg.Suspend_AFE_IRQ_CNT2, MASK_ALL);
   Afe_Set_Reg(AFE_DAIBT_CON,Suspend_reg.Suspend_AFE_DAIBT_CON, MASK_ALL);
   Afe_Set_Reg(AFE_CONN0,Suspend_reg.Suspend_AFE_CONN0, MASK_ALL);
   Afe_Set_Reg(AFE_CONN1,Suspend_reg.Suspend_AFE_CONN1, MASK_ALL);
   Afe_Set_Reg(AFE_CONN2,Suspend_reg.Suspend_AFE_CONN2, MASK_ALL);
   Afe_Set_Reg(AFE_CONN3,Suspend_reg.Suspend_AFE_CONN3, MASK_ALL);
   Afe_Set_Reg(AFE_CONN4,Suspend_reg.Suspend_AFE_CONN4, MASK_ALL);
   Afe_Set_Reg(AFE_DAC_CON0,Suspend_reg.Suspend_AFE_DAC_CON0, MASK_ALL);
   Afe_Set_Reg(AFE_I2S_OUT_CON,Suspend_reg.Suspend_AFE_I2S_OUT_CON, MASK_ALL);

   Afe_Set_Reg(AFE_DL_SDM_CON0,Suspend_reg.Suspend_AFE_DL_SDM_CON0, MASK_ALL);
   Afe_Set_Reg(AFE_DL_SRC2_1,Suspend_reg.Suspend_AFE_DL_SRC2_1, MASK_ALL);
   Afe_Set_Reg(AFE_DL_SRC2_2,Suspend_reg.Suspend_AFE_DL_SRC2_2, MASK_ALL);
   Afe_Set_Reg(AFE_SIDETONE_CON0,Suspend_reg.Suspend_AFE_SIDETONE_CON0, MASK_ALL);
   Afe_Set_Reg(AFE_SIDETONE_CON1,Suspend_reg.Suspend_AFE_SIDETONE_CON1, MASK_ALL);

   Afe_Set_Reg(AFE_UL_SRC_0,Suspend_reg.Suspend_AFE_UL_SRC_0, MASK_ALL);
   Afe_Set_Reg(AFE_UL_SRC_1,Suspend_reg.Suspend_AFE_UL_SRC_1, MASK_ALL);
   Afe_Set_Reg(AFE_TOP_CONTROL_0,Suspend_reg.Suspend_AFE_TOP_CONTROL_0, MASK_ALL);

   Afe_Set_Reg(AFE_SDM_GAIN_STAGE,Suspend_reg.Suspend_AFE_SDM_GAIN_STAGE, MASK_ALL);

	PRINTK_AUDDRV("-AudDrv_Recover_reg_AFE \n");
}


bool Auddrv_Check_Afe_Clock(void)
{
    kal_uint32 aud_top = Afe_Get_Reg(AUDIO_TOP_CON0);
    kal_uint32 afe_clk = (aud_top & 0x4);
    if(afe_clk == 0x4){
        return true;
    }
    else{
        return false;
    }
}

void Auddrv_AWB_timer_Routine(unsigned long data)
{
    AFE_BLOCK_T *AWB_Block = &AWB_input_Control->rBlock;
    kal_uint32 AWB_Base =Afe_Get_Reg(AFE_AWB_BASE);
    volatile kal_uint32 AWB_Current = Afe_Get_Reg(AFE_AWB_CUR);
    volatile kal_uint32 AWB_Input_Index = AWB_Current - AWB_Base;
    volatile kal_uint32 AWB_Input_Size =0;

    printk("Auddrv_AWB_timer_Routine \n");

    if(AWB_Input_Index >AWB_Block->u4BufferSize){
        printk("error !! AWB_Input_Size = %d> u4BufferSize = %d\n",AWB_Input_Index,AWB_Block->u4BufferSize);
        mod_timer(&AWB_timer, jiffies+msecs_to_jiffies(AWB_TIMER_INTERVAL));
        return;
    }
    spin_lock_bh(&auddrv_lock);
    printk("1: AWB_Base = %x AWB_Current = %x AWB_Input_Index = %d\n",AWB_Base,AWB_Current,AWB_Input_Index);
    if(AWB_Input_Index > AWB_Block->u4WriteIdx){
        printk("1: u4DataRemained = %d AWB_Block->u4WriteIdx = %d\n",AWB_Block->u4DataRemained,AWB_Block->u4WriteIdx);
        AWB_Input_Size = AWB_Input_Index - AWB_Block->u4WriteIdx;
        AWB_Block->u4DataRemained +=AWB_Input_Size;
        AWB_Block->u4WriteIdx += AWB_Input_Size;
        if(AWB_Block->u4WriteIdx >= AWB_Block->u4BufferSize){
            AWB_Block->u4WriteIdx -= AWB_Block->u4BufferSize;
        }
        if(AWB_Block->u4DataRemained > AWB_Block->u4BufferSize){
            AWB_Block->u4DataRemained -= AWB_Block->u4BufferSize;
        }
        printk("1: u4DataRemained = %d AWB_Block->u4WriteIdx = %d\n",AWB_Block->u4DataRemained,AWB_Block->u4WriteIdx);
    }
    else{
        kal_uint32 Copy_Size1 =  AWB_Block->u4BufferSize -AWB_Block->u4WriteIdx;
        kal_uint32 Copy_Size2 = AWB_Input_Index;
        AWB_Input_Size = Copy_Size1 + Copy_Size2;
        printk("2: u4DataRemained = %d AWB_Block->u4WriteIdx = %d\n",AWB_Block->u4DataRemained,AWB_Block->u4WriteIdx);
        AWB_Block->u4DataRemained += AWB_Input_Size;
        AWB_Block->u4WriteIdx += AWB_Input_Size;
        if(AWB_Block->u4WriteIdx >= AWB_Block->u4BufferSize){
            AWB_Block->u4WriteIdx -= AWB_Block->u4BufferSize;
        }
        if(AWB_Block->u4DataRemained > AWB_Block->u4BufferSize){
            AWB_Block->u4DataRemained -= AWB_Block->u4BufferSize;
        }
        printk("2: u4DataRemained = %d AWB_Block->u4WriteIdx = %d\n",AWB_Block->u4DataRemained,AWB_Block->u4WriteIdx);
    }
    AWB_wait_queue_flag =1;
    spin_unlock_bh(&auddrv_lock);
    wake_up_interruptible(&AWB_Wait_Queue);
    mod_timer(&AWB_timer, jiffies+msecs_to_jiffies(AWB_TIMER_INTERVAL));
}

// Timer nned to be protect, add_timer twice wiil cause kernel panic
int Auddrv_AWB_timer_on(void)
{
    printk("Auddrv_AWB_timer_on is called! \n");
    if(AWB_Timer_Flag == false){
        AWB_timer.expires = jiffies + msecs_to_jiffies(AWB_TIMER_INTERVAL);
        AWB_timer.data = 0;
        AWB_timer.function = Auddrv_AWB_timer_Routine;
        add_timer(&AWB_timer);
        AWB_Timer_Flag = true;
    }
    return 0;
}

int Auddrv_AWB_timer_off(void)
{
    printk("Auddrv_AWB_timer_off is called! \n");
    if(AWB_Timer_Flag == true){
        del_timer_sync(&AWB_timer);
        AWB_Timer_Flag = false;
    }
    return 0;
}



void AudDrv_allcate_AWB_buffer()
{
   AFE_BLOCK_T *AWB_Block = NULL;
   printk("AudDrv_allcate_AWB_buffer\n");
   AWB_Block   = &(AWB_input_Control->rBlock);
   if(AWB_Block->pucPhysBufAddr == 0){
       //allocate memory for AWB
       AWB_Block->pucVirtBufAddr = dma_alloc_coherent(0,AWB_BUF_LENGTH,&AWB_Block->pucPhysBufAddr,GFP_KERNEL);
       if((0 == AWB_Block->pucPhysBufAddr)||(NULL == AWB_Block->pucVirtBufAddr)){
          printk("AudDrv_allcate_AWB_buffer dma_alloc_coherent fail \n");
          return;
       }
       AWB_input_Control->u4BufferSize = AWB_BUF_LENGTH;
       AWB_Block->u4BufferSize = AWB_BUF_LENGTH;
       if(AWB_Block->pucVirtBufAddr && Auddrv_Check_Afe_Clock() ){
           memset((void*)AWB_Block->pucVirtBufAddr,0,AWB_Block->u4BufferSize);
       }
       AWB_Block->u4DMAReadIdx =0;
       AWB_Block->u4DataRemained =0;
       AWB_Block->u4WriteIdx =0;
       printk("AWB_Block->pucVirtBufAddr = %p  AWB_Block->pucPhysBufAddr = 0x%x\n" ,
           AWB_Block->pucVirtBufAddr, AWB_Block->pucPhysBufAddr);
   }
}


void AudDrv_Start_AWB_Stream(struct file *fp)
{
    AFE_BLOCK_T *AWB_Block = &AWB_input_Control->rBlock;
    printk("AudDrv_Start_AWB_Stream  fp = %p\n", fp);
    if(AWB_Block->pucPhysBufAddr && AWB_Block->pucVirtBufAddr)
    {
        Afe_Set_Reg(AFE_AWB_BASE,AWB_Block->pucPhysBufAddr,0xffffffff);
        Afe_Set_Reg(AFE_AWB_END, (AWB_Block->pucPhysBufAddr + AWB_Block->u4BufferSize -1),0xffffffff);
    }
    spin_lock_bh(&auddrv_lock);
    Aud_AWB_Clk_cntr++;
    spin_unlock_bh(&auddrv_lock);
    if(Aud_AWB_Clk_cntr ==1){
        printk("AudDrv_Start_AWB_Stream Aud_AWB_Clk_cntr = %d\n",Aud_AWB_Clk_cntr);
        AudDrv_Clk_On();
        AudDrv_ADC_Clk_On();
    }
    printk("AWB_Block->pucVirtBufAddr = %p  AWB_Block->pucPhysBufAddr = %x\n",
           AWB_Block->pucVirtBufAddr, AWB_Block->pucPhysBufAddr);
    AWB_Block->flip = fp;
}

void AudDrv_Standby_AWB_Stream(struct file *fp)
{
    AFE_BLOCK_T *AWB_Block = &AWB_input_Control->rBlock;
    printk("AudDrv_Standby_AWB_Stream fp = %p\n", fp);
    spin_lock_bh(&auddrv_lock);
    Aud_AWB_Clk_cntr--;
    spin_unlock_bh(&auddrv_lock);
    if(Aud_AWB_Clk_cntr == 0){
        printk("AudDrv_Standby_AWB_Stream Aud_AWB_Clk_cntr = %d", Aud_AWB_Clk_cntr);
        AudDrv_Clk_Off();
        AudDrv_ADC_Clk_Off();
    }
    if(AWB_Block->pucVirtBufAddr && Auddrv_Check_Afe_Clock()){
        memset((void*)AWB_Block->pucVirtBufAddr, 0, AWB_Block->u4BufferSize);
    }
    AWB_Block->u4DMAReadIdx =0;
    AWB_Block->u4DataRemained =0;
    AWB_Block->u4WriteIdx =0;
    AWB_Block->flip = NULL;
}

void AudDrv_Reset_AWB_Stream(void)
{
    AFE_BLOCK_T *AWB_Block = &AWB_input_Control->rBlock;
    printk("AudDrv_Reset_AWB_Stream \n");
    spin_lock_bh(&auddrv_lock);
    while (Aud_AWB_Clk_cntr == 0){
        printk("AudDrv_Standby_AWB_Stream Aud_AWB_Clk_cntr = %d",Aud_AWB_Clk_cntr);
        AudDrv_Clk_Off();
        AudDrv_ADC_Clk_Off();
        Aud_AWB_Clk_cntr --;
    }
    spin_unlock_bh(&auddrv_lock);
    AWB_Block->u4DMAReadIdx =0;
    AWB_Block->u4DataRemained =0;
    AWB_Block->u4WriteIdx =0;
    AWB_Block->flip = NULL;
}

void AudDrv_Clk_On(void)
{
   spin_lock(&auddrv_lock);
   if(Aud_AFE_Clk_cntr == 0 )
   {
      PRINTK_AUDDRV("+AudDrv_Clk_On, Aud_AFE_Clk_cntr:%d \n", Aud_AFE_Clk_cntr);
#ifdef USE_PM_API
      if(hwEnableClock(MT65XX_PDN_AUDIO_AFE,"AUDIO")==false){
         printk("Aud hwEnableClock MT65XX_PDN_AUDIO_AFE fail !!!\n");
      }
      Afe_Set_Reg(AUDIO_TOP_CON0, 0x0, 0x00010000);  // bit16: power on AFE_CK_DIV_RST

      if (!Flag_AudDrv_ClkOn_1st)
      {
         AudDrv_Recover_reg_AFE();
      }
#else
      Afe_Set_Reg(AUDIO_TOP_CON0, 0x80000008, 0x80000008);
#endif
      // Enable AFE clock
      if(!Flag_Aud_26MClkOn)
      {
         Ana_Set_Reg(PLL_CON2,0x20,0x20);             // turn on AFE (26M Clock)
      }
      Ana_Set_Reg(AUDIO_CON3,0x0,0x2);             // Enable Audio Bias (VaudlbiasDistrib)

   }
   else{
      kal_uint32 aud_top = Afe_Get_Reg(AUDIO_TOP_CON0);
      kal_uint32 afe_clk = (aud_top & 0x4);
      if(afe_clk == 0x4){
         printk("+AudDrv_Clk_On AFE clk(%d)(%x)\n",Aud_AFE_Clk_cntr,afe_clk);
      }
   }

   Aud_AFE_Clk_cntr++;
   spin_unlock(&auddrv_lock);

   PRINTK_AUDDRV("!! AudDrv_Clk_On, bSpeechFlag:%d, bRecordFlag:%d, bBgsFlag:%d, bVT:%d \n",
   SPH_Ctrl_State.bSpeechFlag,SPH_Ctrl_State.bRecordFlag,SPH_Ctrl_State.bBgsFlag,SPH_Ctrl_State.bVT);

   PRINTK_AUDDRV("-AudDrv_Clk_On, Aud_AFE_Clk_cntr:%d \n",Aud_AFE_Clk_cntr);
}

void AudDrv_Clk_Off(void)
{
   PRINTK_AUDDRV("+!! AudDrv_Clk_Off, Aud_AFE_Clk_cntr:%d \n",Aud_AFE_Clk_cntr);

   spin_lock(&auddrv_lock);
   Aud_AFE_Clk_cntr--;
   if(Aud_AFE_Clk_cntr == 0)
   {
      PRINTK_AUDDRV("+ AudDrv_Clk_Off, Aud_AFE_Clk_cntr:%d \n",Aud_AFE_Clk_cntr);
      {
         // Disable AFE clock
         Ana_Set_Reg(AUDIO_CON3,0x2,0x2);       // Disable Audio Bias (VaudlbiasDistrib power down)
#ifdef USE_PM_API
         AudDrv_Store_reg_AFE();
         Afe_Set_Reg(AUDIO_TOP_CON0, 0x00010000, 0x00010000);  // bit16: power off AFE_CK_DIV_RST

         if(hwDisableClock(MT65XX_PDN_AUDIO_AFE,"AUDIO")==false){
            printk("hwDisableClock MT65XX_PDN_AUDIO_AFE fail");
         }
#else
         Afe_Set_Reg(AUDIO_TOP_CON0, 0x10074, 0x10074);   // bit2: power down AFE clock
#endif
      }
   }
   if(Aud_AFE_Clk_cntr < 0){
      PRINTK_AUDDRV("!! AudDrv_Clk_Off, Aud_AFE_Clk_cntr<0 (%d) \n",Aud_AFE_Clk_cntr);
      Aud_AFE_Clk_cntr =0;
   }
   spin_unlock(&auddrv_lock);

   PRINTK_AUDDRV("-!! AudDrv_Clk_Off, Aud_AFE_Clk_cntr:%d \n",Aud_AFE_Clk_cntr);
}

void AudDrv_I2S_Clk_On(void)
{
   kal_uint32 aud_top, i2s_clk;
   PRINTK_AUDDRV("+AudDrv_I2S_Clk_On, Aud_I2S_Clk_cntr:%d \n", Aud_I2S_Clk_cntr);
   spin_lock(&auddrv_lock);
   if(Aud_I2S_Clk_cntr == 0 )
   {
      // Now, for FM playback, no module would set Power Manager as "leave deep idle" mode.
      // Original, UART3 would force system NOT in deep idle mode. But WCN remove this(for BT+audio low power consideration)
      // for FM playback, Audio driver would use non-catchable DRAM or internal SRAM. So the Bus clk clock can't be forced off.
      // But when system enter deep idle, the Bus clk would be force off.
      // (After direct connect I2S-->DL1, this function can be removed.)
      // leave deep idle
      clr_device_working_ability(MT65XX_PDN_AUDIO_I2S, DEEP_IDLE_STATE);
      aud_top = Afe_Get_Reg(AUDIO_TOP_CON0);
      i2s_clk = (aud_top & 0x40);
      PRINTK_AUDDRV("+Aud hwEnableClock I2S clk(%x)\n",i2s_clk);
      if(hwEnableClock(MT65XX_PDN_AUDIO_I2S, "AUDIO")==false){
         printk("Aud hwEnableClock MT65XX_PDN_AUDIO_I2S fail !!!\n");
      }
   }
   Aud_I2S_Clk_cntr++;
   spin_unlock(&auddrv_lock);
}

void AudDrv_I2S_Clk_Off(void)
{
   kal_uint32 aud_top, i2s_clk;
   PRINTK_AUDDRV("+AudDrv_I2S_Clk_Off, Aud_I2S_Clk_cntr:%d \n", Aud_I2S_Clk_cntr);
   spin_lock(&auddrv_lock);
   Aud_I2S_Clk_cntr--;
   if(Aud_I2S_Clk_cntr == 0)
   {
      aud_top = Afe_Get_Reg(AUDIO_TOP_CON0);
      i2s_clk = (aud_top & 0x40);
      PRINTK_AUDDRV("+Aud hwDisableClock I2S clk(%x)\n",i2s_clk);
      if(hwDisableClock(MT65XX_PDN_AUDIO_I2S, "AUDIO")==false){
         printk("hwDisableClock MT65XX_PDN_AUDIO_I2S fail");
      }
      // enter deep idle
      set_device_working_ability(MT65XX_PDN_AUDIO_I2S, DEEP_IDLE_STATE);
   }
   if(Aud_I2S_Clk_cntr < 0){
      PRINTK_AUDDRV("!! AudDrv_I2S_Clk_Off, Aud_I2S_Clk_cntr<0 (%d) \n", Aud_I2S_Clk_cntr);
      Aud_I2S_Clk_cntr =0;
   }
   spin_unlock(&auddrv_lock);

   PRINTK_AUDDRV("-AudDrv_I2S_Clk_Off, Aud_I2S_Clk_cntr:%d \n", Aud_I2S_Clk_cntr);
}
void AudDrv_ADC_Clk_On(void)
{
   kal_uint32 aud_top, adc_clk;
   PRINTK_AUDDRV("+AudDrv_ADC_Clk_On, Aud_ADC_Clk_cntr:%d \n", Aud_ADC_Clk_cntr);
   spin_lock(&auddrv_lock);
   if(Aud_ADC_Clk_cntr == 0 )
   {
      aud_top = Afe_Get_Reg(AUDIO_TOP_CON0);
      adc_clk = (aud_top & 0x20);
      PRINTK_AUDDRV("+Aud hwEnableClock ADC clk(%x)\n",adc_clk);
      if(hwEnableClock(MT65XX_PDN_AUDIO_ADC,"AUDIO")==false){
         printk("Aud hwEnableClock MT65XX_PDN_AUDIO_ADC fail !!!\n");
      }
   }
   Aud_ADC_Clk_cntr++;
   spin_unlock(&auddrv_lock);
}

void AudDrv_ADC_Clk_Off(void)
{
   kal_uint32 aud_top, adc_clk;
   PRINTK_AUDDRV("+AudDrv_ADC_Clk_Off, Aud_ADC_Clk_cntr:%d \n", Aud_ADC_Clk_cntr);
   spin_lock(&auddrv_lock);
   Aud_ADC_Clk_cntr--;

   if(Aud_ADC_Clk_cntr == 0)
   {
      aud_top = Afe_Get_Reg(AUDIO_TOP_CON0);
      adc_clk = (aud_top & 0x20);
      PRINTK_AUDDRV("+Aud hwDisableClock ADC clk(%x)\n",adc_clk);
      if(hwDisableClock(MT65XX_PDN_AUDIO_ADC, "AUDIO")==false){
         printk("hwDisableClock MT65XX_PDN_AUDIO_ADC fail");
      }
   }
   if(Aud_ADC_Clk_cntr < 0){
      PRINTK_AUDDRV("!! AudDrv_ADC_Clk_Off, Aud_ADC_Clk_cntr<0 (%d) \n", Aud_ADC_Clk_cntr);
      Aud_ADC_Clk_cntr =0;
   }
   spin_unlock(&auddrv_lock);

   PRINTK_AUDDRV("-AudDrv_ADC_Clk_Off, Aud_ADC_Clk_cntr:%d \n", Aud_ADC_Clk_cntr);
}

void AudDrv_DL1_Stream_Standby(struct file *fp,unsigned long arg)
{

   kal_uint32 aud_top, afe_clk;
   PRINTK_AUDDRV("+AudDrv_DL1_Stream_Standby \n");

   spin_lock(&auddrv_lock);

   // after discuss with HW designer.
   // System hang for this case: When AFE clk is off and CPU want to write/read AFE internal memory.
   // (because AFE internal memory clk should sync with bus clk)
   // This is HW known issue. Designer(Chiahung) would fix it for later HW.
   // Workaround solution:
   // Before read/write AFE internal memory, check the afe clk.
   // If the afe clk is off, don't read/write AFE internal memroy.
   // And print the log here to check the user space behavior.

   // need check AFE clk
   aud_top = Afe_Get_Reg(AUDIO_TOP_CON0);
   afe_clk = (aud_top & 0x4);
   if(afe_clk == 0x4){
      printk("+AudDrv_DL1_Stream_Standby AFE clk(%x) Chia \n",aud_top);
   }
   else{
      memset(AFE_dl_Control->rBlock.pucVirtBufAddr,0,AFE_dl_Control->rBlock.u4BufferSize);
   }
   AFE_dl_Control->rBlock.u4WriteIdx	  = 0;
   AFE_dl_Control->rBlock.u4DMAReadIdx	  = 0;
   AFE_dl_Control->rBlock.u4DataRemained = 0;
   AFE_dl_Control->rBlock.u4fsyncflag    = false;
   AFE_dl_Control->rBlock.uResetFlag     = true;
   spin_unlock(&auddrv_lock);

   PRINTK_AUDDRV("-AudDrv_DL1_Stream_Standby \n");
}

void AudDrv_Init_DL1_Stream(kal_uint32 Afe_Buf_Length)
{
   kal_uint32 align = 0, ptr = 0, aud_top, afe_clk;
   if(Flag_Aud_DL1_SlaveOn)
   {   
      if(Afe_Buf_Length > AFE_BUF_SLAVE_SIZE)
      {
         printk("AudDrv_Init_DL1_Stream Afe_Buf_Length=%d \n",Afe_Buf_Length);
         Afe_Buf_Length = AFE_BUF_SLAVE_SIZE;
      }
   }
   PRINTK_AUDDRV("AudDrv_Init_DL1_Stream Afe_Buf_Length:%x \n",Afe_Buf_Length);
   AFE_dl_Control->u4BufferSize = Afe_Buf_Length;  // 32 byte align
   // init output stream
   AFE_dl_Control->rBlock.u4BufferSize    = AFE_dl_Control->u4BufferSize;

   if(Flag_Aud_DL1_SlaveOn)
   {
      printk("AudDrv_Init_DL1_Stream Slave Mode \n");
      AFE_dl_Control->rBlock.pucPhysBufAddr = 0xD4000000;
      AFE_dl_Control->rBlock.pucVirtBufAddr = AudSRAMVirtBufAddr;  // use ioremap to map sram
      printk("AudDrv_Init_DL1_Stream slave address %p\n",AFE_dl_Control->rBlock.pucVirtBufAddr );
   }
   else
   {//Audio DL1 master mode used

#if defined(DL1_USE_FLEXL2)
      ////////////
      // Use SysRam (FlexL2)
      printk("AudDrv_Init_DL1_Stream FlexL2 \n");
      AFE_dl_Control->rBlock.pucPhysBufAddr = 0x90000000;
      AFE_dl_Control->rBlock.pucVirtBufAddr = (kal_uint8*)((AFE_dl_Control->rBlock.pucPhysBufAddr - 0x90000000) + 0xf9000000);
#elif defined(DL1_USE_SYSRAM)
      ////////////
      // Use SysRam (only for Test)
      printk("AudDrv_Init_DL1_Stream MT6573_SYSRAM_ALLOC \n");
      AFE_dl_Control->rBlock.pucPhysBufAddr = (void*)MT6573_SYSRAM_ALLOC(MT6573SYSRAMUSR_MFLEXVIDEO,AFE_dl_Control->rBlock.u4BufferSize,32);
      if(NULL == AFE_dl_Control->rBlock.pucPhysBufAddr){
         printk("AudDrv_Init_DL1_Stream MT6573_SYSRAM_ALLOC fail \n");
         return;
      }
      AFE_dl_Control->rBlock.pucVirtBufAddr = (AFE_dl_Control->rBlock.pucPhysBufAddr - MM_SYSRAM_BASE_PA) + MM_SYSRAM_BASE;
#else
      ////////////
      // Use DMA alloc coherent
      AFE_dl_Control->rBlock.pucVirtBufAddr = dma_alloc_coherent(0, AFE_dl_Control->rBlock.u4BufferSize, &AFE_dl_Control->rBlock.pucPhysBufAddr, GFP_KERNEL);
      if(NULL == AFE_dl_Control->rBlock.pucPhysBufAddr){
         printk("AudDrv_Init_DL1_Stream dma_alloc_coherent fail \n");
         return;
      }
#endif
   }

   if((AFE_dl_Control->rBlock.pucPhysBufAddr & 0x1f) != 0 ){
      printk("[Auddrv] DL1 DMA address is not aligned (0x%x) \n",AFE_dl_Control->rBlock.pucPhysBufAddr);
   }

   AFE_dl_Control->rBlock.u4SampleNumMask = 0x001f;  // 32 byte align
   AFE_dl_Control->rBlock.u4WriteIdx	   = 0;
   AFE_dl_Control->rBlock.u4DMAReadIdx    = 0;
   AFE_dl_Control->rBlock.u4DataRemained  = 0;
   AFE_dl_Control->rBlock.u4fsyncflag     = false;
   AFE_dl_Control->rBlock.uResetFlag      = true;

   // need check AFE clk
   aud_top = Afe_Get_Reg(AUDIO_TOP_CON0);
   afe_clk = (aud_top & 0x4);
   if(afe_clk == 0x4){
      printk("+AudDrv_Init_DL1_Stream AFE clk(%x) Chia \n",aud_top);
   }
   else{
      memset((void*)AFE_dl_Control->rBlock.pucVirtBufAddr,0,AFE_dl_Control->rBlock.u4BufferSize);
   }

   ptr   = AFE_dl_Control->rBlock.pucPhysBufAddr;
   align = (ptr & 0x1f);
   if(align != 0){
      printk("AudDrv_Init_DL1_Stream dam dam not 32bytes align ptr:%x \n",ptr);
   }
   // 32 bytes align
   if(!Flag_Aud_DL1_SlaveOn)
   {
      Afe_Set_Reg(AFE_DL1_BASE , AFE_dl_Control->rBlock.pucPhysBufAddr , 0xffffffff);
      Afe_Set_Reg(AFE_DL1_END  , AFE_dl_Control->rBlock.pucPhysBufAddr+(AFE_dl_Control->rBlock.u4BufferSize -1) , 0xffffffff);
   }

   printk("AudDrv_Init_DL1_Stream pucVirtBufAddr=%p, pucPhysBufAddr=%x, ptr=%x\n",
    AFE_dl_Control->rBlock.pucVirtBufAddr,AFE_dl_Control->rBlock.pucPhysBufAddr,ptr);
}

void AudDrv_DeInit_DL1_Stream(void)
{
   PRINTK_AUDDRV("+AudDrv_DeInit_DL1_Stream ");

   if(AFE_dl_Control->rBlock.pucVirtBufAddr != NULL)
   {
      if(!Flag_Aud_DL1_SlaveOn)
      {//Audio DL1 master mode used
#ifdef DL1_USE_SYSRAM
         MT6573_SYSRAM_FREE(MT6573SYSRAMUSR_MFLEXVIDEO);
#elif (!defined (DL1_USE_FLEXL2))
         ////////////
         // Use DMA free coherent
         dma_free_coherent(0, AFE_dl_Control->rBlock.u4BufferSize, AFE_dl_Control->rBlock.pucVirtBufAddr, AFE_dl_Control->rBlock.pucPhysBufAddr);
#endif
      }
      AFE_dl_Control->rBlock.pucVirtBufAddr = NULL;
   }
   AFE_dl_Control->rBlock.u4BufferSize    = AFE_BUF_SLAVE_SIZE;
   AFE_dl_Control->rBlock.pucPhysBufAddr  = 0;
   AFE_dl_Control->rBlock.u4SampleNumMask = 0;
   AFE_dl_Control->rBlock.u4WriteIdx	   = 0;
   AFE_dl_Control->rBlock.u4DMAReadIdx    = 0;
   AFE_dl_Control->rBlock.u4DataRemained  = 0;
   AFE_dl_Control->rBlock.u4fsyncflag     = false;
   AFE_dl_Control->rBlock.uResetFlag      = true;

   PRINTK_AUDDRV("-AudDrv_DeInit_DL1_Stream ");
}

void AudDrv_Reset_DL1_Stream_Buf(void)
{
   if(AFE_dl_Control->rBlock.u4BufferSize > 0 && AFE_dl_Control->rBlock.pucVirtBufAddr != NULL){
      // need check AFE clk
      kal_uint32 aud_top = Afe_Get_Reg(AUDIO_TOP_CON0);
      kal_uint32 afe_clk = (aud_top & 0x4);
      if(afe_clk == 0x4){
         printk("+AudDrv_Reset_DL1_Stream_Buf AFE clk(%x) Chia \n",aud_top);
      }
      else{
         memset((void*)AFE_dl_Control->rBlock.pucVirtBufAddr,0,AFE_dl_Control->rBlock.u4BufferSize);
      }
   }
}

// check if still in normal mode , ex::record.
static bool AudDrv_NormalPlayback(void)
{
   if(SPH_Ctrl_State.bBgsFlag || SPH_Ctrl_State.bSpeechFlag || SPH_Ctrl_State.bTtyFlag || SPH_Ctrl_State.bVT)
      return false;
   else
      return true;
}

// check if data is fine to go
static bool AudDrv_Check(AFE_BLOCK_T *Afe_Block)
{
   if(Afe_Block == NULL){
      return true;
   }
   if(Afe_Block->u4BufferSize <= 0 || Afe_Block->u4DMAReadIdx < 0 || Afe_Block->u4DataRemained  <0){
      return true;
   }
   else{
      return false;
   }
}

void AudDrv_Do_Tasklet(void)  // interrupt tasklet
{
   kal_int32 Afe_consumed_bytes = 0;
   kal_int32 HW_Cur_ReadIdx = 0;
   kal_uint32 Reg_AFE_DAC_CON1, Reg_DL1_OPMODE, Reg_AFE_IRQ_MCU_CNT1, Reg_AFE_MODEM_IR_STATUS;
   AFE_BLOCK_T *Afe_Block = NULL;
   kal_uint32 aud_top, afe_clk;

   Afe_Block   = &(AFE_dl_Control->rBlock);

   PRINTK_AUDDRV("AudDrv_Do_Tasklet Afe_irq_status:%x \n",Afe_irq_status);
   if(Afe_irq_status == 0 )
   {
      PRINTK_AUDDRV("AudDrv_Do_Tasklet Afe_irq_status=0 \n");
      return;
   }

   if(AudDrv_Check(Afe_Block)){
      printk("AudDrv_Do_Tasklet: u4BufferSize= %d, Afe_Block->u4DMAReadIdx = %d   Afe_Block->u4DataRemained  = %d Error!!\n"
          ,Afe_Block->u4BufferSize,Afe_Block->u4DMAReadIdx,Afe_Block->u4DataRemained );
      Afe_irq_status =0;
      return;
   }

HW_READ_DATA_AGAIN:
   if ( (Afe_irq_status&0xf) & IRQ1_MCU_CLR) // VUL/DL1/DL2/AWB
   {
      Afe_irq_status &=(~IRQ1_MCU_CLR);
      PRINTK_AUDDRV("2Tasklet Afe_irq_status=%x \n",Afe_irq_status);
   }
   else if( (Afe_irq_status&0xf) & IRQ2_MCU_CLR)  // I2S
   {
      PRINTK_AUDDRV("AudDrv_Do_Tasklet I2S \n");
      Afe_irq_status &=(~IRQ2_MCU_CLR);
      AudDrv_I2Sin_Tasklet();
      return;
   }
   else if( (Afe_irq_status&0xf) & IRQ_MCU_DAI_SET_CLR)  // BT DAI
   {
      printk("AudDrv_Do_Tasklet IRQ_MCU_DAI_SET_CLR \n");
      DAI_output_Control_context.status =DAI_ASSERTION;
      // reset buffer data for BT , pretend has buffer.

      // need check AFE clk
      aud_top = Afe_Get_Reg(AUDIO_TOP_CON0);
      afe_clk = (aud_top & 0x4);
      if(afe_clk == 0x4){
        printk("+AudDrv_Do_Tasklet AFE clk(%x) Chia \n",aud_top);
      }
      else{
         memset(Afe_Block->pucVirtBufAddr,0,Afe_Block->u4BufferSize);
      }
      spin_lock_bh(&auddrv_lock);
      Reg_AFE_IRQ_MCU_CNT1= Afe_Get_Reg(AFE_IRQ_CNT1);
      Afe_consumed_bytes = Reg_AFE_IRQ_MCU_CNT1<<2;
      Afe_Block->u4DMAReadIdx += Afe_consumed_bytes;
      Afe_Block->u4DMAReadIdx %= Afe_Block->u4BufferSize;
      if(Afe_Block->u4WriteIdx   > Afe_Block->u4DMAReadIdx){
          Afe_Block->u4DataRemained = Afe_Block->u4WriteIdx  -  Afe_Block->u4DMAReadIdx;
      }
      else{
          Afe_Block->u4DataRemained =   Afe_Block->u4DMAReadIdx + Afe_Block->u4BufferSize - Afe_Block->u4WriteIdx;
      }
      spin_unlock_bh(&auddrv_lock);

      Afe_Set_Reg(AFE_DAIBT_CON ,0x8, 0x8); // set data ready
      Afe_irq_status &=(~IRQ_MCU_DAI_SET_CLR);
      return;
   }
   else if( (Afe_irq_status&0xf) & IRQ_MCU_DAI_RST_CLR)  // BT DAI
   {
      printk("AudDrv_Do_Tasklet IRQ_MCU_DAI_RST_CLR \n");
      DAI_output_Control_context.status =DAI_NEGATION;
      Afe_irq_status &=(~IRQ_MCU_DAI_RST_CLR);
      return;
   }

   //If DL1 slave mode, return directly without updating R/W prt and clearing buffer
   Reg_AFE_DAC_CON1= Afe_Get_Reg(AFE_DAC_CON1);
   Reg_DL1_OPMODE = (Reg_AFE_DAC_CON1>>19)&0x1;
   if(Reg_DL1_OPMODE == 0)
   {

      Reg_AFE_MODEM_IR_STATUS= Afe_Get_Reg(AFE_MODEM_IR_STATUS);
      if((Reg_AFE_MODEM_IR_STATUS&0x1)&&(!AudDrv_NormalPlayback()))
      {
         printk("Modem DL1 Slave mode:  DL1_OPMODE=0x%x, AFE_MODEM_IR_STATUS=0x%x\n", Reg_DL1_OPMODE, Reg_AFE_MODEM_IR_STATUS);
         return;
      }
      else
      {
         if(Flag_Aud_DL1_SlaveOn)
         {
            PRINTK_AUDDRV("AP DL1 Slave\n");
            //update current pointer
            Reg_AFE_IRQ_MCU_CNT1= Afe_Get_Reg(AFE_IRQ_CNT1);
            Afe_consumed_bytes = Reg_AFE_IRQ_MCU_CNT1<<2;
         }
         else
         {
            printk("Error! Not AP and Modem use DL1 slave!!\n");
         }
      }
   }
   else
   {
      // Calculate the number of data that has been HW read
      if(Afe_Block->uResetFlag == true){
         Afe_Block->uResetFlag = false;
         HW_Cur_ReadIdx = Afe_Get_Reg(AFE_DL1_CUR);
         PRINTK_AUDDRV("Tasklet HW_Cur_ReadIdx=%x, ReadIdx=%x, WriteIdx=%x, DataRemained=%x \n",HW_Cur_ReadIdx,Afe_Block->u4DMAReadIdx,Afe_Block->u4WriteIdx,Afe_Block->u4DataRemained);
      }

      HW_Cur_ReadIdx = Afe_Get_Reg(AFE_DL1_CUR);

      if(HW_Cur_ReadIdx == 0){
         printk("Aud DL Tasklet error, AFE_DL1_CUR, NULL pointer ");
         return;
      }

      HW_Cur_ReadIdx = HW_Cur_ReadIdx - Afe_Block->pucPhysBufAddr;
      Afe_consumed_bytes = (HW_Cur_ReadIdx) - Afe_Block->u4DMAReadIdx;  // HW already consume

      if(Afe_consumed_bytes <0){
         Afe_consumed_bytes += Afe_Block->u4BufferSize;
      }
      PRINTK_AUDDRV("Tasklet HW_con:%x \n",Afe_consumed_bytes);
   }

   if(Afe_Block->u4DataRemained < Afe_consumed_bytes)
   {
      // buffer underflow --> clear the whole buffer and notify user space

      // need check AFE clk
      kal_uint32 aud_top = Afe_Get_Reg(AUDIO_TOP_CON0);
      kal_uint32 afe_clk = (aud_top & 0x4);
      if(afe_clk == 0x4){
         printk("+AudDrv_Do_Tasklet1 AFE clk(%x) Chia \n",aud_top);
      }
      else{
         memset(Afe_Block->pucVirtBufAddr,0,Afe_Block->u4BufferSize);
      }

      if(b_SPKSound_Happen == false)
      {
         printk("+Tasklet underflow R:%x, W:%x, DataR:%x, HW_cons:%x \n",
                 Afe_Block->u4DMAReadIdx,Afe_Block->u4WriteIdx,Afe_Block->u4DataRemained,Afe_consumed_bytes);
      }

      spin_lock_bh(&auddrv_lock);
      Afe_Block->u4DMAReadIdx += Afe_consumed_bytes;
      Afe_Block->u4DMAReadIdx %= Afe_Block->u4BufferSize;
      
      if(Afe_Block->u4WriteIdx   > Afe_Block->u4DMAReadIdx){
         Afe_Block->u4DataRemained =  Afe_Block->u4WriteIdx  -  Afe_Block->u4DMAReadIdx;
         Afe_Block->u4DataRemained %= Afe_Block->u4BufferSize;
      }
      else{
         Afe_Block->u4DataRemained =   Afe_Block->u4DMAReadIdx + Afe_Block->u4BufferSize - Afe_Block->u4WriteIdx;
         Afe_Block->u4DataRemained %= Afe_Block->u4BufferSize;
      }      
      spin_unlock_bh(&auddrv_lock);

      if(Afe_Block->u4fsyncflag == false)
      {
         kill_fasync(&AudDrv_async, SIGIO, POLL_IN);  // notify the user space
         spin_lock_bh(&auddrv_lock);
         Afe_Block->u4fsyncflag = true;
         spin_unlock_bh(&auddrv_lock);
      }

      if(b_SPKSound_Happen == false)
      {
         printk("-Tasklet 2underflow R:%x, W:%x, DataR:%x \n",
                 Afe_Block->u4DMAReadIdx,Afe_Block->u4WriteIdx,Afe_Block->u4DataRemained);
      }
   }
   else
   {
      spin_lock_bh(&auddrv_lock);
      Afe_Block->u4DataRemained -= Afe_consumed_bytes;
      Afe_Block->u4DMAReadIdx += Afe_consumed_bytes;
      Afe_Block->u4DMAReadIdx %= Afe_Block->u4BufferSize;
      spin_unlock_bh(&auddrv_lock);

      PRINTK_AUDDRV("Tasklet normal R:%x, W:%x, DataR:%x \n",
              Afe_Block->u4DMAReadIdx,Afe_Block->u4WriteIdx,Afe_Block->u4DataRemained);
   }

   //should wake up before read data again, because DL data has been processed.
   wait_queue_flag =1;
   wake_up_interruptible(&DL_Wait_Queue);

   if( (Afe_irq_status&0xf) != 0){
      PRINTK_AUDDRV("Tasklet  HW_READ_DATA_AGAIN \n");
      goto HW_READ_DATA_AGAIN;
   }
   return ;
}

void AudDrv_Init_I2S_InputStream(kal_uint32 I2S_Buffer_Length,struct file *fp)
{
   if(I2S_Buffer_Length > I2S_BUF_MAX)
   {
      PRINTK_AUDDRV("AudDrv_Init_I2S_InputStream I2S buf Length=%d\n",I2S_Buffer_Length);
      I2S_Buffer_Length = I2S_BUF_MAX;
   }
   PRINTK_AUDDRV("AudDrv_Init_I2S_InputStream I2S_Buffer_Length=%x\n",I2S_Buffer_Length);
   I2S_input_Control->u4BufferSize = I2S_Buffer_Length;
   I2S_input_Control->rBlock.u4BufferSize = I2S_Buffer_Length;


// Use SysRam (only for Test)
    I2S_input_Control->rBlock.pucPhysBufAddr = I2SInPhyBufAddr;
    I2S_input_Control->rBlock.pucVirtBufAddr = I2SInVirtBufAddr;


// Use kmalloc
//   I2S_input_Control->rBlock.pucVirtBufAddr   = (kal_uint8*)kmalloc(I2S_Buffer_Length , GFP_KERNEL);  // use ioremap to map sram
//   I2S_input_Control->rBlock.pucPhysBufAddr   = virt_to_phys((void*)(I2S_input_Control->rBlock.pucVirtBufAddr));

   if((I2S_input_Control->rBlock.pucPhysBufAddr & 0x1f) != 0 ){
      printk("[Auddrv] I2S In address is not aligned (0x%x) \n",I2S_input_Control->rBlock.pucPhysBufAddr);
   }

   I2S_input_Control->rBlock.u4SampleNumMask  = 0x001f;  // 32 byte align
   I2S_input_Control->rBlock.u4WriteIdx	    = 0;
   I2S_input_Control->rBlock.u4DMAReadIdx     = 0;
   I2S_input_Control->rBlock.u4DataRemained   = 0;
   I2S_input_Control->rBlock.u4fsyncflag      = false;
   I2S_input_Control->rBlock.flip                   = fp;
   memset((void*)I2S_input_Control->rBlock.pucVirtBufAddr,0,I2S_Buffer_Length);

   Afe_Set_Reg(AFE_I2S_BASE ,I2S_input_Control->rBlock.pucPhysBufAddr, 0xffffffff);
   Afe_Set_Reg(AFE_I2S_END  ,I2S_input_Control->rBlock.pucPhysBufAddr + (I2S_input_Control->rBlock.u4BufferSize-1), 0xffffffff);

   printk("-AudDrv_Init_I2S_InputStream  pucVirtBufAddr:%p, pucPhysBufAddr:0x%x \n",
   I2S_input_Control->rBlock.pucVirtBufAddr, I2S_input_Control->rBlock.pucPhysBufAddr);
}

void AudDrv_DeInit_I2S_InputStream(struct file *fp)
{
   PRINTK_AUDDRV("+AudDrv_DeInit_I2S_InputStream ");
   if(I2S_input_Control->rBlock.pucVirtBufAddr != NULL){

#if (!defined (I2SIN_USE_FLEXL2))
// Use DMA free coherent
      printk("+AudDrv_DeInit_I2S_InputStream I2S_input_Control->rBlock.u4BufferSize=%d", I2S_input_Control->rBlock.u4BufferSize);
//      dma_free_coherent(0, I2S_input_Control->rBlock.u4BufferSize, I2S_input_Control->rBlock.pucVirtBufAddr, I2S_input_Control->rBlock.pucPhysBufAddr);
#endif

      I2S_input_Control->rBlock.pucVirtBufAddr = NULL;
   }
   I2S_input_Control->rBlock.u4BufferSize    = I2S_BUF_LENGTH;
   I2S_input_Control->rBlock.pucPhysBufAddr  = 0;
   I2S_input_Control->rBlock.u4SampleNumMask = 0;
   I2S_input_Control->rBlock.u4WriteIdx	   = 0;
   I2S_input_Control->rBlock.u4DMAReadIdx    = 0;
   I2S_input_Control->rBlock.u4DataRemained  = 0;
   I2S_input_Control->rBlock.flip                   = NULL;
   I2S_input_Control->rBlock.u4fsyncflag     =  false;
   PRINTK_AUDDRV("-AudDrv_DeInit_I2S_InputStream ");
}

void  AudDrv_I2Sin_Tasklet(void)
{
   kal_int32 I2S_filled_bytes=0;
   kal_int32 HW_Cur_WriteIdx=0;
   AFE_BLOCK_T *I2Sin_Block = &(I2S_input_Control->rBlock);
   HW_Cur_WriteIdx = Afe_Get_Reg(AFE_I2S_CUR); //todo

   if(HW_Cur_WriteIdx == 0){
      printk("AudDrv_I2Sin_Tasklet, Error, AFE_I2S_CUR, NULL pointer ");
      return;
   }

   if(I2Sin_Block->u4BufferSize == 0){
      printk("AudDrv_I2Sin_Tasklet: u4BufferSize=0, Error");
      return;
   }

   // HW already fill in
   I2S_filled_bytes = (HW_Cur_WriteIdx-I2Sin_Block->pucPhysBufAddr)-I2Sin_Block->u4WriteIdx;
   if(I2S_filled_bytes <0){
   	I2S_filled_bytes += I2Sin_Block->u4BufferSize;
   }

   PRINTK_AUDDRV("AudDrv_I2Sin_Tasklet1 +I2S_filled_bytes:%x \n",I2S_filled_bytes);

   // 32 bytes alignment
   I2S_filled_bytes &= (~I2Sin_Block->u4SampleNumMask);

   PRINTK_AUDDRV("AudDrv_I2Sin_Tasklet +I2S_filled_bytes:%x, HW_Cur_WriteIdx:%x, u4DMAReadIdx:%x, u4WriteIdx:0x%x, pucPhysBufAddr:%x \n",
      I2S_filled_bytes,HW_Cur_WriteIdx,I2Sin_Block->u4DMAReadIdx,I2Sin_Block->u4WriteIdx,I2Sin_Block->pucPhysBufAddr);

   if(I2S_filled_bytes ==0)
      return;

   spin_lock_bh(&auddrv_lock);
   I2Sin_Block->u4WriteIdx  += I2S_filled_bytes;
   I2Sin_Block->u4WriteIdx  %= I2Sin_Block->u4BufferSize;
   I2Sin_Block->u4DataRemained += I2S_filled_bytes;
   // buffer overflow
   if(I2Sin_Block->u4DataRemained > I2Sin_Block->u4BufferSize)
   {
      if(b_SPKSound_Happen == false)
      {
         printk("AudDrv_I2Sin_Tasklet buffer overflow u4DMAReadIdx:%x, u4WriteIdx:%x, u4DataRemained:%x, u4BufferSize:%x \n",
         I2Sin_Block->u4DMAReadIdx,I2Sin_Block->u4WriteIdx,I2Sin_Block->u4DataRemained, I2Sin_Block->u4BufferSize);
      }

      if(I2Sin_Block->u4WriteIdx   > I2Sin_Block->u4DMAReadIdx){
          I2Sin_Block->u4DataRemained =  I2Sin_Block->u4WriteIdx  -  I2Sin_Block->u4DMAReadIdx;
          I2Sin_Block->u4DataRemained %= I2Sin_Block->u4BufferSize;
      }
      else{
          I2Sin_Block->u4DataRemained = I2Sin_Block->u4BufferSize - I2Sin_Block->u4DMAReadIdx + I2Sin_Block->u4WriteIdx;
          I2Sin_Block->u4DataRemained %= I2Sin_Block->u4BufferSize;
      }

      if( I2Sin_Block->u4DMAReadIdx <0){
          I2Sin_Block->u4DMAReadIdx+=I2Sin_Block->u4BufferSize;
      }
   }
   spin_unlock_bh(&auddrv_lock);

   PRINTK_AUDDRV("AudDrv_I2Sin_Tasklet -u4DMAReadIdx:0x%x, u4WriteIdx:0x%x, u4DataRemained:%x, u4BufferSize:%x \n",
     I2Sin_Block->u4DMAReadIdx,I2Sin_Block->u4WriteIdx,I2Sin_Block->u4DataRemained,I2Sin_Block->u4BufferSize);

   I2S_wait_queue_flag =1;
   wake_up_interruptible(&I2Sin_Wait_Queue);
}

void AudDrv_I2S_InputStream_Standby(struct file *fp,unsigned long arg)
{
   PRINTK_AUDDRV("+AudDrv_I2S_InputStream_Standby \n");

   spin_lock(&auddrv_lock);
   memset(I2S_input_Control->rBlock.pucVirtBufAddr,0,I2S_input_Control->rBlock.u4BufferSize);
   I2S_input_Control->rBlock.u4WriteIdx     = 0;
   I2S_input_Control->rBlock.u4DMAReadIdx	  = 0;
   I2S_input_Control->rBlock.u4DataRemained = 0;
   spin_unlock(&auddrv_lock);
}

static irqreturn_t AudDrv_IRQ_handler(int irq, void *dev_id)
{
   Afe_irq_status |=  Afe_Get_Reg(AFE_IR_STATUS);
   Afe_irq_status &= 0xf;
   Afe_Set_Reg(AFE_IR_CLR,Afe_irq_status,0x00ff);
   tasklet_schedule(&magic_tasklet_handle);
   return IRQ_HANDLED;
}

static void AudDrv_magic_tasklet(unsigned long data)
{
   AudDrv_Do_Tasklet();
}

static int AudDrv_open(struct inode *inode, struct file *fp)
{
   // do nothing
	PRINTK_AUDDRV("AudDrv_open do nothing inode:%p, file:%p \n",inode,fp);
	return 0;
}

static int AudDrv_release(struct inode *inode, struct file *fp)
{
	PRINTK_AUDDRV("AudDrv_release inode:%p, file:%p \n",inode,fp);
   AudDrv_fasync(-1,fp,0);
	if (!(fp->f_mode & FMODE_WRITE || fp->f_mode & FMODE_READ)){
      return -ENODEV;
   }
	return 0;
}


static int AudDrv_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
   int ret =0;
   Register_Control Reg_Data;
   AudDrv_Clk_On();

   switch(cmd)
   {
// for DL1 output SRC
      case OPEN_DL1_STREAM:
      {
         PRINTK_AUDDRV("AudDrv OPEN_DL1_STREAM \n");
         AFE_dl_Control->open_cnt++;
         break;  // allocate this fp with one asm block
      }
      case INIT_DL1_STREAM:
      {
         AudDrv_Clk_On();
         if(Aud_Flush_cntr !=0){
            printk("AudDrv_ioctl(INIT_DL1_STREAM) flush cntr(%d)",Aud_Flush_cntr);

            // Timing:
            // If IRQ is keep running, but MediaServer die, the u4BufferSize would be reset to 0.
            // but Tasklet function maybe use u4BufferSize to calculate the size.
            // One case, AudDrv_Reset executed, then printk happen. Then Schedule to Tasklet function, the u4BufferSize maybe 0.
            // So, this maybe cause KE.
            AudDrv_Reset();
            Aud_Flush_cntr =0;
            // For one case:
            // Audio start play, but mediaServer die. So the power status maybe keep the previous state.
            // Need to check the Afe_Mem_Pwr_on flag and execute power off.
            if(Afe_Mem_Pwr_on > 0 )
            {
               printk("AudDrv INIT_DL1_STREAM MediaServer abnormal,AFE(%d),AFEp(%d)\n",Aud_AFE_Clk_cntr,Afe_Mem_Pwr_on);
               spin_lock_bh(&auddrv_lock);
               Afe_Mem_Pwr_on = 0;
               spin_unlock_bh(&auddrv_lock);
               AudDrv_Clk_Off();
               b_reset_afe_pwr = true;
            }
            if(I2S_Pwr_on > 0 )
            {
               printk("AudDrv INIT_DL1_STREAM MediaServer abnormal,I2S(%d),I2Sp(%d)\n",Aud_I2S_Clk_cntr,I2S_Pwr_on);
               spin_lock_bh(&auddrv_lock);
               I2S_Pwr_on = 0;
               spin_unlock_bh(&auddrv_lock);
               AudDrv_I2S_Clk_Off();
               AudDrv_Clk_Off();
               b_reset_i2s_pwr = true;
            }
            
            if(b_adc_clk_on > 0){
               // When enter in-call mode, the adc counter will add 1.
               // If media server die, need to sub the reference count of adc clk
               // For the speech/record/bgs, the media server die handling will 
               //   be in AudioYusuHardware::Recover_State() function. (Don't care the power control)
               printk("AudDrv INIT_DL1_STREAM, ADC(%d),b_adc_clk_on(%d) \n",Aud_ADC_Clk_cntr,b_adc_clk_on);
               b_adc_clk_on--;
               AudDrv_ADC_Clk_Off();
               AudDrv_Clk_Off();
            }            
            if(b_afe_line_in_clk_on > 0){
               printk("AudDrv INIT_DL1_STREAM,AFE(%d),AFEp(%d),b_afe_line_in_clk_on(%d) \n",Aud_AFE_Clk_cntr,Afe_Mem_Pwr_on,b_afe_line_in_clk_on);
               b_afe_line_in_clk_on--;
               AudDrv_Clk_Off();
            }

            
         }

         printk("AudDrv INIT_DL1_STREAM bsize(%ld),AFE(%d),AFEp(%d)\n",arg,Aud_AFE_Clk_cntr,Afe_Mem_Pwr_on);
         printk("AudDrv INIT_DL1_STREAM I2S(%d),I2Sp(%d)\n",Aud_I2S_Clk_cntr,I2S_Pwr_on);
         //init stream
         AudDrv_Init_DL1_Stream(arg);
         Sound_Speaker_Turnoff(Channel_Stereo);
         AudDrv_Clk_Off();
         break;
      }
      case START_DL1_STREAM:
      {
         AudDrv_Clk_On();
         PRINTK_AUDDRV("+Aud START_DL1_STREAM arg:%ld \n",arg);

         if(!Flag_Aud_DL1_SlaveOn)
         {
            Afe_Set_Reg(AFE_DL1_BASE , AFE_dl_Control->rBlock.pucPhysBufAddr , 0xffffffff);
            Afe_Set_Reg(AFE_DL1_END  , AFE_dl_Control->rBlock.pucPhysBufAddr+(AFE_dl_Control->rBlock.u4BufferSize -1) , 0xffffffff);
         }

         if(b_reset_afe_pwr==true){
            b_reset_afe_pwr=false;
            printk("AudDrv START_DL1_STREAM, reset AFE clk AFE(%d),AFEp(%d)\n",Aud_AFE_Clk_cntr,Afe_Mem_Pwr_on);
         }

         PRINTK_AUDDRV("+Aud START_DL1_STREAM VirtBuf=%p, PhysBuf=%x \n",
           AFE_dl_Control->rBlock.pucVirtBufAddr,AFE_dl_Control->rBlock.pucPhysBufAddr);

         spin_lock_bh(&auddrv_lock);
         Afe_Mem_Pwr_on++;
         spin_unlock_bh(&auddrv_lock);
         if(Afe_Mem_Pwr_on == 1 ){
            PRINTK_AUDDRV("!! AudDrv START_DL1_STREAM, AFE(%d),AFEp(%d)\n",Aud_AFE_Clk_cntr,Afe_Mem_Pwr_on);
            AudDrv_Clk_On();

            PRINTK_AUDDRV("AudDrv START_DL1_STREAM, wake_lock \n");
            wake_lock(&Audio_wake_lock);
         }
         else{
            printk("!! AudDrv START_DL1_STREAM, AFE(%d),AFEp>1(%d)\n",Aud_AFE_Clk_cntr,Afe_Mem_Pwr_on);
         }
         AudDrv_Reset_DL1_Stream_Buf();
         printk("-Aud START_DL1_STREAM AFE(%d),AFEp(%d)\n",Aud_AFE_Clk_cntr,Afe_Mem_Pwr_on);
         AudDrv_Clk_Off();
         break;
      }
      case STANDBY_DL1_STREAM:
      {
         AudDrv_Clk_On();

         PRINTK_AUDDRV("+Aud STANDBY_DL1_STREAM, AFE(%d),AFEp(%d)\n",Aud_AFE_Clk_cntr,Afe_Mem_Pwr_on);
         AudDrv_DL1_Stream_Standby(fp,arg);
         spin_lock_bh(&auddrv_lock);
         Afe_Mem_Pwr_on--;
         spin_unlock_bh(&auddrv_lock);
         if(Afe_Mem_Pwr_on == 0 )
         {
            PRINTK_AUDDRV(" AudDrv STANDBY_DL1_STREAM, AFE(%d),AFEp(%d)\n",Aud_AFE_Clk_cntr,Afe_Mem_Pwr_on);
            PRINTK_AUDDRV("!!! AudDrv STANDBY_DL1_STREAM Speech:%d, VT:%d \n",SPH_Ctrl_State.bSpeechFlag,SPH_Ctrl_State.bVT);
            AudDrv_Clk_Off();
            PRINTK_AUDDRV("AudDrv STANDBY_DL1_STREAM,1 wake_unlock \n");
            wake_unlock(&Audio_wake_lock);
         }
         else if(Afe_Mem_Pwr_on < 0)
         {
            printk(" AudDrv STANDBY_DL1_STREAM, AFE(%d),AFEp(%d)\n",Aud_AFE_Clk_cntr,Afe_Mem_Pwr_on);
            spin_lock_bh(&auddrv_lock);
            Afe_Mem_Pwr_on = 0;
            spin_unlock_bh(&auddrv_lock);
            PRINTK_AUDDRV("AudDrv STANDBY_DL1_STREAM,2 wake_unlock \n");
            wake_unlock(&Audio_wake_lock);
         }
         printk("-Aud STANDBY_DL1_STREAM AFE(%d),AFEp(%d)\n",Aud_AFE_Clk_cntr,Afe_Mem_Pwr_on);

         AudDrv_Clk_Off();
         break;
      }
      case CLOSE_DL1_STREAM:
      {
         PRINTK_AUDDRV("!!! AudDrv CLOSE_DL1_STREAM \n");
         AudDrv_Clk_On();
         AudDrv_DeInit_DL1_Stream();
         AFE_dl_Control->open_cnt--;
         AudDrv_Clk_Off();
         break;
      }
      case SET_DL1_AFE_BUFFER:
      {
         PRINTK_AUDDRV("!!! Aud SET_DL1_AFE_BUFFER PhyBufAddr:0x%x \n",AFE_dl_Control->rBlock.pucPhysBufAddr);
         AudDrv_Clk_On();
         if(!Flag_Aud_DL1_SlaveOn)
         {
             Afe_Set_Reg(AFE_DL1_BASE , AFE_dl_Control->rBlock.pucPhysBufAddr , 0xffffffff);
             Afe_Set_Reg(AFE_DL1_END  , AFE_dl_Control->rBlock.pucPhysBufAddr+(AFE_dl_Control->rBlock.u4BufferSize -1) , 0xffffffff);
         }
         AudDrv_Clk_Off();
         break;
      }

      case SET_DL1_SLAVE_MODE:
      {
          if(arg ==1)
          {
              Flag_Aud_DL1_SlaveOn = true;
          }
          else
          {
              Flag_Aud_DL1_SlaveOn = false;
          }
          PRINTK_AUDDRV("!!! Aud SET_DL1_SLAVE_MODE Flag_Aud_DL1_SlaveOn:%d \n", Flag_Aud_DL1_SlaveOn);
          break;
      }
      case GET_DL1_SLAVE_MODE:
      {
          ret = Flag_Aud_DL1_SlaveOn;
          PRINTK_AUDDRV("!!! Aud GET_DL1_SLAVE_MODE Flag_Aud_DL1_SlaveOn:%d \n", Flag_Aud_DL1_SlaveOn);
          break;
      }
      case GET_AFE_BUFFER_SIZE:
      {
          PRINTK_AUDDRV("AudDrv GET_AFE_BUFFER_SIZE \n");
          ret = AFE_dl_Control->rBlock.u4BufferSize;
          break;
      }
// for I2S input
      case OPEN_I2S_INPUT_STREAM:
      {
         AudDrv_Clk_On();
         AudDrv_I2S_Clk_On();
         PRINTK_AUDDRV("AudDrv OPEN_I2S_INPUT_STREAM, AFE(%d),AFEp(%d),I2S(%d),I2Sp(%d)\n",Aud_AFE_Clk_cntr,Afe_Mem_Pwr_on,Aud_I2S_Clk_cntr,I2S_Pwr_on);
         AudDrv_Init_I2S_InputStream(arg,fp);
         I2S_input_Control->open_cnt++;
         AudDrv_I2S_Clk_Off();
         AudDrv_Clk_Off();
         break;
      }
      case START_I2S_INPUT_STREAM:
      {
         AudDrv_Clk_On();
         AudDrv_I2S_Clk_On();
         PRINTK_AUDDRV("+AudDrv START_I2S_INPUT, AFE(%d),AFEp(%d),I2S(%d),I2Sp(%d)\n",Aud_AFE_Clk_cntr,Afe_Mem_Pwr_on,Aud_I2S_Clk_cntr,I2S_Pwr_on);
         if(I2S_input_Control->rBlock.pucPhysBufAddr == 0)
         {
             printk("+START_I2S_INPUT_STREAM pucPhysBufAddr=0, Error");
             I2S_input_Control->rBlock.pucPhysBufAddr = I2SInPhyBufAddr;
         }
         if(I2S_input_Control->rBlock.u4BufferSize!=I2S_BUF_LENGTH)
         {
             printk("+START_I2S_INPUT_STREAM u4BufferSize=%d, Error", I2S_input_Control->rBlock.u4BufferSize);
             I2S_input_Control->u4BufferSize = I2S_BUF_LENGTH;
             I2S_input_Control->rBlock.u4BufferSize = I2S_BUF_LENGTH;
         }
         Afe_Set_Reg(AFE_I2S_BASE ,I2S_input_Control->rBlock.pucPhysBufAddr, 0xffffffff);
         Afe_Set_Reg(AFE_I2S_END  ,I2S_input_Control->rBlock.pucPhysBufAddr + (I2S_input_Control->rBlock.u4BufferSize-1), 0xffffffff);

         if(b_reset_i2s_pwr==true){
            b_reset_i2s_pwr=false;
            printk("AudDrv START_I2S_INPUT, reset AFE/I2S AFE(%d),AFEp(%d),I2S(%d),I2Sp(%d)\n",Aud_AFE_Clk_cntr,Afe_Mem_Pwr_on,Aud_I2S_Clk_cntr,I2S_Pwr_on);
         }

         printk("+START_I2S_INPUT pucVirtBufAddr:%p,pucPhysBufAddr:%x\n",
          I2S_input_Control->rBlock.pucVirtBufAddr, I2S_input_Control->rBlock.pucPhysBufAddr);

         spin_lock_bh(&auddrv_lock);
         I2S_Pwr_on++;
         spin_unlock_bh(&auddrv_lock);
         if(I2S_Pwr_on==1)
         {
            PRINTK_AUDDRV("AudDrv START_I2S_INPUT, I2Sp(%d)\n",I2S_Pwr_on);
            AudDrv_Clk_On();
            AudDrv_I2S_Clk_On();
         }
         else if (I2S_Pwr_on > 1)
         {
            printk("AudDrv START_I2S_INPUT, I2Sp>1(%d)\n",I2S_Pwr_on);
         }
         AudDrv_I2S_Clk_Off();
         AudDrv_Clk_Off();
         printk("-AudDrv START_I2S_INPUT, AFE(%d),AFEp(%d),I2S(%d),I2Sp(%d)\n",Aud_AFE_Clk_cntr,Afe_Mem_Pwr_on,Aud_I2S_Clk_cntr,I2S_Pwr_on);
       	break;
      }
      case STANDBY_I2S_INPUT_STREAM:
      {
         PRINTK_AUDDRV("+AudDrv STANDBY_I2S_INPUT, AFE(%d),AFEp(%d),I2S(%d),I2Sp(%d)\n",Aud_AFE_Clk_cntr,Afe_Mem_Pwr_on,Aud_I2S_Clk_cntr,I2S_Pwr_on);
         AudDrv_Clk_On();
         AudDrv_I2S_Clk_On();

         AudDrv_I2S_InputStream_Standby(fp,arg);
         spin_lock_bh(&auddrv_lock);
         I2S_Pwr_on--;
         spin_unlock_bh(&auddrv_lock);
         if(I2S_Pwr_on == 0)
         {
            PRINTK_AUDDRV("AudDrv STANDBY_I2S_INPUT,I2Sp(%d),AFEp(%d)\n",I2S_Pwr_on,Afe_Mem_Pwr_on);
            AudDrv_I2S_Clk_Off();
            AudDrv_Clk_Off();
         }
         else if(I2S_Pwr_on < 0)
         {
            printk("AudDrv STANDBY_I2S_INPUT, I2Sp(%d)<0\n",I2S_Pwr_on);
            spin_lock_bh(&auddrv_lock);
            I2S_Pwr_on = 0;
            spin_unlock_bh(&auddrv_lock);
         }
         AudDrv_I2S_Clk_Off();
         AudDrv_Clk_Off();
         printk("-AudDrv STANDBY_I2S_INPUT,AFE(%d),AFEp(%d),I2S(%d),I2Sp(%d)\n",Aud_AFE_Clk_cntr,Afe_Mem_Pwr_on,Aud_I2S_Clk_cntr,I2S_Pwr_on);
       	break;
      }
      case SET_AWB_INPUT_STREAM_STATE:
      {
          PRINTK_AUDDRV("SET_AWB_INPUT_STREAM_STATE arg = %ld fp = %p \n", arg, fp);
          AudDrv_Clk_On();
          spin_lock_bh(&auddrv_lock);
          if(arg){
              AudDrv_Start_AWB_Stream(fp);
              Auddrv_AWB_timer_on();
              wake_lock(&Audio_record_wake_lock);
          }
          else{
              Auddrv_AWB_timer_off();
              AudDrv_Standby_AWB_Stream(fp);
              wake_unlock(&Audio_record_wake_lock);
          }
          spin_unlock_bh(&auddrv_lock);
          AudDrv_Clk_Off();
          break;
      }
      case CLOSE_I2S_INPUT_STREAM:
      {
         PRINTK_AUDDRV("+AudDrv CLOSE_I2S_INPUT \n");
         I2S_input_Control->open_cnt--;
         AudDrv_DeInit_I2S_InputStream(fp);
         PRINTK_AUDDRV("-AudDrv CLOSE_I2S_INPUT \n");
         break;
      }
      case SET_I2S_Input_BUFFER:
      {
         PRINTK_AUDDRV("+Aud SET_I2S_Input_BUFFER PhysBufAddr:0x%x\n", I2S_input_Control->rBlock.pucPhysBufAddr );
         AudDrv_Clk_On();
         AudDrv_I2S_Clk_On();
         if(I2S_input_Control->rBlock.pucPhysBufAddr == 0)
         {
             printk("+SET_I2S_Input_BUFFER pucPhysBufAddr=0, Error \n");
             I2S_input_Control->rBlock.pucPhysBufAddr = I2SInPhyBufAddr;
         }
         if(I2S_input_Control->rBlock.u4BufferSize!=I2S_BUF_LENGTH)
         {
             printk("+SET_I2S_Input_BUFFER u4BufferSize=%d, Error \n", I2S_input_Control->rBlock.u4BufferSize);
             I2S_input_Control->u4BufferSize = I2S_BUF_LENGTH;
             I2S_input_Control->rBlock.u4BufferSize = I2S_BUF_LENGTH;
         }
         Afe_Set_Reg(AFE_I2S_BASE , I2S_input_Control->rBlock.pucPhysBufAddr , 0xffffffff);
         Afe_Set_Reg(AFE_I2S_END  , I2S_input_Control->rBlock.pucPhysBufAddr+(I2S_input_Control->rBlock.u4BufferSize -1) , 0xffffffff);
         AudDrv_I2S_Clk_Off();
         AudDrv_Clk_Off();
         PRINTK_AUDDRV("-Aud SET_I2S_Input_BUFFER \n");
         break;
      }
      case SET_I2S_Output_BUFFER:
      {
         AudDrv_Clk_On();
         AudDrv_I2S_Clk_On();
         PRINTK_AUDDRV("+AudDrv SET_I2S_Output_BUFFER\n");
         Afe_Set_Reg(AFE_I2S_BASE, 0x90008000, 0xffffffff);
         Afe_Set_Reg(AFE_I2S_END, 0x9000B000, 0xffffffff);
         AudDrv_I2S_Clk_Off();
         AudDrv_Clk_Off();
       	break;
      }
      case AUD_SET_LINE_IN_CLOCK:
      {
         PRINTK_AUDDRV("+AudDrv AUD_SET_LINE_IN_CLOCK(%ld) \n", arg);

         if(arg ==1)
         {
            b_afe_line_in_clk_on++;         
            AudDrv_Clk_On();
         }
         else
         {
            AudDrv_Clk_Off();
            b_afe_line_in_clk_on--;            
         }
         PRINTK_AUDDRV("-AudDrv AUD_SET_LINE_IN_CLOCK, AFE(%d),AFEp(%d) \n",Aud_AFE_Clk_cntr,Afe_Mem_Pwr_on);
         break;
      }      
      case AUD_SET_CLOCK:
      {
         PRINTK_AUDDRV("+AudDrv AUD_SET_CLOCK(%ld) \n", arg);

         if( b_reset_i2s_pwr==true || b_reset_afe_pwr==true ){
            printk("AudDrv AUD_SET_CLOCK, reset AFE(%d),AFEp(%d),I2S(%d),I2Sp(%d)\n",Aud_AFE_Clk_cntr,Afe_Mem_Pwr_on,Aud_I2S_Clk_cntr,I2S_Pwr_on);
         }

         if(arg ==1)
         {
            b_afe_clk_on++;         
            AudDrv_Clk_On();
         }
         else
         {
            AudDrv_Clk_Off();
            b_afe_clk_on--;            
         }
         PRINTK_AUDDRV("-AudDrv AUD_SET_CLOCK, AFE(%d),AFEp(%d) \n",Aud_AFE_Clk_cntr,Afe_Mem_Pwr_on);
         break;
      }
      case AUD_SET_26MCLOCK:
      {
         printk("+AudDrv AUD_SET_26MCLOCK \n");
         break;
      }
      case AUD_SET_ADC_CLOCK:
      {
         PRINTK_AUDDRV("+AudDrv AUD_SET_ADC_CLOCK(%ld) \n", arg);

         if( b_reset_i2s_pwr==true || b_reset_afe_pwr==true ){
            printk("AudDrv AUD_SET_ADC_CLOCK, reset AFE(%d),AFEp(%d),I2S(%d),I2Sp(%d)\n",Aud_AFE_Clk_cntr,Afe_Mem_Pwr_on,Aud_I2S_Clk_cntr,I2S_Pwr_on);
         }

         if(arg ==1)
         {
            b_adc_clk_on++;
            AudDrv_Clk_On();
            AudDrv_ADC_Clk_On();
         }
         else
         {
            AudDrv_ADC_Clk_Off();
            AudDrv_Clk_Off();
            b_adc_clk_on--;            
         }
         PRINTK_AUDDRV("-AudDrv AUD_SET_ADC_CLOCK, AFE(%d),AFEp(%d),ADC(%d)\n",Aud_AFE_Clk_cntr,Afe_Mem_Pwr_on,Aud_ADC_Clk_cntr);
         break;
      }
      case AUD_SET_I2S_CLOCK:
      {
         PRINTK_AUDDRV("+AudDrv AUD_SET_I2S_CLOCK(%ld) \n", arg);

         if( b_reset_i2s_pwr==true || b_reset_afe_pwr==true ){
            printk("AudDrv AUD_SET_I2S_CLOCK, reset AFE(%d),AFEp(%d),I2S(%d),I2Sp(%d)\n",Aud_AFE_Clk_cntr,Afe_Mem_Pwr_on,Aud_I2S_Clk_cntr,I2S_Pwr_on);
         }

         if(arg ==1)
         {
            b_i2s_clk_on++;
            AudDrv_Clk_On();
            AudDrv_I2S_Clk_On();
         }
         else
         {
            AudDrv_I2S_Clk_Off();
            AudDrv_Clk_Off();
            b_i2s_clk_on--;            
         }
         PRINTK_AUDDRV("-AudDrv AUD_SET_I2S_CLOCK, AFE(%d),AFEp(%d),I2S(%d),I2Sp(%d)\n",Aud_AFE_Clk_cntr,Afe_Mem_Pwr_on,Aud_I2S_Clk_cntr,I2S_Pwr_on);
         break;
      }

// Set/Get AudioSys Register
      case SET_AUDSYS_REG:
      {
         AudDrv_Clk_On();
         if(copy_from_user((void *)(&Reg_Data), (const void __user *)( arg), sizeof(Reg_Data))){
             AudDrv_Clk_Off();
             return -EFAULT;
         }
         //printk("AudDrv SET_AUDSYS_REG offset=%x, value=%x, mask=%x \n",Reg_Data.offset,Reg_Data.value,Reg_Data.mask);
         spin_lock_bh(&auddrv_lock);
         Afe_Set_Reg(Reg_Data.offset,Reg_Data.value,Reg_Data.mask);
         spin_unlock_bh(&auddrv_lock);
         AudDrv_Clk_Off();
         break;
      }
      case GET_AUDSYS_REG:
      {
         AudDrv_Clk_On();
         //printk("AudDrv GET_AUDSYS_REG \n");
         if(copy_from_user((void *)(&Reg_Data), (const void __user *)( arg), sizeof(Reg_Data))){
            AudDrv_Clk_Off();
            return -EFAULT;
         }
         spin_lock_bh(&auddrv_lock);
         Reg_Data.value = Afe_Get_Reg(Reg_Data.offset);
         spin_unlock_bh(&auddrv_lock);
         if(copy_to_user((void __user *)( arg),(void *)(&Reg_Data), sizeof(Reg_Data))){
            AudDrv_Clk_Off();
            return -EFAULT;
         }
         AudDrv_Clk_Off();
         break;
      }
// Set/Get AnalogAFE Register
      case SET_ANAAFE_REG:
      {
         AudDrv_Clk_On();
         if(copy_from_user((void *)(&Reg_Data), (const void __user *)( arg), sizeof(Reg_Data))){
            AudDrv_Clk_Off();
            return -EFAULT;
         }
         //printk("AudDrv SET_ANAAFE_REG offset=%x, value=%x, mask=%x \n",Reg_Data.offset,Reg_Data.value,Reg_Data.mask);
         spin_lock_bh(&auddrv_lock);
         Ana_Set_Reg(Reg_Data.offset,Reg_Data.value,Reg_Data.mask);
         spin_unlock_bh(&auddrv_lock);
         AudDrv_Clk_Off();
         break;
      }
      case GET_ANAAFE_REG:
      {
         AudDrv_Clk_On();
	    	//printk("AudDrv GET_ANAAFE_REG \n");
	    	if(copy_from_user((void *)(&Reg_Data), (const void __user *)( arg), sizeof(Reg_Data))){
	    	    AudDrv_Clk_Off();
	    	    return -EFAULT;
	    	}
	    	spin_lock_bh(&auddrv_lock);
	    	Reg_Data.value = Ana_Get_Reg(Reg_Data.offset);
	    	spin_unlock_bh(&auddrv_lock);
	    	if(copy_to_user((void __user *)( arg),(void *)(&Reg_Data), sizeof(Reg_Data))){
	    	    AudDrv_Clk_Off();
	    	    return -EFAULT;
	    	}
         AudDrv_Clk_Off();
	    	break;
      }
// for Speaker Setting
      case SET_SPEAKER_VOL:
      {
         PRINTK_AUDDRV("AudDrv SET_SPEAKER_VOL level:%u \n",arg);
         Sound_Speaker_SetVolLevel(arg);
         break;
      }
      case SET_SPEAKER_ON:
      {
         spin_lock_bh(&auddrv_lock);
         if(AMP_Flag != true){
            AMP_Flag = true;
            spin_unlock_bh(&auddrv_lock);
            PRINTK_AUDDRV("AudDrv SET_SPEAKER_ON arg:%u \n",arg);

            Sound_Speaker_Turnon(arg);
         }
         else{
            spin_unlock_bh(&auddrv_lock);
         }

         break;
      }
      case SET_SPEAKER_OFF:
      {
         spin_lock_bh(&auddrv_lock);
         if(AMP_Flag !=false){
             AMP_Flag = false;
             spin_unlock_bh(&auddrv_lock);
             PRINTK_AUDDRV("AudDrv SET_SPEAKER_OFF arg:%u \n",arg);
             Sound_Speaker_Turnoff(arg);
             //Sound_Speaker_Turnoff(arg);
         }
         else{
             spin_unlock_bh(&auddrv_lock);
         }

         break;
      }
      case SET_HEADSET_:
      {
         spin_lock_bh(&auddrv_lock);
         PRINTK_AUDDRV("!! AudDrv SET_HEADSET_ arg:%u \n",arg);
         if(arg){
             Sound_Headset_Turnon();
         }
         else{
             Sound_Headset_Turnoff();
         }
         spin_unlock_bh(&auddrv_lock);
         break;
      }
// record the Speech/Background/Recording state in kernel
      case SET_AUDIO_STATE:
      {
         PRINTK_AUDDRV("AudDrv SET_AUDIO_STATE \n");
         if(copy_from_user((void *)(&SPH_Ctrl_State), (const void __user *)( arg), sizeof(SPH_Control))){
            return -EFAULT;
         }
         printk("AudDrv bBgsFlag:%d,bRecordFlag:%d,bSpeechFlag:%d,bTtyFlag:%d,bVT:%d,bAudio:%d \n",
         SPH_Ctrl_State.bBgsFlag,
         SPH_Ctrl_State.bRecordFlag,
         SPH_Ctrl_State.bSpeechFlag,
         SPH_Ctrl_State.bTtyFlag,
         SPH_Ctrl_State.bVT,
         SPH_Ctrl_State.bAudioPlay);
         break;
      }
      case GET_AUDIO_STATE:
      {
         PRINTK_AUDDRV("AudDrv GET_AUDIO_STATE \n");
         if(copy_to_user((void __user *)arg,(void *)&SPH_Ctrl_State, sizeof(SPH_Control))){
            return -EFAULT;
         }
         PRINTK_AUDDRV("AudDrv bBgsFlag:%d,bRecordFlag:%d,bSpeechFlag:%d,bTtyFlag:%d,bVT:%d,bAudio:%d \n",
         SPH_Ctrl_State.bBgsFlag,
         SPH_Ctrl_State.bRecordFlag,
         SPH_Ctrl_State.bSpeechFlag,
         SPH_Ctrl_State.bTtyFlag,
         SPH_Ctrl_State.bVT,
         SPH_Ctrl_State.bAudioPlay);
         break;
      }
// Set GPIO pin mux
// for MT6620
// typedef enum {
//    COMBO_AUDIO_STATE_0 = 0, /* 0000: BT_PCM_OFF & FM line in/out */
//    COMBO_AUDIO_STATE_1 = 1, /* 0001: BT_PCM_ON & FM line in/out */
//    COMBO_AUDIO_STATE_2 = 2, /* 0010: BT_PCM_OFF & FM I2S */
//    COMBO_AUDIO_STATE_3 = 3, /* 0011: BT_PCM_ON & FM I2S (invalid in 73evb & 1.2 phone configuration) */
//} COMBO_AUDIO_STATE;
#if defined(CONFIG_MTK_COMBO) || defined(CONFIG_MTK_COMBO_MODULE)
      case AUDDRV_RESET_BT_FM_GPIO:
      {
         printk("!! AudDrv, RESET, COMBO_AUDIO_STATE_1 \n");
         mt_combo_audio_ctrl(COMBO_AUDIO_STATE_1);
         break;
      }
      case AUDDRV_SET_BT_PCM_GPIO:
      {
         printk("!! AudDrv, BT PCM, COMBO_AUDIO_STATE_1 \n");
         mt_combo_audio_ctrl(COMBO_AUDIO_STATE_1);
         break;
      }
      case AUDDRV_SET_FM_I2S_GPIO:
      {
         printk("!! AudDrv, FM I2S, COMBO_AUDIO_STATE_2 \n");
         mt_combo_audio_ctrl(COMBO_AUDIO_STATE_2);
         break;
      }
#else
      case AUDDRV_RESET_BT_FM_GPIO:
      {
         printk("!! NoCombo, COMBO_AUDIO_STATE_0 \n");
         break;
      }
      case AUDDRV_SET_BT_PCM_GPIO:
      {
         printk("!! NoCombo, COMBO_AUDIO_STATE_1 \n");
         break;
      }
      case AUDDRV_SET_FM_I2S_GPIO:
      {
         printk("!! NoCombo, COMBO_AUDIO_STATE_2 \n");
         break;
      }
#endif

// Set GPIO pin mux
// for MT519x(ATV) & MT6620(FM) use I2S path for the same project
      case AUDDRV_ENABLE_ATV_I2S_GPIO:
      {
#if defined(MTK_MT5192) || defined(MTK_MT5193)         
         printk("!! AudDrv, AUDDRV_ENABLE_ATV_I2S_GPIO \n");
         cust_matv_gpio_on();
#endif         
         break;
      }
      case AUDDRV_DISABLE_ATV_I2S_GPIO:
      {
#if defined(MTK_MT5192) || defined(MTK_MT5193)         
         printk("!! AudDrv, AUDDRV_DISABLE_ATV_I2S_GPIO \n");
         cust_matv_gpio_off();
#endif         
         break;
      }

      case AUDDRV_MT6573_CHIP_VER:
      {
         if (get_chip_eco_ver() == CHIP_E1)
         {
            // MT6573 E1 chip
            ret = CHIP_E1;
            printk("!! E1(0x%x) AUDDRV_MT6573_CHIP_VER \n",ret );
         }
         else if(get_chip_eco_ver() == CHIP_E2)
         {
            // MT6573 E2 chip or later.
            ret = CHIP_E2;
            printk("!! E2(0x%x) AUDDRV_MT6573_CHIP_VER \n",ret );
         }
         else
         {
            printk("!! No: AUDDRV_MT6573_CHIP_VER \n");
         }
         break;
      }

      case AUDDRV_FM_ANALOG_PATH:
      {
         printk("!! AudDrv, AUDDRV_FM_ANALOG_PATH (%d) \n",arg);
         if(arg==1){
            b_FM_Analog_Enable = true;  
         }
         else if(arg==0){
            b_FM_Analog_Enable = false;            
         }      
         else{
            printk("!! AudDrv, AUDDRV_FM_ANALOG_PATH error (%d) \n",arg);   
         }
	   break;
      }
// for debug
      case AUDDRV_BEE_IOCTL: // this ioctl is only use for debugging
      {
	    	printk("AudDrv AUDDRV_BEE_IOCTL arg:%ld \n",arg);
	    	LouderSPKSound(300);
         break;
      }
      case SET_2IN1_SPEAKER:
      {
         break;
      }
      case YUSU_INFO_FROM_USER:
      {
         _Info_Data InfoData;
         PRINTK_AUDDRV("YUSU_INFO_FROM_USER\n");
      	if(copy_from_user((void *)(&InfoData), (const void __user *)( arg), sizeof(InfoData))){
      	   return -EFAULT;
         }
         spin_lock_bh(&auddrv_lock);

         //call sound extenstion function
         printk("YUSU_INFO_FROM_USER  Info:%d, param1:%d, param2:%d\n",InfoData.info, InfoData.param1, InfoData.param2);
         switch(InfoData.info) {
           case INFO_U2K_MATV_AUDIO_START:
              Sound_ExtFunction("InfoMATVAudioStart", &(InfoData.param1), sizeof(InfoData.param1) + sizeof(InfoData.param2));
              break;
           case INFO_U2K_MATV_AUDIO_STOP:
              Sound_ExtFunction("InfoMATVAudioStop", &(InfoData.param1), sizeof(InfoData.param1) + sizeof(InfoData.param2));
              break;
	//#ifdef HEARING_AID_KERNEL //zml
	   case INFO_U2K_HEARINGAID_ENABLE:
	   	 Sound_ExtFunction("infoHearingAidEnable", &(InfoData.param1), sizeof(InfoData.param1) + sizeof(InfoData.param2));
	   	break;
	   case INFO_U2K_HEARINGAID_DISABLE:
	   	 Sound_ExtFunction("infoHearingAidDisable", &(InfoData.param1), sizeof(InfoData.param1) + sizeof(InfoData.param2));
		 break;
	//#endif
           default:
              break;
         }

         spin_unlock_bh(&auddrv_lock);
         break;
      }
      // use to DAI_OUTPUT
      case AUDDRV_START_DAI_OUTPUT:
      {
          printk("AUDDRV_START_DAI_OUTPUT\n");
          DAI_output_Control_context.status =DAI_INIT;
          break;
      }
      case AUDDRV_STOP_DAI_OUTPUT:
      {
          printk("AUDDRV_STOP_DAI_OUTPUT\n");
          DAI_output_Control_context.status = DAI_STOP;
          Afe_Set_Reg(AFE_DAIBT_CON ,0x0, 0x8); // set data ready
          break;
      }
      case AUDDRV_LOG_PRINT: // this ioctl is only use for debugging
      {
         PRINTK_AUDDRV("AudDrv AUDDRV_LOG_PRINT \n");
         AudDrv_Clk_On();
         AudDrv_AudReg_Log_Print();
         AudDrv_Clk_Off();
         break;
      }
      case AUDDRV_ASSERT_IOCTL:
      {
         printk("AudDrv AUDDRV_ASSERT_IOCTL \n");
         BUG_ON(1);
         break;
      }
      default:
      {
      	printk("AudDrv Fail IOCTL command (%d)\r\n", cmd);
      	break;
      }
	}
   AudDrv_Clk_Off();

	return ret;
}



static kal_uint32 Get_Afe_wait_period(void)
{
    kal_uint32 period =0;
    kal_uint32 samplerate , interrupt_cnt;
    samplerate = Afe_Get_Reg(AFE_IRQ_CON);
    samplerate = samplerate >> 4;
    samplerate = samplerate &0xf;
    switch(samplerate)
    {
        case 0:
            samplerate = 8000;
            break;
        case 1:
            samplerate = 11025;
            break;
        case 2:
            samplerate = 12000;
            break;
        case 4:
            samplerate = 16000;
            break;
        case 5:
            samplerate = 22050;
            break;
        case 6:
            samplerate = 24000;
            break;
        case 8:
            samplerate = 32000;
            break;
        case 9:
            samplerate = 44100;
            break;
        case 10:
            samplerate = 48000;
            break;
        default:{
            printk("Get_Afe_wait_period default samplerate = 44100\n");
            samplerate = 44100;
        }
        break;
    }
    interrupt_cnt = Afe_Get_Reg (AFE_IRQ_CNT1);
    if(interrupt_cnt ==0)
        return 0;
    return (interrupt_cnt * 1000/samplerate)/10*2;

}


static ssize_t AudDrv_write(struct file *fp, const char __user *data, size_t count, loff_t *offset)
{
   //for DL1 output stream write data to HW buffer
   int ret;
   AFE_BLOCK_T  *Afe_Block = NULL;
   ssize_t written_size = count;
//   kal_int32 HW_Cur_ReadIdx = 0;  // for debugging
   kal_int32 copy_size = 0;
   char *data_w_ptr = (char*)data;
   kal_uint32 size_1, size_2,Afe_wait_priod =0;
   PRINTK_AUDDRV("+AudDrv_write cnt:%x \n", count);

   Afe_Block = &(AFE_dl_Control->rBlock);

//   HW_Cur_ReadIdx = Afe_Get_Reg(AFE_DL1_CUR);  // for debugging

   PRINTK_AUDDRV("AudDrv_write WriteIdx=%x, ReadIdx=%x, DataRemained=%x \n",
      Afe_Block->u4WriteIdx, Afe_Block->u4DMAReadIdx,Afe_Block->u4DataRemained);  // audio data count to write

   if(Afe_Block->u4BufferSize == 0){
      printk("AudDrv_write: u4BufferSize=0, Error");
      // do sleep
      msleep(AFE_INT_TIMEOUT);
      return -1;
   }
   // base on samplerate and counter
   Afe_wait_priod =Get_Afe_wait_period()+2;
   if(Afe_wait_priod <= 1){
       Afe_wait_priod = AFE_INT_TIMEOUT;
   }

   while(count)
   {
      spin_lock_bh(&auddrv_lock);   
      copy_size = Afe_Block->u4BufferSize - Afe_Block->u4DataRemained;  //free space of the buffer
      spin_unlock_bh(&auddrv_lock);
      
      PRINTK_AUDDRV("AudDrv_write, copy(%x),DataR(%x) \n",copy_size, Afe_Block->u4DataRemained);  // (free space of buffer)          
      if(count <= (kal_uint32) copy_size)
      {
         copy_size = count;
         PRINTK_AUDDRV("AudDrv_write copy_size:%x \n", copy_size);  // (free space of buffer)
      }

      if(copy_size != 0)
      {
         if(Afe_Block->u4WriteIdx + copy_size < Afe_Block->u4BufferSize ) // copy once
         {
            if(!access_ok(VERIFY_READ,data_w_ptr,copy_size)){
               printk("AudDrv_write 0ptr invalid data_w_ptr=%p, size=%d",data_w_ptr,copy_size);
               printk("AudDrv_write u4BufferSize=%d, u4DataRemained=%d",Afe_Block->u4BufferSize, Afe_Block->u4DataRemained);
            }
            else
            {
               // need check AFE clk
               kal_uint32 aud_top = Afe_Get_Reg(AUDIO_TOP_CON0);
               kal_uint32 afe_clk = (aud_top & 0x4);
               if(afe_clk == 0x4){
                  printk("+AudDrv_write AFE clk(%x) Chia \n",aud_top);
               }
               else{
                  if(copy_from_user((Afe_Block->pucVirtBufAddr+Afe_Block->u4WriteIdx),data_w_ptr,copy_size))
                  {
                     printk("AudDrv_write Fail copy from user");
                     return -1;
                  }
               }
            }
            spin_lock_bh(&auddrv_lock);
            Afe_Block->u4DataRemained += copy_size;
            Afe_Block->u4WriteIdx += copy_size;
            Afe_Block->u4WriteIdx %= Afe_Block->u4BufferSize;
            spin_unlock_bh(&auddrv_lock);
            data_w_ptr += copy_size;
            count -= copy_size;
            PRINTK_AUDDRV("AudDrv_write f1, copy:%x, W:%x, R:%x, DataR:%x, cnt=%x \n",copy_size,Afe_Block->u4WriteIdx,Afe_Block->u4DMAReadIdx,Afe_Block->u4DataRemained,count);
         }
         else  // copy twice
         {
            spin_lock_bh(&auddrv_lock);
            size_1 = Afe_Block->u4BufferSize - Afe_Block->u4WriteIdx;
            if(size_1 > AFE_BUF_SLAVE_SIZE){
               printk("AudDrv_write warning, W(%x), BS(%x) \n",Afe_Block->u4WriteIdx,Afe_Block->u4BufferSize);
               size_1 = AFE_BUF_SLAVE_SIZE - Afe_Block->u4WriteIdx;
            }              
            size_2 = copy_size - size_1;
            spin_unlock_bh(&auddrv_lock);

            PRINTK_AUDDRV("AudDrv_write f2, s1:%x, s2:%x, BZ=%x \n",size_1,size_2,Afe_Block->u4BufferSize);
            
            if(!access_ok (VERIFY_READ,data_w_ptr,size_1)){
                printk("AudDrv_write 1ptr invalid data_w_ptr=%p, size_1=%d", data_w_ptr, size_1);
                printk("AudDrv_write u4BufferSize=%d, u4DataRemained=%d", Afe_Block->u4BufferSize, Afe_Block->u4DataRemained);
            }
            else
            {
               // need check AFE clk
               kal_uint32 aud_top = Afe_Get_Reg(AUDIO_TOP_CON0);
               kal_uint32 afe_clk = (aud_top & 0x4);
               if(afe_clk == 0x4){
                  printk("+AudDrv_write AFE clk(%x) Chia \n",aud_top);
               }
               else{
                  if ((copy_from_user( (Afe_Block->pucVirtBufAddr + Afe_Block->u4WriteIdx), data_w_ptr , size_1)) )
                  {
                     printk("AudDrv_write Fail 1 copy from user");
                     return -1;
                  }
               }
            }
            spin_lock_bh(&auddrv_lock);
            Afe_Block->u4DataRemained += size_1;
            Afe_Block->u4WriteIdx += size_1;
            Afe_Block->u4WriteIdx %= Afe_Block->u4BufferSize;
            spin_unlock_bh(&auddrv_lock);
            if(!access_ok (VERIFY_READ,data_w_ptr+size_1, size_2)){
                printk("AudDrv_write 2ptr invalid data_w_ptr=%p, size_1=%d, size_2=%d", data_w_ptr, size_1, size_2);
                printk("AudDrv_write u4BufferSize=%d, u4DataRemained=%d", Afe_Block->u4BufferSize, Afe_Block->u4DataRemained);
            }
            else
            {
               // need check AFE clk
               kal_uint32 aud_top = Afe_Get_Reg(AUDIO_TOP_CON0);
               kal_uint32 afe_clk = (aud_top & 0x4);
               if(afe_clk == 0x4){
                  printk("+AudDrv_write AFE clk(%x) Chia \n",aud_top);
               }
               else{
                  if ((copy_from_user((Afe_Block->pucVirtBufAddr+Afe_Block->u4WriteIdx),(data_w_ptr+size_1), size_2)))
                  {
                     printk("AudDrv_write Fail 2 copy from user");
                     return -1;
                  }
               }
            }
            spin_lock_bh(&auddrv_lock);
            Afe_Block->u4DataRemained += size_2;
            Afe_Block->u4WriteIdx += size_2;
            Afe_Block->u4WriteIdx %= Afe_Block->u4BufferSize;
            spin_unlock_bh(&auddrv_lock);
            count -= copy_size;
            data_w_ptr += copy_size;
            PRINTK_AUDDRV("AudDrv_write f2, copy:%x, W:%x, R:%x, DataR:%x \n",copy_size,Afe_Block->u4WriteIdx,Afe_Block->u4DMAReadIdx,Afe_Block->u4DataRemained );
         }
      }

      if(count != 0)
      {
         PRINTK_AUDDRV("AudDrv_write wait int count=%x \n",count);
         wait_queue_flag =0;
         ret = wait_event_interruptible_timeout(DL_Wait_Queue, wait_queue_flag,Afe_wait_priod); // 100ms
         if(ret <= 0)
         {
            kal_uint32 irq_mcu     = Afe_Get_Reg(AFE_IRQ_CON);
            kal_uint32 irq1_mcu_on = irq_mcu & 0x1;             
            if(irq1_mcu_on == 0x1){
               printk("AudDrv_write wait timeout,[Warning]blocking by others..(0x%x)!\n",irq_mcu);
               printk("AudDrv_write wait,copy:%x,Widx:%x,Ridx=%x,DataR:%x,cnt=%x \r",copy_size,Afe_Block->u4WriteIdx,Afe_Block->u4DMAReadIdx,Afe_Block->u4DataRemained,count);

               Afe_Set_Reg(AFE_IRQ_CON,0x0,0x1);
               Afe_Set_Reg(AFE_DAC_CON0,0x0,0x2);  // DL1_ON=0
               spin_lock_bh(&auddrv_lock);
               AFE_dl_Control->rBlock.u4WriteIdx	  = 0;
               AFE_dl_Control->rBlock.u4DMAReadIdx	  = 0;
               AFE_dl_Control->rBlock.u4DataRemained = AFE_BUF_SLAVE_SIZE;
               AFE_dl_Control->rBlock.u4fsyncflag    = false;
               spin_unlock_bh(&auddrv_lock);
               printk("AudDrv_write wait timeout,restart \n");               
               Afe_Set_Reg(AFE_DAC_CON0,0x2,0x2);
               Afe_Set_Reg(AFE_IRQ_CON,0x1,0x1);                  
            }
            else{
               printk("AudDrv_write wait timeout, No Aud Int (0x%x)!\n",irq_mcu);
            }

//            AudDrv_AudReg_Log_Print();
            return written_size;
         }
      }
   }

   return written_size;
}

ssize_t AudDrv_AWB_Read(struct file *fp,  char __user *data, size_t count, loff_t *offset){

    AFE_BLOCK_T  *AWB_Block = NULL;
    int ret = 0;
    char *Read_Data_Ptr = (char*)data;
    ssize_t DMA_Read_Ptr =0 , read_size = 0,read_count = 0;
    AWB_Block = &(AWB_input_Control->rBlock);

    printk("+AudDrv_AWB_Read count:%x \n", count);  //I2S input data count that can be copy_block
    if(AWB_Block->u4BufferSize == 0){
        printk("AudDrv_AWB_Read: u4BufferSize=0, Error");
        msleep(AWB_TIMER_INTERVAL);
        return -1;
    }

    while(count)
    {
        if(AWB_Block->pucPhysBufAddr == 0){
            printk("AudDrv_read pucPhysBufAddr == NULL \n");
            break;
        }
        spin_lock_bh(&auddrv_lock);
        if( AWB_Block->u4DataRemained >  AWB_Block->u4BufferSize){
            printk("AudDrv_AWB_Read u4DataRemained=%x > u4BufferSize=%x" ,AWB_Block->u4DataRemained, AWB_Block->u4BufferSize);
            AWB_Block->u4DataRemained = 0;
            AWB_Block->u4DMAReadIdx   = AWB_Block->u4WriteIdx;
        }
        if(count >  AWB_Block->u4DataRemained){
            read_size = AWB_Block->u4DataRemained;
        }
        else{
            read_size = count;
        }
        DMA_Read_Ptr = AWB_Block->u4DMAReadIdx;
        spin_unlock_bh(&auddrv_lock);

        printk("AudDrv_read finish0, read_count:%x, read_size:%x, u4DataRemained:%x, u4DMAReadIdx:0x%x, u4WriteIdx:%x \r\n",
            read_count,read_size,AWB_Block->u4DataRemained,AWB_Block->u4DMAReadIdx,AWB_Block->u4WriteIdx);
        if(DMA_Read_Ptr+read_size < AWB_Block->u4BufferSize)
        {
        #ifndef SOUND_FAKE_READ
            if(DMA_Read_Ptr != AWB_Block->u4DMAReadIdx){
                printk("AudDrv_read 1, read_size:%x, DataRemained:%x, DMA_Read_Ptr:0x%x, DMAReadIdx:%x \r\n",
                    read_size,AWB_Block->u4DataRemained,DMA_Read_Ptr,AWB_Block->u4DMAReadIdx);
            }
            if(copy_to_user((void __user *)Read_Data_Ptr,(AWB_Block->pucVirtBufAddr+DMA_Read_Ptr),read_size))
            {
                printk("AudDrv_read Fail 1 copy to user Read_Data_Ptr:%p, pucVirtBufAddr:%p, u4DMAReadIdx:0x%x, DMA_Read_Ptr:0x%x, read_size:%x",
                Read_Data_Ptr,AWB_Block->pucVirtBufAddr, AWB_Block->u4DMAReadIdx, DMA_Read_Ptr, read_size);
            if(read_count == 0){
                return -1;
            }
            else {
                return read_count;
            }
         }
        #else
            copy_to_user_fake(Read_Data_Ptr ,read_size);
        #endif
         read_count += read_size;
            spin_lock_bh(&auddrv_lock);
            AWB_Block->u4DataRemained -= read_size;
            AWB_Block->u4DMAReadIdx += read_size;
            AWB_Block->u4DMAReadIdx %= AWB_Block->u4BufferSize;
            DMA_Read_Ptr = AWB_Block->u4DMAReadIdx;
            spin_unlock_bh(&auddrv_lock);
            Read_Data_Ptr += read_size;
            count -= read_size;
            PRINTK_AUDDRV("AudDrv_AWB_Read finish1, copy size:%x, u4DMAReadIdx:0x%x, u4WriteIdx:%x, u4DataRemained:%x \r\n",read_size,I2Sin_Block->u4DMAReadIdx,I2Sin_Block->u4WriteIdx,I2Sin_Block->u4DataRemained );
        }
        else
        {
            kal_uint32 size_1 = AWB_Block->u4BufferSize - DMA_Read_Ptr;
            kal_uint32 size_2 = read_size - size_1;
        #ifndef SOUND_FAKE_READ
            if(DMA_Read_Ptr != AWB_Block->u4DMAReadIdx){
                printk("AudDrv_AWB_Read 2, read_size1:%x, DataRemained:%x, DMA_Read_Ptr:0x%x, DMAReadIdx:%x \r\n",
                    size_1,AWB_Block->u4DataRemained,DMA_Read_Ptr,AWB_Block->u4DMAReadIdx);
            }
            if (copy_to_user( (void __user *)Read_Data_Ptr,(AWB_Block->pucVirtBufAddr+DMA_Read_Ptr),size_1))
            {
                printk("AudDrv_AWB_Read Fail 2 copy to user Read_Data_Ptr:%p, pucVirtBufAddr:%p, u4DMAReadIdx:0x%x, DMA_Read_Ptr:0x%x, read_size:%x",
                Read_Data_Ptr,AWB_Block->pucVirtBufAddr, AWB_Block->u4DMAReadIdx, DMA_Read_Ptr, read_size);
            if(read_count == 0){
                return -1;
            }
            else {
                return read_count;
            }
	 }
        #else
            copy_to_user_fake(Read_Data_Ptr,size_1);
        #endif
         read_count += size_1;

            spin_lock_bh(&auddrv_lock);
            AWB_Block->u4DataRemained -= size_1;
            AWB_Block->u4DMAReadIdx += size_1;
            AWB_Block->u4DMAReadIdx %= AWB_Block->u4BufferSize;
            DMA_Read_Ptr = AWB_Block->u4DMAReadIdx;
            spin_unlock_bh(&auddrv_lock);
            printk("AudDrv_read finish2, copy size_1:%x, u4DMAReadIdx:0x%x, u4WriteIdx:0x%x, u4DataRemained:%x \r\n",
                size_1,AWB_Block->u4DMAReadIdx,AWB_Block->u4WriteIdx,AWB_Block->u4DataRemained );
        #ifndef SOUND_FAKE_READ
            if(DMA_Read_Ptr != AWB_Block->u4DMAReadIdx){
                printk("AudDrv_AWB_Read 3, read_size2:%x, DataRemained:%x, DMA_Read_Ptr:0x%x, DMAReadIdx:%x \r\n",
                    size_2,AWB_Block->u4DataRemained,DMA_Read_Ptr,AWB_Block->u4DMAReadIdx);
            }
            if(copy_to_user(  (void __user *)(Read_Data_Ptr+size_1),(AWB_Block->pucVirtBufAddr + DMA_Read_Ptr), size_2))
            {
                printk("AudDrv_AWB_Read Fail 3 copy to user Read_Data_Ptr:%p, pucVirtBufAddr:%p, u4DMAReadIdx:0x%x, DMA_Read_Ptr:0x%x, read_size:%x",
                    Read_Data_Ptr, AWB_Block->pucVirtBufAddr, AWB_Block->u4DMAReadIdx, DMA_Read_Ptr, read_size);
            if(read_count == 0){
                return -1;
            }
            else {
                return read_count;
            }
         }
        #else
            copy_to_user_fake((Read_Data_Ptr+size_1),size_2);
        #endif
         read_count += size_2;

            spin_lock_bh(&auddrv_lock);
            AWB_Block->u4DataRemained -= size_2;
            AWB_Block->u4DMAReadIdx += size_2;
            DMA_Read_Ptr = AWB_Block->u4DMAReadIdx;
            spin_unlock_bh(&auddrv_lock);
            count -= read_size;
            Read_Data_Ptr += read_size;
            printk("AudDrv_AWB_Read finish3, copy size_2:%x, u4DMAReadIdx:0x%x, u4WriteIdx:0x%x u4DataRemained:%x \r\n",
                size_2,AWB_Block->u4DMAReadIdx,AWB_Block->u4WriteIdx,AWB_Block->u4DataRemained );
        }
        if(count != 0){
            printk("AudDrv_AWB_Read wait for interrupt signal\n");
            AWB_wait_queue_flag =0;
            ret = wait_event_interruptible_timeout(AWB_Wait_Queue,AWB_wait_queue_flag,AWB_TIMER_TIMEOUT);
            if(ret <= 0)
            {
                printk("AudDrv_AWB_Read wait_event_interruptible_timeout, No Audio Interrupt! \n");
                return read_count;
            }
        }
    }
   return read_count;
}

ssize_t AudDrv_I2S_Read(struct file *fp,  char __user *data, size_t count, loff_t *offset)
{
   //for I2S input stream read data to HW buffer
   AFE_BLOCK_T  *I2Sin_Block = NULL;
   int ret;
   char *Read_Data_Ptr = (char*)data;
   int DMA_Read_Ptr=0;
   ssize_t read_size = 0;
   ssize_t read_count = 0;
   PRINTK_AUDDRV("+AudDrv_I2S_Read count:%x \n", count);  //I2S input data count that can be copy_block
   PRINTK_AUDDRV("+AudDrv_read count:0x%x \n", count);  //I2S input data count that can be copy

   I2Sin_Block = &(I2S_input_Control->rBlock);

   if(I2Sin_Block->u4BufferSize == 0){
      printk("AudDrv_read: u4BufferSize=0, Error");
      msleep(AFE_INT_TIMEOUT);
      return -1;
   }

   while(count)
   {
      if(I2Sin_Block->pucPhysBufAddr == 0){
         printk("AudDrv_read pucPhysBufAddr == NULL \n");
         break;
      }
      spin_lock_bh(&auddrv_lock);
      if( I2Sin_Block->u4DataRemained >  I2Sin_Block->u4BufferSize){
         printk("AudDrv_read u4DataRemained=%x > u4BufferSize=%x" ,I2Sin_Block->u4DataRemained, I2Sin_Block->u4BufferSize);
         I2Sin_Block->u4DataRemained = 0;
         I2Sin_Block->u4DMAReadIdx   = I2Sin_Block->u4WriteIdx;
      }
      if(count >  I2Sin_Block->u4DataRemained){    // u4DataRemained --> data store in HW buffer
         read_size = I2Sin_Block->u4DataRemained;
      }
      else{
         read_size = count;
      }
      DMA_Read_Ptr = I2Sin_Block->u4DMAReadIdx;
      spin_unlock_bh(&auddrv_lock);


      PRINTK_AUDDRV("AudDrv_read finish0, read_count:%x, read_size:%x, u4DataRemained:%x, u4DMAReadIdx:0x%x, u4WriteIdx:%x \r\n",read_count,read_size,I2Sin_Block->u4DataRemained,I2Sin_Block->u4DMAReadIdx,I2Sin_Block->u4WriteIdx);

      if(DMA_Read_Ptr+read_size < I2Sin_Block->u4BufferSize)
      {
#ifndef SOUND_FAKE_READ
         if(DMA_Read_Ptr != I2Sin_Block->u4DMAReadIdx){
            printk("AudDrv_read 1, read_size:%x, DataRemained:%x, DMA_Read_Ptr:0x%x, DMAReadIdx:%x \r\n",read_size,I2Sin_Block->u4DataRemained,DMA_Read_Ptr,I2Sin_Block->u4DMAReadIdx);
         }
         if(copy_to_user((void __user *)Read_Data_Ptr,(I2Sin_Block->pucVirtBufAddr+DMA_Read_Ptr),read_size))
         {
            printk("AudDrv_read Fail 1 copy to user Read_Data_Ptr:%p, pucVirtBufAddr:%p, u4DMAReadIdx:0x%x, DMA_Read_Ptr:0x%x, read_size:%x",
                     Read_Data_Ptr,I2Sin_Block->pucVirtBufAddr, I2Sin_Block->u4DMAReadIdx, DMA_Read_Ptr, read_size);
            if(read_count == 0){
            return -1;
         }
            else {
                return read_count;
            }
         }
#else
         copy_to_user_fake(Read_Data_Ptr ,read_size);
#endif
         read_count += read_size;
         spin_lock_bh(&auddrv_lock);
         I2Sin_Block->u4DataRemained -= read_size;
         I2Sin_Block->u4DMAReadIdx += read_size;
         I2Sin_Block->u4DMAReadIdx %= I2Sin_Block->u4BufferSize;
         DMA_Read_Ptr = I2Sin_Block->u4DMAReadIdx;
         spin_unlock_bh(&auddrv_lock);
         Read_Data_Ptr += read_size;
         count -= read_size;
         PRINTK_AUDDRV("AudDrv_read finish1, copy size:%x, u4DMAReadIdx:0x%x, u4WriteIdx:%x, u4DataRemained:%x \r\n",read_size,I2Sin_Block->u4DMAReadIdx,I2Sin_Block->u4WriteIdx,I2Sin_Block->u4DataRemained );
      }
      else
      {
         kal_uint32 size_1 = I2Sin_Block->u4BufferSize - DMA_Read_Ptr;
         kal_uint32 size_2 = read_size - size_1;

#ifndef SOUND_FAKE_READ
         if(DMA_Read_Ptr != I2Sin_Block->u4DMAReadIdx){
            printk("AudDrv_read 2, read_size1:%x, DataRemained:%x, DMA_Read_Ptr:0x%x, DMAReadIdx:%x \r\n",size_1,I2Sin_Block->u4DataRemained,DMA_Read_Ptr,I2Sin_Block->u4DMAReadIdx);
         }
         if(copy_to_user( (void __user *)Read_Data_Ptr,(I2Sin_Block->pucVirtBufAddr+DMA_Read_Ptr),size_1))
         {
            printk("AudDrv_read Fail 2 copy to user Read_Data_Ptr:%p, pucVirtBufAddr:%p, u4DMAReadIdx:0x%x, DMA_Read_Ptr:0x%x, read_size:%x",
                     Read_Data_Ptr,I2Sin_Block->pucVirtBufAddr, I2Sin_Block->u4DMAReadIdx, DMA_Read_Ptr, read_size);
            if(read_count == 0){
	         return -1;
	      }
            else {
                return read_count;
            }
	 }
#else
	      copy_to_user_fake(Read_Data_Ptr,size_1);
#endif
         read_count += size_1;

         spin_lock_bh(&auddrv_lock);
         I2Sin_Block->u4DataRemained -= size_1;
         I2Sin_Block->u4DMAReadIdx += size_1;
         I2Sin_Block->u4DMAReadIdx %= I2Sin_Block->u4BufferSize;
         DMA_Read_Ptr = I2Sin_Block->u4DMAReadIdx;
         spin_unlock_bh(&auddrv_lock);

         PRINTK_AUDDRV("AudDrv_read finish2, copy size_1:%x, u4DMAReadIdx:0x%x, u4WriteIdx:0x%x, u4DataRemained:%x \r\n",size_1,I2Sin_Block->u4DMAReadIdx,I2Sin_Block->u4WriteIdx,I2Sin_Block->u4DataRemained );

#ifndef SOUND_FAKE_READ
         if(DMA_Read_Ptr != I2Sin_Block->u4DMAReadIdx){
            printk("AudDrv_read 3, read_size2:%x, DataRemained:%x, DMA_Read_Ptr:0x%x, DMAReadIdx:%x \r\n",size_2,I2Sin_Block->u4DataRemained,DMA_Read_Ptr,I2Sin_Block->u4DMAReadIdx);
         }
         if (copy_to_user(  (void __user *)(Read_Data_Ptr+size_1),(I2Sin_Block->pucVirtBufAddr + DMA_Read_Ptr), size_2))
         {
            printk("AudDrv_read Fail 3 copy to user Read_Data_Ptr:%p, pucVirtBufAddr:%p, u4DMAReadIdx:0x%x, DMA_Read_Ptr:0x%x, read_size:%x",
                     Read_Data_Ptr, I2Sin_Block->pucVirtBufAddr, I2Sin_Block->u4DMAReadIdx, DMA_Read_Ptr, read_size);
            if(read_count == 0){
            return -1;
         }
            else {
                return read_count;
            }
         }
#else
         copy_to_user_fake((Read_Data_Ptr+size_1),size_2);
#endif
         read_count += size_2;

         spin_lock_bh(&auddrv_lock);
         I2Sin_Block->u4DataRemained -= size_2;
         I2Sin_Block->u4DMAReadIdx += size_2;
         DMA_Read_Ptr = I2Sin_Block->u4DMAReadIdx;
         spin_unlock_bh(&auddrv_lock);

         count -= read_size;
         Read_Data_Ptr += read_size;
         PRINTK_AUDDRV("AudDrv_read finish3, copy size_2:%x, u4DMAReadIdx:0x%x, u4WriteIdx:0x%x u4DataRemained:%x \r\n",size_2,I2Sin_Block->u4DMAReadIdx,I2Sin_Block->u4WriteIdx,I2Sin_Block->u4DataRemained );
      }

      if(count != 0){
         //PRINTK_AUDDRV("read wait for interrupt signal\n");
         I2S_wait_queue_flag =0;
         ret = wait_event_interruptible_timeout(I2Sin_Wait_Queue,I2S_wait_queue_flag,I2S_INPUT_INT_TIMEOUT);
         if(ret <= 0)
         {
            printk("AudDrv_read wait_event_interruptible_timeout, No Audio Interrupt! \n");
            return read_count;
         }
      }
   }

   PRINTK_AUDDRV("-AudDrv_read, 0x%x, 0x%x, 0x%x, 0x%x \n",*Read_Data_Ptr,*(Read_Data_Ptr+1),*(Read_Data_Ptr+2),*(Read_Data_Ptr+3));
   PRINTK_AUDDRV("-AudDrv_read, read_count:%x \n",read_count);
   return read_count;
}

static ssize_t AudDrv_read(struct file *fp,  char __user *data, size_t count, loff_t *offset)
{
    kal_uint32 read_count =0;
    if(fp == I2S_input_Control->rBlock.flip){
        read_count = AudDrv_I2S_Read(fp, data,  count, offset);
    }
    else if(fp == AWB_input_Control->rBlock.flip){
        read_count = AudDrv_AWB_Read(fp,data, count, offset);
    }
    else{
        printk("no match fp to read !!!!\n");
    }
    return read_count;
}

static int AudDrv_Reset(void)
{
   printk("+AudDrv_Reset \n");

   if(AFE_dl_Control->rBlock.pucVirtBufAddr != NULL){
      if(!Flag_Aud_DL1_SlaveOn)
      {//Audio DL1 master mode used
#ifdef DL1_USE_SYSRAM
         MT6573_SYSRAM_FREE(MT6573SYSRAMUSR_MFLEXVIDEO);
#elif (!defined (DL1_USE_FLEXL2))
////////////
// Use DMA free coherent
         dma_free_coherent(0, AFE_dl_Control->rBlock.u4BufferSize, AFE_dl_Control->rBlock.pucVirtBufAddr, AFE_dl_Control->rBlock.pucPhysBufAddr);
#endif
      }
   }

   AFE_dl_Control->rBlock.u4SampleNumMask = 0;
   AFE_dl_Control->rBlock.u4WriteIdx	   = 0;
   AFE_dl_Control->rBlock.u4DMAReadIdx    = 0;
   AFE_dl_Control->rBlock.u4DataRemained  = 0;
   AFE_dl_Control->rBlock.u4fsyncflag     = false;

   if(I2S_input_Control->rBlock.pucVirtBufAddr != NULL){

#if (!defined (I2SIN_USE_FLEXL2))
////////////
// Use DMA free coherent
      printk("+AudDrv_DeInit_I2S_InputStream I2S_input_Control->rBlock.u4BufferSize=%d , sizex1\n", I2S_input_Control->rBlock.u4BufferSize);
//      dma_free_coherent(0, I2S_input_Control->rBlock.u4BufferSize, I2S_input_Control->rBlock.pucVirtBufAddr, I2S_input_Control->rBlock.pucPhysBufAddr);
#endif
   }

   I2S_input_Control->rBlock.u4SampleNumMask = 0;
   I2S_input_Control->rBlock.u4WriteIdx	   = 0;
   I2S_input_Control->rBlock.u4DMAReadIdx    = 0;
   I2S_input_Control->rBlock.u4DataRemained  = 0;
   I2S_input_Control->rBlock.flip                       = NULL;
   I2S_input_Control->rBlock.u4fsyncflag    = false;

   //Reset Amp Flag
   AMP_Flag = false;

   // if AWB is enable , reset AWB stream
   if(Aud_AWB_Clk_cntr){
       Auddrv_AWB_timer_off();
       AudDrv_Reset_AWB_Stream ();
       wake_unlock(&Audio_record_wake_lock);
   }

   return 1;
}

static int AudDrv_flush(struct file *flip, fl_owner_t id)
{
   PRINTK_AUDDRV("+AudDrv_flush \n");
   /*
   // flush will set all register to default
   AMP_Flag = false;
   Afe_Mem_Pwr_on = 0;
   Aud_AFE_Clk_cntr =0;
   AudDrv_Reset();

   // TBD.. turn off power
   AudDrv_Clk_Off();  // turn off asm afe clock
//   Afe_Disable_Memory_Power ();	//disable memory buffer
*/

   if( (SPH_Ctrl_State.bSpeechFlag==false) && (SPH_Ctrl_State.bVT==false) && (SPH_Ctrl_State.bRecordFlag==false) ){
      // Disable interrupt (timer)
      printk("AudDrv_flush, Disable DL_SRC2, IRQ \n");
      Afe_Set_Reg(AFE_DL_SRC2_1,0x0,0x1);   // DL2_SRC2_ON=0
      Afe_Set_Reg(AFE_IRQ_CON,0x0,0x1);
      Afe_Set_Reg(AFE_DAC_CON0,0x0,0x2);  // DL1_ON=0      
      Afe_Set_Reg(AFE_IR_CLR,0x1,0x1);
      Afe_Set_Reg(AFE_IRQ_CON,0x0,0x2);  // bit1: I2S,  IRQ2_MCU_ON
      Afe_Set_Reg(AFE_IR_CLR,0x2,0x2);           // bit1: I2S,  IRQ1_MCU_CLR
   }

   Aud_Flush_cntr++;

   printk("-AudDrv_flush cntr(%d)\n", Aud_Flush_cntr);
   return 0;
}

static int AudDrv_fasync(int fd, struct file *flip, int mode)
{
   PRINTK_AUDDRV("AudDrv_fasync \n");
   return fasync_helper(fd,flip,mode,&AudDrv_async);
}


void AudDrv_vma_open(struct vm_area_struct *vma)
{
   printk("AudDrv_vma_open virt:%lx, phys:%lx, length:%lx \n",vma->vm_start, vma->vm_pgoff<<PAGE_SHIFT,vma->vm_end - vma->vm_start);
}

void AudDrv_vma_close(struct vm_area_struct *vma)
{
   printk("AudDrv_vma_close virt");
}

static struct vm_operations_struct AudDrv_remap_vm_ops =
{
   .open  = AudDrv_vma_open,
   .close = AudDrv_vma_close
};

static int AudDrv_remap_mmap(struct file *flip, struct vm_area_struct *vma)
{
	PRINTK_AUDDRV("AudDrv_remap_mmap \n");

   vma->vm_pgoff =( AFE_dl_Control->rBlock.pucPhysBufAddr)>>PAGE_SHIFT;
   if(remap_pfn_range(vma , vma->vm_start , vma->vm_pgoff ,
   	vma->vm_end - vma->vm_start , vma->vm_page_prot) < 0)
   {
      printk("AudDrv_remap_mmap remap_pfn_range Fail \n");
      return -EIO;
   }
   vma->vm_ops = &AudDrv_remap_vm_ops;
   AudDrv_vma_open(vma);
   return 0;
}


static struct file_operations AudDrv_fops = {
   .owner   = THIS_MODULE,
   .open    = AudDrv_open,
   .release = AudDrv_release,
   .unlocked_ioctl   = AudDrv_ioctl,
   .write   = AudDrv_write,
   .read   	= AudDrv_read,
   .flush   = AudDrv_flush,
   .fasync	= AudDrv_fasync,
   .mmap    = AudDrv_remap_mmap
};

static struct miscdevice AudDrv_audio_device = {
   .minor = MISC_DYNAMIC_MINOR,
   .name = "eac",
   .fops = &AudDrv_fops,
};

void AudDrv_Suspend_Clk_On(void)
{
   printk("AudDrv_Suspend_Clk_On AFE(%d) \n",Aud_AFE_Clk_cntr);
   if(Aud_AFE_Clk_cntr>0)
   {
#ifdef USE_PM_API
      if(hwEnableClock(MT65XX_PDN_AUDIO_AFE,"AUDIO")==false)
         printk("Aud hwEnableClock MT65XX_PDN_AUDIO_AFE fail");
      if(Aud_ADC_Clk_cntr>0)
      {
         if(hwEnableClock(MT65XX_PDN_AUDIO_ADC,"AUDIO")==false)
            printk("Aud hwEnableClock MT65XX_PDN_AUDIO_ADC fail");
      }

      if(Aud_I2S_Clk_cntr>0)
      {
         if(hwEnableClock(MT65XX_PDN_AUDIO_I2S,"AUDIO")==false)
            printk("Aud hwEnableClock MT65XX_PDN_AUDIO_I2S fail");
      }
      // Enable AFE clock
      Ana_Set_Reg(PLL_CON2,0x20,0x20);             // turn on AFE (26M Clock)
      Ana_Set_Reg(AUDIO_CON3,0x0,0x2);             // Enable Audio Bias (VaudlbiasDistrib)
      Afe_Set_Reg(AUDIO_TOP_CON0, 0x0, 0x00010000);  // bit16: power on AFE_CK_DIV_RST
#else
      Afe_Set_Reg(AUDIO_TOP_CON0,0x0, 0xffffffff);  // bit2: power down AFE clock
#endif
   }
}

void AudDrv_Suspend_Clk_Off(void)
{
   printk("AudDrv_Suspend_Clk_Off AFE(%d) \n",Aud_AFE_Clk_cntr);
   if(Aud_AFE_Clk_cntr>0)
   {
      // Disable AFE clock
      Ana_Set_Reg(AUDIO_CON3,0x2,0x2);       // Disable Audio Bias (VaudlbiasDistrib power down)
#ifdef USE_PM_API
      Afe_Set_Reg(AUDIO_TOP_CON0, 0x00010000, 0x00010000);  // bit16: power off AFE_CK_DIV_RST

      if(hwDisableClock(MT65XX_PDN_AUDIO_AFE,"AUDIO")==false)
         printk("hwDisableClock MT65XX_PDN_AUDIO_AFE fail");

      if(Aud_ADC_Clk_cntr>0)
      {
         if(hwDisableClock(MT65XX_PDN_AUDIO_ADC,"AUDIO")==false)
         printk("hwDisableClock MT65XX_PDN_AUDIO_ADC fail");
      }
      if(Aud_I2S_Clk_cntr>0)
      {
         if(hwDisableClock(MT65XX_PDN_AUDIO_I2S,"AUDIO")==false)
         printk("hwDisableClock MT65XX_PDN_AUDIO_I2S fail");
      }
#else
      Afe_Set_Reg(AUDIO_TOP_CON0, 0x10074, 0x10074);   // bit2: power down AFE clock
#endif
   }
}


static int AudDrv_probe(struct platform_device *dev)
{
   int ret = 0;
   int value = 0;
   printk("+AudDrv_probe \n");

   //power on
   PRINTK_AUDDRV("AudDrv_probe +AudDrv_Clk_On\n");
   Flag_AudDrv_ClkOn_1st = true;
   AudDrv_Clk_On();
   Flag_AudDrv_ClkOn_1st = false;
   PRINTK_AUDDRV("AudDrv_probe -AudDrv_Clk_On\n");

   Speaker_Init();

   Afe_Set_Reg(AFE_IRQ_CON   ,0x0,0xffffffff);
   Afe_Set_Reg(AFE_IR_STATUS ,0x0,0xffffffff);
   Afe_Set_Reg(AFE_IR_CLR    ,0x0,0xffffffff);
   Afe_Set_Reg(AFE_IRQ_CNT1  ,0x0,0xffffffff);
   Afe_Set_Reg(AFE_IRQ_CNT2  ,0x0,0xffffffff);
   Afe_Set_Reg(AFE_IRQ_MON   ,0x0,0xffffffff);

   // Init Analog Register
   Ana_Set_Reg(AUDIO_CON0,0x0000,0xffff);
   Ana_Set_Reg(AUDIO_CON1,0x000C,0xffff);
   Ana_Set_Reg(AUDIO_CON2,0x000C,0xffff);
   Ana_Set_Reg(AUDIO_CON4,0x1818,0xffff);
   Ana_Set_Reg(AUDIO_CON5,0x0440,0xffff);
   Ana_Set_Reg(AUDIO_CON6,0x001B,0xffff);
   Ana_Set_Reg(AUDIO_CON7,0x0400,0xffff);  //DRTZ 0; disable HP amp startup mode
   Ana_Set_Reg(AUDIO_CON8,0x7000,0xffff);  //Disable short circuit protection
   Ana_Set_Reg(AUDIO_CON9,0x0003,0xffff);
   Ana_Set_Reg(AUDIO_CON10,0x0020,0xffff); // Zero-Padding off
   Ana_Set_Reg(AUDIO_CON23,0x0000,0x8800); // power down Analog ADC

   Ana_Set_Reg(VAUDN_CON0,0x0000,0xffff);//Initialized to power off
   Ana_Set_Reg(VAUDP_CON0,0x0000,0xffff);//Initialized to power off
   Ana_Set_Reg(VAUDP_CON1,0x0000,0xffff);
   Ana_Set_Reg(VAUDP_CON2,0x012B,0xffff);
   //hwPowerDown(MT65XX_POWER_LDO_VMIC, "Audio"); //tina remove
   //Ana_Set_Reg(VMIC_CON0,0x0000,0xffff);
   //Ana_Set_Reg(VA12_CON0,0x000D,0xffff);
   //Ana_Set_Reg(VA25_CON0,0x0001,0xffff);

   //Initialize AFE Register
   //Power on all HW modules
   Afe_Set_Reg(AUDIO_TOP_CON0, 0x0, 0xffffffff);

   Afe_Set_Reg(AFE_DL_SDM_CON0, 0x08800000, 0xffffffff);
   Afe_Set_Reg(AFE_SDM_GAIN_STAGE, 0x0000001e, 0xffffffff);

   // Reset AudioSys HW registers
   Afe_Set_Reg(AFE_DAC_CON0 ,0x0, 0xffffffff);
   Afe_Set_Reg(AFE_DAC_CON1 ,0x0, 0xffffffff);

   Afe_Set_Reg(AFE_CONN0,0x0,0xffffffff);
   Afe_Set_Reg(AFE_CONN1,0x0,0xffffffff);
   Afe_Set_Reg(AFE_CONN2,0x0,0xffffffff);
   Afe_Set_Reg(AFE_CONN3,0x0,0xffffffff);
   Afe_Set_Reg(AFE_CONN4,0x80000000,0xffffffff);

   Afe_Set_Reg(AFE_DL1_BASE ,0x0, 0xffffffff);
   Afe_Set_Reg(AFE_DL1_CUR  ,0x0, 0xffffffff);
   Afe_Set_Reg(AFE_DL1_END  ,0x0, 0xffffffff);
   Afe_Set_Reg(AFE_DL2_BASE ,0x0, 0xffffffff);
   Afe_Set_Reg(AFE_DL2_CUR  ,0x0, 0xffffffff);
   Afe_Set_Reg(AFE_DL2_END  ,0x0, 0xffffffff);
   Afe_Set_Reg(AFE_I2S_BASE ,0x0, 0xffffffff);
   Afe_Set_Reg(AFE_I2S_CUR  ,0x0, 0xffffffff);
   Afe_Set_Reg(AFE_I2S_END  ,0x0, 0xffffffff);

   Afe_Set_Reg(AFE_DL_SRC1_1,0x1, 0xffffffff);  // for MT6573 E1, must enable DL1_SRC_ON

   Afe_Set_Reg(AFE_DL_SRC2_1,0x0, 0xffffffff);
   Afe_Set_Reg(AFE_DL_SRC2_2,0xffff, 0xffffffff);

   value = 0;
   value=(value & ( ~(0x3<<24) )) | (0x3 << 24);  //up-sampleing x8   bit24 ~ bit25
   Afe_Set_Reg(AFE_DL_SRC2_1 ,value,0x03000000);

   /* Set default mute speed */
   value = 0;
   value=(value & ( ~(0x3<<9) )) | (0x3 << 9);   // bit9 ~ bit10
   Afe_Set_Reg(AFE_DL_SRC2_1 ,value,0x0600);

   Speaker_Init();

   // register IRQ line
   ret = request_irq(MT6573_AFE_MCU_IRQ_LINE, AudDrv_IRQ_handler, 0, "Afe_ISR_Handle", dev);
   if(ret < 0 ){
      printk("AudDrv_probe request_irq Fail \n");
   }

   PRINTK_AUDDRV("AudDrv_probe +AudDrv_Clk_Off\n");
   AudDrv_Clk_Off();
   PRINTK_AUDDRV("AudDrv_probe -AudDrv_Clk_Off\n");
   AudSRAMVirtBufAddr = (kal_uint8*)ioremap_nocache( 0xD4000000 , (unsigned long)(AFE_DL1_SYSRAM_END-AFE_DL1_SYSRAM_BASE) );  // use ioremap to map sram

#if defined(I2SIN_USE_FLEXL2)
   ////////////
   // Use SysRam (FlexL2)
   printk("AudDrv_probe FlexL2 \n");
   I2SInPhyBufAddr = 0x90010000;
   I2SInVirtBufAddr = 0xf9001000;
#else
   ////////////
   // Use DMA alloc coherent
   I2SInPhyBufAddr = 0;
   I2SInVirtBufAddr = NULL;

   I2SInVirtBufAddr = dma_alloc_coherent(0, 9216, &I2SInPhyBufAddr, GFP_KERNEL);
   if((0 == I2SInPhyBufAddr)||(NULL == I2SInVirtBufAddr)){
      printk("I2SInPhyBufAddr dma_alloc_coherent fail \n");
   }
#endif
    //allocate memory for AWB , high sample rate record
    memset((void*)&AWB_input_Control_context,0,sizeof(AWB_INPUT_CONTROL_T));
    if(AWB_input_Control->rBlock.pucPhysBufAddr == 0){
        AudDrv_allcate_AWB_buffer ();
    }
    PRINTK_AUDDRV("init AWB_timer\n");
    init_timer(&AWB_timer);

   printk("-AudDrv_probe \n");
   return 0;
}

static int AudDrv_suspend(struct platform_device *dev, pm_message_t state)  // only one suspend mode
{
   kal_uint32 Reg_VAUDN_CON0, Reg_VAUDP_CON0, Reg_VAUDP_CON1, Reg_VAUDP_CON2;
   kal_uint32 Reg_AUDIO_CON3, Reg_AUDIO_CON9;
   kal_uint32 Reg_AUDIO_CON21, Reg_AUDIO_CON22, Reg_AUDIO_CON23;
   kal_uint32 Reg_VMIC_CON0;
   PRINTK_AUDDRV("AudDrv_suspend \n");

   // if modem side use Audio HW, don't turn off audio AFE.
   if( (SPH_Ctrl_State.bSpeechFlag==true) || 
       (SPH_Ctrl_State.bVT==true) || 
       (SPH_Ctrl_State.bRecordFlag==true) ||
       (SPH_Ctrl_State.bBgsFlag==true) ||
       (SPH_Ctrl_State.bTtyFlag==true)
     )
   {
      printk("AudDrv_suspend bBgsFlag:%d,bRecordFlag:%d,bSpeechFlag:%d,bTtyFlag:%d,bVT:%d,bAudio:%d \n",
      SPH_Ctrl_State.bBgsFlag,
      SPH_Ctrl_State.bRecordFlag,
      SPH_Ctrl_State.bSpeechFlag,
      SPH_Ctrl_State.bTtyFlag,
      SPH_Ctrl_State.bVT,
      SPH_Ctrl_State.bAudioPlay);
      return 0;
   }
   
   if(IsSuspen == false)
   {
      // when suspend , if is not in speech mode , close amp.
      if(AMP_Flag ==true && (SPH_Ctrl_State.bSpeechFlag== false) && (SPH_Ctrl_State.bVT== false) )
      {
         Sound_Speaker_Turnoff(Channel_Stereo);//turn off speaker
         mdelay(1);
      }
      AudDrv_Clk_On();
      // Analog register setting
      Ana_Set_Reg(VAUDN_CON0,0x0000,0x000F);
      Ana_Set_Reg(VAUDP_CON0,0x0000,0x000F);
      Ana_Set_Reg(VAUDP_CON2,0x0000,0x0F00);
      Ana_Set_Reg(AUDIO_CON9,0x0000,0x0007);
      // Analog register reading
      Reg_VAUDN_CON0 = Ana_Get_Reg(VAUDN_CON0);
      Reg_VAUDP_CON0 = Ana_Get_Reg(VAUDP_CON0);
      Reg_VAUDP_CON1 = Ana_Get_Reg(VAUDP_CON1);
      Reg_VAUDP_CON2 = Ana_Get_Reg(VAUDP_CON2);
      Reg_AUDIO_CON3 = Ana_Get_Reg(AUDIO_CON3);
      Reg_AUDIO_CON9 = Ana_Get_Reg(AUDIO_CON9);
      Reg_AUDIO_CON21 = Ana_Get_Reg(AUDIO_CON21);
      Reg_AUDIO_CON22 = Ana_Get_Reg(AUDIO_CON22);
      Reg_AUDIO_CON23 = Ana_Get_Reg(AUDIO_CON23);
      Reg_VMIC_CON0 = Ana_Get_Reg(VMIC_CON0);

      AudDrv_Clk_Off();

      spin_lock_bh(&auddrv_lock);
      AudDrv_Reset_DL1_Stream_Buf();
      AudDrv_Store_reg();
      tasklet_disable(&magic_tasklet_handle);  // disable tasklet
      AudDrv_Suspend_Clk_Off();  // turn off asm afe clock
      IsSuspen = true;// set suspend mode to true
      spin_unlock_bh(&auddrv_lock);
   }
   return 0;
}

static int AudDrv_resume(struct platform_device *dev) // wake up
{
   PRINTK_AUDDRV("AudDrv_resume \n");
   if(IsSuspen == true)
   {
      printk("AudDrv_resume AFE(%d),AFEp(%d),I2S(%d),I2Sp(%d)\n",Aud_AFE_Clk_cntr,Afe_Mem_Pwr_on,Aud_I2S_Clk_cntr,I2S_Pwr_on);

      spin_lock_bh(&auddrv_lock);
      AudDrv_Reset_DL1_Stream_Buf();

      AudDrv_Suspend_Clk_On();

      tasklet_enable(&magic_tasklet_handle); // enable tasklet
      AudDrv_Recover_reg();
      IsSuspen = false;  // set suspend mode to false
      spin_unlock_bh(&auddrv_lock);
      AudDrv_Clk_On();

      // Analog register setting
      Ana_Set_Reg(VAUDN_CON0,0x0001,0x000f);
      Ana_Set_Reg(VAUDP_CON0,0x0001,0x000f);
      Ana_Set_Reg(VAUDP_CON2,0x0100,0x0f00);
      Ana_Set_Reg(AUDIO_CON9,0x0003,0x0007);
      AudDrv_Clk_Off();

      // when resume , if amp is closed , reopen it.
      if(AMP_Flag ==true && (SPH_Ctrl_State.bSpeechFlag== false) && (SPH_Ctrl_State.bVT== false))
      {
         printk("AudDrv_resume, Sound_Speaker_Turnon \n");
         Sound_Speaker_Turnon(Channel_Stereo);
      }
   }
   return 0;
}

static void AudDrv_shutdown(struct platform_device *dev)
{
	PRINTK_AUDDRV("+AudDrv_shutdown \n");

	spin_lock_bh(&auddrv_lock);
	AudDrv_Clk_Off();  // disable afe clock
	spin_unlock_bh(&auddrv_lock);

	printk("-AudDrv_shutdown \n");
}


static int AudDrv_remove(struct platform_device *dev)
{
	PRINTK_AUDDRV("+AudDrv_remove \n");
#if (!defined (I2SIN_USE_FLEXL2))
    ////////////
    // Use DMA free coherent
          dma_free_coherent(0, 9216, I2SInVirtBufAddr, I2SInPhyBufAddr);
#endif

	tasklet_kill(&magic_tasklet_handle);
	spin_lock_bh(&auddrv_lock);
	AudDrv_Clk_Off();  // disable afe clock
	spin_unlock_bh(&auddrv_lock);

	printk("-AudDrv_remove \n");
	return 0;
}


static int AudDrv_Read_Procmem(char *buf,char **start, off_t offset, int count , int *eof, void *data)
{
    int len = 0;
    // AudioSys Register Setting
    printk("+AudDrv_Read_Procmem\n");
    len += sprintf(buf+ len, "AUDIO_TOP_CON0  = 0x%x\n",Afe_Get_Reg(AUDIO_TOP_CON0));
    len += sprintf(buf+ len, "AFE_DAC_CON0    = 0x%x\n",Afe_Get_Reg(AFE_DAC_CON0));
    len += sprintf(buf+ len, "AFE_DAC_CON1    = 0x%x\n",Afe_Get_Reg(AFE_DAC_CON1));
    len += sprintf(buf+ len, "AFE_I2S_IN_CON  = 0x%x\n",Afe_Get_Reg(AFE_I2S_IN_CON));
    len += sprintf(buf+ len, "AFE_FOC_CON     = 0x%x\n",Afe_Get_Reg(AFE_FOC_CON));
    len += sprintf(buf+ len, "AFE_DAIBT_CON   = 0x%x\n",Afe_Get_Reg(AFE_DAIBT_CON));
    len += sprintf(buf+ len, "AFE_CONN0       = 0x%x\n",Afe_Get_Reg(AFE_CONN0));
    len += sprintf(buf+ len, "AFE_CONN1       = 0x%x\n",Afe_Get_Reg(AFE_CONN1));
    len += sprintf(buf+ len, "AFE_CONN2       = 0x%x\n",Afe_Get_Reg(AFE_CONN2));
    len += sprintf(buf+ len, "AFE_CONN3       = 0x%x\n",Afe_Get_Reg(AFE_CONN3));
    len += sprintf(buf+ len, "AFE_CONN4       = 0x%x\n",Afe_Get_Reg(AFE_CONN4));
    len += sprintf(buf+ len, "AFE_I2S_OUT_CON = 0x%x\n",Afe_Get_Reg(AFE_I2S_OUT_CON));
    len += sprintf(buf+ len, "AFE_DL1_BASE    = 0x%x\n",Afe_Get_Reg(AFE_DL1_BASE));
    len += sprintf(buf+ len, "AFE_DL1_CUR     = 0x%x\n",Afe_Get_Reg(AFE_DL1_CUR));
    len += sprintf(buf+ len, "AFE_DL1_END     = 0x%x\n",Afe_Get_Reg(AFE_DL1_END));
    len += sprintf(buf+ len, "AFE_DL2_BASE    = 0x%x\n",Afe_Get_Reg(AFE_DL2_BASE));
    len += sprintf(buf+ len, "AFE_DL2_CUR     = 0x%x\n",Afe_Get_Reg(AFE_DL2_CUR));
    len += sprintf(buf+ len, "AFE_DL2_END     = 0x%x\n",Afe_Get_Reg(AFE_DL2_END));
    len += sprintf(buf+ len, "AFE_I2S_BASE    = 0x%x\n",Afe_Get_Reg(AFE_I2S_BASE));
    len += sprintf(buf+ len, "AFE_I2S_CUR     = 0x%x\n",Afe_Get_Reg(AFE_I2S_CUR));
    len += sprintf(buf+ len, "AFE_I2S_END     = 0x%x\n",Afe_Get_Reg(AFE_I2S_END));
    len += sprintf(buf+ len, "AFE_AWB_BASE    = 0x%x\n",Afe_Get_Reg(AFE_AWB_BASE));
    len += sprintf(buf+ len, "AFE_AWB_CUR     = 0x%x\n",Afe_Get_Reg(AFE_AWB_CUR));
    len += sprintf(buf+ len, "AFE_AWB_END     = 0x%x\n",Afe_Get_Reg(AFE_AWB_END));

    len += sprintf(buf+ len, "AFE_VUL_CUR     = 0x%x\n",Afe_Get_Reg(AFE_VUL_CUR));
    len += sprintf(buf+ len, "AFE_DAI_CUR     = 0x%x\n",Afe_Get_Reg(AFE_DAI_CUR));

    len += sprintf(buf+ len, "AFE_IRQ_CON     = 0x%x\n",Afe_Get_Reg(AFE_IRQ_CON));
    len += sprintf(buf+ len, "AFE_IR_STATUS   = 0x%x\n",Afe_Get_Reg(AFE_IR_STATUS));
    len += sprintf(buf+ len, "AFE_IR_CLR      = 0x%x\n",Afe_Get_Reg(AFE_IR_CLR));
    len += sprintf(buf+ len, "AFE_IRQ_CNT1    = 0x%x\n",Afe_Get_Reg(AFE_IRQ_CNT1));
    len += sprintf(buf+ len, "AFE_IRQ_CNT2    = 0x%x\n",Afe_Get_Reg(AFE_IRQ_CNT2));
    len += sprintf(buf+ len, "AFE_IRQ_MON     = 0x%x\n",Afe_Get_Reg(AFE_IRQ_MON));

    len += sprintf(buf+ len, "AFE_MODEM_IRQ_CON   = 0x%x\n",Afe_Get_Reg(AFE_MODEM_IRQ_CON));
    len += sprintf(buf+ len, "AFE_MODEM_IR_STATUS = 0x%x\n",Afe_Get_Reg(AFE_MODEM_IR_STATUS));
    len += sprintf(buf+ len, "AFE_MODEM_IR_CLR    = 0x%x\n",Afe_Get_Reg(AFE_MODEM_IR_CLR));
    len += sprintf(buf+ len, "AFE_MODEM_IRQ_CNT1  = 0x%x\n",Afe_Get_Reg(AFE_MODEM_IRQ_CNT1));
    len += sprintf(buf+ len, "AFE_MODEM_IRQ_CNT2  = 0x%x\n",Afe_Get_Reg(AFE_MODEM_IRQ_CNT2));

    len += sprintf(buf+ len, "AFE_DL_SDM_CON0 = 0x%x\n",Afe_Get_Reg(AFE_DL_SDM_CON0));
    len += sprintf(buf+ len, "AFE_DL_SRC2_1   = 0x%x\n",Afe_Get_Reg(AFE_DL_SRC2_1));
    len += sprintf(buf+ len, "AFE_DL_SRC2_2   = 0x%x\n",Afe_Get_Reg(AFE_DL_SRC2_2));
    len += sprintf(buf+ len, "AFE_DL_SRC1_1   = 0x%x\n",Afe_Get_Reg(AFE_DL_SRC1_1));

// for Audio HQA
    len += sprintf(buf+ len, "AFE_UL_AGC5     = 0x%x\n",Afe_Get_Reg(AFE_UL_AGC5));
    len += sprintf(buf+ len, "AFE_UL_AGC13    = 0x%x\n",Afe_Get_Reg(AFE_UL_AGC13));

    len += sprintf(buf+ len, "AFE_SIDETONE_CON0 = 0x%x\n",Afe_Get_Reg(AFE_SIDETONE_CON0));
    len += sprintf(buf+ len, "AFE_SIDETONE_CON1 = 0x%x\n",Afe_Get_Reg(AFE_SIDETONE_CON1));

    len += sprintf(buf+ len, "AFE_UL_SRC_0    = 0x%x\n",Afe_Get_Reg(AFE_UL_SRC_0));
    len += sprintf(buf+ len, "AFE_UL_SRC_1    = 0x%x\n",Afe_Get_Reg(AFE_UL_SRC_1));
    len += sprintf(buf+ len, "AFE_SDM_GAIN_STAGE  = 0x%x\n",Afe_Get_Reg(AFE_SDM_GAIN_STAGE));

    // Audio Variable Setting
    len += sprintf(buf+ len, "AMP_Flag         = %d\n",AMP_Flag);
    len += sprintf(buf+ len, "Afe_Mem_Pwr_on   = %d\n",Afe_Mem_Pwr_on);
    len += sprintf(buf+ len, "I2S_Pwr_on       = %d\n",I2S_Pwr_on);
    len += sprintf(buf+ len, "Aud_AFE_Clk_cntr = %d\n",Aud_AFE_Clk_cntr);
    len += sprintf(buf+ len, "Aud_ADC_Clk_cntr = %d\n",Aud_ADC_Clk_cntr);
    len += sprintf(buf+ len, "Aud_I2S_Clk_cntr = %d\n",Aud_I2S_Clk_cntr);
    // Analog ABB Register Setting
    len += sprintf(buf+ len, "PLL_CON2   = 0x%x\n",Ana_Get_Reg(PLL_CON2));
    len += sprintf(buf+ len, "AUDIO_CON0 = 0x%x\n",Ana_Get_Reg(AUDIO_CON0));
    len += sprintf(buf+ len, "AUDIO_CON1 = 0x%x\n",Ana_Get_Reg(AUDIO_CON1));
    len += sprintf(buf+ len, "AUDIO_CON2 = 0x%x\n",Ana_Get_Reg(AUDIO_CON2));
    len += sprintf(buf+ len, "AUDIO_CON3 = 0x%x\n",Ana_Get_Reg(AUDIO_CON3));
    len += sprintf(buf+ len, "AUDIO_CON4 = 0x%x\n",Ana_Get_Reg(AUDIO_CON4));
    len += sprintf(buf+ len, "AUDIO_CON5 = 0x%x\n",Ana_Get_Reg(AUDIO_CON5));
    len += sprintf(buf+ len, "AUDIO_CON6 = 0x%x\n",Ana_Get_Reg(AUDIO_CON6));
    len += sprintf(buf+ len, "AUDIO_CON7 = 0x%x\n",Ana_Get_Reg(AUDIO_CON7));
    len += sprintf(buf+ len, "AUDIO_CON8 = 0x%x\n",Ana_Get_Reg(AUDIO_CON8));
    len += sprintf(buf+ len, "AUDIO_CON9 = 0x%x\n",Ana_Get_Reg(AUDIO_CON9));
    len += sprintf(buf+ len, "AUDIO_CON10 = 0x%x\n",Ana_Get_Reg(AUDIO_CON10));

    len += sprintf(buf+ len, "AUDIO_CON20 = 0x%x\n",Ana_Get_Reg(AUDIO_CON20));
    len += sprintf(buf+ len, "AUDIO_CON21 = 0x%x\n",Ana_Get_Reg(AUDIO_CON21));
    len += sprintf(buf+ len, "AUDIO_CON22 = 0x%x\n",Ana_Get_Reg(AUDIO_CON22));
    len += sprintf(buf+ len, "AUDIO_CON23 = 0x%x\n",Ana_Get_Reg(AUDIO_CON23));
    len += sprintf(buf+ len, "AUDIO_CON24 = 0x%x\n",Ana_Get_Reg(AUDIO_CON24));
    len += sprintf(buf+ len, "AUDIO_CON25 = 0x%x\n",Ana_Get_Reg(AUDIO_CON25));
    len += sprintf(buf+ len, "AUDIO_CON26 = 0x%x\n",Ana_Get_Reg(AUDIO_CON26));
    len += sprintf(buf+ len, "AUDIO_CON27 = 0x%x\n",Ana_Get_Reg(AUDIO_CON27));
    len += sprintf(buf+ len, "AUDIO_CON28 = 0x%x\n",Ana_Get_Reg(AUDIO_CON28));
    len += sprintf(buf+ len, "AUDIO_CON29 = 0x%x\n",Ana_Get_Reg(AUDIO_CON29));
    len += sprintf(buf+ len, "AUDIO_CON30 = 0x%x\n",Ana_Get_Reg(AUDIO_CON30));
    len += sprintf(buf+ len, "AUDIO_CON31 = 0x%x\n",Ana_Get_Reg(AUDIO_CON31));
    len += sprintf(buf+ len, "AUDIO_CON32 = 0x%x\n",Ana_Get_Reg(AUDIO_CON32));

    // Analog PMU Register Setting
    len += sprintf(buf+ len, "VAUDN_CON0 = 0x%x\n",Ana_Get_Reg(VAUDN_CON0));
    len += sprintf(buf+ len, "VAUDP_CON0 = 0x%x\n",Ana_Get_Reg(VAUDP_CON0));
    len += sprintf(buf+ len, "VAUDP_CON1 = 0x%x\n",Ana_Get_Reg(VAUDP_CON1));
    len += sprintf(buf+ len, "VAUDP_CON2 = 0x%x\n",Ana_Get_Reg(VAUDP_CON2));
    len += sprintf(buf+ len, "VA12_CON0  = 0x%x\n",Ana_Get_Reg(VA12_CON0));
    len += sprintf(buf+ len, "VMIC_CON0  = 0x%x\n",Ana_Get_Reg(VMIC_CON0));
    len += sprintf(buf+ len, "VA25_CON0  = 0x%x\n",Ana_Get_Reg(VA25_CON0));
    len += sprintf(buf+ len, "ACIF_WR_PATH = 0x%x\n",Ana_Get_Reg(ACIF_WR_PATH));

    // Speech/Record/BGS/TTY/VT Setting
    len += sprintf(buf+ len, "Speech_on  = %d\n",SPH_Ctrl_State.bSpeechFlag);
    len += sprintf(buf+ len, "BGS_on     = %d\n",SPH_Ctrl_State.bBgsFlag);
    len += sprintf(buf+ len, "Record_on  = %d\n",SPH_Ctrl_State.bRecordFlag);
    len += sprintf(buf+ len, "TTY_on     = %d\n",SPH_Ctrl_State.bTtyFlag);
    len += sprintf(buf+ len, "VT_on      = %d\n",SPH_Ctrl_State.bVT);
    len += sprintf(buf+ len, "Audio_on   = %d\n",SPH_Ctrl_State.bAudioPlay);
    return len;
}




static struct platform_driver AudDrv_driver = {
   .probe	 = AudDrv_probe,
   .remove	 = AudDrv_remove,
   .shutdown = AudDrv_shutdown,
   .suspend	 = AudDrv_suspend,
   .resume	 = AudDrv_resume,
   .driver   = {
                  .name = auddrv_name,
               },
};

struct platform_device AudDrv_device = {
	.name  = auddrv_name,
	.id    = 0,
	.dev   = {
		        .dma_mask = &AudDrv_dmamask,
		        .coherent_dma_mask =  0xffffffffUL
	         }
};

static int AudDrv_mod_init(void)
{
   int ret = 0;
	printk("+AudDrv_mod_init \n");

	// Register platform DRIVER
	ret = platform_driver_register(&AudDrv_driver);
	if(ret)
   {
		printk("AudDrv Fail:%d - Register DRIVER \n",ret);
		return ret;
	}

   // register MISC device
   if((ret = misc_register(&AudDrv_audio_device)))
   {
   	printk("AudDrv_probe misc_register Fail:%d \n", ret);
   	return ret;
   }

   // cat /proc/audio
	create_proc_read_entry("audio",
	                       	     0,
                        	  NULL,
         	  AudDrv_Read_Procmem,
  		                       NULL);

   wake_lock_init(&Audio_wake_lock,WAKE_LOCK_SUSPEND,"Audio_WakeLock");
   wake_lock_init(&Audio_record_wake_lock,WAKE_LOCK_SUSPEND,"Audio_Record_WakeLock");
   printk("AudDrv_mod_init: Init Audio Wake Lock\n");

   memset(&AFE_dl_Control_context,0,sizeof(AFE_DL_CONTROL_T));
   memset(&I2S_input_Control_context,0,sizeof(I2S_INPUT_CONTROL_T));
   memset(&DAI_output_Control_context,0,sizeof(DAI_OUTPUT_CONTROL_T));
   printk("-AudDrv_mod_init return \n");

	return 0;
}

static void  AudDrv_mod_exit(void)
{
   printk("+AudDrv_mod_exit \n");

   remove_proc_entry("Audio", NULL);
   platform_driver_unregister(&AudDrv_driver);

   printk("-AudDrv_mod_exit \n");
}



MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(AUDDRV_NAME);
MODULE_AUTHOR(AUDDRV_AUTHOR);

module_init(AudDrv_mod_init);
module_exit(AudDrv_mod_exit);


