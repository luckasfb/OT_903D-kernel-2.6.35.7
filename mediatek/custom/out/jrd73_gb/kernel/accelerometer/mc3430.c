

#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/earlysuspend.h>
#include <linux/platform_device.h>
#include <asm/atomic.h>

#include <cust_acc.h>
#include <linux/hwmsensor.h>
#include <linux/hwmsen_dev.h>
#include <linux/sensors_io.h>
#include "mc3430.h"
#include <linux/hwmsen_helper.h>

#ifdef MT6516
#include <mach/mt6516_devs.h>
#include <mach/mt6516_typedefs.h>
#include <mach/mt6516_gpio.h>
#include <mach/mt6516_pll.h>
#endif

#ifdef MT6573
#include <mach/mt6573_devs.h>
#include <mach/mt6573_typedefs.h>
#include <mach/mt6573_gpio.h>
#include <mach/mt6573_pll.h>
#endif

#ifdef MT6575
#include <mach/mt6575_devs.h>
#include <mach/mt6575_typedefs.h>
#include <mach/mt6575_gpio.h>
#include <mach/mt6575_pm_ldo.h>
#endif


/*-------------------------MT6516&MT6573 define-------------------------------*/
#ifdef MT6516
#define POWER_NONE_MACRO MT6516_POWER_NONE
#endif

#ifdef MT6573
#define POWER_NONE_MACRO MT65XX_POWER_NONE
#endif

#ifdef MT6575
#define POWER_NONE_MACRO MT65XX_POWER_NONE
#endif


/*----------------------------------------------------------------------------*/
#define I2C_DRIVERID_MC34X0 210
/*----------------------------------------------------------------------------*/
#define DEBUG 1
/*----------------------------------------------------------------------------*/
#define CONFIG_MC34X0_LOWPASS   /*apply low pass filter on output*/       
#define SW_CALIBRATION

/*----------------------------------------------------------------------------*/
#define MC34X0_AXIS_X          0
#define MC34X0_AXIS_Y          1
#define MC34X0_AXIS_Z          2
#define MC34X0_AXES_NUM        3
#define MC34X0_DATA_LEN      3//  6
#define MC34X0_DEV_NAME        "MC34X0"
/*----------------------------------------------------------------------------*/
static const struct i2c_device_id mc34x0_i2c_id[] = {{MC34X0_DEV_NAME,0},{}};
/*the adapter id will be available in customization*/
static unsigned short mc34x0_force[] = {0x00, MC34X0_I2C_SLAVE_ADDR, I2C_CLIENT_END, I2C_CLIENT_END};
static const unsigned short *const mc34x0_forces[] = { mc34x0_force, NULL };
static struct i2c_client_address_data mc34x0_addr_data = { .forces = mc34x0_forces,};

/*----------------------------------------------------------------------------*/
static int mc34x0_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id); 
static int mc34x0_i2c_remove(struct i2c_client *client);
static int mc34x0_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);
static int mc34x0_suspend(struct i2c_client *client, pm_message_t msg) ;
static int mc34x0_resume(struct i2c_client *client);

/*----------------------------------------------------------------------------*/
typedef enum {
    ADX_TRC_FILTER  = 0x01,
    ADX_TRC_RAWDATA = 0x02,
    ADX_TRC_IOCTL   = 0x04,
    ADX_TRC_CALI	= 0X08,
    ADX_TRC_INFO	= 0X10,
} ADX_TRC;
/*----------------------------------------------------------------------------*/
struct scale_factor{
    u8  whole;
    u8  fraction;
};
/*----------------------------------------------------------------------------*/
struct data_resolution {
    struct scale_factor scalefactor;
    int                 sensitivity;
};
/*----------------------------------------------------------------------------*/
#define C_MAX_FIR_LENGTH (32)
/*----------------------------------------------------------------------------*/
struct data_filter {
    s16 raw[C_MAX_FIR_LENGTH][MC34X0_AXES_NUM];
    int sum[MC34X0_AXES_NUM];
    int num;
    int idx;
};
/*----------------------------------------------------------------------------*/
struct mc34x0_i2c_data {
    struct i2c_client *client;
    struct acc_hw *hw;
    struct hwmsen_convert   cvt;
    
    /*misc*/
    struct data_resolution *reso;
    atomic_t                trace;
    atomic_t                suspend;
    atomic_t                selftest;
	atomic_t				filter;
    s16                     cali_sw[MC34X0_AXES_NUM+1];

    /*data*/
    s8                      offset[MC34X0_AXES_NUM+1];  /*+1: for 4-byte alignment*/
    s16                     data[MC34X0_AXES_NUM+1];

#if defined(CONFIG_MC34X0_LOWPASS)
    atomic_t                firlen;
    atomic_t                fir_en;
    struct data_filter      fir;
#endif 
    /*early suspend*/
#if defined(CONFIG_HAS_EARLYSUSPEND)
    struct early_suspend    early_drv;
#endif     
};
/*----------------------------------------------------------------------------*/
static struct i2c_driver mc34x0_i2c_driver = {
    .driver = {
        .owner          = THIS_MODULE,
        .name           = MC34X0_DEV_NAME,
    },
	.probe      		= mc34x0_i2c_probe,
	.remove    			= mc34x0_i2c_remove,
	.detect				= mc34x0_i2c_detect,
//#if !defined(CONFIG_HAS_EARLYSUSPEND)    
    .suspend            = mc34x0_suspend,
    .resume             = mc34x0_resume,
//#endif
	.id_table = mc34x0_i2c_id,
	.address_data = &mc34x0_addr_data,
};

/*----------------------------------------------------------------------------*/
static struct i2c_client *mc34x0_i2c_client = NULL;

static struct platform_driver mc34x0_gsensor_driver;

static struct mc34x0_i2c_data *obj_i2c_data = NULL;
static bool sensor_power = false;
static GSENSOR_VECTOR3D gsensor_gain;
static char selftestRes[8]= {0}; 


/*----------------------------------------------------------------------------*/
#define GSE_TAG                  "[Gsensor] "
#define GSE_FUN(f)               printk(KERN_INFO GSE_TAG"%s\n", __FUNCTION__)
#define GSE_ERR(fmt, args...)    printk(KERN_ERR GSE_TAG"%s %d : "fmt, __FUNCTION__, __LINE__, ##args)
#define GSE_LOG(fmt, args...)    printk(KERN_INFO GSE_TAG fmt, ##args)
/*----------------------------------------------------------------------------*/
static struct data_resolution mc34x0_data_resolution[] = {
 /*8 combination by {FULL_RES,RANGE}*/
    {{ 3, 9}, 256},   /*+/-2g  in 10-bit resolution:  3.9 mg/LSB*/
    {{ 7, 8}, 128},   /*+/-4g  in 10-bit resolution:  7.8 mg/LSB*/
    {{15, 6},  64},   /*+/-8g  in 10-bit resolution: 15.6 mg/LSB*/
    {{ 0, 9}, 256},   /*+/-8g  in 14-bit resolution:  0.98 mg/LSB*/
    {{ 10, 7}, 86},   /*+/-1.5g  in 8-bit resolution:  15.6 mg/LSB*/
};
/*----------------------------------------------------------------------------*/
static struct data_resolution mc34x0_offset_resolution = {{3, 9}, 256}; 

/*--------------------MC34X0 power control function----------------------------------*/
static void MC34X0_power(struct acc_hw *hw, unsigned int on) 
{
	static unsigned int power_on = 0;

	if(hw->power_id != POWER_NONE_MACRO)		// have externel LDO
	{        
		GSE_LOG("power %s\n", on ? "on" : "off");
		if(power_on == on)	// power status not change
		{
			GSE_LOG("ignore power control: %d\n", on);
		}
		else if(on)	// power on
		{
			if(!hwPowerOn(hw->power_id, hw->power_vol, "MC34X0"))
			{
				GSE_ERR("power on fails!!\n");
			}
		}
		else	// power off
		{
			if (!hwPowerDown(hw->power_id, "MC34X0"))
			{
				GSE_ERR("power off fail!!\n");
			}			  
		}
	}
	power_on = on;    
}
/*----------------------------------------------------------------------------*/
static int MC34X0_SetDataResolution(struct mc34x0_i2c_data *obj)
{
	//int err;
	//u8  dat;
	u8 reso;

	/*if((err = hwmsen_read_byte(obj->client, MC34X0_REG_DATA_FORMAT, &dat)))
	{
		GSE_ERR("write data format fail!!\n");
		return err;
	}

	/*the data_reso is combined by 2 bits: {FULL_RES, DATA_RANGE}*/
	//reso = (dat & MC34X0_RANGE_MASK)>>2; 

	reso=4;

	if(reso < sizeof(mc34x0_data_resolution)/sizeof(mc34x0_data_resolution[0]))
	{        
		obj->reso = &mc34x0_data_resolution[reso];
		return 0;
	}
	else
	{
		return -EINVAL;
	}
}

/*----------------------------------------------------------------------------*/
static int MC34X0_ReadData(struct i2c_client *client, s16 data[MC34X0_AXES_NUM])
{
	struct mc34x0_i2c_data *priv = i2c_get_clientdata(client);        
	u8 addr = MC34X0_REG_DATAX0;
	u8 buf[MC34X0_DATA_LEN] = {0};
	int err = 0;
	int i;

	if(NULL == client)
	{
		err = -EINVAL;
	}
	else if((err = hwmsen_read_block(client, addr, buf, 0x03)))
	{
		GSE_ERR("error: %d\n", err);
	}
	else
	{
		data[MC34X0_AXIS_X] = (s8)(buf[MC34X0_AXIS_X]);
		data[MC34X0_AXIS_Y] = (s8)(buf[MC34X0_AXIS_Y]);
		data[MC34X0_AXIS_Z] = (s8)(buf[MC34X0_AXIS_Z]);

           /*
		for(i=0;i<3;i++)				
		{								//because the data is store in binary complement number formation in computer system
			if ( data[i] == 0x0200 )	       //so we want to calculate actual number here
				data[i]= -512;			//10bit resolution, 512= 2^(10-1)
			else if ( data[i] & 0x0200 )//transfor format
			{							//printk("data 0 step %x \n",data[i]);
				data[i] -= 0x1;			//printk("data 1 step %x \n",data[i]);
				data[i] = ~data[i];		//printk("data 2 step %x \n",data[i]);
				data[i] &= 0x01ff;		//printk("data 3 step %x \n\n",data[i]);
				data[i] = -data[i];		
			}
		}	
           */

		data[MC34X0_AXIS_X] += priv->cali_sw[MC34X0_AXIS_X];
		data[MC34X0_AXIS_Y] += priv->cali_sw[MC34X0_AXIS_Y];
		data[MC34X0_AXIS_Z] += priv->cali_sw[MC34X0_AXIS_Z];

		if(atomic_read(&priv->trace) & ADX_TRC_RAWDATA)
		{
			GSE_LOG("[%08X %08X %08X] => [%5d %5d %5d]\n", data[MC34X0_AXIS_X], data[MC34X0_AXIS_Y], data[MC34X0_AXIS_Z],
		                               data[MC34X0_AXIS_X], data[MC34X0_AXIS_Y], data[MC34X0_AXIS_Z]);
		}
#ifdef CONFIG_MC34X0_LOWPASS
		if(atomic_read(&priv->filter))
		{
			if(atomic_read(&priv->fir_en) && !atomic_read(&priv->suspend))
			{
				int idx, firlen = atomic_read(&priv->firlen);   
				if(priv->fir.num < firlen)
				{                
					priv->fir.raw[priv->fir.num][MC34X0_AXIS_X] = data[MC34X0_AXIS_X];
					priv->fir.raw[priv->fir.num][MC34X0_AXIS_Y] = data[MC34X0_AXIS_Y];
					priv->fir.raw[priv->fir.num][MC34X0_AXIS_Z] = data[MC34X0_AXIS_Z];
					priv->fir.sum[MC34X0_AXIS_X] += data[MC34X0_AXIS_X];
					priv->fir.sum[MC34X0_AXIS_Y] += data[MC34X0_AXIS_Y];
					priv->fir.sum[MC34X0_AXIS_Z] += data[MC34X0_AXIS_Z];
					if(atomic_read(&priv->trace) & ADX_TRC_FILTER)
					{
						GSE_LOG("add [%2d] [%5d %5d %5d] => [%5d %5d %5d]\n", priv->fir.num,
							priv->fir.raw[priv->fir.num][MC34X0_AXIS_X], priv->fir.raw[priv->fir.num][MC34X0_AXIS_Y], priv->fir.raw[priv->fir.num][MC34X0_AXIS_Z],
							priv->fir.sum[MC34X0_AXIS_X], priv->fir.sum[MC34X0_AXIS_Y], priv->fir.sum[MC34X0_AXIS_Z]);
					}
					priv->fir.num++;
					priv->fir.idx++;
				}
				else
				{
					idx = priv->fir.idx % firlen;
					priv->fir.sum[MC34X0_AXIS_X] -= priv->fir.raw[idx][MC34X0_AXIS_X];
					priv->fir.sum[MC34X0_AXIS_Y] -= priv->fir.raw[idx][MC34X0_AXIS_Y];
					priv->fir.sum[MC34X0_AXIS_Z] -= priv->fir.raw[idx][MC34X0_AXIS_Z];
					priv->fir.raw[idx][MC34X0_AXIS_X] = data[MC34X0_AXIS_X];
					priv->fir.raw[idx][MC34X0_AXIS_Y] = data[MC34X0_AXIS_Y];
					priv->fir.raw[idx][MC34X0_AXIS_Z] = data[MC34X0_AXIS_Z];
					priv->fir.sum[MC34X0_AXIS_X] += data[MC34X0_AXIS_X];
					priv->fir.sum[MC34X0_AXIS_Y] += data[MC34X0_AXIS_Y];
					priv->fir.sum[MC34X0_AXIS_Z] += data[MC34X0_AXIS_Z];
					priv->fir.idx++;
					data[MC34X0_AXIS_X] = priv->fir.sum[MC34X0_AXIS_X]/firlen;
					data[MC34X0_AXIS_Y] = priv->fir.sum[MC34X0_AXIS_Y]/firlen;
					data[MC34X0_AXIS_Z] = priv->fir.sum[MC34X0_AXIS_Z]/firlen;
					if(atomic_read(&priv->trace) & ADX_TRC_FILTER)
					{
						GSE_LOG("add [%2d] [%5d %5d %5d] => [%5d %5d %5d] : [%5d %5d %5d]\n", idx,
						priv->fir.raw[idx][MC34X0_AXIS_X], priv->fir.raw[idx][MC34X0_AXIS_Y], priv->fir.raw[idx][MC34X0_AXIS_Z],
						priv->fir.sum[MC34X0_AXIS_X], priv->fir.sum[MC34X0_AXIS_Y], priv->fir.sum[MC34X0_AXIS_Z],
						data[MC34X0_AXIS_X], data[MC34X0_AXIS_Y], data[MC34X0_AXIS_Z]);
					}
				}
			}
		}	
#endif         
	}
	return err;
}
/*----------------------------------------------------------------------------*/
static int MC34X0_ReadOffset(struct i2c_client *client, s8 ofs[MC34X0_AXES_NUM])
{    
	int err;

#ifdef SW_CALIBRATION
	ofs[0]=ofs[1]=ofs[2]=0x0;
#else
	if((err = hwmsen_read_block(client, MC34X0_REG_OFSX, ofs, MC34X0_AXES_NUM)))
	{
		GSE_ERR("error: %d\n", err);
	}
#endif

	return err;    
}
/*----------------------------------------------------------------------------*/
static int MC34X0_ResetCalibration(struct i2c_client *client)
{
	struct mc34x0_i2c_data *obj = i2c_get_clientdata(client);
	s8 ofs[MC34X0_AXES_NUM] = {0x00, 0x00, 0x00};
	int err;
	
	#ifdef SW_CALIBRATION
		
	#else
		if(err = hwmsen_write_block(client, MC34X0_REG_OFSX, ofs, 4))
		{
			GSE_ERR("error: %d\n", err);
		}
	#endif

	memset(obj->cali_sw, 0x00, sizeof(obj->cali_sw));
	memset(obj->offset, 0x00, sizeof(obj->offset));
	return err;    
}

/*----------------------------------------------------------------------------*/
static int MC34X0_ReadCalibration(struct i2c_client *client, int dat[MC34X0_AXES_NUM])
{
    struct mc34x0_i2c_data *obj = i2c_get_clientdata(client);
    int err = 0;
    int mul;

       #ifdef SW_CALIBRATION
	  struct mc34x0_i2c_data *priv = i2c_get_clientdata(client);        
	  u8 addr = MC34X0_REG_DATAX0;
	  u8 buf[MC34X0_DATA_LEN] = {0};
	  int i;
	  s16 data[MC34X0_AXES_NUM] = {0x00,0x00,0x00};

	  if(NULL == client)
	  {
	  	err = -EINVAL;
	  }
	  else if((err = hwmsen_read_block(client, addr, buf, 0x03)))
	  {
		GSE_ERR("error: %d\n", err);
          }
	  else
	  {
		data[MC34X0_AXIS_X] = (s8)(buf[MC34X0_AXIS_X]);
		data[MC34X0_AXIS_Y] = (s8)(buf[MC34X0_AXIS_Y]);
		data[MC34X0_AXIS_Z] = (s8)(buf[MC34X0_AXIS_Z]);
	  }	
       #endif


       #ifdef SW_CALIBRATION
		mul = 0;//only SW Calibration, disable HW Calibration	 
	#else
           if ((err = MC34X0_ReadOffset(client, obj->offset))) {
        GSE_ERR("read offset fail, %d\n", err);
        return err;
        }    
        mul = obj->reso->sensitivity/mc34x0_offset_resolution.sensitivity;
       #endif 

   dat[obj->cvt.map[MC34X0_AXIS_X]] = obj->cvt.sign[MC34X0_AXIS_X]*(obj->offset[MC34X0_AXIS_X]*mul + obj->cali_sw[MC34X0_AXIS_X]);
   dat[obj->cvt.map[MC34X0_AXIS_Y]] = obj->cvt.sign[MC34X0_AXIS_Y]*(obj->offset[MC34X0_AXIS_Y]*mul + obj->cali_sw[MC34X0_AXIS_Y]);
   dat[obj->cvt.map[MC34X0_AXIS_Z]] = obj->cvt.sign[MC34X0_AXIS_Z]*(obj->offset[MC34X0_AXIS_Z]*mul + obj->cali_sw[MC34X0_AXIS_Z]);                        

    return 0;
}

/*----------------------------------------------------------------------------*/
static int MC34X0_ReadCalibrationEx(struct i2c_client *client, int act[MC34X0_AXES_NUM], int raw[MC34X0_AXES_NUM])
{  
	/*raw: the raw calibration data; act: the actual calibration data*/
	struct mc34x0_i2c_data *obj = i2c_get_clientdata(client);
	int err;
	int mul;

	#ifdef SW_CALIBRATION
		mul = 0;//only SW Calibration, disable HW Calibration	
	#else
	      if((err = MC34X0_ReadOffset(client, obj->offset)))
       	{
		GSE_ERR("read offset fail, %d\n", err);
		return err;
	       }    
       	mul = obj->reso->sensitivity/mc34x0_offset_resolution.sensitivity;
	#endif
	
	raw[MC34X0_AXIS_X] = obj->offset[MC34X0_AXIS_X]*mul + obj->cali_sw[MC34X0_AXIS_X];
	raw[MC34X0_AXIS_Y] = obj->offset[MC34X0_AXIS_Y]*mul + obj->cali_sw[MC34X0_AXIS_Y];
	raw[MC34X0_AXIS_Z] = obj->offset[MC34X0_AXIS_Z]*mul + obj->cali_sw[MC34X0_AXIS_Z];

	act[obj->cvt.map[MC34X0_AXIS_X]] = obj->cvt.sign[MC34X0_AXIS_X]*raw[MC34X0_AXIS_X];
	act[obj->cvt.map[MC34X0_AXIS_Y]] = obj->cvt.sign[MC34X0_AXIS_Y]*raw[MC34X0_AXIS_Y];
	act[obj->cvt.map[MC34X0_AXIS_Z]] = obj->cvt.sign[MC34X0_AXIS_Z]*raw[MC34X0_AXIS_Z];                        

	return 0;
}

/*----------------------------------------------------------------------------*/
static int MC34X0_WriteCalibration(struct i2c_client *client, int dat[MC34X0_AXES_NUM])
{
	struct mc34x0_i2c_data *obj = i2c_get_clientdata(client);
	int err;
	int cali[MC34X0_AXES_NUM], raw[MC34X0_AXES_NUM];
	int lsb = mc34x0_offset_resolution.sensitivity;
	int divisor = obj->reso->sensitivity/lsb;

	if((err = MC34X0_ReadCalibrationEx(client, cali, raw)))	/*offset will be updated in obj->offset*/
	{ 
		GSE_ERR("read offset fail, %d\n", err);
		return err;
	}

	GSE_LOG("OLDOFF: (%+3d %+3d %+3d): (%+3d %+3d %+3d) / (%+3d %+3d %+3d)\n", 
		raw[MC34X0_AXIS_X], raw[MC34X0_AXIS_Y], raw[MC34X0_AXIS_Z],
		obj->offset[MC34X0_AXIS_X], obj->offset[MC34X0_AXIS_Y], obj->offset[MC34X0_AXIS_Z],
		obj->cali_sw[MC34X0_AXIS_X], obj->cali_sw[MC34X0_AXIS_Y], obj->cali_sw[MC34X0_AXIS_Z]);

	/*calculate the real offset expected by caller*/
	cali[MC34X0_AXIS_X] += dat[MC34X0_AXIS_X];
	cali[MC34X0_AXIS_Y] += dat[MC34X0_AXIS_Y];
	cali[MC34X0_AXIS_Z] += dat[MC34X0_AXIS_Z];

	GSE_LOG("UPDATE: (%+3d %+3d %+3d)\n", 
		dat[MC34X0_AXIS_X], dat[MC34X0_AXIS_Y], dat[MC34X0_AXIS_Z]);

#ifdef SW_CALIBRATION
	obj->cali_sw[MC34X0_AXIS_X] = obj->cvt.sign[MC34X0_AXIS_X]*(cali[obj->cvt.map[MC34X0_AXIS_X]]);
	obj->cali_sw[MC34X0_AXIS_Y] = obj->cvt.sign[MC34X0_AXIS_Y]*(cali[obj->cvt.map[MC34X0_AXIS_Y]]);
	obj->cali_sw[MC34X0_AXIS_Z] = obj->cvt.sign[MC34X0_AXIS_Z]*(cali[obj->cvt.map[MC34X0_AXIS_Z]]);	

	GSE_LOG("NEWOFF: (%+3d %+3d %+3d): (%+3d %+3d %+3d) / (%+3d %+3d %+3d)\n", 
		obj->offset[MC34X0_AXIS_X]*divisor + obj->cali_sw[MC34X0_AXIS_X], 
		obj->offset[MC34X0_AXIS_Y]*divisor + obj->cali_sw[MC34X0_AXIS_Y], 
		obj->offset[MC34X0_AXIS_Z]*divisor + obj->cali_sw[MC34X0_AXIS_Z], 
		obj->offset[MC34X0_AXIS_X], obj->offset[MC34X0_AXIS_Y], obj->offset[MC34X0_AXIS_Z],
		obj->cali_sw[MC34X0_AXIS_X], obj->cali_sw[MC34X0_AXIS_Y], obj->cali_sw[MC34X0_AXIS_Z]);
#else
	obj->offset[MC34X0_AXIS_X] = (s8)(obj->cvt.sign[MC34X0_AXIS_X]*(cali[obj->cvt.map[MC34X0_AXIS_X]])/(divisor));
	obj->offset[MC34X0_AXIS_Y] = (s8)(obj->cvt.sign[MC34X0_AXIS_Y]*(cali[obj->cvt.map[MC34X0_AXIS_Y]])/(divisor));
	obj->offset[MC34X0_AXIS_Z] = (s8)(obj->cvt.sign[MC34X0_AXIS_Z]*(cali[obj->cvt.map[MC34X0_AXIS_Z]])/(divisor));

	/*convert software calibration using standard calibration*/
	obj->cali_sw[MC34X0_AXIS_X] = obj->cvt.sign[MC34X0_AXIS_X]*(cali[obj->cvt.map[MC34X0_AXIS_X]])%(divisor);
	obj->cali_sw[MC34X0_AXIS_Y] = obj->cvt.sign[MC34X0_AXIS_Y]*(cali[obj->cvt.map[MC34X0_AXIS_Y]])%(divisor);
	obj->cali_sw[MC34X0_AXIS_Z] = obj->cvt.sign[MC34X0_AXIS_Z]*(cali[obj->cvt.map[MC34X0_AXIS_Z]])%(divisor);

	GSE_LOG("NEWOFF: (%+3d %+3d %+3d): (%+3d %+3d %+3d) / (%+3d %+3d %+3d)\n", 
		obj->offset[MC34X0_AXIS_X]*divisor + obj->cali_sw[MC34X0_AXIS_X], 
		obj->offset[MC34X0_AXIS_Y]*divisor + obj->cali_sw[MC34X0_AXIS_Y], 
		obj->offset[MC34X0_AXIS_Z]*divisor + obj->cali_sw[MC34X0_AXIS_Z], 
		obj->offset[MC34X0_AXIS_X], obj->offset[MC34X0_AXIS_Y], obj->offset[MC34X0_AXIS_Z],
		obj->cali_sw[MC34X0_AXIS_X], obj->cali_sw[MC34X0_AXIS_Y], obj->cali_sw[MC34X0_AXIS_Z]);

	if((err = hwmsen_write_block(obj->client, MC34X0_REG_OFSX, obj->offset, MC34X0_AXES_NUM)))
	{
		GSE_ERR("write offset fail: %d\n", err);
		return err;
	}
#endif

	return err;
}
/*----------------------------------------------------------------------------*/
static int MC34X0_CheckDeviceID(struct i2c_client *client)
{
	u8 databuf[10];    
	int res = 0;

	memset(databuf, 0, sizeof(u8)*10);    
	databuf[0] = MC34X0_REG_DEVID;    

	res = i2c_master_send(client, databuf, 0x1);
	if(res <= 0)
	{
		goto exit_MC34X0_CheckDeviceID;
	}
	
	udelay(500);

	databuf[0] = 0x0;        
	res = i2c_master_recv(client, databuf, 0x01);
	if(res <= 0)
	{
		goto exit_MC34X0_CheckDeviceID;
	}
	
	exit_MC34X0_CheckDeviceID:
	if (res <= 0)
	{
		return MC34X0_ERR_I2C;
	}
	
	return MC34X0_SUCCESS;
}
/*----------------------------------------------------------------------------*/
static int MC34X0_SetPowerMode(struct i2c_client *client, bool enable)
{
	u8 databuf[2];    
	int res = 0;
	u8 addr = MC34X0_REG_POWER_CTL;
	struct mc34x0_i2c_data *obj = i2c_get_clientdata(client);

	if(enable == sensor_power)
	{
		GSE_LOG("Sensor power status is newest!\n");
		return MC34X0_SUCCESS;
	}

	if(hwmsen_read_block(client, addr, databuf, 0x01))
	{
		GSE_ERR("read power ctl register err!\n");
		return MC34X0_ERR_I2C;
	}

	databuf[0] = MC34X0_WAKE_MODE; 

	if(enable == FALSE) 
	{
		databuf[0] |= MC34X0_STANDBY_MODE;
	}
	else
	{
		// do nothing
	}
	databuf[1] = databuf[0];
	databuf[0] = MC34X0_REG_POWER_CTL;
	

	res = i2c_master_send(client, databuf, 0x2);

	if(res <= 0)
	{
		GSE_LOG("set power mode failed!\n");
		return MC34X0_ERR_I2C;
	}
	else if(atomic_read(&obj->trace) & ADX_TRC_INFO)
	{
		GSE_LOG("set power mode ok %d!\n", databuf[1]);
	}

	sensor_power = enable;
	
	return MC34X0_SUCCESS;    
}
/*----------------------------------------------------------------------------*/
static int MC34X0_SetDataFormat(struct i2c_client *client, u8 dataformat)
{
	struct mc34x0_i2c_data *obj = i2c_get_clientdata(client);
	//u8 databuf[10];    
	//int res = 0;

	/*memset(databuf, 0, sizeof(u8)*10);    

	if(hwmsen_read_block(client, MC34X0_REG_DATA_FORMAT, databuf, 0x01))
	{
		printk("mc34x0 read Dataformat failt \n");
		return MC34X0_ERR_I2C;
	}

	databuf[0] &= ~MC34X0_RANGE_MASK;
	databuf[0] |= dataformat|MC34X0_RANGE_MUSTWRITE; 
	databuf[1] = databuf[0];
	databuf[0] = MC34X0_REG_DATA_FORMAT;

	res = i2c_master_send(client, databuf, 0x2);

	if(res <= 0)
	{
		return MC34X0_ERR_I2C;
	}*/

	//printk("MC34X0_SetDataFormat OK! \n");

	return MC34X0_SetDataResolution(obj);    
}
/*----------------------------------------------------------------------------*/
static int MC34X0_SetBWRate(struct i2c_client *client, u8 bwrate)
{
	/*u8 databuf[10];    
	int res = 0;

	memset(databuf, 0, sizeof(u8)*10);    

	if(hwmsen_read_block(client, MC34X0_REG_BW_RATE, databuf, 0x01))
	{
		printk("mc34x0 read rate failt \n");
		return MC34X0_ERR_I2C;
	}

	databuf[0] &= ~MC34X0_BW_MASK;
	databuf[0] |= bwrate;
	databuf[1] = databuf[0];
	databuf[0] = MC34X0_REG_BW_RATE;

	res = i2c_master_send(client, databuf, 0x2);

	if(res <= 0)
	{
		return MC34X0_ERR_I2C;
	}*/
	
	return MC34X0_SUCCESS;    
}
/*----------------------------------------------------------------------------*/
static int MC34X0_SetIntEnable(struct i2c_client *client, u8 intenable)
{
	u8 databuf[10];    
	int res = 0;

	memset(databuf, 0, sizeof(u8)*10);    
	databuf[0] = MC34X0_REG_INT_ENABLE;    
	databuf[1] = intenable;

	res = i2c_master_send(client, databuf, 0x2);

	if(res <= 0)
	{
		return MC34X0_ERR_I2C;
	}
	
	return MC34X0_SUCCESS;    
}
/*----------------------------------------------------------------------------*/
static int mc34x0_init_client(struct i2c_client *client, int reset_cali)
{
	struct mc34x0_i2c_data *obj = i2c_get_clientdata(client);
	int res = 0;

       //client->addr |= I2C_ENEXT_FLAG; //for EVB Borad
	
	res = MC34X0_CheckDeviceID(client); 
	if(res != MC34X0_SUCCESS)
	{
		return res;
	}

	res = MC34X0_SetPowerMode(client, false);
	if(res != MC34X0_SUCCESS)
	{
		return res;
	}

    #if 1//Added by jrd.lipeng: the otp values will be modified by TP(reading id operation). So reload them to registers.
        {
            hwmsen_write_byte(client, 0x17, 0x80);
            msleep(5);
            hwmsen_write_byte(client, 0x1c, 0x80);
            hwmsen_write_byte(client, 0x1c, 0x0);
            hwmsen_write_byte(client, 0x17, 0x0);
            msleep(5);
        }
    #endif

	res = MC34X0_SetBWRate(client, MC34X0_BW_128HZ);
	if(res != MC34X0_SUCCESS ) //0x2C->BW=100Hz
	{
		return res;
	}

	res = MC34X0_SetDataFormat(client, MC34X0_RANGE_2G);
	if(res != MC34X0_SUCCESS) //0x2C->BW=100Hz
	{
		return res;
	}

	gsensor_gain.x = gsensor_gain.y = gsensor_gain.z = obj->reso->sensitivity;

	res = MC34X0_SetIntEnable(client, 0x00);//disable INT        
	if(res != MC34X0_SUCCESS) 
	{
		return res;
	}

	if(0 != reset_cali)
	{ 
		/*reset calibration only in power on*/
		res = MC34X0_ResetCalibration(client);
		if(res != MC34X0_SUCCESS)
		{
			return res;
		}
	}

#ifdef CONFIG_MC34X0_LOWPASS
	memset(&obj->fir, 0x00, sizeof(obj->fir));  
#endif

	return MC34X0_SUCCESS;
}
/*----------------------------------------------------------------------------*/
static int MC34X0_ReadChipInfo(struct i2c_client *client, char *buf, int bufsize)
{
	u8 databuf[10];    

	memset(databuf, 0, sizeof(u8)*10);

	if((NULL == buf)||(bufsize<=30))
	{
		return -1;
	}
	
	if(NULL == client)
	{
		*buf = 0;
		return -2;
	}

	sprintf(buf, "MC34X0 Chip");
	return 0;
}
/*----------------------------------------------------------------------------*/
static int MC34X0_ReadSensorData(struct i2c_client *client, char *buf, int bufsize)
{
	struct mc34x0_i2c_data *obj = obj_i2c_data; //(struct mc34x0_i2c_data*)i2c_get_clientdata(client);
	client = obj->client;
	//u8 databuf[20];
	int acc[MC34X0_AXES_NUM];
	int res = 0;
	//memset(databuf, 0, sizeof(u8)*10);

	if(NULL == buf)
	{
		return -1;
	}
	if(NULL == client)
	{
		*buf = 0;
		return -2;
	}

	if(sensor_power == FALSE)
	{
		res = MC34X0_SetPowerMode(client, true);
		if(res)
		{
			GSE_ERR("Power on mc34x0 error %d!\n", res);
		}
		msleep(20);
	}

	if((res = MC34X0_ReadData(client, obj->data)))
	{        
		GSE_ERR("I2C error: ret value=%d", res);
		return -3;
	}
	else
	{
              #ifdef SW_CALIBRATION     

              #else
		obj->data[MC34X0_AXIS_X] += obj->cali_sw[MC34X0_AXIS_X];
		obj->data[MC34X0_AXIS_Y] += obj->cali_sw[MC34X0_AXIS_Y];
		obj->data[MC34X0_AXIS_Z] += obj->cali_sw[MC34X0_AXIS_Z];
	      #endif

		/*remap coordinate*/
		acc[obj->cvt.map[MC34X0_AXIS_X]] = obj->cvt.sign[MC34X0_AXIS_X]*obj->data[MC34X0_AXIS_X];
		acc[obj->cvt.map[MC34X0_AXIS_Y]] = obj->cvt.sign[MC34X0_AXIS_Y]*obj->data[MC34X0_AXIS_Y];
		acc[obj->cvt.map[MC34X0_AXIS_Z]] = obj->cvt.sign[MC34X0_AXIS_Z]*obj->data[MC34X0_AXIS_Z];

		//GSE_LOG("Mapped gsensor data: %d, %d, %d!\n", acc[MC34X0_AXIS_X], acc[MC34X0_AXIS_Y], acc[MC34X0_AXIS_Z]);

		//Out put the mg
		acc[MC34X0_AXIS_X] = acc[MC34X0_AXIS_X] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
		acc[MC34X0_AXIS_Y] = acc[MC34X0_AXIS_Y] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
		acc[MC34X0_AXIS_Z] = acc[MC34X0_AXIS_Z] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;		


		sprintf(buf, "%04x %04x %04x", acc[MC34X0_AXIS_X], acc[MC34X0_AXIS_Y], acc[MC34X0_AXIS_Z]);
		if(atomic_read(&obj->trace) & ADX_TRC_IOCTL)
		{
			GSE_LOG("gsensor data: %s!\n", buf);
		}
	}
	
	return 0;
}
/*----------------------------------------------------------------------------*/
static int MC34X0_ReadRawData(struct i2c_client *client, char *buf)
{
	struct mc34x0_i2c_data *obj = (struct mc34x0_i2c_data*)i2c_get_clientdata(client);
	int res = 0;

	if (!buf || !client)
	{
		return EINVAL;
	}
	
	if((res = MC34X0_ReadData(client, obj->data)))
	{        
		GSE_ERR("I2C error: ret value=%d", res);
		return EIO;
	}
	else
	{
		sprintf(buf, "%04x %04x %04x", obj->data[MC34X0_AXIS_X], 
			obj->data[MC34X0_AXIS_Y], obj->data[MC34X0_AXIS_Z]);
	
	}
	
	return 0;
}
/*----------------------------------------------------------------------------*/
static int MC34X0_InitSelfTest(struct i2c_client *client)
{
	int res = 0;

	MC34X0_SetPowerMode(client, true);

	res = MC34X0_SetBWRate(client, MC34X0_BW_128HZ);
	if(res != MC34X0_SUCCESS ) //0x2C->BW=100Hz
	{
		return res;
	}
	
	res = MC34X0_SetDataFormat(client, MC34X0_RANGE_2G);
	if(res != MC34X0_SUCCESS) //0x2C->BW=100Hz
	{
		return res;
	}

	mdelay(60);

	return MC34X0_SUCCESS;
}
/*----------------------------------------------------------------------------*/
static int MC34X0_JudgeTestResult(struct i2c_client *client)
{

	struct mc34x0_i2c_data *obj = (struct mc34x0_i2c_data*)i2c_get_clientdata(client);
	int res = 0;
	s16  acc[MC34X0_AXES_NUM];
	int  self_result;

	
	if(res = MC34X0_ReadData(client, acc))
	{        
		GSE_ERR("I2C error: ret value=%d", res);
		return EIO;
	}
	else
	{			
		printk("0 step: %d %d %d\n", acc[0],acc[1],acc[2]);

		acc[MC34X0_AXIS_X] = acc[MC34X0_AXIS_X] * 1000 / 128;
		acc[MC34X0_AXIS_Y] = acc[MC34X0_AXIS_Y] * 1000 / 128;
		acc[MC34X0_AXIS_Z] = acc[MC34X0_AXIS_Z] * 1000 / 128;
		
		printk("1 step: %d %d %d\n", acc[0],acc[1],acc[2]);
		
		self_result = acc[MC34X0_AXIS_X]*acc[MC34X0_AXIS_X] 
			+ acc[MC34X0_AXIS_Y]*acc[MC34X0_AXIS_Y] 
			+ acc[MC34X0_AXIS_Z]*acc[MC34X0_AXIS_Z];
			
		
		printk("2 step: result = %d", self_result);

	    if ( (self_result>1960000) && (self_result<9000000) ) //between 0.7g and 1.5g 
	    {												 
			GSE_ERR("MC34X0_JudgeTestResult successful\n");
			return MC34X0_SUCCESS;
		}
		{
	        GSE_ERR("MC34X0_JudgeTestResult failt\n");
	        return -EINVAL;
	    }
	
	}
	
}
/*----------------------------------------------------------------------------*/
static ssize_t show_chipinfo_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = mc34x0_i2c_client;
	char strbuf[MC34X0_BUFSIZE];
	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}
	
	MC34X0_ReadChipInfo(client, strbuf, MC34X0_BUFSIZE);
	return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);        
}
/*----------------------------------------------------------------------------*/
static ssize_t show_sensordata_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = mc34x0_i2c_client;
	char strbuf[MC34X0_BUFSIZE];
	
	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}
	MC34X0_ReadSensorData(client, strbuf, MC34X0_BUFSIZE);
	return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);            
}
/*----------------------------------------------------------------------------*/
static ssize_t show_cali_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = mc34x0_i2c_client;
	struct mc34x0_i2c_data *obj;
	int err, len = 0, mul;
	int tmp[MC34X0_AXES_NUM];

	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}

	obj = i2c_get_clientdata(client);

	if((err = MC34X0_ReadOffset(client, obj->offset)))
	{
		return -EINVAL;
	}
	else if((err = MC34X0_ReadCalibration(client, tmp)))
	{
		return -EINVAL;
	}
	else
	{    
		mul = obj->reso->sensitivity/mc34x0_offset_resolution.sensitivity;
		len += snprintf(buf+len, PAGE_SIZE-len, "[HW ][%d] (%+3d, %+3d, %+3d) : (0x%02X, 0x%02X, 0x%02X)\n", mul,                        
			obj->offset[MC34X0_AXIS_X], obj->offset[MC34X0_AXIS_Y], obj->offset[MC34X0_AXIS_Z],
			obj->offset[MC34X0_AXIS_X], obj->offset[MC34X0_AXIS_Y], obj->offset[MC34X0_AXIS_Z]);
		len += snprintf(buf+len, PAGE_SIZE-len, "[SW ][%d] (%+3d, %+3d, %+3d)\n", 1, 
			obj->cali_sw[MC34X0_AXIS_X], obj->cali_sw[MC34X0_AXIS_Y], obj->cali_sw[MC34X0_AXIS_Z]);

		len += snprintf(buf+len, PAGE_SIZE-len, "[ALL]    (%+3d, %+3d, %+3d) : (%+3d, %+3d, %+3d)\n", 
			obj->offset[MC34X0_AXIS_X]*mul + obj->cali_sw[MC34X0_AXIS_X],
			obj->offset[MC34X0_AXIS_Y]*mul + obj->cali_sw[MC34X0_AXIS_Y],
			obj->offset[MC34X0_AXIS_Z]*mul + obj->cali_sw[MC34X0_AXIS_Z],
			tmp[MC34X0_AXIS_X], tmp[MC34X0_AXIS_Y], tmp[MC34X0_AXIS_Z]);
		
		return len;
    }
}
/*----------------------------------------------------------------------------*/
static ssize_t store_cali_value(struct device_driver *ddri, const char *buf, size_t count)
{
	struct i2c_client *client = mc34x0_i2c_client;  
	int err, x, y, z;
	int dat[MC34X0_AXES_NUM];

	if(!strncmp(buf, "rst", 3))
	{
		if((err = MC34X0_ResetCalibration(client)))
		{
			GSE_ERR("reset offset err = %d\n", err);
		}	
	}
	else if(3 == sscanf(buf, "0x%02X 0x%02X 0x%02X", &x, &y, &z))
	{
		dat[MC34X0_AXIS_X] = x;
		dat[MC34X0_AXIS_Y] = y;
		dat[MC34X0_AXIS_Z] = z;
		if((err = MC34X0_WriteCalibration(client, dat)))
		{
			GSE_ERR("write calibration err = %d\n", err);
		}		
	}
	else
	{
		GSE_ERR("invalid format\n");
	}
	
	return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t show_self_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = mc34x0_i2c_client;
	//struct mc34x0_i2c_data *obj;

	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}

	//obj = i2c_get_clientdata(client);

    return snprintf(buf, 8, "%s\n", selftestRes);
}
/*----------------------------------------------------------------------------*/
static ssize_t store_self_value(struct device_driver *ddri, const char *buf, size_t count)
{   /*write anything to this register will trigger the process*/
	struct item{
	s16 raw[MC34X0_AXES_NUM];
	};
	
	struct i2c_client *client = mc34x0_i2c_client;  
	int idx, res, num;
	struct item *prv = NULL, *nxt = NULL;


	if(1 != sscanf(buf, "%d", &num))
	{
		GSE_ERR("parse number fail\n");
		return count;
	}
	else if(num == 0)
	{
		GSE_ERR("invalid data count\n");
		return count;
	}

	prv = kzalloc(sizeof(*prv) * num, GFP_KERNEL);
	nxt = kzalloc(sizeof(*nxt) * num, GFP_KERNEL);
	if (!prv || !nxt)
	{
		goto exit;
	}


	GSE_LOG("NORMAL:\n");
	MC34X0_SetPowerMode(client,true); 

	/*initial setting for self test*/
	MC34X0_InitSelfTest(client);
	GSE_LOG("SELFTEST:\n");    

	if(!MC34X0_JudgeTestResult(client))
	{
		GSE_LOG("SELFTEST : PASS\n");
		strcpy(selftestRes,"y");
	}	
	else
	{
		GSE_LOG("SELFTEST : FAIL\n");		
		strcpy(selftestRes,"n");
	}
	
	exit:
	/*restore the setting*/    
	mc34x0_init_client(client, 0);
	kfree(prv);
	kfree(nxt);
	return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t show_selftest_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = mc34x0_i2c_client;
	struct mc34x0_i2c_data *obj;

	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}

	obj = i2c_get_clientdata(client);
	return snprintf(buf, PAGE_SIZE, "%d\n", atomic_read(&obj->selftest));
}
/*----------------------------------------------------------------------------*/
static ssize_t store_selftest_value(struct device_driver *ddri, const char *buf, size_t count)
{
	struct mc34x0_i2c_data *obj = obj_i2c_data;
	int tmp;

	if(NULL == obj)
	{
		GSE_ERR("i2c data obj is null!!\n");
		return 0;
	}
	
	
	if(1 == sscanf(buf, "%d", &tmp))
	{        
		if(atomic_read(&obj->selftest) && !tmp)
		{
			/*enable -> disable*/
			mc34x0_init_client(obj->client, 0);
		}
		else if(!atomic_read(&obj->selftest) && tmp)
		{
			/*disable -> enable*/
			MC34X0_InitSelfTest(obj->client);            
		}
		
		GSE_LOG("selftest: %d => %d\n", atomic_read(&obj->selftest), tmp);
		atomic_set(&obj->selftest, tmp); 
	}
	else
	{ 
		GSE_ERR("invalid content: '%s', length = %d\n", buf, count);   
	}
	return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t show_firlen_value(struct device_driver *ddri, char *buf)
{
#ifdef CONFIG_MC34X0_LOWPASS
	struct i2c_client *client = mc34x0_i2c_client;
	struct mc34x0_i2c_data *obj = i2c_get_clientdata(client);
	if(atomic_read(&obj->firlen))
	{
		int idx, len = atomic_read(&obj->firlen);
		GSE_LOG("len = %2d, idx = %2d\n", obj->fir.num, obj->fir.idx);

		for(idx = 0; idx < len; idx++)
		{
			GSE_LOG("[%5d %5d %5d]\n", obj->fir.raw[idx][MC34X0_AXIS_X], obj->fir.raw[idx][MC34X0_AXIS_Y], obj->fir.raw[idx][MC34X0_AXIS_Z]);
		}
		
		GSE_LOG("sum = [%5d %5d %5d]\n", obj->fir.sum[MC34X0_AXIS_X], obj->fir.sum[MC34X0_AXIS_Y], obj->fir.sum[MC34X0_AXIS_Z]);
		GSE_LOG("avg = [%5d %5d %5d]\n", obj->fir.sum[MC34X0_AXIS_X]/len, obj->fir.sum[MC34X0_AXIS_Y]/len, obj->fir.sum[MC34X0_AXIS_Z]/len);
	}
	return snprintf(buf, PAGE_SIZE, "%d\n", atomic_read(&obj->firlen));
#else
	return snprintf(buf, PAGE_SIZE, "not support\n");
#endif
}
/*----------------------------------------------------------------------------*/
static ssize_t store_firlen_value(struct device_driver *ddri, const char *buf, size_t count)
{
#ifdef CONFIG_MC34X0_LOWPASS
	struct i2c_client *client = mc34x0_i2c_client;  
	struct mc34x0_i2c_data *obj = i2c_get_clientdata(client);
	int firlen;

	if(1 != sscanf(buf, "%d", &firlen))
	{
		GSE_ERR("invallid format\n");
	}
	else if(firlen > C_MAX_FIR_LENGTH)
	{
		GSE_ERR("exceeds maximum filter length\n");
	}
	else
	{ 
		atomic_set(&obj->firlen, firlen);
		if(0 == firlen)
		{
			atomic_set(&obj->fir_en, 0);
		}
		else
		{
			memset(&obj->fir, 0x00, sizeof(obj->fir));
			atomic_set(&obj->fir_en, 1);
		}
	}
#endif    
	return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t show_trace_value(struct device_driver *ddri, char *buf)
{
	ssize_t res;
	struct mc34x0_i2c_data *obj = obj_i2c_data;
	if (obj == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return 0;
	}
	
	res = snprintf(buf, PAGE_SIZE, "0x%04X\n", atomic_read(&obj->trace));     
	return res;    
}
/*----------------------------------------------------------------------------*/
static ssize_t store_trace_value(struct device_driver *ddri, const char *buf, size_t count)
{
	struct mc34x0_i2c_data *obj = obj_i2c_data;
	int trace;
	if (obj == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return 0;
	}
	
	if(1 == sscanf(buf, "0x%x", &trace))
	{
		atomic_set(&obj->trace, trace);
	}	
	else
	{
		GSE_ERR("invalid content: '%s', length = %d\n", buf, count);
	}
	
	return count;    
}
/*----------------------------------------------------------------------------*/
static ssize_t show_status_value(struct device_driver *ddri, char *buf)
{
	ssize_t len = 0;    
	struct mc34x0_i2c_data *obj = obj_i2c_data;
	if (obj == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return 0;
	}	
	
	if(obj->hw)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "CUST: %d %d (%d %d)\n", 
	            obj->hw->i2c_num, obj->hw->direction, obj->hw->power_id, obj->hw->power_vol);   
	}
	else
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "CUST: NULL\n");
	}
	return len;    
}
/*----------------------------------------------------------------------------*/
static DRIVER_ATTR(chipinfo,             S_IRUGO, show_chipinfo_value,      NULL);
static DRIVER_ATTR(sensordata,           S_IRUGO, show_sensordata_value,    NULL);
static DRIVER_ATTR(cali,       S_IWUSR | S_IRUGO, show_cali_value,          store_cali_value);
static DRIVER_ATTR(self,       S_IWUSR | S_IRUGO, show_selftest_value,          store_selftest_value);
static DRIVER_ATTR(selftest,   S_IWUSR | S_IRUGO, show_self_value ,      store_self_value );  //MC34X0 don't have selftest
static DRIVER_ATTR(firlen,     S_IWUSR | S_IRUGO, show_firlen_value,        store_firlen_value);
static DRIVER_ATTR(trace,      S_IWUSR | S_IRUGO, show_trace_value,         store_trace_value);
static DRIVER_ATTR(status,               S_IRUGO, show_status_value,        NULL);
/*----------------------------------------------------------------------------*/
static struct driver_attribute *mc34x0_attr_list[] = {
	&driver_attr_chipinfo,     /*chip information*/
	&driver_attr_sensordata,   /*dump sensor data*/
	&driver_attr_cali,         /*show calibration data*/
	&driver_attr_self,         /*self test demo*/
	&driver_attr_selftest,     /*self control: 0: disable, 1: enable*/
	&driver_attr_firlen,       /*filter length: 0: disable, others: enable*/
	&driver_attr_trace,        /*trace log*/
	&driver_attr_status,        
};
/*----------------------------------------------------------------------------*/
static int mc34x0_create_attr(struct device_driver *driver) 
{
	int idx, err = 0;
	int num = (int)(sizeof(mc34x0_attr_list)/sizeof(mc34x0_attr_list[0]));
	if (driver == NULL)
	{
		return -EINVAL;
	}

	for(idx = 0; idx < num; idx++)
	{
		if((err = driver_create_file(driver, mc34x0_attr_list[idx])))
		{            
			GSE_ERR("driver_create_file (%s) = %d\n", mc34x0_attr_list[idx]->attr.name, err);
			break;
		}
	}    
	return err;
}
/*----------------------------------------------------------------------------*/
static int mc34x0_delete_attr(struct device_driver *driver)
{
	int idx ,err = 0;
	int num = (int)(sizeof(mc34x0_attr_list)/sizeof(mc34x0_attr_list[0]));

	if(driver == NULL)
	{
		return -EINVAL;
	}
	

	for(idx = 0; idx < num; idx++)
	{
		driver_remove_file(driver, mc34x0_attr_list[idx]);
	}
	

	return err;
}


/*----------------------------------------------------------------------------*/
int mc3430_gsensor_operate(void* self, uint32_t command, void* buff_in, int size_in,
		void* buff_out, int size_out, int* actualout)

{
	int err = 0;
	int value, sample_delay;	
	struct mc34x0_i2c_data *priv = (struct mc34x0_i2c_data*)self;
	hwm_sensor_data* gsensor_data;
	char buff[MC34X0_BUFSIZE];
	
	//GSE_FUN(f);
	switch (command)
	{
		case SENSOR_DELAY:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				GSE_ERR("Set delay parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				value = *(int *)buff_in;
				if(value <= 5)
				{
					sample_delay = MC34X0_BW_256HZ;
				}
				else if(value <= 10)
				{
					sample_delay = MC34X0_BW_128HZ;
				}
				else
				{
					sample_delay = MC34X0_BW_64HZ;
				}
				
				err = MC34X0_SetBWRate(priv->client, sample_delay);
				if(err != MC34X0_SUCCESS ) //0x2C->BW=100Hz
				{
					GSE_ERR("Set delay parameter error!\n");
				}

				if(value >= 50)
				{
					atomic_set(&priv->filter, 0);
				}
				else
				{			
				#if defined(CONFIG_MC34X0_LOWPASS)  
					priv->fir.num = 0;
					priv->fir.idx = 0;
					priv->fir.sum[MC34X0_AXIS_X] = 0;
					priv->fir.sum[MC34X0_AXIS_Y] = 0;
					priv->fir.sum[MC34X0_AXIS_Z] = 0;
					atomic_set(&priv->filter, 1);
                             #endif					
				}
			}
			break;

		case SENSOR_ENABLE:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				GSE_ERR("Enable sensor parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				value = *(int *)buff_in;
				if(((value == 0) && (sensor_power == false)) ||((value == 1) && (sensor_power == true)))
				{
					GSE_LOG("Gsensor device have updated!\n");
				}
				else
				{
					err = MC34X0_SetPowerMode( priv->client, !sensor_power);
				}
			}
			break;

		case SENSOR_GET_DATA:
			if((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data)))
			{
				GSE_ERR("get sensor data parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				gsensor_data = (hwm_sensor_data *)buff_out;
				err = MC34X0_ReadSensorData(priv->client, buff, MC34X0_BUFSIZE);
				if(!err)
				{
				   sscanf(buff, "%x %x %x", &gsensor_data->values[0], 
					   &gsensor_data->values[1], &gsensor_data->values[2]);				
				   gsensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;				
				   gsensor_data->value_divide = 1000;
				}
			}
			break;
		default:
			GSE_ERR("gsensor operate function no this parameter %d!\n", command);
			err = -1;
			break;
	}
	
	return err;
}

static int mc34x0_open(struct inode *inode, struct file *file)
{
	file->private_data = mc34x0_i2c_client;

	if(file->private_data == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return -EINVAL;
	}
	return nonseekable_open(inode, file);
}
/*----------------------------------------------------------------------------*/
static int mc34x0_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}
/*----------------------------------------------------------------------------*/
static int mc34x0_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
       unsigned long arg)
{
	struct i2c_client *client = (struct i2c_client*)file->private_data;
	struct mc34x0_i2c_data *obj = (struct mc34x0_i2c_data*)i2c_get_clientdata(client);	
	char strbuf[MC34X0_BUFSIZE];
	void __user *data;
	SENSOR_DATA sensor_data;
	int err = 0;
	int cali[3];

	//GSE_FUN(f);
	if(_IOC_DIR(cmd) & _IOC_READ)
	{
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	}
	else if(_IOC_DIR(cmd) & _IOC_WRITE)
	{
		err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	}

	if(err)
	{
		GSE_ERR("access error: %08X, (%2d, %2d)\n", cmd, _IOC_DIR(cmd), _IOC_SIZE(cmd));
		return -EFAULT;
	}

	switch(cmd)
	{
		case GSENSOR_IOCTL_INIT:
			mc34x0_init_client(client, 0);			
			break;

		case GSENSOR_IOCTL_READ_CHIPINFO:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			
			MC34X0_ReadChipInfo(client, strbuf, MC34X0_BUFSIZE);
			if(copy_to_user(data, strbuf, strlen(strbuf)+1))
			{
				err = -EFAULT;
				break;
			}				 
			break;	  

		case GSENSOR_IOCTL_READ_SENSORDATA:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			
			MC34X0_ReadSensorData(client, strbuf, MC34X0_BUFSIZE);
			if(copy_to_user(data, strbuf, strlen(strbuf)+1))
			{
				err = -EFAULT;
				break;	  
			}				 
			break;

		case GSENSOR_IOCTL_READ_GAIN:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}			
			
			if(copy_to_user(data, &gsensor_gain, sizeof(GSENSOR_VECTOR3D)))
			{
				err = -EFAULT;
				break;
			}				 
			break;

		case GSENSOR_IOCTL_READ_RAW_DATA:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			MC34X0_ReadRawData(client, strbuf);
			if(copy_to_user(data, strbuf, strlen(strbuf)+1))
			{
				err = -EFAULT;
				break;	  
			}
			break;	  

		case GSENSOR_IOCTL_SET_CALI:
			data = (void __user*)arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			if(copy_from_user(&sensor_data, data, sizeof(sensor_data)))
			{
				err = -EFAULT;
				break;	  
			}
			
			if(atomic_read(&obj->suspend))
			{
				GSE_ERR("Perform calibration in suspend state!!\n");
				err = -EINVAL;
			}
			else
			{
				cali[MC34X0_AXIS_X] = sensor_data.x * obj->reso->sensitivity / GRAVITY_EARTH_1000;
				cali[MC34X0_AXIS_Y] = sensor_data.y * obj->reso->sensitivity / GRAVITY_EARTH_1000;
				cali[MC34X0_AXIS_Z] = sensor_data.z * obj->reso->sensitivity / GRAVITY_EARTH_1000;		

				printk("mc34x0_ioctrl(----yanfang----write---%d,%d,%d\n",cali[MC34X0_AXIS_X],cali[MC34X0_AXIS_Y],cali[MC34X0_AXIS_Z]);
				
				err = MC34X0_WriteCalibration(client, cali);			 
			}
			break;

		case GSENSOR_IOCTL_CLR_CALI:
			err = MC34X0_ResetCalibration(client);
			break;

		case GSENSOR_IOCTL_GET_CALI:
			data = (void __user*)arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			if((err = MC34X0_ReadCalibration(client, cali)))
			{
				break;
			}

			sensor_data.x = cali[MC34X0_AXIS_X] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
			sensor_data.y = cali[MC34X0_AXIS_Y] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
			sensor_data.z = cali[MC34X0_AXIS_Z] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;

		
			
			if(copy_to_user(data, &sensor_data, sizeof(sensor_data)))
			{
				err = -EFAULT;
				break;
			}		
			break;
		

		default:
			GSE_ERR("unknown IOCTL: 0x%08x\n", cmd);
			err = -ENOIOCTLCMD;
			break;
			
	}

	return err;
}


/*----------------------------------------------------------------------------*/
static struct file_operations mc34x0_fops = {
	.owner = THIS_MODULE,
	.open = mc34x0_open,
	.release = mc34x0_release,
	.ioctl = mc34x0_ioctl,
};
/*----------------------------------------------------------------------------*/
static struct miscdevice mc34x0_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "gsensor",
	.fops = &mc34x0_fops,
};
/*----------------------------------------------------------------------------*/
//#ifndef CONFIG_HAS_EARLYSUSPEND
/*----------------------------------------------------------------------------*/
static int mc34x0_suspend(struct i2c_client *client, pm_message_t msg) 
{
	struct mc34x0_i2c_data *obj = i2c_get_clientdata(client);    
	int err = 0;
	GSE_FUN();    

	if(msg.event == PM_EVENT_SUSPEND)
	{   
		if(obj == NULL)
		{
			GSE_ERR("null pointer!!\n");
			return -EINVAL;
		}
		atomic_set(&obj->suspend, 1);
		if((err = MC34X0_SetPowerMode(obj->client, false)))
		{
			GSE_ERR("write power control fail!!\n");
			return err;
		}        
		MC34X0_power(obj->hw, 0);
		GSE_LOG("mc34x0_suspend ok\n");
	}
	return err;
}
/*----------------------------------------------------------------------------*/
static int mc34x0_resume(struct i2c_client *client)
{
	struct mc34x0_i2c_data *obj = i2c_get_clientdata(client);        
	int err;
	GSE_FUN();

	if(obj == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return -EINVAL;
	}

	MC34X0_power(obj->hw, 1);
	if((err = mc34x0_init_client(client, 0)))
	{
		GSE_ERR("initialize client fail!!\n");
		return err;        
	}
	atomic_set(&obj->suspend, 0);
	GSE_LOG("mc34x0_resume ok\n");

	return 0;
}
/*----------------------------------------------------------------------------*/
//#else /*CONFIG_HAS_EARLY_SUSPEND is defined*/
/*----------------------------------------------------------------------------*/
static void mc34x0_early_suspend(struct early_suspend *h) 
{
	struct mc34x0_i2c_data *obj = container_of(h, struct mc34x0_i2c_data, early_drv);   
	int err;
	GSE_FUN();    

	if(obj == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return;
	}
	atomic_set(&obj->suspend, 1); 
	if((err = MC34X0_SetPowerMode(obj->client, false)))
	{
		GSE_ERR("write power control fail!!\n");
		return;
	}

	sensor_power = false;
	
	MC34X0_power(obj->hw, 0);
}
/*----------------------------------------------------------------------------*/
static void mc34x0_late_resume(struct early_suspend *h)
{
	struct mc34x0_i2c_data *obj = container_of(h, struct mc34x0_i2c_data, early_drv);         
	int err;
	GSE_FUN();

	if(obj == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return;
	}

	MC34X0_power(obj->hw, 1);
	if((err = mc34x0_init_client(obj->client, 0)))
	{
		GSE_ERR("initialize client fail!!\n");
		return;        
	}
	atomic_set(&obj->suspend, 0);    
}
/*----------------------------------------------------------------------------*/
//#endif /*CONFIG_HAS_EARLYSUSPEND*/
/*----------------------------------------------------------------------------*/
static int mc34x0_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) 
{    
	strcpy(info->type, MC34X0_DEV_NAME);
	return 0;
}

/*----------------------------------------------------------------------------*/
static int mc34x0_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct i2c_client *new_client;
	struct mc34x0_i2c_data *obj;
	struct hwmsen_object sobj;
	int err = 0;
	GSE_FUN();

	if(!(obj = kzalloc(sizeof(*obj), GFP_KERNEL)))
	{
		err = -ENOMEM;
		goto exit;
	}
	
	memset(obj, 0, sizeof(struct mc34x0_i2c_data));

	obj->hw = mc3430_get_cust_acc_hw();
	if((err = hwmsen_get_convert(obj->hw->direction, &obj->cvt)))
	{
		GSE_ERR("invalid direction: %d\n", obj->hw->direction);
		goto exit;
	}

	obj_i2c_data = obj;
	obj->client = client;
	new_client = obj->client;
	i2c_set_clientdata(new_client,obj);
	
	atomic_set(&obj->trace, 0);
	atomic_set(&obj->suspend, 0);
	
#ifdef CONFIG_MC34X0_LOWPASS
	if(obj->hw->firlen > C_MAX_FIR_LENGTH)
	{
		atomic_set(&obj->firlen, C_MAX_FIR_LENGTH);
	}	
	else
	{
		atomic_set(&obj->firlen, obj->hw->firlen);
	}
	
	if(atomic_read(&obj->firlen) > 0)
	{
		atomic_set(&obj->fir_en, 1);
	}
	
#endif

	mc34x0_i2c_client = new_client;	

	if((err = mc34x0_init_client(new_client, 1)))
	{
		goto exit_init_failed;
	}
	

	if((err = misc_register(&mc34x0_device)))
	{
		GSE_ERR("mc34x0_device register failed\n");
		goto exit_misc_device_register_failed;
	}

	if((err = mc34x0_create_attr(&mc34x0_gsensor_driver.driver)))


	{
		GSE_ERR("create attribute err = %d\n", err);
		goto exit_create_attr_failed;
	}

	sobj.self = obj;
    sobj.polling = 1;

    sobj.sensor_operate = mc3430_gsensor_operate;

	if((err = hwmsen_attach(ID_ACCELEROMETER, &sobj)))
	{
		GSE_ERR("attach fail = %d\n", err);
		goto exit_kfree;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	obj->early_drv.level    = EARLY_SUSPEND_LEVEL_DISABLE_FB - 1,
	obj->early_drv.suspend  = mc34x0_early_suspend,
	obj->early_drv.resume   = mc34x0_late_resume,    
	register_early_suspend(&obj->early_drv);
#endif 

	GSE_LOG("%s: OK\n", __func__);    
	return 0;

	exit_create_attr_failed:
	misc_deregister(&mc34x0_device);
	exit_misc_device_register_failed:
	exit_init_failed:
	//i2c_detach_client(new_client);
	exit_kfree:
	kfree(obj);
	exit:
	GSE_ERR("%s: err = %d\n", __func__, err);        
	return err;
}

/*----------------------------------------------------------------------------*/
static int mc34x0_i2c_remove(struct i2c_client *client)
{
	int err = 0;	

	if((err = mc34x0_delete_attr(&mc34x0_gsensor_driver.driver)))	
	{
		GSE_ERR("mc34x0_delete_attr fail: %d\n", err);
	}
	
	if((err = misc_deregister(&mc34x0_device)))
	{
		GSE_ERR("misc_deregister fail: %d\n", err);
	}

	if((err = hwmsen_detach(ID_ACCELEROMETER)))
	    

	mc34x0_i2c_client = NULL;
	i2c_unregister_device(client);
	kfree(i2c_get_clientdata(client));
	return 0;
}
/*----------------------------------------------------------------------------*/
static int mc34x0_probe(struct platform_device *pdev) 
{
	struct acc_hw *hw = mc3430_get_cust_acc_hw();
	GSE_FUN();

	MC34X0_power(hw, 1);
	mc34x0_force[0] = hw->i2c_num;
	if(i2c_add_driver(&mc34x0_i2c_driver))
	{
		GSE_ERR("add driver error\n");
		return -1;
	}
	return 0;
}
/*----------------------------------------------------------------------------*/
static int mc34x0_remove(struct platform_device *pdev)
{
    struct acc_hw *hw = mc3430_get_cust_acc_hw();

    GSE_FUN();    
    MC34X0_power(hw, 0);    
    i2c_del_driver(&mc34x0_i2c_driver);
    return 0;
}
/*----------------------------------------------------------------------------*/
static struct platform_driver mc34x0_gsensor_driver = {
	.probe      = mc34x0_probe,
	.remove     = mc34x0_remove,    
	.driver     = {
		.name  = "mc3430_gsensor",
		.owner = THIS_MODULE,
	}
};

/*----------------------------------------------------------------------------*/
static int __init mc34x0_init(void)
{
	GSE_FUN();
	if(platform_driver_register(&mc34x0_gsensor_driver))
	{
		GSE_ERR("failed to register driver");
		return -ENODEV;
	}

	return 0;    
}
/*----------------------------------------------------------------------------*/
static void __exit mc34x0_exit(void)
{
	GSE_FUN();

	platform_driver_unregister(&mc34x0_gsensor_driver);
}
/*----------------------------------------------------------------------------*/
module_init(mc34x0_init);
module_exit(mc34x0_exit);
/*----------------------------------------------------------------------------*/
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MC34X0 I2C driver");
MODULE_AUTHOR("tc.chu@mediatek.com");
