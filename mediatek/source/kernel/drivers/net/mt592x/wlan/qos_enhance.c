







#include "precomp.h"

extern WIFI_CFG_DATA gPlatformCfg;

/* WMM Test Plan 1.4 Appendix F */
const WMM_AC_PARAM_T arDefaultWmmParamFor11agSTA[AC_NUM] = {
    {AC_BE, ACM_FLAG_ADM_NOT_REQUIRED, 3, 4, 10, 0},
    {AC_BK, ACM_FLAG_ADM_NOT_REQUIRED, 7, 4, 10, 0},
    {AC_VI, ACM_FLAG_ADM_NOT_REQUIRED, 2, 3, 4,  94},
    {AC_VO, ACM_FLAG_ADM_NOT_REQUIRED, 2, 2, 3,  47}};

const WMM_AC_PARAM_T arDefaultWmmParamFor11bSTA[AC_NUM] = {
    {AC_BE, ACM_FLAG_ADM_NOT_REQUIRED, 3, 4, 10, 0},
    {AC_BK, ACM_FLAG_ADM_NOT_REQUIRED, 7, 4, 10, 0},
    {AC_VI, ACM_FLAG_ADM_NOT_REQUIRED, 2, 3, 4,  188},
    {AC_VO, ACM_FLAG_ADM_NOT_REQUIRED, 2, 2, 3,  102}};






/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
qosWmmInfoInit (
    IN P_WMM_INFO_T prWmmInfo,
    IN BOOLEAN fgIs802_11b
    )
{

    ASSERT(prWmmInfo);

    kalMemZero(prWmmInfo, sizeof(WMM_INFO_T) - sizeof(prWmmInfo->arWmmAcParams));

    if (fgIs802_11b) {
        kalMemCopy((PVOID)prWmmInfo->arWmmAcParams,
                   (PVOID)arDefaultWmmParamFor11bSTA,
                   sizeof(arDefaultWmmParamFor11bSTA));
    }
    else {
        kalMemCopy((PVOID)prWmmInfo->arWmmAcParams,
                   (PVOID)arDefaultWmmParamFor11agSTA,
                   sizeof(arDefaultWmmParamFor11agSTA));
    }

    return;
} /* end of qosWmmInfoInit() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
qosCheckQoSCapabilityByWMMInfoElem (
    IN PUINT_8 pucBuf,
    OUT PUINT_8 pucWmmFlag
    )
{
    P_IE_WMM_INFO_ELEM_T prIeWmmInfoElem ;

    DEBUGFUNC("qosCheckQoSCapbilityByWMMInfoElem");


    ASSERT(pucBuf);
    ASSERT(pucWmmFlag);
    prIeWmmInfoElem = (P_IE_WMM_INFO_ELEM_T)pucBuf;

    /* Verify the length of the WMM Information Element. */
    if (prIeWmmInfoElem->ucLength != ELEM_MAX_LEN_WMM_INFO) {
        DBGLOG(MGT, LOUD, ("Invalid WMM Information Element length %d\n",
            prIeWmmInfoElem->ucLength));
        return FALSE;
    }

    *pucWmmFlag = WMM_FLAG_SUPPORT_WMM;

    if(gPlatformCfg.rWifiCustom.ucWmmPsEnable){
        if (prIeWmmInfoElem->ucQosInfo & WMM_QOS_INFO_UAPSD) {
            *pucWmmFlag |= WMM_FLAG_SUPPORT_UAPSD;
        }
	}

    return TRUE;

} /* end of qosCheckQoSCapabilityByWMMInfoElem() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
qosCheckQoSCapabilityByWMMParamElem (
    IN PUINT_8 pucBuf,
    OUT PUINT_8 pucWmmFlag
    )
{
    P_IE_WMM_PARAM_ELEM_T prIeWmmParamElem;

    DEBUGFUNC("qosCheckQoSCapbilityByWMMParamElem");


    ASSERT(pucBuf);
    ASSERT(pucWmmFlag);
    prIeWmmParamElem = (P_IE_WMM_PARAM_ELEM_T)pucBuf;

    /* Verify the length of the WME Parameter element. */
    if (prIeWmmParamElem->ucLength != ELEM_MAX_LEN_WMM_PARAM) {
        DBGLOG(MGT, LOUD, ("Invalid WMM Parameter Element length %d\n",
            prIeWmmParamElem->ucLength));
        return FALSE;
    }

    *pucWmmFlag = WMM_FLAG_SUPPORT_WMM;
	
    if(gPlatformCfg.rWifiCustom.ucWmmPsEnable){
        if (prIeWmmParamElem->ucQosInfo & WMM_QOS_INFO_UAPSD) {
            *pucWmmFlag |= WMM_FLAG_SUPPORT_UAPSD;
        }
    }
    return TRUE;

} /* end of qosCheckQoSCapabilityByWMMParamElem() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
qosParseWMMParamElem (
    IN PUINT_8 pucBuf,
    IN OUT P_WMM_INFO_T prWmmInfo
    )
{
    P_IE_WMM_PARAM_ELEM_T prIeWmmParamElem;
    P_WMM_AC_PARAM_RECORD_T prParamRecord;
    P_WMM_AC_PARAM_T prAcParam;
    UINT_32 i;

    DEBUGFUNC("qosParseWMMParamElem");


    ASSERT(pucBuf);
    ASSERT(prWmmInfo);
    prIeWmmParamElem = (P_IE_WMM_PARAM_ELEM_T)pucBuf;

    /* Verify the length of the WMM Parameter element. */
    if (prIeWmmParamElem->ucLength != ELEM_MAX_LEN_WMM_PARAM) {
        DBGLOG(MGT, LOUD, ("Invalid WMM Parameter length %d\n",
            prIeWmmParamElem->ucLength));
        return FALSE;
    }

    /* Check AC Parameters Records. */
    if ((prIeWmmParamElem->ucAcParamBEAciAifsn & WMM_ACIAIFSN_ACI) != WMM_ACI_AC_BE) {
        DBGLOG(MGT, LOUD, ("Invalid AC Parameters BE\n"));
        return FALSE;
    }

    if ((prIeWmmParamElem->ucAcParamBKAciAifsn & WMM_ACIAIFSN_ACI) != WMM_ACI_AC_BK) {
        DBGLOG(MGT, LOUD, ("Invalid AC Parameters BK\n"));
        return FALSE;
    }

    if ((prIeWmmParamElem->ucAcParamVIAciAifsn & WMM_ACIAIFSN_ACI) != WMM_ACI_AC_VI) {
        DBGLOG(MGT, LOUD, ("Invalid AC Parameters VI\n"));
        return FALSE;
    }

    if ((prIeWmmParamElem->ucAcParamVOAciAifsn & WMM_ACIAIFSN_ACI) != WMM_ACI_AC_VO) {
        DBGLOG(MGT, LOUD, ("Invalid AC Parameters VO\n"));
        return FALSE;
    }

    /* Update the WMM parameters. */
    prWmmInfo->ucWmmFlag = WMM_FLAG_SUPPORT_WMM;


    prWmmInfo->ucWmmParamSetCount =
        prIeWmmParamElem->ucQosInfo & WMM_QOS_INFO_PARAM_SET_CNT;

    DBGLOG(MGT, TRACE, ("WMM Parameter: Parameter Set Count = 0x%02x.\n",
        prWmmInfo->ucWmmParamSetCount));


    /* Update the AC parameters for AC_BE, AC_BK, AC_VI, and AC_VO. */
    for (i = AC_BE; i < AC_NUM; i++) {
        switch (i) {
        case AC_BE:
            prParamRecord =
                (P_WMM_AC_PARAM_RECORD_T)&prIeWmmParamElem->ucAcParamBEAciAifsn;
            break;

        case AC_BK:
            prParamRecord =
                (P_WMM_AC_PARAM_RECORD_T)&prIeWmmParamElem->ucAcParamBKAciAifsn;
            break;

        case AC_VI:
            prParamRecord =
                (P_WMM_AC_PARAM_RECORD_T)&prIeWmmParamElem->ucAcParamVIAciAifsn;
            break;

        case AC_VO:
            prParamRecord =
                (P_WMM_AC_PARAM_RECORD_T)&prIeWmmParamElem->ucAcParamVOAciAifsn;
            break;

        default:
            /* Won't reach here */
            return FALSE;
        }

        prAcParam = &prWmmInfo->arWmmAcParams[i];

        prAcParam->ucAci = (prParamRecord->ucAciAifsn & WMM_ACIAIFSN_ACI) >> WMM_ACIAIFSN_ACI_OFFSET;
        if (prParamRecord->ucAciAifsn & WMM_ACIAIFSN_ACM) {
            prAcParam->ucAcmFlag = ACM_FLAG_ADM_REQUIRED;
        }
        else {
            prAcParam->ucAcmFlag = ACM_FLAG_ADM_NOT_REQUIRED;
        }
        prAcParam->ucAifsn = prParamRecord->ucAciAifsn & WMM_ACIAIFSN_AIFSN;

        prAcParam->ucECWmin = prParamRecord->ucECWminECWmax &
            WMM_ECWMINECWMAX_ECWMIN;
        prAcParam->ucECWmax = (prParamRecord->ucECWminECWmax &
            WMM_ECWMINECWMAX_ECWMAX) >> WMM_ECWMINECWMAX_ECWMAX_OFFSET;

        WLAN_GET_FIELD_16(&prParamRecord->u2TxopLimit, &prAcParam->u2TxopLimit);

        DBGLOG(MGT, TRACE,
            ("WMM Parameter: ACI=%d, ACM=%d, AIFSN=%d, CWmin=%d, CWmax=%d, TXOPLimit=%d .\n",
                prAcParam->ucAci,
                prAcParam->ucAcmFlag,
                prAcParam->ucAifsn,
                ECW_TO_CW(prAcParam->ucECWmin),
                ECW_TO_CW(prAcParam->ucECWmax),
                prAcParam->u2TxopLimit));
    }

    prWmmInfo->ucWmmFlag |= WMM_FLAG_AC_PARAM_PRESENT;

    if(gPlatformCfg.rWifiCustom.ucWmmPsEnable){
        if (prIeWmmParamElem->ucQosInfo & WMM_QOS_INFO_UAPSD) {
            prWmmInfo->ucWmmFlag |= WMM_FLAG_SUPPORT_UAPSD;
        }
    }

    DBGLOG(MGT, TRACE, ("WMM Flags: %02x.\n", prWmmInfo->ucWmmFlag));


    return TRUE;

} /* end of qosParseWMMParamElem() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
UINT_8
qosConstructWMMInfoElem (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 ucWmmFlag,
    IN OUT PUINT_8 pucBuffer
    )
{
    P_IE_WMM_INFO_ELEM_T prIeWmmInfo;
    P_PM_PROFILE_SETUP_INFO_T prPmProfSetupInfo;

    UINT_32 ucUapsd[] = {
        WMM_QOS_INFO_BE_UAPSD,
        WMM_QOS_INFO_BK_UAPSD,
        WMM_QOS_INFO_VI_UAPSD,
        WMM_QOS_INFO_VO_UAPSD
    };
    UINT_8 aucWfaOui[] = VENDOR_OUI_WFA;


    ASSERT(prAdapter);
    ASSERT(pucBuffer);
    prIeWmmInfo = (P_IE_WMM_INFO_ELEM_T)pucBuffer;
    prPmProfSetupInfo = &prAdapter->rPmInfo.rPmProfSetupInfo;

    prIeWmmInfo->ucId = ELEM_ID_WMM;
    prIeWmmInfo->ucLength = ELEM_MAX_LEN_WMM_INFO;

    /* WMM-2.2.1 WMM Information Element Field Values */
    prIeWmmInfo->aucOui[0] = aucWfaOui[0];
    prIeWmmInfo->aucOui[1] = aucWfaOui[1];
    prIeWmmInfo->aucOui[2] = aucWfaOui[2];
    prIeWmmInfo->ucOuiType = VENDOR_OUI_TYPE_WMM;
    prIeWmmInfo->ucOuiSubtype = VENDOR_OUI_SUBTYPE_WMM_INFO;

    prIeWmmInfo->ucVersion = VERSION_WMM;
    prIeWmmInfo->ucQosInfo = 0;

    if (ucWmmFlag & WMM_FLAG_SUPPORT_UAPSD) {
        UINT_8 ucQosInfo = 0;
        ENUM_AC_T eQueNum;

        /* Static U-APSD setting */
        for (eQueNum = AC_BE; eQueNum <= AC_VO; eQueNum++) {
            if (PM_IS_AC_QUEUE_DELIVERY_AND_TRIGGER_ENABLED(prAdapter, eQueNum)) {
                ucQosInfo |= (UINT_8)ucUapsd[eQueNum];
            }
        }

        switch (prPmProfSetupInfo->ucUapsdSp) {
        case WMM_MAX_SP_LENGTH_ALL:
            ucQosInfo |= WMM_QOS_INFO_MAX_SP_ALL;
            break;
        case WMM_MAX_SP_LENGTH_2:
            ucQosInfo |= WMM_QOS_INFO_MAX_SP_2;
            break;
        case WMM_MAX_SP_LENGTH_4:
            ucQosInfo |= WMM_QOS_INFO_MAX_SP_4;
            break;
        case WMM_MAX_SP_LENGTH_6:
            ucQosInfo |= WMM_QOS_INFO_MAX_SP_6;
            break;
        default:
            WARNLOG(("U-APSD should not have this Service Period: %d\n",
                        prPmProfSetupInfo->ucUapsdSp));
        }
        WMM_INFO_IE(prIeWmmInfo)->ucQosInfo = ucQosInfo;
    }

    return (ELEM_HDR_LEN + ELEM_MAX_LEN_WMM_INFO);

} /* end of qosConstructWMMInfoElem() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
qosUpdateWMMParametersAndAssignAllowedACI (
    IN P_ADAPTER_T prAdapter,
    IN P_WMM_INFO_T prWmmInfo
    )
{
    const UINT_8 aucACI2ACPriority[] = {
                AC_BE_PRIORITY, /* Index by ACI */
                AC_BK_PRIORITY,
                AC_VI_PRIORITY,
                AC_VO_PRIORITY
                };

    const UINT_8 aucACPriority2ACI[] = {
                AC_BK, /* Index by AC Priority */
                AC_BE,
                AC_VI,
                AC_VO
                };

    UINT_8 aucACI2AdmittedACI[] = {
                AC_BE, /* Index by ACI */
                AC_BK,
                AC_VI,
                AC_VO
                };
    UINT_32 u4ACI;
    INT_32 j;

    DEBUGFUNC("qosUpdateWMMParametersAndAssignAllowedACI");


    ASSERT(prAdapter);
    ASSERT(prWmmInfo);

    //4 <1> Update GRANTED Admission Control ACI mapping table
    /* Loop by ACI */
    for (u4ACI = AC_BE; u4ACI < AC_NUM; u4ACI++) {
        if ((prWmmInfo->arWmmAcParams[u4ACI].ucAcmFlag & (ACM_FLAG_ADM_GRANTED | ACM_FLAG_ADM_REQUIRED))
            == ACM_FLAG_ADM_REQUIRED) {
            BOOLEAN fgIsFindAllowedAC = FALSE;


            /* Loop by lower priority */
            for (j = (aucACI2ACPriority[u4ACI] - 1); (j >= AC_BK_PRIORITY && j <= AC_VO_PRIORITY); j--) {
                UINT_8 ucAcmFlag = prWmmInfo->arWmmAcParams[aucACPriority2ACI[j]].ucAcmFlag;

                if ((ucAcmFlag == ACM_FLAG_ADM_NOT_REQUIRED) ||
                    ((ucAcmFlag & (ACM_FLAG_ADM_GRANTED | ACM_FLAG_ADM_REQUIRED)) ==
                        (ACM_FLAG_ADM_GRANTED | ACM_FLAG_ADM_REQUIRED))) {

                    fgIsFindAllowedAC = TRUE;
                    break;
                }
            }

            if (!fgIsFindAllowedAC) {
                /* TODO(Kevin): should we use the BK for this not granted AC ?
                 * ucACI2AdmittedACI[i] = ucACPriority2ACI[AC_BK_PRIORITY];
                 */
                DBGLOG(MGT, WARN,
                    ("Can not find allowed AC for not granted AC - %ld\n", u4ACI));
            }
            else if (j >= AC_BK_PRIORITY) {
                aucACI2AdmittedACI[u4ACI] = aucACPriority2ACI[j];
                //printk("t1 %d\n", aucACI2AdmittedACI[u4ACI]);
            }
        }
    }

    nicTxQoSAssignAdmittedTXQ(prAdapter,
                              aucACI2AdmittedACI);

    nicTxQoSUpdateTXQParameters(prAdapter,
                                prWmmInfo->arWmmAcParams);

    return;
} /* end of qosUpdateWMMParametersAndAssignAllowedACI() */



