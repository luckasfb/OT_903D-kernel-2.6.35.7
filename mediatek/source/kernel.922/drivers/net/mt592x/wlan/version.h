




#ifndef _VERSION_H
#define _VERSION_H



#ifndef NIC_AUTHOR
#define NIC_AUTHOR      "NIC_AUTHOR"
#endif
#ifndef NIC_DESC
#define NIC_DESC        "NIC_DESC"
#endif

#ifndef NIC_NAME
#if defined(MT5921)
#define NIC_NAME        "MT5921"
#define NIC_DEVICE_ID   "MT5921"
#endif
#if defined(MT5922)
#define NIC_NAME        "MT5922"
#define NIC_DEVICE_ID   "MT5922"
#endif
#endif

/* NIC driver information */
#define NIC_VENDOR                      "MediaTek Inc."
#define NIC_VENDOR_OUI                  {0x00, 0x0C, 0xE7}

#define NIC_PRODUCT_NAME                "MediaTek Inc. MT5921 Wireless LAN Adapter"
#define NIC_DRIVER_NAME                 "MediaTek Inc. MT5921 Wireless LAN Adapter Driver"

/* Define our driver version */
#define NIC_DRIVER_MAJOR_VERSION        1
#define NIC_DRIVER_MINOR_VERSION        27
#define NIC_DRIVER_VERSION              1,27,9,2009
#define NIC_DRIVER_VERSION_STRING       "1.27.9.2009"

#define NIC_LEAST_APP_MAJOR_VERSION     0
#define NIC_LEAST_APP_MINOR_VERSION     5









#endif /* _VERSION_H */

