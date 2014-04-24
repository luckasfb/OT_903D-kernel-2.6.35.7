






#include "precomp.h"







/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
parseCheckForWFAInfoElem (
    IN PUINT_8 pucBuf,
    OUT PUINT_8 pucOuiType,
    OUT PUINT_16 pu2SubTypeVersion
    )
{
    UINT_8 aucWfaOui[] = VENDOR_OUI_WFA;
    P_IE_WFA_T prWfaIE;


    ASSERT(pucBuf);
    ASSERT(pucOuiType);
    ASSERT(pu2SubTypeVersion);
    prWfaIE = (P_IE_WFA_T)pucBuf;


    do {
        if (IE_LEN(pucBuf) <= ELEM_MIN_LEN_WFA_OUI_TYPE_SUBTYPE) {
            //DBGLOG(MGT, INFO, ("Unreasonable WFA IE len: %d\n", IE_LEN(pucBuf)));
            break;
        }
        else if (prWfaIE->aucOui[0] != aucWfaOui[0] ||
                 prWfaIE->aucOui[1] != aucWfaOui[1] ||
                 prWfaIE->aucOui[2] != aucWfaOui[2]) {
            //DBGLOG(MGT, INFO, ("Unreasonable WFA OUI: [%02x %02x %02x]\n",
                //prWfaIE->aucOui[0], prWfaIE->aucOui[1], prWfaIE->aucOui[2]));
            break;
        }

        *pucOuiType = prWfaIE->ucOuiType;
        WLAN_GET_FIELD_16(&prWfaIE->aucOuiSubTypeVersion[0], pu2SubTypeVersion);

        return TRUE;
    }
    while (FALSE);

    return FALSE;

} /* end of parseCheckForWFAInfoElem() */



