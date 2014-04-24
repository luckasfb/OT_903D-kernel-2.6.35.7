
	 
#include <common.h>
//#include <asm/arch/mt65xx.h>
//#include <asm/arch/mt65xx_typedefs.h>
#include <asm/io.h>

#include <asm/arch/mt6573_gpio.h>
#include <asm/arch/boot_mode.h>

#include "pmu6573_hw.h"
#include "pmu6573_sw.h"
#include "upmu_common_sw.h"

//#if (defined(PMIC_6573_REG_API))

#define DRV_Reg(addr)               (*(volatile kal_uint16 *)(addr))
#define DRV_WriteReg(addr,data)     ((*(volatile kal_uint16 *)(addr)) = (kal_uint16)(data))

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
#define PMU_DRV_ReadReg16(addr)                   DRV_Reg(addr)
#define PMU_DRV_SetData16(addr, bitmask, value)   DRV_SetData(addr, bitmask, value)
#define PMU_DRV_SetData32(addr, bitmask, value)   DRV_SetData32(addr, bitmask, value)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GPIO setting
#define GPIO_BASE (0x70023000)

#define GPIO_DIR_BASE           GPIO_BASE
#define GPIO_PULLEN_BASE        (GPIO_BASE+0x200)
#define GPIO_PULLSEL_BASE       (GPIO_BASE+0x400)
#define GPIO_DINV_BASE          (GPIO_BASE+0x600)
#define GPIO_DOUT_BASE          (GPIO_BASE+0x800)
#define GPIO_DIN_BASE           (GPIO_BASE+0xA00)
#define GPIO_MODE_BASE          (GPIO_BASE+0xC00)

#define GPIO_SET_DIR(_n,dir)    *(volatile unsigned short *)(GPIO_DIR_BASE + (_n>>4)*0x10) =  \
                                    (*(volatile unsigned short *)(GPIO_DIR_BASE + (_n>>4)*0x10) & (~(0x1 <<(_n&0xF)))) | (dir<<(_n&0xF))   


#define GPIO_SET_PULL(_n,en)        *(volatile unsigned short *)(GPIO_PULLEN_BASE + (_n>>4)*0x10) =  \
                                    (*(volatile unsigned short *)(GPIO_PULLEN_BASE + (_n>>4)*0x10) & (~(0x1 <<(_n&0xF)))) | (en<<(_n&0xF))

#define GPIO_SELECT_PULL(_n,sel)    *(volatile unsigned short *)(GPIO_PULLSEL_BASE + (_n>>4)*0x10) =  \
                                    (*(volatile unsigned short *)(GPIO_PULLSEL_BASE + (_n>>4)*0x10) & (~(0x1 <<(_n&0xF)))) | (sel<<(_n&0xF))

#define GPIO_SET_MODE(_n,mode)      (*(volatile unsigned short *)(GPIO_MODE_BASE + (_n/5)*0x10)) =  \
                                    (*(volatile unsigned short *)(GPIO_MODE_BASE + (_n/5)*0x10)) & ((~(0x7 <<(3*(_n%0x5))))) | (mode<<(3*(_n%0x5)))


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Chip specific LDOs list
//const upmu_ldo_profile_entry upmu_ldo_profile[] =
upmu_ldo_profile_entry upmu_ldo_profile[] =
{
/* LDO 1 */
/* VA28 */    
	{	VA28_CON0,     
		1, 
		{				UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* VA25 */    
	{	VA25_CON0,     
		1, 
		{				UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* VA12 */    
	{	VA12_CON0,     
		1, 
		{				UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* VRTC */	  
	{	VRTC_CON0,	   
		1, 
		{				UPMU_VOLT_MAX,		 UPMU_VOLT_MAX, 	  UPMU_VOLT_MAX,	   UPMU_VOLT_MAX,
						UPMU_VOLT_MAX,		 UPMU_VOLT_MAX, 	  UPMU_VOLT_MAX,	   UPMU_VOLT_MAX,
						UPMU_VOLT_MAX,		 UPMU_VOLT_MAX, 	  UPMU_VOLT_MAX,	   UPMU_VOLT_MAX,
						UPMU_VOLT_MAX,		 UPMU_VOLT_MAX, 	  UPMU_VOLT_MAX,	   UPMU_VOLT_MAX
		}
	},


/* VMIC */    
	{	VMIC_CON0,     
		1, 
		{				UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},	

/* VTV */    
	{	VTV_CON0,     
		1, 
		{				UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},		

/* VAUDN */    
	{	VAUDN_CON0,     
		1, 
		{				UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* VAUDP */    
	{	VAUDP_CON0,     
		1, 
		{				UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* PMUA */    
	{	PMUA_CON0,     
		1, 
		{				UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* LDO 2 */	
/* VRF */    
	{	VRF_CON0,     
		1, 
		{				UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* VCAMA */  
	{	VCAMA_CON0,   
		4, 
		{				UPMU_VOLT_1_5_0_0_V, UPMU_VOLT_1_8_0_0_V, UPMU_VOLT_2_5_0_0_V, UPMU_VOLT_2_8_0_0_V,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* VCAMD */ 
	{	VCAMD_CON0,   
		7, 
		{				UPMU_VOLT_1_3_0_0_V, UPMU_VOLT_1_5_0_0_V, UPMU_VOLT_1_8_0_0_V, UPMU_VOLT_2_5_0_0_V,
	                            	UPMU_VOLT_2_8_0_0_V, UPMU_VOLT_3_0_0_0_V, UPMU_VOLT_3_3_0_0_V, UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* VIO */   
	{	VIO_CON0,    
		1, 
		{				UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* VUSB */   
	{	VUSB_CON0,    
		1, 
		{				UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* VSIM */   
	{	VSIM_CON0,    
		2, 
		{				UPMU_VOLT_1_8_0_0_V, UPMU_VOLT_3_0_0_0_V, UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* VSIM2 */  
	{	VSIM2_CON0,   
		7, 
		{				UPMU_VOLT_1_3_0_0_V, UPMU_VOLT_1_5_0_0_V, UPMU_VOLT_1_8_0_0_V, UPMU_VOLT_2_5_0_0_V,
	                            	UPMU_VOLT_2_8_0_0_V, UPMU_VOLT_3_0_0_0_V, UPMU_VOLT_3_3_0_0_V, UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* VIBR */   
	{	VIBR_CON0,    
		7, 
		{				UPMU_VOLT_1_3_0_0_V, UPMU_VOLT_1_5_0_0_V, UPMU_VOLT_1_8_0_0_V, UPMU_VOLT_2_5_0_0_V,
	                            	UPMU_VOLT_2_8_0_0_V, UPMU_VOLT_3_0_0_0_V, UPMU_VOLT_3_3_0_0_V, UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* VMC */    
	{	VMC_CON0,     
		7, 
		{				UPMU_VOLT_1_3_0_0_V, UPMU_VOLT_1_5_0_0_V, UPMU_VOLT_1_8_0_0_V, UPMU_VOLT_2_5_0_0_V,
	                            	UPMU_VOLT_2_8_0_0_V, UPMU_VOLT_3_0_0_0_V, UPMU_VOLT_3_3_0_0_V, UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* VCAMA2 */    
	{	VCAMA2_CON0,     
		4, 
		{				UPMU_VOLT_1_5_0_0_V, UPMU_VOLT_1_8_0_0_V, UPMU_VOLT_2_5_0_0_V, UPMU_VOLT_2_8_0_0_V,
	                            	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},
	
/* VCAMD2 */ 
	{	VCAMD2_CON0,   
		7, 
		{				UPMU_VOLT_1_3_0_0_V, UPMU_VOLT_1_5_0_0_V, UPMU_VOLT_1_8_0_0_V, UPMU_VOLT_2_5_0_0_V,
	                            	UPMU_VOLT_2_8_0_0_V, UPMU_VOLT_3_0_0_0_V, UPMU_VOLT_3_3_0_0_V, UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* VM12 */   
	{	VM12_CON0,    
		1, 
		{				UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
                                	UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX
		}
	},

/* VM12_INT */   
	{	VM12_INT_CON0,    
		1, 
		{				UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,       UPMU_VOLT_MAX,
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
		{			   UPMU_VOLT_1_2_0_0_V, UPMU_VOLT_1_2_2_5_V, UPMU_VOLT_1_2_5_0_V, UPMU_VOLT_1_2_7_5_V,
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
		{			   UPMU_VOLT_1_8_0_0_V, UPMU_VOLT_1_8_2_5_V, UPMU_VOLT_1_8_5_0_V, UPMU_VOLT_1_8_7_5_V,
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
		{			   UPMU_VOLT_1_2_0_0_V, UPMU_VOLT_1_2_2_5_V, UPMU_VOLT_1_2_5_0_V, UPMU_VOLT_1_2_7_5_V,
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
		{			   UPMU_VOLT_1_8_0_0_V, UPMU_VOLT_1_8_2_5_V, UPMU_VOLT_1_8_5_0_V, UPMU_VOLT_1_8_7_5_V,
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
		{			   UPMU_VOLT_0_9_0_0_V, UPMU_VOLT_1_0_0_0_V, UPMU_VOLT_1_1_0_0_V, UPMU_VOLT_1_2_0_0_V,
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
// External Function

CHARGER_TYPE g_ret = CHARGER_UNKNOWN;
int g_charger_in_flag = 1;
int g_first_check=0;

CHARGER_TYPE hw_charger_type_detection(void)
{
	CHARGER_TYPE ret 				= CHARGER_UNKNOWN;
	unsigned int CHR_CON_9 			= 0x7002FA24;
	unsigned int CHR_CON_10 		= 0x7002FA28;
	unsigned int USB_U2PHYACR3_2 	= 0x7005081E;
	unsigned int MEM_ID_USB20		= 0x70026038;
	unsigned int PDN_ID_USB			= 0x70026308;
	unsigned int USBPHYRegs			= 0x70050800; //U2B20_Base+0x800
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
	PMU_DRV_SetData32(PDN_ID_USB,0x80,1); //PDN_ID_USB bit 7
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
	//printf("mt_charger_type_detection : step A : wChargerAvail=%x\r\n", wChargerAvail);
	CLRREG16(CHR_CON_9,0x0003);//RG_BC11_VSRC_EN[1:0]=00
	CLRREG16(CHR_CON_10,0x0004);//RG_BC11_IPD_EN[1.0] = 00
	CLRREG16(CHR_CON_10,0x0003);//RG_BC11_CMP_EN[1.0] = 00
	mdelay(80);

	if(wChargerAvail & 0x0200)
	{
/********* Step B *************************************/
		//printf("mt_charger_type_detection : step B\r\n");

		SETREG16(CHR_CON_10,0x0020); //RG_BC11_IPU_EN[1:0]=10
		mdelay(80);
		
		bLineState_B = INREG8(USBPHYRegs+0x72);
		//printf("mt_charger_type_detection : step B : bLineState_B=%x\r\n", bLineState_B);
		if(bLineState_B & 0x40)
		{
			ret = STANDARD_CHARGER;
			printf("[UBOOT] mt_charger_type_detection : step B : STANDARD CHARGER!\r\n");
		}
		else
		{
			ret = CHARGING_HOST;
			printf("[UBOOT] mt_charger_type_detection : step B : Charging Host!\r\n");
		}
	}
	else
	{
/********* Step C *************************************/
		//printf("mt_charger_type_detection : step C\r\n");

		SETREG16(CHR_CON_10,0x0010); //RG_BC11_IPU_EN[1:0]=01
		SETREG16(CHR_CON_10,0x0001);//RG_BC11_CMP_EN[1.0] = 01
		mdelay(80);
		
		bLineState_C = INREG16(CHR_CON_10);
		//printf("mt_charger_type_detection : step C : bLineState_C=%x\r\n", bLineState_C);
		if(bLineState_C & 0x0200)
		{
			ret = NONSTANDARD_CHARGER;
			printf("[UBOOT] mt_charger_type_detection : step C : UNSTANDARD CHARGER!\r\n");
		}
		else
		{
			ret = STANDARD_HOST;
			printf("[UBOOT] mt_charger_type_detection : step C : Standard USB Host!\r\n");
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
		printf("[mt_charger_type_detection] Got data %d!!\r\n", g_ret);
    }

    return g_ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Internal Function
void pmu6573_dump_register(void)
{
	printf("REG[0x7002FA1C]=%x\r\n", INREG16(0x7002FA1C));
	printf("REG[0x7002FA18]=%x\r\n", INREG16(0x7002FA18));
	printf("REG[0x7002FA24]=%x\r\n", INREG16(0x7002FA24));
	printf("REG[0x7002FA0C]=%x\r\n", INREG16(0x7002FA0C));

	printf("REG[0x7002F900]=%x\r\n", INREG16(0x7002F900));
	printf("REG[0x7002F904]=%x\r\n", INREG16(0x7002F904));
	printf("REG[0x7002F908]=%x\r\n", INREG16(0x7002F908));
	printf("REG[0x7002F90C]=%x\r\n", INREG16(0x7002F90C));
	printf("REG[0x7002F910]=%x\r\n", INREG16(0x7002F910));
	printf("REG[0x7002F914]=%x\r\n\n", INREG16(0x7002F914));
	
	printf("REG[0x7002F920]=%x\r\n", INREG16(0x7002F920));
	printf("REG[0x7002F924]=%x\r\n", INREG16(0x7002F924));
	printf("REG[0x7002F928]=%x\r\n", INREG16(0x7002F928));
	printf("REG[0x7002F92C]=%x\r\n", INREG16(0x7002F92C));
	printf("REG[0x7002F930]=%x\r\n", INREG16(0x7002F930));
	printf("REG[0x7002F934]=%x\r\n\n", INREG16(0x7002F934));

	printf("REG[0x7002F940]=%x\r\n", INREG16(0x7002F940));
	printf("REG[0x7002F944]=%x\r\n", INREG16(0x7002F944));
	printf("REG[0x7002F948]=%x\r\n", INREG16(0x7002F948));
	printf("REG[0x7002F94C]=%x\r\n", INREG16(0x7002F94C));
	printf("REG[0x7002F950]=%x\r\n", INREG16(0x7002F950));
	printf("REG[0x7002F954]=%x\r\n\n", INREG16(0x7002F954));

	printf("REG[0x7002F960]=%x\r\n", INREG16(0x7002F960));
	printf("REG[0x7002F964]=%x\r\n", INREG16(0x7002F964));
	printf("REG[0x7002F968]=%x\r\n", INREG16(0x7002F968));
	printf("REG[0x7002F96C]=%x\r\n", INREG16(0x7002F96C));
	printf("REG[0x7002F970]=%x\r\n", INREG16(0x7002F970));
	printf("REG[0x7002F974]=%x\r\n", INREG16(0x7002F974));	

	printf("REG[0x7002F760]=%x\r\n", INREG16(0x7002F760));	
	printf("REG[0x7002F764]=%x\r\n", INREG16(0x7002F764));

	printf("REG[0x7002F810]=%x\r\n", INREG16(0x7002F810));
	printf("REG[0x7002F814]=%x\r\n", INREG16(0x7002F814));

	printf("REG[0x7002F7C0]=%x\r\n", INREG16(0x7002F7C0));
	printf("REG[0x7002F7C4]=%x\r\n", INREG16(0x7002F7C4));	

	printf("REG[0x7002F700]=%x\r\n", INREG16(0x7002F700));
	printf("REG[0x7002F704]=%x\r\n", INREG16(0x7002F704));

	printf("REG[0x7002FB00]=%x\r\n", INREG16(0x7002FB00));
	printf("REG[0x7002FB04]=%x\r\n", INREG16(0x7002FB04));

	printf("REG[0x7002F100]=%x\r\n", INREG16(0x7002F100));
	printf("REG[0x7002F104]=%x\r\n", INREG16(0x7002F104));
	printf("REG[0x7002F108]=%x\r\n", INREG16(0x7002F108));
	printf("REG[0x7002F114]=%x\r\n", INREG16(0x7002F114));
	printf("REG[0x7002FE88]=%x\r\n", INREG16(0x7002FE88));

	printf("REG[0x70024104]=%x\r\n", INREG16(0x70024104));
	printf("REG[0x7002F210]=%x\r\n", INREG16(0x7002F210));
	printf("REG[0x7002FA24]=%x\r\n", INREG16(0x7002FA24));
	printf("REG[0x7002FA04]=%x\r\n", INREG16(0x7002FA04));

	printf("REG[0x7002FC80]=%x\r\n", INREG16(0x7002FC80));

	printf("REG[0x7002F78C]=%x\r\n", INREG16(0x7002F78C));
	printf("REG[0x7002F79C]=%x\r\n", INREG16(0x7002F79C));
	
}

void pmu6573_dump_register_2(void)
{
	printf("REG[0x7002F730]=%x\r\n", INREG16(0x7002F730));
	printf("REG[0x7002F734]=%x\r\n", INREG16(0x7002F734));

	printf("REG[0x7002F740]=%x\r\n", INREG16(0x7002F740));
	printf("REG[0x7002F744]=%x\r\n", INREG16(0x7002F744));

	printf("REG[0x7002F780]=%x\r\n", INREG16(0x7002F780));
	printf("REG[0x7002F784]=%x\r\n", INREG16(0x7002F784));

	printf("REG[0x7002F790]=%x\r\n", INREG16(0x7002F790));
	printf("REG[0x7002F794]=%x\r\n", INREG16(0x7002F794));

	printf("REG[0x7002F7B0]=%x\r\n", INREG16(0x7002F7B0));
	printf("REG[0x7002F7B4]=%x\r\n", INREG16(0x7002F7B4));

	printf("REG[0x7002F7D0]=%x\r\n", INREG16(0x7002F7D0));
	printf("REG[0x7002F7D4]=%x\r\n", INREG16(0x7002F7D4));	

	printf("REG[0x7002F7E0]=%x\r\n", INREG16(0x7002F7E0));
	printf("REG[0x7002F7E4]=%x\r\n", INREG16(0x7002F7E4));

	printf("REG[0x7002F800]=%x\r\n", INREG16(0x7002F800));
	printf("REG[0x7002F804]=%x\r\n", INREG16(0x7002F804));

	printf("REG[0x7002FC80]=%x\r\n", INREG16(0x7002FC80));	

	printf("REG[0x7002FA00]=%x\r\n", INREG16(0x7002FA00));	
	printf("REG[0x7002FA0C]=%x\r\n", INREG16(0x7002FA0C));

	printf("REG[0x7002FE84]=%x\r\n", INREG16(0x7002FE84));

	printf("REG[0x7002F700]=%x\r\n", INREG16(0x7002F700));
}

void pmu6573_hw_init(void)
{
	printf("****[mt6573_pmu_init] INIT : Depending on the PMU Driver Setting SPEC 0.5.1 \n");

	printf("****[mt6573_pmu_init] INIT : HW init settings for all platforms : charger timer\n");
	upmu_chrwdt_td(CHR, 0x3);
	upmu_chrwdt_int_enable(CHR, KAL_TRUE);
	upmu_chrwdt_enable(CHR, KAL_TRUE);

	printf("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VCORE\n");
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

	printf("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VAPROC\n");
	upmu_buck_disable_anti_undershoot(BUCK_VAPROC, KAL_TRUE); // V0.3
	//upmu_buck_rs(BUCK_VAPROC,BUCK_REMOTE_SENSE);// V0.3.2
	upmu_buck_rs(BUCK_VAPROC,BUCK_LOCAL_SENSE);// V0.4
	upmu_buck_normal_voltage_adjust(BUCK_VAPROC, UPMU_VOLT_1_3_7_5_V); // V0.5.1
	upmu_buck_burst(BUCK_VAPROC, 0x0);
	upmu_buck_csl(BUCK_VAPROC, 0x3);
	upmu_buck_csr(BUCK_VAPROC, 0x5); //0.3
	upmu_buck_oc_td(BUCK_VAPROC, BUCK_OC_TD_100_US);

	printf("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VIO1V8\n");
	upmu_buck_disable_anti_undershoot(BUCK_VIO1V8, KAL_TRUE); // V0.3
	upmu_buck_normal_voltage_adjust(BUCK_VIO1V8, UPMU_VOLT_1_8_0_0_V);
	upmu_buck_burst(BUCK_VIO1V8, 0x0);
	upmu_buck_csl(BUCK_VIO1V8, 0x3);
	upmu_buck_csr(BUCK_VIO1V8, 0x5); //0.3
	upmu_buck_oc_td(BUCK_VIO1V8, BUCK_OC_TD_100_US);

	printf("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VRF1V8\n");
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

	printf("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VUSB\n");
	upmu_ldo_ocfb_enable(LDO_VUSB, KAL_TRUE);
	upmu_ldo_oc_auto_off(LDO_VUSB, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VUSB, KAL_TRUE);
	upmu_ldo_cal(LDO_VUSB, 0x0);
	upmu_ldo_stb_td(LDO_VUSB, LDO_STB_TD_200_US);

	printf("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VM12_INT\n");
	upmu_ldo_ocfb_enable(LDO_VM12_INT, KAL_TRUE);
	upmu_ldo_oc_auto_off(LDO_VM12_INT, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VM12_INT, KAL_TRUE);
	upmu_ldo_oc_td(LDO_VM12_INT, LDO_OC_TD_100_US);

	printf("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VMC\n");
	upmu_ldo_ocfb_enable(LDO_VMC, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VMC, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VMC, KAL_TRUE);
	//upmu_ldo_vol_sel(LDO_VMC, UPMU_VOLT_3_0_0_0_V);
	upmu_ldo_vol_sel(LDO_VMC, UPMU_VOLT_3_3_0_0_V); //v3.3 for SMT
	upmu_ldo_cal(LDO_VMC, 0x0);
	upmu_ldo_stb_td(LDO_VMC, LDO_STB_TD_200_US);

	printf("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VRF\n");
	upmu_ldo_ocfb_enable(LDO_VRF, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VRF, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VRF, KAL_TRUE);
	upmu_ldo_cal(LDO_VRF, 0x0);
	upmu_ldo_stb_td(LDO_VRF, LDO_STB_TD_200_US);

	printf("****[mt6573_pmu_init] INIT : HW init settings for all platforms : KPLED\n");
	upmu_kpled_sfstrt_en(KPLED, KAL_TRUE);
	upmu_kpled_sfstrt_c(KPLED, KPLED_SFSTRT_C_31US_X_1);
	
}

void pmu6573_pmu_customization(void)
{
	printf("****[mt6573_pmu_init] INIT : HW init settings for customization : VCAMA\n");
	upmu_ldo_ocfb_enable(LDO_VCAMA, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VCAMA, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VCAMA, KAL_TRUE);
	upmu_ldo_vol_sel(LDO_VCAMA, UPMU_VOLT_2_8_0_0_V);
	//upmu_ldo_cal(LDO_VCAMA, 0x0);
	upmu_ldo_stb_td(LDO_VCAMA, LDO_STB_TD_200_US);
	
	printf("****[mt6573_pmu_init] INIT : HW init settings for customization : VCAMD\n");
	upmu_ldo_ocfb_enable(LDO_VCAMD, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VCAMD, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VCAMD, KAL_TRUE);
	upmu_ldo_vol_sel(LDO_VCAMD, UPMU_VOLT_1_8_0_0_V);
	//upmu_ldo_cal(LDO_VCAMD, 0x0);
	upmu_ldo_stb_td(LDO_VCAMD, LDO_STB_TD_200_US);
	
	printf("****[mt6573_pmu_init] INIT : HW init settings for customization : VSIM\n");
	upmu_ldo_ocfb_enable(LDO_VSIM, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VSIM, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VSIM, KAL_TRUE);
	//upmu_ldo_cal(LDO_VSIM, 0x0);
	upmu_ldo_stb_td(LDO_VSIM, LDO_STB_TD_200_US);

	printf("****[mt6573_pmu_init] INIT : HW init settings for customization : VSIM2\n");
	upmu_ldo_ocfb_enable(LDO_VSIM2, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VSIM2, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VSIM2, KAL_TRUE);
	//upmu_ldo_cal(LDO_VSIM2, 0x0);
	upmu_ldo_stb_td(LDO_VSIM2, LDO_STB_TD_200_US);

	printf("****[mt6573_pmu_init] INIT : HW init settings for customization : VIBR\n");
	upmu_ldo_ocfb_enable(LDO_VIBR, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VIBR, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VIBR, KAL_TRUE);
	upmu_ldo_vol_sel(LDO_VIBR, UPMU_VOLT_1_3_0_0_V);
	//upmu_ldo_cal(LDO_VIBR, 0x0);
	upmu_ldo_stb_td(LDO_VIBR, LDO_STB_TD_200_US);
		
	printf("****[mt6573_pmu_init] INIT : HW init settings for customization : VCAMA2\n");
	upmu_ldo_ocfb_enable(LDO_VCAMA2, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VCAMA2, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VCAMA2, KAL_TRUE);
	upmu_ldo_vol_sel(LDO_VCAMA2, UPMU_VOLT_2_8_0_0_V);
	//upmu_ldo_cal(LDO_VCAMA2, 0x0);
	upmu_ldo_stb_td(LDO_VCAMA2, LDO_STB_TD_200_US);
	
	printf("****[mt6573_pmu_init] INIT : HW init settings for customization : VCAMD2\n");
	upmu_ldo_ocfb_enable(LDO_VCAMD2, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VCAMD2, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VCAMD2, KAL_TRUE);
	upmu_ldo_vol_sel(LDO_VCAMD2, UPMU_VOLT_1_8_0_0_V);
	//upmu_ldo_cal(LDO_VCAMD2, 0x0);
	upmu_ldo_stb_td(LDO_VCAMD2, LDO_STB_TD_200_US);
	
	printf("****[mt6573_pmu_init] INIT : HW init settings for customization : VM12\n");
	upmu_ldo_ocfb_enable(LDO_VM12, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VM12, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VM12, KAL_TRUE);
	//upmu_ldo_cal(LDO_VM12, 0x0);
	upmu_ldo_stb_td(LDO_VM12, LDO_STB_TD_200_US);
	
	printf("****[mt6573_pmu_init] INIT : HW init settings for customization : KPLED\n");
	upmu_kpled_sel(KPLED, 0x7);
	
}

void pmu6573_new_hw_init(void)
{
	unsigned int ret_val=0;
	unsigned int ret_val_check=0;
	
	printf("****[mt6573_pmu_init] INIT : Depending on the PMU Driver Setting SPEC 0.8.3 +Ray \n");

	printf("****[mt6573_pmu_init] INIT : HW init settings for all platforms : charger timer\n");
	upmu_chrwdt_int_enable(CHR, KAL_TRUE);
	upmu_chrwdt_td(CHR, 0x3);
	upmu_bc11_reset_circuit(CHR, 1);
	upmu_csdac_dly(CHR,0x3);
	upmu_csdac_stp(CHR,0x0);

	printf("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VCORE\n");
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

	printf("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VAPROC\n");	
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

	printf("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VIO1V8\n");
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

	printf("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VRF1V8\n");	
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

	printf("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VUSB\n");
	upmu_ldo_ocfb_enable(LDO_VUSB, KAL_TRUE);	
	upmu_ldo_oc_auto_off(LDO_VUSB, KAL_FALSE);	
	upmu_ldo_ndis_enable(LDO_VUSB, KAL_TRUE);
	upmu_ldo_stb_td(LDO_VUSB, LDO_STB_TD_200_US);

	printf("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VM12_INT\n");
	upmu_ldo_ocfb_enable(LDO_VM12_INT, KAL_TRUE);	
	upmu_ldo_oc_auto_off(LDO_VM12_INT, KAL_FALSE);	
	upmu_ldo_ndis_enable(LDO_VM12_INT, KAL_TRUE);	
	upmu_ldo_oc_td(LDO_VM12_INT, LDO_OC_TD_100_US);
	
	printf("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VMC\n");
	upmu_ldo_ocfb_enable(LDO_VMC, KAL_TRUE);	
	upmu_ldo_oc_auto_off(LDO_VMC, KAL_FALSE);	
	upmu_ldo_ndis_enable(LDO_VMC, KAL_TRUE);	
	upmu_ldo_vol_sel(LDO_VMC, UPMU_VOLT_3_3_0_0_V);	
	upmu_ldo_stb_td(LDO_VMC, LDO_STB_TD_200_US);

	printf("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VRF\n");
	upmu_ldo_ocfb_enable(LDO_VRF, KAL_FALSE);	
	upmu_ldo_oc_auto_off(LDO_VRF, KAL_FALSE);	
	upmu_ldo_ndis_enable(LDO_VRF, KAL_TRUE);	
	upmu_ldo_stb_td(LDO_VRF, LDO_STB_TD_200_US);

#if 1
	printf("****[mt6573_pmu_init] INIT : HW init settings for all platforms : VPA (BB)\n");
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
	printf("****[mt6573_pmu_init] INIT : HW init settings for all platforms : LPOSC+0\n");	
	upmu_lposc_i_bias_cali(LPOCS, CALI_0_9_X);
	upmu_lposc_buck_boost_freq_divider(LPOCS, DIVIDER_RATIO_4);
	upmu_lposc_buck_output_freq_switching(LPOCS, FREQ_1_60_MHz);
	upmu_lposc_fd_resolution_adjust(LPOCS, PERCENT_1_0);
#if 1	
	upmu_lposc_init_dac_enable(LPOCS, KAL_FALSE);
	upmu_lpocs_sw_mode(KAL_TRUE);
	upmu_lposc_buck_boost_enable(LPOCS, KAL_TRUE);	
#endif

	ret_val = INREG16(0x70024104);
	ret_val_check = ret_val;
	printf("****[mt6573_pmu_init] INIT : HW init settings for all platforms : TRIM STRUP (%x)\n", ret_val);	
	if(ret_val_check & 0x8000)
	{
		ret_val = ret_val & 0x7800;
		ret_val = (ret_val >> 11);
		PMU_DRV_SetData16(0x7002F210, 0x00F0, ((kal_uint16)(ret_val) << 4));
	}
	else
	{
		PMU_DRV_SetData16(0x7002F210, 0x00F0, ((kal_uint16)(0x0) << 4));
	}
	
	ret_val = INREG16(0x70024104);
	ret_val_check = ret_val;
	printf("****[mt6573_pmu_init] INIT : HW init settings for all platforms : TRIM OVP (%x)\n", ret_val);	
	if(ret_val_check & 0x0400)
	{
		ret_val = ret_val & 0x03C0;
		ret_val = (ret_val >> 6);
		PMU_DRV_SetData16(0x7002FA24, 0x00F0, ((kal_uint16)(ret_val) << 4));
	}
	else
	{
		PMU_DRV_SetData16(0x7002FA24, 0x00F0, ((kal_uint16)(0x8) << 4));
	}

	ret_val = INREG16(0x70024104);
	ret_val_check = ret_val;
	printf("****[mt6573_pmu_init] INIT : HW init settings for all platforms : TRIM CV_VTH (%x)\n", ret_val);	
	if(ret_val_check & 0x0020)
	{
		ret_val = ret_val & 0x001F;
		PMU_DRV_SetData16(0x7002FA04, 0x001F, ((kal_uint16)(ret_val)));
	}
	else
	{
		PMU_DRV_SetData16(0x7002FA04, 0x001F, ((kal_uint16)(0x0)));
	}		
	//--------------------------------------------------------------------------------------------------
	ret_val = INREG32(0x70024104);
	ret_val_check = ret_val;	
	if(ret_val_check & 0x00200000)
	{
		ret_val = ret_val & 0x001F0000;
		ret_val = (ret_val >> 16);
		ret_val = (ret_val << 4);
		PMU_DRV_SetData16(0x7002F940, 0x01F0, ((kal_uint16)(ret_val)));
		printf("****[mt6573_pmu_init] INIT : Need TRIM VAPROC\r\n");
	}
	else
	{
		printf("****[mt6573_pmu_init] INIT : No need TRIM VAPROC\r\n");
	}
	printf("****[mt6573_pmu_init] INIT : HW init settings for all platforms : TRIM VAPROC (%x,%x,%x)\n", 
		ret_val_check, ret_val, INREG16(0x7002F940));	
	//--------------------------------------------------------------------------------------------------
	ret_val = INREG32(0x70024104);
	ret_val_check = ret_val;	
	if(ret_val_check & 0x08000000)
	{
		ret_val = ret_val & 0x07C00000;
		ret_val = (ret_val >> 22);
		ret_val = (ret_val << 4);
		PMU_DRV_SetData16(0x7002F900, 0x01F0, ((kal_uint16)(ret_val)));
		printf("****[mt6573_pmu_init] INIT : Need TRIM VCORE\r\n");
	}
	else
	{
		printf("****[mt6573_pmu_init] INIT : No need TRIM VCORE\r\n");
	}
	printf("****[mt6573_pmu_init] INIT : HW init settings for all platforms : TRIM VCORE (%x,%x,%x)\n", 
		ret_val_check, ret_val, INREG16(0x7002F900));
	//--------------------------------------------------------------------------------------------------
	printf("****[mt6573_pmu_init] INIT : HW init settings for all platforms : KPLED\n");
	upmu_kpled_sfstrt_en(KPLED, KAL_TRUE);
	upmu_kpled_sfstrt_c(KPLED, KPLED_SFSTRT_C_31US_X_1);

	printf("****[mt6573_pmu_init] INIT : HW init settings for all platforms : SIM\n");
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
	printf("****[mt6573_pmu_init] INIT : HW init settings for customization : VCAMA\n");
	upmu_ldo_ocfb_enable(LDO_VCAMA, KAL_FALSE);	
	upmu_ldo_oc_auto_off(LDO_VCAMA, KAL_FALSE);	
	upmu_ldo_ndis_enable(LDO_VCAMA, KAL_TRUE);	
	upmu_ldo_vol_sel(LDO_VCAMA, UPMU_VOLT_2_8_0_0_V);	
	upmu_ldo_stb_td(LDO_VCAMA, LDO_STB_TD_200_US);

	printf("****[mt6573_pmu_init] INIT : HW init settings for customization : VCAMD\n");
	upmu_ldo_ocfb_enable(LDO_VCAMD, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VCAMD, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VCAMD, KAL_TRUE);
	upmu_ldo_vol_sel(LDO_VCAMD, UPMU_VOLT_1_3_0_0_V);
	upmu_ldo_cal(LDO_VCAMD, 0x0);
	upmu_ldo_stb_td(LDO_VCAMD, LDO_STB_TD_200_US);

	printf("****[mt6573_pmu_init] INIT : HW init settings for customization : VSIM\n");
	upmu_ldo_ocfb_enable(LDO_VSIM, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VSIM, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VSIM, KAL_TRUE);
	upmu_ldo_stb_td(LDO_VSIM, LDO_STB_TD_200_US);

	printf("****[mt6573_pmu_init] INIT : HW init settings for customization : VSIM2\n");
	upmu_ldo_ocfb_enable(LDO_VSIM2, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VSIM2, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VSIM2, KAL_TRUE);
	upmu_ldo_stb_td(LDO_VSIM2, LDO_STB_TD_200_US);

	printf("****[mt6573_pmu_init] INIT : HW init settings for customization : VIBR\n");
	upmu_ldo_ocfb_enable(LDO_VIBR, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VIBR, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VIBR, KAL_TRUE);
	upmu_ldo_vol_sel(LDO_VIBR, UPMU_VOLT_1_3_0_0_V);	
	upmu_ldo_stb_td(LDO_VIBR, LDO_STB_TD_200_US);

	printf("****[mt6573_pmu_init] INIT : HW init settings for customization : VCAMA2\n");
	upmu_ldo_ocfb_enable(LDO_VCAMA2, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VCAMA2, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VCAMA2, KAL_TRUE);
	upmu_ldo_vol_sel(LDO_VCAMA2, UPMU_VOLT_2_8_0_0_V);
	upmu_ldo_stb_td(LDO_VCAMA2, LDO_STB_TD_200_US);

	printf("****[mt6573_pmu_init] INIT : HW init settings for customization : VCAMD2\n");
	upmu_ldo_ocfb_enable(LDO_VCAMD2, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VCAMD2, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VCAMD2, KAL_TRUE);
	upmu_ldo_vol_sel(LDO_VCAMD2, UPMU_VOLT_1_8_0_0_V);
	upmu_ldo_stb_td(LDO_VCAMD2, LDO_STB_TD_200_US);

	printf("****[mt6573_pmu_init] INIT : HW init settings for customization : VM12\n");
	upmu_ldo_ocfb_enable(LDO_VM12, KAL_FALSE);
	upmu_ldo_oc_auto_off(LDO_VM12, KAL_FALSE);
	upmu_ldo_ndis_enable(LDO_VM12, KAL_TRUE);
	upmu_ldo_stb_td(LDO_VM12, LDO_STB_TD_200_US);

	printf("****[mt6573_pmu_init] INIT : HW init settings for customization : KPLED\n");
	upmu_kpled_sel(KPLED, 0x7);
	
	printf("****[mt6573_pmu_init] INIT : HW init settings for customization : P_CHARGER\n");
	upmu_vcdt_hv_vth(CHR, PMIC_ADPT_VOLT_07_000000_V);
	upmu_vcdt_hv_enable(CHR, KAL_TRUE);
	upmu_vbat_ov_vth(CHR, PMIC_ADPT_VOLT_04_350000_V);
	upmu_baton_ht_enable(CHR, KAL_FALSE);
	//upmu_otg_bvalid_det_enable(CHR, KAL_FALSE); //only charger exist can do
	//SETREG16(0x7002FE84,0x0004); // move to bat
	CLRREG16(0x7002F700,0x0002); // VRF_ON_SEL=0
	
}

void pmu6573_vaproc_protect(void)
{
	unsigned int ret_val=0;
	unsigned int ret_val_check=0;
	//--------------------------------------------------------------------------------------------------
	ret_val = INREG32(0x70024104);
	ret_val_check = ret_val;	
	if(ret_val_check & 0x10000000)
	{
		if(ret_val & 0x00080000)
		{
			printf("****[pmu6573_uboot_init] INIT : No need VAPROC Protect 1\r\n");
		}
		else
		{
			PMU_DRV_SetData16(0x7002F940, 0x01F0, ((kal_uint16)(0x8) << 4));
			printf("****[pmu6573_uboot_init] INIT : Need set VAPROC 1.4V (0x%x)\r\n", INREG16(0x7002F940));	
		}
	}
	else
	{
		printf("****[pmu6573_uboot_init] INIT : No need VAPROC Protect 2\r\n");
	}
	printf("****[pmu6573_uboot_init] pmu6573_vaproc_protect (%x,%x,%x)\n", 
		ret_val_check, ret_val, INREG16(0x7002F940));	
	//--------------------------------------------------------------------------------------------------
}

void pmu6573_VIO18_trim(void)
{
	unsigned int ret_val=0;
	unsigned int ret_val_check=0;
	//--------------------------------------------------------------------------------------------------
	ret_val = INREG32(0x70024108);
	ret_val_check = ret_val;	
	if(ret_val_check & 0x0020)
	{
		ret_val = ret_val & 0x001F;
		PMU_DRV_SetData16(0x7002F920, 0x01F0, ((kal_uint16)(ret_val) << 4));
		printf("****[mt6573_pmu_init] INIT : Need trim VIO18\r\n");			
	}
	else
	{
		printf("****[mt6573_pmu_init] INIT : No need trim VIO18\r\n");
	}
	printf("****[mt6573_pmu_init] pmu6573_VIO18_trim (%d,%d,0x%x,VIO18_CON0=0x%x)\n", 
		ret_val_check, ret_val, INREG32(0x70024108), INREG16(0x7002F920) );	
	//--------------------------------------------------------------------------------------------------
}

void mt6573_pmu_init(void)
{
	upmu_adc_measure_vbat_enable(CHR, KAL_TRUE);
	upmu_adc_measure_vsen_enable(CHR, KAL_TRUE);
	upmu_adc_measure_vchr_enable(CHR, KAL_TRUE);

#if 0 // init at kernel botting
	/* Driver init settings for all platforms */	
	pmu6573_hw_init();	
	printf("[mt6573_pmu_init] pmu6573_hw_init... \r\n");

	/* PMU Customization : Different may have different values */
	pmu6573_pmu_customization();
	printf("[mt6573_pmu_init] pmu6573_pmu_customization... \r\n");
#endif	

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
	PMU_DRV_SetData16(0x7002F204, 0x0800, ((kal_uint16)(0x1) << 11));
	printf("REG[0x7002F204]=%x\r\n", INREG16(0x7002F204));

#ifdef CFG_POWER_CHARGING
	/* Disable DRVBUS */
	mt_set_gpio_mode(GPIO2,GPIO_MODE_05);
	mt_set_gpio_dir(GPIO2,GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO2,GPIO_OUT_ZERO);
	printf("****[mt6573_pmu_init] UBOOT : FIX ME! Disable DRVBUS for avoiding the HW issue \n");
#endif	

	printf("[mt6573_pmu_init] Done \r\n");
}

void mt65xx_pmu_init(void)
{
	mt6573_pmu_init();
}

//#endif // PMIC_6573_REG_API

