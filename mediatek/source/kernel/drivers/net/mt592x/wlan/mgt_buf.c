






#include "precomp.h"







/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
mgtBufInitialize (
    IN P_ADAPTER_T prAdapter
    )
{
    P_MGT_BUF_INFO_T prMgtBufInfo;
    PUINT_8 pucMemHandle;
    P_MGT_PACKET_T prMgtPacket;
    UINT_32 i;


    ASSERT(prAdapter);
    prMgtBufInfo = &prAdapter->rMgtBufInfo;

    //4 <0> Clear allocated memory.
    kalMemZero((PVOID) prMgtBufInfo->pucMgtBufCached, prMgtBufInfo->u4MgtBufCachedSize);

    ASSERT(IS_ALIGN_4((UINT_32)prMgtBufInfo->pucMgtBufCached));
    prMgtBufInfo->pucMgtBufPoolCached = prMgtBufInfo->pucMgtBufCached +
                                        MGT_PACKET_DESCRIPTORS_SIZE;


    /* Initial MGT Buffer allocation lookup table, for example:
     * 5 memory blocks are represented as 00011111'B.
     * = (BIT(5) - 1) (but we'll encounter a problem of allocating 32 blocks in this formula)
     * = (BIT(4) | (BIT(4)-1))
     */
    for (i = 0; i <= MAX_NUM_OF_MGT_BUF_BLOCKS; i++) {
        prMgtBufInfo->rMgtBufBlocksToBitsTable[i] =
            (BUF_BITMAP)(i ? (BIT(i-1) | (BIT(i-1) - 1)) : 0);
    }

    /* Setup available memory blocks.(8 memory blocks = 11111111'B */
    prMgtBufInfo->rFreeMgtBufBlocksBitmap =
        (BUF_BITMAP)(BIT(MAX_NUM_OF_MGT_BUF_BLOCKS-1) | (BIT(MAX_NUM_OF_MGT_BUF_BLOCKS-1) - 1));


    pucMemHandle = prMgtBufInfo->pucMgtBufCached;

    QUEUE_INITIALIZE(&prMgtBufInfo->rFreeMgtPacketList);
    for (i = 0; i < MAX_NUM_OF_MGT_PACKETS; i++) {
        prMgtPacket = (P_MGT_PACKET_T)pucMemHandle;

        prMgtPacket->rAllocatedMgtBufBlocksBitmap = 0;
        QUEUE_INSERT_TAIL(&prMgtBufInfo->rFreeMgtPacketList, &prMgtPacket->rQueEntry);

        pucMemHandle += ALIGN_4(sizeof(MGT_PACKET_T));
    }

    /* Check if the memory allocation consist with this initialization function */
    ASSERT(pucMemHandle == prMgtBufInfo->pucMgtBufPoolCached);

    return;

} /* end of mgtBufInitialize() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
P_MGT_PACKET_T
mgtBufAllocateMgtPacket (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32 u4Length
    )
{
    P_MGT_BUF_INFO_T prMgtBufInfo;
    UINT_32 u4BlockNum;
    BUF_BITMAP rRequiredBitmap = 0;
    P_MGT_PACKET_T prMgtPacket = (P_MGT_PACKET_T)NULL;
    PUINT_8 pucBuffer;
    UINT_32 i;

    DEBUGFUNC("mgtBufAllocateMgtPacket");


    ASSERT(prAdapter);
    prMgtBufInfo = &prAdapter->rMgtBufInfo;

    do {

        /* No available MGMT PACKET Descriptor */
        if (QUEUE_IS_EMPTY(&prMgtBufInfo->rFreeMgtPacketList)) {
            break;
        }

        ASSERT(u4Length);

        u4Length += MGT_PACKET_RESERVED_SIZE; /* reserved space */
        u4Length += (MGT_BUF_BLOCK_SIZE - 1);
        u4BlockNum = u4Length >> MGT_BUF_BLOCK_SIZE_IN_POWER_OF_2;
        /* Will use shift instruction if denominator is power of 2 */

        ASSERT(u4BlockNum <= MAX_NUM_OF_MGT_BUF_BLOCKS);

        if ((u4BlockNum > 0) && (u4BlockNum <= MAX_NUM_OF_MGT_BUF_BLOCKS)) {

            /* Convert number of block into bit cluster */
            rRequiredBitmap = prMgtBufInfo->rMgtBufBlocksToBitsTable[u4BlockNum];

            for (i = 0;
                 i <= (MAX_NUM_OF_MGT_BUF_BLOCKS - u4BlockNum);
                 i++ , rRequiredBitmap <<= 1) {

                /* Have available memory blocks */
                if ((prMgtBufInfo->rFreeMgtBufBlocksBitmap & rRequiredBitmap) == rRequiredBitmap) {

                    QUEUE_REMOVE_HEAD(&prMgtBufInfo->rFreeMgtPacketList, prMgtPacket, P_MGT_PACKET_T);
                    if (!prMgtPacket) {
                        DBGLOG(MGT, ERROR, ("Free MGT Packet Queue Error !\n"));
                        break;
                    }

                    /* Clear corresponding bits of allocated memory blocks */
                    prMgtBufInfo->rFreeMgtBufBlocksBitmap &= ~rRequiredBitmap;

#if CFG_TX_DBG_MGT_BUF
                    /* Update maximux allocated depth */
                    if (prMgtBufInfo->ucMaxAllocatedDepth < (UINT_8)(u4BlockNum + i)) {
                        prMgtBufInfo->ucMaxAllocatedDepth = (UINT_8)(u4BlockNum + i);
                    }
#endif /* CFG_TX_DBG_MGT_BUF */

                    /* Store corresponding bits of allocated memory blocks */
                    prMgtPacket->rAllocatedMgtBufBlocksBitmap = rRequiredBitmap;

                    /* Return the start address of allocated memory */
                    pucBuffer = prMgtBufInfo->pucMgtBufPoolCached + \
                                MGT_BUF_BLOCKS_SIZE(i);

                    /* Setup initial value in MGMT_PACKET_T */
                    prMgtPacket->pucHead = pucBuffer;
                    prMgtPacket->pucEnd = pucBuffer + \
                                          MGT_BUF_BLOCKS_SIZE(u4BlockNum);
                    prMgtPacket->pucData = pucBuffer + MGT_PACKET_RESERVED_SIZE;
                    prMgtPacket->pucTail = prMgtPacket->pucData;

                    break;
                }
            }

        }

    }
    while (FALSE);

    return prMgtPacket;

} /* end of mgtBufAllocateMgtPacket() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
mgtBufFreeMgtPacket (
    IN P_ADAPTER_T prAdapter,
    IN P_MGT_PACKET_T prMgtPacket
    )
{
    P_MGT_BUF_INFO_T prMgtBufInfo

    DEBUGFUNC("mgtBufFreeMgtPacket");


    ASSERT(prAdapter);
    ASSERT(prMgtPacket);
    prMgtBufInfo = &prAdapter->rMgtBufInfo;

    if (prMgtPacket) {

        /* Memory should not overlap */
        ASSERT((prMgtBufInfo->rFreeMgtBufBlocksBitmap & \
            prMgtPacket->rAllocatedMgtBufBlocksBitmap) == 0);

        /* Set corresponding bits of released memory blocks */
        prMgtBufInfo->rFreeMgtBufBlocksBitmap |= prMgtPacket->rAllocatedMgtBufBlocksBitmap;

        /* Clear corresponding bits of released memory blocks */
        prMgtPacket->rAllocatedMgtBufBlocksBitmap = 0;

        QUEUE_INSERT_TAIL(&prMgtBufInfo->rFreeMgtPacketList, &prMgtPacket->rQueEntry);
    }

    return;

} /* end of mgtBufFreeMgtPacket() */


#if CFG_TX_DBG_MGT_BUF
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
mgtBufQueryStatus (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8 pucBuffer,
    OUT PUINT_32 pu4Count
    )
{
    P_MGT_BUF_INFO_T prMgtBufInfo;
    PUINT_8 pucCurrBuf = pucBuffer;
    UINT_32 u4FreeMgtBufBlocks = 0;
    UINT_32 i;


    ASSERT(prAdapter);
    prMgtBufInfo = &prAdapter->rMgtBufInfo;
    if (pucBuffer) {} /* For Windows, we'll print directly instead of sprintf() */
    ASSERT(pu4Count);

    SPRINTF(pucCurrBuf, ("\n\nMGT BUFFER STATUS:"));
    SPRINTF(pucCurrBuf, ("\n=================="));
    SPRINTF(pucCurrBuf, ("\nFREE MGMT PACKET LIST         :%8ld",
        prMgtBufInfo->rFreeMgtPacketList.u4NumElem));


    for (i = 0; i < MAX_NUM_OF_MGT_BUF_BLOCKS; i++) {
        if (prMgtBufInfo->rFreeMgtBufBlocksBitmap & BIT(i)) {
            u4FreeMgtBufBlocks++;
        }
    }

    SPRINTF(pucCurrBuf, ("\nFREE MGMT BUFFER SIZE(bytes)  :%8ld",
        u4FreeMgtBufBlocks * MGT_BUF_BLOCK_SIZE));
    SPRINTF(pucCurrBuf, ("\nFREE MGMT BUFFER BLOCK BITMAP :%08lx",
        (UINT_32)prMgtBufInfo->rFreeMgtBufBlocksBitmap));
    SPRINTF(pucCurrBuf, ("\nMAXIMUM ALLOCATED DEPTH(bytes):%8ld",
        (UINT_32)prMgtBufInfo->ucMaxAllocatedDepth * MGT_BUF_BLOCK_SIZE));

    SPRINTF(pucCurrBuf, ("\n\n"));

    *pu4Count = (UINT_32)((UINT_32)pucCurrBuf - (UINT_32)pucBuffer);

    return;

} /* end of mgtBufQueryStatus() */
#endif /* CFG_TX_DBG_MGT_BUF */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
mgtPacketCheckHeadroom (
    IN P_MGT_PACKET_T prMgtPacket,
    IN UINT_32 u4Length
    )
{
    ASSERT(prMgtPacket);

    return (BOOLEAN)((u4Length <= (UINT_32)(prMgtPacket->pucData - prMgtPacket->pucHead)) ? TRUE : FALSE);

} /* end of mgtPacketCheckHeadroom() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
PUINT_8
mgtPacketPut (
    IN P_MGT_PACKET_T prMgtPacket,
    IN UINT_32 u4Length
    )
{
    PUINT_8 pucBuffer;


    ASSERT(prMgtPacket);
    pucBuffer = prMgtPacket->pucTail;

    if (((UINT_32)pucBuffer + u4Length) > (UINT_32)(prMgtPacket->pucEnd)) {
        ASSERT_REPORT(0, ("Frame size %ld exceed the limit of allocated frame buffer\n", u4Length));
        pucBuffer = (PUINT_8)NULL;
    }
    else {
        prMgtPacket->pucTail += u4Length;
    }

    return pucBuffer;

} /* end of mgtPacketPut() */


#if 0 /* RESERVE */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
PUINT_8
mgtPacketPush (
    IN P_MGT_PACKET_T prMgtPacket,
    IN UINT_32 u4Length
    )
{
    PUINT_8 pucBuffer;


    ASSERT(prMgtPacket);

    if ((prMgtPacket->pucData - u4Length) < prMgtPacket->pucHead) {
        ASSERT_REPORT(0, ("No succifucent head room for %ld bytes.\n", u4Length));
        pucBuffer = (PUINT_8)NULL;
    }
    else {
        prMgtPacket->pucData -= u4Length;
        pucBuffer = prMgtPacket->pucData;
    }

    return pucBuffer;

} /* end of mgtPacketPush() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
mgtPacketReserveHeadroom (
    IN P_MGT_PACKET_T prMgtPacket,
    IN UINT_32 u4Length
    )
{
    ASSERT(prMgtPacket);

    if (u4Length > (UINT_32)(prMgtPacket->pucEnd - prMgtPacket->pucTail)) {
        ASSERT_REPORT(0, ("No succifucent space for %ld bytes of head room.\n", u4Length));
        return FALSE;
    }
    else {
        prMgtPacket->pucData += u4Length;
        prMgtPacket->pucTail += u4Length;
    }

    return TRUE;

} /* end of mgtPacketReserveHeadroom() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
UINT_32
mgtPacketHeadroomLength (
    IN P_MGT_PACKET_T prMgtPacket
    )
{
    ASSERT(prMgtPacket);

    return (UINT_32)(prMgtPacket->pucData - prMgtPacket->pucHead);

} /* end of mgtPacketHeadroomLength() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
UINT_32
mgtPacketLength (
    IN P_MGT_PACKET_T prMgtPacket
    )
{
    ASSERT(prMgtPacket);

    return (UINT_32)(prMgtPacket->pucTail - prMgtPacket->pucData);

} /* end of mgtPacketLength() */
#endif


