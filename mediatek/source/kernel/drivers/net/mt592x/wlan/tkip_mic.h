





#ifndef _TKIP_MIC_H
#define _TKIP_MIC_H








VOID
tkipMicB (
    IN OUT PUINT_32 pu4L,
    IN OUT PUINT_32 pu4R
    );

VOID
tkipMicGen (
    IN  PUCHAR        pucMickey,
    IN  PUCHAR        pucData,
    IN  UINT_32       u4DataLen,
    IN  PUCHAR        pucSa,
    IN  PUCHAR        pucDa,
    IN  UCHAR         ucPriority,
    OUT PUCHAR        pucMic
    );

VOID
tkipMicEncapsulate (
    IN P_ADAPTER_T       prAdapter,
    IN PUINT_8           pucDa,
    IN PUINT_8           pucSa,
    IN UINT_8            ucPriority,
    IN UINT_16           u2PayloadLen,
    IN PUINT_8           pucPayload,
    IN PUINT_8           pucMic
    );

BOOLEAN
tkipMicDecapsulate (
    IN  P_ADAPTER_T     prAdapter,
    IN  P_SW_RFB_T      prSwRfb,
    OUT PUINT_16        pu2ResultFrameBodyLen
    );

#endif /* _TKIP_MIC_H */


