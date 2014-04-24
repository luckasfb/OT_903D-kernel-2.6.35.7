







#include "precomp.h"









#if 0 //removed(Kevin)
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
mt592xRFBMorePkt (
    IN P_SW_RFB_T prSWRfb
    )
{
    P_RX_STATUS_T prRxStatus = prSWRfb->prRxStatus;
    //DBGLOG(RX, TRACE, ("mt592xRFBMorePkt = %d\n", RFB_IS_MORE_PKT(prRxStatus->u2StatusFlag)));
    return RFB_IS_MORE_PKT(prRxStatus->u2StatusFlag);

}
#endif

