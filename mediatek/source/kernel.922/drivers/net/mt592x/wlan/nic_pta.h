




#ifndef _NIC_PTA_H
#define _NIC_PTA_H









VOID
nicPTASetConfig(
    IN  P_ADAPTER_T         prAdapter,
    IN  P_PTA_PARAM_T       prPtaParam,
    OUT PUINT_32            pu4PTAWireMode
    );

VOID
nicPTASetProfile (
    IN  P_ADAPTER_T         prAdapter,
    IN  P_PTA_PROFILE_T     prPtaProfile
    );

VOID
nicPTAUpdateParams (
    IN  P_ADAPTER_T         prAdapter,
    IN  PUINT_8             pBTPParams
    );


VOID
nicPtaGetProfile (
    IN  P_ADAPTER_T         prAdapter,
    IN  PUINT_8             pucBuffer,
    OUT PUINT_32            pu4Count
    );   

VOID
nicPtaSetFunc (
    IN  P_ADAPTER_T     prAdapter,
    IN  BOOL            fgEnabled
    );

VOID
nicPtaSetAnt (
    IN  P_ADAPTER_T     prAdapter,
    IN  BOOL                 fgPrefWiFi
    );
#if PTA_NEW_BOARD_DESIGN
VOID
nicPtaGetAnt (
    IN  P_ADAPTER_T     prAdapter,
    IN  PBOOL           pfgPrefWiFi
    );
#endif
#endif /* _NIC_PTA_H */

