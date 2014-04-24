

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <asm/outercache.h>
#include <asm/page.h>

#define TEST_BUF_LEN 1024

unsigned int test_buf[TEST_BUF_LEN];
unsigned int power_test_addr[] = {0xFFFFFFFF, 0xEEEEEEEE, 0xDDDDDDDD, 0xCCCCCCCC, 
                                  0xBBBBBBBB, 0xAAAAAAAA, 0x99999999, 0x88888888,
                                  0x77777777, 0x66666666, 0x55555555, 0x44444444,
                                  0x33333333, 0x22222222, 0x11111111, 0x12345678, 
                                  0xFFFFFFFF, 0xEEEEEEEE, 0xDDDDDDDD, 0xCCCCCCCC, 
                                  0xBBBBBBBB, 0xAAAAAAAA, 0x99999999, 0x88888888,
                                  0x77777777, 0x66666666, 0x55555555, 0x44444444,
                                  0x33333333, 0x22222222, 0x11111111, 0x12345678};

float VFP_SPDATA_ADDR[] = {1234567891234567891237.0, 447753.0};
double VFP_DPDATA_ADDR[] = {1111111111111111111111.1, 2.2222222222222222222222,
                            333333333.33333333333333, 44444444444444444444.444,
                            55555555555555555.555555, 66666666666.666666666666,
                            7.7777777777777777777777, 888888888888888888.88888,
                            1234567891234567891237.1, 9876543210987.6543210982,
                            246.81357924678135792463, 978675.64534231201908974}; 


unsigned int branch_test_addr[] = {0xe0921003, 0xe2b55000, 0xe0921004, 0xe2b55000, 0xe12fff1e};

extern void apmcu_disable_irq(void);
extern void apmcu_disable_branch_prediction(void);
extern void apmcu_disable_prefetch_halt(void);
extern void vfp_setup(unsigned int param1, unsigned int param2);
extern void apmcu_clean_dcache(void);
extern void apmcu_dsb(void);
extern void apmcu_invalidate_icache(void);
extern void apmcu_clean_invalidate_dcache(void);
extern void apmcu_invalidate_tlb(void);
extern void apmcu_enable_irq(void);
extern void apmcu_enable_branch_prediction(void);
extern void apmcu_enable_prefetch_halt(void);
extern unsigned int power_test2(unsigned int param1, unsigned int param2, unsigned int param3);
extern unsigned int power_test3(unsigned int param1, unsigned int param2, unsigned int param3);

static char *result_buf;
static volatile unsigned int gui_sync = 0;

int arm_max_pwr_test(void)
{
    volatile unsigned int j;    
    unsigned int reg_tmp;

    *((volatile unsigned int *)0xF702F940) = 0x2871;
    for( j=0; j<24000; j++) ;

    printk("\n[ARM MAX PWR Test] Start...\r\n");

    //Enable APMCU HW DCM
    //*RG_CK_ALW_ON = 0x2;
    *((volatile unsigned int *) 0xF7026124) = 0x0;
    //Dsiable DCM_EN_IRQCLR
    *((volatile unsigned int *) 0xF7026128) = 0x0;
    //*APMCU_CG_CLR1 = 0xFF;
    *((volatile unsigned int *) 0xF7026318) = 0xFF;


    //Disable Interrup
    apmcu_disable_irq();

    reg_tmp = power_test3((unsigned int)(power_test_addr), 0xF9012000, 100000);
    if(reg_tmp!=0x87abba7)
    {
        printk("[ARM MAX PWR Test] Failed at datapath test...\r\n");
        printk("[ARM MAX PWR Test] Result: %x\r\n", reg_tmp);
        return 0;
    }

    printk("\n[ARM MAX PWR Test] End...\r\n");
    return 1;
}


int arm_pwr_trans_test(void)
{
    unsigned int i;
    unsigned int reg_tmp;


    printk("\n[ARM MAX PWR TRANS Test] Start...\r\n");

    //Enable APMCU HW DCM
    //*RG_CK_ALW_ON = 0x2;
    *((volatile unsigned int *) 0xF7026124) = 0x0;
    //Dsiable DCM_EN_IRQCLR
    *((volatile unsigned int *) 0xF7026128) = 0x0;
    //*APMCU_CG_CLR1 = 0xFF;
    *((volatile unsigned int *) 0xF7026318) = 0xFF;
    //Unmask CIRQ_MASK
    *((volatile unsigned int *) 0xF7017040) = 0x10;


    //Set branch instruction in FlexL2/MMSYSRAM
    *((volatile unsigned int *) 0xF9018000) = 0xe12fff1e;  //BX lr
    *((volatile unsigned int *) 0xF4000000) = 0xe12fff1e;  //BX lr

    //Initialize test code for branch test
    //FlexL2
    //*((volatile unsigned int *) 0x90018100) = 0xe0921003;  //ADDS r1, r2, r3
    //*((volatile unsigned int *) 0x90018104) = 0xe2b55000;  //ADCS r5, r5, #0
    //*((volatile unsigned int *) 0x90018108) = 0xe0921004;  //ADDS r1, r2, r4
    //*((volatile unsigned int *) 0x9001810C) = 0xe2b55000;  //ADCS r5, r5, #0
    //*((volatile unsigned int *) 0x90018110) = 0xe12fff1e;  //BX lr
    //*((volatile unsigned int *) 0x40000100) = 0xe0921003;  //ADDS r1, r2, r3
    //*((volatile unsigned int *) 0x40000104) = 0xe2b55000;  //ADCS r5, r5, #0
    //*((volatile unsigned int *) 0x40000108) = 0xe0921004;  //ADDS r1, r2, r4
    //*((volatile unsigned int *) 0x4000010C) = 0xe2b55000;  //ADCS r5, r5, #0
    //*((volatile unsigned int *) 0x40000110) = 0xe12fff1e;  //BX lr

    //Mask all CIRQs except XGPT3
    //*((volatile unsigned int *) 0xF7017020) = 0xffffffef; 
    //*((volatile unsigned int *) 0xF7017024) = 0xffffffff; 
    //*((volatile unsigned int *) 0xF7017028) = 0xffffffff; 
    //*((volatile unsigned int *) 0xF701702C) = 0xffffffff; 
    //*((volatile unsigned int *) 0xF701703C) = 0xffffffff; 
    //*((volatile unsigned int *) 0xF7017040) = 0x10;


    //Disable Interrup
    apmcu_disable_irq();

    //Disalbe branch_prediction / prefetch_halting for speed test
    apmcu_disable_branch_prediction();
    apmcu_disable_prefetch_halt();
    //apmcu_tlb_lockdown();

    vfp_setup((unsigned int)(VFP_SPDATA_ADDR), (unsigned int)(VFP_DPDATA_ADDR));

    for(i=0; i<400; i++)
    {
      apmcu_clean_dcache();
      apmcu_dsb();
      
      //*((volatile unsigned int *) 0xF70407FC) |= 0xFFFF;
      //while (0 != *((volatile unsigned int *) 0xF70407FC));       
      //*((volatile unsigned int *) 0xF7040730) = 0x1;
      //while (0 != *((volatile unsigned int *) 0xF7040730));       
      outer_flush_all(); 
      apmcu_invalidate_icache();
      apmcu_clean_invalidate_dcache();
      apmcu_dsb();
      apmcu_invalidate_tlb();

      //Set APXGPT Channel Compare Value
      //*APXGPT3_COMPARE = 0x2;
      *((volatile unsigned int *) 0xF702C03C) = 0x20;
      //Set APXGPT Interrupt Enable
      //*APXGPT_IRQEN = 0x4;
      *((volatile unsigned int *) 0xF702C000) = *((volatile unsigned int *) 0xF702C000) | 0x4;
      //Clear Timer
      //*APXGPT3_CON = 0x2;
      *((volatile unsigned int *) 0xF702C030) = 0x2;
      //Enable Timer
      //*APXGPT3_CON = 0x1;
      *((volatile unsigned int *) 0xF702C030) = 0x1;

      //WFI
      //asm("MOV %0, 0x0"
      //    "MCR p15,0,%0,c7,c0,4"
      //  : "+r" (temp)
      //  :
      //  : "cc");
      __asm__ __volatile__("mcr p15,0,%0,c7,c0,4"::"r" (0));


      /***** Speed Test *****/
      reg_tmp = power_test2((unsigned int)(power_test_addr), 0xF9012000, 100);
      //if(reg_tmp!=0xe4916c99)
      if(reg_tmp!=0x4fd454fa)
      {
          printk("[ARM MAX PWR TRANS Test] Failed at datapath test...\r\n");
          printk("[ARM MAX PWR TRANS Test] Result: %x\r\n", reg_tmp);
          return 0;
      }
      
      vfp_setup((unsigned int)(VFP_SPDATA_ADDR), (unsigned int)(VFP_DPDATA_ADDR));

      //ACK XGPT interrupt
      //*XGPT_IRQACK = 0x4;
      *((volatile unsigned int *) 0xF702C008) = 0x4;

      //for(k=0; k<24000; k++);
    }

    apmcu_enable_irq();
    apmcu_enable_branch_prediction();
    apmcu_enable_prefetch_halt();
    printk("[ARM MAX PWR TRANS Test] Passed...\r\n");
    return 1;
}

#define AMPLL_CON0_REG *(volatile unsigned int *)0xF702E160
#define AMPLL_CON1_REG *(volatile unsigned int *)0xF702E164
#define PLL_CON5_REG *(volatile unsigned int *)0xF702E114

int arm_fsel_test(int sel)
{    
     unsigned int j;
    //PLL Setting Start    
     AMPLL_CON1_REG = AMPLL_CON1_REG | 0x0100;
     PLL_CON5_REG  = PLL_CON5_REG & 0x0FFF;  // m/d/e/c (switch to 26M)
     AMPLL_CON0_REG = AMPLL_CON0_REG & 0x00FE;  // Disable AMPLL
     AMPLL_CON0_REG = AMPLL_CON0_REG | 0x004C; // DPLL setting: [13:8] FSEL, [6:4] just set 3'b100, [3:2] = 2'b11 means 6.9us lock time

     switch(sel)
     {
       case 0: 
         printk("=> Set APMCU Frequency = 520MHz  \r\n");
         break;
       case 1: 
         printk("=> Set APMCU Frequency = 533MHz  \r\n");
         break;
       case 2: 
         printk("=> Set APMCU Frequency = 546MHz  \r\n");
         break;
       case 3: 
         printk("=> Set APMCU Frequency = 559MHz  \r\n");
         break;
       case 4: 
         printk("=> Set APMCU Frequency = 572MHz  \r\n");
         break;
       case 5: 
         printk("=> Set APMCU Frequency = 585MHz  \r\n");
         break;
       case 6: 
         printk("=> Set APMCU Frequency = 598MHz  \r\n");
         break;
       case 7: 
         printk("=> Set APMCU Frequency = 611MHz  \r\n");
         break;
       case 8: 
         printk("=> Set APMCU Frequency = 624MHz  \r\n");
         break;
       case 9: 
         printk("=> Set APMCU Frequency = 637MHz  \r\n");
         break;
       case 10: 
         printk("=> Set APMCU Frequency = 650MHz  \r\n");
         break;
       case 11:
         printk("=> Set APMCU Frequency = 663MHz  \r\n");
         break;
       case 12:
         printk("=> Set APMCU Frequency = 676MHz  \r\n");
         break;
       case 13:
         printk("=> Set APMCU Frequency = 689MHz  \r\n");
         break;
       case 14:
         printk("=> Set APMCU Frequency = 702MHz  \r\n");
         break;
       case 15:
         printk("=> Set APMCU Frequency = 715MHz  \r\n");
         break;
       case 16:
         printk("=> Set APMCU Frequency = 728MHz  \r\n");
         break;
       case 17:
         printk("=> Set APMCU Frequency = 741MHz  \r\n");
         break;
       case 18:
         printk("=> Set APMCU Frequency = 754MHz  \r\n");
         break;
       case 19:
         printk("=> Set APMCU Frequency = 767MHz  \r\n");
         break;
       case 20:
         printk("=> Set APMCU Frequency = 780MHz  \r\n");
         break;
       case 21:
         printk("=> Set APMCU Frequency = 793MHz  \r\n");
         break;
       case 22:
         printk("=> Set APMCU Frequency = 806MHz  \r\n");
         break;
       default: break;
     }	  


     switch(sel) {
       case( 0): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x6900; break;}  // 
       case( 1): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x6A00; break;}  // 
       case( 2): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x6B00; break;}  // 
       case( 3): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x6C00; break;}  // 
       case( 4): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x6D00; break;}  // 
       case( 5): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x6E00; break;}  // 
       case( 6): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x6F00; break;}  // 
       case( 7): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7000; break;}  // 
       case( 8): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7100; break;}  // 
       case( 9): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7200; break;}  // 
       case(10): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7300; break;}  // 
       case(11): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7400; break;}  // 
       case(12): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7500; break;}  // 
       case(13): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7600; break;}  // 
       case(14): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7700; break;}  // 
       case(15): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7800; break;}  // 
       case(16): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7900; break;}  // 
       case(17): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7A00; break;}  // 
       case(18): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7B00; break;}  // 
       case(19): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7C00; break;}  // 
       case(20): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7D00; break;}  // 
       case(21): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7E00; break;}  // 
       case(22): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7F00; break;}  // 
     
       default: { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x0000; break;}  // 650MHz
     }
     
     AMPLL_CON0_REG = AMPLL_CON0_REG | 0x0001;  // Enable DPLL
     
    // wait 6.9us < 180 T(26M)
    for( j=0; j<600; j++) ;
    //while(AMPLL_CON3_REG == 0x20) ;
    
    PLL_CON5_REG = PLL_CON5_REG | 0x1000;  // m/d/e/c (switch to PLL)
    
    // **PLL Setting END**
#if 1
    // Setting clock switch
      if ( arm_pwr_trans_test() != 1)
      {
          printk("\r\n ARM Power Trans Test Pattern Failed!! \r\n");
          //failed_freq = AMPLL_CON0_REG & 0xFF00;
          return 0;
      }
      else
      {
          return 1;
      }
#endif      
}

int arm11_slt_main(void)
{  
    int i4Ret = 0;
    int i, j;

    printk("[SLT ARM11] Test Start\r\n");
    //Set VAPROC to 1.275V
    AMPLL_CON1_REG = AMPLL_CON1_REG | 0x0100;
    PLL_CON5_REG  = PLL_CON5_REG & 0x0FFF;  // m/d/e/c (switch to 26M)
    AMPLL_CON0_REG = AMPLL_CON0_REG & 0x00FE;  // Disable AMPLL
    AMPLL_CON0_REG = AMPLL_CON0_REG | 0x004C; // DPLL setting: [13:8] FSEL, [6:4] just set 3'b100, [3:2] = 2'b11 means 6.9us lock time
    AMPLL_CON0_REG = AMPLL_CON0_REG | 0x6900;  
    AMPLL_CON0_REG = AMPLL_CON0_REG | 0x0001;  // Enable DPLL
    *((volatile unsigned int *)0xF702F940) = 0x2821;
    for( j=0; j<24000; j++) ;
    PLL_CON5_REG = PLL_CON5_REG | 0x1000;  // m/d/e/c (switch to PLL)

    for (i=8; i<23; i++ )  // 520MHz ~ 806MHz
    {
      int pmu_vaproc;
      //Read PMU settings (Current sense ratio)
      pmu_vaproc = *((volatile unsigned int *)0xF702F940);
      printk("=> VAPROC = %x  \r\n", pmu_vaproc);

      //Speed Test
      if(arm_fsel_test(i))
        printk("*************************** ARM Speed Grading Test PASS!! ******************************\r\n");
      else
      {
        printk("*************************** ARM Speed Grading Test FAILED!! ****************************\r\n");
        //while(1);
      }
    }

    return i4Ret;
}

////////////////////////////////////////////////////////////////////////////////////////////
/*Clone the testing code for GUI*/
////////////////////////////////////////////////////////////////////////////////////////////

int arm_fsel_test_gui(int sel)
{
    int j;
    char *ptr = result_buf;
    
    //PLL Setting Start    
     AMPLL_CON1_REG = AMPLL_CON1_REG | 0x0100;
     PLL_CON5_REG  = PLL_CON5_REG & 0x0FFF;  // m/d/e/c (switch to 26M)
     AMPLL_CON0_REG = AMPLL_CON0_REG & 0x00FE;  // Disable AMPLL
     AMPLL_CON0_REG = AMPLL_CON0_REG | 0x004C; // DPLL setting: [13:8] FSEL, [6:4] just set 3'b100, [3:2] = 2'b11 means 6.9us lock time

     switch(sel)
     {
       case 0: 
         ptr += sprintf(ptr, "=> Set APMCU Frequency = 520MHz  \r\n");
         break;
       case 1: 
         ptr += sprintf(ptr, "=> Set APMCU Frequency = 533MHz  \r\n");
         break;
       case 2: 
         ptr += sprintf(ptr, "=> Set APMCU Frequency = 546MHz  \r\n");
         break;
       case 3: 
         ptr += sprintf(ptr, "=> Set APMCU Frequency = 559MHz  \r\n");
         break;
       case 4: 
         ptr += sprintf(ptr, "=> Set APMCU Frequency = 572MHz  \r\n");
         break;
       case 5: 
         ptr += sprintf(ptr, "=> Set APMCU Frequency = 585MHz  \r\n");
         break;
       case 6: 
         ptr += sprintf(ptr, "=> Set APMCU Frequency = 598MHz  \r\n");
         break;
       case 7: 
         ptr += sprintf(ptr, "=> Set APMCU Frequency = 611MHz  \r\n");
         break;
       case 8: 
         ptr += sprintf(ptr, "=> Set APMCU Frequency = 624MHz  \r\n");
         break;
       case 9: 
         ptr += sprintf(ptr, "=> Set APMCU Frequency = 637MHz  \r\n");
         break;
       case 10: 
         ptr += sprintf(ptr, "=> Set APMCU Frequency = 650MHz  \r\n");
         break;
       case 11:
         ptr += sprintf(ptr, "=> Set APMCU Frequency = 663MHz  \r\n");
         break;
       case 12:
         ptr += sprintf(ptr, "=> Set APMCU Frequency = 676MHz  \r\n");
         break;
       case 13:
         ptr += sprintf(ptr, "=> Set APMCU Frequency = 689MHz  \r\n");
         break;
       case 14:
         ptr += sprintf(ptr, "=> Set APMCU Frequency = 702MHz  \r\n");
         break;
       case 15:
         ptr += sprintf(ptr, "=> Set APMCU Frequency = 715MHz  \r\n");
         break;
       case 16:
         ptr += sprintf(ptr, "=> Set APMCU Frequency = 728MHz  \r\n");
         break;
       case 17:
         ptr += sprintf(ptr, "=> Set APMCU Frequency = 741MHz  \r\n");
         break;
       case 18:
         ptr += sprintf(ptr, "=> Set APMCU Frequency = 754MHz  \r\n");
         break;
       case 19:
         ptr += sprintf(ptr, "=> Set APMCU Frequency = 767MHz  \r\n");
         break;
       case 20:
         ptr += sprintf(ptr, "=> Set APMCU Frequency = 780MHz  \r\n");
         break;
       case 21:
         ptr += sprintf(ptr, "=> Set APMCU Frequency = 793MHz  \r\n");
         break;
       case 22:
         ptr += sprintf(ptr, "=> Set APMCU Frequency = 806MHz  \r\n");
         break;
       default: break;
     }	  


     switch(sel) {
       case( 0): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x6900; break;}  // 
       case( 1): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x6A00; break;}  // 
       case( 2): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x6B00; break;}  // 
       case( 3): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x6C00; break;}  // 
       case( 4): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x6D00; break;}  // 
       case( 5): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x6E00; break;}  // 
       case( 6): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x6F00; break;}  // 
       case( 7): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7000; break;}  // 
       case( 8): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7100; break;}  // 
       case( 9): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7200; break;}  // 
       case(10): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7300; break;}  // 
       case(11): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7400; break;}  // 
       case(12): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7500; break;}  // 
       case(13): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7600; break;}  // 
       case(14): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7700; break;}  // 
       case(15): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7800; break;}  // 
       case(16): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7900; break;}  // 
       case(17): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7A00; break;}  // 
       case(18): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7B00; break;}  // 
       case(19): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7C00; break;}  // 
       case(20): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7D00; break;}  // 
       case(21): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7E00; break;}  // 
       case(22): { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x7F00; break;}  // 
     
       default: { AMPLL_CON0_REG = AMPLL_CON0_REG | 0x0000; break;}  // 650MHz
     }
     
     AMPLL_CON0_REG = AMPLL_CON0_REG | 0x0001;  // Enable DPLL
     
    // wait 6.9us < 180 T(26M)
    for( j=0; j<600; j++) ;
    //while(AMPLL_CON3_REG == 0x20) ;
    
    PLL_CON5_REG = PLL_CON5_REG | 0x1000;  // m/d/e/c (switch to PLL)
    
    // **PLL Setting END**

    // Setting clock switch
      if ( arm_pwr_trans_test() != 1)
      {
          ptr += sprintf(ptr, "ARM Power Trans Test Pattern Failed!! \r\n");
          //failed_freq = AMPLL_CON0_REG & 0xFF00;
          return 0;
      }
      else
      {
          ptr += sprintf(ptr, "ARM Power Trans Test Pattern PASS \r\n");
          return 1;
      }
}


int arm11_slt_main_gui(int sel)
{  
    int i4Ret = 0;
    int j;
    unsigned int restore;
    int pmu_vaproc;

    restore = *((volatile unsigned int *)0xF702F940);

    printk("[SLT ARM11] Test Start\r\n");
    //Set VAPROC to 1.275V
    AMPLL_CON1_REG = AMPLL_CON1_REG | 0x0100;
    PLL_CON5_REG  = PLL_CON5_REG & 0x0FFF;  // m/d/e/c (switch to 26M)
    AMPLL_CON0_REG = AMPLL_CON0_REG & 0x00FE;  // Disable AMPLL
    AMPLL_CON0_REG = AMPLL_CON0_REG | 0x004C; // DPLL setting: [13:8] FSEL, [6:4] just set 3'b100, [3:2] = 2'b11 means 6.9us lock time
    AMPLL_CON0_REG = AMPLL_CON0_REG | 0x6900;  
    AMPLL_CON0_REG = AMPLL_CON0_REG | 0x0001;  // Enable DPLL
    *((volatile unsigned int *)0xF702F940) = 0x2821;
    for( j=0; j<24000; j++) ;
    PLL_CON5_REG = PLL_CON5_REG | 0x1000;  // m/d/e/c (switch to PLL)
    
    //Read PMU settings (Current sense ratio)
    pmu_vaproc = *((volatile unsigned int *)0xF702F940);
    printk("=> VAPROC = %x  \r\n", pmu_vaproc);

    //Speed Test
    if(arm_fsel_test_gui(sel))
      printk("*************************** ARM Speed Grading Test PASS!! ******************************\r\n");
    else
    {
      printk("*************************** ARM Speed Grading Test FAILED!! ****************************\r\n");
      //while(1);
    }

    *((volatile unsigned int *)0xF702F940) = restore;
    for( j=0; j<24000; j++) ;


    return i4Ret;
}


static ssize_t arm_pwr_test_show(struct device_driver *driver, char *buf)
{
    return 0;
}

ssize_t arm_pwr_test_store(struct device_driver *driver, const char *buf, size_t count)
{
    arm11_slt_main();

    return count;
}

DRIVER_ATTR(arm_pwr_test, 0644, arm_pwr_test_show, arm_pwr_test_store);

static ssize_t arm_pwr_test_gui_show(struct device_driver *driver, char *buf)
{
    while(gui_sync == 0);
    
    strcpy(buf, result_buf);
    kfree(result_buf);

    gui_sync = 0;
    
    return strlen(buf);
}

ssize_t arm_pwr_test_gui_store(struct device_driver *driver, const char *buf, size_t count)
{
    int sel;
    char *p = (char *)buf;

    while(gui_sync == 1);
    
    sel = simple_strtoul(p, &p, 10);
    result_buf = kmalloc((size_t)PAGE_SIZE, GFP_KERNEL);
    
    if(sel == 23)
    {
      arm_max_pwr_test();
    }
    else
    {
      arm11_slt_main_gui(sel);
    }

    gui_sync = 1;
    
    return count;
}

DRIVER_ATTR(arm_pwr_test_gui, 0644, arm_pwr_test_gui_show, arm_pwr_test_gui_store);


static struct device_driver arm_pwr_test_drv =
{
    .name = "arm_pwr_test",
    .bus = &platform_bus_type,
    .owner = THIS_MODULE,
};

static int __init arm_pwr_test_mod_init(void)
{
    int ret;

    ret = driver_register(&arm_pwr_test_drv);
    if (ret) {
        printk(KERN_ERR"fail to register arm_pwr_test_drv\n");
        return ret;
    }

    ret = driver_create_file(&arm_pwr_test_drv, &driver_attr_arm_pwr_test);
    ret = driver_create_file(&arm_pwr_test_drv, &driver_attr_arm_pwr_test_gui);
    if (ret) {
        printk(KERN_ERR"fail to create arm_pwr_test sysfs file\n");
        return ret;
    }

    return 0;
}

static void __exit arm_pwr_test_mod_exit(void)
{
}

module_init(arm_pwr_test_mod_init);
module_exit(arm_pwr_test_mod_exit);

