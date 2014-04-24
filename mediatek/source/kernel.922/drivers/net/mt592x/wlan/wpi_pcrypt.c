
#ifdef	MTK_WAPI_SUPPORT
#include "precomp.h"

/* The MAC Header for constricture the Cal MIC */
typedef  struct _WAPI_MAC_HEADER_FOR_MIC_T {
    UINT_16     u2FrameCtrl;
    UINT_8      aucAddr1[MAC_ADDR_LEN];
    UINT_8      aucAddr2[MAC_ADDR_LEN];
    UINT_16     u2SeqCtrl;
    UINT_8      aucAddr3[MAC_ADDR_LEN];
    UINT_8      aucAddr4[MAC_ADDR_LEN];
} __KAL_ATTRIB_PACKED__ WAPI_MAC_HEADER_FOR_MIC_T, *P_WAPI_MAC_HEADER_FOR_MIC_T;

/*ofb encrypt*/
int wpi_encrypt(unsigned char * pofbiv_in,unsigned char * pbw_in,unsigned int plbw_in,unsigned char * pkey,unsigned char * pcw_out)
{
	unsigned int ofbtmp[4];
	unsigned int * pint0, * pint1;
	unsigned char * pchar0, * pchar1,* pchar2;
	unsigned int counter,comp,i;
	//unsigned int prkey_in[32];


	if(plbw_in<1)	return 1;
	//if(plbw_in>65536) return 1;

	//SMS4KeyExt(pkey,  prkey_in, 0);

	counter=plbw_in / 16;
	comp=plbw_in % 16;

//get the iv
	SMS4Crypt(pofbiv_in,(unsigned char *)ofbtmp, pkey /*prkey_in*/);
	pint0=(unsigned int *)pbw_in;
	pint1=(unsigned int *)pcw_out;
	for(i=0;i<counter;i++) {
		pint1[0]=pint0[0]^ofbtmp[0];
		pint1[1]=pint0[1]^ofbtmp[1];
		pint1[2]=pint0[2]^ofbtmp[2];
		pint1[3]=pint0[3]^ofbtmp[3];
		SMS4Crypt((unsigned char *)ofbtmp,(unsigned char *)ofbtmp, pkey /*prkey_in*/);
		pint0+=4;
		pint1+=4;
	}
	pchar0=(unsigned char *)pint0;
	pchar1=(unsigned char *)pint1;
	pchar2=(unsigned char *)ofbtmp;
	for(i=0;i<comp;i++) {
		pchar1[i]=pchar0[i]^pchar2[i];
	}
	
	return 0;	
}

/*ofb decrypt*/
int wpi_decrypt(unsigned char * pofbiv_in,unsigned char * pcw_in,unsigned int plcw_in,unsigned char * prkey_in,unsigned char * pbw_out)
{
	return wpi_encrypt(pofbiv_in,pcw_in,plcw_in,prkey_in,pbw_out);	
}

/*cbc_mac*/
int wpi_pmac(unsigned char * pmaciv_in,unsigned char * pmac_in,unsigned int pmacpc_in,unsigned char * pkey,unsigned char * pmac_out)
{
	unsigned int mactmp[4];
	unsigned int i;
	unsigned int * pint0;
	//unsigned int prmackey_in[32];

	if(pmacpc_in<1) return 1;
	if(pmacpc_in>4096) return 1;

	//SMS4KeyExt(pkey,  prmackey_in, 0);
	
	pint0=(unsigned int *)pmac_in;
	SMS4Crypt(pmaciv_in, (unsigned char *)mactmp, pkey /*prmackey_in*/);	
	for(i=0;i<pmacpc_in;i++) {
		mactmp[0]^=pint0[0];
		mactmp[1]^=pint0[1];
		mactmp[2]^=pint0[2];
		mactmp[3]^=pint0[3];
		pint0 += 4;
		SMS4Crypt((unsigned char *)mactmp, (unsigned char *)mactmp, pkey /*prmackey_in*/);
	}
	pint0 = (unsigned int *)pmac_out;
	pint0[0] = mactmp[0];
	pint0[1] = mactmp[1];
	pint0[2] = mactmp[2];
	pint0[3] = mactmp[3];

	return 0;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void
wpi_mic_compose (
    IN BOOLEAN              fgDir,
    IN UINT_8               ucKeyIdx,
    IN UINT_16              u2PayloadLen,
    P_WLAN_MAC_HEADER_QOS_T prMacHdr,
    IN PUINT_8              pucMicHdr,
    IN BOOLEAN              fgQoS
    )
{

    P_WAPI_MAC_HEADER_FOR_MIC_T prMicHdr;
    UINT_8                      ucPadLen1;
    UINT_8                      ucPadLen2;
    UINT_8                      ucMacLen;
    UINT_8                      ucQosIdxLen;

    if (fgQoS) {
        ucMacLen = WLAN_MAC_HEADER_QOS_LEN;
        ucQosIdxLen = KEYID_LEN + KEYID_RSV_LEN + PDU_LEN + 2 ;
    }
    else {
	 ucMacLen = WLAN_MAC_HEADER_LEN;
        ucQosIdxLen = KEYID_LEN + KEYID_RSV_LEN + PDU_LEN;
    }

    if (fgDir == 1) {
        ucPadLen2 = 16 - ((u2PayloadLen + LLC_LEN) % 16);
        if (ucPadLen2 == 16)
            ucPadLen2 = 0;
    }
    else {
	
        u2PayloadLen -= (KEYID_LEN + KEYID_RSV_LEN + PN_LEN);

        ucPadLen2 = 16 - ((u2PayloadLen - WPI_MIC_LEN)% 16);
        if (ucPadLen2 == 16)
            ucPadLen2 = 0;
    }

    ucPadLen1 = 16 - ((sizeof(WAPI_MAC_HEADER_FOR_MIC_T) + ucQosIdxLen) % 16);
    if (ucPadLen1 == 16) 
		ucPadLen1 = 0;

    prMicHdr = (P_WAPI_MAC_HEADER_FOR_MIC_T)pucMicHdr;

    kalMemCopy((PUINT_8)&prMicHdr->u2FrameCtrl, (PUINT_8)&prMacHdr->u2FrameCtrl, 2);
    prMicHdr->u2FrameCtrl &= ~BITS(4,6);
    prMicHdr->u2FrameCtrl &= ~BITS(11,13);
    prMicHdr->u2FrameCtrl |= BIT(14);

    kalMemCopy(prMicHdr->aucAddr1, prMacHdr->aucAddr1, MAC_ADDR_LEN);
    kalMemCopy(prMicHdr->aucAddr2, prMacHdr->aucAddr2, MAC_ADDR_LEN);
    kalMemCopy((PUINT_8)&prMicHdr->u2SeqCtrl, (PUINT_8)&prMacHdr->u2SeqCtrl, 2);
    prMicHdr->u2SeqCtrl &= ~BITS(4,15);
    kalMemCopy(prMicHdr->aucAddr3, prMacHdr->aucAddr3, MAC_ADDR_LEN);
    kalMemZero((PUINT_8)prMicHdr->aucAddr4, MAC_ADDR_LEN);
    if (fgQoS) {
        kalMemCopy(&pucMicHdr[sizeof(WAPI_MAC_HEADER_FOR_MIC_T)], (PUINT_8)&prMacHdr->u2QosCtrl, 2);
        pucMicHdr[sizeof(WAPI_MAC_HEADER_FOR_MIC_T) + 2 ] = ucKeyIdx;
        pucMicHdr[sizeof(WAPI_MAC_HEADER_FOR_MIC_T) + 2 + KEYID_LEN] = 0;
        if (fgDir == 1) {
            WLAN_SET_FIELD_16_BE(&pucMicHdr[sizeof(WAPI_MAC_HEADER_FOR_MIC_T) + 2 + KEYID_LEN + KEYID_RSV_LEN],
                u2PayloadLen + LLC_LEN);
        }
        else {
            WLAN_SET_FIELD_16_BE(&pucMicHdr[sizeof(WAPI_MAC_HEADER_FOR_MIC_T) + 2 + KEYID_LEN + KEYID_RSV_LEN],
                u2PayloadLen - WPI_MIC_LEN);
        }
    }
    else {		
        pucMicHdr[sizeof(WAPI_MAC_HEADER_FOR_MIC_T)] = ucKeyIdx;
        pucMicHdr[sizeof(WAPI_MAC_HEADER_FOR_MIC_T) + KEYID_LEN] = 0;
        if (fgDir == 1) {
            WLAN_SET_FIELD_16_BE(&pucMicHdr[sizeof(WAPI_MAC_HEADER_FOR_MIC_T) + KEYID_LEN + KEYID_RSV_LEN],
                u2PayloadLen + LLC_LEN);
        }
        else {
            WLAN_SET_FIELD_16_BE(&pucMicHdr[sizeof(WAPI_MAC_HEADER_FOR_MIC_T) + KEYID_LEN + KEYID_RSV_LEN],
                u2PayloadLen - WPI_MIC_LEN);
        }
    }

    kalMemZero(&pucMicHdr[sizeof(WAPI_MAC_HEADER_FOR_MIC_T) + ucQosIdxLen], ucPadLen1);
    if (fgDir == 1)
        kalMemZero(&pucMicHdr[sizeof(WAPI_MAC_HEADER_FOR_MIC_T) + ucQosIdxLen + ucPadLen1 + u2PayloadLen + LLC_LEN], ucPadLen2);
    else
        kalMemZero(&pucMicHdr[sizeof(WAPI_MAC_HEADER_FOR_MIC_T) + ucQosIdxLen + ucPadLen1 + u2PayloadLen - WPI_MIC_LEN], ucPadLen2);

}

#endif

