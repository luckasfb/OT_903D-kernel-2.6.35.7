
#ifndef MC34X0_H
#define MC34X0_H
	 
#include <linux/ioctl.h>
	 
#define MC34X0_I2C_SLAVE_ADDR		0x98  
	 
	 /* MC34X0 Register Map  (Please refer to MC34X0 Specifications) */
#define MC34X0_REG_INT_ENABLE		0x06   
#define MC34X0_REG_POWER_CTL		0x07   
#define MC34X0_WAKE_MODE		       0x01   
#define MC34X0_STANDBY_MODE		0x03   
#define MC34X0_REG_DATAX0			0x00   
#define MC34X0_REG_DEVID			0x18   
#define MC34X0_REG_DATA_FORMAT	0x20  
#define MC34X0_RANGE_MUSTWRITE     0x03   
#define MC34X0_RANGE_MASK			0x0C   
#define MC34X0_RANGE_2G			0x00  
#define MC34X0_RANGE_4G			0x04   
#define MC34X0_RANGE_8G			0x08   
#define MC34X0_RANGE_8G_14BIT		0x0C  
#define MC34X0_REG_BW_RATE	       0x20  
#define MC34X0_BW_MASK			0x70  
#define MC34X0_BW_512HZ			0x00   
#define MC34X0_BW_256HZ			0x10 
#define MC34X0_BW_128HZ			0x20 
#define MC34X0_BW_64HZ				0x30   

#define MC34X0_FIXED_DEVID			0x88
	 
#define MC34X0_SUCCESS						 0
#define MC34X0_ERR_I2C						-1
#define MC34X0_ERR_STATUS					-3
#define MC34X0_ERR_SETUP_FAILURE			-4
#define MC34X0_ERR_GETGSENSORDATA	       -5
#define MC34X0_ERR_IDENTIFICATION			-6
	 
	 
#define MC34X0_BUFSIZE				256
	 
#endif

