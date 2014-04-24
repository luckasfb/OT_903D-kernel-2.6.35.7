
#include "precomp.h"

int 
wpi_encrypt(
    UINT_8 * pofbiv_in,
    UINT_8 * pbw_in,
    UINT_32 plbw_in,
    UINT_8 * pkey,
    UINT_8 * pcw_out
    );

int 
wpi_decrypt(
    UINT_8 * pofbiv_in,
    UINT_8 * pcw_in,
    UINT_32 plcw_in,
    UINT_8 * prkey_in,
    UINT_8 * pbw_out
    );

int 
wpi_pmac(
    UINT_8 * pmaciv_in,
    UINT_8 * pmac_in,
    UINT_32 pmacpc_in,
    UINT_8 * pkey,
    UINT_8 * pmac_out
    );

void
wpi_mic_compose (
    BOOLEAN               fgDir,
    UINT_8               ucKeyIdx,
    UINT_16              u2PayloadLen,
    P_WLAN_MAC_HEADER_QOS_T prMacHdr,
    PUINT_8              pucMicHdr,
    BOOLEAN              fgQoS
    );

