

#include <utils/Log.h>
#include <utils/Errors.h>
#include <fcntl.h>
#include <math.h>

#include "MediaHal.h"
//#include "msdk_sensor_exp.h"
#include "camera_custom_sensor.h"
#if defined(IMX073_MIPI_RAW)
#include "camera_calibration_eeprom.h"
#endif
#include "kd_imgsensor.h"
//#include "image_sensor.h"

//TODO:remove once build system ready
//#include "camera_custom_cfg.h"

//#include "src/lib/inc/MediaLog.h"

//#define LOG_TAG "SENSORLIST"
//MUINT32 MT9P012_getCalData(PGET_SENSOR_CALIBRATION_DATA_STRUCT pGetCalData);
extern UINT32 MT9P015GetCalData(UINT32* pGetSensorCalData);


#define YUV_INFO(_id, name, getCalData)\
    {_id, name, \
    NSFeature::YUVSensorInfo<_id>::GetInstance, \
    NSFeature::YUVSensorInfo<_id>::GetDefaultData, \
    getCalData \
    }
#define RAW_INFO(_id, name, getCalData)\
    {_id, name, \
    NSFeature::RAWSensorInfo<_id>::GetInstance, \
    NSFeature::RAWSensorInfo<_id>::GetDefaultData, \
    getCalData \
    }


//MSDK_SENSOR_INIT_FUNCTION_STRUCT SensorList[MAX_NUM_OF_SUPPORT_SENSOR] =
MSDK_SENSOR_INIT_FUNCTION_STRUCT SensorList[] =
{
//8M
#if defined(IMX073_MIPI_RAW)
    RAW_INFO(IMX073_SENSOR_ID, SENSOR_DRVNAME_IMX073_MIPI_RAW,EEPROMGetCalData), 
#endif
//5M
#if defined(OV5642_RAW)
    RAW_INFO(OV5642_SENSOR_ID, SENSOR_DRVNAME_OV5642_RAW, NULL), 
#endif
#if defined(OV5642_YUV)
    YUV_INFO(OV5642_SENSOR_ID, SENSOR_DRVNAME_OV5642_YUV, NULL), 
#endif
#if defined(OV5647_RAW)
    RAW_INFO(OV5647_SENSOR_ID, SENSOR_DRVNAME_OV5647_RAW, NULL), 
#endif
#if defined(OV5650_RAW)
    YUV_INFO(OV5650_SENSOR_ID, SENSOR_DRVNAME_OV5650_RAW, NULL), 
#endif
#if defined(MT9P012_RAW)
    RAW_INFO(MT9P012_SENSOR_ID, SENSOR_DRVNAME_MT9P012_RAW, NULL), 
#endif
#if defined(MT9P015_RAW)
    RAW_INFO(MT9P015_SENSOR_ID, SENSOR_DRVNAME_MT9P015_RAW, MT9P015GetCalData), 
#endif
//3M
#if defined(OV3640_RAW)
    RAW_INFO(OV3640_SENSOR_ID, SENSOR_DRVNAME_OV3640_RAW, NULL), 
#endif
#if defined(OV3640_YUV)
    YUV_INFO(OV3640_SENSOR_ID, SENSOR_DRVNAME_OV3640_YUV, NULL), 
#endif
#if defined(HM3451_RAW)
    RAW_INFO(HM3451_SENSOR_ID, SENSOR_DRVNAME_HM3451_RAW,NULL), 
#endif
#if defined(MT9T112_YUV)
    YUV_INFO(MT9T112_SENSOR_ID, SENSOR_DRVNAME_MT9T112_YUV,NULL), 
#endif
#if defined(MT9T113_YUV)
    YUV_INFO(MT9T113_SENSOR_ID, SENSOR_DRVNAME_MT9T113_YUV,NULL), 
#endif
#if defined(HI342_RAW)
    RAW_INFO(HI342_SENSOR_ID, SENSOR_DRVNAME_HI342_RAW, NULL), 
#endif
#if defined(S5K5CAGX_YUV)
    YUV_INFO(S5K5CAGX_SENSOR_ID, SENSOR_DRVNAME_S5K5CAGX_YUV,NULL), 
#endif
//2M
#if defined(OV2650_RAW)
    RAW_INFO(OV2650_SENSOR_ID, SENSOR_DRVNAME_OV2650_RAW, NULL), 
#endif
#if defined(OV2655_YUV)
    YUV_INFO(OV2655_SENSOR_ID, SENSOR_DRVNAME_OV2655_YUV, NULL), 
#endif
#if defined(MT9D113_YUV)
    YUV_INFO(MT9D113_SENSOR_ID, SENSOR_DRVNAME_MT9D113_YUV,NULL), 
#endif
#if defined(MT9D115_YUV)
    YUV_INFO(MT9D115_SENSOR_ID, SENSOR_DRVNAME_MT9D115_YUV,NULL), 
#endif
#if defined(SID130B_YUV)
    YUV_INFO(SID130B_SENSOR_ID, SENSOR_DRVNAME_SID130B_YUV,NULL), 
#endif
#if defined(HI253_YUV)
    YUV_INFO(HI253_SENSOR_ID, SENSOR_DRVNAME_HI253_YUV,NULL), 
#endif
#if defined(GT2005_YUV)
    YUV_INFO(GT2005_SENSOR_ID, SENSOR_DRVNAME_GT2005_YUV,NULL), 
#endif
//VGA
#if defined(OV7675_YUV)
    YUV_INFO(OV7675_SENSOR_ID, SENSOR_DRVNAME_OV7675_YUV, NULL), 
#endif
#if defined(OV7690_YUV)
    YUV_INFO(OV7690_SENSOR_ID, SENSOR_DRVNAME_OV7690_YUV,NULL), 
#endif

#if defined(MT9V113_YUV)
    YUV_INFO(MT9V113_SENSOR_ID, SENSOR_DRVNAME_MT9V113_YUV, NULL), 
#endif
#if defined(MT9V114_YUV)
    YUV_INFO(MT9V114_SENSOR_ID, SENSOR_DRVNAME_MT9V114_YUV,NULL), 
#endif
#if defined(SIV120B_YUV)
    YUV_INFO(SIV120B_SENSOR_ID, SENSOR_DRVNAME_SIV120B_YUV, NULL), 
#endif
#if defined(GC0309_YUV)
    YUV_INFO(GC0309_SENSOR_ID, SENSOR_DRVNAME_GC0309_YUV,NULL), 
#endif
#if defined(HI704_YUV)
    YUV_INFO(HI704_SENSOR_ID, SENSOR_DRVNAME_HI704_YUV,NULL), 
#endif

/*  ADD sensor driver before this line */
    {0,{0},NULL, NULL, NULL}//end of list
};

UINT32 GetSensorInitFuncList(MSDK_SENSOR_INIT_FUNCTION_STRUCT **ppSensorList)
{
    if (NULL == ppSensorList) {
        LOGE("ERROR: NULL pSensorList\n");
        return MHAL_UNKNOWN_ERROR;
    }
    *ppSensorList = &SensorList[0];
	return MHAL_NO_ERROR;
} // GetSensorInitFuncList()






