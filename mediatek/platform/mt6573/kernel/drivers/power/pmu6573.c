
#include <linux/init.h>        /* For init/exit macros */
#include <linux/module.h>      /* For MODULE_ marcros  */
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/wakelock.h>

#include <asm/uaccess.h>

#include <mach/mt6573_eint.h>
#include <mach/mt6573_gpio.h>
#include <mach/irqs.h>
#include <mach/mt6573_boot.h>
#include <mach/mt6573_pll.h>

#include "pmu6573_hw.h"
#include "pmu6573_sw.h"
#include "upmu_common_sw.h"

#include "mt6573_battery.h"
#include "mt6573_udc.h"

//#include <pmic_drv.h>

//#if (defined(PMIC_6573_REG_API))

//#define DRV_Reg(addr)               (*(volatile kal_uint16 *)(addr))
//#define DRV_Reg32(addr)               (*(volatile kal_uint32 *)(addr))
//#define DRV_WriteReg(addr,data)     ((*(volatile kal_uint16 *)(addr)) = (kal_uint16)(data))
//#define DRV_WriteReg32(addr,data)     ((*(volatile kal_uint32 *)(addr)) = (kal_uint32)(data))

#define DRV_ClearBits(addr,data)     {\
   kal_uint16 temp;\
   temp = DRV_Reg(addr);\
   temp &=~(data);\
   DRV_WriteReg(addr,temp);\
}

#define DRV_SetBits(addr,data)     {\
   kal_uint16 temp;\
   temp = DRV_Reg(addr);\
   temp |= (data);\
   DRV_WriteReg(addr,temp);\
}

#define DRV_SetData(addr, bitmask, value)     {\
   kal_uint16 temp;\
   temp = (~(bitmask)) & DRV_Reg(addr);\
   temp |= (value);\
   DRV_WriteReg(addr,temp);\
}

#define DRV_SetData32(addr, bitmask, value)     {\
   kal_uint32 temp;\
   temp = (~(bitmask)) & DRV_Reg32(addr);\
   temp |= (value);\
   DRV_WriteReg32(addr,temp);\
}

#define PMU_DRV_ClearBits16(addr, data)           DRV_ClearBits(addr,data)

#define PMU_DRV_SetBits16(addr, data)             DRV_SetBits(addr,data)

#define PMU_DRV_WriteReg16(addr, data)            DRV_WriteReg(addr, data)
#define PMU_DRV_WriteReg32(addr, data)            DRV_WriteReg32(addr, data)

#define PMU_DRV_ReadReg16(addr)                   DRV_Reg(addr)
#define PMU_DRV_ReadReg32(addr)                   DRV_Reg32(addr)

#define PMU_DRV_SetData16(addr, bitmask, value)   DRV_SetData(addr, bitmask, value)
#define PMU_DRV_SetData32(addr, bitmask, value)   DRV_SetData32(addr, bitmask, value)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Chip specific LDOs list
upmu_ldo_profile_entry upmu_ldo_profile[] =
{
/* LDO 1 */
/* VA28 */    
	{	VA28_CON0,     
		1, 
		{							UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* VA25 */    
	{	VA25_CON0,     
		1, 
		{							UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* VA12 */    
	{	VA12_CON0,     
		1, 
		{							UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* VRTC */	  
	{	VRTC_CON0,	   
		1, 
		{							UPMU_VOLT_MAX,		 UPMU_VOLT_MAX, 	  UPMU_VOLT_MAX,	   UPMU_VOLT_MAX,
									UPMU_VOLT_MAX,		 UPMU_VOLT_MAX, 	  UPMU_VOLT_MAX,	   UPMU_VOLT_MAX,
									UPMU_VOLT_MAX,		 UPMU_VOLT_MAX, 	  UPMU_VOLT_MAX,	   UPMU_VOLT_MAX,
									UPMU_VOLT_MAX,		 UPMU_VOLT_MAX, 	  UPMU_VOLT_MAX,	   UPMU_VOLT_MAX
		}
	},


/* VMIC */    
	{	VMIC_CON0,     
		1, 
		{							UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},	

/* VTV */    
	{	VTV_CON0,     
		1, 
		{							UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},		

/* VAUDN */    
	{	VAUDN_CON0,     
		1, 
		{							UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* VAUDP */    
	{	VAUDP_CON0,     
		1, 
		{							UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* PMUA */    
	{	PMUA_CON0,     
		1, 
		{							UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* LDO 2 */	
/* VRF */    
	{	VRF_CON0,     
		1, 
		{							UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* VCAMA */  
	{	VCAMA_CON0,   
		4, 
		{							UPMU_VOLT_1_5_0_0_V, UPMU_VOLT_1_8_0_0_V, UPMU_VOLT_2_5_0_0_V, UPMU_VOLT_2_8_0_0_V,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* VCAMD */ 
	{	VCAMD_CON0,   
		7, 
		{							UPMU_VOLT_1_3_0_0_V, UPMU_VOLT_1_5_0_0_V, UPMU_VOLT_1_8_0_0_V, UPMU_VOLT_2_5_0_0_V,
	                            	UPMU_VOLT_2_8_0_0_V, UPMU_VOLT_3_0_0_0_V, UPMU_VOLT_3_3_0_0_V, UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* VIO */   
	{	VIO_CON0,    
		1, 
		{							UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* VUSB */   
	{	VUSB_CON0,    
		1, 
		{							UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* VSIM */   
	{	VSIM_CON0,    
		2, 
		{							UPMU_VOLT_1_8_0_0_V, UPMU_VOLT_3_0_0_0_V, UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* VSIM2 */  
	{	VSIM2_CON0,   
		7, 
		{							UPMU_VOLT_1_3_0_0_V, UPMU_VOLT_1_5_0_0_V, UPMU_VOLT_1_8_0_0_V, UPMU_VOLT_2_5_0_0_V,
	                            	UPMU_VOLT_2_8_0_0_V, UPMU_VOLT_3_0_0_0_V, UPMU_VOLT_3_3_0_0_V, UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* VIBR */   
	{	VIBR_CON0,    
		7, 
		{							UPMU_VOLT_1_3_0_0_V, UPMU_VOLT_1_5_0_0_V, UPMU_VOLT_1_8_0_0_V, UPMU_VOLT_2_5_0_0_V,
	                            	UPMU_VOLT_2_8_0_0_V, UPMU_VOLT_3_0_0_0_V, UPMU_VOLT_3_3_0_0_V, UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* VMC */    
	{	VMC_CON0,     
		7, 
		{							UPMU_VOLT_1_3_0_0_V, UPMU_VOLT_1_5_0_0_V, UPMU_VOLT_1_8_0_0_V, UPMU_VOLT_2_5_0_0_V,
	                            	UPMU_VOLT_2_8_0_0_V, UPMU_VOLT_3_0_0_0_V, UPMU_VOLT_3_3_0_0_V, UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* VCAMA2 */    
	{	VCAMA2_CON0,     
		4, 
		{							UPMU_VOLT_1_5_0_0_V, UPMU_VOLT_1_8_0_0_V, UPMU_VOLT_2_5_0_0_V, UPMU_VOLT_2_8_0_0_V,
	                            	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},
	
/* VCAMD2 */ 
	{	VCAMD2_CON0,   
		7, 
		{							UPMU_VOLT_1_3_0_0_V, UPMU_VOLT_1_5_0_0_V, UPMU_VOLT_1_8_0_0_V, UPMU_VOLT_2_5_0_0_V,
	                            	UPMU_VOLT_2_8_0_0_V, UPMU_VOLT_3_0_0_0_V, UPMU_VOLT_3_3_0_0_V, UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* VM12 */   
	{	VM12_CON0,    
		1, 
		{							UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* VM12_INT */   
	{	VM12_INT_CON0,    
		1, 
		{							UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	}	
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Chip specific BUCKs list
upmu_buck_profile_entry upmu_buck_profile[] =
{
/* VCORE */    
	{	VCORE_CON0,  
		32, 
		{						   UPMU_VOLT_1_2_0_0_V, UPMU_VOLT_1_2_2_5_V, UPMU_VOLT_1_2_5_0_V, UPMU_VOLT_1_2_7_5_V,
                                   UPMU_VOLT_1_3_0_0_V, UPMU_VOLT_1_3_2_5_V, UPMU_VOLT_1_3_5_0_V, UPMU_VOLT_1_3_7_5_V,
                                   UPMU_VOLT_1_3_7_5_V, UPMU_VOLT_1_3_7_5_V, UPMU_VOLT_1_3_7_5_V, UPMU_VOLT_1_3_7_5_V,
                                   UPMU_VOLT_1_3_7_5_V, UPMU_VOLT_1_3_7_5_V, UPMU_VOLT_1_3_7_5_V, UPMU_VOLT_1_3_7_5_V,
                                   UPMU_VOLT_0_8_0_0_V, UPMU_VOLT_0_8_2_5_V, UPMU_VOLT_0_8_5_0_V, UPMU_VOLT_0_8_7_5_V,
                                   UPMU_VOLT_0_9_0_0_V, UPMU_VOLT_0_9_2_5_V, UPMU_VOLT_0_9_5_0_V, UPMU_VOLT_0_9_7_5_V,
                                   UPMU_VOLT_1_0_0_0_V, UPMU_VOLT_1_0_2_5_V, UPMU_VOLT_1_0_5_0_V, UPMU_VOLT_1_0_7_5_V,
                                   UPMU_VOLT_1_1_0_0_V, UPMU_VOLT_1_1_2_5_V, UPMU_VOLT_1_1_5_0_V, UPMU_VOLT_1_1_7_5_V
       	}
	},
	
/* VIO1V8 */    
	{	VIO1V8_CON0,  
		32, 
		{						   UPMU_VOLT_1_8_0_0_V, UPMU_VOLT_1_8_2_5_V, UPMU_VOLT_1_8_5_0_V, UPMU_VOLT_1_8_7_5_V,
                                   UPMU_VOLT_1_9_0_0_V, UPMU_VOLT_1_9_2_5_V, UPMU_VOLT_1_9_5_0_V, UPMU_VOLT_1_9_7_5_V,
                                   UPMU_VOLT_1_9_7_5_V, UPMU_VOLT_1_9_7_5_V, UPMU_VOLT_1_9_7_5_V, UPMU_VOLT_1_9_7_5_V,
                                   UPMU_VOLT_1_9_7_5_V, UPMU_VOLT_1_9_7_5_V, UPMU_VOLT_1_9_7_5_V, UPMU_VOLT_1_9_7_5_V,
                                   UPMU_VOLT_1_4_0_0_V, UPMU_VOLT_1_4_2_5_V, UPMU_VOLT_1_4_5_0_V, UPMU_VOLT_1_4_7_5_V,
                                   UPMU_VOLT_1_5_0_0_V, UPMU_VOLT_1_5_2_5_V, UPMU_VOLT_1_5_5_0_V, UPMU_VOLT_1_5_7_5_V,
                                   UPMU_VOLT_1_6_0_0_V, UPMU_VOLT_1_6_2_5_V, UPMU_VOLT_1_6_5_0_V, UPMU_VOLT_1_6_7_5_V,
                                   UPMU_VOLT_1_7_0_0_V, UPMU_VOLT_1_7_2_5_V, UPMU_VOLT_1_7_5_0_V, UPMU_VOLT_1_7_7_5_V
       	}
	},

/* VAPROC */    
	{	VAPROC_CON0,  
		32, 
		{						   UPMU_VOLT_1_2_0_0_V, UPMU_VOLT_1_2_2_5_V, UPMU_VOLT_1_2_5_0_V, UPMU_VOLT_1_2_7_5_V,
                                   UPMU_VOLT_1_3_0_0_V, UPMU_VOLT_1_3_2_5_V, UPMU_VOLT_1_3_5_0_V, UPMU_VOLT_1_3_7_5_V,
                                   UPMU_VOLT_1_4_0_0_V, UPMU_VOLT_1_4_2_5_V, UPMU_VOLT_1_4_5_0_V, UPMU_VOLT_1_3_7_5_V,
                                   UPMU_VOLT_1_3_7_5_V, UPMU_VOLT_1_3_7_5_V, UPMU_VOLT_1_3_7_5_V, UPMU_VOLT_1_3_7_5_V,
                                   UPMU_VOLT_0_8_0_0_V, UPMU_VOLT_0_8_2_5_V, UPMU_VOLT_0_8_5_0_V, UPMU_VOLT_0_8_7_5_V,
                                   UPMU_VOLT_0_9_0_0_V, UPMU_VOLT_0_9_2_5_V, UPMU_VOLT_0_9_5_0_V, UPMU_VOLT_0_9_7_5_V,
                                   UPMU_VOLT_1_0_0_0_V, UPMU_VOLT_1_0_2_5_V, UPMU_VOLT_1_0_5_0_V, UPMU_VOLT_1_0_7_5_V,
                                   UPMU_VOLT_1_1_0_0_V, UPMU_VOLT_1_1_2_5_V, UPMU_VOLT_1_1_5_0_V, UPMU_VOLT_1_1_7_5_V
       	}
	},

/* VRF1V8 */    
	{	VRF1V8_CON0,  
		32, 
		{						   UPMU_VOLT_1_8_0_0_V, UPMU_VOLT_1_8_2_5_V, UPMU_VOLT_1_8_5_0_V, UPMU_VOLT_1_8_7_5_V,
                                   UPMU_VOLT_1_9_0_0_V, UPMU_VOLT_1_9_2_5_V, UPMU_VOLT_1_9_5_0_V, UPMU_VOLT_1_9_7_5_V,
                                   UPMU_VOLT_1_9_7_5_V, UPMU_VOLT_1_9_7_5_V, UPMU_VOLT_1_9_7_5_V, UPMU_VOLT_1_9_7_5_V,
                                   UPMU_VOLT_1_9_7_5_V, UPMU_VOLT_1_9_7_5_V, UPMU_VOLT_1_9_7_5_V, UPMU_VOLT_1_9_7_5_V,
                                   UPMU_VOLT_1_4_0_0_V, UPMU_VOLT_1_4_2_5_V, UPMU_VOLT_1_4_5_0_V, UPMU_VOLT_1_4_7_5_V,
                                   UPMU_VOLT_1_5_0_0_V, UPMU_VOLT_1_5_2_5_V, UPMU_VOLT_1_5_5_0_V, UPMU_VOLT_1_5_7_5_V,
                                   UPMU_VOLT_1_6_0_0_V, UPMU_VOLT_1_6_2_5_V, UPMU_VOLT_1_6_5_0_V, UPMU_VOLT_1_6_7_5_V,
                                   UPMU_VOLT_1_7_0_0_V, UPMU_VOLT_1_7_2_5_V, UPMU_VOLT_1_7_5_0_V, UPMU_VOLT_1_7_7_5_V
       	}
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Chip specific BOOSTs list
upmu_buck_boost_profile_entry upmu_buck_boost_profile[] =
{
/* BUCK BOOST */ 
	{	PMIC_BB_CON0,  
		26, 
		{						   UPMU_VOLT_0_9_0_0_V, UPMU_VOLT_1_0_0_0_V, UPMU_VOLT_1_1_0_0_V, UPMU_VOLT_1_2_0_0_V,
                                   UPMU_VOLT_1_3_0_0_V, UPMU_VOLT_1_4_0_0_V, UPMU_VOLT_1_5_0_0_V, UPMU_VOLT_1_6_0_0_V,
                                   UPMU_VOLT_1_7_0_0_V, UPMU_VOLT_1_8_0_0_V, UPMU_VOLT_1_9_0_0_V, UPMU_VOLT_2_0_0_0_V,
                                   UPMU_VOLT_2_1_0_0_V, UPMU_VOLT_2_2_0_0_V, UPMU_VOLT_2_3_0_0_V, UPMU_VOLT_2_4_0_0_V,
                                   UPMU_VOLT_2_5_0_0_V, UPMU_VOLT_2_6_0_0_V, UPMU_VOLT_2_7_0_0_V, UPMU_VOLT_2_8_0_0_V,
                                   UPMU_VOLT_2_9_0_0_V, UPMU_VOLT_3_0_0_0_V, UPMU_VOLT_3_1_0_0_V, UPMU_VOLT_3_2_0_0_V,
                                   UPMU_VOLT_3_3_0_0_V, UPMU_VOLT_3_4_0_0_V, UPMU_VOLT_MAX, UPMU_VOLT_MAX,
                                   UPMU_VOLT_MAX, UPMU_VOLT_MAX, UPMU_VOLT_MAX, UPMU_VOLT_MAX
       	}
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Chip specific KPLEDs list
upmu_kpled_profile_entry upmu_kpled_profile[] =
{
/* KPLED */ {KPLED_CON0}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
// Chip specific CHRs list
upmu_chr_profile_entry upmu_chr_profile[] =
{
/* CHR */ {CHR_CON0,  8, {UPMU_CHARGE_CURRENT_800_MA, UPMU_CHARGE_CURRENT_700_MA, UPMU_CHARGE_CURRENT_600_MA, UPMU_CHARGE_CURRENT_500_MA,
                          UPMU_CHARGE_CURRENT_400_MA, UPMU_CHARGE_CURRENT_300_MA, UPMU_CHARGE_CURRENT_200_MA, UPMU_CHARGE_CURRENT_100_MA,
                          UPMU_CHARGE_CURRENT_MAX,    UPMU_CHARGE_CURRENT_MAX,    UPMU_CHARGE_CURRENT_MAX,    UPMU_CHARGE_CURRENT_MAX,
                          UPMU_CHARGE_CURRENT_MAX,    UPMU_CHARGE_CURRENT_MAX,    UPMU_CHARGE_CURRENT_MAX,    UPMU_CHARGE_CURRENT_MAX}}
};
#endif
// Chip specific CHRs list
upmu_chr_profile_entry upmu_chr_profile[] =
{
/* CHR */
  {
    CHR_CON0,
    8,
    /* chr_current_list */
    {
      PMIC_ADPT_CHARGE_CURRENT_800_MA, PMIC_ADPT_CHARGE_CURRENT_700_MA, PMIC_ADPT_CHARGE_CURRENT_600_MA, PMIC_ADPT_CHARGE_CURRENT_500_MA,
      PMIC_ADPT_CHARGE_CURRENT_400_MA, PMIC_ADPT_CHARGE_CURRENT_300_MA, PMIC_ADPT_CHARGE_CURRENT_200_MA, PMIC_ADPT_CHARGE_CURRENT_100_MA,
      PMIC_ADPT_CHARGE_CURRENT_MAX,    PMIC_ADPT_CHARGE_CURRENT_MAX,    PMIC_ADPT_CHARGE_CURRENT_MAX,    PMIC_ADPT_CHARGE_CURRENT_MAX,
      PMIC_ADPT_CHARGE_CURRENT_MAX,    PMIC_ADPT_CHARGE_CURRENT_MAX,    PMIC_ADPT_CHARGE_CURRENT_MAX,    PMIC_ADPT_CHARGE_CURRENT_MAX
    },
    16,
    /* chr_det_lv_list */
    {
      PMIC_ADPT_VOLT_04_200000_V, PMIC_ADPT_VOLT_04_250000_V, PMIC_ADPT_VOLT_04_300000_V, PMIC_ADPT_VOLT_04_350000_V,
      PMIC_ADPT_VOLT_04_400000_V, PMIC_ADPT_VOLT_04_450000_V, PMIC_ADPT_VOLT_04_500000_V, PMIC_ADPT_VOLT_04_550000_V,
      PMIC_ADPT_VOLT_04_600000_V, PMIC_ADPT_VOLT_06_000000_V, PMIC_ADPT_VOLT_06_500000_V, PMIC_ADPT_VOLT_06_750000_V,
      PMIC_ADPT_VOLT_07_000000_V, PMIC_ADPT_VOLT_07_250000_V, PMIC_ADPT_VOLT_07_500000_V, PMIC_ADPT_VOLT_08_000000_V,
      PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,
      PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,
      PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,
      PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX
    },
    16,
    /* chr_det_hv_list */
    {
      PMIC_ADPT_VOLT_04_200000_V, PMIC_ADPT_VOLT_04_250000_V, PMIC_ADPT_VOLT_04_300000_V, PMIC_ADPT_VOLT_04_350000_V,
      PMIC_ADPT_VOLT_04_400000_V, PMIC_ADPT_VOLT_04_450000_V, PMIC_ADPT_VOLT_04_500000_V, PMIC_ADPT_VOLT_04_550000_V,
      PMIC_ADPT_VOLT_04_600000_V, PMIC_ADPT_VOLT_06_000000_V, PMIC_ADPT_VOLT_06_500000_V, PMIC_ADPT_VOLT_07_000000_V,
      PMIC_ADPT_VOLT_07_500000_V, PMIC_ADPT_VOLT_08_500000_V, PMIC_ADPT_VOLT_09_500000_V, PMIC_ADPT_VOLT_10_500000_V,
      PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,
      PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,
      PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,
      PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX
    },
    4,
    /* chr_vbat_cc_list */
    {
      PMIC_ADPT_VOLT_03_250000_V, PMIC_ADPT_VOLT_03_275000_V, PMIC_ADPT_VOLT_03_300000_V, PMIC_ADPT_VOLT_03_325000_V,
      PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,
      PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,
      PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX
    },
    24,
    /* chr_vbat_cv_list */
    {
      PMIC_ADPT_VOLT_04_000000_V, PMIC_ADPT_VOLT_04_012500_V, PMIC_ADPT_VOLT_04_025000_V, PMIC_ADPT_VOLT_04_037500_V,
      PMIC_ADPT_VOLT_04_050000_V, PMIC_ADPT_VOLT_04_062500_V, PMIC_ADPT_VOLT_04_075000_V, PMIC_ADPT_VOLT_04_087500_V,
      PMIC_ADPT_VOLT_04_100000_V, PMIC_ADPT_VOLT_04_112500_V, PMIC_ADPT_VOLT_04_125000_V, PMIC_ADPT_VOLT_04_137500_V,
      PMIC_ADPT_VOLT_04_150000_V, PMIC_ADPT_VOLT_04_162500_V, PMIC_ADPT_VOLT_04_175000_V, PMIC_ADPT_VOLT_04_187500_V,
      PMIC_ADPT_VOLT_04_200000_V, PMIC_ADPT_VOLT_04_212500_V, PMIC_ADPT_VOLT_04_225000_V, PMIC_ADPT_VOLT_04_237500_V,
      PMIC_ADPT_VOLT_04_250000_V, PMIC_ADPT_VOLT_04_262500_V, PMIC_ADPT_VOLT_04_275000_V, PMIC_ADPT_VOLT_04_287500_V,
      PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,
      PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX
    },
    4,
    /* chr_vbat_ov_list */
    {
      PMIC_ADPT_VOLT_04_325000_V, PMIC_ADPT_VOLT_04_350000_V, PMIC_ADPT_VOLT_04_375000_V, PMIC_ADPT_VOLT_04_116000_V,
      PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX,         PMIC_ADPT_VOLT_MAX
    }
  }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Chip specific OC list
upmu_oc_profile_entry upmu_oc_profile[] =
{
/* OC */ {PMIC_OC_CON0}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Chip specific STRUP list
upmu_strup_profile_entry upmu_strup_profile[] =
{
/* STRUP */ {STRUP_CON0}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Chip specific LPOSC list
upmu_lposc_profile_entry upmu_lposc_profile[] =
{
/* LPOSC */ {LPOSC_CON0}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Chip specific RETENTION list
upmu_retention_profile_entry upmu_retention_profile[] =
{
/* RETENTION */ {RETENTION_CON0}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Setting
#define CONFIG_CHREDT_INT   1	
#define CONFIG_BATON_UNDET_INT	0
#define CONFIG_CHREDT_and_BATON_UNDET_INT   0

#define EINT_CHR_DET_NUM	23
//#define EINT_CHR_DET_SEN	MT65xx_EDGE_SENSITIVE
#define EINT_CHR_DET_SEN	MT65xx_LEVEL_SENSITIVE
#define EINT_CHR_DET_POL	MT65XX_EINT_POL_POS
//#define EINT_CHR_DET_DEB	0x7FF
#define EINT_CHR_DET_DEB	0x0

extern struct wake_lock battery_suspend_lock;
extern void wake_up_bat (void);
extern void dummy_update(void);

CHARGER_TYPE g_ret = CHARGER_UNKNOWN;
int g_charger_in_flag = 0;
int g_first_check=0;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// External Function
void print_BC11_register(void)
{
	printk("REG[0xF702FA24]=0x%x\r\n", INREG16(0xF702FA24));
	printk("REG[0xF702FA28]=0x%x\r\n", INREG16(0xF702FA28));
	printk("REG[0xF705081E]=0x%x\r\n", INREG16(0xF705081E));
	printk("REG[0xF7050800]=0x%x\r\n", INREG8(0xF7050800));
	printk("REG[0xF7050810]=0x%x\r\n", INREG8(0xF7050810));
	printk("REG[0xF7050814]=0x%x\r\n", INREG8(0xF7050814));
	printk("REG[0xF7050872]=0x%x\r\n", INREG8(0xF7050872));
}

CHARGER_TYPE hw_charger_type_detection(void)
{
	CHARGER_TYPE ret 				= CHARGER_UNKNOWN;
	unsigned int CHR_CON_9 			= 0xF702FA24;
	unsigned int CHR_CON_10 		= 0xF702FA28;
	unsigned int USB_U2PHYACR3_2 	= 0xF705081E;
	//unsigned int MEM_ID_USB20		= 0xF7026038;
	//unsigned int PDN_ID_USB			= 0xF7026308;
	unsigned int USBPHYRegs			= 0xF7050800; //U2B20_Base+0x800
	unsigned short wChargerAvail	= 0;
	unsigned short bLineState_B		= 0;
	unsigned short bLineState_C 	= 0;

	//msleep(400);
	//printk("mt_charger_type_detection : start\r\n");

/********* Step 0.0 : enable USB memory and clock *********/
	//PMU_DRV_SetData32(MEM_ID_USB20,0x02,0); //MEM_ID_USB20
	//PMU_DRV_SetData32(MEM_ID_USB20,0x20,0); //MEM_ID_USB20 bit5
	//msleep(1);          
	
	//PMU_DRV_SetData32(PDN_ID_USB,0x80,0); //PDN_ID_USB bit 7
	//PMU_DRV_SetData32(PDN_ID_USB,0x80,1); //PDN_ID_USB bit 7
	hwEnableClock(MT65XX_PDN_PERI_USB, "pmu");
	mdelay(1);

/********* Step 1.0 : PMU_BC11_Detect_Init ***************/
	//PMU_DRV_SetBits16(USB_U2PHYACR3_2,0x04); //USB_U2PHYACR3_2[2]=1
	SETREG8(USB_U2PHYACR3_2,0x04); //USB_U2PHYACR3_2[2]=1
	
	SETREG16(CHR_CON_9,0x0100);
	CLRREG16(CHR_CON_9,0x0100);	
	
	SETREG16(CHR_CON_10,0x0180);//RG_BC11_BIAS_EN=1	
	CLRREG16(CHR_CON_9,0x0003);//RG_BC11_VSRC_EN[1:0]=00
	CLRREG16(CHR_CON_10,0x0040);//RG_BC11_VREF_VTH = 0
	CLRREG16(CHR_CON_10,0x0003);//RG_BC11_CMP_EN[1.0] = 00
	CLRREG16(CHR_CON_10,0x0030);//RG_BC11_IPU_EN[1.0] = 00
	CLRREG16(CHR_CON_10,0x000C);//RG_BC11_IPD_EN[1.0] = 00

/********* Step A *************************************/
	//printk("mt_charger_type_detection : step A\r\n");
	CLRREG16(CHR_CON_10,0x0030);//RG_BC11_IPU_EN[1.0] = 00
	
	SETREG8(USBPHYRegs+0x10,0x0010);//RG_PUPD_BIST_EN = 1
	CLRREG8(USBPHYRegs+0x14,0x0040);//RG_EN_PD_DM=0
	
	SETREG16(CHR_CON_9,0x0002);//RG_BC11_VSRC_EN[1.0] = 10 
	SETREG16(CHR_CON_10,0x0004);//RG_BC11_IPD_EN[1:0] = 01
	CLRREG16(CHR_CON_10,0x0040);//RG_BC11_VREF_VTH = 0
  	SETREG16(CHR_CON_10,0x0001);//RG_BC11_CMP_EN[1.0] = 01
	mdelay(80);
	
	wChargerAvail = INREG16(CHR_CON_10);
	//printk("mt_charger_type_detection : step A : wChargerAvail=%x\r\n", wChargerAvail);
	CLRREG16(CHR_CON_9,0x0003);//RG_BC11_VSRC_EN[1:0]=00
	CLRREG16(CHR_CON_10,0x0004);//RG_BC11_IPD_EN[1.0] = 00
	CLRREG16(CHR_CON_10,0x0003);//RG_BC11_CMP_EN[1.0] = 00
	mdelay(80);

	if(wChargerAvail & 0x0200)
	{
/********* Step B *************************************/
		//printk("mt_charger_type_detection : step B\r\n");

		SETREG16(CHR_CON_10,0x0020); //RG_BC11_IPU_EN[1:0]=10
		mdelay(80);
		
		bLineState_B = INREG8(USBPHYRegs+0x72);
		//printk("mt_charger_type_detection : step B : bLineState_B=%x\r\n", bLineState_B);
		if(bLineState_B & 0x40)
		{
			ret = STANDARD_CHARGER;
			printk("mt_charger_type_detection : step B : STANDARD CHARGER!\r\n");
		}
		else
		{
			ret = CHARGING_HOST;
			printk("mt_charger_type_detection : step B : Charging Host!\r\n");
		}
	}
	else
	{
/********* Step C *************************************/
		//printk("mt_charger_type_detection : step C\r\n");

		SETREG16(CHR_CON_10,0x0010); //RG_BC11_IPU_EN[1:0]=01
		SETREG16(CHR_CON_10,0x0001);//RG_BC11_CMP_EN[1.0] = 01		
		mdelay(80);
		
		bLineState_C = INREG16(CHR_CON_10);
		//printk("mt_charger_type_detection : step C : bLineState_C=%x\r\n", bLineState_C);
		if(bLineState_C & 0x0200)
		{
			ret = NONSTANDARD_CHARGER;
			printk("mt_charger_type_detection : step C : UNSTANDARD CHARGER!\r\n");
		}
		else
		{
			ret = STANDARD_HOST;
			printk("mt_charger_type_detection : step C : Standard USB Host!\r\n");
		}
	}
/********* Finally setting *******************************/
	CLRREG16(CHR_CON_9,0x0003);//RG_BC11_VSRC_EN[1:0]=00
	CLRREG16(CHR_CON_10,0x0040);//RG_BC11_VREF_VTH = 0
	CLRREG16(CHR_CON_10,0x0003);//RG_BC11_CMP_EN[1.0] = 00
	CLRREG16(CHR_CON_10,0x0030);//RG_BC11_IPU_EN[1.0] = 00
	CLRREG16(CHR_CON_10,0x000C);//RG_BC11_IPD_EN[1.0] = 00
	CLRREG16(CHR_CON_10,0x0080);//RG_BC11_BIAS_EN=0
	
	CLRREG8(USB_U2PHYACR3_2,0x04); //USB_U2PHYACR3_2[2]=0

	hwDisableClock(MT65XX_PDN_PERI_USB, "pmu");

	if( (ret==STANDARD_HOST) || (ret==CHARGING_HOST) )
	{
		printk("mt_charger_type_detection : SW workaround for USB\r\n");
		SETREG16(CHR_CON_10,0x0100); // RG_BC11_BB_CTRL=1
		SETREG16(CHR_CON_10,0x0080); // RG_BC11_BIAS_EN=1
		SETREG16(CHR_CON_9,0x0003);  // RG_BC11_VSRC_EN[1.0] = 11		
		printk("CHR_CON_9=0x%x, CHR_CON_10=0x%x\r\n",INREG16(CHR_CON_9), INREG16(CHR_CON_10));		
	}

	//step4:done, ret the type
	return ret;
	
}


CHARGER_TYPE mt_charger_type_detection(void)
{
    if( (g_charger_in_flag == 1) && (g_first_check == 0) )
    {
		g_first_check = 1;
		g_ret = hw_charger_type_detection();
    }
    else
    {
		//printk("Got data !!\r\n");
    }

    return g_ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Internal Function
void pmu6573_dump_register(void)
{
	printk("REG[0xF702FA1C]=%x\r\n", INREG16(0xF702FA1C));
	printk("REG[0xF702FA18]=%x\r\n", INREG16(0xF702FA18));
	printk("REG[0xF702FA24]=%x\r\n", INREG16(0xF702FA24));
	printk("REG[0xF702FA0C]=%x\r\n", INREG16(0xF702FA0C));

	printk("REG[0xF702F900]=%x\r\n", INREG16(0xF702F900));
	printk("REG[0xF702F904]=%x\r\n", INREG16(0xF702F904));
	printk("REG[0xF702F908]=%x\r\n", INREG16(0xF702F908));
	printk("REG[0xF702F90C]=%x\r\n", INREG16(0xF702F90C));
	printk("REG[0xF702F910]=%x\r\n", INREG16(0xF702F910));
	printk("REG[0xF702F914]=%x\r\n\n", INREG16(0xF702F914));
	
	printk("REG[0xF702F920]=%x\r\n", INREG16(0xF702F920));
	printk("REG[0xF702F924]=%x\r\n", INREG16(0xF702F924));
	printk("REG[0xF702F928]=%x\r\n", INREG16(0xF702F928));
	printk("REG[0xF702F92C]=%x\r\n", INREG16(0xF702F92C));
	printk("REG[0xF702F930]=%x\r\n", INREG16(0xF702F930));
	printk("REG[0xF702F934]=%x\r\n\n", INREG16(0xF702F934));

	printk("REG[0xF702F940]=%x\r\n", INREG16(0xF702F940));
	printk("REG[0xF702F944]=%x\r\n", INREG16(0xF702F944));
	printk("REG[0xF702F948]=%x\r\n", INREG16(0xF702F948));
	printk("REG[0xF702F94C]=%x\r\n", INREG16(0xF702F94C));
	printk("REG[0xF702F950]=%x\r\n", INREG16(0xF702F950));
	printk("REG[0xF702F954]=%x\r\n\n", INREG16(0xF702F954));

	printk("REG[0xF702F960]=%x\r\n", INREG16(0xF702F960));
	printk("REG[0xF702F964]=%x\r\n", INREG16(0xF702F964));
	printk("REG[0xF702F968]=%x\r\n", INREG16(0xF702F968));
	printk("REG[0xF702F96C]=%x\r\n", INREG16(0xF702F96C));
	printk("REG[0xF702F970]=%x\r\n", INREG16(0xF702F970));
	printk("REG[0xF702F974]=%x\r\n", INREG16(0xF702F974));	

	printk("REG[0xF702F760]=%x\r\n", INREG16(0xF702F760));	
	printk("REG[0xF702F764]=%x\r\n", INREG16(0xF702F764));

	printk("REG[0xF702F810]=%x\r\n", INREG16(0xF702F810));
	printk("REG[0xF702F814]=%x\r\n", INREG16(0xF702F814));

	printk("REG[0xF702F7C0]=%x\r\n", INREG16(0xF702F7C0));
	printk("REG[0xF702F7C4]=%x\r\n", INREG16(0xF702F7C4));	

	printk("REG[0xF702F700]=%x\r\n", INREG16(0xF702F700));
	printk("REG[0xF702F704]=%x\r\n", INREG16(0xF702F704));

	printk("REG[0xF702FB00]=%x\r\n", INREG16(0xF702FB00));
	printk("REG[0xF702FB04]=%x\r\n", INREG16(0xF702FB04));

	printk("REG[0xF702F100]=%x\r\n", INREG16(0xF702F100));
	printk("REG[0xF702F104]=%x\r\n", INREG16(0xF702F104));
	printk("REG[0xF702F108]=%x\r\n", INREG16(0xF702F108));
	printk("REG[0xF702F114]=%x\r\n", INREG16(0xF702F114));
	printk("REG[0xF702FE88]=%x\r\n", INREG16(0xF702FE88));

	printk("REG[0xF7024104]=%x\r\n", INREG16(0xF7024104));
	printk("REG[0xF702F210]=%x\r\n", INREG16(0xF702F210));
	printk("REG[0xF702FA24]=%x\r\n", INREG16(0xF702FA24));
	printk("REG[0xF702FA04]=%x\r\n", INREG16(0xF702FA04));

	printk("REG[0xF702FC80]=%x\r\n", INREG16(0xF702FC80));

	printk("REG[0xF702F78C]=%x\r\n", INREG16(0xF702F78C));
	printk("REG[0xF702F79C]=%x\r\n", INREG16(0xF702F79C));
	
}

void pmu6573_dump_register_2(void)
{
	printk("REG[0xF702F730]=%x\r\n", INREG16(0xF702F730));
	printk("REG[0xF702F734]=%x\r\n", INREG16(0xF702F734));

	printk("REG[0xF702F740]=%x\r\n", INREG16(0xF702F740));
	printk("REG[0xF702F744]=%x\r\n", INREG16(0xF702F744));

	printk("REG[0xF702F780]=%x\r\n", INREG16(0xF702F780));
	printk("REG[0xF702F784]=%x\r\n", INREG16(0xF702F784));

	printk("REG[0xF702F790]=%x\r\n", INREG16(0xF702F790));
	printk("REG[0xF702F794]=%x\r\n", INREG16(0xF702F794));

	printk("REG[0xF702F7B0]=%x\r\n", INREG16(0xF702F7B0));
	printk("REG[0xF702F7B4]=%x\r\n", INREG16(0xF702F7B4));

	printk("REG[0xF702F7D0]=%x\r\n", INREG16(0xF702F7D0));
	printk("REG[0xF702F7D4]=%x\r\n", INREG16(0xF702F7D4));	

	printk("REG[0xF702F7E0]=%x\r\n", INREG16(0xF702F7E0));
	printk("REG[0xF702F7E4]=%x\r\n", INREG16(0xF702F7E4));

	printk("REG[0xF702F800]=%x\r\n", INREG16(0xF702F800));
	printk("REG[0xF702F804]=%x\r\n", INREG16(0xF702F804));

	printk("REG[0xF702FC80]=%x\r\n", INREG16(0xF702FC80));	

	printk("REG[0xF702FA00]=%x\r\n", INREG16(0xF702FA00));
	printk("REG[0xF702FA0C]=%x\r\n", INREG16(0xF702FA0C));

	printk("REG[0xF702FE84]=%x\r\n", INREG16(0xF702FE84));	

	printk("REG[0xF702F700]=%x\r\n", INREG16(0xF702F700));	
}

void pmu6573_turn_on_all_LDO(void)
{
	printk("****LDO 1---------------------------- \n");
	printk("****[mt6573_pmu_init] enable LDO_VA28 \n");
	upmu_ldo_enable(LDO_VA28, KAL_TRUE);
	printk("****[mt6573_pmu_init] enable LDO_VA25 \n");
	upmu_ldo_enable(LDO_VA25, KAL_TRUE);
	printk("****[mt6573_pmu_init] enable LDO_VA12 \n");
	upmu_ldo_enable(LDO_VA12, KAL_TRUE);
	printk("****[mt6573_pmu_init] enable LDO_VRTC \n");
	upmu_ldo_enable(LDO_VRTC, KAL_TRUE);	
	printk("****[mt6573_pmu_init] enable LDO_VMIC \n");
	upmu_ldo_enable(LDO_VMIC, KAL_TRUE);
	printk("****[mt6573_pmu_init] enable LDO_VTV \n");
	upmu_ldo_enable(LDO_VTV, KAL_TRUE);
	printk("****[mt6573_pmu_init] enable LDO_VAUDN \n");
	upmu_ldo_enable(LDO_VAUDN, KAL_TRUE);
	printk("****[mt6573_pmu_init] enable LDO_VAUDP \n");
	upmu_ldo_enable(LDO_VAUDP, KAL_TRUE);	

	printk("****LDO 2---------------------------- \n");
	printk("****[mt6573_pmu_init] enable LDO_VRF \n");
	upmu_ldo_enable(LDO_VRF, KAL_TRUE);	
	printk("****[mt6573_pmu_init] enable LDO_VCAMA \n");
	upmu_ldo_enable(LDO_VCAMA, KAL_TRUE);
	printk("****[mt6573_pmu_init] enable LDO_VCAMD \n");
	upmu_ldo_enable(LDO_VCAMD, KAL_TRUE);
	printk("****[mt6573_pmu_init] enable LDO_VIO \n");
	upmu_ldo_enable(LDO_VIO, KAL_TRUE);	
	printk("****[mt6573_pmu_init] enable LDO_VUSB \n");
	upmu_ldo_enable(LDO_VUSB, KAL_TRUE);
	printk("****[mt6573_pmu_init] enable LDO_VSIM \n");
	upmu_ldo_enable(LDO_VSIM, KAL_TRUE);
	printk("****[mt6573_pmu_init] enable LDO_VSIM2 \n");
	upmu_ldo_enable(LDO_VSIM2, KAL_TRUE);
	//printk("****[mt6573_pmu_init] enable LDO_VIBR \n");
	//upmu_ldo_enable(LDO_VIBR, KAL_TRUE);	
	printk("****[mt6573_pmu_init] enable LDO_VMC \n");
	upmu_ldo_enable(LDO_VMC, KAL_TRUE);	
	printk("****[mt6573_pmu_init] enable LDO_VCAMA2 \n");
	upmu_ldo_enable(LDO_VCAMA2, KAL_TRUE);	
	printk("****[mt6573_pmu_init] enable LDO_VCAMD2 \n");
	upmu_ldo_enable(LDO_VCAMD2, KAL_TRUE);	
	printk("****[mt6573_pmu_init] enable LDO_VM12 \n");
	upmu_ldo_enable(LDO_VM12, KAL_TRUE);	
	printk("****[mt6573_pmu_init] enable LDO_VM12_INT \n");
	upmu_ldo_enable(LDO_VM12_INT, KAL_TRUE);	
}

void pmu6573_hw_init(void)
{
	printk("****[mt6573_pmu_init] INIT : Depending on the PMU Driver Setting SPEC 0.5.1 \n");

	printk("****[mt6573_pmu_init] INIT : HW init settings for all platforms : charger timer\n");
	upmu_chrwdt_td(CHR, 0x3);
	upmu_chrwdt_int_enable(CHR, KAL_TRUE);
	upmu_chrwdt_enable(CHR, KAL_TRUE);

	printk("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VCORE\n");
	upmu_buck_disable_anti_undershoot(BUCK_VCORE, KAL_TRUE); // V0.3
	//upmu_buck_rs(BUCK_VCORE,BUCK_REMOTE_SENSE);// V0.3.2
	upmu_buck_rs(BUCK_VCORE,BUCK_LOCAL_SENSE);// V0.4
	//upmu_buck_normal_voltage_adjust(BUCK_VCORE, UPMU_VOLT_1_2_0_0_V);
	upmu_buck_normal_voltage_adjust(BUCK_VCORE, UPMU_VOLT_1_3_0_0_V); // V0.5
	upmu_buck_sleep_voltage_adjust(BUCK_VCORE, UPMU_VOLT_1_2_0_0_V);
	upmu_buck_burst(BUCK_VCORE, 0x0);
	upmu_buck_csl(BUCK_VCORE, 0x3);
	upmu_buck_csr(BUCK_VCORE, 0x5); //0.3
	upmu_buck_oc_td(BUCK_VCORE, BUCK_OC_TD_100_US);

	printk("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VAPROC\n");
	upmu_buck_disable_anti_undershoot(BUCK_VAPROC, KAL_TRUE); // V0.3
	//upmu_buck_rs(BUCK_VAPROC,BUCK_REMOTE_SENSE);// V0.3.2
	upmu_buck_rs(BUCK_VAPROC,BUCK_LOCAL_SENSE);// V0.4
	upmu_buck_normal_voltage_adjust(BUCK_VAPROC, UPMU_VOLT_1_3_7_5_V); // V0.5.1
	upmu_buck_burst(BUCK_VAPROC, 0x0);
	upmu_buck_csl(BUCK_VAPROC, 0x3);
	upmu_buck_csr(BUCK_VAPROC, 0x5); //0.3
	upmu_buck_oc_td(BUCK_VAPROC, BUCK_OC_TD_100_US);

	printk("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VIO1V8\n");
	upmu_buck_disable_anti_undershoot(BUCK_VIO1V8, KAL_TRUE); // V0.3
	upmu_buck_normal_voltage_adjust(BUCK_VIO1V8, UPMU_VOLT_1_8_0_0_V);
	upmu_buck_burst(BUCK_VIO1V8, 0x0);
	upmu_buck_csl(BUCK_VIO1V8, 0x3);
	upmu_buck_csr(BUCK_VIO1V8, 0x5); //0.3
	upmu_buck_oc_td(BUCK_VIO1V8, BUCK_OC_TD_100_US);

	printk("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VRF1V8\n");
	//upmu_buck_enable(BUCK_VRF1V8, KAL_TRUE); //0.3
	upmu_buck_enable(BUCK_VRF1V8, KAL_FALSE); //0.3.1
	upmu_buck_normal_voltage_adjust(BUCK_VRF1V8, UPMU_VOLT_1_8_0_0_V);
	upmu_buck_burst(BUCK_VRF1V8, 0x0);
	upmu_buck_csl(BUCK_VRF1V8, 0x7);
	upmu_buck_ocfb_enable(BUCK_VRF1V8, KAL_FALSE);
	upmu_buck_oc_auto_off(BUCK_VRF1V8, KAL_FALSE);
	upmu_buck_stb_td(BUCK_VRF1V8, BUCK_STB_TD_219_US);
	upmu_buck_csl(BUCK_VRF1V8, 0x3); //0.3
	upmu_buck_csr(BUCK_VRF1V8, 0x7); //0.3

	printk("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VUSB\n");
	upmu_ldo_ocfb_enable(LDO_VUSB, KAL_TRUE);
	upmu_ldo_oc_auto_off(LDO_VUSB, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VUSB, KAL_TRUE);
	upmu_ldo_cal(LDO_VUSB, 0x0);
	upmu_ldo_stb_td(LDO_VUSB, LDO_STB_TD_200_US);

	printk("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VM12_INT\n");
	upmu_ldo_ocfb_enable(LDO_VM12_INT, KAL_TRUE);
	upmu_ldo_oc_auto_off(LDO_VM12_INT, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VM12_INT, KAL_TRUE);
	upmu_ldo_oc_td(LDO_VM12_INT, LDO_OC_TD_100_US);

	printk("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VMC\n");
	upmu_ldo_ocfb_enable(LDO_VMC, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VMC, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VMC, KAL_TRUE);
	//upmu_ldo_vol_sel(LDO_VMC, UPMU_VOLT_3_0_0_0_V);
	upmu_ldo_vol_sel(LDO_VMC, UPMU_VOLT_3_3_0_0_V); //v3.3 for SMT
	upmu_ldo_cal(LDO_VMC, 0x0);
	upmu_ldo_stb_td(LDO_VMC, LDO_STB_TD_200_US);

	printk("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VRF\n");
	upmu_ldo_ocfb_enable(LDO_VRF, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VRF, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VRF, KAL_TRUE);
	upmu_ldo_cal(LDO_VRF, 0x0);
	upmu_ldo_stb_td(LDO_VRF, LDO_STB_TD_200_US);
	
	printk("****[mt6573_pmu_init] INIT : HW init settings for all platforms : KPLED\n");
	upmu_kpled_sfstrt_en(KPLED, KAL_TRUE);
	upmu_kpled_sfstrt_c(KPLED, KPLED_SFSTRT_C_31US_X_1);
	
}

void pmu6573_pmu_customization(void)
{
	printk("****[mt6573_pmu_init] INIT : HW init settings for customization : VCAMA\n");
	upmu_ldo_ocfb_enable(LDO_VCAMA, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VCAMA, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VCAMA, KAL_TRUE);
	upmu_ldo_vol_sel(LDO_VCAMA, UPMU_VOLT_2_8_0_0_V);
	//upmu_ldo_cal(LDO_VCAMA, 0x0);
	upmu_ldo_stb_td(LDO_VCAMA, LDO_STB_TD_200_US);
	
	printk("****[mt6573_pmu_init] INIT : HW init settings for customization : VCAMD\n");
	upmu_ldo_ocfb_enable(LDO_VCAMD, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VCAMD, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VCAMD, KAL_TRUE);
	upmu_ldo_vol_sel(LDO_VCAMD, UPMU_VOLT_1_8_0_0_V);
	//upmu_ldo_cal(LDO_VCAMD, 0x0);
	upmu_ldo_stb_td(LDO_VCAMD, LDO_STB_TD_200_US);
	
	printk("****[mt6573_pmu_init] INIT : HW init settings for customization : VSIM\n");
	upmu_ldo_ocfb_enable(LDO_VSIM, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VSIM, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VSIM, KAL_TRUE);
	//upmu_ldo_cal(LDO_VSIM, 0x0);
	upmu_ldo_stb_td(LDO_VSIM, LDO_STB_TD_200_US);

	printk("****[mt6573_pmu_init] INIT : HW init settings for customization : VSIM2\n");
	upmu_ldo_ocfb_enable(LDO_VSIM2, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VSIM2, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VSIM2, KAL_TRUE);
	//upmu_ldo_cal(LDO_VSIM2, 0x0);
	upmu_ldo_stb_td(LDO_VSIM2, LDO_STB_TD_200_US);

	printk("****[mt6573_pmu_init] INIT : HW init settings for customization : VIBR\n");
	upmu_ldo_ocfb_enable(LDO_VIBR, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VIBR, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VIBR, KAL_TRUE);
	upmu_ldo_vol_sel(LDO_VIBR, UPMU_VOLT_1_3_0_0_V);
	//upmu_ldo_cal(LDO_VIBR, 0x0);
	upmu_ldo_stb_td(LDO_VIBR, LDO_STB_TD_200_US);
	
	printk("****[mt6573_pmu_init] INIT : HW init settings for customization : VCAMA2\n");
	upmu_ldo_ocfb_enable(LDO_VCAMA2, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VCAMA2, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VCAMA2, KAL_TRUE);
	upmu_ldo_vol_sel(LDO_VCAMA2, UPMU_VOLT_2_8_0_0_V);
	//upmu_ldo_cal(LDO_VCAMA2, 0x0);
	upmu_ldo_stb_td(LDO_VCAMA2, LDO_STB_TD_200_US);
	
	printk("****[mt6573_pmu_init] INIT : HW init settings for customization : VCAMD2\n");
	upmu_ldo_ocfb_enable(LDO_VCAMD2, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VCAMD2, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VCAMD2, KAL_TRUE);
	upmu_ldo_vol_sel(LDO_VCAMD2, UPMU_VOLT_1_8_0_0_V);
	//upmu_ldo_cal(LDO_VCAMD2, 0x0);
	upmu_ldo_stb_td(LDO_VCAMD2, LDO_STB_TD_200_US);
	
	printk("****[mt6573_pmu_init] INIT : HW init settings for customization : VM12\n");
	upmu_ldo_ocfb_enable(LDO_VM12, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VM12, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VM12, KAL_TRUE);
	//upmu_ldo_cal(LDO_VM12, 0x0);
	upmu_ldo_stb_td(LDO_VM12, LDO_STB_TD_200_US);
	
	printk("****[mt6573_pmu_init] INIT : HW init settings for customization : KPLED\n");
	upmu_kpled_sel(KPLED, 0x7);
	
}

void pmu6573_new_hw_init(void)
{
	unsigned int ret_val=0;
	unsigned int ret_val_check=0;

	printk("****[mt6573_pmu_init] INIT : Depending on the PMU Driver Setting SPEC 0.8.3 +Ray \n");

	printk("****[mt6573_pmu_init] INIT : HW init settings for all platforms : charger timer\n");
	upmu_chrwdt_int_enable(CHR, KAL_TRUE);
	upmu_chrwdt_td(CHR, 0x3);
	upmu_bc11_reset_circuit(CHR, 1);
	upmu_csdac_dly(CHR,0x3);
	upmu_csdac_stp(CHR,0x0);

	printk("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VCORE\n");
	upmu_buck_disable_anti_undershoot(BUCK_VCORE, KAL_FALSE); 
	upmu_buck_rs(BUCK_VCORE,BUCK_LOCAL_SENSE);	
	upmu_buck_normal_voltage_adjust(BUCK_VCORE, UPMU_VOLT_1_2_7_5_V);//Ray
	upmu_buck_burst(BUCK_VCORE, 0x2);
	upmu_buck_ical(BUCK_VCORE, 0x2);
	upmu_buck_csl(BUCK_VCORE, 0x3);
	upmu_buck_csr(BUCK_VCORE, 0x6);
	upmu_buck_oc_td(BUCK_VCORE, BUCK_OC_TD_100_US);
	upmu_buck_cpmcksel(BUCK_VCORE, 0x0);
	upmu_buck_modeset(BUCK_VCORE, BUCK_AUTO_MODE);
	upmu_buck_vosel(BUCK_VCORE, 0x3);
	upmu_buck_adjcksel(BUCK_VCORE, 0x0);
	upmu_buck_zx_pdn(BUCK_VCORE, 0x0);

	printk("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VAPROC\n");	
	upmu_buck_disable_anti_undershoot(BUCK_VAPROC, KAL_FALSE); 	
	upmu_buck_rs(BUCK_VAPROC,BUCK_LOCAL_SENSE);	
	upmu_buck_normal_voltage_adjust(BUCK_VAPROC, UPMU_VOLT_1_3_7_5_V);//Ray	
	upmu_buck_burst(BUCK_VAPROC, 0x2);
	upmu_buck_ical(BUCK_VAPROC, 0x2);	
	upmu_buck_csl(BUCK_VAPROC, 0x3);	
	upmu_buck_csr(BUCK_VAPROC, 0x6);	
	upmu_buck_oc_td(BUCK_VAPROC, BUCK_OC_TD_100_US);
	upmu_buck_cpmcksel(BUCK_VAPROC, 0x0);
	upmu_buck_modeset(BUCK_VAPROC, BUCK_AUTO_MODE);
	upmu_buck_vosel(BUCK_VAPROC, 0x3);
	upmu_buck_zx_pdn(BUCK_VAPROC, 0x0);

	printk("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VIO1V8\n");
	upmu_buck_disable_anti_undershoot(BUCK_VIO1V8, KAL_FALSE);	
	upmu_buck_normal_voltage_adjust(BUCK_VIO1V8, UPMU_VOLT_1_8_0_0_V);	
	upmu_buck_burst(BUCK_VIO1V8, 0x2);
	upmu_buck_ical(BUCK_VIO1V8, 0x2);	
	upmu_buck_csl(BUCK_VIO1V8, 0x3);	
	upmu_buck_csr(BUCK_VIO1V8, 0x7);	
	upmu_buck_oc_td(BUCK_VIO1V8, BUCK_OC_TD_100_US);
	upmu_buck_cpmcksel(BUCK_VIO1V8, 0x0);
	upmu_buck_modeset(BUCK_VIO1V8, BUCK_AUTO_MODE);
	upmu_buck_vosel(BUCK_VIO1V8, 0x2);
	upmu_buck_zx_pdn(BUCK_VIO1V8, 0x0);

	printk("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VRF1V8\n");	
	upmu_buck_enable(BUCK_VRF1V8, KAL_FALSE);	
	upmu_buck_normal_voltage_adjust(BUCK_VRF1V8, UPMU_VOLT_1_8_0_0_V);	
	//upmu_buck_burst(BUCK_VRF1V8, 0x2);
	upmu_buck_burst(BUCK_VRF1V8, 0x0);
	upmu_buck_ical(BUCK_VRF1V8, 0x2);	
	upmu_buck_csl(BUCK_VRF1V8, 0x2);	
	upmu_buck_ocfb_enable(BUCK_VRF1V8, KAL_FALSE);	
	upmu_buck_oc_auto_off(BUCK_VRF1V8, KAL_FALSE);	
	upmu_buck_stb_td(BUCK_VRF1V8, BUCK_STB_TD_219_US);
	//upmu_buck_csr(BUCK_VRF1V8, 0x4);
	upmu_buck_csr(BUCK_VRF1V8, 0x0);
	upmu_buck_cpmcksel(BUCK_VRF1V8, 0x0);
	//upmu_buck_modeset(BUCK_VRF1V8, BUCK_AUTO_MODE);
	upmu_buck_modeset(BUCK_VRF1V8, BUCK_FORCE_PWM_MODE);
	upmu_buck_vosel(BUCK_VRF1V8, 0x2);
	//upmu_buck_zx_pdn(BUCK_VRF1V8, 0x0);
	upmu_buck_zx_pdn(BUCK_VRF1V8, 0x1);

	printk("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VUSB\n");
	upmu_ldo_ocfb_enable(LDO_VUSB, KAL_TRUE);	
	upmu_ldo_oc_auto_off(LDO_VUSB, KAL_FALSE);	
	upmu_ldo_ndis_enable(LDO_VUSB, KAL_TRUE);
	upmu_ldo_stb_td(LDO_VUSB, LDO_STB_TD_200_US);

	printk("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VM12_INT\n");
	upmu_ldo_ocfb_enable(LDO_VM12_INT, KAL_TRUE);	
	upmu_ldo_oc_auto_off(LDO_VM12_INT, KAL_FALSE);	
	upmu_ldo_ndis_enable(LDO_VM12_INT, KAL_TRUE);	
	upmu_ldo_oc_td(LDO_VM12_INT, LDO_OC_TD_100_US);
	
	printk("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VMC\n");
	upmu_ldo_ocfb_enable(LDO_VMC, KAL_TRUE);	
	upmu_ldo_oc_auto_off(LDO_VMC, KAL_FALSE);	
	upmu_ldo_ndis_enable(LDO_VMC, KAL_TRUE);	
	upmu_ldo_vol_sel(LDO_VMC, UPMU_VOLT_3_3_0_0_V);	
	upmu_ldo_stb_td(LDO_VMC, LDO_STB_TD_200_US);

	printk("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VRF\n");
	upmu_ldo_ocfb_enable(LDO_VRF, KAL_FALSE);	
	upmu_ldo_oc_auto_off(LDO_VRF, KAL_FALSE);	
	upmu_ldo_ndis_enable(LDO_VRF, KAL_TRUE);	
	upmu_ldo_stb_td(LDO_VRF, LDO_STB_TD_200_US);

#if 1
	printk("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VPA (BB)\n");
	upmu_buck_boost_enable(BUCK_BOOST, KAL_FALSE);
	upmu_buck_boost_cs_adj(BUCK_BOOST, 0x4);
	upmu_buck_boost_rc(BUCK_BOOST, 0x2);
	upmu_buck_boost_cc(BUCK_BOOST, 0x0);
	upmu_buck_boost_fpwm(BUCK_BOOST, 0x1);
	//upmu_buck_boost_voltage_tune_0(BUCK_BOOST, 0x0);
	//upmu_buck_boost_voltage_tune_1(BUCK_BOOST, 0x0);
	//upmu_buck_boost_voltage_tune_2(BUCK_BOOST, 0x0);
	//upmu_buck_boost_voltage_tune_3(BUCK_BOOST, 0x0);
	//upmu_buck_boost_voltage_tune_4(BUCK_BOOST, 0x0);
	//upmu_buck_boost_voltage_tune_5(BUCK_BOOST, 0x0);
	//upmu_buck_boost_voltage_tune_6(BUCK_BOOST, 0x0);
	//upmu_buck_boost_voltage_tune_7(BUCK_BOOST, 0x0);
#endif	

//#if 0 
	//upmu_lposc_digtal_circuit_enable(LPOCS, KAL_TRUE);
	printk("****[mt6573_pmu_init] INIT : HW init settings for all platforms : LPOSC + 0\n");
	upmu_lposc_i_bias_cali(LPOCS, CALI_0_9_X);
	upmu_lposc_buck_boost_freq_divider(LPOCS, DIVIDER_RATIO_4);
	upmu_lposc_buck_output_freq_switching(LPOCS, FREQ_1_60_MHz);
	upmu_lposc_fd_resolution_adjust(LPOCS, PERCENT_1_0);
#if 1	
	upmu_lposc_init_dac_enable(LPOCS, KAL_FALSE);
	upmu_lpocs_sw_mode(KAL_TRUE);
	upmu_lposc_buck_boost_enable(LPOCS, KAL_TRUE);
#endif
	
	ret_val = INREG16(0xF7024104);
	ret_val_check = ret_val;
	printk("****[mt6573_pmu_init] INIT : HW init settings for all platforms : TRIM STRUP (%x)\n", ret_val);	
	if(ret_val_check & 0x8000)
	{
		ret_val = ret_val & 0x7800;
		ret_val = (ret_val >> 11);
		PMU_DRV_SetData16(0xF702F210, 0x00F0, ((kal_uint16)(ret_val) << 4));
	}
	else
	{
		PMU_DRV_SetData16(0xF702F210, 0x00F0, ((kal_uint16)(0x0) << 4));
	}
	
	ret_val = INREG16(0xF7024104);
	ret_val_check = ret_val;
	printk("****[mt6573_pmu_init] INIT : HW init settings for all platforms : TRIM OVP (%x)\n", ret_val);	
	if(ret_val_check & 0x0400)
	{
		ret_val = ret_val & 0x03C0;
		ret_val = (ret_val >> 6);
		PMU_DRV_SetData16(0xF702FA24, 0x00F0, ((kal_uint16)(ret_val) << 4));
	}
	else
	{
		PMU_DRV_SetData16(0xF702FA24, 0x00F0, ((kal_uint16)(0x8) << 4));
	}
	
	ret_val = INREG16(0xF7024104);
	ret_val_check = ret_val;
	printk("****[mt6573_pmu_init] INIT : HW init settings for all platforms : TRIM CV_VTH (%x)\n", ret_val);	
	if(ret_val_check & 0x0020)
	{
		ret_val = ret_val & 0x001F;
		PMU_DRV_SetData16(0xF702FA04, 0x001F, ((kal_uint16)(ret_val)));
	}
	else
	{
		PMU_DRV_SetData16(0xF702FA04, 0x001F, ((kal_uint16)(0x0)));
	}	
	//--------------------------------------------------------------------------------------------------
	ret_val = INREG32(0xF7024104);
	ret_val_check = ret_val;	
	if(ret_val_check & 0x00200000)
	{
		ret_val = ret_val & 0x001F0000;
		ret_val = (ret_val >> 16);
		ret_val = (ret_val << 4);
		PMU_DRV_SetData16(0xF702F940, 0x01F0, ((kal_uint16)(ret_val)));
		printk("****[mt6573_pmu_init] INIT : Need TRIM VAPROC\r\n");
	}
	else
	{
		printk("****[mt6573_pmu_init] INIT : No need TRIM VAPROC\r\n");
	}
	printk("****[mt6573_pmu_init] INIT : HW init settings for all platforms : TRIM VAPROC (%x,%x,%x)\n", 
		ret_val_check, ret_val, INREG16(0xF702F940));	
	//--------------------------------------------------------------------------------------------------
	ret_val = INREG32(0xF7024104);
	ret_val_check = ret_val;	
	if(ret_val_check & 0x08000000)
	{
		ret_val = ret_val & 0x07C00000;
		ret_val = (ret_val >> 22);
		ret_val = (ret_val << 4);
		PMU_DRV_SetData16(0xF702F900, 0x01F0, ((kal_uint16)(ret_val)));
		printk("****[mt6573_pmu_init] INIT : Need TRIM VCORE\r\n");
	}
	else
	{
		printk("****[mt6573_pmu_init] INIT : No need TRIM VCORE\r\n");
	}
	printk("****[mt6573_pmu_init] INIT : HW init settings for all platforms : TRIM VCORE (%x,%x,%x)\n", 
		ret_val_check, ret_val, INREG16(0xF702F900));
	//--------------------------------------------------------------------------------------------------
	printk("****[mt6573_pmu_init] INIT : HW init settings for all platforms : KPLED\n");
	upmu_kpled_sfstrt_en(KPLED, KAL_TRUE);
	upmu_kpled_sfstrt_c(KPLED, KPLED_SFSTRT_C_31US_X_1);

	printk("****[mt6573_pmu_init] INIT : HW init settings for all platforms : SIM\n");
	upmu_ldo_simxio_drv(LDO_VSIM, 0x0);
	upmu_ldo_simx_bias(LDO_VSIM, 0x2);
	upmu_ldo_simx_srp(LDO_VSIM, 0x0);
	upmu_ldo_simx_srn(LDO_VSIM, 0x0);
	upmu_ldo_simxio_drv(LDO_VSIM2, 0x0);
	upmu_ldo_simx_bias(LDO_VSIM2, 0x2);
	upmu_ldo_simx_srp(LDO_VSIM2, 0x0);
	upmu_ldo_simx_srn(LDO_VSIM2, 0x0);
	
}

void pmu6573_new_pmu_customization(void)
{
	printk("****[mt6573_pmu_init] INIT : HW init settings for customization : VCAMA\n");
	upmu_ldo_ocfb_enable(LDO_VCAMA, KAL_FALSE);	
	upmu_ldo_oc_auto_off(LDO_VCAMA, KAL_FALSE);	
	upmu_ldo_ndis_enable(LDO_VCAMA, KAL_TRUE);	
	upmu_ldo_vol_sel(LDO_VCAMA, UPMU_VOLT_2_8_0_0_V);	
	upmu_ldo_stb_td(LDO_VCAMA, LDO_STB_TD_200_US);

	printk("****[mt6573_pmu_init] INIT : HW init settings for customization : VCAMD\n");
	upmu_ldo_ocfb_enable(LDO_VCAMD, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VCAMD, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VCAMD, KAL_TRUE);
	upmu_ldo_vol_sel(LDO_VCAMD, UPMU_VOLT_1_3_0_0_V);
	upmu_ldo_cal(LDO_VCAMD, 0x0);
	upmu_ldo_stb_td(LDO_VCAMD, LDO_STB_TD_200_US);

	printk("****[mt6573_pmu_init] INIT : HW init settings for customization : VSIM\n");
	upmu_ldo_ocfb_enable(LDO_VSIM, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VSIM, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VSIM, KAL_TRUE);
	upmu_ldo_stb_td(LDO_VSIM, LDO_STB_TD_200_US);

	printk("****[mt6573_pmu_init] INIT : HW init settings for customization : VSIM2\n");
	upmu_ldo_ocfb_enable(LDO_VSIM2, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VSIM2, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VSIM2, KAL_TRUE);
	upmu_ldo_stb_td(LDO_VSIM2, LDO_STB_TD_200_US);

	printk("****[mt6573_pmu_init] INIT : HW init settings for customization : VIBR\n");
	upmu_ldo_ocfb_enable(LDO_VIBR, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VIBR, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VIBR, KAL_TRUE);
	upmu_ldo_vol_sel(LDO_VIBR, UPMU_VOLT_1_3_0_0_V);	
	upmu_ldo_stb_td(LDO_VIBR, LDO_STB_TD_200_US);

	printk("****[mt6573_pmu_init] INIT : HW init settings for customization : VCAMA2\n");
	upmu_ldo_ocfb_enable(LDO_VCAMA2, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VCAMA2, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VCAMA2, KAL_TRUE);
	upmu_ldo_vol_sel(LDO_VCAMA2, UPMU_VOLT_2_8_0_0_V);
	upmu_ldo_stb_td(LDO_VCAMA2, LDO_STB_TD_200_US);

	printk("****[mt6573_pmu_init] INIT : HW init settings for customization : VCAMD2\n");
	upmu_ldo_ocfb_enable(LDO_VCAMD2, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VCAMD2, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VCAMD2, KAL_TRUE);
	upmu_ldo_vol_sel(LDO_VCAMD2, UPMU_VOLT_1_8_0_0_V);
	upmu_ldo_stb_td(LDO_VCAMD2, LDO_STB_TD_200_US);

	printk("****[mt6573_pmu_init] INIT : HW init settings for customization : VM12\n");
	upmu_ldo_ocfb_enable(LDO_VM12, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VM12, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VM12, KAL_TRUE);
	upmu_ldo_stb_td(LDO_VM12, LDO_STB_TD_200_US);

	printk("****[mt6573_pmu_init] INIT : HW init settings for customization : KPLED\n");
	upmu_kpled_sel(KPLED, 0x7);
	
	printk("****[mt6573_pmu_init] INIT : HW init settings for customization : P_CHARGER\n");
	upmu_vcdt_hv_vth(CHR, PMIC_ADPT_VOLT_07_000000_V);
	upmu_vcdt_hv_enable(CHR, KAL_TRUE);
	upmu_vbat_ov_vth(CHR, PMIC_ADPT_VOLT_04_350000_V);
	upmu_baton_ht_enable(CHR, KAL_FALSE);
	//upmu_otg_bvalid_det_enable(CHR, KAL_FALSE); //only charger exist can do
	SETREG16(0xF702FE84,0x0004);
	CLRREG16(0xF702F700,0x0002); // VRF_ON_SEL=0
	
}

#define WAKE_LOCK_INITIALIZED            (1U << 8)

void pmu6573_chrdet_eint_irq(void)
{		
	unsigned int CHR_CON_9 			= 0xF702FA24;
	unsigned int CHR_CON_10 		= 0xF702FA28;

	mt65xx_eint_mask(EINT_CHR_DET_NUM);

	printk("[pmu6573_chrdet_eint_irq] Get INT!!\r\n");
	
#if defined(CONFIG_POWER_EXT)
#else
	if(battery_suspend_lock.flags & WAKE_LOCK_INITIALIZED)
	{
		wake_lock(&battery_suspend_lock);	
		printk("[pmu6573_chrdet_eint_irq] battery_suspend_lock(%d) inited \r\n", battery_suspend_lock.flags);
	}
	else
	{
		printk("[pmu6573_chrdet_eint_irq] battery_suspend_lock(%d) not init \r\n", battery_suspend_lock.flags);
	}
#endif

	if (CONFIG_CHREDT_INT == 1)
	{
		if(g_charger_in_flag == 0)
		{
			printk("[pmu6573_chrdet_eint_irq] Plug IN!!\r\n");		
			CLRREG16(0xF702FE8C,0x0080);
			SETREG16(0xF702FE8C,0x0100);
			CLRREG16(0xF702FE8C,0x0200);
			SETREG16(0xF702FE8C,0x0400);
			g_charger_in_flag = 1;
		}
		else
		{
			printk("[pmu6573_chrdet_eint_irq] Plug OUT!!\r\n");
			SETREG16(0xF702FE8C,0x0080);
			SETREG16(0xF702FE8C,0x0100);
			CLRREG16(0xF702FE8C,0x0200);
			SETREG16(0xF702FE8C,0x0400);
			g_charger_in_flag = 0;

			g_first_check = 0;
			
			SETREG16(CHR_CON_10,0x0100); // RG_BC11_BB_CTRL=1
			CLRREG16(CHR_CON_10,0x0080); //RG_BC11_BIAS_EN=0
			CLRREG16(CHR_CON_9,0x0003);  //RG_BC11_VSRC_EN[1:0]=00
			printk("CHR_CON_9=0x%x, CHR_CON_10=0x%x\r\n",INREG16(CHR_CON_9), INREG16(CHR_CON_10));
		}
	}
	else if (CONFIG_BATON_UNDET_INT == 1)
	{
	}
	else if (CONFIG_CHREDT_and_BATON_UNDET_INT == 1)
	{
	}
	else 
	{
	}

	printk("[pmu6573_chrdet_eint_irq] REG[0xF702FE8C]=%x \r\n", INREG16(0xF702FE8C));

	mt65xx_eint_unmask(EINT_CHR_DET_NUM);

#if defined(CONFIG_POWER_EXT)
//#if 0
	printk("[pmu6573_chrdet_eint_irq] EVB(POWER_EXT)\r\n");
    mt6573_usb_charger_event_for_evb(g_charger_in_flag);
#else
	wake_up_bat();
#endif

	return;
}

void pmu6573_pmu_eint_setting(void)
{

	if (CONFIG_CHREDT_INT == 1)
	{
		printk("[pmu6573_pmu_eint_setting] CONFIG_CHREDT_INT == 1\r\n");
		if(upmu_is_chr_det(CHR) == KAL_TRUE)
		{
			printk("[pmu6573_pmu_eint_setting] Boot with charger\r\n");					
			CLRREG16(0xF702FE8C,0x0080);	
			g_charger_in_flag = 1;
		}
		else
		{
			printk("[pmu6573_pmu_eint_setting] Boot with no charger\r\n");
			SETREG16(0xF702FE8C,0x0080);
			g_charger_in_flag = 0;
		}
		SETREG16(0xF702FE8C,0x0100);
		CLRREG16(0xF702FE8C,0x0200);
		SETREG16(0xF702FE8C,0x0400);	
	}
	else if (CONFIG_BATON_UNDET_INT == 1)
	{
		printk("[pmu6573_pmu_eint_setting] CONFIG_BATON_UNDET_INT == 1\r\n");
		CLRREG16(0xF702FE8C,0x0080);
		CLRREG16(0xF702FE8C,0x0100);
		SETREG16(0xF702FE8C,0x0200);
		SETREG16(0xF702FE8C,0x0400);	
	}
	else if (CONFIG_CHREDT_and_BATON_UNDET_INT == 1)
	{
		printk("[pmu6573_pmu_eint_setting] CONFIG_CHREDT_and_BATON_UNDET_INT == 1\r\n");
		CLRREG16(0xF702FE8C,0x0080);
		SETREG16(0xF702FE8C,0x0100);
		SETREG16(0xF702FE8C,0x0200);
		SETREG16(0xF702FE8C,0x0400);	
	}
	else 
	{
		printk("[pmu6573_pmu_eint_setting] CONFIG Error\r\n");
	}

	mt65xx_eint_set_sens(EINT_CHR_DET_NUM,EINT_CHR_DET_SEN);
	mt65xx_eint_set_polarity(EINT_CHR_DET_NUM,EINT_CHR_DET_POL);
	mt65xx_eint_set_hw_debounce(EINT_CHR_DET_NUM,EINT_CHR_DET_DEB);
	//mt65xx_eint_registration(EINT_CHR_DET_NUM,
	//						1,
	//						EINT_CHR_DET_POL,
	//						pmu6573_chrdet_eint_irq,
	//						1);
	mt65xx_eint_registration(EINT_CHR_DET_NUM,
							0,
							EINT_CHR_DET_POL,
							pmu6573_chrdet_eint_irq,
							1);
	mt65xx_eint_unmask(EINT_CHR_DET_NUM);

	printk("[pmu6573_pmu_eint_setting] EINT_CHR_DET_SEN=%d\r\n", EINT_CHR_DET_SEN);
	printk("[pmu6573_pmu_eint_setting] EINT_CHR_DET_POL=%d\r\n", EINT_CHR_DET_POL);
	printk("[pmu6573_pmu_eint_setting] EINT_CHR_DET_DEB=%d\r\n", EINT_CHR_DET_DEB);
	printk("[pmu6573_pmu_eint_setting] End\r\n");
	
}

void check_DCT_setting(void)
{
	printk("BB_0   : REG[0xF702F100]=0x%x\r\n", INREG16(0xF702F100));
	printk("BB_0   : REG[0xF702F104]=0x%x\r\n", INREG16(0xF702F104));
	printk("BB_0   : REG[0xF702F108]=0x%x\r\n", INREG16(0xF702F108));
	printk("BB_0   : REG[0xF702FB00]=0x%x\r\n", INREG16(0xF702FB00)); //bit12
	printk("BB_0   : REG[0xF702FB04]=0x%x\r\n", INREG16(0xF702FB04)); 
	printk("BB_0   : REG[0xF702FB08]=0x%x\r\n", INREG16(0xF702FB08)); //bit4~0
	
	printk("VCAMA  : REG[0xF702F730]=0x%x\r\n", INREG16(0xF702F730)); //bit0, bit5~4
	
	printk("VCAMA2 : REG[0xF702F7D0]=0x%x\r\n", INREG16(0xF702F7D0)); //bit0, bit5~4
	
	printk("VCAMD  : REG[0xF702F740]=0x%x\r\n", INREG16(0xF702F740)); //bit0, bit6~4
	printk("VCAMD  : REG[0xF702F744]=0x%x\r\n", INREG16(0xF702F744)); //bit7~4
	
	printk("VCAMD2 : REG[0xF702F7E0]=0x%x\r\n", INREG16(0xF702F7E0)); //bit0, bit6~4
	printk("VCAMD2 : REG[0xF702F7E4]=0x%x\r\n", INREG16(0xF702F7E4)); //bit7~4
	
	printk("VSIM2  : REG[0xF702F790]=0x%x\r\n", INREG16(0xF702F790)); //bit0, bit6~4
	
	printk("VMC    : REG[0xF702F7C0]=0x%x\r\n", INREG16(0xF702F7C0)); //bit0, bit6~4
	
	printk("VIBR   : REG[0xF702F7B0]=0x%x\r\n", INREG16(0xF702F7B0)); //bit0, bit6~4	

	printk("VM12   : REG[0xF702F800]=0x%x\r\n", INREG16(0xF702F800)); //bit0
}

void pmu6573_vaproc_protect(void)
{
	unsigned int ret_val=0;
	unsigned int ret_val_check=0;
	//--------------------------------------------------------------------------------------------------
	ret_val = INREG32(0xF7024104);
	ret_val_check = ret_val;	
	if(ret_val_check & 0x10000000)
	{
		if(ret_val & 0x00080000)
		{
			printk("****[mt6573_pmu_init] INIT : No need VAPROC Protect 1\r\n");
		}
		else
		{
			PMU_DRV_SetData16(0xF702F940, 0x01F0, ((kal_uint16)(0x8) << 4));
			printk("****[mt6573_pmu_init] INIT : Need set VAPROC 1.4V (0x%x)\r\n", INREG16(0xF702F940));	
		}
	}
	else
	{
		printk("****[mt6573_pmu_init] INIT : No need VAPROC Protect 2\r\n");
	}
	printk("****[mt6573_pmu_init] pmu6573_vaproc_protect (%x,%x,%x)\n", 
		ret_val_check, ret_val, INREG16(0xF702F940));	
	//--------------------------------------------------------------------------------------------------
}

void pmu6573_VIO18_trim(void)
{
	unsigned int ret_val=0;
	unsigned int ret_val_check=0;
	//--------------------------------------------------------------------------------------------------
	ret_val = INREG32(0xF7024108);
	ret_val_check = ret_val;	
	if(ret_val_check & 0x0020)
	{
		ret_val = ret_val & 0x001F;
		PMU_DRV_SetData16(0xF702F920, 0x01F0, ((kal_uint16)(ret_val) << 4));
		printk("****[mt6573_pmu_init] INIT : Need trim VIO18\r\n");			
	}
	else
	{
		printk("****[mt6573_pmu_init] INIT : No need trim VIO18\r\n");
	}
	printk("****[mt6573_pmu_init] pmu6573_VIO18_trim (%d,%d,0x%x,VIO18_CON0=0x%x)\n", 
		ret_val_check, ret_val, INREG32(0xF7024108), INREG16(0xF702F920) );	
	//--------------------------------------------------------------------------------------------------
}

void pmu6573_init(void)
{
	upmu_usbdl_enable(LDO_VRTC, KAL_FALSE);

	if(get_chip_eco_ver()==CHIP_E1)	
	{
	/* Driver init settings for all platforms */	
	pmu6573_hw_init();	
		pmu6573_dump_register();
	
		/* PMU Customization : Different may have different values */
		pmu6573_pmu_customization();
		pmu6573_dump_register_2();
	}
	else
	{
		/* Driver init settings for all platforms */	
		pmu6573_new_hw_init();	
		pmu6573_vaproc_protect();
		pmu6573_VIO18_trim();
		pmu6573_dump_register();

		/* PMU Customization : Different may have different values */
		pmu6573_new_pmu_customization();
		pmu6573_dump_register_2();
	}

	//Disable the power good detection
	PMU_DRV_SetData16( (0xF702F204),(0x0800),(1<<11));
	printk("REG[0xF702F204]=%x\r\n", INREG16(0xF702F204));
	
//#if defined(CONFIG_POWER_EXT)	
	/* TEMP SOLUTION : REMOVE ME ! */
	//pmu6573_turn_on_all_LDO();
//#endif	

	/* PMU EINT */
	pmu6573_pmu_eint_setting();

#if defined(CONFIG_POWER_EXT)
	printk("****[mt6573_pmu_init] INIT : EVB (POWER_EXT)\n");
#else
	// 2.3_temp
	/* Disable DRVBUS */
	//mt_set_gpio_mode(GPIO2,GPIO_MODE_05);
	//mt_set_gpio_dir(GPIO2,GPIO_DIR_OUT);
	//mt_set_gpio_out(GPIO2,GPIO_OUT_ZERO);
	//printk("****[mt6573_pmu_init] INIT : FIX ME! Disable DRVBUS for avoiding the HW issue \n");

	printk("****[mt6573_pmu_init] INIT : DONE (POWER_EXT) \n");

	//pmic_init();
	//printk("****[mt6573_pmu_init] DCT INIT : DONE (POWER_EXT) \n");	
	check_DCT_setting();
	printk("****[mt6573_pmu_init] Check DCT setting : DONE (POWER_EXT) \n");	
#endif

}

///////////////////////////////////////////////////////////////////////////////////////////
//// APIs For DCT  
///////////////////////////////////////////////////////////////////////////////////////////
void dct_pmic_VPA_enable(kal_bool enable) //BUCK BUST 0
{
	printk("****[dct_pmic_VPA_enable] value=%d \n", enable);

	if(get_chip_eco_ver()==CHIP_E1)
	{
		upmu_buck_boost_enable(BUCK_BOOST, enable);
	}
	else
	{
		upmu_lposc_digtal_circuit_enable(LPOCS, enable);
		upmu_buck_boost_enable(BUCK_BOOST, enable);		
	}
}
void dct_pmic_VPA_sel(kal_uint32 volt)
{
	printk("****[dct_pmic_VPA_sel] value=%d \n", volt);

	//upmu_buck_boost_voltage_tune_0();

	if(volt == UPMU_VOLT_0_9_0_0_V){
		upmu_buck_boost_voltage_tune_0(BUCK_BOOST, UPMU_VOLT_0_9_0_0_V);
	}
	else if(volt == UPMU_VOLT_1_0_0_0_V){
		upmu_buck_boost_voltage_tune_0(BUCK_BOOST, UPMU_VOLT_1_0_0_0_V);
	}
	else if(volt == UPMU_VOLT_1_1_0_0_V){
		upmu_buck_boost_voltage_tune_0(BUCK_BOOST, UPMU_VOLT_1_1_0_0_V);
	}
	else if(volt == UPMU_VOLT_1_2_0_0_V){
		upmu_buck_boost_voltage_tune_0(BUCK_BOOST, UPMU_VOLT_1_2_0_0_V);
	}
	else if(volt == UPMU_VOLT_1_3_0_0_V){
		upmu_buck_boost_voltage_tune_0(BUCK_BOOST, UPMU_VOLT_1_3_0_0_V);
	}
	else if(volt == UPMU_VOLT_1_4_0_0_V){
		upmu_buck_boost_voltage_tune_0(BUCK_BOOST, UPMU_VOLT_1_4_0_0_V);
	}
	else if(volt == UPMU_VOLT_1_5_0_0_V){
		upmu_buck_boost_voltage_tune_0(BUCK_BOOST, UPMU_VOLT_1_5_0_0_V);
	}
	else if(volt == UPMU_VOLT_1_6_0_0_V){
		upmu_buck_boost_voltage_tune_0(BUCK_BOOST, UPMU_VOLT_1_6_0_0_V);
	}
	else if(volt == UPMU_VOLT_1_7_0_0_V){
		upmu_buck_boost_voltage_tune_0(BUCK_BOOST, UPMU_VOLT_1_7_0_0_V);
	}
	else if(volt == UPMU_VOLT_1_8_0_0_V){
		upmu_buck_boost_voltage_tune_0(BUCK_BOOST, UPMU_VOLT_1_8_0_0_V);
	}
	else if(volt == UPMU_VOLT_1_9_0_0_V){
		upmu_buck_boost_voltage_tune_0(BUCK_BOOST, UPMU_VOLT_1_9_0_0_V);
	}
	else if(volt == UPMU_VOLT_2_0_0_0_V){
		upmu_buck_boost_voltage_tune_0(BUCK_BOOST, UPMU_VOLT_2_0_0_0_V);
	}
	else if(volt == UPMU_VOLT_2_1_0_0_V){
		upmu_buck_boost_voltage_tune_0(BUCK_BOOST, UPMU_VOLT_2_1_0_0_V);
	}
	else if(volt == UPMU_VOLT_2_2_0_0_V){
		upmu_buck_boost_voltage_tune_0(BUCK_BOOST, UPMU_VOLT_2_2_0_0_V);
	}
	else if(volt == UPMU_VOLT_2_3_0_0_V){
		upmu_buck_boost_voltage_tune_0(BUCK_BOOST, UPMU_VOLT_2_3_0_0_V);
	}
	else if(volt == UPMU_VOLT_2_4_0_0_V){
		upmu_buck_boost_voltage_tune_0(BUCK_BOOST, UPMU_VOLT_2_4_0_0_V);
	}
	else if(volt == UPMU_VOLT_2_5_0_0_V){
		upmu_buck_boost_voltage_tune_0(BUCK_BOOST, UPMU_VOLT_2_5_0_0_V);
	}
	else if(volt == UPMU_VOLT_2_6_0_0_V){
		upmu_buck_boost_voltage_tune_0(BUCK_BOOST, UPMU_VOLT_2_6_0_0_V);
	}
	else if(volt == UPMU_VOLT_2_7_0_0_V){
		upmu_buck_boost_voltage_tune_0(BUCK_BOOST, UPMU_VOLT_2_7_0_0_V);
	}
	else if(volt == UPMU_VOLT_2_8_0_0_V){
		upmu_buck_boost_voltage_tune_0(BUCK_BOOST, UPMU_VOLT_2_8_0_0_V);
	}
	else if(volt == UPMU_VOLT_2_9_0_0_V){
		upmu_buck_boost_voltage_tune_0(BUCK_BOOST, UPMU_VOLT_2_9_0_0_V);
	}
	else if(volt == UPMU_VOLT_3_0_0_0_V){
		upmu_buck_boost_voltage_tune_0(BUCK_BOOST, UPMU_VOLT_3_0_0_0_V);
	}
	else if(volt == UPMU_VOLT_3_1_0_0_V){
		upmu_buck_boost_voltage_tune_0(BUCK_BOOST, UPMU_VOLT_3_1_0_0_V);
	}
	else if(volt == UPMU_VOLT_3_2_0_0_V){
		upmu_buck_boost_voltage_tune_0(BUCK_BOOST, UPMU_VOLT_3_2_0_0_V);
	}
	else if(volt == UPMU_VOLT_3_3_0_0_V){
		upmu_buck_boost_voltage_tune_0(BUCK_BOOST, UPMU_VOLT_3_3_0_0_V);
	}
	else if(volt == UPMU_VOLT_3_4_0_0_V){
		upmu_buck_boost_voltage_tune_0(BUCK_BOOST, UPMU_VOLT_3_4_0_0_V);
	}
	else{
		printk("Error Setting %d. DO nothing.\r\n", volt);
	}
}
void dct_pmic_VCAMA_enable(kal_bool enable)
{
	printk("****[dct_pmic_VCAMA_enable] value=%d \n", enable);

	upmu_ldo_enable(LDO_VCAMA, enable);
}
void dct_pmic_VCAMA_sel(kal_uint32 volt)
{
	printk("****[dct_pmic_VCAMA_sel] value=%d \n", volt);

	if(volt == UPMU_VOLT_1_5_0_0_V){
		upmu_ldo_vol_sel(LDO_VCAMA, UPMU_VOLT_1_5_0_0_V);
	}
	else if(volt == UPMU_VOLT_1_8_0_0_V){
		upmu_ldo_vol_sel(LDO_VCAMA, UPMU_VOLT_1_8_0_0_V);
	}
	else if(volt == UPMU_VOLT_2_5_0_0_V){
		upmu_ldo_vol_sel(LDO_VCAMA, UPMU_VOLT_2_5_0_0_V);
	}
	else if(volt == UPMU_VOLT_2_8_0_0_V){
		upmu_ldo_vol_sel(LDO_VCAMA, UPMU_VOLT_2_8_0_0_V);   
	}
	else{
		printk("Error Setting %d. DO nothing.\r\n", volt);
	}
}
void dct_pmic_VCAMA2_enable(kal_bool enable)
{
	printk("****[dct_pmic_VCAMA2_enable] value=%d \n", enable);

	upmu_ldo_enable(LDO_VCAMA2, enable);
}
void dct_pmic_VCAMA2_sel(kal_uint32 volt)
{
	printk("****[dct_pmic_VCAMA2_sel] value=%d \n", volt);

	if(volt == UPMU_VOLT_1_5_0_0_V){
		upmu_ldo_vol_sel(LDO_VCAMA2, UPMU_VOLT_1_5_0_0_V);
	}
	else if(volt == UPMU_VOLT_1_8_0_0_V){
		upmu_ldo_vol_sel(LDO_VCAMA2, UPMU_VOLT_1_8_0_0_V);
	}
	else if(volt == UPMU_VOLT_2_5_0_0_V){
		upmu_ldo_vol_sel(LDO_VCAMA2, UPMU_VOLT_2_5_0_0_V);
	}
	else if(volt == UPMU_VOLT_2_8_0_0_V){
		upmu_ldo_vol_sel(LDO_VCAMA2, UPMU_VOLT_2_8_0_0_V);
	}
	else{
		printk("Error Setting %d. DO nothing.\r\n", volt);
	}
}
void dct_pmic_VM12_INT_enable(kal_bool enable)
{
	printk("****[dct_pmic_VM12_INT_enable] value=%d \n", enable);

	upmu_ldo_enable(LDO_VM12_INT, enable);
}
void dct_pmic_VM12_INT_sel(kal_uint32 volt)
{
	printk("****[dct_pmic_VM12_INT_sel] value=%d \n", volt);

	printk("****[dct_pmic_VM12_INT_sel] Can not set voltage\n");
}
void dct_pmic_VM12_enable(kal_bool enable)
{
	printk("****[dct_pmic_VM12_enable] value=%d \n", enable);

	upmu_ldo_enable(LDO_VM12, enable);
}
void dct_pmic_VM12_sel(kal_uint32 volt)
{
	printk("****[dct_pmic_INT_sel] value=%d \n", volt);

	printk("****[dct_pmic_INT_sel] Can not set voltage\n");
}
void dct_pmic_VCAMD_enable(kal_bool enable)
{
	printk("****[dct_pmic_VCAMD_enable] value=%d \n", enable);

	upmu_ldo_enable(LDO_VCAMD, enable);
}
void dct_pmic_VCAMD_sel(kal_uint32 volt)
{
	printk("****[dct_pmic_VCAMD_sel] value=%d \n", volt);

	if(volt == UPMU_VOLT_1_2_0_0_V){
		upmu_ldo_cal(LDO_VCAMD, 0x7);
		upmu_ldo_vol_sel(LDO_VCAMD, UPMU_VOLT_1_3_0_0_V);		
	}
	else if(volt == UPMU_VOLT_1_3_0_0_V){
		upmu_ldo_cal(LDO_VCAMD, 0x0);
		upmu_ldo_vol_sel(LDO_VCAMD, UPMU_VOLT_1_3_0_0_V);
	}
	else if(volt == UPMU_VOLT_1_5_0_0_V){
		upmu_ldo_cal(LDO_VCAMD, 0x0);
		upmu_ldo_vol_sel(LDO_VCAMD, UPMU_VOLT_1_5_0_0_V);
	}
	else if(volt == UPMU_VOLT_1_8_0_0_V){
		upmu_ldo_cal(LDO_VCAMD, 0x0);
		upmu_ldo_vol_sel(LDO_VCAMD, UPMU_VOLT_1_8_0_0_V);
	}
	else if(volt == UPMU_VOLT_2_5_0_0_V){
		upmu_ldo_cal(LDO_VCAMD, 0x0);
		upmu_ldo_vol_sel(LDO_VCAMD, UPMU_VOLT_2_5_0_0_V);
	}
	else if(volt == UPMU_VOLT_2_8_0_0_V){
		upmu_ldo_cal(LDO_VCAMD, 0x0);
		upmu_ldo_vol_sel(LDO_VCAMD, UPMU_VOLT_2_8_0_0_V);
	}
	else if(volt == UPMU_VOLT_3_0_0_0_V){
		upmu_ldo_cal(LDO_VCAMD, 0x0);
		upmu_ldo_vol_sel(LDO_VCAMD, UPMU_VOLT_3_0_0_0_V);
	}
	else if(volt == UPMU_VOLT_3_3_0_0_V){
		upmu_ldo_cal(LDO_VCAMD, 0x0);
		upmu_ldo_vol_sel(LDO_VCAMD, UPMU_VOLT_3_3_0_0_V);
	}
	else{
		printk("Error Setting %d. DO nothing.\r\n", volt);
	}
}
void dct_pmic_VCAMD2_enable(kal_bool enable)
{
	printk("****[dct_pmic_VCAMD2_enable] value=%d \n", enable);

	upmu_ldo_enable(LDO_VCAMD2, enable);
}
void dct_pmic_VCAMD2_sel(kal_uint32 volt)
{
	printk("****[dct_pmic_VCAMD2_sel] value=%d \n", volt);

	if(volt == UPMU_VOLT_1_2_0_0_V){
		upmu_ldo_cal(LDO_VCAMD2, 0x7);
		upmu_ldo_vol_sel(LDO_VCAMD2, UPMU_VOLT_1_3_0_0_V);
	}
	else if(volt == UPMU_VOLT_1_3_0_0_V){
		upmu_ldo_cal(LDO_VCAMD2, 0x0);
		upmu_ldo_vol_sel(LDO_VCAMD2, UPMU_VOLT_1_3_0_0_V);
	}
	else if(volt == UPMU_VOLT_1_5_0_0_V){
		upmu_ldo_cal(LDO_VCAMD2, 0x0);	
		upmu_ldo_vol_sel(LDO_VCAMD2, UPMU_VOLT_1_5_0_0_V);
	}
	else if(volt == UPMU_VOLT_1_8_0_0_V){
		upmu_ldo_cal(LDO_VCAMD2, 0x0);
		upmu_ldo_vol_sel(LDO_VCAMD2, UPMU_VOLT_1_8_0_0_V);
	}
	else if(volt == UPMU_VOLT_2_5_0_0_V){
		upmu_ldo_cal(LDO_VCAMD2, 0x0);	
		upmu_ldo_vol_sel(LDO_VCAMD2, UPMU_VOLT_2_5_0_0_V);
	}
	else if(volt == UPMU_VOLT_2_8_0_0_V){
		upmu_ldo_cal(LDO_VCAMD2, 0x0);
		upmu_ldo_vol_sel(LDO_VCAMD2, UPMU_VOLT_2_8_0_0_V);
	}
	else if(volt == UPMU_VOLT_3_0_0_0_V){
		upmu_ldo_cal(LDO_VCAMD2, 0x0);
		upmu_ldo_vol_sel(LDO_VCAMD2, UPMU_VOLT_3_0_0_0_V);
	}
	else if(volt == UPMU_VOLT_3_3_0_0_V){
		upmu_ldo_cal(LDO_VCAMD2, 0x0);
		upmu_ldo_vol_sel(LDO_VCAMD2, UPMU_VOLT_3_3_0_0_V);
	}
	else{
		printk("Error Setting %d. DO nothing.\r\n", volt);
	}
}
void dct_pmic_VSIM2_enable(kal_bool enable)
{
	printk("****[dct_pmic_VSIM2_enable] value=%d \n", enable);

	upmu_ldo_enable(LDO_VSIM2, enable);
}
void dct_pmic_VSIM2_sel(kal_uint32 volt)
{
	printk("****[dct_pmic_VSIM2_sel] value=%d \n", volt);

	if(volt == UPMU_VOLT_1_3_0_0_V){
		upmu_ldo_vol_sel(LDO_VSIM2, UPMU_VOLT_1_3_0_0_V);
	}
	else if(volt == UPMU_VOLT_1_5_0_0_V){
		upmu_ldo_vol_sel(LDO_VSIM2, UPMU_VOLT_1_5_0_0_V);
	}
	else if(volt == UPMU_VOLT_1_8_0_0_V){
		upmu_ldo_vol_sel(LDO_VSIM2, UPMU_VOLT_1_8_0_0_V);
	}
	else if(volt == UPMU_VOLT_2_5_0_0_V){
		upmu_ldo_vol_sel(LDO_VSIM2, UPMU_VOLT_2_5_0_0_V);
	}
	else if(volt == UPMU_VOLT_2_8_0_0_V){
		upmu_ldo_vol_sel(LDO_VSIM2, UPMU_VOLT_2_8_0_0_V);
	}
	else if(volt == UPMU_VOLT_3_0_0_0_V){
		upmu_ldo_vol_sel(LDO_VSIM2, UPMU_VOLT_3_0_0_0_V);
	}
	else if(volt == UPMU_VOLT_3_3_0_0_V){
		upmu_ldo_vol_sel(LDO_VSIM2, UPMU_VOLT_3_3_0_0_V);
	}
	else{
		printk("Error Setting %d. DO nothing.\r\n", volt);
	}
}
void dct_pmic_VMC_enable(kal_bool enable)
{
	printk("****[dct_pmic_VMC_enable] value=%d \n", enable);

	upmu_ldo_enable(LDO_VMC, enable);
}
void dct_pmic_VMC_sel(kal_uint32 volt)
{
	printk("****[dct_pmic_VMC_sel] value=%d \n", volt);

	if(volt == UPMU_VOLT_1_3_0_0_V){
		upmu_ldo_vol_sel(LDO_VMC, UPMU_VOLT_1_3_0_0_V);
	}
	else if(volt == UPMU_VOLT_1_5_0_0_V){
		upmu_ldo_vol_sel(LDO_VMC, UPMU_VOLT_1_5_0_0_V);
	}
	else if(volt == UPMU_VOLT_1_8_0_0_V){
		upmu_ldo_vol_sel(LDO_VMC, UPMU_VOLT_1_8_0_0_V);
	}
	else if(volt == UPMU_VOLT_2_5_0_0_V){
		upmu_ldo_vol_sel(LDO_VMC, UPMU_VOLT_2_5_0_0_V);
	}
	else if(volt == UPMU_VOLT_2_8_0_0_V){
		upmu_ldo_vol_sel(LDO_VMC, UPMU_VOLT_2_8_0_0_V);
	}
	else if(volt == UPMU_VOLT_3_0_0_0_V){
		upmu_ldo_vol_sel(LDO_VMC, UPMU_VOLT_3_0_0_0_V);
	}
	else if(volt == UPMU_VOLT_3_3_0_0_V){
		upmu_ldo_vol_sel(LDO_VMC, UPMU_VOLT_3_3_0_0_V);
	}
	else{
		printk("Error Setting %d. DO nothing.\r\n", volt);
	}
}
void dct_pmic_VIBR_enable(kal_bool enable)
{
	printk("****[dct_pmic_VIBR_enable] value=%d \n", enable);

	upmu_ldo_enable(LDO_VIBR, enable);
}
void dct_pmic_VIBR_sel(kal_uint32 volt)
{
	printk("****[dct_pmic_VIBR_sel] value=%d \n", volt);

	if(volt == UPMU_VOLT_1_3_0_0_V){
		upmu_ldo_vol_sel(LDO_VIBR, UPMU_VOLT_1_3_0_0_V);
	}
	else if(volt == UPMU_VOLT_1_5_0_0_V){
		upmu_ldo_vol_sel(LDO_VIBR, UPMU_VOLT_1_5_0_0_V);
	}
	else if(volt == UPMU_VOLT_1_8_0_0_V){
		upmu_ldo_vol_sel(LDO_VIBR, UPMU_VOLT_1_8_0_0_V);
	}
	else if(volt == UPMU_VOLT_2_5_0_0_V){
		upmu_ldo_vol_sel(LDO_VIBR, UPMU_VOLT_2_5_0_0_V);
	}
	else if(volt == UPMU_VOLT_2_8_0_0_V){
		upmu_ldo_vol_sel(LDO_VIBR, UPMU_VOLT_2_8_0_0_V);
	}
	else if(volt == UPMU_VOLT_3_0_0_0_V){
		upmu_ldo_vol_sel(LDO_VIBR, UPMU_VOLT_3_0_0_0_V);
	}
	else if(volt == UPMU_VOLT_3_3_0_0_V){
		upmu_ldo_vol_sel(LDO_VIBR, UPMU_VOLT_3_3_0_0_V);
	}
	else{
		printk("Error Setting %d. DO nothing.\r\n", volt);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : PMU_Register
///////////////////////////////////////////////////////////////////////////////////////////
unsigned int g_PMU_value=0;
static ssize_t show_PMU_Register(struct device *dev,struct device_attribute *attr, char *buf)
{
	printk("[EM] show_PMU_Register : %x\n", g_PMU_value);
	return sprintf(buf, "%u\n", g_PMU_value);
}
static ssize_t store_PMU_Register(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	char *pvalue = NULL;
	unsigned int reg_value = 0;
	unsigned int reg_address = 0;
	printk("[EM] store_PMU_Register\n");
	if(buf != NULL && size != 0)
	{
		printk("[EM] buf is %s and size is %d \n",buf,size);
		reg_address = simple_strtoul(buf,&pvalue,16);
		
		if(size > 9)
		{		
			reg_value = simple_strtoul((pvalue+1),NULL,16);		
			printk("[EM] write PMU reg %x with value %x \n",reg_address,reg_value);
			OUTREG32(reg_address,reg_value);
		}
		else
		{	
			g_PMU_value = INREG32(reg_address);
			printk("[EM] read PMU reg %x with value %x \n",reg_address,g_PMU_value);
			printk("[EM] Please use \"cat PMU_Register\" to get value\r\n");
		}		
	}		
	return size;
}
static DEVICE_ATTR(PMU_Register, 0664, show_PMU_Register, store_PMU_Register);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : BUCK_VCORE_STATUS
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_BUCK_VCORE_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	unsigned int reg_address = VCORE_CON0;
	ret_value = INREG16(reg_address);
	ret_value = ret_value >> 15;
	printk("[EM] BUCK_VCORE_STATUS : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_BUCK_VCORE_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(BUCK_VCORE_STATUS, 0664, show_BUCK_VCORE_STATUS, store_BUCK_VCORE_STATUS);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : BUCK_VIO1V8_STATUS
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_BUCK_VIO1V8_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	unsigned int reg_address = VIO1V8_CON0;
	ret_value = INREG16(reg_address);
	ret_value = ret_value >> 15;
	printk("[EM] BUCK_VIO1V8_STATUS : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_BUCK_VIO1V8_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(BUCK_VIO1V8_STATUS, 0664, show_BUCK_VIO1V8_STATUS, store_BUCK_VIO1V8_STATUS);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : BUCK_VAPROC_STATUS
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_BUCK_VAPROC_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	unsigned int reg_address = VAPROC_CON0;
	ret_value = INREG16(reg_address);
	ret_value = ret_value >> 15;
	printk("[EM] BUCK_VAPROC_STATUS : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_BUCK_VAPROC_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(BUCK_VAPROC_STATUS, 0664, show_BUCK_VAPROC_STATUS, store_BUCK_VAPROC_STATUS);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : BUCK_VRF1V8_STATUS
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_BUCK_VRF1V8_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	unsigned int reg_address = VRF1V8_CON0;
	ret_value = INREG16(reg_address);
	ret_value = ret_value >> 15;
	printk("[EM] BUCK_VRF1V8_STATUS : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_BUCK_VRF1V8_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(BUCK_VRF1V8_STATUS, 0664, show_BUCK_VRF1V8_STATUS, store_BUCK_VRF1V8_STATUS);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : BUCK_VCORE_VOLTAGE
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_BUCK_VCORE_VOLTAGE(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	int voltage_init=1200;
	int voltage_step=25;
	unsigned int reg_address = VCORE_CON0;
	ret_value = INREG16(reg_address);
	ret_value = ret_value & 0x01F0;
	ret_value = ret_value >> 4;
	ret_value = voltage_init + (ret_value * voltage_step);
	printk("[EM] BUCK_VCORE_VOLTAGE : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_BUCK_VCORE_VOLTAGE(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(BUCK_VCORE_VOLTAGE, 0664, show_BUCK_VCORE_VOLTAGE, store_BUCK_VCORE_VOLTAGE);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : BUCK_VIO1V8_VOLTAGE
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_BUCK_VIO1V8_VOLTAGE(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	int voltage_init=1800;
	int voltage_step=25;
	unsigned int reg_address = VIO1V8_CON0;
	ret_value = INREG16(reg_address);
	ret_value = ret_value & 0x01F0;
	ret_value = ret_value >> 4;
	ret_value = voltage_init + (ret_value * voltage_step);
	printk("[EM] BUCK_VIO1V8_VOLTAGE : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_BUCK_VIO1V8_VOLTAGE(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(BUCK_VIO1V8_VOLTAGE, 0664, show_BUCK_VIO1V8_VOLTAGE, store_BUCK_VIO1V8_VOLTAGE);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : BUCK_VAPROC_VOLTAGE
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_BUCK_VAPROC_VOLTAGE(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	int voltage_init=1200;
	int voltage_step=25;
	unsigned int reg_address = VAPROC_CON0;
	ret_value = INREG16(reg_address);
	ret_value = ret_value & 0x01F0;
	ret_value = ret_value >> 4;
	ret_value = voltage_init + (ret_value * voltage_step);
	printk("[EM] BUCK_VAPROC_VOLTAGE : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_BUCK_VAPROC_VOLTAGE(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(BUCK_VAPROC_VOLTAGE, 0664, show_BUCK_VAPROC_VOLTAGE, store_BUCK_VAPROC_VOLTAGE);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : BUCK_VRF1V8_VOLTAGE
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_BUCK_VRF1V8_VOLTAGE(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	int voltage_init=1200;
	int voltage_step=25;
	unsigned int reg_address = VRF1V8_CON0;
	ret_value = INREG16(reg_address);
	ret_value = ret_value & 0x01F0;
	ret_value = ret_value >> 4;
	ret_value = voltage_init + (ret_value * voltage_step);
	printk("[EM] BUCK_VRF1V8_VOLTAGE : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_BUCK_VRF1V8_VOLTAGE(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(BUCK_VRF1V8_VOLTAGE, 0664, show_BUCK_VRF1V8_VOLTAGE, store_BUCK_VRF1V8_VOLTAGE);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : LDO_VRF_STATUS
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_LDO_VRF_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	unsigned int reg_address = VRF_CON0;
	ret_value = INREG16(reg_address);
	ret_value = ret_value >> 15;
	printk("[EM] LDO_VRF_STATUS : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VRF_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(LDO_VRF_STATUS, 0664, show_LDO_VRF_STATUS, store_LDO_VRF_STATUS);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : LDO_VCAMA_STATUS
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_LDO_VCAMA_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	unsigned int reg_address = VCAMA_CON0;
	ret_value = INREG16(reg_address);
	ret_value = ret_value >> 15;
	printk("[EM] LDO_VCAMA_STATUS : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VCAMA_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(LDO_VCAMA_STATUS, 0664, show_LDO_VCAMA_STATUS, store_LDO_VCAMA_STATUS);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : LDO_VCAMD_STATUS
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_LDO_VCAMD_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	unsigned int reg_address = VCAMD_CON0;
	ret_value = INREG16(reg_address);
	ret_value = ret_value >> 15;
	printk("[EM] LDO_VCAMD_STATUS : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VCAMD_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(LDO_VCAMD_STATUS, 0664, show_LDO_VCAMD_STATUS, store_LDO_VCAMD_STATUS);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : LDO_VIO_STATUS
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_LDO_VIO_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	unsigned int reg_address = VIO_CON0;
	ret_value = INREG16(reg_address);
	ret_value = ret_value >> 15;
	printk("[EM] LDO_VIO_STATUS : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VIO_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(LDO_VIO_STATUS, 0664, show_LDO_VIO_STATUS, store_LDO_VIO_STATUS);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : LDO_VUSB_STATUS
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_LDO_VUSB_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	unsigned int reg_address = VUSB_CON0;
	ret_value = INREG16(reg_address);
	ret_value = ret_value >> 15;
	printk("[EM] LDO_VUSB_STATUS : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VUSB_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(LDO_VUSB_STATUS, 0664, show_LDO_VUSB_STATUS, store_LDO_VUSB_STATUS);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : LDO_VSIM_STATUS
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_LDO_VSIM_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	unsigned int reg_address = VSIM_CON0;
	ret_value = INREG16(reg_address);
	ret_value = ret_value >> 15;
	printk("[EM] LDO_VSIM_STATUS : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VSIM_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(LDO_VSIM_STATUS, 0664, show_LDO_VSIM_STATUS, store_LDO_VSIM_STATUS);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : LDO_VSIM2_STATUS
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_LDO_VSIM2_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	unsigned int reg_address = VSIM2_CON0;
	ret_value = INREG16(reg_address);
	ret_value = ret_value >> 15;
	printk("[EM] LDO_VSIM2_STATUS : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VSIM2_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(LDO_VSIM2_STATUS, 0664, show_LDO_VSIM2_STATUS, store_LDO_VSIM2_STATUS);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : LDO_VIBR_STATUS
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_LDO_VIBR_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	unsigned int reg_address = VIBR_CON0;
	ret_value = INREG16(reg_address);
	ret_value = ret_value >> 15;
	printk("[EM] LDO_VIBR_STATUS : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VIBR_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(LDO_VIBR_STATUS, 0664, show_LDO_VIBR_STATUS, store_LDO_VIBR_STATUS);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : LDO_VMC_STATUS
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_LDO_VMC_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	unsigned int reg_address = VMC_CON0;
	ret_value = INREG16(reg_address);
	ret_value = ret_value >> 15;
	printk("[EM] LDO_VMC_STATUS : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VMC_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(LDO_VMC_STATUS, 0664, show_LDO_VMC_STATUS, store_LDO_VMC_STATUS);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : LDO_VCAMA2_STATUS
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_LDO_VCAMA2_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	unsigned int reg_address = VCAMA2_CON0;
	ret_value = INREG16(reg_address);
	ret_value = ret_value >> 15;
	printk("[EM] LDO_VCAMA2_STATUS : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VCAMA2_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(LDO_VCAMA2_STATUS, 0664, show_LDO_VCAMA2_STATUS, store_LDO_VCAMA2_STATUS);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : LDO_VCAMD2_STATUS
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_LDO_VCAMD2_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	unsigned int reg_address = VCAMD2_CON0;
	ret_value = INREG16(reg_address);
	ret_value = ret_value >> 15;
	printk("[EM] LDO_VCAMD2_STATUS : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VCAMD2_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(LDO_VCAMD2_STATUS, 0664, show_LDO_VCAMD2_STATUS, store_LDO_VCAMD2_STATUS);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : LDO_VM12_STATUS
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_LDO_VM12_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	unsigned int reg_address = VM12_CON0;
	ret_value = INREG16(reg_address);
	ret_value = ret_value >> 15;
	printk("[EM] LDO_VM12_STATUS : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VM12_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(LDO_VM12_STATUS, 0664, show_LDO_VM12_STATUS, store_LDO_VM12_STATUS);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : LDO_VM12_INT_STATUS
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_LDO_VM12_INT_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	unsigned int reg_address = VM12_INT_CON0;
	ret_value = INREG16(reg_address);
	ret_value = ret_value >> 15;
	printk("[EM] LDO_VM12_INT_STATUS : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VM12_INT_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(LDO_VM12_INT_STATUS, 0664, show_LDO_VM12_INT_STATUS, store_LDO_VM12_INT_STATUS);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : LDO_VCAMA_VOLTAGE
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_LDO_VCAMA_VOLTAGE(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	unsigned int reg_address = VCAMA_CON0;
	ret_value = INREG16(reg_address);
	ret_value = ret_value & 0x0030;
	ret_value = ret_value >> 4;
	if(ret_value == 0)
	{
		ret_value=1500;
	}
	else if(ret_value == 1)
	{
		ret_value=1800;
	}
	else if(ret_value == 2)
	{
		ret_value=2500;
	}
	else if(ret_value == 3)
	{
		ret_value=2800;
	}
	else
	{
		ret_value=0;
	}
	
	printk("[EM] LDO_VCAMA_VOLTAGE : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VCAMA_VOLTAGE(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(LDO_VCAMA_VOLTAGE, 0664, show_LDO_VCAMA_VOLTAGE, store_LDO_VCAMA_VOLTAGE);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : LDO_VCAMD_VOLTAGE
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_LDO_VCAMD_VOLTAGE(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	unsigned int reg_address = VCAMD_CON0;
	ret_value = INREG16(reg_address);
	ret_value = ret_value & 0x0070;
	ret_value = ret_value >> 4;
	if(ret_value == 0)
	{
		ret_value=1300;
	}
	else if(ret_value == 1)
	{
		ret_value=1500;
	}
	else if(ret_value == 2)
	{
		ret_value=1800;
	}
	else if(ret_value == 3)
	{
		ret_value=2500;
	}
	else if(ret_value == 4)
	{
		ret_value=2800;
	}
	else if(ret_value == 5)
	{
		ret_value=3000;
	}
	else if(ret_value == 6)
	{
		ret_value=3300;
	}
	else if(ret_value == 7)
	{
		ret_value=1;
	}
	else
	{
		ret_value=0;
	}
	
	printk("[EM] LDO_VCAMD_VOLTAGE : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VCAMD_VOLTAGE(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(LDO_VCAMD_VOLTAGE, 0664, show_LDO_VCAMD_VOLTAGE, store_LDO_VCAMD_VOLTAGE);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : LDO_VSIM_VOLTAGE
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_LDO_VSIM_VOLTAGE(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	unsigned int reg_address = VSIM_CON0;
	ret_value = INREG16(reg_address);
	ret_value = ret_value & 0x0010;
	ret_value = ret_value >> 4;
	if(ret_value == 0)
	{
		ret_value=1800;
	}
	else if(ret_value == 1)
	{
		ret_value=3000;
	}
	else
	{
		ret_value=0;
	}
	
	printk("[EM] LDO_VSIM_VOLTAGE : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VSIM_VOLTAGE(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(LDO_VSIM_VOLTAGE, 0664, show_LDO_VSIM_VOLTAGE, store_LDO_VSIM_VOLTAGE);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : LDO_VSIM2_VOLTAGE
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_LDO_VSIM2_VOLTAGE(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	unsigned int reg_address = VSIM2_CON0;
	ret_value = INREG16(reg_address);
	ret_value = ret_value & 0x0070;
	ret_value = ret_value >> 4;
	if(ret_value == 0)
	{
		ret_value=1300;
	}
	else if(ret_value == 1)
	{
		ret_value=1500;
	}
	else if(ret_value == 2)
	{
		ret_value=1800;
	}
	else if(ret_value == 3)
	{
		ret_value=2500;
	}
	else if(ret_value == 4)
	{
		ret_value=2800;
	}
	else if(ret_value == 5)
	{
		ret_value=3000;
	}
	else if(ret_value == 6)
	{
		ret_value=3300;
	}
	else if(ret_value == 7)
	{
		ret_value=1;
	}
	else
	{
		ret_value=0;
	}
	
	printk("[EM] LDO_VSIM2_VOLTAGE : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VSIM2_VOLTAGE(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(LDO_VSIM2_VOLTAGE, 0664, show_LDO_VSIM2_VOLTAGE, store_LDO_VSIM2_VOLTAGE);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : LDO_VIBR_VOLTAGE
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_LDO_VIBR_VOLTAGE(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	unsigned int reg_address = VIBR_CON0;
	ret_value = INREG16(reg_address);
	ret_value = ret_value & 0x0070;
	ret_value = ret_value >> 4;
	if(ret_value == 0)
	{
		ret_value=1300;
	}
	else if(ret_value == 1)
	{
		ret_value=1500;
	}
	else if(ret_value == 2)
	{
		ret_value=1800;
	}
	else if(ret_value == 3)
	{
		ret_value=2500;
	}
	else if(ret_value == 4)
	{
		ret_value=2800;
	}
	else if(ret_value == 5)
	{
		ret_value=3000;
	}
	else if(ret_value == 6)
	{
		ret_value=3300;
	}
	else if(ret_value == 7)
	{
		ret_value=1;
	}
	else
	{
		ret_value=0;
	}
	
	printk("[EM] LDO_VIBR_VOLTAGE : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VIBR_VOLTAGE(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(LDO_VIBR_VOLTAGE, 0664, show_LDO_VIBR_VOLTAGE, store_LDO_VIBR_VOLTAGE);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : LDO_VMC_VOLTAGE
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_LDO_VMC_VOLTAGE(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	unsigned int reg_address = VMC_CON0;
	ret_value = INREG16(reg_address);
	ret_value = ret_value & 0x0070;
	ret_value = ret_value >> 4;
	if(ret_value == 0)
	{
		ret_value=1300;
	}
	else if(ret_value == 1)
	{
		ret_value=1500;
	}
	else if(ret_value == 2)
	{
		ret_value=1800;
	}
	else if(ret_value == 3)
	{
		ret_value=2500;
	}
	else if(ret_value == 4)
	{
		ret_value=2800;
	}
	else if(ret_value == 5)
	{
		ret_value=3000;
	}
	else if(ret_value == 6)
	{
		ret_value=3300;
	}
	else if(ret_value == 7)
	{
		ret_value=1;
	}
	else
	{
		ret_value=0;
	}
	
	printk("[EM] LDO_VMC_VOLTAGE : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VMC_VOLTAGE(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(LDO_VMC_VOLTAGE, 0664, show_LDO_VMC_VOLTAGE, store_LDO_VMC_VOLTAGE);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : LDO_VCAMA2_VOLTAGE
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_LDO_VCAMA2_VOLTAGE(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	unsigned int reg_address = VCAMA2_CON0;
	ret_value = INREG16(reg_address);
	ret_value = ret_value & 0x0030;
	ret_value = ret_value >> 4;
	if(ret_value == 0)
	{
		ret_value=1500;
	}
	else if(ret_value == 1)
	{
		ret_value=1800;
	}
	else if(ret_value == 2)
	{
		ret_value=2500;
	}
	else if(ret_value == 3)
	{
		ret_value=2800;
	}	
	else
	{
		ret_value=0;
	}
	
	printk("[EM] LDO_VCAMA2_VOLTAGE : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VCAMA2_VOLTAGE(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(LDO_VCAMA2_VOLTAGE, 0664, show_LDO_VCAMA2_VOLTAGE, store_LDO_VCAMA2_VOLTAGE);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : LDO_VCAMD2_VOLTAGE
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_LDO_VCAMD2_VOLTAGE(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	unsigned int reg_address = VCAMD2_CON0;
	ret_value = INREG16(reg_address);
	ret_value = ret_value & 0x0070;
	ret_value = ret_value >> 4;
	if(ret_value == 0)
	{
		ret_value=1300;
	}
	else if(ret_value == 1)
	{
		ret_value=1500;
	}
	else if(ret_value == 2)
	{
		ret_value=1800;
	}
	else if(ret_value == 3)
	{
		ret_value=2500;
	}
	else if(ret_value == 4)
	{
		ret_value=2800;
	}
	else if(ret_value == 5)
	{
		ret_value=3000;
	}
	else if(ret_value == 6)
	{
		ret_value=3300;
	}
	else if(ret_value == 7)
	{
		ret_value=1;
	}
	else
	{
		ret_value=0;
	}
	
	printk("[EM] LDO_VCAMD2_VOLTAGE : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VCAMD2_VOLTAGE(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(LDO_VCAMD2_VOLTAGE, 0664, show_LDO_VCAMD2_VOLTAGE, store_LDO_VCAMD2_VOLTAGE);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : BUCK_BOOST_EN
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_BUCK_BOOST_EN(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	unsigned int reg_address = PMIC_BB_CON0;
	ret_value = INREG16(reg_address);
	ret_value = ret_value & 0x1000;
	ret_value = ret_value >> 12;
	printk("[EM] BUCK_BOOST_EN : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_BUCK_BOOST_EN(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(BUCK_BOOST_EN, 0664, show_BUCK_BOOST_EN, store_BUCK_BOOST_EN);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : BUCK_BOOST_VOLTAGE_0
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_BUCK_BOOST_VOLTAGE_0(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	int voltage_init=900;
	int voltage_step=100;
	unsigned int reg_address = (PMIC_BB_CON0 + 0x08);
	ret_value = INREG16(reg_address);
	ret_value = ret_value & 0x001F;
	//ret_value = ret_value >> 0;
	ret_value = voltage_init + (ret_value * voltage_step);
	printk("[EM] BUCK_BOOST_VOLTAGE_0 : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_BUCK_BOOST_VOLTAGE_0(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(BUCK_BOOST_VOLTAGE_0, 0664, show_BUCK_BOOST_VOLTAGE_0, store_BUCK_BOOST_VOLTAGE_0);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : BUCK_BOOST_VOLTAGE_1
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_BUCK_BOOST_VOLTAGE_1(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	int voltage_init=900;
	int voltage_step=100;
	unsigned int reg_address = (PMIC_BB_CON0 + 0x08);
	ret_value = INREG16(reg_address);
	ret_value = ret_value & 0x1F00;
	ret_value = ret_value >> 8;
	ret_value = voltage_init + (ret_value * voltage_step);
	printk("[EM] BUCK_BOOST_VOLTAGE_1 : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_BUCK_BOOST_VOLTAGE_1(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(BUCK_BOOST_VOLTAGE_1, 0664, show_BUCK_BOOST_VOLTAGE_1, store_BUCK_BOOST_VOLTAGE_1);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : BUCK_BOOST_VOLTAGE_2
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_BUCK_BOOST_VOLTAGE_2(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	int voltage_init=900;
	int voltage_step=100;
	unsigned int reg_address = (PMIC_BB_CON0 + 0x0C);
	ret_value = INREG16(reg_address);
	ret_value = ret_value & 0x001F;
	//ret_value = ret_value >> 0;
	ret_value = voltage_init + (ret_value * voltage_step);
	printk("[EM] BUCK_BOOST_VOLTAGE_2 : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_BUCK_BOOST_VOLTAGE_2(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(BUCK_BOOST_VOLTAGE_2, 0664, show_BUCK_BOOST_VOLTAGE_2, store_BUCK_BOOST_VOLTAGE_2);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : BUCK_BOOST_VOLTAGE_3
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_BUCK_BOOST_VOLTAGE_3(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	int voltage_init=900;
	int voltage_step=100;
	unsigned int reg_address = (PMIC_BB_CON0 + 0x0C);
	ret_value = INREG16(reg_address);
	ret_value = ret_value & 0x1F00;
	ret_value = ret_value >> 8;
	ret_value = voltage_init + (ret_value * voltage_step);
	printk("[EM] BUCK_BOOST_VOLTAGE_3 : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_BUCK_BOOST_VOLTAGE_3(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(BUCK_BOOST_VOLTAGE_3, 0664, show_BUCK_BOOST_VOLTAGE_3, store_BUCK_BOOST_VOLTAGE_3);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : BUCK_BOOST_VOLTAGE_4
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_BUCK_BOOST_VOLTAGE_4(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	int voltage_init=900;
	int voltage_step=100;
	unsigned int reg_address = (PMIC_BB_CON0 + 0x10);
	ret_value = INREG16(reg_address);
	ret_value = ret_value & 0x001F;
	//ret_value = ret_value >> 0;
	ret_value = voltage_init + (ret_value * voltage_step);
	printk("[EM] BUCK_BOOST_VOLTAGE_4 : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_BUCK_BOOST_VOLTAGE_4(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(BUCK_BOOST_VOLTAGE_4, 0664, show_BUCK_BOOST_VOLTAGE_4, store_BUCK_BOOST_VOLTAGE_4);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : BUCK_BOOST_VOLTAGE_5
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_BUCK_BOOST_VOLTAGE_5(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	int voltage_init=900;
	int voltage_step=100;
	unsigned int reg_address = (PMIC_BB_CON0 + 0x10);
	ret_value = INREG16(reg_address);
	ret_value = ret_value & 0x1F00;
	ret_value = ret_value >> 8;
	ret_value = voltage_init + (ret_value * voltage_step);
	printk("[EM] BUCK_BOOST_VOLTAGE_5 : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_BUCK_BOOST_VOLTAGE_5(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(BUCK_BOOST_VOLTAGE_5, 0664, show_BUCK_BOOST_VOLTAGE_5, store_BUCK_BOOST_VOLTAGE_5);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : BUCK_BOOST_VOLTAGE_6
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_BUCK_BOOST_VOLTAGE_6(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	int voltage_init=900;
	int voltage_step=100;
	unsigned int reg_address = (PMIC_BB_CON0 + 0x14);
	ret_value = INREG16(reg_address);
	ret_value = ret_value & 0x001F;
	//ret_value = ret_value >> 0;
	ret_value = voltage_init + (ret_value * voltage_step);
	printk("[EM] BUCK_BOOST_VOLTAGE_6 : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_BUCK_BOOST_VOLTAGE_6(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(BUCK_BOOST_VOLTAGE_6, 0664, show_BUCK_BOOST_VOLTAGE_6, store_BUCK_BOOST_VOLTAGE_6);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : BUCK_BOOST_VOLTAGE_7
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_BUCK_BOOST_VOLTAGE_7(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	int voltage_init=900;
	int voltage_step=100;
	unsigned int reg_address = (PMIC_BB_CON0 + 0x14);
	ret_value = INREG16(reg_address);
	ret_value = ret_value & 0x1F00;
	ret_value = ret_value >> 8;
	ret_value = voltage_init + (ret_value * voltage_step);
	printk("[EM] BUCK_BOOST_VOLTAGE_7 : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_BUCK_BOOST_VOLTAGE_7(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(BUCK_BOOST_VOLTAGE_7, 0664, show_BUCK_BOOST_VOLTAGE_7, store_BUCK_BOOST_VOLTAGE_7);

///////////////////////////////////////////////////////////////////////////////////////////
//// platform_driver API 
///////////////////////////////////////////////////////////////////////////////////////////
static int mt6573_pmu_probe(struct platform_device *dev)	
{
	int ret_device_file = 0;

	printk("******** MT6573 PMU driver probe!! ********\n" );

	pmu6573_init();

	ret_device_file = device_create_file(&(dev->dev), &dev_attr_PMU_Register);

	ret_device_file = device_create_file(&(dev->dev), &dev_attr_BUCK_VCORE_STATUS);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_BUCK_VCORE_VOLTAGE);	
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_BUCK_VIO1V8_STATUS);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_BUCK_VIO1V8_VOLTAGE);	
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_BUCK_VAPROC_STATUS);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_BUCK_VAPROC_VOLTAGE);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_BUCK_VRF1V8_STATUS);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_BUCK_VRF1V8_VOLTAGE);	

	ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VRF_STATUS);
	//ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VRF_VOLTAGE);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VCAMA_STATUS);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VCAMA_VOLTAGE);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VCAMD_STATUS);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VCAMD_VOLTAGE);	
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VIO_STATUS);
	//ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VIO_VOLTAGE);	
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VUSB_STATUS);
	//ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VUSB_VOLTAGE);		
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VSIM_STATUS);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VSIM_VOLTAGE);	
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VSIM2_STATUS);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VSIM2_VOLTAGE);	
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VIBR_STATUS);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VIBR_VOLTAGE);	
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VMC_STATUS);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VMC_VOLTAGE);		
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VCAMA2_STATUS);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VCAMA2_VOLTAGE);	
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VCAMD2_STATUS);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VCAMD2_VOLTAGE);	
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VM12_STATUS);
	//ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VM12_VOLTAGE);	
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VM12_INT_STATUS);
	//ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VM12_INT_VOLTAGE);
	
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_BUCK_BOOST_EN);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_BUCK_BOOST_VOLTAGE_0);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_BUCK_BOOST_VOLTAGE_1);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_BUCK_BOOST_VOLTAGE_2);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_BUCK_BOOST_VOLTAGE_3);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_BUCK_BOOST_VOLTAGE_4);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_BUCK_BOOST_VOLTAGE_5);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_BUCK_BOOST_VOLTAGE_6);	
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_BUCK_BOOST_VOLTAGE_7);

	printk("******** MT6573 PMU : Prepare for EM done!! ********\n" );

	return 0;
}

static int mt6573_pmu_remove(struct platform_device *dev)	
{
	printk("******** MT6573 PMU driver remove!! ********\n" );

	return 0;
}

static void mt6573_pmu_shutdown(struct platform_device *dev)	
{
	printk("******** MT6573 PMU driver shutdown!! ********\n" );

}

static int mt6573_pmu_suspend(struct platform_device *dev, pm_message_t state)	
{
	printk("******** MT6573 PMU driver suspend!! ********\n" );

	return 0;
}

static int mt6573_pmu_resume(struct platform_device *dev)
{
	printk("******** MT6573 PMU driver resume!! ********\n" );

	return 0;
}

struct platform_device MT6573_pmu_device = {
		.name				= "mt6573-pmu",
		.id					= -1,
};

static struct platform_driver mt6573_pmu_driver = {
	.probe		= mt6573_pmu_probe,
	.remove		= mt6573_pmu_remove,
	.shutdown	= mt6573_pmu_shutdown,
	#ifdef CONFIG_PM
	.suspend	= mt6573_pmu_suspend,
	.resume		= mt6573_pmu_resume,
	#endif
	.driver         = {
        .name = "mt6573-pmu",
    },
};

static int __init mt6573_pmu_init(void)
{
	int ret;
	
	ret = platform_device_register(&MT6573_pmu_device);
	if (ret) {
		printk("****[mt6573_pmu_driver] Unable to device register(%d)\n", ret);
		return ret;
	}
	
	ret = platform_driver_register(&mt6573_pmu_driver);
	if (ret) {
		printk("****[mt6573_pmu_driver] Unable to register driver (%d)\n", ret);
		return ret;
	}

	printk("****[mt6573_pmu_driver] Initialization : DONE \n");

	return 0;

}

static void __exit mt6573_pmu_exit (void)
{
}

module_init(mt6573_pmu_init);
module_exit(mt6573_pmu_exit);

MODULE_AUTHOR("James Lo");
MODULE_DESCRIPTION("MT6573 PMU Device Driver");
MODULE_LICENSE("GPL");

//#endif // PMIC_6573_REG_API

