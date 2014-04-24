






#include "precomp.h"








/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
UINT_32
utilComputeCSUM (
    IN PUINT_8 pucInput,
    IN UINT_32 u4Size,
    IN UINT_32 u4CSUM
    )
{

    PUINT_16 pu2Input;
    UINT_32 u4Sum = 0;
    UINT_32 u4WordLen = 0;
//    UINT_8  ucTemp = 0;

    UINT_32 i = 0;
    UINT_16 u2Trail = 0; /* Fix Klocwork ABR warning */

    ASSERT(pucInput);

    u4Sum = u4CSUM;
    pu2Input = (PUINT_16)pucInput;

#if 1
    /* Fix Klocwork warning: array bound violation
    * "ucTemp = pucInput[u4Size];" access address beyond the input buffer!
    */
    u4WordLen = u4Size / 2;

    if (u4Size % 2 != 0) {
        u2Trail = (UINT_16)pucInput[u4Size-1];
        u2Trail <<= 8;
    }

    for (i = 0; i < u4WordLen; i++) {
        u4Sum += pu2Input[i];
    }
    /* add trail word (0 if length is even) */
    u4Sum += u2Trail;

#else
    if (u4Size % 2 != 0) {
        u4WordLen = (u4Size / 2) + 1;
    }
    else {
        u4WordLen = u4Size / 2;
    }

    for (i = 0; i < u4WordLen; i++) {
        if (i == u4WordLen-1) {
            ucTemp = pucInput[u4Size];
            pucInput[u4Size] = 0;
        }
        u4Sum += pu2Input[i];
    }
    pucInput[u4Size] = ucTemp;
#endif

    return u4Sum;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
utilValidateTCPChecksum (
    IN PUINT_8 pucSAddr,
    IN PUINT_8 pucDAddr,
    IN UINT_8 ucAddrlen,
    IN PUINT_8 pucDatagram,
    IN UINT_32 u4Datalen
    )
{
    UINT_32 u4CSUM32 = 0;

    DEBUGFUNC("utilValidateTCPChecksum");

    ASSERT(pucSAddr);
    ASSERT(pucDAddr);

    u4CSUM32 = utilComputeCSUM(pucDatagram, u4Datalen, 0);
    u4CSUM32 = utilComputeCSUM(pucSAddr, ucAddrlen, u4CSUM32);
    u4CSUM32 = utilComputeCSUM(pucDAddr, ucAddrlen, u4CSUM32);
    u4CSUM32 += (HTONL(u4Datalen)&0xffff) + ((HTONL(u4Datalen)>>16) & 0xffff);
/*lint -save -e572 Excessive shift value (precision 0 shifted right by 16) Kevin: shift a constant */
    u4CSUM32 += (HTONL(0x6UL)&0xffff) + ((HTONL(0x6UL)>>16) & 0xffff);
/*lint -restore */

    while (u4CSUM32 > 0xffff) {
        u4CSUM32 = (u4CSUM32 >> 16) + (u4CSUM32 & 0xffff);
    }
    if (u4CSUM32 != 0xffff) {
        DBGLOG(RX, LOUD, ("TCP Checksum Error!\n"));
        return FALSE;
    }
    else {
        DBGLOG(RX, LOUD, ("TCP Checksum Correct!\n"));
        return TRUE;
    }


}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
utilValidateUDPChecksum (
    IN PUINT_8 pucSAddr,
    IN PUINT_8 pucDAddr,
    IN UINT_8 ucAddrlen,
    IN PUINT_8 pucDatagram,
    IN UINT_32 ucDatalen
    )
{
    UINT_32 u4CSUM32 = 0;

    DEBUGFUNC("utilValidateUDPChecksum");

    ASSERT(pucSAddr);
    ASSERT(pucDAddr);

    u4CSUM32 = utilComputeCSUM(pucDatagram, ucDatalen, 0);
    u4CSUM32 = utilComputeCSUM(pucSAddr, ucAddrlen, u4CSUM32);
    u4CSUM32 = utilComputeCSUM(pucDAddr, ucAddrlen, u4CSUM32);
    u4CSUM32 += (HTONL(ucDatalen)&0xffff) + ((HTONL(ucDatalen)>>16) & 0xffff);
/*lint -save -e572 Excessive shift value (precision 0 shifted right by 16) Kevin: shift a constant */
    u4CSUM32 += (HTONL(0x11UL)&0xffff) + ((HTONL(0x11UL)>>16) & 0xffff);
/*lint -restore */

    while (u4CSUM32 > 0xffff) {
        u4CSUM32 = (u4CSUM32 >> 16) + (u4CSUM32 & 0xffff);
    }
    if (u4CSUM32 != 0xffff) {

        DBGLOG(RX, LOUD, ("UDP Checksum Error!\n"));
        return FALSE;
    }
    else {
        DBGLOG(RX, LOUD, ("UDP Checksum Correct!\n"));
        return TRUE;
    }

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
utilValidateIPv6Checksum (
    IN PUINT_8 pucDatagram,
    IN PUINT_8 pucProto,
    OUT P_ENUM_CSUM_RESULT_T peL4CSUMRes
    )
{
    UINT_32 hdr_len = sizeof(ipv6_hdr_t);

    pipv6_hdr_t prHdr;
    pipv6_ext_hdr_t prExtHdr;
    unsigned char next_hdr;
    unsigned short payload_len;

    DEBUGFUNC("utilValidateIPv6Checksum");

    ASSERT(pucDatagram);
    ASSERT(pucProto);
    ASSERT(peL4CSUMRes);

    prHdr = (pipv6_hdr_t)pucDatagram;
    prExtHdr = (pipv6_ext_hdr_t)(pucDatagram+hdr_len);
    next_hdr = prHdr->next_hdr;
    payload_len = NTOHS(prHdr->payload_len);

    *peL4CSUMRes = CSUM_RES_NONE;

    //Parse extension header
    while (next_hdr != 0x6 && next_hdr != 0x11) {
        next_hdr = prExtHdr->next_hdr;
        hdr_len = prExtHdr->len+1;
        prExtHdr = (pipv6_ext_hdr_t)((PUINT_8)prExtHdr+(prExtHdr->len+1));
        payload_len -= (prExtHdr->len+1);
        /*Todo: Parse routing header */
    }

    *pucProto = prHdr->next_hdr;
    if (prHdr->next_hdr == 0x6) {
        *peL4CSUMRes = (utilValidateTCPChecksum(
            (PUINT_8)prHdr->saddr, (PUINT_8)prHdr->daddr, 16, pucDatagram+hdr_len, payload_len
        ))?CSUM_RES_SUCCESS:CSUM_RES_FAILED;
    }
    else if (prHdr->next_hdr == 0x11) {
        *peL4CSUMRes = (utilValidateUDPChecksum(
            (PUINT_8)prHdr->saddr, (PUINT_8)prHdr->daddr, 16, pucDatagram+hdr_len, payload_len
        ))?CSUM_RES_SUCCESS:CSUM_RES_FAILED;

    }
    else {
        DBGLOG(RX, LOUD, ("Unknow transport layer protocol (%#x)\n", prHdr->next_hdr));
    }
    return TRUE;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
utilValidateIPv4Checksum (
    IN PUINT_8 pucDatagram,
    IN PUINT_8 pucProto,
    OUT P_ENUM_CSUM_RESULT_T peL4CSUMRes
    )
{
    UINT_32 hdr_len;
    pipv4_hdr_t prHdr;
    UINT_32 csum32 = 0;

    DEBUGFUNC("utilValidateIPv4Checksum");

    ASSERT(pucDatagram);
    ASSERT(pucProto);
    ASSERT(peL4CSUMRes);

    hdr_len = (pucDatagram[0] & 0xf)*4;
    prHdr = (pipv4_hdr_t)pucDatagram;

    *peL4CSUMRes = CSUM_RES_NONE;

    csum32 = utilComputeCSUM(pucDatagram, hdr_len, 0);
    while(csum32 > 0xffff) {
        csum32 = (csum32 >> 16) + (csum32 & 0xffff);
    }
    if(csum32 != 0xffff) {
        DBGLOG(RX, LOUD, ("IPv4 Header Checksum Error!\n"));
        return FALSE;
    }
    else{
        //printk("IPv4 Header Checksum Correct!\n");
    }

    DBGLOG(RX, LOUD, ("IPv4(%ld), Tx_Proto(%d): ", NTOHS(prHdr->len)-hdr_len, prHdr->proto));

    if (prHdr->flagfragoffset != 0x40) {
        DBGLOG(RX, LOUD, (", ip fragment(%#x)\n", prHdr->flagfragoffset));
        return TRUE;
    }

    *pucProto = prHdr->proto;
    if(prHdr->proto == 0x6) {
        *peL4CSUMRes = (utilValidateTCPChecksum(
            (PUINT_8)&prHdr->src, (PUINT_8)&prHdr->dest, 4, pucDatagram+hdr_len, NTOHS(prHdr->len)-hdr_len
        ))?CSUM_RES_SUCCESS:CSUM_RES_FAILED;
    }
    else if (prHdr->proto == 0x11) {
        *peL4CSUMRes= (utilValidateUDPChecksum(
            (PUINT_8)&prHdr->src, (PUINT_8)&prHdr->dest, 4, pucDatagram+hdr_len, NTOHS(prHdr->len)-hdr_len
        ))?CSUM_RES_SUCCESS:CSUM_RES_FAILED;
    }
    else {
        DBGLOG(RX, LOUD, ("Unknow transport layer protocol (%#x)\n", prHdr->proto));
    }
    return TRUE;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
utilGenIPv6CSUM (
    IN PUINT_8 pucData
    )
{
    pipv6_hdr_t prHdr;
    UINT_32 u4IPPseudoHdrCSUM = 0;
    //UINT_32 u4L4Datalen = NTOHS(prHdr->payload_len) - sizeof(ipv6_hdr_t);
    UINT_32 hdr_len = sizeof(ipv6_hdr_t);
    pipv6_ext_hdr_t prExtHdr;
    unsigned char next_hdr;
    unsigned long payload_len;

    ASSERT(pucData);

    prHdr = (pipv6_hdr_t)pucData;
    prExtHdr = (pipv6_ext_hdr_t)(pucData+hdr_len);
    next_hdr = prHdr->next_hdr;
    payload_len = NTOHS(prHdr->payload_len);

    while (next_hdr != 0x6 && next_hdr != 0x11) {
        next_hdr = prExtHdr->next_hdr;
        hdr_len = prExtHdr->len+1;
        prExtHdr = (pipv6_ext_hdr_t)((PUINT_8)prExtHdr+(prExtHdr->len+1));
        payload_len -= (prExtHdr->len+1);
        /*Todo: Parse routing header */
    }

    u4IPPseudoHdrCSUM = utilComputeCSUM((PUINT_8)prHdr->daddr, IPv6_ADDR_LEN, u4IPPseudoHdrCSUM);
    u4IPPseudoHdrCSUM = utilComputeCSUM((PUINT_8)prHdr->saddr, IPv6_ADDR_LEN, u4IPPseudoHdrCSUM);
    u4IPPseudoHdrCSUM += (HTONL(payload_len)&0xffff) + ((HTONL(payload_len)>>16) & 0xffff);

    if (next_hdr == PROTO_TCP) {
/*lint -save -e572 Excessive shift value (precision 0 shifted right by 16) Kevin: shift a constant */
        u4IPPseudoHdrCSUM += (HTONL(0x6)&0xffff) + ((HTONL(0x6)>>16) & 0xffff);
/*lint -restore */
        utilGenTCPCSUM(pucData+hdr_len, payload_len, u4IPPseudoHdrCSUM);
    }
    else if (next_hdr == PROTO_UDP) {
/*lint -save -e572 Excessive shift value (precision 0 shifted right by 16) Kevin: shift a constant */
        u4IPPseudoHdrCSUM += (HTONL(0x11)&0xffff) + ((HTONL(0x11)>>16) & 0xffff);
/*lint -restore */
        utilGenUDPCSUM(pucData+hdr_len, payload_len, u4IPPseudoHdrCSUM);
    }
    else {

    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
utilGenIPv4CSUM (
    IN PUINT_8 pucData
    )
{
    pipv4_hdr_t prHdr;

    UINT_32 hdr_len;
    UINT_32 csum32 = 0;
    PUINT_8 pucL4Pkt;
    UINT_32 u4IPPseudoHdrCSUM = 0;
    UINT_32 u4L4Datalen;

    ASSERT(pucData);

    hdr_len = (pucData[0] & 0xf)*4;
    prHdr = (pipv4_hdr_t)pucData;
    pucL4Pkt = pucData + sizeof(ipv4_hdr_t);
    u4L4Datalen = NTOHS(prHdr->len) - sizeof(ipv4_hdr_t);

    prHdr->hdrchecksum = 0;
    csum32 = utilComputeCSUM(pucData, hdr_len, 0);
    while(csum32 > 0xffff){
        csum32 = (csum32 >> 16) + (csum32 & 0xffff);
    }
    csum32 = (~csum32 & 0xffff);
    prHdr->hdrchecksum = (UINT_16)(csum32 & 0xffff);

    //u4IPPseudoHdrCSUM = utilComputeCSUM(pucData, NTOHS(prHdr->len) - sizeof(ipv4_hdr_t), 0);
    u4IPPseudoHdrCSUM = utilComputeCSUM((PUINT_8)&prHdr->dest, IPv4_ADDR_LEN, u4IPPseudoHdrCSUM);
    u4IPPseudoHdrCSUM = utilComputeCSUM((PUINT_8)&prHdr->src, IPv4_ADDR_LEN, u4IPPseudoHdrCSUM);
    u4IPPseudoHdrCSUM += (UINT_16)(HTONL(u4L4Datalen)&0xffff) + (UINT_16)((HTONL(u4L4Datalen)>>16) & 0xffff);


    if (prHdr->flagfragoffset == 0x40) {
        if (prHdr->proto == PROTO_TCP) {
/*lint -save -e572 Excessive shift value (precision 0 shifted right by 16) Kevin: shift a constant */
            u4IPPseudoHdrCSUM += (HTONL(0x6)&0xffff) + ((HTONL(0x6)>>16) & 0xffff);
/*lint -restore */
            utilGenTCPCSUM(pucL4Pkt, u4L4Datalen, u4IPPseudoHdrCSUM);
        }
        else if (prHdr->proto == PROTO_UDP) {
/*lint -save -e572 Excessive shift value (precision 0 shifted right by 16) Kevin: shift a constant */
            u4IPPseudoHdrCSUM += (HTONL(0x11)&0xffff) + ((HTONL(0x11)>>16) & 0xffff);
/*lint -restore */
            utilGenUDPCSUM(pucL4Pkt, u4L4Datalen, u4IPPseudoHdrCSUM);
        }
        else {

        }
    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
utilGenTCPCSUM (
    IN PUINT_8 pucData,
    IN UINT_32 u4Size,
    IN UINT_32 u4IPCSUM
    )
{
    ptcp_hdr_t tcp;
    UINT_32 CSUM = 0;

    ASSERT(pucData);
    tcp = (ptcp_hdr_t)pucData;

    tcp->check = 0;
    if(u4Size < sizeof(tcp_hdr_t)){
        ASSERT(0);
        return;
    }

    CSUM = utilComputeCSUM(pucData, u4Size, u4IPCSUM);

    while(CSUM > 0xffff){
        CSUM = (CSUM >> 16) + (CSUM & 0xffff);
    }
    CSUM = (~CSUM & 0xffff);

    tcp->check = (UINT_16)CSUM;

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
utilGenUDPCSUM (
    IN PUINT_8 pucData,
    IN UINT_32 u4Size,
    IN UINT_32 u4IPCSUM
    )
{
    pudp_hdr_t udp;
    UINT_32 CSUM = 0;

    ASSERT(pucData);
    udp = (pudp_hdr_t)pucData;

    udp->check = 0;
    if(u4Size < sizeof(udp_hdr_t)){
        ASSERT(0);
        return;
    }

    CSUM = utilComputeCSUM(pucData, u4Size, u4IPCSUM);

    while(CSUM > 0xffff){
        CSUM = (CSUM >> 16) + (CSUM & 0xffff);
    }

    CSUM = (~CSUM & 0xffff);

    if(CSUM == 0){
        CSUM = 0xffff;
    }

    udp->check = (UINT_16)CSUM;

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
utilRxComputeCSUM (
    IN PUINT_8 pucEtherPkt,
    IN UINT_16 u2PktLen,
    OUT ENUM_CSUM_RESULT_T aeCSUM[]
    )
{
    UINT_16 u2Proto = 0;
    peth_hdr_t prEtherHdr;
    PUINT_8 pucDatagram = 0;
    UINT_8 ucProto = 0;
    //BOOLEAN fgL4ProtoRes = FALSE;
    ENUM_CSUM_RESULT_T eL4CSUMRes = CSUM_RES_FAILED;

    DEBUGFUNC("utilRxComputeCSUM");

    ASSERT(pucEtherPkt);
    ASSERT(aeCSUM);

    prEtherHdr = (peth_hdr_t)pucEtherPkt;

    u2Proto = prEtherHdr->u2TypeLen;
    pucDatagram = prEtherHdr->aucData;


    if(u2Proto == TL_IPV4) {
        aeCSUM[CSUM_TYPE_IPV6] = CSUM_RES_NONE;
        if (utilValidateIPv4Checksum(pucDatagram, &ucProto, &eL4CSUMRes)){
            aeCSUM[CSUM_TYPE_IPV4] = CSUM_RES_SUCCESS;
        }
        else {
            aeCSUM[CSUM_TYPE_IPV4] = CSUM_RES_FAILED;
        }
    }
    else if (u2Proto == TL_IPV6) {
        aeCSUM[CSUM_TYPE_IPV4] = CSUM_RES_NONE;
        if (utilValidateIPv6Checksum(pucDatagram, &ucProto, &eL4CSUMRes)){
            aeCSUM[CSUM_TYPE_IPV6] = CSUM_RES_SUCCESS;
        }
        else {
            aeCSUM[CSUM_TYPE_IPV6] = CSUM_RES_FAILED;
        }
    }
    else {
        DBGLOG(RX, LOUD, ("UNKNOW L3 Header"));
        aeCSUM[CSUM_TYPE_IPV6] = CSUM_RES_NONE;
        aeCSUM[CSUM_TYPE_IPV4] = CSUM_RES_NONE;
    }
    if(ucProto == 0x6) {
        aeCSUM[CSUM_TYPE_TCP] = eL4CSUMRes;//?CSUM_RES_SUCCESS:CSUM_RES_FAILED;
    }
    else if (ucProto == 0x11) {
        aeCSUM[CSUM_TYPE_UDP] = eL4CSUMRes;//?CSUM_RES_SUCCESS:CSUM_RES_FAILED;
    }
    else {
        aeCSUM[CSUM_TYPE_UDP] = CSUM_RES_NONE;
        aeCSUM[CSUM_TYPE_TCP] = CSUM_RES_NONE;
    }


}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
utilTxComputeCSUM (
    IN PUINT_8 pucPacket,
    IN UINT_32 u4PktLen
    )
{

    peth_hdr_t prEtherPkt;

    ASSERT(pucPacket);
    prEtherPkt = (peth_hdr_t)pucPacket;

    if(prEtherPkt && u4PktLen > 14) {
        if (prEtherPkt->u2TypeLen == TL_IPV4) {
            utilGenIPv4CSUM(prEtherPkt->aucData);
        }
        else if (prEtherPkt->u2TypeLen == TL_IPV6) {
            utilGenIPv6CSUM(prEtherPkt->aucData);
        }
        else {

        }
    }

}



