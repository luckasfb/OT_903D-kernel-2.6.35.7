





#ifndef _CFG_BT_FILE_H
#define _CFG_BT_FILE_H


// the record structure define of bt nvram file
#ifdef MTK_MT6620
typedef struct
{
    unsigned char addr[6];
    unsigned char Voice[2];
    unsigned char Codec[4];
    unsigned char Radio[6];
    unsigned char Sleep[7];
    unsigned char BtFTR[2];
    unsigned char TxPWOffset[3];
} ap_nvram_btradio_mt6610_struct;
#else
typedef struct
{
    unsigned char addr[6];
    unsigned char CapId[1];
    unsigned char Codec[1];
} ap_nvram_btradio_mt6610_struct;
#endif

//the record size and number of bt nvram file
#define CFG_FILE_BT_ADDR_REC_SIZE    sizeof(ap_nvram_btradio_mt6610_struct)
#define CFG_FILE_BT_ADDR_REC_TOTAL   1

#endif


