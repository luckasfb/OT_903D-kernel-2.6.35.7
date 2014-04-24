






#include "precomp.h"







#if PTA_ENABLED

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
ptaFsmInit (
    IN P_ADAPTER_T prAdapter
    )
{
    P_PTA_INFO_T        prPtaInfo;

    ASSERT(prAdapter);

    prPtaInfo = &prAdapter->rPtaInfo;
    prPtaInfo->eCurrentState = PTA_STATE_IDLE;

    kalMemCopy(prPtaInfo->rBtProfile.u.au4Btcr, &prPtaInfo->rPtaParam, sizeof(PTA_PARAM_T));
#if PTA_NEW_BOARD_DESIGN
    /*  BWCS changes OID structure information. need to specify dual or single setting */
    prPtaInfo->rBtProfile.u.aucBTPParams[BTPPARAM_PTA_MODE_OFFSET] = 
        BTPPARAM_PTA_MODE_VALID | BTPPARAM_PTA_MODE_DUAL;
#endif
    nicPTASetProfile(prAdapter, (P_PTA_PROFILE_T)&prPtaInfo->rBtProfile);

    return;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
ptaFsmSteps (
    IN P_ADAPTER_T      prAdapter,
    IN ENUM_PTA_STATE_T eNextState
    )
{
    P_PTA_INFO_T     prPtaInfo;

    ASSERT(prAdapter);
    prPtaInfo = &prAdapter->rPtaInfo;
    prPtaInfo->eCurrentState = eNextState;
    return;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
ptaFsmRunEventSetConfig (
    IN P_ADAPTER_T          prAdapter,
    IN P_PTA_PARAM_T        prPtaParam
    )
{
    P_PTA_INFO_T        prPtaInfo;
    ENUM_PTA_STATE_T    eNextState;
    P_CONNECTION_SETTINGS_T prConnSettings;
    P_PM_INFO_T                 prPmInfo;


    ASSERT(prAdapter && prPtaParam);
    prPtaInfo = &prAdapter->rPtaInfo;
    eNextState = prPtaInfo->eCurrentState;
    prConnSettings = &prAdapter->rConnSettings;
    prPmInfo = &prAdapter->rPmInfo;

    kalMemCopy(&prConnSettings->rPtaParam, prPtaParam, sizeof(PTA_PARAM_T));
    kalMemCopy(&prPtaInfo->rPtaParam, prPtaParam, sizeof(PTA_PARAM_T));

    if (prPtaInfo->fgEnabled) {
        prPtaParam->u4BtCR0 |= PTA_BTCER0_COEXIST_EN;
    } else {
        prPtaParam->u4BtCR0 &= ~PTA_BTCER0_COEXIST_EN;
    }

    nicPTASetConfig(prAdapter, prPtaParam, &prPtaInfo->u4PTAWireMode);

    switch(prPtaInfo->eCurrentState) {
    case PTA_STATE_IDLE:
        if (prPtaParam->u4BtCR0 & PTA_BTCER0_COEXIST_EN) {
            PTA_STATE_TRANSITION(prAdapter, IDLE, ON);
        }
        break;

    case PTA_STATE_ON:
        if ( !(prPtaParam->u4BtCR0 & PTA_BTCER0_COEXIST_EN)) {
            PTA_STATE_TRANSITION(prAdapter, ON, IDLE);
        }
        break;

    default:
        ASSERT(0);
    }

    /* Call ptaFsmSteps() when we are going to change PTA STATE */
    if (prPtaInfo->eCurrentState != eNextState) {
        ptaFsmSteps(prAdapter, eNextState);
    }

    /* Notify LP module to modify LP instruction */
    NIC_PM_PROGRAM_LP_INSRUCTION(prAdapter, prPmInfo->fgIsContinousPollingEnabled);
#if PTA_NEW_BOARD_DESIGN
    /* no need to specify antenna here. BWCS will set OID */
#else
    /* GeorgeKuo(20090805): new single-antenna design */
    if (prPtaInfo->fgSingleAntenna) {
        nicPtaSetAnt(prAdapter, FALSE);
    }
    else {
        nicPtaSetAnt(prAdapter, TRUE);
    }
#endif
    return;
}

#endif
