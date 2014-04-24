






#include "precomp.h"








/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halWlanTableInit (
    IN  P_ADAPTER_T          prAdapter
    )
{
    WLAN_ENTRY_CTRL_T        rWTEntry;
    UINT_8                   i;

    DEBUGFUNC("halWlanTableInit");

    ASSERT(prAdapter);
    
    kalMemZero((PVOID)&rWTEntry, sizeof(WLAN_ENTRY_CTRL_T));

    for (i = 0; i < WLAN_TABLE_SIZE; i++) {
        if (!halWlanTableAccess(prAdapter, (PVOID)&rWTEntry, i, HWTDR_UPDATE_MODE_0)) {
            DBGLOG(RSN, TRACE, ("Wlan Table Initial failed\n"));
        }
    }

}/* halWlanTableInit */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
halWlanTableBusyStatus (
    IN P_ADAPTER_T          prAdapter
    )
{
    UINT_32 u4temp, i;

    DEBUGFUNC("halWlanTableBusyStatus");

    ASSERT(prAdapter);
    
    kalUdelay(3);

    HAL_MCR_RD(prAdapter, MCR_CIR, &u4temp);

    if (WLAN_WT_IS_BUSY(u4temp)){
        for (i=0; i< 10; i++) {
            kalUdelay(3);
            HAL_MCR_RD(prAdapter, MCR_CIR, &u4temp);
            if (WLAN_WT_IS_BUSY(u4temp)){
                DBGLOG(HAL, INFO, ("Busy (%08lx)!!!!\n", u4temp));
            }
            else {
                DBGLOG(HAL, INFO, ("ok (%08lx)!!!!\n", u4temp));
                break;
            }
        }
        if (i==10) {
            DBGLOG(HAL, TRACE, ("Wlan Table is busy!!!!\n"));
            return FALSE;
        }
    }

    return TRUE;
} /* halWlanTableBusyStatus */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
halWlanTableAccess (
    IN  P_ADAPTER_T         prAdapter,
    IN  PVOID               pvWlanCtrl,
    IN  UCHAR               ucEntryIndex,
    IN  UCHAR               ucMode
    )
{
    HW_WLAN_TABLE_T         rWKey;
    P_WLAN_ENTRY_CTRL_T     prWlanCtrl;
    P_SW_KEY_STRUC_T        prSwKey;
    P_WT_CTRL_STRUC_T       prCtrl;
    UINT_32                 u4Ctrl = 0;
    UINT_8                  ucUpdateLen = 0;
    BOOLEAN                 fgResult = TRUE;

    DEBUGFUNC("halWlanTableAccess");

    ASSERT(prAdapter);
    ASSERT(pvWlanCtrl);
    
    if (pvWlanCtrl == NULL ||
        ucEntryIndex > WLAN_TABLE_SIZE || /* max wlan table entry */
        ucMode > HWTDR_UPDATE_MODE_3 /* max wlan table update mode */) {
        ASSERT(FALSE);
    }

    prWlanCtrl = (P_WLAN_ENTRY_CTRL_T)pvWlanCtrl;

    prSwKey = &prWlanCtrl->rSWKey;
    prCtrl = &prWlanCtrl->rCtrl;

    switch (ucMode) {
    case HWTDR_UPDATE_MODE_0:
        ucUpdateLen = HWTDR_UPDATE_MODE_0_SZ;
        break;
    case HWTDR_UPDATE_MODE_1:
        ucUpdateLen = HWTDR_UPDATE_MODE_1_SZ;
        break;
    case HWTDR_UPDATE_MODE_2:
        ucUpdateLen = HWTDR_UPDATE_MODE_2_SZ;
        break;
    case HWTDR_UPDATE_MODE_3:
        ucUpdateLen = HWTDR_UPDATE_MODE_3_SZ;
        break;
    default :
        break;
    }

    ASSERT(prSwKey && prCtrl);

    u4Ctrl = u4Ctrl | (ucEntryIndex & MASK_HWTDR_TINDEX) | ((ucMode & BITS(0,1)) << HWTDR_MODE_OFFSET);
    u4Ctrl = u4Ctrl | (prCtrl->fgTV << HWTDR_TV_OFFSET);
    u4Ctrl = u4Ctrl | (prCtrl->fgTKV << HWTDR_TKV_OFFSET);
#if 1 /* Always use SW Port Control */
    u4Ctrl = u4Ctrl | (0 << HWTDR_T1X_OFFSET);
#endif
    u4Ctrl = u4Ctrl | (prCtrl->fgRV << HWTDR_RV_OFFSET);
    u4Ctrl = u4Ctrl | (prCtrl->fgRKV << HWTDR_RKV_OFFSET);
    u4Ctrl = u4Ctrl | ((prSwKey->ucRCA1 & BITS(0,1)) << HWTDR_RCA1_OFFSET);
    u4Ctrl = u4Ctrl | (prSwKey->fgRCA2 << HWTDR_RCA2_OFFSET);
    u4Ctrl = u4Ctrl | (prSwKey->fgRCID << HWTDR_RCID_OFFSET);
#if 1 /* Always use SW Port Control */
    u4Ctrl = u4Ctrl | (0 << HWTDR_R1X_OFFSET);
#endif
    u4Ctrl = u4Ctrl | (prCtrl->fgQoS << HWTDR_Q_OFFSET);
    u4Ctrl = u4Ctrl | (prCtrl->fgAntenna << HWTDR_A_OFFSET);
    u4Ctrl = u4Ctrl | (prSwKey->fgIKV << HWTDR_IKV_OFFSET);
    u4Ctrl = u4Ctrl | ((prSwKey->ucCipher & BITS(0,3)) << HWTDR_CIPHER_OFFSET);
    u4Ctrl = u4Ctrl | ((prSwKey->ucKeyId & BITS(0,1)) << HWTDR_KID_OFFSET);

    rWKey.u4Ctrl = u4Ctrl;

    if (ucMode >= HWTDR_UPDATE_MODE_1) {
        rWKey.u4Rate = prCtrl->ucRate1 |
                      (prCtrl->ucRate2 << 6) |
                      (prCtrl->ucRate3 << 12) |
                      (prCtrl->ucRate4 << 18);
    }

    if (ucMode >= HWTDR_UPDATE_MODE_2) {
        rWKey.ucMUAR_ID = (0 & BITS(0,5));
        rWKey.ucReserved = 0;
        kalMemCopy(rWKey.aucAddr, prSwKey->aucAddr, MAC_ADDR_LEN);
    }
    if (ucMode == HWTDR_UPDATE_MODE_3) {
        kalMemCopy(&rWKey.aucKey[0], prSwKey->aucKeyMaterial, TKIP_KEY_LEN);
    }

    if (halWlanTableBusyStatus(prAdapter)== FALSE) {
        ASSERT(FALSE);
    }

    HAL_PORT_WR(prAdapter,
                MCR_HWTDR,
               (UINT_16)ucUpdateLen,
               (PUCHAR)&rWKey,
               (UINT_16)ucUpdateLen);

    if (!fgResult) {
        goto BUS_ERROR;
    }

    if (halWlanTableBusyStatus(prAdapter)== FALSE) {
        ASSERT(FALSE);
    }

    if (halWlanTableVerify(prAdapter,
                (PUINT_8)&rWKey,
                ucEntryIndex,
                ucUpdateLen) == FALSE){
        goto BUS_ERROR;
    }

    return TRUE;
BUS_ERROR:
    DBGLOG(HAL, ERROR, ("Write bus error\n"));

    return FALSE;
} /* halWlanTableAccess */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
halWlanTableVerify (
    IN  P_ADAPTER_T         prAdapter,
    IN  PUINT_8             pucWKey,
    IN  UCHAR               ucEntryIndex,
    IN  UCHAR               ucLen
    )
{
    UINT_32         u4Value = 0;
    UINT_8          ucTmpWlanEntry[HWTDR_UPDATE_MODE_3_SZ];
    BOOLEAN         fgResult = TRUE;

    DEBUGFUNC("halWlanTableVerify");

    ASSERT(prAdapter);
    ASSERT(pucWKey);
    
    if (ucEntryIndex > WLAN_TABLE_SIZE /* max wlan table entry */ ||
        ucLen > HWTDR_UPDATE_MODE_3_SZ /* max wlan table entry len */) {
        ASSERT(FALSE);
    }

    u4Value = ucEntryIndex;
    u4Value |= HWTCR_W_RMODE;

    HAL_MCR_WR(prAdapter, MCR_HWTCR, u4Value);

    HAL_PORT_RD(prAdapter, MCR_HWTDR, ucLen,
                (PUCHAR)&ucTmpWlanEntry[0], ucLen);

    if (!fgResult) {
        return FALSE;
    }

    if ((kalMemCmp(&pucWKey[1], &ucTmpWlanEntry[1], ucLen - 1) != 0)) {
        DBGLOG(HAL, ERROR, ("Wlan table verified fail!!\n"));
        DBGLOG_MEM8(HAL, TRACE, (PUINT_8)&pucWKey[0], ucLen);
        DBGLOG(HAL, TRACE, ("=============================================\n"));
        DBGLOG_MEM8(HAL, TRACE, (PUINT_8)&ucTmpWlanEntry[0], ucLen);
        ASSERT(FALSE);
        return FALSE;
    }
    return TRUE;

} /* halWlanTableVerify */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halWlanTableDump(
    IN  P_ADAPTER_T         prAdapter,
    IN  UCHAR               ucEntryIndex
    )
{
    UINT_32         u4Value = 0;
#if DBG
    UINT_8          ucTmpWlanEntry[HWTDR_UPDATE_MODE_3_SZ];
#endif /* DBG */

    DEBUGFUNC("halWlanTableDump");

    ASSERT(prAdapter);
    
    if (ucEntryIndex > WLAN_TABLE_SIZE /* max wlan table entry */ ) {
        DBGLOG(HAL, WARN, ("Not support this entry index \n"));
    }

    u4Value = ucEntryIndex;
    u4Value |= HWTCR_W_RMODE;

    HAL_MCR_WR(prAdapter, MCR_HWTCR, u4Value);

    DBGLOG(RSN, WARN, ("Wlan table index %d\n", ucEntryIndex));
    DBGLOG_MEM8(TX, TRACE, (PUINT_8)&ucTmpWlanEntry[0], HWTDR_UPDATE_MODE_3_SZ);

}

