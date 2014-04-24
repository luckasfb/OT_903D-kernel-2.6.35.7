





#ifndef _UTIL_H
#define _UTIL_H





#define PROTO_TCP 0x6
#define PROTO_UDP 0x11

#define MAC_ADDR_LEN  6

#define IPv4_ADDR_LEN 4

#define IPv6_ADDR_LEN 16

/* Ethernet Frame Structure */
typedef struct _eth_hdr_t_ {
    UINT_8      aucDestAddr[MAC_ADDR_LEN];
    UINT_8      aucSrcAddr[MAC_ADDR_LEN];
    UINT_16     u2TypeLen;
    UINT_8      aucData[1];
} __KAL_ATTRIB_PACKED__ eth_hdr_t, *peth_hdr_t;



typedef struct _ipv4_hdr_t_ {
    unsigned char hdrlen:4;
    unsigned char version:4;
    unsigned char tos;
    unsigned short len;
    unsigned short id;
    unsigned short flagfragoffset;
    unsigned char ttl;
    unsigned char proto;
    unsigned short hdrchecksum;
    unsigned int src;
    unsigned int dest;
} ipv4_hdr_t, *pipv4_hdr_t;

typedef struct _ipv6_hdr_t_ {
    unsigned char priority:4;
    unsigned char version:4;
    unsigned char flow_lbl[3];
    unsigned short payload_len;
    unsigned char next_hdr;
    unsigned char hop_limit;
    unsigned char saddr[16];
    unsigned char daddr[16];
} ipv6_hdr_t, *pipv6_hdr_t;

typedef struct _tcp_hdr_t_ {
    unsigned short source;
    unsigned short dest;
    unsigned int seq;
    unsigned int ack_seq;
    unsigned short fin:1,
                   syn:1,
                   rst:1,
                   psh:1,
                   ack:1,
                   urg:1,
                   ece:1,
                   cwr:1,
                   resl:4,
                   doff:4;
    unsigned short window;
    unsigned short check;
    unsigned short urg_ptr;
} tcp_hdr_t, *ptcp_hdr_t;

typedef struct _udp_hdr_t_ {
    unsigned short source;
    unsigned short dest;
    unsigned short len;
    unsigned short check;
} udp_hdr_t, *pudp_hdr_t;

typedef struct _ipv6_pseudo_hdr_t_ {
    unsigned char saddr[16];
    unsigned char daddr[16];
    unsigned int payload_len;
    unsigned char reserved[3];
    unsigned char next_hdr;
} ipv6_pseudo_hdr_t, *pipv6_pseudo_hdr_t;

typedef struct _ipv4_pseudo_hdr_t_ {
    unsigned int saddr;
    unsigned int daddr;
    unsigned int payload_len;
    unsigned char reserved[3];
    unsigned char next_hdr;
} ipv4_pseudo_hdr_t, *pipv4_pseudo_hdr_t;

typedef struct _ipv6_ext_hdr_t_ {
    unsigned char next_hdr;
    unsigned char len;
} ipv6_ext_hdr_t, *pipv6_ext_hdr_t;







UINT_32
utilComputeCSUM (
    IN PUINT_8 pucInput,
    IN UINT_32 u4Size,
    IN UINT_32 u4CSUM
    );

BOOLEAN
utilValidateTCPChecksum (
    IN PUINT_8 pucSAddr,
    IN PUINT_8 pucDAddr,
    IN UINT_8 ucAddrlen,
    IN PUINT_8 pucDatagram,
    IN UINT_32 ucDatalen
    );

BOOLEAN
utilValidateUDPChecksum (
    IN PUINT_8 pucSAddr,
    IN PUINT_8 pucDAddr,
    IN UINT_8 ucAddrlen,
    IN PUINT_8 pucDatagram,
    IN UINT_32 ucDatalen
    );


BOOLEAN
utilValidateIPv4Checksum (
    IN PUINT_8 pucDatagram,
    IN PUINT_8 pucProto,
    OUT P_ENUM_CSUM_RESULT_T peL4CSUMRes
    );

BOOLEAN
utilValidateIPv6Checksum (
    IN PUINT_8 pucDatagram,
    IN PUINT_8 pucProto,
    OUT P_ENUM_CSUM_RESULT_T peL4CSUMRes
    );

VOID
utilGenIPv4CSUM (
    IN PUINT_8 pucData
    );

VOID
utilGenIPv6CSUM (
    IN PUINT_8 pucData
    );

VOID
utilGenTCPCSUM (
    IN PUINT_8 pucData,
    IN UINT_32 u4Size,
    IN UINT_32 u4IPCSUM
    );

VOID
utilGenUDPCSUM (
    IN PUINT_8 pucData,
    IN UINT_32 u4Size,
    IN UINT_32 u4IPCSUM
    );

VOID
utilTxComputeCSUM (
    IN PUINT_8 pucPacket,
    IN UINT_32 u4PktLen
    );

VOID
utilRxComputeCSUM (
    IN PUINT_8 pucEtherPkt,
    IN UINT_16 u2PktLen,
    OUT ENUM_CSUM_RESULT_T aeCSUM[]
    );


#endif /* _UTIL_H */

