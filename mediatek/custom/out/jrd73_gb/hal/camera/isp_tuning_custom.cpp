
#ifndef MT6516
//
#define LOG_TAG "IspTuningCustom"
#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (0)
#endif
//
#include <utils/Errors.h>
#include <cutils/log.h>
//
#define USE_CUSTOM_ISP_TUNING
#include "isp_tuning.h"
//
using namespace NSIspTuning;
//


MVOID
IspTuningCustom::
evaluate_nvram_index(RAWIspCamInfo const& rCamInfo, IndexMgr& rIdxMgr)
{
    MBOOL fgRet = MFALSE;
    ECamMode_T const       eCamMode = rCamInfo.eCamMode;
    EIndex_Scene_T const eIdx_Scene = rCamInfo.eIdx_Scene;
    EIndex_ISO_T const     eIdx_ISO = rCamInfo.eIdx_ISO;
    MUINT32 const        u4ISOValue = rCamInfo.u4ISOValue;
    MUINT32 const             i4CCT = rCamInfo.i4CCT;
    MUINT32 const  u4ZoomRatio_x100 = rCamInfo.u4ZoomRatio_x100;
    MINT32 const   i4LightValue_x10 = rCamInfo.i4LightValue_x10;

    //  (0) We have:
    //      eCamMode, eScene, ......
//..............................................................................
    //  (1) Dump info. before customizing.
#if ENABLE_MY_LOG
    rCamInfo.dump();
#endif

#if 0
    LOGD("[+evaluate_nvram_index][before customizing]");
    rIdxMgr.dump();
#endif
//..............................................................................
    //  (2) Modify each index based on conditions.
    //
    //  setIdx_XXX() returns:
    //      MTURE: if successful
    //      MFALSE: if the input index is out of range.
    //
#if 0
    fgRet = rIdxMgr.setIdx_DM(XXX);
    fgRet = rIdxMgr.setIdx_DP(XXX);
    fgRet = rIdxMgr.setIdx_NR1(XXX);
    fgRet = rIdxMgr.setIdx_NR2(XXX);
    fgRet = rIdxMgr.setIdx_Saturation(XXX);
    fgRet = rIdxMgr.setIdx_Contrast(XXX);
    fgRet = rIdxMgr.setIdx_Hue(XXX);
    fgRet = rIdxMgr.setIdx_Gamma(XXX);
    fgRet = rIdxMgr.setIdx_EE(XXX);
#endif
//..............................................................................
    //  (3) Finally, dump info. after modifying.
#if 0
    LOGD("[-evaluate_nvram_index][after customizing]");
    rIdxMgr.dump();
#endif
}


MVOID
IspTuningCustom::
refine_NR1(RAWIspCamInfo const& rCamInfo, ISP_NVRAM_NR1_T& rNR1)
{
    //  (1) Check to see if it works or not.
    switch  (rCamInfo.eCamMode)
    {
    //  Normal
    case ECamMode_Online_Preview:
    case ECamMode_Online_Capture:
    case ECamMode_Offline_Capture_Pass1:
    //   TODO: Add your code below...

        break;

    //  HDR
    case ECamMode_HDR_Cap_Pass1_SF:
    //   TODO: Add your code below...

        break;

    case ECamMode_HDR_Cap_Pass1_MF1:
        rNR1.ctrl.bits.NR_EN = 0;
        break;

//  case ECamMode_Offline_Capture_Pass2:
//  case ECamMode_HDR_Cap_Pass1_MF2:
//  case ECamMode_HDR_Cap_Pass2:
    default:
    //  Usually, NR1 is disabled in capture pass2.
    //  Of course, you can do what you want.
#if 1
        ::memset(rNR1.set, 0, sizeof(ISP_NVRAM_NR1_T));
#endif
        break;
    }
}


MVOID
IspTuningCustom::
refine_NR2(RAWIspCamInfo const& rCamInfo, ISP_NVRAM_NR2_T& rNR2)
{
    //  (1) Check to see if it works or not.
    if  ( ECamMode_Offline_Capture_Pass1 == rCamInfo.eCamMode )
    {   //  Usually, NR2 is disabled in capture pass1.
        //  Of course, you can do what you want.
#if 1
        ::memset(rNR2.set, 0, sizeof(ISP_NVRAM_NR2_T));
#endif
        return;
    }

    //  (2) HDR Mode.
    if  ( ECamMode_HDR_Cap_Pass2 == rCamInfo.eCamMode )
    {   //  TODO: Add your code below...

        return;
    }

    //  (3) TODO: Add your code below...
}


MVOID
IspTuningCustom::
refine_DM(RAWIspCamInfo const& rCamInfo, ISP_NVRAM_DEMOSAIC_T& rDM)
{
    //  (1) Check to see if it works or not.
    if  ( ECamMode_Offline_Capture_Pass1 == rCamInfo.eCamMode )
        return; //  It does not work in capture pass1.

    //  (2) TODO: Add your code below...
}


MVOID
IspTuningCustom::
refine_EE(RAWIspCamInfo const& rCamInfo, ISP_NVRAM_EE_T& rEE)
{
    //  (1) Check to see if it works or not.
    if  ( ECamMode_Offline_Capture_Pass1 == rCamInfo.eCamMode )
        return; //  It does not work in capture pass1.

    //  (2) HDR Mode.
    if  ( ECamMode_HDR_Cap_Pass2 == rCamInfo.eCamMode )
    {   //  TODO: Add your code below...

        return;
    }

    //  (3) TODO: Add your code below...
}


MVOID
IspTuningCustom::
refine_CCM(RAWIspCamInfo const& rCamInfo, ISP_NVRAM_CCM_T& rCCM)
{
    //  (1) Check to see if it works or not.
    if  ( ECamMode_Offline_Capture_Pass1 == rCamInfo.eCamMode )
        return; //  It does not work in capture pass1.

    //  (2) TODO: Add your code below...
    if  ( ECamMode_Online_Preview == rCamInfo.eCamMode || 
    				ECamMode_Online_Capture == rCamInfo.eCamMode ||
    				ECamMode_Offline_Capture_Pass2 == rCamInfo.eCamMode)
    {
    #if	0
        switch  (rCamInfo.eIdx_ISO)
        {
        case eIDX_ISO_100:    	
        case eIDX_ISO_200:     	
        case eIDX_ISO_400:
        case eIDX_ISO_800:
        case eIDX_ISO_1600:
        default:
        	if(rCamInfo.eIdx_CCM_CCT==eIDX_CCM_CCT_TL84)
        	{
            	rCCM.set[0] = 0x002C8686;
            	rCCM.set[1] = 0x008A2A80;
            	rCCM.set[2] = 0x00818A2B;
          	}
          	else if(rCamInfo.eIdx_CCM_CCT==eIDX_CCM_CCT_CWF)
          	{
            	rCCM.set[0] = 0x00389286;
            	rCCM.set[1] = 0x00892801;
            	rCCM.set[2] = 0x00818B2C;          		
          	}
          	else
          	{
            	rCCM.set[0] = 0x00318889;
            	rCCM.set[1] = 0x00862F89;
            	rCCM.set[2] = 0x00018423;                 		
          	}
            break;
        }
	#else
		int	CCM_Out[3][3],CCM[3][3];
		int	iTemp,x,y,temp[3];		
		float fRatio,fDSaturate=0.6;
		int	iLowISO=100,iHighISO=640;

		if(rCamInfo.u4ISOValue<iLowISO)
		{
			fRatio	= 1;
		}
		else if(rCamInfo.u4ISOValue>iHighISO)
		{
			fRatio	= fDSaturate;
		}
		else
		{
			fRatio	= 1+(fDSaturate-1)*(rCamInfo.u4ISOValue-iLowISO)/(iHighISO-iLowISO);
		}
		for (y=0; y<3; y++)
		{
			CCM[0][y]=	(rCCM.set[y]&0xff0000)>>16;
			CCM[1][y]=	(rCCM.set[y]&0xff00)>>8;
			CCM[2][y]=	(rCCM.set[y]&0xff);
			for (x=0; x<2; x++)
			{
				int nOrg = CCM[x][y];

				if (x == y)
				{			
					iTemp = 0.5 + (nOrg-32) * fRatio + 32;
				}
				else
				{
					if (nOrg >= 128)
					{
						iTemp = 0.5 + 128 - (128 - nOrg) * fRatio;
					}
					else
					{
						iTemp = 0.5 + nOrg * fRatio;
					}
				}

				if (iTemp == 128)
				{
					iTemp = 0;
				}

				CCM_Out[x][y]=iTemp;
				if(iTemp>=128)
				{
					temp[x]=128-iTemp;
				}
				else
				{
					temp[x]=iTemp;
				}
			}
			temp[2]=32-temp[1]-temp[0];
			if(temp[2]<0)
				CCM_Out[2][y]=128-temp[2];
			else
				CCM_Out[2][y]=temp[2];
			//printf("%d\t%d\t%d\n",CCM_Out[x][0],CCM_Out[x][1],CCM_Out[x][2]);
			rCCM.set[y]	= (CCM_Out[0][y]<<16)|(CCM_Out[1][y]<<8)|(CCM_Out[2][y]);
		}
	#endif	
    }
   
}


MVOID
IspTuningCustom::
prepare_edge_gamma(ISP_NVRAM_EDGE_GAMMA_T& rEGamma)
{
    rEGamma.ctrl.bits.ED_GM_EN = 1;
    rEGamma.cfg1.bits.EGAMMA_B1 = 0x45;
    rEGamma.cfg1.bits.EGAMMA_B2 = 0x72;
    rEGamma.cfg1.bits.EGAMMA_B3 = 0x90;
    rEGamma.cfg1.bits.EGAMMA_B4 = 0xA6;
    rEGamma.cfg2.bits.EGAMMA_B5 = 0xC4;
    rEGamma.cfg2.bits.EGAMMA_B6 = 0xD7;
    rEGamma.cfg2.bits.EGAMMA_B7 = 0xE6;
    rEGamma.cfg2.bits.EGAMMA_B8 = 0xF1;
    rEGamma.cfg3.bits.EGAMMA_B9 = 0xF5;
    rEGamma.cfg3.bits.EGAMMA_B10= 0xF9;
    rEGamma.cfg3.bits.EGAMMA_B11= 0xFC;
}


EIndex_CCM_CCT_T
IspTuningCustom::
evaluate_CCM_CCT_index  (
    EIndex_CCM_CCT_T const eIdx_CCM_CCT_old, 
    MINT32 const i4CCT, 
    MINT32 const i4FluorescentIndex
)   const
{
    MY_LOG(
        "[+evaluate_CCM_CCT_index]"
        "(eIdx_CCM_CCT_old, i4CCT, i4FluorescentIndex)=(%d, %d, %d)"
        , eIdx_CCM_CCT_old, i4CCT, i4FluorescentIndex
    );

    EIndex_CCM_CCT_T eIdx_CCM_CCT_new = eIdx_CCM_CCT_old;

//    -----------------|---|---|--------------|---|---|------------------
//                                THA TH1 THB              THC TH2  THD

    MINT32 const THA = 3318;
    MINT32 const TH1 = 3484;
    MINT32 const THB = 3667;
    MINT32 const THC = 4810;
    MINT32 const TH2 = 5050;
    MINT32 const THD = 5316;
    MINT32 const F_IDX_TH1 = 25;
    MINT32 const F_IDX_TH2 = -25;

    switch  (eIdx_CCM_CCT_old)
    {
    case eIDX_CCM_CCT_TL84:
        if  ( i4CCT < THB )
        {
            eIdx_CCM_CCT_new = eIDX_CCM_CCT_TL84;
        }
        else if ( i4CCT < THD )
        {
            if  ( i4FluorescentIndex < F_IDX_TH2 )
                eIdx_CCM_CCT_new = eIDX_CCM_CCT_CWF;
            else 
                eIdx_CCM_CCT_new = eIDX_CCM_CCT_TL84;
        }
        else
        {
            eIdx_CCM_CCT_new = eIDX_CCM_CCT_D65;
        }
        break;
    case eIDX_CCM_CCT_CWF:
        if  ( i4CCT < THA )
        {
            eIdx_CCM_CCT_new = eIDX_CCM_CCT_TL84;
        }
        else if ( i4CCT < THD )
        {
            if  ( i4FluorescentIndex > F_IDX_TH1 )
                eIdx_CCM_CCT_new = eIDX_CCM_CCT_TL84;
            else 
                eIdx_CCM_CCT_new = eIDX_CCM_CCT_CWF;
        }
        else 
        {
            eIdx_CCM_CCT_new = eIDX_CCM_CCT_D65;
        }
        break;
    case eIDX_CCM_CCT_D65:
        if  ( i4CCT > THC )
        {
	        eIdx_CCM_CCT_new = eIDX_CCM_CCT_D65;
        } 
        else if ( i4CCT > TH1 )
        {
            if(i4FluorescentIndex > F_IDX_TH2)
                eIdx_CCM_CCT_new = eIDX_CCM_CCT_TL84;
            else 
                eIdx_CCM_CCT_new = eIDX_CCM_CCT_CWF;
        }
        else 
        {
            eIdx_CCM_CCT_new = eIDX_CCM_CCT_TL84;
        }
        break;
    }

//#if ENABLE_MY_LOG
    if  ( eIdx_CCM_CCT_old != eIdx_CCM_CCT_new )
    {
        LOGD(
            "[-evaluate_CCM_CCT_index] CCM CCT Idx(old,new)=(%d,%d)"
            , eIdx_CCM_CCT_old, eIdx_CCM_CCT_new
        );
    }
//#endif
    return  eIdx_CCM_CCT_new;
}


EIndex_Shading_CCT_T
IspTuningCustom::
evaluate_Shading_CCT_index  (
    EIndex_Shading_CCT_T const eIdx_Shading_CCT_old, 
    MINT32 const i4CCT
)   const
{
    MY_LOG(
        "[+evaluate_Shading_CCT_index]"
        "(eIdx_Shading_CCT_old, i4CCT,)=(%d, %d)"
        , eIdx_Shading_CCT_old, i4CCT
    );

    EIndex_Shading_CCT_T eIdx_Shading_CCT_new = eIdx_Shading_CCT_old;

//    -----------------|----|----|--------------|----|----|------------------
//                   THH2  TH2  THL2                   THH1  TH1  THL1

    MINT32 const THL1 = 3257;
    MINT32 const THH1 = 3484;
    MINT32 const TH1 = (THL1+THH1)/2; //(THL1 +THH1)/2
    MINT32 const THL2 = 4673;
    MINT32 const THH2 = 5155;
    MINT32 const TH2 = (THL2+THH2)/2;//(THL2 +THH2)/2

    switch  (eIdx_Shading_CCT_old)
    {
    case eIDX_Shading_CCT_ALight:
        if  ( i4CCT < THH1 )
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_ALight;
        }
        else if ( i4CCT <  TH2)
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
        }
        else
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
        }
        break;
    case eIDX_Shading_CCT_CWF:
        if  ( i4CCT < THL1 )
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_ALight;
        }
        else if ( i4CCT < THH2 )
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
        }
        else 
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
        }
        break;
    case eIDX_Shading_CCT_D65:
        if  ( i4CCT < TH1 )
        {
	     eIdx_Shading_CCT_new = eIDX_Shading_CCT_ALight;
        } 
        else if ( i4CCT < THL2 )
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
        }
        else 
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
        }
        break;
    }

//#if ENABLE_MY_LOG
    if  ( eIdx_Shading_CCT_old != eIdx_Shading_CCT_new )
    {
        LOGD(
            "[-evaluate_Shading_CCT_index] Shading CCT Idx(old,new)=(%d,%d), i4CCT = %d\n"
            , eIdx_Shading_CCT_old, eIdx_Shading_CCT_new,i4CCT
        );
    }
//#endif
    return  eIdx_Shading_CCT_new;
}


EIndex_ISO_T
IspTuningCustom::
map_ISO_value_to_index(MUINT32 const u4Iso) const
{
    if      ( u4Iso < 150 )
    {
        return  eIDX_ISO_100;
    }
    else if ( u4Iso < 300 )
    {
        return  eIDX_ISO_200;
    }
    else if ( u4Iso < 600 )
    {
        return  eIDX_ISO_400;
    }
    else if ( u4Iso < 900 )
    {
        return  eIDX_ISO_800;
    }
    return  eIDX_ISO_1600;
}


MBOOL
IspTuningCustom::
is_to_invoke_offline_capture(RAWIspCamInfo const& rCamInfo) const
{
#if 1
    EIndex_Scene_T const eIdx_Scene = rCamInfo.eIdx_Scene;
    EIndex_ISO_T const     eIdx_ISO = rCamInfo.eIdx_ISO;        //  ISO enum
    MUINT32 const        u4ISOValue = rCamInfo.u4ISOValue;      //  real ISO
    MUINT32 const             i4CCT = rCamInfo.i4CCT;
    MUINT32 const  u4ZoomRatio_x100 = rCamInfo.u4ZoomRatio_x100;
    MINT32 const   i4LightValue_x10 = rCamInfo.i4LightValue_x10;
#endif
#if 0
    switch  (eIdx_ISO)
    {
    case eIDX_ISO_100:
    case eIDX_ISO_200:
    case eIDX_ISO_400:
    case eIDX_ISO_800:
    case eIDX_ISO_1600:
    default:
        break;
    }
#endif
#if 1
		if(eIdx_ISO==eIDX_ISO_400 ||eIdx_ISO==eIDX_ISO_800 || eIdx_ISO==eIDX_ISO_1600)
		{
			return  MTRUE;
		}
		else
#endif			
    return  MFALSE;
}


#endif  //  ! MT6516

