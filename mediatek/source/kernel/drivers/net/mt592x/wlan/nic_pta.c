






#include "precomp.h"







/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicPTASetConfig (
    IN  P_ADAPTER_T         prAdapter,
    IN  P_PTA_PARAM_T       prPtaParam,
    OUT PUINT_32            pu4PTAWireMode
    )
{
    UINT_32         u4Ioudr, u4Scr, u4Mode, u4BtCR1;
    P_PTA_INFO_T    prPtaInfo;

    DEBUGFUNC("nicPTASetConfig");

    ASSERT(prAdapter && prPtaParam);
    ASSERT(pu4PTAWireMode);

    prPtaInfo = &prAdapter->rPtaInfo;

    *pu4PTAWireMode = PTA_SW_1_WIRE_MODE;

    /* When PTA is disabled, signle ant shall be set to 0 */
    prPtaParam->u4BtCR1 &= ~PTA_BTCER1_SINGLE_ANT;
    if (prPtaInfo->fgEnabled) {
        prPtaParam->u4BtCR0 |= PTA_BTCER0_COEXIST_EN;
    }
    else {
        prPtaParam->u4BtCR0 &= ~PTA_BTCER0_COEXIST_EN;
    }
    u4BtCR1 = prPtaParam->u4BtCR1;

    /* Disable PTA before any change */
    halPTACtrl(prAdapter, FALSE);

    HAL_MCR_RD(prAdapter, MCR_IOUDR, &u4Ioudr);
    HAL_MCR_RD(prAdapter, MCR_SCR, &u4Scr);
    u4Ioudr = (u4Ioudr & ~(IOUDR_BT_PRI_PU | IOUDR_BT_PRI_PD))
                | IOUDR_BT_PRI_PD; /* bt_pr: pull low */

	//Renbang : GPIO0 is used as single ant switch config, we should not config it here
    //u4Scr = u4Scr & ~(SCR_GPIO0_ENABLE_OUTPUT_MODE | SCR_BT_ACT_SEL |
    //        SCR_GPIO1_ENABLE_OUTPUT_MODE | SCR_BTFREQ_SEL); /* gpio0~1: GPI */
    u4Scr = u4Scr & ~( SCR_BT_ACT_SEL | SCR_GPIO1_ENABLE_OUTPUT_MODE | SCR_BTFREQ_SEL); /* gpio0~1: GPI */    

    /* Check if PTA will be in disabled mode */
    if (!prPtaInfo->fgEnabled) {
        HAL_MCR_WR(prAdapter, MCR_IOUDR, u4Ioudr);
        HAL_MCR_WR(prAdapter, MCR_SCR, u4Scr);
       // return;
    }
    else {
    /* Configure GPIO0~1 and BT_PRI based on PTA mode selection
     * SW define these two bits different from HW register definition
     * 2'b00: 1-wire, 2'b01: 2-wire, 2'b10: 3-wire, 2'b11: 4-wire
     */
        u4Mode = u4BtCR1 & PTA_SW_WIRE_MODE;
        prPtaParam->u4BtCR1 &= ~PTA_BTCER1_WIRE_MODE;    /* 1-wire */
        switch (u4Mode) {
        case (PTA_SW_4_WIRE_MODE):    /* 4 wire */
            ASSERT((prAdapter->rEEPROMCtrl.ucDaisyChain & EEPROM_DAISY_GPIO1_MASK)
               == EEPROM_DAISY_GPIO1_UNSPEC);
            /* GeorgeKuo(20090805): new single-antenna design. Single-antenna
             * uses GPIO0 to control antenna switch, which is incompatible with
             * PTA 4-wire mode.
             */
            ASSERT(prPtaInfo->fgSingleAntenna == FALSE);

            prPtaParam->u4BtCR1 |= PTA_BTCER1_4_WIRE_MODE;
            u4Scr = u4Scr | (SCR_BTFREQ_SEL | SCR_BT_ACT_SEL);
            break;

        case (PTA_SW_3_WIRE_MODE):         /* 3 wire (not defined in HW) */
            /* GeorgeKuo(20090805): new single-antenna design. Single-antenna
             * uses GPIO0 to control antenna switch, which is incompatible with
             * PTA 3-wire mode.
             */
            ASSERT(prPtaInfo->fgSingleAntenna == FALSE);
            prPtaParam->u4BtCR1 |= PTA_BTCER1_3_WIRE_MODE;
            u4Scr = u4Scr | SCR_BT_ACT_SEL;
            break;
        case (PTA_SW_2_WIRE_MODE):         /* 2 wire */
            /* GeorgeKuo(20090805): new single-antenna design. Single-antenna
             * uses GPIO0 and GPIO2 to control antenna switch.
             */
#if PTA_NEW_BOARD_DESIGN
            if(prPtaInfo->fgSingleAntenna){
            u4Scr |= SCR_GPIO0_ENABLE_OUTPUT_MODE | SCR_GPIO2_ENABLE_OUTPUT_MODE;
            }
#endif
            prPtaParam->u4BtCR1 |= PTA_BTCER1_2_WIRE_MODE;
            break;
        default:                /* 1 wire (00, 0x defined in HW) */
            /* GeorgeKuo(20090805): new single-antenna design. Single-antenna
             * uses GPIO0 and GPIO2 to control antenna switch.
             */
#if PTA_NEW_BOARD_DESIGN
            if(prPtaInfo->fgSingleAntenna){
            u4Scr |= SCR_GPIO0_ENABLE_OUTPUT_MODE | SCR_GPIO2_ENABLE_OUTPUT_MODE;
            }
#endif
            /* Not pull low/high */
            u4Ioudr &= ~(IOUDR_BT_PRI_PU | IOUDR_BT_PRI_PD);
            break;
        }
        *pu4PTAWireMode = u4Mode;

        HAL_MCR_WR(prAdapter, MCR_IOUDR, u4Ioudr);
        HAL_MCR_WR(prAdapter, MCR_SCR, u4Scr);
    }

    /* Set 4 BT registers including antenna, and PTA is still disabled */
    halPTASetConfig(prAdapter, prPtaParam);
    if (prPtaInfo->fgEnabled) {
        halPTACtrl(prAdapter, TRUE);
    }

    /* Restore input value of u4BtCR1 (important) */
    prPtaParam->u4BtCR1 = u4BtCR1;

#if PTA_NEW_BOARD_DESIGN
    /* ANT_SEL_P_N is no longer used to control ANT switch */
#else
    //4 2008/10/30, mikewu, moved here from fsm_pta.c
    HAL_MCR_RD(prAdapter, MCR_IOPCR, &u4Ioudr);
    if (prPtaInfo->fgSingleAntenna) { /* Single antenna */
        HAL_MCR_WR(prAdapter, MCR_IOPCR,
            u4Ioudr | (IOPCR_IO_ANT_SEL_P_DIR | IOPCR_IO_ANT_SEL_N_DIR));
    }
    else { /* Dual antenna */
        HAL_MCR_WR(prAdapter, MCR_IOPCR,
            u4Ioudr & ~(IOPCR_IO_ANT_SEL_P_DIR | IOPCR_IO_ANT_SEL_N_DIR));
    }
    /* Workaround for single antenna because initial time is long
     * The SW mode of antenna control is set in function nicInitializeChip().
     */
    /* GeorgeKuo(20090805): new single-antenna design. */
    if (prPtaInfo->fgSingleAntenna) { /* Single antenna */
        nicPtaSetAnt(prAdapter, FALSE);
    }
    else {
        nicPtaSetAnt(prAdapter, TRUE);
    }

#endif
    return;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicPTASetProfile (
    IN  P_ADAPTER_T         prAdapter,
    IN  P_PTA_PROFILE_T     prPtaProfile
    )
{
    P_PTA_INFO_T            prPtaInfo;
    PTA_PARAM_T		        rPtaParam;
    P_CONNECTION_SETTINGS_T prConnSetting;

    ASSERT(prAdapter);
    prPtaInfo = &prAdapter->rPtaInfo;
    prConnSetting = &prAdapter->rConnSettings;

    /* Ignore unknown profile */
    if (prPtaProfile->eBtProfile >= BT_PROFILE_NUM) {
        return;
    }

    ASSERT(prPtaProfile);
    kalMemCopy(&prPtaInfo->rBtProfile, prPtaProfile, sizeof(PTA_PROFILE_T));

#if PTA_NEW_BOARD_DESIGN
    /* prPtaProfile->eBtProfile == BT_PROFILE_CUSTOM
     */
    if (prPtaProfile->eBtProfile == BT_PROFILE_CUSTOM) {
        ptaFsmRunEventSetConfig(prAdapter,
                                (P_PTA_PARAM_T)prPtaProfile->u.au4Btcr);
        return;
    }


    ASSERT((prPtaProfile->eBtProfile == BT_PROFILE_ACL) ||
        (prPtaProfile->eBtProfile == BT_PROFILE_MIXED));

    if (prPtaProfile->eBtProfile == BT_PROFILE_ACL) {
        //ASSERT(prPtaProfile->u.aucBTPParams[BTPPARAM_PTA_MODE_OFFSET] & BTPPARAM_PTA_MODE_VALID);

        //Find correct parameters
        if(prPtaProfile->u.aucBTPParams[BTPPARAM_PTA_MODE_OFFSET] & BTPPARAM_PTA_MODE_VALID){
            if((prPtaProfile->u.aucBTPParams[BTPPARAM_PTA_MODE_OFFSET] & BTPPARAM_PTA_MODE_DUAL) == BTPPARAM_PTA_MODE_DUAL){
                kalMemCopy(&rPtaParam, (PVOID)&prPtaInfo->rDualAclPtaParam, sizeof(PTA_PARAM_T));
            }

            if((prPtaProfile->u.aucBTPParams[BTPPARAM_PTA_MODE_OFFSET] & BTPPARAM_PTA_MODE_DUAL) == BTPPARAM_PTA_MODE_SINGLE){
                kalMemCopy(&rPtaParam, (PVOID)&prPtaInfo->rSingleAclPtaParam, sizeof(PTA_PARAM_T));
            }
        }
        kalMemCopy(&prPtaInfo->rBtProfile.u, &rPtaParam, sizeof(PTA_PARAM_T));
        ptaFsmRunEventSetConfig(prAdapter, &rPtaParam);

    }
    else if (prPtaProfile->eBtProfile == BT_PROFILE_MIXED) {
        ASSERT(prPtaProfile->u.aucBTPParams[BTPPARAM_PTA_MODE_OFFSET] & BTPPARAM_PTA_MODE_VALID);

        //Find correct parameters
        if(prPtaProfile->u.aucBTPParams[BTPPARAM_PTA_MODE_OFFSET] & BTPPARAM_PTA_MODE_VALID){
            if((prPtaProfile->u.aucBTPParams[BTPPARAM_PTA_MODE_OFFSET] & BTPPARAM_PTA_MODE_DUAL) == BTPPARAM_PTA_MODE_DUAL){
                kalMemCopy(&rPtaParam, (PVOID)&prPtaInfo->rDualMixPtaParam, sizeof(PTA_PARAM_T));
            }
            if((prPtaProfile->u.aucBTPParams[BTPPARAM_PTA_MODE_OFFSET] & BTPPARAM_PTA_MODE_DUAL) == BTPPARAM_PTA_MODE_SINGLE){
                kalMemCopy(&rPtaParam, (PVOID)&prPtaInfo->rSingleMixPtaParam, sizeof(PTA_PARAM_T));
            }
        }
        kalMemCopy(&prPtaInfo->rBtProfile.u, &rPtaParam, sizeof(PTA_PARAM_T));
        ptaFsmRunEventSetConfig(prAdapter, &rPtaParam);

    }
#else
    if (prPtaProfile->eBtProfile == BT_PROFILE_SCO) {
        //4 2008/10/29, mikewu <todo> enable new VoIP profile
        kalMemCopy(&rPtaParam, (PVOID)&prPtaInfo->rScoPtaParam, sizeof(PTA_PARAM_T));
        kalMemCopy(&prPtaInfo->rBtProfile.u, &rPtaParam, sizeof(PTA_PARAM_T));
        ptaFsmRunEventSetConfig(prAdapter, &rPtaParam);
    }
    else if (prPtaProfile->eBtProfile == BT_PROFILE_ACL) {
        //4 2008/10/29, mikewu <todo> need calculate the quota time limit
        kalMemCopy(&rPtaParam, (PVOID)&prPtaInfo->rAclPtaParam, sizeof(PTA_PARAM_T));
        kalMemCopy(&prPtaInfo->rBtProfile.u, &rPtaParam, sizeof(PTA_PARAM_T));
        ptaFsmRunEventSetConfig(prAdapter, &rPtaParam);
    }
    else if (prPtaProfile->eBtProfile == BT_PROFILE_MIXED) {
        //4 2008/10/29, mikewu <todo> need calculate the quota time limit
        kalMemCopy(&rPtaParam, (PVOID)&prPtaInfo->rMixPtaParam, sizeof(PTA_PARAM_T));
        kalMemCopy(&prPtaInfo->rBtProfile.u, &rPtaParam, sizeof(PTA_PARAM_T));
        ptaFsmRunEventSetConfig(prAdapter, &rPtaParam);
    }
    else {
        /* prPtaProfile->eBtProfile == BT_PROFILE_CUSTOM
         * or BT_PROFILE_NO_CONNECTION
         * BWCS shall issue this profile after end of ACL, SCO and Mix
         */
        if (prPtaProfile->eBtProfile == BT_PROFILE_CUSTOM) {
            ptaFsmRunEventSetConfig(prAdapter,
                                    (P_PTA_PARAM_T)prPtaProfile->u.au4Btcr);
        }
    }

#endif

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicPTAUpdateParams (
    IN  P_ADAPTER_T         prAdapter,
    IN  PUINT_8             pBTPParams
    )
{
    P_PTA_INFO_T prPtaInfo;

    ASSERT(prAdapter);
    ASSERT(pBTPParams);

    prPtaInfo = &prAdapter->rPtaInfo;
    kalMemCopy(prPtaInfo->rBtProfile.u.aucBTPParams, pBTPParams, BT_PROFILE_PARAM_LEN);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicPtaGetProfile (
    IN  P_ADAPTER_T     prAdapter,
    IN  PUINT_8         pucBuffer,
    OUT PUINT_32        pu4Count
    )
{
    P_PTA_INFO_T        prPtaInfo;
    P_PTA_PROFILE_T     prPtaProfileBuf;
    P_PTA_PROFILE_T     prPtaProfile;

    ASSERT(prAdapter && pucBuffer);

    ASSERT(pu4Count);

    prPtaInfo = &prAdapter->rPtaInfo;
    prPtaProfileBuf = (P_PTA_PROFILE_T)pucBuffer;
    prPtaProfile = &prPtaInfo->rBtProfile;

    prPtaProfileBuf->eBtProfile = prPtaProfile->eBtProfile;

    kalMemCopy(&prPtaProfileBuf->u, &prPtaProfile->u, sizeof(PTA_PARAM_T));

    *pu4Count = sizeof(PTA_PROFILE_T);

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicPtaSetFunc (
    IN  P_ADAPTER_T     prAdapter,
    IN  BOOL            fgEnabled
    )
{
    P_PTA_INFO_T        prPtaInfo;

    ASSERT(prAdapter);

    prPtaInfo = &prAdapter->rPtaInfo;

    if (fgEnabled) {
        prPtaInfo->fgEnabled = TRUE;
    }
    else {
        prPtaInfo->fgEnabled = FALSE;
    }
    ptaFsmRunEventSetConfig(prAdapter, &prPtaInfo->rPtaParam);
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicPtaSetAnt (
    IN  P_ADAPTER_T     prAdapter,
    IN  BOOL                 fgPrefWiFi
    )
{
    P_PTA_INFO_T        prPtaInfo;
    UINT_32                 u4ScrValue;

    ASSERT(prAdapter);

    prPtaInfo = &prAdapter->rPtaInfo;

    if (prAdapter->rPtaInfo.fgSingleAntenna == FALSE) {
        /* There is no need to set preferred antenna in dual antenna mode */
        return;
    }

    HAL_MCR_RD(prAdapter, MCR_SCR, &u4ScrValue);

    ASSERT(prAdapter->rPtaInfo.u4PTAWireMode != PTA_SW_3_WIRE_MODE);
    ASSERT(prAdapter->rPtaInfo.u4PTAWireMode != PTA_SW_4_WIRE_MODE);

    u4ScrValue &= ~(SCR_BT_ACT_SEL|
                   SCR_GPIO0_ENABLE_OUTPUT_MODE   |
                   SCR_GPIO0_CHAIN_SEL |
                   SCR_GPIO0_WDATA |
                   SCR_GPIO2_ENABLE_OUTPUT_MODE |
                   SCR_GPIO2_CHAIN_SEL |
                   SCR_GPIO2_WDATA);

    /* Set GPIO0 and GPIO2 as output */
    u4ScrValue |= SCR_GPIO0_ENABLE_OUTPUT_MODE | SCR_GPIO2_ENABLE_OUTPUT_MODE;

    if (fgPrefWiFi) {
        u4ScrValue |= SCR_GPIO0_WDATA;
    }
    else {
        u4ScrValue |= SCR_GPIO2_WDATA;
    }
    HAL_MCR_WR(prAdapter, MCR_SCR, u4ScrValue);
}
#if PTA_NEW_BOARD_DESIGN
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicPtaGetAnt (
    IN  P_ADAPTER_T     prAdapter,
    IN  PBOOL           pfgPrefWiFi
    )
{
    UINT_32 u4ScrValue;

    ASSERT(prAdapter);

    ASSERT(prAdapter->rPtaInfo.fgSingleAntenna);
    ASSERT(prAdapter->rPtaInfo.u4PTAWireMode != PTA_SW_3_WIRE_MODE);
    ASSERT(prAdapter->rPtaInfo.u4PTAWireMode != PTA_SW_4_WIRE_MODE);

    if (prAdapter->rPtaInfo.fgSingleAntenna == FALSE) {
        /* There is no need to set preferred antenna in dual antenna mode */
        //RETAILMSG(TRUE, (L"nicPtaGetAnt queried in dual antenna \n"));
        *pfgPrefWiFi = TRUE;
        return;
    }

    HAL_MCR_RD(prAdapter, MCR_SCR, &u4ScrValue);

    u4ScrValue &=  SCR_GPIO0_WDATA | SCR_GPIO2_WDATA;

    if(u4ScrValue == SCR_GPIO0_WDATA){
        *pfgPrefWiFi = TRUE;
    }else if(u4ScrValue == SCR_GPIO2_WDATA){
        *pfgPrefWiFi = FALSE;
    }else{
        //RETAILMSG(TRUE, (L"nicPtaGetAnt strange 0x%08x\n", u4ScrValue));
    }

    return;
}

#endif
