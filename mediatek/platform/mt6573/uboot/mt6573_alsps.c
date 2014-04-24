

#include <asm/arch/mt6573_gpio.h>
#include <asm/arch/mt6573_i2c.h>


#ifdef CM3623
#define APS_ERR printf
int cm3623_read_rar(u8 *data)
{     
	int ret = 0;
    ret = mt6573_i2c_polling_read(0,0x19,data,1);

	if(ret > 0)
	{

	  return -1;
	}
	return 0;
}

int cm3623_init_device()
{
	        
	u8 buf[] = {0x10};
	int ret = 0;
	u8 data;
    APS_ERR("init_device\n");
    ret = mt6573_i2c_polling_write(0,0x92,buf,1);

	if(ret > 0)
	{

	  return -1;
	}
	return 0;
}


static int cm3623_check_and_clear_intr() 
{
	int err;
	u8 addr;

	if (mt_get_gpio_in(GPIO_ALS_EINT_PIN) == 1) /*skip if no interrupt*/  
		{
		   APS_ERR("alsps no interrupt\n"); 
		   return 0;
		}
	    

	if(err = cm3623_read_rar(&addr))
	{
		APS_ERR("WARNING: read rar: %d\n", err);
		return 0;
	}
 
	return 0;
}

int cm3623_write_als( u8 cmd)
{
	u8 buf = cmd;
	int ret = 0;

    ret = mt6573_i2c_polling_write(0,0x90,buf,1);

	if(ret > 0)
	{

	  return -1;
	}
	return 0;    
}


int cm3623_write_ps(u8 cmd)
{      
	u8 buf = cmd;
	int ret = 0;
	ret = mt6573_i2c_polling_write(0,0xF0,buf,1);

	if(ret > 0)
	{

	  return -1;
	}

	return 0;    
}

int cm3623_write_ps_thd(u8 thd)
{       
	u8 buf = thd;
	int ret = 0;

	ret = mt6573_i2c_polling_write(0,0xF2,buf,1);

	if(ret > 0)
	{

	  return -1;
	}
	
	return 0;    
}



int cm3623_init_client()
{
	int err = -1;
	
	if(err = cm3623_check_and_clear_intr())
	{
		APS_ERR("check/clear intr: %d\n", err);
		//    return err;
	}
	
	if(err = cm3623_init_device())
	{
		APS_ERR("init dev: %d\n", err);
		return err;
	}
	
	if(err = cm3623_write_als(0xD3))
	{
		APS_ERR("write als: %d\n", err);
		return err;
	}
	
	if(err = cm3623_write_ps(0xC1))
	{
		APS_ERR("write ps: %d\n", err);
		return err;        
	}
	
	if(err = cm3623_write_ps_thd(0x03))
	{
		APS_ERR("write thd: %d\n", err);
		return err;        
	}
	
	APS_ERR("init alsps in uboot ok\n");
	
	return 0;
}

#endif

