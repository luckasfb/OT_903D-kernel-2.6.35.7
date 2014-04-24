
#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "MediaHal.h"
#include "src/lib/inc/MediaLog.h" //#include "src/lib/inc/MediaLog.h"
#include "camera_custom_nvram.h"

//eeprom
#include "eeprom.h"
#include "eeprom_define.h"
extern "C"{
#include "camera_custom_eeprom.h"
}
#include "camera_calibration_mt9p015.h"

#define TAG_LOG "CAMEEPROM"


#define DEBUG_CALIBRATION_LOAD
#define CUSTOM_EEPROM_ROTATION 1

#define FIRST_HIGH_BYTE_PER_WORD

#define EEPROM_SHOW_LOG 1

#ifdef EEPROM_SHOW_LOG
#define CAMEEPROM_LOG(fmt, arg...)    LOGD(fmt, ##arg)
#else
#define CAMEEPROM_LOG(fmt, arg...)    void(0)
#endif

////<
static UINT32 DoDefectLoad(INT32 epprom_fd, UINT32 start_addr, UINT32* pGetSensorCalData);
static UINT32 DoPregainLoad(INT32 epprom_fd, UINT32 start_addr, UINT32* pGetSensorCalData);
static UINT32 DoISPSlimShadingLoad(INT32 epprom_fd, UINT32 start_addr, UINT32* pGetSensorCalData);
static UINT32 DoISPDynamicShadingLoad(INT32 epprom_fd, UINT32 start_addr, UINT32* pGetSensorCalData);
static UINT32 DoISPFixShadingLoad(INT32 epprom_fd, UINT32 start_addr, UINT32* pGetSensorCalData);
static UINT32 DoISPSensorShadingLoad(INT32 epprom_fd, UINT32 start_addr, UINT32* pGetSensorCalData);
enum
{
#if 0
	CALIBRATION_LAYOUT_SLIM_LSC1 = 0,
	CALIBRATION_LAYOUT_SLIM_LSC2,
	CALIBRATION_LAYOUT_DYANMIC_LSC1,
	CALIBRATION_LAYOUT_DYANMIC_LSC2,
	CALIBRATION_LAYOUT_FIX_LSC1,
	CALIBRATION_LAYOUT_FIX_LSC2,
	CALIBRATION_LAYOUT_SENSOR_LSC1,
	CALIBRATION_LAYOUT_SENSOR_LSC2,
	CALIBRATION_LAYOUT_SUNNY_Q8N03D_LSC1,  //SL 110317
#else
	CALIBRATION_LAYOUT_SENSOR_LSC1,
#endif
	MAX_CALIBRATION_LAYOUT_NUM
};
 
enum
{
	CALIBRATION_ITEM_DEFECT = 0,
	CALIBRATION_ITEM_PREGAIN,
	CALIBRATION_ITEM_SHADING,
	MAX_CALIBRATION_ITEM_NUM	
};

UINT32 GetCalErr[MAX_CALIBRATION_ITEM_NUM] =
{
	EEPROM_ERR_NO_DEFECT,
	EEPROM_ERR_NO_PREGAIN,
	EEPROM_ERR_NO_SHADING,
};

typedef struct
{
	UINT16 Include; //calibration layout include this item?
	UINT32 StartAddr; // item Start Address
	UINT32 (*GetCalDataProcess)(INT32 epprom_fd, UINT32 start_addr, UINT32* pGetSensorCalData);
} CALIBRATION_ITEM_STRUCT;

typedef struct
{
	UINT32 HeaderAddr; //Header Address
	UINT32 HeaderId;   //Header ID
	UINT32 CheckShading; // Do check shading ID?
	UINT32 ShadingID;    // Shading ID
	CALIBRATION_ITEM_STRUCT CalItemTbl[MAX_CALIBRATION_ITEM_NUM];
} CALIBRATION_LAYOUT_STRUCT;

UINT8 gIsInited = 0;
const CALIBRATION_LAYOUT_STRUCT CalLayoutTbl[MAX_CALIBRATION_LAYOUT_NUM]=
{
#if 0
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
	},
	{//CALIBRATION_LAYOUT_SUNY
		0x00000000, 0x796e7573, 0x00000001,0x010200FF , 
		{		
			{0x00000000, 0x00000000, DoDefectLoad}, //CALIBRATION_ITEM_DEFECT
			{0x00000001, 0x00000004, DoPregainLoad}, //CALIBRATION_ITEM_PREGAIN
			{0x00000001, 0x0000000C, DoISPSlimShadingLoad}  //CALIBRATION_ITEM_SHADING of WCP1
		}
	}
#else
	{//CALIBRATION_LAYOUT_SENSOR_LSC1
		0x00000C00, 0xFFFFFFFF, 0x00000001, 0xFFFFFFFF, 
		{
			{0x00000000, 0x00000000, DoDefectLoad}, //CALIBRATION_ITEM_DEFECT
			{0x00000000, 0x00000000, DoPregainLoad}, //CALIBRATION_ITEM_PREGAIN
			{0x00000001, 0x00000C08, DoISPSensorShadingLoad}	//CALIBRATION_ITEM_SHADING
		}
	}	
#endif
};


static UINT32 DoDefectLoad(INT32 epprom_fd, UINT32 start_addr, UINT32* pGetSensorCalData)
{
    PEEPROM_DATA_STRUCT pEerom_data = (PEEPROM_DATA_STRUCT)pGetSensorCalData;
    UINT32 err= GetCalErr[CALIBRATION_ITEM_DEFECT];
    CAMEEPROM_LOG("DoDefectLoad, please remake it\n");
    if(pEerom_data->Defect.ISPComm.CommReg[EEPROM_INFO_IN_COMM_LOAD] ==CAL_DATA_LOAD)
    {		
      CAMEEPROM_LOG("ALREADY HAS UPDATED DEFECT TABLE");
	}
	else
    {
      CAMEEPROM_LOG("PLEASE PUT Defect Table here!");	
      err = 0;
	}
    return err;
}
static UINT32 DoPregainLoad(INT32 epprom_fd, UINT32 start_addr, UINT32* pGetSensorCalData)
{
	stEEPROM_INFO_STRUCT  eepromCfg;
	UINT32 PregainFactor, PregainOffset;
    UINT32 PregainFactorH, PregainOffsetH;
	UINT32 GainValue;
    PEEPROM_DATA_STRUCT pEerom_data = (PEEPROM_DATA_STRUCT)pGetSensorCalData;
    UINT32 err= GetCalErr[CALIBRATION_ITEM_PREGAIN];
    CAMEEPROM_LOG("DoPregainLoad(0x%8x)----\n",(unsigned int)pGetSensorCalData);
	eepromCfg.u4Offset = start_addr;
	eepromCfg.u4Length = 4;
	eepromCfg.pu1Params = (u8 *)&PregainFactor;
	ioctl(epprom_fd, EEPROMIOC_G_READ , &eepromCfg);
	eepromCfg.u4Offset = start_addr+4;
	eepromCfg.u4Length = 4;
	eepromCfg.pu1Params = (u8 *)&PregainOffset;
	ioctl(epprom_fd, EEPROMIOC_G_READ , &eepromCfg);
    PregainFactorH = ((PregainFactor>>16)&0xFFFF);
    PregainOffsetH = ((PregainOffset>>16)&0xFFFF);
	//pre gain
    pEerom_data->Pregain.rCalGainu4R = (((PregainFactor&0xFF)<<8)|((PregainFactor&0xFF00)>>8))*512/(((PregainOffset&0xFF)<<8)|((PregainOffset&0xFF00)>>8));
    pEerom_data->Pregain.rCalGainu4G = 512;
    pEerom_data->Pregain.rCalGainu4B  = (((PregainFactorH&0xFF)<<8)|((PregainFactorH&0xFF00)>>8))*512/(((PregainOffsetH&0xFF)<<8)|((PregainOffsetH&0xFF00)>>8));
#ifdef DEBUG_CALIBRATION_LOAD
    CAMEEPROM_LOG("======================AWB EEPROM==================\n");
    CAMEEPROM_LOG("[EEPROM PREGAIN VALUE] = %x\n", PregainFactor);
    CAMEEPROM_LOG("[EEPROM PREGAIN OFFSET] = %x\n", PregainOffset);
    CAMEEPROM_LOG("[rCalGain.u4R] = %d\n", pEerom_data->Pregain.rCalGainu4R);
    CAMEEPROM_LOG("[rCalGain.u4G] = %d\n",  pEerom_data->Pregain.rCalGainu4G);
    CAMEEPROM_LOG("[rCalGain.u4B] = %d\n", pEerom_data->Pregain.rCalGainu4B);	
    CAMEEPROM_LOG("======================AWB EEPROM==================\n");
#endif
	err=0;
	return err;
}


static UINT32 DoISPSlimShadingLoad(INT32 epprom_fd, UINT32 start_addr, UINT32* pGetSensorCalData)
{
    PEEPROM_DATA_STRUCT pEerom_data = (PEEPROM_DATA_STRUCT)pGetSensorCalData;
    stEEPROM_INFO_STRUCT eepromCfg;		
    MUINT32 u4capW,u4capH,u4PvW,u4PvH;
    UINT32 uiSlimShadingBuffer[EEPROM_LSC_DATA_SIZE_SLIM_LSC1]={0};
    UINT32 PreviewREG[5], CaptureREG[5];
    INT32 i4XPreGrid, i4YPreGrid, i4XCapGrid, i4YCapGrid;
    UINT16 i;
    INT32 err= GetCalErr[CALIBRATION_ITEM_SHADING];
    
    CAMEEPROM_LOG("DoISPSlimShadingLoad(0x%8x) \n",start_addr);	
    if(pEerom_data->Shading.ISPComm.CommReg[EEPROM_INFO_IN_COMM_LOAD] ==CAL_DATA_LOAD)
    {
        CAMEEPROM_LOG("NVRAM has EEPROM data!");     
    }
    else
    {
        eepromCfg.u4Offset = start_addr+0x44;
        eepromCfg.u4Length= EEPROM_LSC_DATA_SIZE_SLIM_LSC1*4;
        eepromCfg.pu1Params = (u8 *)uiSlimShadingBuffer;
        ioctl(epprom_fd, EEPROMIOC_G_READ , &eepromCfg);	
        //shading PV
        eepromCfg.u4Offset = start_addr+0x1C;
        eepromCfg.u4Length = 20;
        eepromCfg.pu1Params = (u8 *)&PreviewREG[0];
        ioctl(epprom_fd, EEPROMIOC_G_READ , &eepromCfg);	
        //shading Cap
        eepromCfg.u4Offset = start_addr+0x30;
        eepromCfg.u4Length = 20;
        eepromCfg.pu1Params = (u8 *)&CaptureREG[0];
        ioctl(epprom_fd, EEPROMIOC_G_READ , &eepromCfg);
	
        i4XPreGrid = ((PreviewREG[1]&0xF0000000) >> 28) + 2;
        i4YPreGrid = ((PreviewREG[1]&0x0000F000) >> 12) + 2;
        i4XCapGrid = ((CaptureREG[1]&0xF0000000) >> 28) + 2;
        i4YCapGrid = ((CaptureREG[1]&0x0000F000) >> 12) + 2;	  
			
        pEerom_data->Shading.CapRegister.shading_ctrl1 = CaptureREG[0];
        pEerom_data->Shading.CapRegister.shading_ctrl2 = CaptureREG[1];
        pEerom_data->Shading.CapRegister.shading_read_addr = CaptureREG[2];
        pEerom_data->Shading.CapRegister.shading_last_blk = CaptureREG[3];
        pEerom_data->Shading.CapRegister.shading_ratio_cfg = CaptureREG[4];
		
        pEerom_data->Shading.PreRegister.shading_ctrl1 = PreviewREG[0];
        pEerom_data->Shading.PreRegister.shading_ctrl2 = PreviewREG[1];
        pEerom_data->Shading.PreRegister.shading_read_addr = PreviewREG[2];
        pEerom_data->Shading.PreRegister.shading_last_blk = PreviewREG[3];
        pEerom_data->Shading.PreRegister.shading_ratio_cfg = PreviewREG[4];        

        // seanlin 110407 because SUNNY Module has wrong eeprom data about Preview register value
        //just using SUNNY module's block x number and y number and re-calculate the block size according to preview X/Y raw sizes

        u4PvW = pEerom_data->Shading.ISPComm.CommReg[EEPROM_INFO_IN_PV_WIDTH]/2;
        u4PvH = pEerom_data->Shading.ISPComm.CommReg[EEPROM_INFO_IN_PV_HEIGHT]/2;

        PreviewREG[1] = (PreviewREG[1] &0xf000f000)|(((u4PvW/(i4XPreGrid-1))&0xfff)<<16)|((u4PvH/(i4YPreGrid-1))&0xfff);
        PreviewREG[3] = (((u4PvW-((u4PvW/(i4XPreGrid-1))&0xfff)*(i4XPreGrid-2))&0xfff)<<16)|((u4PvH-((u4PvH/(i4YPreGrid-1))&0xfff)*(i4YPreGrid-2))&0xfff);
        pEerom_data->Shading.PreRegister.shading_ctrl2 = PreviewREG[1];
        pEerom_data->Shading.PreRegister.shading_last_blk = PreviewREG[3]; 
			   
        pEerom_data->Shading.CaptureSize = (i4XCapGrid-1)*(i4YCapGrid-1)*16;
        pEerom_data->Shading.PreviewSize = (i4XPreGrid-1)*(i4YPreGrid-1)*16;	
        CAMEEPROM_LOG("pEerom_data->Shading.CaptureSize 0x%x, pEerom_data->Shading.PreviewSize 0x%x \n",pEerom_data->Shading.CaptureSize,pEerom_data->Shading.PreviewSize);		
        memcpy((char*)&pEerom_data->Shading.CaptureTable[0][0], (char*)uiSlimShadingBuffer,EEPROM_LSC_DATA_SIZE_SLIM_LSC1*4);
        err =0;
	}
    return err;
}
static UINT32 DoISPDynamicShadingLoad(INT32 epprom_fd, UINT32 start_addr, UINT32* pGetSensorCalData)
{
    INT32 err= GetCalErr[CALIBRATION_ITEM_SHADING];
    //TBD
    CAMEEPROM_LOG("DoISPDynamicShadingLoad (NOT YET)\n");    
    return err;
}
static UINT32 DoISPFixShadingLoad(INT32 epprom_fd, UINT32 start_addr, UINT32* pGetSensorCalData)
{
    INT32 err= GetCalErr[CALIBRATION_ITEM_SHADING];
    CAMEEPROM_LOG("DoISPFixShadingLoad (NOT YET) \n");
    return err;
}
static UINT32 DoISPSensorShadingLoad(INT32 epprom_fd, UINT32 start_addr, UINT32* pGetSensorCalData)
{
    PEEPROM_DATA_STRUCT pEerom_data = (PEEPROM_DATA_STRUCT)pGetSensorCalData;
    stEEPROM_INFO_STRUCT eepromCfg;		
	UINT32 SensorLSCSize;
	UINT32 i;
#ifdef FIRST_HIGH_BYTE_PER_WORD
	UINT32 TempValue[4];
#endif
    INT32 err= GetCalErr[CALIBRATION_ITEM_SHADING];
    CAMEEPROM_LOG("DoISPSensorShadingLoad(0x%8x) \n",start_addr);	
    if(pEerom_data->Shading.ISPComm.CommReg[EEPROM_INFO_IN_COMM_LOAD] == CAL_DATA_LOAD)
    {		
        CAMEEPROM_LOG("NVRAM has EEPROM data!");     
    }
    else
    {
    
#if 0
		eepromCfg.u4Offset = start_addr+4;	
		eepromCfg.u4Length = 4; 
		eepromCfg.pu1Params = (u8 *)&SensorLSCSize; 
		ioctl(epprom_fd, EEPROMIOC_G_READ , &eepromCfg);
		CAMEEPROM_LOG("SensorLSCSize = %d \n", SensorLSCSize);
		if (SensorLSCSize > ((1024-8)/4))
		{		
			SensorLSCSize = ((1024-8)/4);	
		}
		eepromCfg.u4Offset = start_addr;
		eepromCfg.u4Length = 4+4+SensorLSCSize*4;
		eepromCfg.pu1Params = (u8 *)&pEerom_data->Shading.SensorCalTable[0];
		ioctl(epprom_fd, EEPROMIOC_G_READ , &eepromCfg);				
#else
    	SensorLSCSize = 102;
		((UINT32*)pEerom_data->Shading.SensorCalTable)[0] = (1<<16) | 1;
		((UINT32*)pEerom_data->Shading.SensorCalTable)[1] = SensorLSCSize;	
        eepromCfg.u4Offset = start_addr;
        eepromCfg.u4Length= SensorLSCSize*4;
        eepromCfg.pu1Params = (u8 *)&(((UINT32*)pEerom_data->Shading.SensorCalTable)[2]);
        ioctl(epprom_fd, EEPROMIOC_G_READ , &eepromCfg);			
#endif
#ifdef FIRST_HIGH_BYTE_PER_WORD
        i = 0;
        do	{
			TempValue[0]=((UINT32*)pEerom_data->Shading.SensorCalTable)[i+2]&0xFF;
			TempValue[1]=(((UINT32*)pEerom_data->Shading.SensorCalTable)[i+2]>>8)&0xFF;
			TempValue[2]=(((UINT32*)pEerom_data->Shading.SensorCalTable)[i+2]>>16)&0xFF;
			TempValue[3]=(((UINT32*)pEerom_data->Shading.SensorCalTable)[i+2]>>24)&0xFF;
			((UINT32*)pEerom_data->Shading.SensorCalTable)[i+2] = (TempValue[2]<<24) | (TempValue[3]<<16) | (TempValue[0]<<8) | TempValue[1];	
			i++;
        }while(i<SensorLSCSize);
#endif
#ifdef DEBUG_CALIBRATION_LOAD		// defect setting
        CAMEEPROM_LOG("Sensor Shading Table \n");
        i = 0;
		do	{
			CAMEEPROM_LOG("0x%08x, ", ((UINT32*)pEerom_data->Shading.SensorCalTable)[i]);
			if ((i+1) % 4 == 0)	
				CAMEEPROM_LOG("\n");
			i++;
		}while(i<(2+SensorLSCSize));
#endif
        err =0;		
    }
    return (err|EEPROM_ERR_SENSOR_SHADING);
}
UINT32 MT9P015GetCalData(UINT32* pGetSensorCalData)
{
    UCHAR cBuf[128] = "/dev/";
    UINT32 result = EEPROM_ERR_NO_DATA;
    //eeprom
    stEEPROM_INFO_STRUCT  eepromCfg;
    UINT32 CheckID,i ;
    INT32 err;
    UINT16 LayoutType;
    INT32 epprom_fd = 0;
    UINT16 u2IDMatch = 0;
	
    PEEPROM_DATA_STRUCT pEerom_data = (PEEPROM_DATA_STRUCT)pGetSensorCalData;
	
    if ((gIsInited==0) && (EEPROMInit() != EEPROM_NONE) && (EEPROMDeviceName(&cBuf[0]) == 0))
    {
        epprom_fd = open(cBuf, O_RDWR);	 
        if(epprom_fd == -1)
        {
            CAMEEPROM_LOG("----error: can't open EEPROM %s----\n",cBuf);
            result = GetCalErr[CALIBRATION_ITEM_SHADING]|GetCalErr[CALIBRATION_ITEM_PREGAIN]|GetCalErr[CALIBRATION_ITEM_DEFECT];
            return result;
        }	
        //result = 0;
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
                err= ioctl(epprom_fd, EEPROMIOC_G_READ , &eepromCfg);
                if(err< 0)
                {
                    CAMEEPROM_LOG("[EEPROM] Read header ID fail err = 0x%x \n",err);
                    break;
                }
            }
            if (((CalLayoutTbl[i].HeaderId == 0xFFFFFFFF) && ((CheckID != 0xFFFFFFFF)&&(CheckID != 0x000000)))
            	|| ((CalLayoutTbl[i].HeaderId != 0xFFFFFFFF) && (CheckID == CalLayoutTbl[i].HeaderId)))
            {
                LayoutType = i;
                u2IDMatch = 1;
                pEerom_data->Shading.ShadingID = CalLayoutTbl[i].ShadingID;
        		CAMEEPROM_LOG("ShadingID =  %d \n", pEerom_data->Shading.ShadingID);      
                #ifdef CUSTOM_EEPROM_ROTATION //seeanlin110421 because Sensor might rotated from PCB
                pEerom_data->Shading.TableRotation = 1;
                #endif                
                break;
            }			
        }
        CAMEEPROM_LOG("LayoutType= 0x%x",LayoutType);	

        if(u2IDMatch == 1)
        {     
            //result = 0;
            if (CalLayoutTbl[LayoutType].CheckShading != 0)
            {
                eepromCfg.u4Offset = CalLayoutTbl[LayoutType].CalItemTbl[CALIBRATION_ITEM_SHADING].StartAddr;
                eepromCfg.u4Length = 4;
                eepromCfg.pu1Params = (u8 *)&CheckID;
                ioctl(epprom_fd, EEPROMIOC_G_READ , &eepromCfg);
                
                CAMEEPROM_LOG("CheckID = 0x%x\n",CheckID);
                if (((CalLayoutTbl[LayoutType].ShadingID == 0xFFFFFFFF) && ((CheckID != 0xFFFFFFFF)&&(CheckID != 0x000000)))
                	|| ((CalLayoutTbl[LayoutType].ShadingID != 0xFFFFFFFF) && (CheckID == CalLayoutTbl[LayoutType].ShadingID)))
                {
                    result = 0;
                    for (i= 0; i < MAX_CALIBRATION_ITEM_NUM; i++)
                    {
                        if ((CalLayoutTbl[LayoutType].CalItemTbl[i].Include != 0) 
                        	&&(CalLayoutTbl[LayoutType].CalItemTbl[i].GetCalDataProcess != NULL))
                        {			
                            
                            result =  result | CalLayoutTbl[LayoutType].CalItemTbl[i].GetCalDataProcess(epprom_fd, CalLayoutTbl[LayoutType].CalItemTbl[i].StartAddr, pGetSensorCalData);	
                            
                        }
                        else
                        {
                            result = result| GetCalErr[i];
                            CAMEEPROM_LOG("result = 0x%x\n",result);							
                        }
                    }
                    gIsInited = 1;					
                }
            }
            else
            {
                CAMEEPROM_LOG("CalLayoutTbl[LayoutType].CheckShading==0");
            }
        }
		close(epprom_fd);		 
	}	
    else
    {
        CAMEEPROM_LOG("----NO EEPROM_%s!----\n",cBuf);
        result = GetCalErr[CALIBRATION_ITEM_SHADING]|GetCalErr[CALIBRATION_ITEM_PREGAIN]|GetCalErr[CALIBRATION_ITEM_DEFECT];
    }
	result = result | EEPROM_ERR_NOT_WRITENVRAM;
	return result;
}

