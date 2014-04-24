








#include "precomp.h"

extern PHY_ATTRIBUTE_T rPhyAttributes[];







/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
scanInitialize (
    IN P_ADAPTER_T prAdapter
    )
{
    P_SCAN_INFO_T prScanInfo;
    PUINT_8 pucBSSBuff;
    P_BSS_DESC_T prBSSDesc = (P_BSS_DESC_T)NULL;
    UINT_32 i;


    ASSERT(prAdapter);
    prScanInfo =  &prAdapter->rScanInfo;
    pucBSSBuff = prScanInfo->pucBSSCached;

    //4 <0> Clear allocated memory.
    kalMemZero((PVOID) prScanInfo->pucBSSCached, prScanInfo->u4BSSCachedSize);

    LINK_INITIALIZE(&prScanInfo->rFreeBSSDescList);
    LINK_INITIALIZE(&prScanInfo->rBSSDescList);

    for (i = 0; i < CFG_MAX_NUM_BSS_LIST; i++) {

        prBSSDesc = (P_BSS_DESC_T)pucBSSBuff;

        BSS_DESC_SET_GUID(prBSSDesc);

        LINK_INSERT_TAIL(&prScanInfo->rFreeBSSDescList, &prBSSDesc->rLinkEntry);

        pucBSSBuff += ALIGN_4(sizeof(BSS_DESC_T));
    }
    /* Check if the memory allocation consist with this initialization function */
    ASSERT((UINT_32)(pucBSSBuff - prScanInfo->pucBSSCached) == prScanInfo->u4BSSCachedSize);

    return;

} /* end of scanInitialize() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
P_BSS_DESC_T
scanSearchBssDescByPolicy (
    IN P_ADAPTER_T prAdapter
    )
{
    P_CONNECTION_SETTINGS_T prConnSettings;
    P_BSS_INFO_T prBssInfo;
    P_SCAN_INFO_T prScanInfo;
    P_LINK_T prBSSDescList;
    P_BSS_DESC_T prBssDesc = (P_BSS_DESC_T)NULL;
    P_BSS_DESC_T prPrimaryBssDesc;
    P_BSS_DESC_T prCandidateBssDesc = (P_BSS_DESC_T)NULL;

    P_STA_RECORD_T prStaRec = (P_STA_RECORD_T)NULL;
    P_STA_RECORD_T prPrimaryStaRec;
    P_STA_RECORD_T prCandidateStaRec = (P_STA_RECORD_T)NULL;

    /* The first one reach the check point will be our candidate */
    BOOLEAN fgIsFindFirst = (BOOLEAN)FALSE;

    BOOLEAN fgIsFindBestRSSI = (BOOLEAN)FALSE;
    BOOLEAN fgIsFindBestEncryptionLevel = (BOOLEAN)FALSE;
    BOOLEAN fgIsFindMinChannelLoad = (BOOLEAN)FALSE;

#if CFG_SW_TCL /* Compare the TSF counter by SW */
    BOOLEAN fgIsLocalTSFRead = (BOOLEAN)FALSE;
    ULARGE_INTEGER rCurrentTsf;
#endif /* CFG_SW_TCL */

    /* TODO(Kevin): Support Min Channel Load */
    //UINT_8 aucChannelLoad[CHANNEL_NUM] = {0};

    DEBUGFUNC("scanSearchBssDescByPolicy");


    ASSERT(prAdapter);
    prConnSettings = &prAdapter->rConnSettings;
    prBssInfo = &prAdapter->rBssInfo;
    prScanInfo =  &prAdapter->rScanInfo;
    prBSSDescList = &prScanInfo->rBSSDescList;

    /* Check if such BSS Descriptor exists in a valid list */
    DBGLOG(SCAN, TRACE, ("Number Of SCAN Result = %ld\n",
        prBSSDescList->u4NumElem));

    //4 <1> The outer loop to search for a candidate.
    LINK_FOR_EACH_ENTRY(prBssDesc, prBSSDescList, rLinkEntry, BSS_DESC_T) {

        BSS_DESC_CHK_GUID(prBssDesc);

        /* TODO(Kevin): Update Minimum Channel Load Information here */
        DBGLOG(SCAN, TRACE, ("BSS DESC(%#lx): MAC: "MACSTR"\n",
            (UINT_32)prBssDesc, MAC2STR(prBssDesc->aucBSSID)));

        //4 <2> Check various conditions for mismatch.
        //4 <2.A> Check Previous Status Code from JOIN result first.
        /* NOTE(Kevin): STA_RECORD_T is recorded by TA. */
        prStaRec = staRecGetStaRecordByAddr(prAdapter,
                                            prBssDesc->aucSrcAddr);

        if (prStaRec) {
            /* NOTE(Kevin):
             * The Status Code is the result of a Previous Connection Request, we use this as SCORE for choosing a proper
             * candidate (Used for compare see <6>)
             * The Reason Code is an indication of the reason why AP reject us, we use this Code for "Reject"
             * a SCAN result to become our candidate(Like a blacklist).
             */
            if (prStaRec->u2ReasonCode != REASON_CODE_RESERVED) {
                DBGLOG(SCAN, TRACE, ("Ignore BSS DESC MAC: "MACSTR", JOIN Reason Code = %d\n",
                    MAC2STR(prBssDesc->aucBSSID), prStaRec->u2ReasonCode));
                continue;
            }
            else if (prStaRec->u2StatusCode != STATUS_CODE_SUCCESSFUL) {

                if (prStaRec->ucJoinFailureCount < JOIN_MAX_RETRY_FAILURE_COUNT) {
                    OS_SYSTIME rCurrentTime;


                    GET_CURRENT_SYSTIME(&rCurrentTime);

                    /* NOTE(Kevin): greedy association - after timeout, we'll still
                     * try to associate to the AP whose STATUS of conection attempt
                     * was not success.
                     * We may also use (ucJoinFailureCount x JOIN_RETRY_INTERVAL_SEC) for
                     * time bound.
                     */
                    if (CHECK_FOR_TIMEOUT(rCurrentTime,
                                          prStaRec->rLastJoinTime,
                                          SEC_TO_SYSTIME(JOIN_RETRY_INTERVAL_SEC))) {

#if DBG
                        if ((prStaRec->u2StatusCode == STATUS_CODE_JOIN_TIMEOUT) ||
                            (prStaRec->u2StatusCode == STATUS_CODE_AUTH_TIMEOUT) ||
                            (prStaRec->u2StatusCode == STATUS_CODE_ASSOC_TIMEOUT)) {
                            DBGLOG(SCAN, TRACE, ("Accept BSS DESC MAC: "MACSTR", but previous JOIN Status Code = %s\n",
                                MAC2STR(prBssDesc->aucBSSID),
                                ((prStaRec->u2StatusCode == STATUS_CODE_JOIN_TIMEOUT) ? "JOIN_TIMEOUT" :
                                 ((prStaRec->u2StatusCode == STATUS_CODE_AUTH_TIMEOUT) ? "AUTH_TIMEOUT" : "ASSOC_TIMEOUT") ) ) );
                        }
                        else {
                            DBGLOG(SCAN, TRACE, ("Accpet BSS DESC MAC: "MACSTR", but previous JOIN Status Code = %d\n",
                                MAC2STR(prBssDesc->aucBSSID), prStaRec->u2StatusCode));
                        }
#endif /* DBG */
                    }
                    else {
                        DBGLOG(SCAN, TRACE, ("Ignore BSS DESC MAC: "MACSTR", JOIN Status Code = %d\n",
                            MAC2STR(prBssDesc->aucBSSID), prStaRec->u2StatusCode));
                        continue;
                    }
                }
                else {
                    DBGLOG(SCAN, TRACE, ("* Ignore BSS DESC MAC: "MACSTR", JOIN Status Code = %d\n",
                        MAC2STR(prBssDesc->aucBSSID), prStaRec->u2StatusCode));
                    continue;
                }
            }
        }


        //4 <2.B> Check BSS Type for the corresponding Operation Mode in Connection Setting
        /* NOTE(Kevin): For NET_TYPE_AUTO_SWITCH, we will always pass following check. */
        if (((prConnSettings->eOPMode == NET_TYPE_INFRA) &&
             (prBssDesc->eBSSType != BSS_TYPE_INFRASTRUCTURE)) ||
            (((prConnSettings->eOPMode == NET_TYPE_IBSS) ||
              (prConnSettings->eOPMode == NET_TYPE_DEDICATED_IBSS)) &&
             (prBssDesc->eBSSType != BSS_TYPE_IBSS))) {

            DBGLOG(SCAN, TRACE, ("Ignore BSS DESC MAC: "MACSTR", eBSSType = %s\n",
                MAC2STR(prBssDesc->aucBSSID), ((prBssDesc->eBSSType == BSS_TYPE_INFRASTRUCTURE) ?
                    "BSS_TYPE_INFRASTRUCTURE" : "BSS_TYPE_IBSS")));
            continue;
        }


        //4 <2.C> Check if this SCAN record has been updated recently for IBSS.
        /* NOTE(Kevin): Because some STA may change its BSSID frequently after it
         * create the IBSS, so we need to make sure we get the new one.
         * For BSS, if the old record was matched, however it won't be able to pass
         * the Join Process later.
         */
        if (prBssDesc->eBSSType == BSS_TYPE_IBSS) {
            OS_SYSTIME rCurrentTime;

            GET_CURRENT_SYSTIME(&rCurrentTime);
            if (CHECK_FOR_TIMEOUT(rCurrentTime, prBssDesc->rUpdateTime,
                                  SEC_TO_SYSTIME(BSS_DESC_TIMEOUT_SEC))) {
                DBGLOG(SCAN, TRACE, ("Skip old record of BSS Descriptor - BSSID:["MACSTR"]\n\n",
                          MAC2STR(prBssDesc->aucBSSID)));
                continue;
            }
        }

        if ((prBssDesc->eBSSType == BSS_TYPE_INFRASTRUCTURE) &&
            (prAdapter->eConnectionState == MEDIA_STATE_CONNECTED)) {
            OS_SYSTIME rCurrentTime;

            GET_CURRENT_SYSTIME(&rCurrentTime);
            if (CHECK_FOR_TIMEOUT(rCurrentTime, prBssDesc->rUpdateTime,
                                  SEC_TO_SYSTIME(BSS_DESC_TIMEOUT_SEC))) {
                DBGLOG(SCAN, TRACE, ("Skip old record of BSS Descriptor - BSSID:["MACSTR"]\n\n",
                          MAC2STR(prBssDesc->aucBSSID)));
                continue;
            }
        }


        //4 <2.D> Check Unsupported BSS PHY Type
        if (!(BIT(prBssDesc->ePhyType) & prAdapter->u2AvailablePhyTypeSet)) {

            DBGLOG(SCAN, TRACE, ("Ignore BSS DESC MAC: "MACSTR", PhyType = %s not supported\n",
                MAC2STR(prBssDesc->aucBSSID),
                ((prBssDesc->ePhyType == PHY_TYPE_ERP_INDEX) ? "PHY_TYPE_ERP" :
                 ((prBssDesc->ePhyType == PHY_TYPE_HR_DSSS_INDEX) ? "PHY_TYPE_HR_DSSS" : "PHY_TYPE_OFDM") ) ));
            continue;
        }


        //4 <2.E> Check AP's BSSID if OID_802_11_BSSID has been set.
        if ((prConnSettings->fgIsConnByBssidIssued) &&
            (prBssDesc->eBSSType == BSS_TYPE_INFRASTRUCTURE)) {

            if (UNEQUAL_MAC_ADDR(prConnSettings->aucBSSID, prBssDesc->aucBSSID)) {

                DBGLOG(SCAN, TRACE, ("Ignore BSS DESC MAC: "MACSTR", Expected BSSID :"MACSTR"\n",
                    MAC2STR(prBssDesc->aucBSSID), MAC2STR(prConnSettings->aucBSSID)));
                continue;
            }
        }


        prPrimaryBssDesc = (P_BSS_DESC_T)NULL;

        //4 <3> Check current Connection Policy.
        switch (prConnSettings->eConnectionPolicy) {
        case CONNECT_BY_SSID_BEST_RSSI:
            /* Choose Hidden SSID to join only if the `fgIsEnableJoin...` is TRUE */
            if (prAdapter->fgIsEnableJoinToHiddenSSID && prBssDesc->fgIsHiddenSSID) {
                /* NOTE(Kevin): following if () statement means that
                 * If Target is hidden, then we won't connect when user specify SSID_ANY policy.
                 */
                if (prConnSettings->ucSSIDLen) {
                    prPrimaryBssDesc = prBssDesc;

                    fgIsFindBestRSSI = TRUE;
                }

            }
            else if (EQUAL_SSID(prBssDesc->aucSSID,
                                prBssDesc->ucSSIDLen,
                                prConnSettings->aucSSID,
                                prConnSettings->ucSSIDLen)) {
                prPrimaryBssDesc = prBssDesc;

                fgIsFindBestRSSI = TRUE;
            }
            break;

        case CONNECT_BY_SSID:
            /* Choose Hidden SSID to join only if the `fgIsEnableJoin...` is TRUE */
            if (prAdapter->fgIsEnableJoinToHiddenSSID && prBssDesc->fgIsHiddenSSID) {
                /* NOTE(Kevin): following if () statement means that
                 * If Target is hidden, then we won't connect when user specify SSID_ANY policy.
                 */
                if (prConnSettings->ucSSIDLen) {
                    prPrimaryBssDesc = prBssDesc;
                }
            }
            else if (EQUAL_SSID(prBssDesc->aucSSID,
                                prBssDesc->ucSSIDLen,
                                prConnSettings->aucSSID,
                                prConnSettings->ucSSIDLen)) {
                prPrimaryBssDesc = prBssDesc;

                fgIsFindFirst = TRUE;
            }
            break;

        case CONNECT_BY_SSID_ANY:
            /* NOTE(Kevin): In this policy, we don't know the desired
             * SSID from user, so we should exclude the Hidden SSID from candidacy.
             * And because we refuse to connect to Hidden SSID node at the beginning, so
             * when the JOIN Module deal with a BSS_DESC_T which has fgIsHiddenSSID == TRUE,
             * then the Connection Settings must be valid without doubt.
             */
            if (!prBssDesc->fgIsHiddenSSID) {
                prPrimaryBssDesc = prBssDesc;

                fgIsFindFirst = TRUE;
            }
            break;

        case CONNECT_BY_SSID_BEST_RSSI_MIN_CH_LOAD:
            /* Choose Hidden SSID to join only if the `fgIsEnableJoin...` is TRUE */
            if (prAdapter->fgIsEnableJoinToHiddenSSID && prBssDesc->fgIsHiddenSSID) {
                /* NOTE(Kevin): following if () statement means that
                 * If Target is hidden, then we won't connect when user specify SSID_ANY policy.
                 */
                if (prConnSettings->ucSSIDLen) {
                    prPrimaryBssDesc = prBssDesc;

                    fgIsFindBestRSSI = TRUE;
                    fgIsFindMinChannelLoad = TRUE;
                }
            }
            else if (EQUAL_SSID(prBssDesc->aucSSID,
                                prBssDesc->ucSSIDLen,
                                prConnSettings->aucSSID,
                                prConnSettings->ucSSIDLen)) {
                prPrimaryBssDesc = prBssDesc;

                fgIsFindBestRSSI = TRUE;
                fgIsFindMinChannelLoad = TRUE;
            }
            break;

        default:
            break;
        }


        /* Primary Candidate was not found */
        if (prPrimaryBssDesc == NULL) {
            continue;
        }

        prPrimaryStaRec = prStaRec;

        //4 <4> Validate this primary Scan Record.
        //4 <4A> Check the Unknown or Unsupported BSS Basic Rate Set.
        /* Skip if one or more BSS Basic Rate are not supported or unknown */
        if ((prPrimaryBssDesc->u2BSSBasicRateSet &
             ~(rPhyAttributes[prPrimaryBssDesc->ePhyType].u2SupportedRateSet)) ||
            prPrimaryBssDesc->fgIsUnknownBssBasicRate) {

            continue;
        }

        //4 <4B> Check for IBSS AdHoc Mode.
        /* Skip if one or more BSS Basic Rate are not supported by current AdHocMode */
        if (prPrimaryBssDesc->eBSSType == BSS_TYPE_IBSS) {
            //4 <4B.1> Check if match the Capability of current IBSS AdHoc Mode.
            if (ibssCheckCapabilityForAdHocMode(prAdapter, prPrimaryBssDesc) == WLAN_STATUS_FAILURE) {

                DBGLOG(SCAN, TRACE,
                    ("Ignore BSS DESC MAC: "MACSTR", Capability is not supported for current AdHoc Mode.\n",
                    MAC2STR(prPrimaryBssDesc->aucBSSID)));

                continue;
            }


            //4 <4B.2> IBSS Merge Decision Flow for SEARCH STATE.
            if (prAdapter->fgIsIBSSActive &&
                UNEQUAL_MAC_ADDR(prBssInfo->aucBSSID, prPrimaryBssDesc->aucBSSID)) {

#if !CFG_SW_TCL // NOTE(Kevin 2008/03/29): Use TCL in RFB.
                DBGLOG(SCAN, TRACE,
                    ("prPrimaryBssDesc->fgIsLargerTSF = %d\n",
                        prPrimaryBssDesc->fgIsLargerTSF));

                if (!prPrimaryBssDesc->fgIsLargerTSF) {
                    DBGLOG(SCAN, TRACE,
                        ("Ignore BSS DESC MAC: ["MACSTR"], Current BSSID: ["MACSTR"].\n",
                            MAC2STR(prPrimaryBssDesc->aucBSSID), MAC2STR(prBssInfo->aucBSSID)));
                    continue;
                }
#else /* CFG_SW_TCL */
                if (!fgIsLocalTSFRead) {
                    NIC_GET_CURRENT_TSF(prAdapter, &rCurrentTsf);

                    DBGLOG(SCAN, TRACE,
                        ("\n\nCurrent TSF : %08lx-%08lx\n\n",
                            rCurrentTsf.u.HighPart, rCurrentTsf.u.LowPart));
                }

                if (rCurrentTsf.QuadPart > prPrimaryBssDesc->u8TimeStamp.QuadPart) {
                    DBGLOG(SCAN, TRACE,
                        ("Ignore BSS DESC MAC: ["MACSTR"], Current BSSID: ["MACSTR"].\n",
                            MAC2STR(prPrimaryBssDesc->aucBSSID), MAC2STR(prBssInfo->aucBSSID)));

                    DBGLOG(SCAN, TRACE,
                        ("\n\nBSS's TSF : %08lx-%08lx\n\n",
                            prPrimaryBssDesc->u8TimeStamp.u.HighPart, prPrimaryBssDesc->u8TimeStamp.u.LowPart));

                    prPrimaryBssDesc->fgIsLargerTSF = FALSE;
                    continue;
                }
                else {
                    prPrimaryBssDesc->fgIsLargerTSF = TRUE;
                }
#endif /* CFG_SW_TCL */

            }
        }

        //4 <5> Check the Encryption Status.
#if SUPPORT_WAPI
        if (prAdapter->fgUseWapi) {
            DBGLOG(WAPI, INFO, ("wapiPerformPolicySelection\n"));
            if (wapiPerformPolicySelection(prAdapter, prPrimaryBssDesc)) {
                fgIsFindFirst = TRUE;
            }
            else {
                /* Can't pass the Encryption Status Check, get next one */
                continue;
            }       
        }
        else
#endif          
        if (rsnPerformPolicySelection(prAdapter, prPrimaryBssDesc)) {

            if (prPrimaryBssDesc->ucEncLevel > 0) {
                fgIsFindBestEncryptionLevel = TRUE;

                fgIsFindFirst = FALSE;
            }
        }
        else {
            /* Can't pass the Encryption Status Check, get next one */
            continue;
        }

        /* For RSN Pre-authentication, update the PMKID canidate list for
           same SSID and encrypt status */
        /* Update PMKID candicate list. */
        if (prAdapter->rConnSettings.eAuthMode == AUTH_MODE_WPA2) {
            rsnUpdatePmkidCandidateList(prAdapter,	prPrimaryBssDesc);
            if (prAdapter->rSecInfo.u4PmkidCandicateCount) {
                prAdapter->rSecInfo.fgIndicatePMKID = rsnCheckPmkidCandicate(prAdapter);
            }
        }

        //4 <6> Compare the Candidate and the Primary Scan Record.
        if (!prCandidateBssDesc) {
            prCandidateBssDesc = prPrimaryBssDesc;
            prCandidateStaRec = prPrimaryStaRec;

            //4 <6A> Condition - Get the first matched one.
            if (fgIsFindFirst) {
                break;
            }
        }
        else {
            //4 <6B> Condition - Choose the one with best Encryption Score.
            if (fgIsFindBestEncryptionLevel) {
                if (prCandidateBssDesc->ucEncLevel <
                    prPrimaryBssDesc->ucEncLevel) {

                    prCandidateBssDesc = prPrimaryBssDesc;
                    prCandidateStaRec = prPrimaryStaRec;
                    continue;
                }
            }

            /* If reach here, that means they have the same Encryption Score.
             */

            //4 <6C> Condition - Give opportunity to the one we didn't connect before.
            // For roaming, only compare the candidates other than current associated BSSID.
            if (!prCandidateBssDesc->fgIsConnected && !prPrimaryBssDesc->fgIsConnected) {
                if ((prCandidateStaRec != (P_STA_RECORD_T)NULL) &&
                    (prCandidateStaRec->u2StatusCode != STATUS_CODE_SUCCESSFUL)) {

                    DBGLOG(SCAN, TRACE, ("So far -BSS DESC MAC: "MACSTR" has nonzero Status Code = %d\n",
                        MAC2STR(prCandidateBssDesc->aucBSSID), prCandidateStaRec->u2StatusCode));

                    if (prPrimaryStaRec != (P_STA_RECORD_T)NULL) {
                        if (prPrimaryStaRec->u2StatusCode != STATUS_CODE_SUCCESSFUL) {

                            /* Give opportunity to the one with smaller rLastJoinTime */
                            if (TIME_BEFORE(prCandidateStaRec->rLastJoinTime,
                                                        prPrimaryStaRec->rLastJoinTime)) {
                                continue;
                            }
                            /* We've connect to CANDIDATE recently, let us try PRIMARY now */
                            else {
                                prCandidateBssDesc = prPrimaryBssDesc;
                                prCandidateStaRec = prPrimaryStaRec;
                                continue;
                            }
                        }
                        /* PRIMARY's u2StatusCode = 0 */
                        else {
                            prCandidateBssDesc = prPrimaryBssDesc;
                            prCandidateStaRec = prPrimaryStaRec;
                            continue;
                        }
                    }
                    /* PRIMARY has no StaRec - We didn't connet to PRIMARY before */
                    else {
                        prCandidateBssDesc = prPrimaryBssDesc;
                        prCandidateStaRec = prPrimaryStaRec;
                        continue;
                    }
                }
                else {
                    if ((prPrimaryStaRec != (P_STA_RECORD_T)NULL) &&
                        (prPrimaryStaRec->u2StatusCode != STATUS_CODE_SUCCESSFUL)) {
                        continue;
                    }
                }
            }


            //4 <6D> Condition - Visible SSID win Hidden SSID.
            if (prCandidateBssDesc->fgIsHiddenSSID) {
                if (!prPrimaryBssDesc->fgIsHiddenSSID) {
                    prCandidateBssDesc = prPrimaryBssDesc; /* The non Hidden SSID win. */
                    prCandidateStaRec = prPrimaryStaRec;
                    continue;
                }
            }
            else {
                if (prPrimaryBssDesc->fgIsHiddenSSID) {
                    continue;
                }
            }


            //4 <6E> Condition - Choose the one with better RCPI(RSSI).
            if (fgIsFindBestRSSI) {
                /* TODO(Kevin): We shouldn't compare the actual value, we should
                 * allow some acceptable tolerance of some RSSI percentage here.
                 */
                DBGLOG(SCAN, TRACE, ("Candidate ["MACSTR"]: RCPI = %d, Primary ["MACSTR"]: RCPI = %d\n",
                    MAC2STR(prCandidateBssDesc->aucBSSID), prCandidateBssDesc->rRcpi,
                    MAC2STR(prPrimaryBssDesc->aucBSSID), prPrimaryBssDesc->rRcpi));

                ASSERT(!(prCandidateBssDesc->fgIsConnected &&
                         prPrimaryBssDesc->fgIsConnected));

                /* NOTE: To prevent SWING, we do roaming only if target AP has at least 5dBm larger than us. */
                if (prCandidateBssDesc->fgIsConnected) {
                    if (prCandidateBssDesc->rRcpi + ROAMING_NO_SWING_RCPI_STEP <= prPrimaryBssDesc->rRcpi) {

                        prCandidateBssDesc = prPrimaryBssDesc;
                        prCandidateStaRec = prPrimaryStaRec;
                        continue;
                    }
                }
                else if (prPrimaryBssDesc->fgIsConnected) {
                    if (prCandidateBssDesc->rRcpi < prPrimaryBssDesc->rRcpi + ROAMING_NO_SWING_RCPI_STEP) {

                        prCandidateBssDesc = prPrimaryBssDesc;
                        prCandidateStaRec = prPrimaryStaRec;
                        continue;
                    }
                }
                else if (prCandidateBssDesc->rRcpi < prPrimaryBssDesc->rRcpi) {
                    prCandidateBssDesc = prPrimaryBssDesc;
                    prCandidateStaRec = prPrimaryStaRec;
                    continue;
                }
            }

            /* If reach here, that means they have the same Encryption Score, and
             * both RSSI value are close too.
             */
            //4 <6F> Seek the minimum Channel Load for less interference.
            if (fgIsFindMinChannelLoad) {

                /* TODO(Kevin): Check which one has minimum channel load in its channel */
            }
        }
    }


#if DBG
    if (prCandidateBssDesc) {
        DBGLOG(SCAN, TRACE, ("CANDIDATE BSS DESC(%#lx): MAC: "MACSTR", CH: %d, RSSI: %d\n",
            (UINT_32)prCandidateBssDesc, MAC2STR(prCandidateBssDesc->aucBSSID),
            prCandidateBssDesc->ucChannelNum, prCandidateBssDesc->rRcpi));
    }
#endif /* DBG */

    return prCandidateBssDesc;

} /* end of scanSearchBssDescByPolicy() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
P_BSS_DESC_T
scanSearchBssDescByBssid (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 aucBSSID[]
    )
{
    P_SCAN_INFO_T prScanInfo;
    P_LINK_T prBSSDescList;
    P_BSS_DESC_T prBssDesc;

    DEBUGFUNC("scanSearchBssDescByBssid");


    ASSERT(prAdapter);
    ASSERT(aucBSSID);
    prScanInfo = &prAdapter->rScanInfo;
    prBSSDescList = &prScanInfo->rBSSDescList;

    //4 <1> Search BSS Desc from current SCAN result list.
    LINK_FOR_EACH_ENTRY(prBssDesc, prBSSDescList, rLinkEntry, BSS_DESC_T) {

        BSS_DESC_CHK_GUID(prBssDesc);

        if (EQUAL_MAC_ADDR(prBssDesc->aucBSSID, aucBSSID)) {
            return prBssDesc;
        }
    }

    return (P_BSS_DESC_T)NULL;

} /* end of scanSearchBssDescByBssid() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
P_BSS_DESC_T
scanSearchBssDescOrAddIfNotExist (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_BSS_TYPE_T eBSSType,
    IN UINT_8 aucBSSID[],
    IN UINT_8 aucSrcAddr[]
    )
{
    P_SCAN_INFO_T prScanInfo;
    P_LINK_T prBSSDescList;
    P_LINK_T prFreeBSSDescList;
    P_BSS_DESC_T prBssDesc;
    P_BSS_DESC_T prIBssDesc = (P_BSS_DESC_T)NULL;

    DEBUGFUNC("scanSearchBssDescByBssidOrAddIfNotExist");


    ASSERT(prAdapter);
    ASSERT(aucBSSID);
    ASSERT(aucSrcAddr);
    prScanInfo = &prAdapter->rScanInfo;
    prBSSDescList = &prScanInfo->rBSSDescList;
    prFreeBSSDescList = &prScanInfo->rFreeBSSDescList;

    //4 <1> Search BSS Desc from current SCAN result list.
    /* NOTE(Kevin):
     * Rules to maintain the BSSID List:
     * 1. Every BSS/IBSS will map to a single entry of BSSID Descriptor. Because
     *    we report the SCAN result per BSSID(check PARAM_BSSID_EX_T).
     * 2. For Infrastructure - Use same BSSID entry if it exist.
     * 3. For AdHoc -
     *    CASE I ¡V We have TA1(BSSID1), but it change its BSSID to BSSID2
     *              -> Update TA1 entry's BSSID.
     *    CASE II ¡V We have TA1(BSSID1), and get TA1(BSSID1) again
     *              -> Update TA1 entry's contain.
     *    CASE III ¡V We have a SCAN result TA1(BSSID1), and TA2(BSSID2). Sooner or
     *               later, TA2 merge into TA1, we get TA2(BSSID1)
     *              -> Remove TA2 first and then replace TA1 entry's TA with TA2, Still have only one entry of BSSID.
     *    CASE IV ¡V We have a SCAN result TA1(BSSID1), and another TA2 also merge into BSSID1.
     *              -> Replace TA1 entry's TA with TA2, Still have only one entry.
     */
    LINK_FOR_EACH_ENTRY(prBssDesc, prBSSDescList, rLinkEntry, BSS_DESC_T) {

        BSS_DESC_CHK_GUID(prBssDesc);

        if (EQUAL_MAC_ADDR(prBssDesc->aucBSSID, aucBSSID)) {
            if (eBSSType == BSS_TYPE_INFRASTRUCTURE) {
                return prBssDesc;
            }
            else {
                prIBssDesc = prBssDesc;
                break;
            }
        }
    }

    if (eBSSType == BSS_TYPE_IBSS) {
        P_BSS_DESC_T prBSSDescNext;

        LINK_FOR_EACH_ENTRY_SAFE(prBssDesc, prBSSDescNext, prBSSDescList, rLinkEntry, BSS_DESC_T) {

            BSS_DESC_CHK_GUID(prBssDesc);

            if (EQUAL_MAC_ADDR(prBssDesc->aucSrcAddr, aucSrcAddr)) {

                if ((!prIBssDesc) || // CASE I
                    (prBssDesc == prIBssDesc)) { // CASE II

                    return prBssDesc;
                }
                else { // CASE III
                    /* Remove this BSS Desc from the BSS Desc list */
                    LINK_REMOVE_KNOWN_ENTRY(prBSSDescList, prBssDesc);

                    /* Return this BSS Desc to the free BSS Desc list. */
                    LINK_INSERT_TAIL(&prScanInfo->rFreeBSSDescList, &prBssDesc->rLinkEntry);

                    return prIBssDesc;
                }
            }
        }

        if (prIBssDesc) { // CASE IV
            return prIBssDesc;
        }
    }


    //4 <2> No such Record, try to alloc an entry of BSS Desc for this new BSS.
    do {
        LINK_REMOVE_HEAD(prFreeBSSDescList, prBssDesc, P_BSS_DESC_T);
        if (prBssDesc) {
            break;
        }

        //4 <2.1> Hidden is useless, remove the oldest hidden ssid.
        scanRemoveBssDescsByPolicy(prAdapter, (SCAN_RM_POLICY_EXCLUDE_CONNECTED | \
                                               SCAN_RM_POLICY_OLDEST_HIDDEN) );

        //4 <2.2> Try to get an entry again.
        LINK_REMOVE_HEAD(prFreeBSSDescList, prBssDesc, P_BSS_DESC_T);
        if (prBssDesc) {
            break;
        }
        
#if 1
        //4 <2.3> Remove the weakest one
        /* If there are more than half of BSS which has the same ssid as connection
         * setting, remove the weakest one from them. 
         * Else remove the weakest one.
         */
        scanRemoveBssDescsByPolicy(prAdapter, (SCAN_RM_POLICY_EXCLUDE_CONNECTED | \
                                               SCAN_RM_POLICY_SMART_WEAKEST) );

        //4 <2.4> Try to get an entry again.
        LINK_REMOVE_HEAD(prFreeBSSDescList, prBssDesc, P_BSS_DESC_T);
        if (prBssDesc) {
            break;
        }
#else
        //4 <2.3> Remove the weakest one
        scanRemoveBssDescsByPolicy(prAdapter, (SCAN_RM_POLICY_EXCLUDE_CONNECTED | \
                                               SCAN_RM_POLICY_WEAKEST) );

        //4 <2.4> Try to get an entry again.
        LINK_REMOVE_HEAD(prFreeBSSDescList, prBssDesc, P_BSS_DESC_T);
        if (prBssDesc) {
            break;
        }
#endif

        ASSERT(prBssDesc);
    }
    while (FALSE);

    //4 <3> Clear the content of this new SCAN record.
    if (prBssDesc) {
        kalMemZero(prBssDesc, sizeof(BSS_DESC_T));

        BSS_DESC_SET_GUID(prBssDesc);

        //4 <4> Insert this BSS Desc into SCAN result.
        /* Return this BSS Desc to the current BSS Desc list. */
        LINK_INSERT_TAIL(prBSSDescList, &prBssDesc->rLinkEntry);
    }

    return prBssDesc;

} /* end of scanSearchBssDescOrAddIfNotExist() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
scanRemoveBssDescsByPolicy (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32 u4RemovePolicy
    )
{
    P_CONNECTION_SETTINGS_T prConnSettings;
    P_SCAN_INFO_T prScanInfo;
    P_LINK_T prBSSDescList;
    P_BSS_DESC_T prBssDesc;

    DEBUGFUNC("scanRemoveBssDescsByPolicy");


    ASSERT(prAdapter);
    prConnSettings = &prAdapter->rConnSettings;
    prScanInfo =  &prAdapter->rScanInfo;
    prBSSDescList = &prScanInfo->rBSSDescList;

    //DBGLOG(SCAN, TRACE, ("Before Remove - Number Of SCAN Result = %ld\n",
        //prBSSDescList->u4NumElem));

    if (u4RemovePolicy & SCAN_RM_POLICY_TIMEOUT) {
        P_BSS_DESC_T prBSSDescNext;
        OS_SYSTIME rCurrentTime;


        GET_CURRENT_SYSTIME(&rCurrentTime);

        LINK_FOR_EACH_ENTRY_SAFE(prBssDesc, prBSSDescNext, prBSSDescList, rLinkEntry, BSS_DESC_T) {

            BSS_DESC_CHK_GUID(prBssDesc);

            if ((u4RemovePolicy & SCAN_RM_POLICY_EXCLUDE_CONNECTED) &&
                (prBssDesc->fgIsConnected || prBssDesc->fgIsConnecting)) {
                /* Don't remove the one currently we are connected. */
                continue;
            }

            if (CHECK_FOR_TIMEOUT(rCurrentTime, prBssDesc->rUpdateTime,
                SEC_TO_SYSTIME(BSS_DESC_REMOVE_TIMEOUT_SEC))) {

                //DBGLOG(SCAN, TRACE, ("Remove TIMEOUT BSS DESC(%#x): MAC: "MACSTR", Current Time = %08lx, Update Time = %08lx\n",
                    //prBssDesc, MAC2STR(prBssDesc->aucBSSID), rCurrentTime, prBssDesc->rUpdateTime));

                /* Remove this BSS Desc from the BSS Desc list */
                LINK_REMOVE_KNOWN_ENTRY(prBSSDescList, prBssDesc);

                /* Return this BSS Desc to the free BSS Desc list. */
                LINK_INSERT_TAIL(&prScanInfo->rFreeBSSDescList, &prBssDesc->rLinkEntry);
            }
        }
    }
    else if (u4RemovePolicy & SCAN_RM_POLICY_OLDEST) {
        P_BSS_DESC_T prBssDescOldest = (P_BSS_DESC_T)NULL;


        LINK_FOR_EACH_ENTRY(prBssDesc, prBSSDescList, rLinkEntry, BSS_DESC_T) {

            BSS_DESC_CHK_GUID(prBssDesc);

            if ((u4RemovePolicy & SCAN_RM_POLICY_EXCLUDE_CONNECTED) &&
                (prBssDesc->fgIsConnected || prBssDesc->fgIsConnecting)) {
                /* Don't remove the one currently we are connected. */
                continue;
            }

            if (!prBssDescOldest) { /* 1st element */
                prBssDescOldest = prBssDesc;
                continue;
            }

            if (TIME_BEFORE(prBssDesc->rUpdateTime, prBssDescOldest->rUpdateTime)) {
                prBssDescOldest = prBssDesc;
            }
        }

        if (prBssDescOldest) {

            //DBGLOG(SCAN, TRACE, ("Remove OLDEST BSS DESC(%#x): MAC: "MACSTR", Update Time = %08lx\n",
                //prBssDescOldest, MAC2STR(prBssDescOldest->aucBSSID), prBssDescOldest->rUpdateTime));

            /* Remove this BSS Desc from the BSS Desc list */
            LINK_REMOVE_KNOWN_ENTRY(prBSSDescList, prBssDescOldest);

            /* Return this BSS Desc to the free BSS Desc list. */
            LINK_INSERT_TAIL(&prScanInfo->rFreeBSSDescList, &prBssDescOldest->rLinkEntry);
        }
    }
    else if (u4RemovePolicy & SCAN_RM_POLICY_WEAKEST) {
        P_BSS_DESC_T prBssDescWeakest = (P_BSS_DESC_T)NULL;


        LINK_FOR_EACH_ENTRY(prBssDesc, prBSSDescList, rLinkEntry, BSS_DESC_T) {

            BSS_DESC_CHK_GUID(prBssDesc);

            if ((u4RemovePolicy & SCAN_RM_POLICY_EXCLUDE_CONNECTED) &&
                (prBssDesc->fgIsConnected || prBssDesc->fgIsConnecting)) {
                /* Don't remove the one currently we are connected. */
                continue;
            }

            if (!prBssDescWeakest) { /* 1st element */
                prBssDescWeakest = prBssDesc;
                continue;
            }

            if (prBssDesc->rRcpi < prBssDescWeakest->rRcpi) {
                prBssDescWeakest = prBssDesc;
            }
        }

        if (prBssDescWeakest) {

            //DBGLOG(SCAN, TRACE, ("Remove WEAKEST BSS DESC(%#x): MAC: "MACSTR", Update Time = %08lx\n",
                //prBssDescOldest, MAC2STR(prBssDescOldest->aucBSSID), prBssDescOldest->rUpdateTime));

            /* Remove this BSS Desc from the BSS Desc list */
            LINK_REMOVE_KNOWN_ENTRY(prBSSDescList, prBssDescWeakest);

            /* Return this BSS Desc to the free BSS Desc list. */
            LINK_INSERT_TAIL(&prScanInfo->rFreeBSSDescList, &prBssDescWeakest->rLinkEntry);
        }
    }
    else if (u4RemovePolicy & SCAN_RM_POLICY_OLDEST_HIDDEN) {
        P_BSS_DESC_T prBssDescOldest = (P_BSS_DESC_T)NULL;


        LINK_FOR_EACH_ENTRY(prBssDesc, prBSSDescList, rLinkEntry, BSS_DESC_T) {

            BSS_DESC_CHK_GUID(prBssDesc);

            if ((u4RemovePolicy & SCAN_RM_POLICY_EXCLUDE_CONNECTED) &&
                (prBssDesc->fgIsConnected || prBssDesc->fgIsConnecting)) {
                /* Don't remove the one currently we are connected. */
                continue;
            }

            if (!prBssDesc->fgIsHiddenSSID) {
                continue;
            }

            if (!prBssDescOldest) { /* 1st element */
                prBssDescOldest = prBssDesc;
                continue;
            }

            if (TIME_BEFORE(prBssDesc->rUpdateTime, prBssDescOldest->rUpdateTime)) {
                prBssDescOldest = prBssDesc;
            }
        }

        if (prBssDescOldest) {

            //DBGLOG(SCAN, TRACE, ("Remove OLDEST BSS DESC(%#x): MAC: "MACSTR", Update Time = %08lx\n",
                //prBssDescOldest, MAC2STR(prBssDescOldest->aucBSSID), prBssDescOldest->rUpdateTime));

            /* Remove this BSS Desc from the BSS Desc list */
            LINK_REMOVE_KNOWN_ENTRY(prBSSDescList, prBssDescOldest);

            /* Return this BSS Desc to the free BSS Desc list. */
            LINK_INSERT_TAIL(&prScanInfo->rFreeBSSDescList, &prBssDescOldest->rLinkEntry);
        }
    }
    else if (u4RemovePolicy & SCAN_RM_POLICY_SMART_WEAKEST) {
        P_BSS_DESC_T prBssDescWeakest = (P_BSS_DESC_T)NULL;
        P_BSS_DESC_T prBssDescWeakestSameSSID = (P_BSS_DESC_T)NULL;
        UINT_32 u4SameSSIDCount = 0;

        LINK_FOR_EACH_ENTRY(prBssDesc, prBSSDescList, rLinkEntry, BSS_DESC_T) {

            BSS_DESC_CHK_GUID(prBssDesc);

            if ((u4RemovePolicy & SCAN_RM_POLICY_EXCLUDE_CONNECTED) &&
                (prBssDesc->fgIsConnected || prBssDesc->fgIsConnecting)) {
                /* Don't remove the one currently we are connected. */
                continue;
            }

            if ((!prBssDesc->fgIsHiddenSSID) &&
                (EQUAL_SSID(prBssDesc->aucSSID,
                           prBssDesc->ucSSIDLen,
                           prConnSettings->aucSSID,
                           prConnSettings->ucSSIDLen))) {

                u4SameSSIDCount++;
                
                if (!prBssDescWeakestSameSSID) {
                    prBssDescWeakestSameSSID = prBssDesc;
                }
                else if (prBssDesc->rRcpi < prBssDescWeakestSameSSID->rRcpi) {
                    prBssDescWeakestSameSSID = prBssDesc;
                }
            }

            if (!prBssDescWeakest) { /* 1st element */
                prBssDescWeakest = prBssDesc;
                continue;
            }

            if (prBssDesc->rRcpi < prBssDescWeakest->rRcpi) {
                prBssDescWeakest = prBssDesc;
            }
        }

        if ((u4SameSSIDCount >= (CFG_MAX_NUM_BSS_LIST / 2)) &&
            (prBssDescWeakestSameSSID)) {
            prBssDescWeakest = prBssDescWeakestSameSSID;
        }

        if (prBssDescWeakest) {

            //DBGLOG(SCAN, TRACE, ("Remove WEAKEST BSS DESC(%#x): MAC: "MACSTR", Update Time = %08lx\n",
                //prBssDescOldest, MAC2STR(prBssDescOldest->aucBSSID), prBssDescOldest->rUpdateTime));

            /* Remove this BSS Desc from the BSS Desc list */
            LINK_REMOVE_KNOWN_ENTRY(prBSSDescList, prBssDescWeakest);

            /* Return this BSS Desc to the free BSS Desc list. */
            LINK_INSERT_TAIL(&prScanInfo->rFreeBSSDescList, &prBssDescWeakest->rLinkEntry);
        }
    }
    else if (u4RemovePolicy & SCAN_RM_POLICY_ENTIRE) {
        P_BSS_DESC_T prBSSDescNext;

        LINK_FOR_EACH_ENTRY_SAFE(prBssDesc, prBSSDescNext, prBSSDescList, rLinkEntry, BSS_DESC_T) {

            BSS_DESC_CHK_GUID(prBssDesc);

            if ((u4RemovePolicy & SCAN_RM_POLICY_EXCLUDE_CONNECTED) &&
                (prBssDesc->fgIsConnected || prBssDesc->fgIsConnecting)) {
                /* Don't remove the one currently we are connected. */
                continue;
            }

            /* Remove this BSS Desc from the BSS Desc list */
            LINK_REMOVE_KNOWN_ENTRY(prBSSDescList, prBssDesc);

            /* Return this BSS Desc to the free BSS Desc list. */
            LINK_INSERT_TAIL(&prScanInfo->rFreeBSSDescList, &prBssDesc->rLinkEntry);
        }
    }

    //DBGLOG(SCAN, TRACE, ("After Remove - Number Of SCAN Result = %ld\n",
        //prBSSDescList->u4NumElem));

    return;

} /* end of scanRemoveBssDescsByPolicy() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
scanRemoveBssDescByBssid (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 aucBSSID[]
    )
{
    P_SCAN_INFO_T prScanInfo;
    P_LINK_T prBSSDescList;
    P_BSS_DESC_T prBssDesc = (P_BSS_DESC_T)NULL;
    P_BSS_DESC_T prBSSDescNext;

    DEBUGFUNC("scanRemoveBssDescByBssid");


    ASSERT(prAdapter);
    ASSERT(aucBSSID);
    prScanInfo =  &prAdapter->rScanInfo;
    prBSSDescList = &prScanInfo->rBSSDescList;

    /* Check if such BSS Descriptor exists in a valid list */
    LINK_FOR_EACH_ENTRY_SAFE(prBssDesc, prBSSDescNext, prBSSDescList, rLinkEntry, BSS_DESC_T) {

        BSS_DESC_CHK_GUID(prBssDesc);

        if (EQUAL_MAC_ADDR(prBssDesc->aucBSSID, aucBSSID)) {
            /* Remove this BSS Desc from the BSS Desc list */
            LINK_REMOVE_KNOWN_ENTRY(prBSSDescList, prBssDesc);

            /* Return this BSS Desc to the free BSS Desc list. */
            LINK_INSERT_TAIL(&prScanInfo->rFreeBSSDescList, &prBssDesc->rLinkEntry);
            return;
        }
    }

    return;

} /* end of scanRemoveBssDescByBssid() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
scanRemoveConnectionFlagOfBssDescByBssid (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 aucBSSID[]
    )
{
    P_SCAN_INFO_T prScanInfo;
    P_LINK_T prBSSDescList;
    P_BSS_DESC_T prBssDesc = (P_BSS_DESC_T)NULL;

    DEBUGFUNC("scanRemoveConnectionFlagOfBssDescByBssid");


    ASSERT(prAdapter);
    ASSERT(aucBSSID);
    prScanInfo =  &prAdapter->rScanInfo;
    prBSSDescList = &prScanInfo->rBSSDescList;

    /* Check if such BSS Descriptor exists in a valid list */
    if (LINK_IS_VALID(prBSSDescList)) {
        LINK_FOR_EACH_ENTRY(prBssDesc, prBSSDescList, rLinkEntry, BSS_DESC_T) {

            BSS_DESC_CHK_GUID(prBssDesc);

            if (EQUAL_MAC_ADDR(prBssDesc->aucBSSID, aucBSSID)) {
                prBssDesc->fgIsConnected = FALSE;
                prBssDesc->fgIsConnecting = FALSE;
                break;
            }
        }
    }

    return;

} /* end of scanRemoveConnectionFlagOfBssDescByBssid() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
scanProcessBeaconAndProbeResp (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T prSWRfb
    )
{
    P_BSS_DESC_T prBSSDesc = NULL;
    PUINT_8 pucIE = NULL;

    UINT_16 u2Offset = 0;
    P_WLAN_MAC_HEADER_T prWLANHdr;
    P_WLAN_BEACON_FRAME_T prWlanBeaconFrame;

    P_IE_SUPPORTED_RATE_T prIeSupportedRate = (P_IE_SUPPORTED_RATE_T)NULL;
    P_IE_EXT_SUPPORTED_RATE_T prIeExtSupportedRate = (P_IE_EXT_SUPPORTED_RATE_T)NULL;
    UINT_16 u2CapInfo;

    UINT_8 ucHwChannelNum = 0; /* Channel Information from MAC */
    ENUM_BAND_T eHwBand = BAND_24G; /* Channel Information from MAC */

    ENUM_BSS_TYPE_T eBSSType;
    RCPI rRcpi;

    DEBUGFUNC("scanProcessBeaconAndProbeResp");


    ASSERT(prAdapter);
    ASSERT(prSWRfb);
    prWLANHdr = (P_WLAN_MAC_HEADER_T)prSWRfb->pvHeader;
    prWlanBeaconFrame = (P_WLAN_BEACON_FRAME_T)prWLANHdr;

    //DBGLOG_MEM8(RX, TRACE, prWlanBeaconFrame, prSWRfb->u2FrameLength);
    //DBGLOG_MEM32(RX, TRACE, (PUINT_32)prSWRfb->prRxStatus,
        //DWORD_TO_BYTE((prSWRfb->prRxStatus->u2OverallBufferLengthDW & RX_STATUS_BUFFER_LENGTH_MASK) - 1));

    if (prSWRfb->u2FrameLength < (WLAN_MAC_MGMT_HEADER_LEN +
                                  TIMESTAMP_FIELD_LEN +
                                  BEACON_INTERVAL_FIELD_LEN +
                                  CAP_INFO_FIELD_LEN)) {
        ASSERT(0);
        return;
    }

    WLAN_GET_FIELD_16(&prWlanBeaconFrame->u2CapInfo,
                        &u2CapInfo);

    switch (u2CapInfo & CAP_INFO_BSS_TYPE) {
    case CAP_INFO_ESS:
        eBSSType = BSS_TYPE_INFRASTRUCTURE;
        break;

    case CAP_INFO_IBSS:
        eBSSType = BSS_TYPE_IBSS;
        break;

    default:
        DBGLOG(MGT, LOUD, ("Invalid ESS and IBSS bits in Cap Info field\n"));
        return;
    }

    //4 <1> Get a record or create one if it isn't exist.
    prBSSDesc = scanSearchBssDescOrAddIfNotExist(prAdapter,
                                                 eBSSType,
                                                 (PUINT_8)prWlanBeaconFrame->aucBSSID,
                                                 (PUINT_8)prWlanBeaconFrame->aucSrcAddr);
    /* NOTE: Keep consistency of Scan Record during JOIN process */
    if (prBSSDesc->fgIsConnecting) {
        return;
    }

    //4 <2> Get information from Fixed Fields
    COPY_MAC_ADDR(prBSSDesc->aucSrcAddr, prWlanBeaconFrame->aucSrcAddr);

    COPY_MAC_ADDR(prBSSDesc->aucBSSID, prWlanBeaconFrame->aucBSSID);

    WLAN_GET_FIELD_64(&prWlanBeaconFrame->au4Timestamp[0], &prBSSDesc->u8TimeStamp);

    WLAN_GET_FIELD_16(&prWlanBeaconFrame->u2BeaconInterval,
                        &prBSSDesc->u2BeaconInterval);

    prBSSDesc->u2CapInfo = u2CapInfo;
    prBSSDesc->eBSSType = eBSSType;

    prBSSDesc->u2ATIMWindow = 0;
    prBSSDesc->fgIsERPPresent = FALSE;
    prBSSDesc->ucERP = 0x0; /*(UINT_8)NULL; */
    prBSSDesc->ucWmmFlag = 0;
    prBSSDesc->prIEWPA = WPA_IE(NULL);
    prBSSDesc->prIERSN = RSN_IE(NULL);
    prBSSDesc->prIeWmmParamElem = WMM_PARAM_IE(NULL);

    prBSSDesc->u2IELength = prSWRfb->u2FrameLength - WLAN_MAC_HEADER_LEN;

    if (prBSSDesc->u2IELength > CFG_IE_BUFFER_SIZE) {
        prBSSDesc->u2IELength = CFG_IE_BUFFER_SIZE;
    }

    kalMemCopy(prBSSDesc->aucIEBuf,
               (PUINT_8)prSWRfb->pvBody,
               prBSSDesc->u2IELength);

    //4 <3> Get information from IEs
    pucIE = ((P_WLAN_BEACON_FRAME_BODY_T)prBSSDesc->aucIEBuf)->aucInfoElem;

    IE_FOR_EACH(pucIE,
                (prBSSDesc->u2IELength - OFFSET_OF(WLAN_BEACON_FRAME_BODY_T, aucInfoElem)),
                u2Offset) {

        switch (IE_ID(pucIE)) {
        case ELEM_ID_SSID:
            if (IE_LEN(pucIE) <= ELEM_MAX_LEN_SSID) {
                BOOLEAN fgIsHiddenSSID = FALSE;
                UINT_32 i;
                UINT_8 ucSSIDChar = '\0';

                /* D-Link DWL-900AP+ */
                if (IE_LEN(pucIE) == 0) {
                    fgIsHiddenSSID = TRUE;
                }
#if 0 /* NOTE(Kevin 2008/02/25): Which AP ?, we should not include such case in my opinion. */
                else if ((IE_LEN(pucIE) == 1) && (SSID_IE(pucIE)->aucSSID[0] == 0x20)) {
                    fgIsHiddenSSID = TRUE;
                }
#endif
                /* Cisco AP1230A - (IE_LEN(pucIE) == 1) && (SSID_IE(pucIE)->aucSSID[0] == '\0') */
                /* Linksys WRK54G/ASUS WL520g - (IE_LEN(pucIE) == n) && (SSID_IE(pucIE)->aucSSID[0~(n-1)] == '\0') */
                else {
                    for (i = 0; i < IE_LEN(pucIE); i++) {
                        ucSSIDChar |= SSID_IE(pucIE)->aucSSID[i];
                    }

                    if (!ucSSIDChar) {
                        fgIsHiddenSSID = TRUE;
                    }
                }

                /* Update SSID to BSS Descriptor only if SSID is not hidden. */
                if (!fgIsHiddenSSID) {
                    COPY_SSID(prBSSDesc->aucSSID,
                              prBSSDesc->ucSSIDLen,
                              SSID_IE(pucIE)->aucSSID,
                              SSID_IE(pucIE)->ucLength);
                }

                /* Update Flag of Hidden SSID for used in SEARCH STATE. */
                /* NOTE(Kevin): in current driver, the ucSSIDLen == 0 represent
                 * all cases of hidden SSID.
                 * If the fgIsHiddenSSID == TRUE, it means we didn't get the ProbeResp with
                 * valid SSID.
                 */
                if (prBSSDesc->ucSSIDLen == 0) {
                    prBSSDesc->fgIsHiddenSSID = TRUE;
                }
                else {
                    prBSSDesc->fgIsHiddenSSID = FALSE;
                }

            }
            break;

        case ELEM_ID_SUP_RATES:
            /* NOTE(Kevin): Buffalo WHR-G54S's supported rate set IE exceed 8.
             * IE_LEN(pucIE) == 12, "1(B), 2(B), 5.5(B), 6(B), 9(B), 11(B),
             * 12(B), 18(B), 24(B), 36(B), 48(B), 54(B)"
             */
            // if (IE_LEN(pucIE) <= ELEM_MAX_LEN_SUP_RATES) {            
            if (IE_LEN(pucIE) <= RATE_NUM) {
                prIeSupportedRate = SUP_RATES_IE(pucIE);
            }
            break;

        case ELEM_ID_DS_PARAM_SET:
            if (IE_LEN(pucIE) == ELEM_MAX_LEN_DS_PARAMETER_SET) {
                prBSSDesc->ucChannelNum = DS_PARAM_IE(pucIE)->ucCurrChnl;
            }
            break;

        case ELEM_ID_TIM:
            if (IE_LEN(pucIE) <= ELEM_MAX_LEN_TIM) {
                prBSSDesc->ucDTIMPeriod = TIM_IE(pucIE)->ucDTIMPeriod;
            }
            break;

        case ELEM_ID_IBSS_PARAM_SET:
            if (IE_LEN(pucIE) == ELEM_MAX_LEN_IBSS_PARAMETER_SET){
                prBSSDesc->u2ATIMWindow = IBSS_PARAM_IE(pucIE)->u2ATIMWindow;
            }
            break;
#if CFG_SUPPORT_802_11D
        case ELEM_ID_COUNTRY_INFO:
            prBSSDesc->prIECountry = (P_IE_COUNTRY_T)pucIE;
            break;
#endif

        case ELEM_ID_ERP_INFO:
            if (IE_LEN(pucIE) == ELEM_MAX_LEN_ERP) {
                prBSSDesc->fgIsERPPresent = TRUE;
                prBSSDesc->ucERP = ERP_INFO_IE(pucIE)->ucERP;
            }
            break;

        case ELEM_ID_RSN:
            if (IE_LEN(pucIE) <= ELEM_MAX_LEN_RSN_IE) {
            RSN_INFO_T  rRsnInfo;

            prBSSDesc->prIERSN = RSN_IE(pucIE);
            if (rsnParseRsnIE(prBSSDesc->prIERSN, &rRsnInfo)) {
                prBSSDesc->u2RsnCap = rRsnInfo.u2RsnCap;
                rsnCheckPmkidCache(prAdapter, prBSSDesc);
            }
            }
        break;

        case ELEM_ID_EXTENDED_SUP_RATES:
            prIeExtSupportedRate = EXT_SUP_RATES_IE(pucIE);
            break;

#if SUPPORT_WAPI
        case ELEM_ID_WAPI:
            prBSSDesc->prIEWAPI = WAPI_IE(pucIE);
            break;
#endif

        case ELEM_ID_VENDOR:
            /* Try to Get WPA IE */
            {   UINT_8 ucOuiType;
                UINT_16 u2SubTypeVersion;

                if (parseCheckForWFAInfoElem(pucIE, &ucOuiType, &u2SubTypeVersion)) {
                    if ((ucOuiType == VENDOR_OUI_TYPE_WPA) &&
                        (u2SubTypeVersion == VERSION_WPA)) {

                        prBSSDesc->prIEWPA = WPA_IE(pucIE);

                    }
                    /* Setup WMM capability */
                    /* NOTE(Kevin 2007/11/30):
                     * From WMM Specification 1.1
                     *     Beacon will have either a WMM IE or WMM PARAM IE.
                     *     ProbeResp(AP) will have WMM PARAM IE.
                     *     ProbeResp(IBSS) will have WMM PARAM IE if the ProbeReq is
                     *         from the same IBSS.
                     */
                    else if ((ucOuiType == VENDOR_OUI_TYPE_WMM) &&
                             (u2SubTypeVersion == VENDOR_OUI_SUBTYPE_VERSION_WMM_INFO)) {
                        UINT_8 ucWmmFlag = 0;

                        if (qosCheckQoSCapabilityByWMMInfoElem(pucIE, &ucWmmFlag)) {
                            prBSSDesc->ucWmmFlag = ucWmmFlag;
                        }
                    }
                    else if ((ucOuiType == VENDOR_OUI_TYPE_WMM) &&
                             (u2SubTypeVersion == VENDOR_OUI_SUBTYPE_VERSION_WMM_PARAM)) {
                        UINT_8 ucWmmFlag = 0;

                        prBSSDesc->prIeWmmParamElem = WMM_PARAM_IE(pucIE);

                        if (qosCheckQoSCapabilityByWMMParamElem(pucIE, &ucWmmFlag)) {
                            prBSSDesc->ucWmmFlag = ucWmmFlag;
                        }
                    }
                }
            }
            break;

        default:
        break;
        }
    }

    //4 <4> Check information in those IEs.
    if (prIeSupportedRate || prIeExtSupportedRate) {
        rateGetRateSetFromIEs(prIeSupportedRate,
                              prIeExtSupportedRate,
                              &prBSSDesc->u2OperationalRateSet,
                              &prBSSDesc->u2BSSBasicRateSet,
                              &prBSSDesc->fgIsUnknownBssBasicRate);
    }


    /* NOTE(Kevin): The TCL has bug, we'll update fgIsLargerTSF with correct info
     * laterly.
     */
    prBSSDesc->fgIsLargerTSF = NIC_RFB_IS_TCL(prSWRfb);


    /* TODO(Kevin): Add code to check PHY TYPE - OFDM here according to the band info from RFB. */
    if ((prBSSDesc->u2OperationalRateSet & RATE_SET_OFDM) ||
        prBSSDesc->fgIsERPPresent) {
        prBSSDesc->ePhyType = PHY_TYPE_ERP_INDEX;
        prBSSDesc->eBand = BAND_24G;
    }
    else {
        prBSSDesc->ePhyType = PHY_TYPE_HR_DSSS_INDEX;
        prBSSDesc->eBand = BAND_24G;
    }

    /* Need to get the channel/ band information from HW */
    nicHwScanScannedChnlFreqBand(prAdapter,
                                 prSWRfb->prRxStatus->u2QoSCtrl,
                                 &ucHwChannelNum,
                                 &eHwBand);

    /* Update RCPI only for Beacon/ProResp frames received in the right channel */
    rRcpi = NIC_RFB_GET_RCPI(prSWRfb);

    if (prBSSDesc->ucChannelNum == ucHwChannelNum) {

#if 0
        if (prBSSDesc->fgIsConnected) {
            UINT_32 u4Rcpi = (UINT_32)prBSSDesc->rRcpi;


            DBGLOG(SCAN, INFO, ("Org prBSSDesc->rRcpi = %d, rRcpi = %d\n", prBSSDesc->rRcpi, rRcpi));

            u4Rcpi = u4Rcpi * 7 + (UINT_32)rRcpi;
            u4Rcpi = u4Rcpi / 8; // NOTE: Follow the same rule of RR_RCPI_PARM_1_OF_8.

            prBSSDesc->rRcpi = (RCPI)u4Rcpi;

            DBGLOG(SCAN, INFO, ("New prBSSDesc->rRcpi = %d\n", prBSSDesc->rRcpi));

        }
        else
#endif        
        {
            prBSSDesc->rRcpi = rRcpi;
            //DBGLOG(SCAN, INFO, ("prBSSDesc->rRcpi = %d\n", prBSSDesc->rRcpi));  
        }
    }
    else if ((!prBSSDesc->rRcpi) ||
            (rRcpi > prBSSDesc->rRcpi)) {
        /* Update for 1st Time Scan and larger RCPI value even it come from other channel */
        prBSSDesc->rRcpi = rRcpi;
    }

#if 0 // NOTE(Kevin) removed */
    if (prBSSDesc->fgIsConnected) {
        RCPI rRcpi = NIC_RFB_GET_RCPI(prSWRfb);
        UINT_32 u4Rcpi = (UINT_32)prBSSDesc->rRcpi;


        DBGLOG(SCAN, INFO, ("Org prBSSDesc->rRcpi = %d, rRcpi = %d\n", prBSSDesc->rRcpi, rRcpi));

        u4Rcpi = u4Rcpi * 7 + (UINT_32)rRcpi;
        u4Rcpi = u4Rcpi / 8; // NOTE: Follow the same rule of RR_RCPI_PARM_1_OF_8.

        prBSSDesc->rRcpi = (RCPI)u4Rcpi;

        DBGLOG(SCAN, INFO, ("New prBSSDesc->rRcpi = %d\n", prBSSDesc->rRcpi));

    }
    else {

#if CFG_LP_PATTERN_SEARCH_SLT
        if (TIME_AFTER(prBSSDesc->rUpdateTime, prAdapter->rArbInfo.rLastScanRequestTime)) {
            if (prBSSDesc->rRcpi < NIC_RFB_GET_RCPI(prSWRfb)) {
                prBSSDesc->rRcpi = NIC_RFB_GET_RCPI(prSWRfb);
            }
        }
        else {
            prBSSDesc->rRcpi = NIC_RFB_GET_RCPI(prSWRfb);
        }
#else
        prBSSDesc->rRcpi = NIC_RFB_GET_RCPI(prSWRfb);
        //DBGLOG(SCAN, INFO, ("prBSSDesc->rRcpi = %d\n", prBSSDesc->rRcpi));
#endif

    }
#endif

/* NOTE(Kevin 2009/07/15): We need to check following code if need to support abg */
#if CFG_ONLY_802_11A
        /* Need to get the channel/ band information from HW */
        nicHwScanScannedChnlFreqBand(prAdapter,
                                     prSWRfb->prRxStatus->u2QoSCtrl,
                                     &prBSSDesc->ucChannelNum,
                                     &prBSSDesc->eBand);
#endif


    //4 Update BSS_DESC_T's Last Update TimeStamp.
    GET_CURRENT_SYSTIME(&prBSSDesc->rUpdateTime);

    //4 <n> Handle the Beacon/ ProbeResp Frame for a specific Operation Mode.
    if ((prAdapter->eCurrentOPMode == OP_MODE_INFRASTRUCTURE) &&
        (prAdapter->eConnectionState == MEDIA_STATE_CONNECTED)) {
        bssProcessBeacon(prAdapter, prBSSDesc, prSWRfb);
    }
    else if (prAdapter->eCurrentOPMode == OP_MODE_IBSS) {
        /* For the case of IBSS ALONE, the ConnectionState != CONNECTED */
        ibssProcessBeacon(prAdapter, prBSSDesc, prSWRfb);
    }
#if 0 /* TODO(Kevin): for customer */
    else if (prAdapter->eCurrentOPMode == OP_MODE_RESERVED) {
        ASSERT(0);
    }
#endif


#if CFG_LP_IOT
    {
    P_BSS_INFO_T prBssInfo = &prAdapter->rBssInfo;
    OS_SYSTIME rCurrentSysTime;
    if (prAdapter->eConnectionState == MEDIA_STATE_CONNECTED) {
        GET_CURRENT_SYSTIME(&rCurrentSysTime);
        if (EQUAL_MAC_ADDR(prBssInfo->aucBSSID, prBSSDesc->aucBSSID)) {
            if ((prWLANHdr->u2FrameCtrl & MASK_FC_SUBTYPE) == MAC_FRAME_BEACON) {
                DBGLOG(LP_IOT, INFO, ("[LP-IOT] Beacon with the same BSSID is received\n"));
                dumpMemory8(prSWRfb->pvHeader, prSWRfb->u2FrameLength);
            } else {
                DBGLOG(LP_IOT, INFO, ("[LP-IOT] probe response with the same BSSID is received\n"));
                dumpMemory8(prSWRfb->pvHeader, prSWRfb->u2FrameLength);
            }
        } else {
            if ((prWLANHdr->u2FrameCtrl & MASK_FC_SUBTYPE) == MAC_FRAME_BEACON) {
                DBGLOG(LP_IOT, INFO, ("[LP-IOT] Beacon with different BSSID is received\n"));
                dumpMemory8(prSWRfb->pvHeader, prSWRfb->u2FrameLength);
            } else {
                DBGLOG(LP_IOT, INFO, ("[LP-IOT] probe response with different BSSID is received\n"));
                dumpMemory8(prSWRfb->pvHeader, prSWRfb->u2FrameLength);
            }
        }
    }
    }
#endif


    /* indicate PM module for the received of beacon, if it is the 1st time after connected */
    if (prAdapter->fgBeaconReceivedAfterConnected == FALSE) {
        P_BSS_INFO_T prBssInfo = &prAdapter->rBssInfo;
        P_PM_INFO_T prPmInfo = &prAdapter->rPmInfo;

        if ((prAdapter->eConnectionState == MEDIA_STATE_CONNECTED) &&
            /*(prBssInfo->eBSSType == BSS_TYPE_INFRASTRUCTURE) &&*/
            ((prWLANHdr->u2FrameCtrl & MASK_FC_SUBTYPE) == MAC_FRAME_BEACON) &&
            EQUAL_MAC_ADDR(prBssInfo->aucBSSID, prBSSDesc->aucBSSID)) {

            /* Update DTIM period store in BSS Info, which is updated in joinComplete() */
            prBssInfo->ucDtimPeriod = prBSSDesc->ucDTIMPeriod;

            /* inidcate PM module for the connection status */
            pmFsmRunEventOnConnect(prAdapter,
                                   prBssInfo->u2AssocId,
                                   prBssInfo->u2BeaconInterval,
                                   prBssInfo->ucDtimPeriod,
                                   prBssInfo->u2ATIMWindow);

            prAdapter->fgBeaconReceivedAfterConnected = TRUE;

            ARB_CANCEL_TIMER(prAdapter,
                             prPmInfo->rWaitBeaconWatchdogTimer);

        }
    }

    /* This is to ensure at least one beacon is received after beacon lost event (for TSF sync) */
    if (prAdapter->fgWaitOneBeaconForTsfRecovery) {
        P_BSS_INFO_T prBssInfo = &prAdapter->rBssInfo;
        P_PM_INFO_T prPmInfo = &prAdapter->rPmInfo;

        if (((prWLANHdr->u2FrameCtrl & MASK_FC_SUBTYPE) == MAC_FRAME_BEACON) &&
            EQUAL_MAC_ADDR(prBssInfo->aucBSSID, prBSSDesc->aucBSSID)) {

            prAdapter->fgWaitOneBeaconForTsfRecovery = FALSE;

            ARB_CANCEL_TIMER(prAdapter, prPmInfo->rWaitBeaconWatchdogTimer);
        }
    }

#if CFG_WORKAROUND_BG_SSID_SCAN_DONE
    if (IS_ARB_IN_BG_SSID_SCAN_STATE(prAdapter)) {
        P_BG_SCAN_SSID_CANDIDATE_T prBgSsidCandidate;
        UINT_8 i;

        prBgSsidCandidate = &prAdapter->rScanInfo.rScanConfig.rBgScanCfg.rScanCandidate;

        /* Check SSID under BG SSID SCAN state, and signal SCAN DONE interrupt when SSID is matched
        */
        for (i = 0; i < prBgSsidCandidate->ucNumHwSsidScanEntry; i++) {

            if (prBgSsidCandidate->arHwSsidScanEntry[i].u4SsidLen == prBSSDesc->ucSSIDLen) {

                if (!kalMemCmp(prBSSDesc->aucSSID,
                               prBgSsidCandidate->arHwSsidScanEntry[i].aucSsid,
                               prBgSsidCandidate->arHwSsidScanEntry[i].u4SsidLen)) {

                    NIC_ADD_INT_SCAN_DONE_EVENT(prAdapter);
                    break;
                }
            }
        }
    }
#endif


#if (CFG_DBG_BEACON_RCPI_MEASUREMENT || CFG_LP_PATTERN_SEARCH_SLT)
    {
    P_BSS_INFO_T prBssInfo = &prAdapter->rBssInfo;
    if (EQUAL_MAC_ADDR(prBssInfo->aucBSSID, prBSSDesc->aucBSSID)) {
        rxBcnRcpiMeasure(prAdapter, prSWRfb);
    }
    }
#endif

    return;

} /* end of scanProcessBeaconAndProbeResp() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
scanComposeProbeReqFrame (
    IN P_ADAPTER_T  prAdapter,
    IN PUINT_8      pucBuffer,
    IN PUINT_8      pucDesiredSsid,
    IN UINT_32      u4DesiredSsidLen,
    OUT PUINT_32    pu4WlanHeaderLen,
    OUT PUINT_32    pu4WlanBodyLen
    )
{
    P_WLAN_MAC_MGMT_HEADER_T prMacHeader;
    UINT_8 aucDestAddr[] = BC_MAC_ADDR;
    UINT_8 aucBSSID[] = BC_BSSID;
    UINT_8 aucAllSupportedRates[RATE_NUM] = {0};
    UINT_8 ucAllSupportedRatesLen;
    UINT_8 ucSupRatesLen;
    UINT_8 ucExtSupRatesLen;

    PUINT_8 pucFrameBody;
    UINT_32 u4FrameBodyLen;
    UINT_16 u2FrameCtrl = MAC_FRAME_PROBE_REQ;

    DEBUGFUNC("scanComposeProbeReqFrame");


    ASSERT(prAdapter);
    ASSERT(pucBuffer);
    ASSERT(pu4WlanHeaderLen);
    ASSERT(pu4WlanBodyLen);
    prMacHeader = (P_WLAN_MAC_MGMT_HEADER_T)pucBuffer;


    //4 <1> Compose the frame header of the (Re)Association Request  frame.
    /* Fill the Frame Control field. */
    WLAN_SET_FIELD_16(&prMacHeader->u2FrameCtrl, u2FrameCtrl);

    /* Fill the DA field with broadcast address. */
    COPY_MAC_ADDR(prMacHeader->aucDestAddr, aucDestAddr);

    /* Fill the SA field. */
    COPY_MAC_ADDR(prMacHeader->aucSrcAddr, prAdapter->aucMacAddress);

    /* Fill the BSSID field with the desired BSSID. */
    COPY_MAC_ADDR(prMacHeader->aucBSSID, aucBSSID);

    /* Clear the SEQ/FRAG_NO field(HW won't overide the FRAG_NO, so we need to clear it). */
    prMacHeader->u2SeqCtrl = 0;


    //4 <2> Update the MAC header/body length.
    /* Set the MAC header length. */
    *pu4WlanHeaderLen = WLAN_MAC_MGMT_HEADER_LEN;
    u4FrameBodyLen = 0;


    /* Compose the frame body of the Probe Request frame. */
    pucFrameBody = pucBuffer + WLAN_MAC_MGMT_HEADER_LEN;

    /* Fill the SSID element. */
    SSID_IE(pucFrameBody)->ucId = ELEM_ID_SSID;
    SSID_IE(pucFrameBody)->ucLength = (UINT_8) u4DesiredSsidLen;
    if (u4DesiredSsidLen) {
        ASSERT(pucDesiredSsid);
        kalMemCopy(SSID_IE(pucFrameBody)->aucSSID, pucDesiredSsid, u4DesiredSsidLen);
    }
    pucFrameBody += ELEM_HDR_LEN + u4DesiredSsidLen;
    u4FrameBodyLen += ELEM_HDR_LEN + u4DesiredSsidLen;


    /* TODO(Kevin): Maybe we can put the aucSupportedRates in SCAN_INFO */
    rateGetDataRatesFromRateSet(rPhyAttributes[prAdapter->eCurrentPhyType].u2SupportedRateSet,
                                0x0, /*(UINT_16)NULL*/
                                aucAllSupportedRates,
                                &ucAllSupportedRatesLen);


    ucSupRatesLen = ((ucAllSupportedRatesLen > ELEM_MAX_LEN_SUP_RATES) ?
                     ELEM_MAX_LEN_SUP_RATES : ucAllSupportedRatesLen);

    ucExtSupRatesLen = ucAllSupportedRatesLen - ucSupRatesLen;

    /* Fill the Supported Rates element. */
    if (ucSupRatesLen) {
        SUP_RATES_IE(pucFrameBody)->ucId = ELEM_ID_SUP_RATES;
        SUP_RATES_IE(pucFrameBody)->ucLength = ucSupRatesLen;
        kalMemCopy(SUP_RATES_IE(pucFrameBody)->aucSupportedRates,
                   aucAllSupportedRates,
                   ucSupRatesLen);

        pucFrameBody += ELEM_HDR_LEN + ucSupRatesLen;
        u4FrameBodyLen += ELEM_HDR_LEN + ucSupRatesLen;
    }

    /* Fill the Extended Supported Rates element. */
    if (ucExtSupRatesLen) {

        EXT_SUP_RATES_IE(pucFrameBody)->ucId = ELEM_ID_EXTENDED_SUP_RATES;
        EXT_SUP_RATES_IE(pucFrameBody)->ucLength = ucExtSupRatesLen;

        kalMemCopy(EXT_SUP_RATES_IE(pucFrameBody)->aucExtSupportedRates,
                   &aucAllSupportedRates[ucSupRatesLen],
                   ucExtSupRatesLen);

        pucFrameBody += ELEM_HDR_LEN + ucExtSupRatesLen;
        u4FrameBodyLen += ELEM_HDR_LEN + ucExtSupRatesLen;
    }

    *pu4WlanBodyLen = u4FrameBodyLen;

    return;
} /* end of scanComposeProbeReqFrame() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
scanSendProbeReqFrames (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8     pucSsid,
    IN UINT_32     u4SsidLen,
    IN UINT_8      ucNumOfPrbReq,
    IN UINT_8      ucNumOfSpecifiedSsidPrbReq
    )
{
    P_MSDU_INFO_T prMsduInfo;
    P_MGT_PACKET_T prMgtPacket;
    UINT_32 u4WlanHeaderLen;
    UINT_32 u4WlanBodyLen;
    UINT_16 u2EstimatedFrameLen;
    UINT_32 i;
    WLAN_STATUS rStatus = WLAN_STATUS_PENDING;

    DEBUGFUNC("scanSendProbeReqFrames");


    ASSERT(prAdapter);
    if (u4SsidLen) {
        ASSERT(pucSsid);
    }

    DBGLOG(SCAN, INFO, ("ucNumOfPrbReq = %d, ucNumOfSpecifiedSsidPrbReq = %d\n",
        ucNumOfPrbReq, ucNumOfSpecifiedSsidPrbReq));

    ASSERT(ucNumOfPrbReq);

    for (i = 0; i < ucNumOfPrbReq; i++) {
        /* Attempt to allocate a buffer to compose a Probe Request frame. */

        //4 <1> Allocate MSDU_INFO_T
        prMsduInfo = nicTxAllocMsduInfo(prAdapter, TCM);
        if (prMsduInfo == (P_MSDU_INFO_T)NULL) {
#if DBG
            if (i == 0) {
                DBGLOG(SCAN, WARN, ("No buffer to send Probe Req\n"));
            }
            else {
                DBGLOG(SCAN, WARN, ("Not enough buffers to send Probe Req\n"));
            }
#endif
            return WLAN_STATUS_RESOURCES;
        }

        //4 <2> Allocate Frame Buffer (in MGT_PACKET_T) for (Re)Association Frame
        u2EstimatedFrameLen = WLAN_MAC_MGMT_HEADER_LEN + \
                              (ELEM_HDR_LEN + ELEM_MAX_LEN_SSID) + \
                              (ELEM_HDR_LEN + ELEM_MAX_LEN_SUP_RATES) + \
                              /* TODO(Kevin): Request IE */
                              (ELEM_HDR_LEN + (RATE_NUM - ELEM_MAX_LEN_SUP_RATES));

        prMgtPacket = mgtBufAllocateMgtPacket(prAdapter, (UINT_32)u2EstimatedFrameLen);
        if (prMgtPacket == (P_MGT_PACKET_T)NULL) {
            DBGLOG(SCAN, WARN, ("No buffer for Probe Request frame\n"));
            nicTxReturnMsduInfo(prAdapter, prMsduInfo);
            return WLAN_STATUS_RESOURCES;
        }

        //4 <3> Compose Association Request frame in MGT_PACKET_T.
        /* Compose the Probe Request frame. We send the frame with the
         * user-specific SSID first to solve the hidden SSID problem, and
         * send left frames with Broadcast(Wildcard) SSID.
         */
        if (ucNumOfSpecifiedSsidPrbReq > i) {
            scanComposeProbeReqFrame(prAdapter,
                                     MGT_PACKET_GET_BUFFER(prMgtPacket),
                                     pucSsid,
                                     u4SsidLen,
                                     &u4WlanHeaderLen,
                                     &u4WlanBodyLen);
        }
        else {
            UINT_8 aucBcSsid[] = BC_SSID;

            scanComposeProbeReqFrame(prAdapter,
                                     MGT_PACKET_GET_BUFFER(prMgtPacket),
                                     &aucBcSsid[0],
                                     BC_SSID_LEN,
                                     &u4WlanHeaderLen,
                                     &u4WlanBodyLen);
        }

        //4 <4> Update the frame length to the packet descriptor (MGT_PACKET_T).
        mgtPacketPut(prMgtPacket, (u4WlanHeaderLen + u4WlanBodyLen));

        //4 <5> Update information in MSDU_INFO_T for TX Module.
        MSDU_INFO_OBJ_INIT(prMsduInfo, \
                           TRUE, \
                           TRUE, \
                           (PVOID)prMgtPacket, \
                           0, \
                           TXQ_TCM, \
                           (UINT_8)u4WlanHeaderLen, \
                           (UINT_16)u4WlanBodyLen, \
                           (UINT_8)(MSDU_INFO_CTRL_FLAG_SPECIFY_AC | \
                                    MSDU_INFO_CTRL_FLAG_BASIC_RATE | \
                                    MSDU_INFO_CTRL_FLAG_LIFETIME_NEVER_EXPIRE), \
                           (PFN_TX_DONE_HANDLER)0, \
                           (OS_SYSTIME)NULL, \
                           NULL \
                           );

        //4 <6> Inform ARB to send this Authentication Request frame.
        if ((rStatus = arbFsmRunEventTxMmpdu(prAdapter, prMsduInfo)) != WLAN_STATUS_PENDING) {

            if(rStatus != WLAN_STATUS_SUCCESS) {
                mgtBufFreeMgtPacket(prAdapter, prMgtPacket);

                nicTxReturnMsduInfo(prAdapter, prMsduInfo);

                rStatus = WLAN_STATUS_FAILURE;

                break; /* Kevin: Break the outer FOR statement once we encounter ERROR. */
            }

        }
    }

    return rStatus;
} /* end of scanSendProbeReqFrames() */



