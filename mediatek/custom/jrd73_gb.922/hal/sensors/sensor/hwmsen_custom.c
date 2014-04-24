
#include <hardware/sensors.h>
#include <linux/hwmsensor.h>
#include "hwmsen_custom.h"

struct sensor_t sSensorList[MAX_NUM_SENSORS] = 
{
	{ 
		.name       = "mmc328x Orientation sensor",
		.vendor     = "memsic",
		.version    = 1,
		.handle     = ID_ORIENTATION,
		.type       = SENSOR_TYPE_ORIENTATION,
		.maxRange   = 360.0f,
		.resolution = 1.0f,
		.power      = 0.25f,
		.reserved   = {}
	},

	{ 
		.name       = "mmc328x 3-axis Magnetic Field sensor",
		.vendor     = "memsic",
		.version    = 1,
		.handle     = ID_MAGNETIC,
		.type       = SENSOR_TYPE_MAGNETIC_FIELD,
		.maxRange   = 600.0f,
		.resolution = 0.0016667f,
		.power      = 0.25f,
		.reserved   = {}
	}, 

	/*MT6516 the spec follows ADXL345*/
	{  
		.name       = "MT6516 3-axis Accelerometer",
		.vendor     = "The Android Open Source Project",
		.version    = 1,
		.handle     = ID_ACCELEROMETER,
		.type       = SENSOR_TYPE_ACCELEROMETER,
		.maxRange   = 32.0f,
		.resolution = 4.0f/1024.0f,
		.power      =130.0f/1000.0f,
		.reserved   = {}
	},        

	{ 
		.name       = "TMD2771 Proximity Sensor",
		.vendor     = "TAOS",
		.version    = 1,
		.handle     = ID_PROXIMITY,
		.type       = SENSOR_TYPE_PROXIMITY,
		.maxRange   = 1.00f,
		.resolution = 1.0f,
		.power      = 0.13f,
		.reserved   = {}
	},

        #if 0
	{ 
		.name       = "MPU3000  gyroscope Sensor",
		.vendor     = "Invensensor",
		.version    = 1,
		.handle     = ID_GYROSCOPE,
		.type       = SENSOR_TYPE_GYROSCOPE,
		.maxRange   = 17.45,
		.resolution = 0.000532113f,
		.power      = 6.1f,
		.reserved   = {}
	},
       #endif

};

