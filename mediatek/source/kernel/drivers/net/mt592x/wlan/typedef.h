





#ifndef _TYPEDEF_H
#define _TYPEDEF_H



#define WLAN_STATUS_SUCCESS                     ((WLAN_STATUS) 0x00000000L)
#define WLAN_STATUS_PENDING                     ((WLAN_STATUS) 0x00000103L)
#define WLAN_STATUS_NOT_ACCEPTED                ((WLAN_STATUS) 0x00010003L)

#define WLAN_STATUS_MEDIA_CONNECT               ((WLAN_STATUS) 0x4001000BL)
#define WLAN_STATUS_MEDIA_DISCONNECT            ((WLAN_STATUS) 0x4001000CL)
#define WLAN_STATUS_MEDIA_SPECIFIC_INDICATION   ((WLAN_STATUS) 0x40010012L)

#define WLAN_STATUS_SCAN_COMPLETE               ((WLAN_STATUS) 0x60010001L)
#define WLAN_STATUS_MSDU_OK                     ((WLAN_STATUS) 0x60010002L)

/* TODO(Kevin): double check if 0x60010001 & 0x60010002 is proprietary */
#define WLAN_STATUS_ROAM_OUT_FIND_BEST          ((WLAN_STATUS) 0x60010101L)
#define WLAN_STATUS_ROAM_DISCOVERY              ((WLAN_STATUS) 0x60010102L)

#define WLAN_STATUS_FAILURE                     ((WLAN_STATUS) 0xC0000001L)
#define WLAN_STATUS_RESOURCES                   ((WLAN_STATUS) 0xC000009AL)
#define WLAN_STATUS_NOT_SUPPORTED               ((WLAN_STATUS) 0xC00000BBL)

#define WLAN_STATUS_MULTICAST_FULL              ((WLAN_STATUS) 0xC0010009L)
#define WLAN_STATUS_INVALID_PACKET              ((WLAN_STATUS) 0xC001000FL)
#define WLAN_STATUS_ADAPTER_NOT_READY           ((WLAN_STATUS) 0xC0010011L)
#define WLAN_STATUS_INVALID_LENGTH              ((WLAN_STATUS) 0xC0010014L)
#define WLAN_STATUS_INVALID_DATA                ((WLAN_STATUS) 0xC0010015L)
#define WLAN_STATUS_BUFFER_TOO_SHORT            ((WLAN_STATUS) 0xC0010016L)

/* NIC status flags */
#define ADAPTER_FLAG_HW_ERR                     0x00400000

/* Type Length */
#define TL_IPV4     0x0008
#define TL_IPV6     0xDD86


/* Type definition for GLUE_INFO structure */
typedef struct _GLUE_INFO_T     GLUE_INFO_T, *P_GLUE_INFO_T;

/* Type definition for WLAN STATUS */
typedef UINT_32                 WLAN_STATUS, *P_WLAN_STATUS;

/* Type definition for ADAPTER structure */
typedef struct _ADAPTER_T       ADAPTER_T, *P_ADAPTER_T;

/* Type definition for MSDU_INFO structure, a SW resource for outgoing packet */
typedef struct _MSDU_INFO_T     MSDU_INFO_T, *P_MSDU_INFO_T;

/* Type definition for Pointer to OS Native Packet */
typedef struct _NATIVE_PACKET   *P_NATIVE_PACKET;

typedef struct _STA_RECORD_T    STA_RECORD_T, *P_STA_RECORD_T;

/* Type definition for BSS_DESC_T structure, to describe parameter sets of a particular BSS */
typedef struct _BSS_DESC_T      BSS_DESC_T, *P_BSS_DESC_T;


typedef struct _PEER_BSS_INFO_T PEER_BSS_INFO_T, *P_PEER_BSS_INFO_T;

typedef struct _BSS_INFO_T      BSS_INFO_T, *P_BSS_INFO_T;

//4 2007/10/05, mikewu, Move this back to nic.h, we don't want it to be seen in glue layer
//typedef const struct _INT_EVENT_MAP_T INT_EVENT_MAP_T, *P_INT_EVENT_MAP_T;


typedef struct _SCAN_REQ_CONFIG_T   SCAN_REQ_CONFIG_T, *P_SCAN_REQ_CONFIG_T;


typedef struct _PACKET_INFO_T {
    P_NATIVE_PACKET prPacket;
    BOOLEAN         fgIs802_11;
    BOOLEAN         fgIs1x;
    UINT_8          ucTID;
    UINT_8          ucMacHeaderLength;
    UINT_16         u2PayloadLength;
    PUINT_8         pucDestAddr;
} PACKET_INFO_T, *P_PACKET_INFO_T;




typedef struct _SW_RFB_T        SW_RFB_T, *P_SW_RFB_T, **PP_SW_RFB_T;

typedef struct _EEPROM_CTRL_T    EEPROM_CTRL_T, *P_EEPROM_CTRL_T;

typedef struct _REG_ENTRY_T     REG_ENTRY_T, *P_REG_ENTRY_T;
typedef struct _TABLE_ENTRY_T       TABLE_ENTRY_T, *P_TABLE_ENTRY_T;
typedef struct _TABLE_RF_ENTRY_T       TABLE_RF_ENTRY_T, *P_TABLE_RF_ENTRY_T;



/* Type definition for function pointer of management frame */
typedef VOID (*PROCESS_RX_MGT_FUNCTION)(P_ADAPTER_T, P_SW_RFB_T);

typedef VOID (*IST_EVENT_FUNCTION)(P_ADAPTER_T);

/* Type definition for function pointer of timer handler */
typedef VOID (*PFN_TIMER_CALLBACK)(IN P_ADAPTER_T);




#define PACKET_INFO_INIT(_prPacketInfo, \
                         _fgIs802_11, \
                         _fgIs802_1x, \
                         _prPacketDescriptor, \
                         _ucTID, \
                         _ucMacHeaderLength, \
                         _u2PayloadLength, \
                         _pucDestAddr \
                         ) \
        { \
        ((P_PACKET_INFO_T)(_prPacketInfo))->prPacket = (_prPacketDescriptor); \
        ((P_PACKET_INFO_T)(_prPacketInfo))->fgIs802_11 = (_fgIs802_11); \
        ((P_PACKET_INFO_T)(_prPacketInfo))->fgIs1x = (_fgIs802_1x); \
        ((P_PACKET_INFO_T)(_prPacketInfo))->ucTID = (_ucTID); \
        ((P_PACKET_INFO_T)(_prPacketInfo))->ucMacHeaderLength = (_ucMacHeaderLength); \
        ((P_PACKET_INFO_T)(_prPacketInfo))->u2PayloadLength = (_u2PayloadLength); \
        ((P_PACKET_INFO_T)(_prPacketInfo))->pucDestAddr = (_pucDestAddr); \
        };

#define PACKET_INFO_PAYLOAD_LEN(prPacketInfo)   (((P_PACKET_INFO_T)prPacketInfo)->u2PayloadLength)


#endif /* _TYPEDEF_H */


