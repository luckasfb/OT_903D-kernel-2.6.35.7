


#ifndef _DOMAIN_H
#define _DOMAIN_H





#if CFG_SUPPORT_802_11D
BOOLEAN
domainParseCountryInfoElem (
    IN P_IE_COUNTRY_T pCountryIE,
    OUT P_DOMAIN_INFO_ENTRY pDomainInfo
    );

VOID
domainConstructCountryInfoElem(
    IN P_ADAPTER_T prAdapter,
    OUT PUINT_8 pucInfoElem,
    OUT PUINT_8 pucInfoElemLen
    );

BOOLEAN
domainAcquireRegInfo(
    IN P_ADAPTER_T prAdapter,
    OUT P_DOMAIN_INFO_ENTRY pNewDomainInfo
    );

BOOLEAN
domainGetDomainInfoByScanResult (
    IN P_ADAPTER_T prAdapter,
    OUT P_DOMAIN_INFO_ENTRY pDomainInfo
    );

#endif

#endif /* _DOMAIN_H */


