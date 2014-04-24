






#include "precomp.h"







#if PTA_ENABLED

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halPTACtrl (
    IN  P_ADAPTER_T         prAdapter,
    IN  BOOLEAN             fgEnable
    )
{
    UINT_32         u4tmp;

    DEBUGFUNC("halPTACtrl");

    ASSERT(prAdapter);

    HAL_MCR_RD(prAdapter, MCR_BTCER0, &u4tmp);
    if (fgEnable) {
        u4tmp |= PTA_BTCER0_COEXIST_EN;
    }
    else {
        u4tmp &= ~PTA_BTCER0_COEXIST_EN;
    }

    HAL_MCR_WR(prAdapter, MCR_BTCER0, u4tmp);

    return;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halPTASetConfig (
    IN  P_ADAPTER_T     prAdapter,
    IN  P_PTA_PARAM_T   prPtaParam
    )
{
    UINT_32         u4tmp;

    DEBUGFUNC("halPTASetConfig");

    ASSERT(prAdapter && prPtaParam);

    u4tmp = (prPtaParam->u4BtCR0) & ~PTA_BTCER0_COEXIST_EN;
    HAL_MCR_WR(prAdapter, MCR_BTCER0, u4tmp);
    HAL_MCR_WR(prAdapter, MCR_BTCER1, prPtaParam->u4BtCR1);
    HAL_MCR_WR(prAdapter, MCR_BTCER2, prPtaParam->u4BtCR2);
    HAL_MCR_WR(prAdapter, MCR_BTCER3, prPtaParam->u4BtCR3);

    return;
}

#endif

