



#include <mach/mt6573_typedefs.h>
#include <linux/spinlock.h>

#include <linux/kernel.h>
#include <linux/io.h>
#include <asm/system.h>

#include "auddrv_register.h"
#include "auddrv_def.h"



void Afe_Set_Reg(kal_uint32 offset, kal_uint32 value , kal_uint32 mask)
{
   volatile kal_uint32 address = (AFE_BASE+offset);
   volatile kal_uint32 *AFE_Register = (volatile kal_uint32*)address;

   volatile kal_uint32 val_tmp;
   PRINTK_AFE_REG("Afe_Set_Reg offset=%x, value=%x, mask=%x \n",offset,value,mask);
   val_tmp = READ_REGISTER_UINT32(AFE_Register);
   val_tmp &= (~mask);
   mt65xx_reg_sync_writel(val_tmp,AFE_Register);
   val_tmp = READ_REGISTER_UINT32(AFE_Register);
   val_tmp |= (value&mask);
   mt65xx_reg_sync_writel(val_tmp,AFE_Register);

}

kal_uint32 Afe_Get_Reg(kal_uint32 offset)
{
   volatile kal_uint32 address = (AFE_BASE+offset);
   volatile kal_uint32 *Analog_Register = (volatile kal_uint32 *)address;
   volatile kal_uint32 val_tmp;
   val_tmp = READ_REGISTER_UINT32(Analog_Register);
   PRINTK_AFE_REG("Afe_Get_Reg offset=%x, value=%d \n",offset,*value);
   return val_tmp;
}

void Ana_Set_Reg(kal_uint32 offset, kal_uint32 value, kal_uint32 mask)
{
   volatile kal_uint32 address = (offset);
   volatile kal_uint32 *Analog_Register = (volatile kal_uint32 *)address;

   volatile kal_uint32 val_tmp;
   PRINTK_ANA_REG("Ana_Set_Reg offset=%x, value=%x, mask=%x \n",offset,value,mask);
   val_tmp = READ_REGISTER_UINT32(Analog_Register);
   val_tmp &= (~mask);
   mt65xx_reg_sync_writel(val_tmp,Analog_Register);
   val_tmp = READ_REGISTER_UINT32(Analog_Register);
   val_tmp |= (value&mask);
   mt65xx_reg_sync_writel(val_tmp,Analog_Register);

}

kal_uint32 Ana_Get_Reg(kal_uint32 offset)
{
   volatile kal_uint32 *value;
   UINT32 address = (offset);
   PRINTK_ANA_REG("Ana_Get_Reg offset=%x \n",offset);

   value = (volatile kal_uint32  *)(address);
   return *value;
}

// TBD...
void Afe_Enable_Memory_Power()
{
   // No need to enable memory power
   return;
}

// TBD...
void Afe_Disable_Memory_Power()
{
   // No need to disable memory power
   return;
}




