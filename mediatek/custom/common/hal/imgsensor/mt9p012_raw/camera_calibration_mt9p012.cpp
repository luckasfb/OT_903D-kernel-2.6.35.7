
#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#if defined(MT6516)
//Shading 
#include "mt6516_lscCalc.h"
#include "MediaHal.h"
#include "src/lib/inc/MediaLog.h"
#include "isp_hal.h"
//#include "msdk_nvram_camera_exp.h"
#include "camera_custom_nvram.h"
#include "camera_calibration_mt9p012.h"

//eeprom
#include "eeprom.h"
#include "eeprom_define.h"
extern "C"{
//#include "eeprom_layout.h"
#include "camera_custom_eeprom.h"
}

#define DEBUG_CALIBRATION_LOAD

extern UINT32 DoDefectLoad(INT32 epprom_fd, UINT32 start_addr, PGET_SENSOR_CALIBRATION_DATA_STRUCT pGetSensorCalData);
extern UINT32 DoPregainLoad(INT32 epprom_fd, UINT32 start_addr, PGET_SENSOR_CALIBRATION_DATA_STRUCT pGetSensorCalData);
extern UINT32 DoISPSlimShadingLoad(INT32 epprom_fd, UINT32 start_addr, PGET_SENSOR_CALIBRATION_DATA_STRUCT pGetSensorCalData);
extern UINT32 DoISPDynamicShadingLoad(INT32 epprom_fd, UINT32 start_addr, PGET_SENSOR_CALIBRATION_DATA_STRUCT pGetSensorCalData);
extern UINT32 DoISPFixShadingLoad(INT32 epprom_fd, UINT32 start_addr, PGET_SENSOR_CALIBRATION_DATA_STRUCT pGetSensorCalData);
extern UINT32 DoISPSensorShadingLoad(INT32 epprom_fd, UINT32 start_addr, PGET_SENSOR_CALIBRATION_DATA_STRUCT pGetSensorCalData);
enum
{
	CALIBRATION_LAYOUT_SLIM_LSC1 = 0,
	CALIBRATION_LAYOUT_SLIM_LSC2,
	CALIBRATION_LAYOUT_DYANMIC_LSC1,
	CALIBRATION_LAYOUT_DYANMIC_LSC2,
	CALIBRATION_LAYOUT_FIX_LSC1,
	CALIBRATION_LAYOUT_FIX_LSC2,
	CALIBRATION_LAYOUT_SENSOR_LSC1,
	CALIBRATION_LAYOUT_SENSOR_LSC2,
	MAX_CALIBRATION_LAYOUT_NUM
};
enum
{
	CALIBRATION_ITEM_DEFECT = 0,
	CALIBRATION_ITEM_PREGAIN,
	CALIBRATION_ITEM_SHADING,
	MAX_CALIBRATION_ITEM_NUM	
};
typedef struct
{
	UINT16 Include; //calibration layout include this item?
	UINT32 StartAddr; // item Start Address
	UINT32 (*GetCalDataProcess)(INT32 epprom_fd, UINT32 start_addr, PGET_SENSOR_CALIBRATION_DATA_STRUCT pGetSensorCalData);
} CALIBRATION_ITEM_STRUCT;

typedef struct
{
	UINT32 HeaderAddr; //Header Address
	UINT32 HeaderId;   //Header ID
	UINT32 CheckShading; // Do check shading ID?
	UINT32 ShadingID;    // Shading ID
	CALIBRATION_ITEM_STRUCT CalItemTbl[MAX_CALIBRATION_ITEM_NUM];
} CALIBRATION_LAYOUT_STRUCT;

const CALIBRATION_LAYOUT_STRUCT CalLayoutTbl[MAX_CALIBRATION_LAYOUT_NUM]=
{
	{//CALIBRATION_LAYOUT_SLIM_LSC1
		0x00000000, 0x010200FF, 0x00000001, 0x010200FF, 
		{
			{0x00000000, 0x00000000, DoDefectLoad}, //CALIBRATION_ITEM_DEFECT
			{0x00000001, 0x00000004, DoPregainLoad}, //CALIBRATION_ITEM_PREGAIN
			{0x00000001, 0x0000000C, DoISPSlimShadingLoad}  //CALIBRATION_ITEM_SHADING
		}
	},
	{//CALIBRATION_LAYOUT_SLIM_LSC2
		0x00000000, 0x010300FF, 0x00000001, 0x010200FF, 
		{
			{0x00000001, 0x00000004, DoDefectLoad}, //CALIBRATION_ITEM_DEFECT
			{0x00000001, 0x00000BF8, DoPregainLoad}, //CALIBRATION_ITEM_PREGAIN
			{0x00000001, 0x00000C00, DoISPSlimShadingLoad}  //CALIBRATION_ITEM_SHADING
		}
	},
	{//CALIBRATION_LAYOUT_DYANMIC_LSC1
		0x00000000, 0x010400FF, 0x00000001, 0x31520000, 
		{
			{0x00000000, 0x00000000, DoDefectLoad}, //CALIBRATION_ITEM_DEFECT
			{0x00000001, 0x00000004, DoPregainLoad}, //CALIBRATION_ITEM_PREGAIN
			{0x00000001, 0x0000000C, DoISPDynamicShadingLoad}  //CALIBRATION_ITEM_SHADING
		}
	},
	{//CALIBRATION_LAYOUT_DYANMIC_LSC2
		0x00000000, 0x010500FF, 0x00000001, 0x31520000, 
		{
			{0x00000001, 0x00000004, DoDefectLoad}, //CALIBRATION_ITEM_DEFECT
			{0x00000001, 0x00000BF8, DoPregainLoad}, //CALIBRATION_ITEM_PREGAIN
			{0x00000001, 0x00000C00, DoISPDynamicShadingLoad}	//CALIBRATION_ITEM_SHADING
		}
	},
	{//CALIBRATION_LAYOUT_FIX_LSC1
		0x00000000, 0x010600FF, 0x00000001, 0x39333236, 
		{
			{0x00000000, 0x00000000, DoDefectLoad}, //CALIBRATION_ITEM_DEFECT
			{0x00000001, 0x00000004, DoPregainLoad}, //CALIBRATION_ITEM_PREGAIN
			{0x00000001, 0x0000000C, DoISPFixShadingLoad}	//CALIBRATION_ITEM_SHADING
		}
	},
	{//CALIBRATION_LAYOUT_FIX_LSC2
		0x00000000, 0x010700FF, 0x00000001, 0x39333236, 
		{
			{0x00000001, 0x00000004, DoDefectLoad}, //CALIBRATION_ITEM_DEFECT
			{0x00000001, 0x00000BF8, DoPregainLoad}, //CALIBRATION_ITEM_PREGAIN
			{0x00000001, 0x00000C00, DoISPFixShadingLoad}	//CALIBRATION_ITEM_SHADING
		}
	},
	{//CALIBRATION_LAYOUT_SENSOR_LSC1
		0x00000000, 0x010800FF, 0x00000001, 0xFFFFFFFF, 
		{
			{0x00000000, 0x00000000, DoDefectLoad}, //CALIBRATION_ITEM_DEFECT
			{0x00000001, 0x00000004, DoPregainLoad}, //CALIBRATION_ITEM_PREGAIN
			{0x00000001, 0x0000000C, DoISPSensorShadingLoad}	//CALIBRATION_ITEM_SHADING
		}
	},
	{//CALIBRATION_LAYOUT_SENSOR_LSC2
		0x00000000, 0x010900FF, 0x00000001, 0xFFFFFFFF, 
		{
			{0x00000001, 0x00000004, DoDefectLoad}, //CALIBRATION_ITEM_DEFECT
			{0x00000001, 0x00000BF8, DoPregainLoad}, //CALIBRATION_ITEM_PREGAIN
			{0x00000001, 0x00000C00, DoISPSensorShadingLoad}	//CALIBRATION_ITEM_SHADING
		}
	}
};

UINT32 DoDefectLoad(INT32 epprom_fd, UINT32 start_addr, PGET_SENSOR_CALIBRATION_DATA_STRUCT pGetSensorCalData)
{
	stEEPROM_INFO_STRUCT  eepromCfg;
	UINT32 i;
	
	MHAL_LOG("DoDefectLoad \n");
	
	eepromCfg.u4Offset = start_addr+4;
	eepromCfg.u4Length = 1020;
	eepromCfg.pu1Params = (u8 *)&(pGetSensorCalData->pCameraDefect->Defect.PreviewTable[0]);
	ioctl(epprom_fd, EEPROMIOC_G_READ , &eepromCfg);
	eepromCfg.u4Offset = start_addr+4+1020+4;
	eepromCfg.u4Length = 2032;
	eepromCfg.pu1Params = (u8 *)&(pGetSensorCalData->pCameraDefect->Defect.CaptureTable1[0]);
	ioctl(epprom_fd, EEPROMIOC_G_READ , &eepromCfg);
	if((pGetSensorCalData->pCameraDefect->Defect.CaptureTable1[0]!=0xFFFFFFFF) ||((pGetSensorCalData->pCameraDefect->Defect.PreviewTable[0]!=0xFFFFFFFF)))
	{//enable table defect
		pGetSensorCalData->pCameraDefect->Defect.PreviewSize = 1; //set to none-zero value, h/w didn't need this value
		pGetSensorCalData->pCameraDefect->Defect.CaptureSize = 1;
		pGetSensorCalData->pCameraPara->ISPComm.CommReg[CAM_DEFECT0_ISPCOMM_INDEX] = 0x01040000; 			  
	}
	else
	{//disable table defect
		pGetSensorCalData->pCameraDefect->Defect.PreviewSize = 0;
		pGetSensorCalData->pCameraDefect->Defect.CaptureSize = 0;
		pGetSensorCalData->pCameraPara->ISPComm.CommReg[CAM_DEFECT0_ISPCOMM_INDEX] = 0x00040000; 			  
	}
#ifdef DEBUG_CALIBRATION_LOAD
	// defect setting
	MHAL_LOG("Preveiw : defect	Table \n");
	i = 0;
	do
	{
		 MHAL_LOG("0x%08x, ", pGetSensorCalData->pCameraDefect->Defect.PreviewTable[i]);
		 if ((i+1) % 4 == 0)
			MHAL_LOG("\n");
		 i++;
	}while((pGetSensorCalData->pCameraDefect->Defect.PreviewTable[i-1] !=0xFFFFFFFF)&&(i<1020/4));

	MHAL_LOG("capture : defect	Table \n");
	i=0;
	do
	{
		 MHAL_LOG("0x%08x, ",pGetSensorCalData->pCameraDefect->Defect.CaptureTable1[i]);
		 if ((i+1) % 4 == 0)
			MHAL_LOG("\n");
		 i++;
	}while((pGetSensorCalData->pCameraDefect->Defect.CaptureTable1[i-1]!=0xFFFFFFFF)&&(i<1020/4));
	MHAL_LOG("\n");
	MHAL_LOG("Defect Preview size = 0x%08x, ", pGetSensorCalData->pCameraDefect->Defect.PreviewSize);
	MHAL_LOG("Defect Capture size = 0x%08x, ", pGetSensorCalData->pCameraDefect->Defect.CaptureSize);
	MHAL_LOG("Defect Register setting = 0x%08x, ", pGetSensorCalData->pCameraPara->ISPComm.CommReg[CAM_DEFECT0_ISPCOMM_INDEX]);
#endif
	return CAL_GET_DEFECT_FLAG|CAL_GET_PARA_FLAG|CAL_GET_PARA_FLAG;
}
UINT32 DoPregainLoad(INT32 epprom_fd, UINT32 start_addr, PGET_SENSOR_CALIBRATION_DATA_STRUCT pGetSensorCalData)
{
	stEEPROM_INFO_STRUCT  eepromCfg;
	UINT32 PregainFactor, PregainOffset;
	UINT32 GainValue;

	MHAL_LOG("DoPregainLoad \n");

	eepromCfg.u4Offset = start_addr;
	eepromCfg.u4Length = 4;
	eepromCfg.pu1Params = (u8 *)&PregainFactor;
	ioctl(epprom_fd, EEPROMIOC_G_READ , &eepromCfg);
	eepromCfg.u4Offset = start_addr+4;
	eepromCfg.u4Length = 4;
	eepromCfg.pu1Params = (u8 *)&PregainOffset;
	ioctl(epprom_fd, EEPROMIOC_G_READ , &eepromCfg);

	//pre gain
	pGetSensorCalData->pCamera3ANVRAMData->rAWBNVRAM.rCalData.rCalGain.u4R = (PregainFactor&0xFFFF)*512/(PregainOffset&0xFFFF);
	pGetSensorCalData->pCamera3ANVRAMData->rAWBNVRAM.rCalData.rCalGain.u4G = 512;
	pGetSensorCalData->pCamera3ANVRAMData->rAWBNVRAM.rCalData.rCalGain.u4B = ((PregainFactor>>16)&0xFFFF)*512/((PregainOffset>>16)&0xFFFF);

#ifdef DEBUG_CALIBRATION_LOAD
	MHAL_LOG("======================AWB EEPROM==================\n");
	MHAL_LOG("[EEPROM PREGAIN VALUE] = %d\n", PregainFactor);
	MHAL_LOG("[EEPROM PREGAIN OFFSET] = %d\n", PregainOffset);
	MHAL_LOG("[rCalGain.u4R] = %d\n", pGetSensorCalData->pCamera3ANVRAMData->rAWBNVRAM.rCalData.rCalGain.u4R);
	MHAL_LOG("[rCalGain.u4G] = %d\n", pGetSensorCalData->pCamera3ANVRAMData->rAWBNVRAM.rCalData.rCalGain.u4G);
	MHAL_LOG("[rCalGain.u4B] = %d\n", pGetSensorCalData->pCamera3ANVRAMData->rAWBNVRAM.rCalData.rCalGain.u4B);
	MHAL_LOG("======================AWB EEPROM==================\n");
#endif

	return CAL_GET_3ANVRAM_FLAG;
}
extern UINT32 DoISPSlimShadingLoad(INT32 epprom_fd, UINT32 start_addr, PGET_SENSOR_CALIBRATION_DATA_STRUCT pGetSensorCalData)
{
	//TBD
	MHAL_LOG("DoISPSlimShadingLoad \n");
	return 0;
}
UINT32 DoISPDynamicShadingLoad(INT32 epprom_fd, UINT32 start_addr, PGET_SENSOR_CALIBRATION_DATA_STRUCT pGetSensorCalData)
{
	//TBD
	MHAL_LOG("DoISPDynamicShadingLoad \n");
	return 0;
}
UINT32 DoISPFixShadingLoad(INT32 epprom_fd, UINT32 start_addr, PGET_SENSOR_CALIBRATION_DATA_STRUCT pGetSensorCalData)
{
	stEEPROM_INFO_STRUCT  eepromCfg;
	UINT32 i;
	INT32 i4XPreGrid, i4YPreGrid;
	INT32 i4XCapGrid, i4YCapGrid;
	UINT32 PreviewREG[5];
	UINT32 CaptureREG[5];

	MHAL_LOG("DoISPFixShadingLoad \n");
		
	//shading
	eepromCfg.u4Offset = start_addr+28;
	eepromCfg.u4Length = 20;
	eepromCfg.pu1Params = (u8 *)&PreviewREG;
	ioctl(epprom_fd, EEPROMIOC_G_READ , &eepromCfg);	
	//shading
	eepromCfg.u4Offset = start_addr+28+20;
	eepromCfg.u4Length = 20;
	eepromCfg.pu1Params = (u8 *)&CaptureREG;
	ioctl(epprom_fd, EEPROMIOC_G_READ , &eepromCfg);
	
	i4XPreGrid = ((PreviewREG[1]&0xF0000000) >> 28) + 2;
	i4YPreGrid = ((PreviewREG[1]&0x0000F000) >> 12) + 2;
	i4XCapGrid = ((CaptureREG[1]&0xF0000000) >> 28) + 2;
	i4YCapGrid = ((CaptureREG[1]&0x0000F000) >> 12) + 2;	  
			
	if ((i4XPreGrid*i4YPreGrid <= MAX_Pre_XGrid*MAX_Pre_YGrid) && (i4XCapGrid*i4YCapGrid<=	MAX_Cap_XGrid*MAX_Cap_YGrid))
	{
		for (i = 0 ; i < 5; i++)
		{
			 pGetSensorCalData->pCameraPara->ISPTuning.ShadingReg[0][6+i] = PreviewREG[i];
			 pGetSensorCalData->pCameraPara->ISPTuning.ShadingReg[1][6+i] = CaptureREG[i];
		} 
		eepromCfg.u4Offset = start_addr+20;
		eepromCfg.u4Length = 4;
		eepromCfg.pu1Params = (u8 *)&pGetSensorCalData->pCameraShading->Shading.PreviewSize;
		ioctl(epprom_fd, EEPROMIOC_G_READ , &eepromCfg);	
		//shading
		eepromCfg.u4Offset = start_addr+20+4;
		eepromCfg.u4Length = 4;
		eepromCfg.pu1Params = (u8 *)&pGetSensorCalData->pCameraShading->Shading.CaptureSize;
		ioctl(epprom_fd, EEPROMIOC_G_READ , &eepromCfg);
		
		eepromCfg.u4Offset = start_addr+28+40;
		eepromCfg.u4Length = 320*4;
		eepromCfg.pu1Params = (u8 *)&pGetSensorCalData->pCameraShading->Shading.PreviewTable[0][0];
		ioctl(epprom_fd, EEPROMIOC_G_READ , &eepromCfg);	
		memcpy((UINT8 *)pGetSensorCalData->pCameraShading->Shading.PreviewTable[1], (UINT8 *)pGetSensorCalData->pCameraShading->Shading.PreviewTable[0], 320*4);
		memcpy((UINT8 *)pGetSensorCalData->pCameraShading->Shading.PreviewTable[2], (UINT8 *)pGetSensorCalData->pCameraShading->Shading.PreviewTable[0], 320*4);
		eepromCfg.u4Offset = start_addr+28+40+320*4;
		eepromCfg.u4Length = 896*4;
		eepromCfg.pu1Params = (u8 *)&pGetSensorCalData->pCameraShading->Shading.CaptureTable[0][0];
		ioctl(epprom_fd, EEPROMIOC_G_READ , &eepromCfg);	
		memcpy((UINT8 *)pGetSensorCalData->pCameraShading->Shading.CaptureTable[1], (UINT8 *)pGetSensorCalData->pCameraShading->Shading.CaptureTable[0], 896*4);
		memcpy((UINT8 *)pGetSensorCalData->pCameraShading->Shading.CaptureTable[2], (UINT8 *)pGetSensorCalData->pCameraShading->Shading.CaptureTable[0], 896*4);

#ifdef DEBUG_CALIBRATION_LOAD
		// defect setting
		MHAL_LOG("fix Shading preview Table \n");
		i = 0;
		do
		{
//			 MHAL_LOG("0x%08x, ", pGetSensorCalData->pCameraShading->Shading.PreviewTable[0][i]);
			 if ((i+1) % 4 == 0)
				MHAL_LOG("\n");
			 i++;
		}while(i<320);
		MHAL_LOG("fix Shading preview register \n");
		i = 0;
		do
		{
//			 MHAL_LOG("0x%08x, ", pGetSensorCalData->pCameraPara->ISPTuning.ShadingReg[0][i]);
			 if ((i+1) % 4 == 0)
				MHAL_LOG("\n");
			 i++;
		}while(i<11);

		MHAL_LOG("fix Shading capture Table \n");
		i = 0;
		do
		{
//			 MHAL_LOG("0x%08x, ", pGetSensorCalData->pCameraShading->Shading.CaptureTable[0][i]);
			 if ((i+1) % 4 == 0)
				MHAL_LOG("\n");
			 i++;
		}while(i<896);
		MHAL_LOG("fix Shading capture register \n");
		i = 0;
		do
		{
//			 MHAL_LOG("0x%08x, ", pGetSensorCalData->pCameraPara->ISPTuning.ShadingReg[1][i]);
			 if ((i+1) % 4 == 0)
				MHAL_LOG("\n");
			 i++;
		}while(i<11);

#endif
			   
		pGetSensorCalData->pCameraPara->ISPComm.CommReg[33] = CAL_SHADING_TYPE_ISP;
		return CAL_GET_SHADING_FLAG|CAL_GET_PARA_FLAG;
	}

	return 0;
}
UINT32 DoISPSensorShadingLoad(INT32 epprom_fd, UINT32 start_addr, PGET_SENSOR_CALIBRATION_DATA_STRUCT pGetSensorCalData)
{
	stEEPROM_INFO_STRUCT  eepromCfg;
	UINT32 SensorLSCSize;
	UINT32 i;

	MHAL_LOG("DoISPSensorShadingLoad \n");

	//shading
	eepromCfg.u4Offset = start_addr+4;
	eepromCfg.u4Length = 4;
	eepromCfg.pu1Params = (u8 *)&SensorLSCSize;
	ioctl(epprom_fd, EEPROMIOC_G_READ , &eepromCfg);
	MHAL_LOG("SensorLSCSize = %d \n", SensorLSCSize);
	
	if (SensorLSCSize > ((1024-8)/4))
	{
		SensorLSCSize = ((1024-8)/4);
	}
	eepromCfg.u4Offset = start_addr;
	eepromCfg.u4Length = 4+4+SensorLSCSize*4;
	eepromCfg.pu1Params = (u8 *)&pGetSensorCalData->pCameraShading->Shading.SensorCalTable[0];
	ioctl(epprom_fd, EEPROMIOC_G_READ , &eepromCfg);	

#ifdef DEBUG_CALIBRATION_LOAD
		// defect setting
		MHAL_LOG("Sensor Shading Table \n");
		i = 0;
		do
		{
			 MHAL_LOG("0x%08x, ", ((UINT32*)pGetSensorCalData->pCameraShading->Shading.SensorCalTable)[i]);
			 if ((i+1) % 4 == 0)
				MHAL_LOG("\n");
			 i++;
		}while(i<(2+SensorLSCSize));
#endif

	pGetSensorCalData->pCameraPara->ISPComm.CommReg[33] = CAL_SHADING_TYPE_SENSOR;
	return CAL_GET_SHADING_FLAG|CAL_GET_PARA_FLAG;
}

UINT8 gIsInited = 0;
UINT32 MT9P012GetCalData(PGET_SENSOR_CALIBRATION_DATA_STRUCT pGetSensorCalData)
{
	UCHAR cBuf[128] = "/dev/";
	
	UINT32 result = 0;
	//eeprom
	INT32 epprom_fd = 0;
	stEEPROM_INFO_STRUCT  eepromCfg;
	UINT32 CheckID;
	UINT16 LayoutType;
	UINT16 i;
	UINT16 u2IDMatch = 0;
	
	MHAL_LOG("MT9P012GetCalData gIsInited=%d\n",gIsInited);
	
	if (((gIsInited==0) || (pGetSensorCalData->pCameraPara->ISPComm.CommReg[34] != CAL_DATA_LOAD)) && (EEPROMInit() != EEPROM_NONE) && (EEPROMDeviceName(&cBuf[0]) == 0))
	{
		MHAL_LOG("MT9P012 get calibration data \r\n");
		MHAL_LOG("MT9P012 get device = %s \r\n", cBuf);
		epprom_fd = open(cBuf, O_RDWR);	 
		if(epprom_fd == -1)
		{
			MHAL_LOG("----error: can't open EEPROM_S24CS64A----\n");
			return 0;
		}	 

		//read ID
		LayoutType = 0xFFFF; 
		eepromCfg.u4Offset = 0xFFFFFFFF;
		for (i = 0; i< MAX_CALIBRATION_LAYOUT_NUM; i++)
		{
			if (eepromCfg.u4Offset != CalLayoutTbl[i].HeaderAddr)
			{
				CheckID = 0x00000000;
				eepromCfg.u4Offset = CalLayoutTbl[i].HeaderAddr;
				eepromCfg.u4Length = 4;
				eepromCfg.pu1Params = (u8 *)&CheckID;
				if(ioctl(epprom_fd, EEPROMIOC_G_READ , &eepromCfg)< 0)
				{
					MHAL_LOG("[EEPROM] Read header ID fail \n");
					break;
				}
			}
			if (((CalLayoutTbl[i].HeaderId == 0xFFFFFFFF) && ((CheckID != 0xFFFFFFFF)&&(CheckID != 0x000000)))
				|| ((CalLayoutTbl[i].HeaderId != 0xFFFFFFFF) && (CheckID == CalLayoutTbl[i].HeaderId)))
			{
				LayoutType = i;
				u2IDMatch = 1;
				break;
			}
		}
		if (u2IDMatch == 1)
		{
			MHAL_LOG("[EEPROM] Get Layout type = %d \n", LayoutType);
			if (CalLayoutTbl[LayoutType].CheckShading != 0)
			{
				eepromCfg.u4Offset = CalLayoutTbl[LayoutType].CalItemTbl[CALIBRATION_ITEM_SHADING].StartAddr;
				eepromCfg.u4Length = 4;
				eepromCfg.pu1Params = (u8 *)&CheckID;
				ioctl(epprom_fd, EEPROMIOC_G_READ , &eepromCfg);
				if (((CalLayoutTbl[i].ShadingID == 0xFFFFFFFF) && ((CheckID != 0xFFFFFFFF)&&(CheckID != 0x000000)))
					|| ((CalLayoutTbl[i].ShadingID != 0xFFFFFFFF) && (CheckID == CalLayoutTbl[i].ShadingID)))
				{
					MHAL_LOG("[EEPROM] Check shading successful.\n");
					for (i= 0; i < MAX_CALIBRATION_ITEM_NUM; i++)
					{
						if ((CalLayoutTbl[LayoutType].CalItemTbl[i].Include != 0) 
							&&(CalLayoutTbl[LayoutType].CalItemTbl[i].GetCalDataProcess != NULL))
						{
							result =  result | CalLayoutTbl[LayoutType].CalItemTbl[i].GetCalDataProcess(epprom_fd, CalLayoutTbl[LayoutType].CalItemTbl[i].StartAddr, pGetSensorCalData);
						}
					}
					gIsInited = 1;
					pGetSensorCalData->pCameraPara->ISPComm.CommReg[34] = CAL_DATA_LOAD;
					result = result | CAL_GET_PARA_FLAG;
				}
			}
		}

		close(epprom_fd);		 
	}	

	return result;
}

#endif  //  MT6516

#if defined(MT6573)

#include "MediaHal.h"
#include "camera_custom_nvram.h"
#include "camera_calibration_mt9p012.h"

UINT32 MT9P012GetCalData(PGET_SENSOR_CALIBRATION_DATA_STRUCT pGetSensorCalData)
{
    return  0;    
}
#endif  //  MT6573
