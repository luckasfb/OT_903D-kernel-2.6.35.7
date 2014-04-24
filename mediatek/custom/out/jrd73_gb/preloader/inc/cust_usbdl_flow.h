

#ifndef _CUST_USBDL_FLOW_H
#define _CUST_USBDL_FLOW_H

// always defined
#define HAS_USBDL_KEY

// depends on customer's requirment
#define USBDL_DEFAULT_ON

//******************************************************
//* modification history :
//*
//* 20100728  : (1) add USBDL flow setting
//*
//******************************************************
typedef struct _DM_USBDL_FLOW
{
    BOOL enum_timeout_enable;
    BOOL handshake_timeout_enable;
} DM_USBDL_FLOW;

//******************************************************
//* modification history :
//* 2010/12/13  : (1) support image boundary check
//                    v1.1050.2 flash tool or above
//                    will send partition information to pre-loader for 
//                    download boundary check
//******************************************************
#define GET_IMG_BOUNDARY_FROM_TOOL 

// tell the pre-loader, the current platform is ZPHN
#define ZPHN_PLATFORM

#endif   /*_CUST_USBDL_FLOW_H*/
