





#ifndef _MGT_BUF_H
#define _MGT_BUF_H



#define POWER_OF_2(n)                           BIT(n)

/* Size of a basic management buffer block in power of 2 */
#define MGT_BUF_BLOCK_SIZE_IN_POWER_OF_2        7 /* 7 to the power of 2 = 128 */

/* Size of a basic management buffer block */
#define MGT_BUF_BLOCK_SIZE                      POWER_OF_2(MGT_BUF_BLOCK_SIZE_IN_POWER_OF_2)

/* Total size of (n) basic management buffer blocks */
#define MGT_BUF_BLOCKS_SIZE(n)                  ((UINT_32)(n) << MGT_BUF_BLOCK_SIZE_IN_POWER_OF_2)

/* Number of management buffer block */
#define MAX_NUM_OF_MGT_BUF_BLOCKS               32 /* Range: 1~32, currently use 32*128=4K Bytes */

/* Number of management frame control unit */
#define MAX_NUM_OF_MGT_PACKETS                  CFG_MAX_NUM_MSDU_INFO_FOR_TCM /* packets */

/* Size of overall management frame buffer */
#define MGT_BUFFER_SIZE                         (MAX_NUM_OF_MGT_BUF_BLOCKS * MGT_BUF_BLOCK_SIZE)

/* Size of all management frame descriptors */
#define MGT_PACKET_DESCRIPTORS_SIZE             (MAX_NUM_OF_MGT_PACKETS * ALIGN_4(sizeof(MGT_PACKET_T)))

/* Reserve some size in every MGT_PACKET's buffer */
#define MGT_PACKET_RESERVED_SIZE                ALIGN_4(TFCB_SIZE)


#if ((MAX_NUM_OF_MGT_BUF_BLOCKS > 32) || (MAX_NUM_OF_MGT_BUF_BLOCKS <= 0))
    #error > #define MAX_NUM_OF_MGT_BUF_BLOCKS : Out of boundary !
#elif MAX_NUM_OF_MGT_BUF_BLOCKS > 16
    typedef UINT_32 BUF_BITMAP;
#elif MAX_NUM_OF_MGT_BUF_BLOCKS > 8
    typedef UINT_16 BUF_BITMAP;
#else
    typedef UINT_8 BUF_BITMAP;
#endif /* MAX_NUM_OF_MGT_BUF_BLOCKS */

/* Control variable of TX management memory pool */
typedef struct _MGT_BUF_INFO_T {
    UINT_32     u4MgtBufCachedSize;
    PUINT_8     pucMgtBufCached;
    PUINT_8     pucMgtBufPoolCached;

    QUE_T       rFreeMgtPacketList;
#if CFG_TX_DBG_MGT_BUF
    UINT_8      ucMaxAllocatedDepth;
#endif /* CFG_TX_DBG_MGT_BUF */
    BUF_BITMAP  rFreeMgtBufBlocksBitmap;
    BUF_BITMAP  rMgtBufBlocksToBitsTable[MAX_NUM_OF_MGT_BUF_BLOCKS + 1];
} MGT_BUF_INFO_T, *P_MGT_BUF_INFO_T;

typedef struct _MGT_PACKET_T {
    QUE_ENTRY_T rQueEntry;
    BUF_BITMAP  rAllocatedMgtBufBlocksBitmap;
    PUINT_8     pucHead;
    PUINT_8     pucData;
    PUINT_8     pucTail;
    PUINT_8     pucEnd;
} MGT_PACKET_T, *P_MGT_PACKET_T;




#define MGT_PACKET_GET_BUFFER(prMgtPacket)  \
    (((P_MGT_PACKET_T)(prMgtPacket))->pucData)

#define MGT_PACKET_GET_LENGTH(prMgtPacket)  \
    ((UINT_32)((UINT_32)((P_MGT_PACKET_T)(prMgtPacket))->pucTail - \
                (UINT_32)((P_MGT_PACKET_T)(prMgtPacket))->pucData))

VOID
mgtBufInitialize (
    IN P_ADAPTER_T prAdapter
    );

P_MGT_PACKET_T
mgtBufAllocateMgtPacket (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32 u4Length
    );

VOID
mgtBufFreeMgtPacket (
    IN P_ADAPTER_T prAdapter,
    IN P_MGT_PACKET_T prMgtPacket
    );

#if CFG_TX_DBG_MGT_BUF
VOID
mgtBufQueryStatus (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8 pucBuffer,
    OUT PUINT_32 pu4Count
    );
#endif /* CFG_TX_DBG_MGT_BUF */

BOOLEAN
mgtPacketCheckHeadroom (
    IN P_MGT_PACKET_T prMgtPacket,
    IN UINT_32 u4Length
    );

PUINT_8
mgtPacketPut (
    IN P_MGT_PACKET_T prMgtPacket,
    IN UINT_32 u4Length
    );

#if 0 /* RESERVE */
PUINT_8
mgtPacketPush (
    IN P_MGT_PACKET_T prMgtPacket,
    IN UINT_32 u4Length
    );

BOOLEAN
mgtPacketReserveHeadroom (
    IN P_MGT_PACKET_T prMgtPacket,
    IN UINT_32 u4Length
    );

UINT_32
mgtPacketHeadroomLength (
    IN P_MGT_PACKET_T prMgtPacket
    );

UINT_32
mgtPacketLength (
    IN P_MGT_PACKET_T prMgtPacket
    );
#endif

#endif /* _MGT_BUF_H */


