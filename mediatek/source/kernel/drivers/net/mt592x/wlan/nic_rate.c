






#include "precomp.h"



const AR_PROFILE_T arARProfileSetting[AR_PROFILE_NUM] = {
    /* Profie Index       , up_limit, down_limit, RC,  PER,  reserve, reserve */
    {AR_PROFILE_THROUGHPUT, 0x0217,   0x0FFF,    0x03, 0x03, {0x00,    0x00}},
    {AR_PROFILE_VOIP,       0x0217,   0x09C4,    0x03, 0x03, {0x00,    0x00}}
};








/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicARInit (
    IN  P_ADAPTER_T prAdapter
    )
{
    nicARDisable(prAdapter);
    halARInitialize(prAdapter);

    nicARSetProfile(prAdapter, AR_PROFILE_THROUGHPUT);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicAREnable (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgIsHWAR /* TRUE: Hardware Autorate, FALSE: Software Autorate */
    )
{
    P_AR_CTRL_T     prARCtrl;
    P_AR_PROFILE_T  prAR_Setting;

    ASSERT(prAdapter);
    prARCtrl = &prAdapter->rARCtrl;
    prAR_Setting = prARCtrl->prAR_Profile;

    if (fgIsHWAR) {
        prARCtrl->eType = AR_HW;

        ASSERT(prAR_Setting);
        if (prAR_Setting) {
            halARSetParam(prAdapter, prAR_Setting->u2FailCount_up_limit,
                prAR_Setting->u2FailCount_down_limit, prAR_Setting->ucRCParam,
                prAR_Setting->ucPERParam);
        }
    }
    else {
        prARCtrl->eType = AR_SW;
    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicARDisable (
    IN  P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);
    
    prAdapter->rARCtrl.eType = AR_DISABLE;
    halARSetParam(prAdapter, 0, 0, 0, 0);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicARSetRate (
    IN P_ADAPTER_T prAdapter,
    IN UINT_16 u2RateSet,
    IN BOOLEAN fgIsShortPreamble,
    IN UINT_8 ucRate1,
    IN UINT_8 ucWlanIdx,
    IN BOOLEAN fgIsRate1Assigned
    )
{

    UINT_8      i = 0;

    ASSERT(prAdapter);

    if (!fgIsRate1Assigned) {
        for (i= 0; i < RATE_NUM; i++) {
            if (u2RateSet & BIT(i)) {
                if (ucRate1 < i) {
                    ucRate1 = i;
                 }
            }
        }
    }

    halARSetRate(prAdapter, ucWlanIdx, u2RateSet, fgIsShortPreamble, ucRate1);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicARReset (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 ucWlanIdx
    )
{
    ASSERT(prAdapter);

    halARReset(prAdapter, ucWlanIdx);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicARSetProfile (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_AUTORATE_PROFILE_T eARProfile
    )
{
    P_AR_CTRL_T     prARCtrl;
    P_AR_PROFILE_T  prAR_Setting;

    ASSERT(prAdapter);
    prARCtrl = &prAdapter->rARCtrl;

    ASSERT(eARProfile < AR_PROFILE_NUM);

    prAR_Setting = (P_AR_PROFILE_T) &arARProfileSetting[eARProfile];
    prARCtrl->prAR_Profile = prAR_Setting;

    if (prARCtrl->eType == AR_HW) {
        halARSetParam(prAdapter, prAR_Setting->u2FailCount_up_limit,
            prAR_Setting->u2FailCount_down_limit, prAR_Setting->ucRCParam,
            prAR_Setting->ucPERParam);
    }

    return;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicRateEnableProtection (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 ucProtectionMode
    )
{
    P_RATE_INFO_T prRateInfo;


    ASSERT(prAdapter);
    prRateInfo = &prAdapter->rRateInfo;

    prRateInfo->fgCTSProtectionEnabled = TRUE;
    prRateInfo->ucCTSProtectionMode = ucProtectionMode;

    halEnableCTSProtectionMode(prAdapter, ucProtectionMode);

    return;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicRateDisableProtection (
    IN P_ADAPTER_T prAdapter
    )
{
    P_RATE_INFO_T prRateInfo;

    ASSERT(prAdapter);
    prRateInfo = &prAdapter->rRateInfo;

    halDisableCTSProtectionMode(prAdapter);
    prRateInfo->fgCTSProtectionEnabled = FALSE;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicRateSetCTSRTSRate (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 ucRateIndex,
    IN BOOLEAN fgIsShortPreamble
    )
{
    P_RATE_INFO_T prRateInfo;


    ASSERT(prAdapter);
    prRateInfo = &prAdapter->rRateInfo;

    prRateInfo->ucRTSCTSRateIndex = ucRateIndex;
    prRateInfo->fgIsRTSCTSRateShortPreamble = fgIsShortPreamble;
    halSetCTSRTSRate(prAdapter, ucRateIndex, fgIsShortPreamble);

    return;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicRateSetBasicRate (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 ucRateIndex,
    IN BOOLEAN fgIsShortPreamble
    )
{
    P_RATE_INFO_T prRateInfo;


    ASSERT(prAdapter);
    prRateInfo = &prAdapter->rRateInfo;

    prRateInfo->fgIsBasicRateShortPreamble = fgIsShortPreamble;
    prRateInfo->ucBasicRateIndex = ucRateIndex;
    halSetBasicRate(prAdapter, ucRateIndex, fgIsShortPreamble);

    return;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicRateSetRTSThreshold (
    IN P_ADAPTER_T prAdapter,
    IN UINT_16 u2RTSThreshold
    )
{
    P_RATE_INFO_T prRateInfo;


    ASSERT(prAdapter);
    ASSERT(u2RTSThreshold);
    prRateInfo = &prAdapter->rRateInfo;

    prRateInfo->u2RTSThreshold = u2RTSThreshold;
    halSetRTSThreshold(prAdapter, u2RTSThreshold);

    return;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicRateSetAckCtsRate(
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 aucAckCtsRateIndex[],
    IN BOOLEAN fgIsShortPreamble
    )
{
    P_RATE_INFO_T   prRateInfo;
    UINT_8          i = 0;


    ASSERT(prAdapter);
    ASSERT(aucAckCtsRateIndex);
    prRateInfo = &prAdapter->rRateInfo;

    for (i = 0; i < RATE_NUM; i++) {
        prRateInfo->aucAckCtsRateIndex[i] = aucAckCtsRateIndex[i];
    }
    prRateInfo->fgIsAckCtsRateShortPreamble = fgIsShortPreamble;
    halSetAckCtsRate(prAdapter, aucAckCtsRateIndex, fgIsShortPreamble);

    return;
}


