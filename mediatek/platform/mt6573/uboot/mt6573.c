

#include <common.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/arch/mt65xx.h>
#include <asm/mach-types.h>
#include <asm/arch/mt65xx_typedefs.h>
#include <asm/arch/boot_mode.h>
#include <video.h>

#include <config.h>

#ifndef CONFIG_CFB_CONSOLE
	#define video_printf 	
#endif

DECLARE_GLOBAL_DATA_PTR;
/* CC: remove temporarily for MT6573 porting */
#if 0
extern void mt6573_timer_init(void);
extern void mt6573_aud_init(void);
#endif
extern int Uboot_power_saving(void);
extern void cpu_clean_env (void);

static inline void delay (unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n"
		"bne 1b":"=r" (loops):"0" (loops));
}

void system_init(void)
{
/* CC: remove temporarily for MT6573 porting */
#if 0
    mt6573_timer_init();
#endif

    /* enabling audio depop takes time,
     * so init it here rather than linux kernel
     */

    delay(2000);
/* CC: remove temporarily for MT6573 porting */
#if 0
    mt6573_aud_init();

 //   led_init();
#endif

    return;
}

int cleanup_before_linux (void)
{
	printf("system cleanup before entering linux ...\n");

	// (1) make sure the cache is off (turn off I/D-cache)
	// (2) make sure the cache is clean (flush I/D-cache)
	printf(" > clean cpu env\n");
	cpu_clean_env();

/* CC: remove temporarily for MT6573 porting */
#if 1
	// (3) deinit leds
	printf(" > deinit leds..\n");	
	leds_deinit();
#endif
	
	// (4) power off some unused LDO
	printf(" > perform power saving\n");		
	Uboot_power_saving();

	return (0);
}


int mt65xx_sw_env (void)
{
#ifdef CFG_RECOVERY_MODE
  if(g_boot_mode != META_BOOT && g_boot_mode != FACTORY_BOOT && recovery_detection() == TRUE)
  {    
      
  }
#endif   

    //**************************************
    //* CHECK BOOT MODE
    //**************************************
    #ifndef USER_BUILD
    switch(g_boot_mode)
    {        case META_BOOT :
	         video_printf(" => META MODE\n");
	         break;
	     case FACTORY_BOOT :
	         video_printf(" => FACTORY MODE\n");
	         break;
	     case RECOVERY_BOOT :
	         video_printf(" => RECOVERY MODE\n");
	         break;
	     case SW_REBOOT :
	         //video_printf(" => SW RESET\n");
	         break;
	     case NORMAL_BOOT :
	         video_printf(" => NORMAL BOOT\n");
	         break;
             case ADVMETA_BOOT:
                 video_printf(" => ADVANCED META MODE\n");
                 break;
		   case ATE_FACTORY_BOOT:
            video_printf(" => ATE FACTORY MODE\n");
		        break;     
	     default :
                 video_printf(" => UNKNOWN BOOT\n");
    }
    #endif
    
    return 0;
}


/* Transparent to DRAM customize */
static int g_nr_bank;
#include <memory_info.h> // for dram customize
int dram_init(void)
{
  
  unsigned short nand_id;
  int i, index, num_record;
  
#ifdef CFG_NAND_BOOT  
  getflashid(&nand_id);
#endif
  
  num_record = sizeof(dram_settings) /sizeof(struct DRAM_SETTINGS);

  if (num_record == 1 && dram_settings[0].NAND_ID == 0)
  {
    nand_id = 0;
  }


  for(i=0; i<num_record; i++)
  {
    if(dram_settings[i].NAND_ID == nand_id)
{
     index = i;
     break;
    }
  }
  
  g_nr_bank = dram_settings[index].BANKS_NR;


  if (g_nr_bank == 1){
    gd->bd->bi_dram[0].start = dram_settings[index].CFG_PHYS_SDRAM_0_START;
    gd->bd->bi_dram[0].size =  dram_settings[index].CFG_PHYS_SDRAM_0_SIZE;
  }
  else if (g_nr_bank == 2){
    gd->bd->bi_dram[0].start = dram_settings[index].CFG_PHYS_SDRAM_0_START;      
    gd->bd->bi_dram[0].size =  dram_settings[index].CFG_PHYS_SDRAM_0_SIZE;  
    gd->bd->bi_dram[1].start = dram_settings[index].CFG_PHYS_SDRAM_1_START;      
    gd->bd->bi_dram[1].size =  dram_settings[index].CFG_PHYS_SDRAM_1_SIZE;
  }
  else if (g_nr_bank == 3){
    gd->bd->bi_dram[0].start = dram_settings[index].CFG_PHYS_SDRAM_0_START;      
    gd->bd->bi_dram[0].size =  dram_settings[index].CFG_PHYS_SDRAM_0_SIZE;
    gd->bd->bi_dram[1].start = dram_settings[index].CFG_PHYS_SDRAM_1_START;
    gd->bd->bi_dram[1].size =  dram_settings[index].CFG_PHYS_SDRAM_1_SIZE;
    gd->bd->bi_dram[2].start = dram_settings[index].CFG_PHYS_SDRAM_2_START;      
    gd->bd->bi_dram[2].size =  dram_settings[index].CFG_PHYS_SDRAM_2_SIZE;
  }
  else if (g_nr_bank == 4){
    gd->bd->bi_dram[0].start = dram_settings[index].CFG_PHYS_SDRAM_0_START;      
    gd->bd->bi_dram[0].size =  dram_settings[index].CFG_PHYS_SDRAM_0_SIZE;
    gd->bd->bi_dram[1].start = dram_settings[index].CFG_PHYS_SDRAM_1_START;      
    gd->bd->bi_dram[1].size =  dram_settings[index].CFG_PHYS_SDRAM_1_SIZE;
    gd->bd->bi_dram[2].start = dram_settings[index].CFG_PHYS_SDRAM_2_START;      
    gd->bd->bi_dram[2].size =  dram_settings[index].CFG_PHYS_SDRAM_2_SIZE;
    gd->bd->bi_dram[3].start = dram_settings[index].CFG_PHYS_SDRAM_3_START;      
    gd->bd->bi_dram[3].size =  dram_settings[index].CFG_PHYS_SDRAM_3_SIZE;
  }
  else{
    //ERROR! DRAM bank number is not correct
  }
  
  return 0;
}

int get_bank_nr(void)
{
  return g_nr_bank;
}

u32 memory_size(void)
{
	// each bank mapping to 256 mb physical address
	int nr_bank = g_nr_bank;
    int size;
    if(nr_bank == 1)
    {
	  size = 256 * 1024 * 1024 * (nr_bank - 1) + gd->bd->bi_dram[nr_bank-1].size + RIL_SIZE;
    }
    else
    {
      size = 256 * 1024 * 1024 * (nr_bank - 1) + gd->bd->bi_dram[nr_bank-1].size;
    }
	return size;
}
