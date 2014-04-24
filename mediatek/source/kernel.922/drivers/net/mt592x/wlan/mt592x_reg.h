




#ifndef _MT592X_REG_H
#define _MT592X_REG_H




typedef enum _ENUM_HW_QUE_T {
    AC0 = 0,
    AC1,
    AC2,
    AC3,
    TS0,
    AC4,
    RXQ,
    HW_QUE_NUM
} ENUM_HW_QUE_T;

#define HW_QUE_MASK                         (BIT(AC0) | BIT(AC1) | BIT(AC2) | \
                                             BIT(AC3) | BIT(TS0) | BIT(AC4) | \
                                             BIT(RXQ))

#define HW_QUE_TX_MASK                      (BIT(AC0) | BIT(AC1) | BIT(AC2) | \
                                             BIT(AC3) | BIT(TS0) | BIT(AC4))








//1 MT5921 MCR Definition

//2 Host Interface

//4 CHIP ID Register
#define MCR_CIR                         0x0000

//4 HIF Interrupt Status Register
#define MCR_HISR                        0x0004

//4 HIF Slow Clock Interrupt Status Register
#define MCR_HSCISR                      0x0008

//4 HIF Interrupt Enable Register
#define MCR_HIER                        0x000C

//4 HIF Slow Clock Interrupt Enable Register
#define MCR_HSCIER                      0x0010

//4 HIF Control Register
#define MCR_HCR                         0x0014

//4 SDIO Control Status Register
#define MCR_SDIOCSR                     0x0018

//4 SPI Control Status Register
#define MCR_SPICSR                      0x0018

//4 CF Control Status Register
#define MCR_CFCSR                       0x0018

//4 HPI Control Status Register
#define MCR_HPICSR                      0x0018

//4 eHPI Control Status Register
#define MCR_eHPICSR                     0x0018

//4 RSSI Register
#define MCR_RR                          0x001C

//4 HIF Tx Data Register
#define MCR_HTDR                        0x0020

//4 HIF Tx Status Register
#define MCR_HTSR                        0x0024

//4 HIF Rx Data Register
#define MCR_HRDR                        0x0028

//4 Queue Address Register
#define MCR_QAR                         0x002C

//4 HIF Beacon Data Register
#define MCR_HBDR                        0x0030

//4 HIF WLAN Table Data Register
#define MCR_HWTDR                       0x0034

//4 HIF WLAN Table Control Register
#define MCR_HWTCR                       0x0038

//4 HIF Low Power Control Register
#define MCR_HLPCR                       0x003C

//4 HIF FIFO Control Register
#define MCR_HFCR                        0x0040

//4 HIF Scan Data Register
#define MCR_HSDR                        0x0044
#define MCR_HFDR                        0x0044

//4 HIF Interrupt threshold Register
#define MCR_HITR                        0x0048


//4 ASR Abnormal Status Register
#define MCR_ASR                         0x004C


//2 CONFIG
//4
//4 Debug Register
#define MCR_DBGR                        0x0050

//4 EEPROM Software Control Register
#define MCR_ESCR                        0x0054

//4 EEPROM Address and Data Register
#define MCR_EADR                        0x0058

//4 32K Clock Calibration Register
#define MCR_32KCCR                      0x005C

//4 Clock Control Register
#define MCR_CCR                         0x0060

//4 Output Driving Control Register
#define MCR_ODCR                        0x0064

//4 I/O Pull Up/Down Register
#define MCR_IOUDR                       0x0068

//4 I/O Pin Control Register
#define MCR_IOPCR                       0x006C

//4 SDCR: SRAM Driving Control Register
#define MCR_SDCR                        0x0070

//4 AFE Configuration Data 0-3 Register
#define MCR_ACDR0                       0x0074
#define MCR_ACDR1                       0x0078
#define MCR_ACDR2                       0x007C
#define MCR_ACDR3                       0x0080
#define MCR_ACDR4                       0x0084


//2 MAC

//4 System Control Register
#define MCR_SCR                         0x0090

//4 LED Control Register
#define MCR_LCR                         0x0094

//4 RF Interface Control Register
#define MCR_RFICR                       0x0098

#define MCR_RICR                        0x0098

//4 RF Synthesizer Configuration Register
#define MCR_RFSCR                       0x009C

#define MCR_RSCR                        0x009C

//4 RF Synthesizer Data Register
#define MCR_RFSDR                       0x00A0

#define MCR_RSDR                        0x00A0

//4  Auto Rate Parameter Register 0
#define MCR_ARPR0                       0x00A4

//4  Auto Rate Parameter Register 1
#define MCR_ARPR1                       0x00A8

//4  Auto Rate Parameter Register 2
#define MCR_ARPR2                       0x00AC


#define MCR_MMCR0                       0x00B0
#define MCR_MMCR1                       0x00B4


//4 Quiet Channel Control Register 0-1
#define MCR_QCCR0                       0x00B8
#define MCR_QCCR1                       0x00BC

//4 MIB Status Control Register
#define MCR_MIBSCR                      0x00C0

//4 MIB Status Data  Register
#define MCR_MIBSDR                      0x00C4


//2 ARB

//4 Queue Control Register
#define MCR_QCR                         0x00D0

//4 Arbitration InterFrame Spacing Register
#define MCR_AIFSR                       0x00D4

//4 Maximum Tx Count Limit Register
#define MCR_MTCLR                       0x00D8

//4 AC Contention Window Max Register 0-1
#define MCR_ACCWXR0                     0x00DC
#define MCR_ACCWXR1                     0x00E0

//4 AC Contention Window Min Register
#define MCR_ACCWIR                      0x00E4

//4 AC4 Contention Window Register
#define MCR_AC4CWR                      0x00E8

//4 Beacon Contention Window Register
#define MCR_BCWR                        0x00EC

//4 DCF Random Number Generator Register
#define MCR_DRNGR                       0x00F0

//4 MAC Status Control Register
#define MCR_MSCR                        0x00F4

//4 Auto Rate Control Register
#define MCR_ARCR                        0x00F8

//4 Tx Control Register
#define MCR_TXCR                        0x00FC

//2 TMAC

//4 Transmit Control Register
#define MCR_TRANSMITCR                  0x0100

//4 AC TxOP Limit Register 0-1
#define MCR_ACTXOPLR0                   0x0104
#define MCR_ACTXOPLR1                   0x0108

//4 Control Response Frame Regiester 0-1
#define MCR_CRFR0                       0x010C
#define MCR_CRFR1                       0x0110

//4 OFDM Frame Power Register
#define MCR_OFPR                        0x0114

//4 CCK Frame Power Register
#define MCR_CFPR                        0x0118

//4 Bluetooth Co-existence Regiester 0-3
#define MCR_BTCER0                      0x011C
#define MCR_BTCER1                      0x0120
#define MCR_BTCER2                      0x0124
#define MCR_BTCER3                      0x0128

//4 BB Ramp UP Register 0-1
#define MCR_BRUR0                       0x012C
#define MCR_BRUR1                       0x0130

//4 BB Ramp Down Register
#define MCR_BRDR                        0x0134

//4 BB Processing Delay Register
#define MCR_BPDR                        0x0138

//4 CCK DCF Timeout Register
#define MCR_CDTR                        0x013C

//4 OFDM DCF TImeout Register
#define MCR_ODTR                        0x0140

//4 RTS/CTS Control Register
#define MCR_RCCR                        0x0144

//2 RMAC

//4 Receive Filter Control Register
#define MCR_RFCR                        0x0150

//4 SSID Scan Contrl Register
#define MCR_SSCR                        0x0154

//4 SSID Scan Data REgiester
#define MCR_SDR                         0x0158

//4 Current BSSID Register 0-1
#define MCR_CBR0                        0x015C
#define MCR_CBR1                        0x0160

//4 Multicast/Unicast Address Register 0-1
#define MCR_MUAR0                       0x0164
#define MCR_MUAR1                       0x0168

//4 Qos Data Register
#define MCR_QDR                         0x016C

//4 NAV Software Update Register
#define MCR_NSUR                        0x0170

//4 Rx Max Packet Length Register
#define MCR_RMLR                        0x0174

//4 ALC Control Register 0
#define MCR_ALCR0                       0x0178

//4 ALC Control Register 1
#define MCR_ALCR1                       0x017C

//2 LP

//4 Local TSF Timer Register 0-1
#define MCR_LTTR0                       0x0180
#define MCR_LTTR1                       0x0184

//4 Packet Pattern Register
#define MCR_PPR                         0x018C

//4 Lower Power Wake Starting Address Register
#define MCR_LPWSAR                      0x0190

//4 MAC Periodic Timer Control Register
#define MCR_MPTCR                       0x0194

//4 ATIM/CFP Window Register
#define MCR_ACWR                        0x0198

//4 Background SSID Scan Starting Address Register
#define MCR_BSSSAR                      0x019C

//4 Timer Data Register
#define MCR_TDR                         0x01A0

//4 Auto Transmit Control Register
#define MCR_ATCR                        0x01A4

//4 TBTT Starting Time Register
#define MCR_TSTR                        0x01A8

//4 TBTT Periodic Control Register
#define MCR_TTPCR                       0x01AC

//4 CFP Time Period Control Register
#define MCR_CTPCR                       0x01B0

//4 Low Power Control Register
#define MCR_LPCR                        0x01B4

//4 Packet Index Register
#define MCR_PIR                         0x01B8

//4 Pattern Mask Register
#define MCR_PMR                         0x01BC

//4 Updated TSF Timer Register 0-1
#define MCR_UTTR0                       0x01C0
#define MCR_UTTR1                       0x01C4

//4 TSF Timer Adjustment Register
#define MCR_TTAR                        0x01C8

//4 Low PowerOn Starting Address Register
#define MCR_LPOSAR                      0x01CC

//4 Media Time Register
#define MCR_MTR0                        0x01D0

//4 Media Time Control Register
#define MCR_MTCR0                       0x01D4

//4 Timeout Regiester
#define MCR_TR                          0x01D8

//4 Media Time Register
#define MCR_MTR1                        0x01E0

//4 Media Time Control Register
#define MCR_MTCR1                       0x01E4

//4 Service Period Control Register
#define MCR_SPCR                        0x01E8

//4 Beacon Earlier Interval Register
#define MCR_BEIR                        0x01EC


//2 BB

//4 BB ID Register CR0
#define BBCR_BIDR                       0x0200

//4 BB Configuration Register CR1
#define BBCR_BCR                        0x0204

//4 BB Interrupt Status Register CR2
#define BBCR_BISR                       0x0208

//4 BB TX Service Register
#define BBCR_BTSR                       0x020C

//4  BB OFDM TX Back-off Register 0-2
#define BBCR_BOTBOR0                    0x0210
#define BBCR_BOTBOR1                    0x0214
#define BBCR_BOTBOR2                    0x0218

//4 BB OFDM TX Filter Register 0
#define BBCR_BOTFR0                     0x021C
#define BBCR_BOTFR1                     0x0220
#define BBCR_BOTFR2                     0x0224
#define BBCR_BOTFR3                     0x0228
#define BBCR_BOTFR4                     0x022C
#define BBCR_BOTFR5                     0x0230

//4 BB CCK TX Filter Register 0
#define BBCR_BCTFR0                     0x0234
#define BBCR_BCTFR1                     0x0238
#define BBCR_BCTFR2                     0x023C

//4 BB AGC Table Register 0-34
#define BBCR_BATR0                      0x0240

//4 BB OFDM Tracking Table Register 0-11
#define BBCR_BOTTR0                     0x02CC

//4 BB CCK SQ Control Register
#define BBCR_BCSQCR                     0x02FC

//4 BB CCK Control MISC Register 0-1
#define BBCR_BCCMR0                     0x0300
#define BBCR_BCCMR1                     0x0304

//4  BB CCK Scrambler Control Register
#define BBCR_BCSCR                      0x0308

//4 BB CCK Preamble Control Register
#define BBCR_BCPCR                      0x030C

//4 BB MPDU-antenna Control Register
#define BBCR_BMACR                      0x0310

//4 BB AP Address W2 Register
#define BBCR_BAAW2R                     0x0314

//4 BB AP Address W1 Register
#define BBCR_BAAW1R                     0x0318

//4 BB AP Address W0 Register
#define BBCR_BAAW0R                     0x031C

//4 BB OFDM MISC Control Register
#define BBCR_BOMCR                      0x0320

//4 BB OFDM ACQ Frequency Tracking Control Register
#define BBCR_BOAFTCR                    0x0324

//4 BB RPI 9_8 Threshold Register(8,9)
#define BBCR_BRPI9_8TR                  0x0330


//4 RF Calibration Control Register
#define BBCR_BRFCCR                     0x0334

//4 RF Calibration Feedback Register
#define BBCR_BRFCFR                     0x0338

//4 TX DC Compensation Register
#define BBCR_BTXDCCR                    0x033C

//4 RF TX Compensation Register
#define BBCR_BRFTXCR                    0x0340

//4 RF RX Compensation Register
#define BBCR_BRFRXCR                    0x0344

//4  BB RPI RDD Control Register
#define BBCR_BRRCR                      0x0348

//4 BB RPI 1_0 Threshold Register(0-7)
#define BBCR_BRPI1_0TR                  0x034C
#define BBCR_BRPI3_2TR                  0x0350
#define BBCR_BRPI5_4TR                  0x0354
#define BBCR_BRPI7_6TR                  0x0358

//4 BB RDD Counter Register
#define BBCR_BRCR                       0x035C

//4 BB Radar Pulse Status Register
#define BBCR_BRPSR                      0x0360

//4 BB Instant Received Power Register
#define BBCR_BIRPR                      0x0364

//4 BB Counter Rx Fail Register
#define BBCR_BCRFR                      0x0368

//4 BB Counter_ED_PASS Register
#define BBCR_BCEPR                      0x036C

//4 BB Counter_OFDM_OSD_PASS Register
#define BBCR_BCOOPR                     0x0370

//4 BB Counter_OFDM_mdrdy_PASS Register
#define BBCR_BCOMPR                     0x0374

//4 BB Counter_CCK_SQ1_PASS Register
#define BBCR_BCCSQPR                    0x0378

//4 BB Counter_CCK_CRC16_PASS Register
#define BBCR_BCCCPR                     0x037C

//4 BB Counter_CCK_SFD_PASS Register
#define BBCR_BCCSPR                     0x0380

//4 BB Testmode Register
#define BBCR_BTMR                       0x0384

//4 BB Testmode TX Control Register
#define BBCR_BTTCR                      0x0388

//4 BB Clock Control Register
#define BBCR_BCLKCR                     0x038C

//4 BB TXDAC Force Data Register
#define BBCR_BTFDR                      0x0390

//4 BB RXADC Output Data Register
#define BBCR_BRODR                      0x0394

//4 New RDD Control Register
#define BBCR_BRDDCR                     0x0398

//4 RF RSSI Register
#define BBCR_BRRSR                      0x039C

//4  BB RPI / IPI 0-10 counter LSB Register
//4 BB RPI / IPI 0-10 counter MSB Register
#define BBCR_BRPI0LCR                   0x03A0
#define BBCR_BRPI0MCR                   0x03A4
//4 The offset is 8

#define BBCR_BPROBECTR                  0x03E8

//2 RF

//4 RF RF Control Register 0
#define RFCR_0                                 0x400

//4 RF RF Control Register 1
#define RFCR_1                                 0x404

//4 RF RF Control Register 2
#define RFCR_2                                 0x408

//4 RF RF Control Register 3
#define RFCR_3                                 0x40c

//4 RF RF Control Register 4
#define RFCR_4                                 0x410

//4 RF RF Control Register 5
#define RFCR_5                                 0x414

//4 RF RF Control Register 6
#define RFCR_6                                 0x418

//4 RF RF Control Register 7
#define RFCR_7                                 0x41C

//4 RF RF Control Register 8
#define RFCR_8                                 0x420

//4 RF RF Control Register 9
#define RFCR_9                                 0x424

//4 RF RF Control Register 10
#define RFCR_10                                0x428

//4 RF RF Control Register 11
#define RFCR_11                                0x42C

//4 RF RF Control Register 12
#define RFCR_12                                0x430

//4 RF RF Control Register 13
#define RFCR_13                                0x434


//2 Definition in each register
//3 CIR
#define CIR_IBSS_MASTER                 BIT(25)
#define CIR_SCAN_BUSY                   BIT(24)
#define CIR_RESET                       BIT(23)
#define CIR_WT_BUSY                     BIT(22)
#define CIR_LP_STATE                    BIT(21)
#define CIR_PLL_READY                   BIT(20)
#define CIR_REVISION_ID                 BITS(16,19)
#define CIR_CHIP_ID                     BITS(0,15)

//3 HISR
#define HISR_ABNORMAL_INT               BIT(31)
#define HISR_RCPI_INT                   BIT(30)
#define HISR_ADMIT_TIME_MET             BIT(29)
#define HISR_ALC_VIOLATE                BIT(28)
#define HISR_SCAN_DONE                  BIT(27)
#define HISR_BB_INT                     BIT(26)
#define HISR_QUIET_DONE                 BIT(25)
#define HISR_MSR_DONE                   BIT(24)
#define HISR_APSD_TIMEOUT               BIT(23)
#define HISR_WATCH_DOG                  BIT(21)
#define HISR_TX_PSPOLL_TIMEOUT          BIT(20)
#define HISR_HCCA_TXOP_SHORT            BIT(19)
#define HISR_Q_CFPOLL_NO_DATA           BIT(18)
#define HISR_NO_MORE_PKT                BIT(17)
#define HISR_TSF_DRIFT                  BIT(16)
#define HISR_PREDTIM                    BIT(15)
#define HISR_PRETBTT                    BIT(14)
#define HISR_TBTT                       BIT(13)
#define HISR_T3_TIME                    BIT(12)
#define HISR_T2_TIME                    BIT(11)
#define HISR_T1_TIME                    BIT(10)
#define HISR_T0_TIME                    BIT(9)
#define HISR_SLOW_WAKEUP                BIT(8)
#define HISR_EXT_INT                    BIT(7)
#define HISR_GPIO1                      BIT(6)
#define HISR_GPIO0                      BIT(5)
#define HISR_ATIM_W_TIMEUP              BIT(4)
#define HISR_BEACON_TR_OK               BIT(3)
#define HISR_BEACON_T_OK                BIT(2)
#define HISR_TX_DONE                    BIT(1)
#define HISR_RX_DONE                    BIT(0)

//3 HSCISR
#define HSCISR_BCN_LOSS                 BIT(10)
#define HSCISR_PKT_SEARCHED             BIT(9)
#define HSCISR_TX_NULL_FAIL             BIT(8)
#define HSCISR_TX_TRIG_FAIL             BIT(7)
#define HSCISR_TX_PSPOLL_FAIL           BIT(6)
#define HSCISR_DRIVER_OWN_BACK          BIT(5)
#define HSCISR_DTIM_CNT_MATCH           BIT(4)
#define HSCISR_BMC_TIMEOUT              BIT(3)
#define HSCISR_RX_BMC_BCN               BIT(2)
#define HSCISR_RX_UC_BCN                BIT(1)
#define HSCISR_BCN_TIMEOUT              BIT(0)

//3 HIER
#define HIER_ABNORMAL_INT               BIT(31)
#define HIER_LOW_RCPI_INT               BIT(30)
#define HIER_ADMIT_TIME_MET             BIT(29)
#define HIER_ALC_VIOLATE                BIT(28)
#define HIER_SCAN_DONE                  BIT(27)
#define HIER_BB_INT                     BIT(26)
#define HIER_QUIET_DONE                 BIT(25)
#define HIER_MSR_DONE                   BIT(24)
#define HIER_APSD_TIMEOUT               BIT(23)
#define HIER_WATCH_DOG                  BIT(21)
#define HIER_TX_PSPOLL_TIMEOUT          BIT(20)
#define HIER_Q_CFPOLL_NO_DATA           BIT(18)
#define HIER_NO_MORE_PKT                BIT(17)
#define HIER_TSF_DRIFT                  BIT(16)
#define HIER_PREDTIM                    BIT(15)
#define HIER_PRETBTT                    BIT(14)
#define HIER_TBTT                       BIT(13)
#define HIER_T3_TIME                    BIT(12)
#define HIER_T2_TIME                    BIT(11)
#define HIER_T1_TIME                    BIT(10)
#define HIER_T0_TIME                    BIT(9)
#define HIER_SLOW_WAKEUP                BIT(8)
#define HIER_EXT_INT                    BIT(7)
#define HIER_GPIO1                      BIT(6)
#define HIER_GPIO0                      BIT(5)
#define HIER_ATIM_W_TIMEUP              BIT(4)
#define HIER_BEACON_TR_OK               BIT(3)
#define HIER_BEACON_T_OK                BIT(2)
#define HIER_TX_DONE                    BIT(1)
#define HIER_RX_DONE                    BIT(0)


//3 HSCIER
#define HSCIER_BCN_LOSS                 BIT(10)
#define HSCIER_PKT_SEARCHED             BIT(9)
#define HSCIER_TX_NULL_FAIL             BIT(8)
#define HSCIER_TX_TRIG_FAIL             BIT(7)
#define HSCIER_TX_PSPOLL_FAIL           BIT(6)
#define HSCIER_DRIVER_OWN_BACK          BIT(5)
#define HSCIER_DTIM_CNT_MATCH           BIT(4)
#define HSCIER_BMC_TIMEOUT              BIT(3)
#define HSCIER_RX_BMC_BCN               BIT(2)
#define HSCIER_RX_UC_BCN                BIT(1)
#define HSCIER_BCN_TIMEOUT              BIT(0)


#define HIER_DEFAULT                    (HIER_ABNORMAL_INT          | \
                                         HIER_LOW_RCPI_INT          | \
                                         HIER_ADMIT_TIME_MET        | \
                                         HIER_SCAN_DONE             | \
                                         HIER_ALC_VIOLATE           | \
                                         HIER_BB_INT                | \
                                         HIER_APSD_TIMEOUT          | \
                                         HIER_WATCH_DOG             | \
                                         HIER_TX_PSPOLL_TIMEOUT     | \
                                         HIER_TSF_DRIFT             | \
                                         HIER_TBTT                  | \
                                         HIER_SLOW_WAKEUP           | \
                                         HIER_TX_DONE               | \
                                         HIER_RX_DONE)

#define HSCIER_DEFAULT                  (HSCIER_BCN_LOSS            | \
                                         HSCIER_TX_NULL_FAIL        | \
                                         HSCIER_TX_TRIG_FAIL        | \
                                         HSCIER_TX_PSPOLL_FAIL      | \
                                         HSCIER_DTIM_CNT_MATCH      | \
                                         HSCIER_BMC_TIMEOUT         | \
                                         HSCIER_RX_BMC_BCN          | \
                                         HSCIER_RX_UC_BCN           | \
                                         HSCIER_BCN_TIMEOUT)

//3 HCR
//4 0: RX, 1: TX
#define HCR_FUNC_TX_RX                  BIT(31)

//4 0: Normal function
#define HCR_INT_OUT_SELECT              BIT(19)

//4 0: active low, 1: active high
#define HCR_POLARITY_HIGH               BIT(18)
#define HCR_POLARITY_LOW                0

//4 802.1x check enable
#define HCR_1X_CK_EN                    BIT(13)
#define HCR_1X_CK_DISABLE               0

//4 0: HISR and HSCISR read clear
//4 1: HISR and HSCISR write 1 clear
#define HCR_INT_ACC_CTRL                BIT(12)

#define HCR_IPV6_RX_OFFLOAD_EN          BIT(14)
#define HCR_IP_RX_OFFLOAD_EN            BIT(11)
#define HCR_TCP_RX_OFFLOAD_EN           BIT(10)
#define HCR_UDP_RX_OFFLOAD_EN           BIT(9)
#define HCR_TCPIP_TX_OFFLOAD_EN         BIT(8)
#define HCR_CSUM_OFFLOAD_MASK           (HCR_TCPIP_TX_OFFLOAD_EN | \
                                         HCR_UDP_RX_OFFLOAD_EN | \
                                         HCR_TCP_RX_OFFLOAD_EN | \
                                         HCR_IP_RX_OFFLOAD_EN | \
                                         HCR_IPV6_RX_OFFLOAD_EN)

#define HCR_RX_DATA_RD_DONE             BIT(5)


//3 SDIOCSR
#define SDIOCSR_MAX_TXSTATUS_PKT        BITS(20,23)
#define SDIOCSR_MAX_TXSTATUS_PKT_OFFSET 20

#define SDIOCSR_MAX_RECEIVE_PKT         BITS(16,19)
#define SDIOCSR_MAX_RECEIVE_PKT_OFFSET  16

#define SDIOCSR_FUN_MODE                BIT(2)
#define SDIOCSR_SDIO_ABORT              BIT(1)


//3 SPICSR
#define SPICSR_MAX_TXSTATUS             BITS(20,23)
#define SPICSR_MAX_RECEIVE_PKT_LEN      BITS(16,19)

//4 00: 8-BIT MODE, 01: 16-BIT MODE
//4 10: 32-BIT MODE, 11: 8-BIT MODE
#define SPICSR_MODE_SEL                 BITS(9,10)
#define SPICSR_MODE_SEL_8BIT            0x00000000
#define SPICSR_MODE_SEL_16BIT           0x00000200
#define SPICSR_MODE_SEL_32BIT           0x00000400

//4 0: LITTLE ENDIAN, 1: BIG ENDIAN
#define SPICSR_BIG_ENDIAN               BIT(8)
#define SPICSR_LITTLE_ENDIAN            0

//4 0: NORMAL MODE, 1: FUNCTION PATTER GENERATION MODE
#define SPICSR_FUNC_MODE                BIT(2)


#define SPICSR_INTOUT_MODE              BIT(1)
#define SPICSR_DATAOUT_MODE             BIT(0)


//3 CFCSR
#define CFCSR_BIG_ENDIAN                BIT(8)
#define CFCSR_LITTLE_ENDIAN             0
#define CFCSR_PAGE_SEL                  BITS(4,6)
#define CFCSR_FUNC_PG_MODE              BIT(2)
#define CFCSR_WD_STS                    BIT(1)

//3 HPI
#define HPICSR_WAIT_SIG_CTRL_NOT_LOW    BIT(15)
#define HPICSR_BIG_ENDIAN               BIT(8)
#define HPICSR_FUNC_PG_MODE             BIT(2)
#define HPICSR_WD_STS                   BIT(1)

//3 eHPI
#define EHPICSR_FUNC_PG_MODE            BIT(2)
#define EHPICSR_WD_STS                  BIT(1)
#define EHPICSR_INTBLK                  BIT(0)


//3 RR
//4 Enable moving average
#define RR_ENABLE_MA                    BIT(31)
#define RR_RCPI_PARM_1_OF_16            0
#define RR_RCPI_PARM_1_OF_8             BIT(24)
#define RR_RCPI_PARM_1_OF_4             BIT(25)
#define RR_RCPI_PARM_1_OF_1             BITS(24,25)

#define RR_RCPI_MOV_AVE_PARM_MASK       BITS(24,25)
#define RR_RCPI_HIGH_THRESHOLD_MASK     BITS(16,23)
#define RR_RCPI_LOW_THRESHOLD_MASK      BITS(8,15)
#define RR_RCPI_VALUE_MASK              BITS(0,7)

#define RR_RCPI_HIGH_MAXIMUM            255
#define RR_RCPI_LOW_MINIMUM             0


#define RR_SET_MOV_AVE_PARA(_movAvePara) \
            (((_movAvePara) << 0) & RR_RCPI_MOV_AVE_PARM_MASK)
#define RR_SET_HIGH_RCPI_THRESHOLD(_rcpiHighThreshold) \
            (((_rcpiHighThreshold) << 16) & RR_RCPI_HIGH_THRESHOLD_MASK)
#define RR_SET_LOW_RCPI_THRESHOLD(_rcpiLowThreshold) \
            (((_rcpiLowThreshold) << 8) & RR_RCPI_LOW_THRESHOLD_MASK)
#define RR_GET_RCPI(_rr) \
            ((RCPI)((_rr) & RR_RCPI_VALUE_MASK))

/* QUEUE_ID */
typedef enum _ENUM_QUEUE_ID_T
{
    ENUM_QUEUE_ID_AC0 = 0,
    ENUM_QUEUE_ID_AC1,
    ENUM_QUEUE_ID_AC2,
    ENUM_QUEUE_ID_AC3,
    ENUM_QUEUE_ID_TSB,
    ENUM_QUEUE_ID_AC4,
    ENUM_QUEUE_ID_RX,
    ENUM_QUEUE_ID_SCAN = 8,
    ENUM_QUEUE_ID_SCAN_CTRL,
    ENUM_QUEUE_ID_MAX
} ENUM_QUEUE_ID_T, *PENUM_QUEUE_ID_T;

//3 QAR
#define QAR_SEL_AC0                     (ENUM_QUEUE_ID_AC0       << 28)
#define QAR_SEL_AC1                     (ENUM_QUEUE_ID_AC1       << 28)
#define QAR_SEL_AC2                     (ENUM_QUEUE_ID_AC2       << 28)
#define QAR_SEL_AC3                     (ENUM_QUEUE_ID_AC3       << 28)
#define QAR_SEL_TSBEACON                (ENUM_QUEUE_ID_TSB       << 28)
#define QAR_SEL_AC4                     (ENUM_QUEUE_ID_AC4       << 28)
#define QAR_SEL_RX                      (ENUM_QUEUE_ID_RX        << 28)
#define QAR_SEL_SCAN                    (ENUM_QUEUE_ID_SCAN      << 28)
#define QAR_SEL_SCAN_CTRL               (ENUM_QUEUE_ID_SCAN_CTRL << 28)
#define QAR_READ_MODE                   BIT(26)

//3 HWTDR
#define HWTDR_CLR_TX_VLD                BIT(8)
#define HWTDR_CLR_RX_VLD                BIT(11)

#define HWTDR_UPDATE_MODE_0             0
#define HWTDR_UPDATE_MODE_1             1
#define HWTDR_UPDATE_MODE_2             2
#define HWTDR_UPDATE_MODE_3             3

#define HWTDR_UPDATE_MODE_0_SZ          4
#define HWTDR_UPDATE_MODE_1_SZ          8
#define HWTDR_UPDATE_MODE_2_SZ          16
#define HWTDR_UPDATE_MODE_3_SZ          48

#define HWTDR_RCA1_ALL                  0
#define HWTDR_RCA1_MUAR_ONLY            1
#define HWTDR_RCA1_BC_ONLY              2
#define HWTDR_RCA1_MUAR_BC              3

//3 HWTCR
#define HWTCR_CLR_TX_VLD                BIT(9)
#define HWTCR_CLR_RX_VLD                BIT(8)
#define HWTCR_B_RMODE                   BIT(5)
#define HWTCR_W_RMODE                   BIT(4)

#define HWTCR_ENTRY_SEL_0               0
#define HWTCR_ENTRY_SEL_1               0x1
#define HWTCR_ENTRY_SEL_2               0x2
#define HWTCR_ENTRY_SEL_3               0x3
#define HWTCR_ENTRY_SEL_4               0x4
#define HWTCR_ENTRY_SEL_5               0x5
#define HWTCR_ENTRY_SEL_6               0x6
#define HWTCR_ENTRY_SEL_7               0x7
#define HWTCR_ENTRY_SEL_8               0x8
#define HWTCR_ENTRY_SEL_9               0x9
#define HWTCR_ENTRY_SEL_10              0xA
#define HWTCR_ENTRY_SEL_11              0xB
#define HWTCR_ENTRY_SEL_12              0xC

#define WLAN_WT_IS_BUSY(_u4Value) \
            ((_u4Value) & CIR_WT_BUSY)

#define MASK_HWTDR_TINDEX               BITS(0,3)
#define MASK_HWTDR_UPMODE               BITS(4,5)

#define HWTDR_MODE_OFFSET               4
#define HWTDR_TV_OFFSET                 8
#define HWTDR_TKV_OFFSET                9
#define HWTDR_T1X_OFFSET               10
#define HWTDR_RV_OFFSET                11
#define HWTDR_RKV_OFFSET               12
#define HWTDR_RCA1_OFFSET              13
#define HWTDR_RCA2_OFFSET              15
#define HWTDR_RCID_OFFSET              16
#define HWTDR_R1X_OFFSET               17
#define HWTDR_Q_OFFSET                 18
#define HWTDR_A_OFFSET                 19
#define HWTDR_IKV_OFFSET               20
#define HWTDR_CIPHER_OFFSET            24
#define HWTDR_KID_OFFSET               28

//3 HLPCR
#define HLPCR_TOG_INT_EN                BIT(31)
#define HLPCR_RESERVED                  BIT(30)
#define HLPCR_HIFMAC_LOGRST             BIT(29)
#define HLPCR_HIF_REGRST                BIT(28)
#define HLPCR_MAC_REGRST                BIT(27)
#define HLPCR_BB_REGRST                 BIT(26)
#define HLPCR_BB_LOGRST                 BIT(25)
#define HLPCR_CHIP_RESET                BIT(24)
#define HLPCR_OSC_EN                    BIT(23)
#define HLPCR_OSC_OUT_EN                BIT(22)
#define HLPCR_BG_EN                     BIT(21)
#define HLPCR_PLL_EN                    BIT(20)
#define HLPCR_ADC_BUFFER_EN             BIT(19)
#define HLPCR_INTERNAL_32K_EN           BIT(18)
#define HLPCR_EXTERNAL_32K_EN           BIT(17)
#define HLPCR_RF_SX_EN                  BIT(16)
#define HLPCR_OSC_PD                    BIT(15)
#define HLPCR_OSC_OUT_PD                BIT(14)
#define HLPCR_BG_PD                     BIT(13)
#define HLPCR_PLL_PD                    BIT(12)
#define HLPCR_ADC_BUFFER_PD             BIT(11)
#define HLPCR_INTERNAL_32K_PD           BIT(10)
#define HLPCR_EXTERNAL_32K_PD           BIT(9)
#define HLPCR_RF_SX_PD                  BIT(8)
#define HLPCR_DELAY_INT                 BIT(7)
#define HLPCR_GINT_ISENABLE             BIT(6)
#define HLPCR_DISABLE_GINT              BIT(5)
#define HLPCR_ENABLE_GINT               BIT(4)
#define HLPCR_PLL_CLOCK_GATED           BIT(3)
#define HLPCR_LP_OWN_STATE              BIT(2)
#define HLPCR_LP_OWN_CLR                BIT(1)
#define HLPCR_LP_OWN_SET                BIT(0)

#define HLPCR_PD_ALL                    BITS(8, 15)

//3 HFCR        0x0040
#define HFCR_SELECT_DATA_FIFO           0
#define HFCR_SELECT_WLAN_FIFO           BIT(15)
#define HFCR_FIFO_WRITE                 BIT(14)
#define HFCR_FIFO_READ                  BIT(13)
#define HFCR_FIFO_ADDR_MASK             BITS(0,12)

//3 HITR        0x0048
#define HFCR_RX_INT_THRESHOLD           BITS(0, 13)



//2 CONFIG
//3 DBGR
#define DBGR_WATCH_DOG_TIMER_ENABLE     BIT(29)
#define DBGR_DEBUG_ENABLE               BIT(28)
#define DBGR_SLEEP_TIMEOUT_UNIT_1024_TU BIT(15)
#define DBGR_SLEEP_TIMEOUT_COUNT        BITS(0,14)

//3 ESCR
#define ESCR_EE_TYPE_MASK               BITS(17,19)
#define ESCR_EE_TYPE_NONE               0
#define ESCR_128BYTE                    BIT(17)
#define ESCR_256BYTE                    BIT(17) | BIT(18)
#define ESCR_512BYTE                    BIT(17) | BIT(18)
#define ESCR_1024BYTE                   BIT(19)
#define ESCR_2048BYTE                   BIT(17) | BIT(19)

#define ESCR_SW_ACCESS                  BIT(16)

#define ESCR_EECS                       BIT(3)
#define ESCR_EESK                       BIT(2)
#define ESCR_EEDI                       BIT(1)

//3 EADR
#define EADR_EE_READ                    BIT(31)
#define EADR_EE_WRITE                   BIT(30)
#define EADR_EE_RDY                     BIT(29)
#define EADR_EE_CSERR                   BIT(28)
#define EADR_EE_RECALL                  BIT(27)

#define EADR_EE_ADDRESS_OFFSET          16
#define EADR_EE_DATA_MASK               BITS(0, 15)

//3 32KCCR
#define X32KCCR_CAL_START               BIT(31)
#define X32KCCR_CLK_SEL_RING_OSC        BIT(30)
#define X32KCCR_RESIDUAL_1000           BITS(24,27)
#define X32KCCR_RESIDUAL_100            BITS(20,23)
#define X32KCCR_RESIDUAL_10             BITS(16,19)
#define X32KCCR_SLOW_CLOCK_TSF          BITS(0,5)

//3 CCR
#define CCR_BB_CLK_OFF                  0
#define CCR_BB_CLK_TXRX                 BIT(31)
#define CCR_BB_CLK_ON                   BIT(30,31)

#define CCR_TXDAC_PWD_MODE_RFTX         0
#define CCR_TXDAC_PWD_MODE_TXPE         BIT(27)
#define CCR_TXDAC_PWD_MODE_RFTX_TXPE    BIT(28)
#define CCR_TXDAC_PWD_MODE_ON           BIT(29)
#define CCR_TXDAC_PWD_MODE_OFF          BIT(29) | BIT(27)
#define CCR_TXDAC_PWD_MODE_MASK         BITS(27,29)


#define CCR_TXDAC_CLK_MODE_RFTX         0
#define CCR_TXDAC_CLK_MODE_TXPE         BIT(24)
#define CCR_TXDAC_CLK_MODE_RFTX_TXPE    BIT(25)
#define CCR_TXDAC_CLK_MODE_ON           BIT(26)
#define CCR_TXDAC_CLK_MODE_OFF          BIT(26) | BIT(24)

#define CCR_RXDAC_PWD_MODE_RFRX         0
#define CCR_RXDAC_PWD_MODE_RXPE         BIT(19)
#define CCR_RXDAC_PWD_MODE_RFRX_RXPE    BIT(20)
#define CCR_RXDAC_PWD_MODE_ON           BIT(21)
#define CCR_RXDAC_PWD_MODE_OFF          BIT(21) | BIT(19)

#define CCR_RXDAC_CLK_MODE_RFRX         0
#define CCR_RXDAC_CLK_MODE_RXPE         BIT(16)
#define CCR_RXDAC_CLK_MODE_RFRX_RXPE    BIT(17)
#define CCR_RXDAC_CLK_MODE_ON           BIT(18)
#define CCR_RXDAC_CLK_MODE_OFF          BIT(18) | BIT(16)

#define CCR_WLAN_CLK_CTRL_11J           BIT(15)
#define CCR_WLAN_CLK_CTRL_11ABG         0

#define CCR_ALCADC_PWD_MODE_BB_ALC      0
#define CCR_ALCADC_PWD_MODE_ON          BIT(14)
#define CCR_ALCADC_PWD_MODE_OFF         BIT(13) | BIT(14)


#define CCR_ALCADC_CLK_MODE_BB_ALC      0
#define CCR_ALCADC_CLK_MODE_ON          BIT(12)
#define CCR_ALCADC_CLK_MODE_OFF         BIT(11) | BIT(12)


#define CCR_EEIF_GC_DIS                 BIT(8)
#define CCR_EEIF_GC_EN                  0

#define CCR_BUF_LOGIC_GC_DIS            BIT(7)
#define CCR_BUF_LOGIC_GC_EN             0

#define CCR_LP_LOGIC_GC_DIS             BIT(6)
#define CCR_LP_LOGIC_GC_EN              0

#define CCR_RMAC_LOGIC_GC_DIS           BIT(5)
#define CCR_RMAC_LOGIC_GC_EN            0

#define CCR_TMAC_LOGIC_GC_DIS           BIT(4)
#define CCR_TMAC_LOGIC_GC_EN            0

#define CCR_SEC_LOGIC_GC_DIS            BIT(3)
#define CCR_SEC_LOGIC_GC_EN             0

#define CCR_ARB_LOGIC_GC_DIS            BIT(2)
#define CCR_ARB_LOGIC_GC_EN             0

#define CCR_HIF_LOGIC_GC_DIS            BIT(1)
#define CCR_HIF_LOGIC_GC_EN             0

#define CCR_MCR_GC_DIS                  BIT(0)
#define CCR_MCR_GC_EN                   0


//3 ODCR
//4 2006/09/11, mikewu, <todo> ODCR

//3 IOUDR
#define IOUDR_BT_PRI_PU                 BIT(19)
#define IOUDR_WE_N_PU                   BIT(18)
#define IOUDR_OE_N_PU                   BIT(17)
#define IOUDR_CS_N_PU                   BIT(16)
#define IOUDR_BT_PRI_PD                 BIT(3)
#define IOUDR_WE_N_PD                   BIT(2)
#define IOUDR_OE_N_PD                   BIT(1)
#define IOUDR_CS_N_PD                   BIT(0)

//3 IOPCR
//4 2006/09/11, mikewu, <todo> IOPCR
#define IOPCR_ALL_TRAP_PIN_OUTPUT_EN    BITS(8, 12)
#define IOPCR_IO_TR_SW_P_DIR            BIT(12)
#define IOPCR_IO_TR_SW_N_DIR            BIT(11)
#define IOPCR_IO_ANT_SEL_P_DIR          BIT(10)
#define IOPCR_IO_ANT_SEL_N_DIR          BIT(9)

//3 ACDR
#define ACDR4_RG_DIVS                   BITS(16, 21)

#define ACDR4_RG_DIVS_20M               0x16
#define ACDR4_RG_DIVS_40M               0x0a
#define ACDR4_RG_DIVS_13M               0x26
#define ACDR4_RG_DIVS_26M               0x12
#define ACDR4_RG_DIVS_19_2M             0x17

//3 SCR
#define SCR_GPIO1_POLAR_HIGH            BIT(29)
#define SCR_GPIO1_POLAR_LOW             0
#define SCR_GPIO1_CHAIN_SEL             BIT(28)
#define SCR_BTFREQ_SEL                  BIT(27)

#define SCR_GPIO1_WDATA                 BIT(26)
#define SCR_GPIO1_ENABLE_OUTPUT_MODE    BIT(25)
#define SCR_GPIO1_ENABLE_INPUT_MODE     0
#define SCR_GPIO1_RDATA                 BIT(24)

#define SCR_GPIO0_POLAR_HIGH            BIT(21)
#define SCR_GPIO0_POLAR_LOW             0
#define SCR_GPIO0_CHAIN_SEL             BIT(20)

#define SCR_BT_ACT_SEL                  BIT(19)

#define SCR_GPIO0_WDATA                 BIT(18)
#define SCR_GPIO0_ENABLE_OUTPUT_MODE    BIT(17)
#define SCR_GPIO0_ENABLE_INPUT_MODE     0
#define SCR_GPIO0_RDATA                 BIT(16)

#define SCR_GPIO2_POLAR_HIGH            BIT(13)
#define SCR_GPIO2_POLAR_LOW             0
#define SCR_GPIO2_CHAIN_SEL             BIT(12)
#define SCR_GPIO2_WDATA                 BIT(10)
#define SCR_GPIO2_ENABLE_OUTPUT_MODE    BIT(9)
#define SCR_GPIO2_ENABLE_INPUT_MODE     0
#define SCR_GPIO2_RDATA                 BIT(8)

#define SCR_DB_EN_GIO                   BIT(7)
#define SCR_DB_EN_EINT                  BIT(6)

#define SCR_EINT_POLAR_HIGH             BIT(5)
#define SCR_EINT_POLAR_LOW              0

#define SCR_ACK                         BIT(4)

#define SCR_SENS_EDGE                   0
#define SCR_SENS_LEVEL                  BIT(3)

#define SCR_MIB_CTR_RD_CLEAR            BIT(2)
#define SCR_OP_MODE_STA                 0
#define SCR_OP_MODE_ADHOC               BIT(0)
#define SCR_OP_MODE_MASK                BITS(0,1)

//3 LCR
#define LCR_LED_OFF_CONT                BITS(0,7)
#define LCR_LED_ON_CONT                 BITS(8,15)
#define LCR_LED_MODE_TX_WO_BEACON       BIT(16)
#define LCR_LED_MODE_TX_BEACON          BIT(17)
#define LCR_LED_MODE_RX                 BIT(18)
#define LCR_LED_MODE_RX_EX_RFCR_BEACON  BIT(19)
#define LCR_LED_MODE_RX_RFCR_BEACON     BIT(20)
#define LCR_LED_POLARITY                BIT(24)
#define LCR_LED_OUTPUT                  BIT(25)


//3 RICR
#define RICR_ANT_SEL_N_SW_MODE          BIT(24)
#define RICR_ANT_SEL_P_SW_MODE          BIT(23)
#define RICR_SWANT_SEL_N_HIGH           BIT(22)
#define RICR_SWANT_SEL_N_LOW            0
#define RICR_SWANT_SEL_P_HIGH           BIT(21)
#define RICR_SWANT_SEL_P_LOW            0

#define RICR_PA5ENPOLARITY              BIT(20)

#define RICR_PA2ENPOLARITY              BIT(19)

//4 Enable PA 2.4G Hz
#define RICR_PA2EN                      BIT(18)

//4 Enable PA 5G Hz
#define RICR_PA5EN                      BIT(17)

//4 make rxhp to 1
#define RICR_SW_RXHP                    BIT(13)


#define RICR_RXHP_MODE_CCK              BIT(12)
#define RICR_RXHP_MODE_OFDR             0

//4 transmission control by MT5921
#define RICR_MAC_TX_EN                  BIT(10)

//4 receiving control by MT5921
#define RICR_MAC_RX_EN                  BIT(9)
#define RICR_MAC_TXRX_MASK              BITS(9,10)

#define RICR_RF_SW_MODE                 BIT(8)

#define RICR_SW_TR_SW_N                 BIT(6)
#define RICR_SW_RXADC_DCCAL_EN          BIT(5)
#define RICR_SW_RF_TX                   BIT(4)
#define RICR_SW_RF_RX                   BIT(3)
#define RICR_SW_TR_SW_P                 BIT(2)
#define RICR_SW_TX_PE                   BIT(1)
#define RICR_SW_RX_PE                   BIT(0)


//3 RSCR
#define RSCR_POLARITY_HIGH              BIT(30)
#define RSCR_POLARITY_LOW               0

#define RSCR_SHIFT_MODE_LEFT            BIT(28)
#define RSCR_SHIFT_MODE_RIGHT           0

#define RSCR_SW_RF_MODE                 BIT(27)

#define RSCR_SW_RFEEDI                  BIT(26)
#define RSCR_SW_RFSE_N                  BIT(25)


//3 RSDR
#define RSDR_SYNT_PROG_START            BIT(31)
#define RSDR_RF_READ                    BIT(30)
#define RSDR_RF_WRITE                   0

//3 MMCR0
#define ARPR1_FAILCOUNT_UP_LIMIT        BITS(16, 31)
#define ARPR1_FAILCOUNT_DOWN_LIMIT      BITS(0, 15)


//3 MMCR0
#define MMCR0_BSS                       BIT(22)
#define MMCR0_BSS_REQ                   BIT(21)
#define MMCR0_IPI_HISTOGRAM_REQ         BIT(20)
#define MMCR0_RPI_HISTOGRAM_REQ         BIT(19)
#define MMCR0_CCA_REQ                   BIT(18)
#define MMCR0_RADAR_REQ                 BIT(17)
#define MMCR0_BBRXSTS_REQ               BIT(16)

//3 QCCR0
#define QCCR0_QUIET_ENABLE              BIT(31)


//3 MIBSCR
#define MIBSCR_TX_COUNT_INDEX           0
#define MIBSCR_BEACON_TX_COUNT_INDEX    1
#define MIBSCR_FRAME_RETRY_COUNT_INDEX  2
#define MIBSCR_RTS_RETRY_COUNT_INDEX    3
#define MIBSCR_RX_FCS_ERROR_COUNT_INDEX 8
#define MIBSCR_RX_FIFO_FULL_COUNT_INDEX 9
#define MIBSCR_RX_MPDU_COUNT_INDEX      10
#define MIBSCR_CHANNEL_IDLE_COUNT_INDEX 11
#define MIBSCR_CCATIME_INDEX            12
#define MIBSCR_CCA_NAV_TX_TIME_INDEX    13
#define MIBSCR_BEACON_TIMEOUT_COUNT_INDEX    14

#define MIBSCR_INDEX_MASK               BITS(0,3)
#define MIBSCR_INDEX(counter)           MIBSCR_##counter##_INDEX

#define MIBSCR_TX_COUNT_EN              BIT(27)
#define MIBSCR_BEACON_TX_COUNT_EN       BIT(26)
#define MIBSCR_FRAME_RETRY_COUNT_EN     BIT(25)
#define MIBSCR_RTS_RETRY_COUNT_EN       BIT(24)
#define MIBSCR_BEACON_TIMEOUT_COUNT_EN  BIT(22)
#define MIBSCR_RX_FCS_ERROR_COUNT_EN    BIT(21)
#define MIBSCR_RX_FIFO_FULL_COUNT_EN    BIT(20)
#define MIBSCR_RX_MPDU_COUNT_EN         BIT(19)
#define MIBSCR_CHANNEL_IDLE_COUNT_EN    BIT(18)
#define MIBSCR_CCATIME_EN               BIT(17)
#define MIBSCR_CCA_NAV_TX_TIME_EN       BIT(16)
#define MIBSCR_EN(counter)              MIBSCR_##counter##_EN

typedef enum _MIB_COUNTER_E {
    TX_COUNT = 0,
    BEACON_TX_COUNT,
    FRAME_RETRY_COUNT,
    RTS_RETRY_COUNT,
    RX_FCS_ERROR_COUNT,
    RX_FIFO_FULL_COUNT,
    RX_MPDU_COUNT,
    CHANNEL_IDLE_COUNT,
    CCATIME,
    CCA_NAV_TX_TIME,
    NUM_MIB_COUNTERS
} MIB_COUNTER_E;

#define MIBSR_DEFAULT   (MIBSCR_BEACON_TX_COUNT_EN | \
                         MIBSCR_BEACON_TIMEOUT_COUNT_EN |\
                         MIBSCR_RX_FCS_ERROR_COUNT_EN | \
                         MIBSCR_RX_FIFO_FULL_COUNT_EN | \
                         MIBSCR_RX_MPDU_COUNT_EN \
                         )


//3 0x00D0 - QCR
#define QCR_QUE_FLUSH_OFFSET            16
#define QCR_QUE_STOP_OFFSET             8
#define QCR_QUE_START_OFFSET            0

#define QCR_RX_FLUSH                    (BIT(RXQ) << QCR_QUE_FLUSH_OFFSET)
#define QCR_AC4_FLUSH                   (BIT(AC4) << QCR_QUE_FLUSH_OFFSET)
#define QCR_TS_B_FLUSH                  (BIT(TS0) << QCR_QUE_FLUSH_OFFSET)
#define QCR_AC3_FLUSH                   (BIT(AC3) << QCR_QUE_FLUSH_OFFSET)
#define QCR_AC2_FLUSH                   (BIT(AC2) << QCR_QUE_FLUSH_OFFSET)
#define QCR_AC1_FLUSH                   (BIT(AC1) << QCR_QUE_FLUSH_OFFSET)
#define QCR_AC0_FLUSH                   (BIT(AC0) << QCR_QUE_FLUSH_OFFSET)

#define QCR_RX_STOP                     (BIT(RXQ) << QCR_QUE_STOP_OFFSET)
#define QCR_AC4_STOP                    (BIT(AC4) << QCR_QUE_STOP_OFFSET)
#define QCR_TS_B_STOP                   (BIT(TS0) << QCR_QUE_STOP_OFFSET)
#define QCR_AC3_STOP                    (BIT(AC3) << QCR_QUE_STOP_OFFSET)
#define QCR_AC2_STOP                    (BIT(AC2) << QCR_QUE_STOP_OFFSET)
#define QCR_AC1_STOP                    (BIT(AC1) << QCR_QUE_STOP_OFFSET)
#define QCR_AC0_STOP                    (BIT(AC0) << QCR_QUE_STOP_OFFSET)

#define QCR_RX_START                    (BIT(RXQ) << QCR_QUE_START_OFFSET)
#define QCR_AC4_START                   (BIT(AC4) << QCR_QUE_START_OFFSET)
#define QCR_TS_B_START                  (BIT(TS0) << QCR_QUE_START_OFFSET)
#define QCR_AC3_START                   (BIT(AC3) << QCR_QUE_START_OFFSET)
#define QCR_AC2_START                   (BIT(AC2) << QCR_QUE_START_OFFSET)
#define QCR_AC1_START                   (BIT(AC1) << QCR_QUE_START_OFFSET)
#define QCR_AC0_START                   (BIT(AC0) << QCR_QUE_START_OFFSET)

#define QCR_ALL_FLUSH                   (HW_QUE_MASK << QCR_QUE_FLUSH_OFFSET)
#define QCR_ALL_STOP                    (HW_QUE_MASK << QCR_QUE_STOP_OFFSET)
#define QCR_ALL_STOP_FLUSH              (QCR_ALL_STOP | QCR_ALL_FLUSH)
#define QCR_ALL_START                   (HW_QUE_MASK << QCR_QUE_START_OFFSET)

#define QCR_TX_FLUSH                    (HW_QUE_TX_MASK << QCR_QUE_FLUSH_OFFSET)
#define QCR_TX_STOP                     (HW_QUE_TX_MASK << QCR_QUE_STOP_OFFSET)
#define QCR_TX_STOP_FLUSH               (QCR_TX_STOP | QCR_TX_FLUSH)
#define QCR_TX_START                    (HW_QUE_TX_MASK << QCR_QUE_START_OFFSET)


//3 0x00D4 - AIFSR
#define AIFSR_CW_CTRL_ENLARGED          BIT(16)
#define AIFSR_AIFS_MASK                 BITS(0,3)
#define AIFSR_AIFS0_MASK                BITS(0,3)
#define AIFSR_AIFS1_MASK                BITS(4,7)
#define AIFSR_AIFS2_MASK                BITS(8,11)
#define AIFSR_AIFS3_MASK                BITS(12,15)
#define AIFSR_AIFS0_OFFSET              0
#define AIFSR_AIFS1_OFFSET              4
#define AIFSR_AIFS2_OFFSET              8
#define AIFSR_AIFS3_OFFSET              12

//4 0x00D8  - MTCLR
#define MTCLR_RATE1_MASK                BITS(0,2)
#define MTCLR_RATE2_MASK                BITS(4,6)
#define MTCLR_RATE3_MASK                BITS(8,10)
#define MTCLR_RATE4_MASK                BITS(12,14)
#define MTCLR_RTS_MASK                  BITS(20,23)
#define MTCLR_BR_MASK                   BITS(24,27)
#define MTCLR_RATE1_OFFSET              0
#define MTCLR_RATE2_OFFSET              4
#define MTCLR_RATE3_OFFSET              8
#define MTCLR_RATE4_OFFSET              12
#define MTCLR_RTS_OFFSET                20
#define MTCLR_BR_OFFSET                 24
#define MTCLR_DISABLE_ALL_RATES         0


//3 0x00DC - ACCWXR0
#define ACCWXR0_AC0_CWMAX_OFFSET        0
#define ACCWXR0_AC1_CWMAX_OFFSET        16

//3 0x00E0 - ACCWXR1
#define ACCWXR1_AC2_CWMAX_OFFSET        0
#define ACCWXR1_AC3_CWMAX_OFFSET        16

//3 0x00E4 - ACCWIR
#define ACCWIR_AC_CWMIN_MASK            BITS(0,7)
#define ACCWIR_AC0_CWMIN_OFFSET         0
#define ACCWIR_AC1_CWMIN_OFFSET         8
#define ACCWIR_AC2_CWMIN_OFFSET         16
#define ACCWIR_AC3_CWMIN_OFFSET         24

//3 0x00E8 - AC4CWR
#define AC4CWR_AC4_CWMAX_OFFSET         0
#define AC4CWR_AC4_CWMIN_MASK           BITS(0,7)
#define AC4CWR_AC4_CWMIN_OFFSET         16
#define AC4CWR_AIFS4_MASK               BITS(0,3)
#define AC4CWR_AIFS4_OFFSET             24


//3 DRNGR
#define DRNGR_FIXED                     BIT(16)

//3 MSCR
#define MSCR_TXOP_REMAINING_EN          (0x1 << 10)
#define MSCR_TX_LENGTH_EN               (0x2 << 10)
#define MSCR_TX_SEQCTRL_EN              (0x3 << 10)
#define MSCR_TXOP_REMAINING_MASK        (0x3 << 10)
#define MSCR_TX_PKT_CS_EN               BIT(9)
#define MSCR_TX_STATUS_TD_EN            BIT(8)
#define MSCR_RX_BBP_RXSTS_SEL           BIT(5)
#define MSCR_RX_TRANSLATION_EN          BIT(4)
#define MSCR_RX_CS_EN                   BIT(3)
#define MSCR_RX_STATUS_G2_EN            BIT(2)
#define MSCR_RX_STATUS_G1_EN            BIT(1)
#define MSCR_RX_STATUS_G0_EN            BIT(0)

//3 ARCR
#define ARCR_1M_IDX                     0
#define ARCR_2ML_IDX                    1
#define ARCR_2MS_IDX                    2
#define ARCR_5_5ML_IDX                  3
#define ARCR_5_5MS_IDX                  4
#define ARCR_11ML_IDX                   5
#define ARCR_11MS_IDX                   6
#define ARCR_6M_IDX                     7
#define ARCR_9M_IDX                     8
#define ARCR_12M_IDX                    9
#define ARCR_18M_IDX                    10
#define ARCR_24M_IDX                    11
#define ARCR_36M_IDX                    12
#define ARCR_48M_IDX                    13
#define ARCR_54M_IDX                    14

#define ARCR_WRITE                      BIT(31)
#define ARCR_RESET                      BIT(30)
#define ARCR_ENTRY_INDEX_OFFSET         24
#define ARCR_CURR_RATE1_INDEX_MASK      BITS(20,23)
#define ARCR_CURR_RATE1_INDEX_OFFSET    20
#define ARCR_RATE1_INDEX_OFFSET         16
#define ARCR_1M                         BIT(ARCR_1M_IDX)
#define ARCR_2ML                        BIT(ARCR_2ML_IDX)
#define ARCR_2MS                        BIT(ARCR_2MS_IDX)
#define ARCR_5_5ML                      BIT(ARCR_5_5ML_IDX)
#define ARCR_5_5MS                      BIT(ARCR_5_5MS_IDX)
#define ARCR_11ML                       BIT(ARCR_11ML_IDX)
#define ARCR_11MS                       BIT(ARCR_11MS_IDX)
#define ARCR_6M                         BIT(ARCR_6M_IDX)
#define ARCR_9M                         BIT(ARCR_9M_IDX)
#define ARCR_12M                        BIT(ARCR_12M_IDX)
#define ARCR_18M                        BIT(ARCR_18M_IDX)
#define ARCR_24M                        BIT(ARCR_24M_IDX)
#define ARCR_36M                        BIT(ARCR_36M_IDX)
#define ARCR_48M                        BIT(ARCR_48M_IDX)
#define ARCR_54M                        BIT(ARCR_54M_IDX)
#define ARCR_SUPPORT_RATES_DEFAULT      BITS(0,14)


//3 TXCR
#define TXCR_AIFS_AC3                   BITS(30,31)
#define TXCR_AIFS_AC2                   BIT(31)
#define TXCR_AIFS_AC1                   BIT(30)
#define TXCR_AIFS_AC0                   0

#define TXCR_NULL_TYPE_QOS              BIT(25)
#define TXCR_NULL_TYPE_NON_QOS          0
#define TXCR_HW_TX_RATE_SEL_BASIC_RATE  BIT(24)
#define TXCR_QOS_NULL_TID               BITS(20, 23)
#define TXCR_BIP_EN                     BIT(19)
#define TXCR_MFP_CCX_MODE               BIT(18)


#define TXCR_QOS_REPORT_CTRL_QSIZE      BIT(17)
#define TXCR_PWRMGT_SET                 BIT(16)
#define TXCR_TS_EXT_MODE_EXTRA          BIT(15)

#define TXCR_AC3_TRIG_ENABLED           BIT(15)
#define TXCR_AC2_TRIG_ENABLED           BIT(14)
#define TXCR_AC1_TRIG_ENABLED           BIT(13)
#define TXCR_AC0_TRIG_ENABLED           BIT(12)
#define TXCR_AC_TRIG_ENABLED_MASK       BITS(12, 15)
#define TXCR_AC_TRIG_FUNCTION_ENABLE    BIT(11)
#define TXCR_RESERVED                   BIT(10)
#define TXCR_IBSS_Q_STOP                BIT(9)
#define TXCR_AC3_DELIVERY_ENABLED       BIT(7)
#define TXCR_AC2_DELIVERY_ENABLED       BIT(6)
#define TXCR_AC1_DELIVERY_ENABLED       BIT(5)
#define TXCR_AC0_DELIVERY_ENABLED       BIT(4)
#define TXCR_AC_DLVR_ENABLED_MASK       BITS(4, 7)
#define TXCR_AUTO_RATE_ENABLE           BIT(3)


//2 TMAC

//3 0x0100 - TCR
#define TCR_TX_RESP_ANT_MODE_0          BIT(25)
#define TCR_TX_RESP_ANT_MODE_1          BIT(26)
#define TCR_TX_RESP_ANT_MODE_LAST       BITS(25,26)

#define TCR_DURATION_CAL                BIT(13)
#define TCR_TXOP_BURST_STOP             BIT(12)

#define TCR_END_PACKET_TYPE_QOSNULL     BIT(6)
#define TCR_END_PACKET_TYPE_NORESP      0

//3 0x0104 - ACTXOPLR0
#define ACTXOPLR0_AC0_LIMIT_OFFSET      0
#define ACTXOPLR0_AC1_LIMIT_OFFSET      16

#define ACTXOPLR0_AC0_LIMIT_MASK        BITS(0,15)
#define ACTXOPLR0_AC1_LIMIT_MASK        BITS(16,31)


//3 0x0108 - ACTXOPLR1
#define ACTXOPLR1_AC2_LIMIT_OFFSET      0
#define ACTXOPLR1_AC3_LIMIT_OFFSET      16

#define ACTXOPLR1_AC2_LIMIT_MASK        BITS(0,15)
#define ACTXOPLR1_AC3_LIMIT_MASK        BITS(16,31)

//3 OFPR
#define OFPR_48_54_FRAME_POWER_MASK     BITS(24,30)
#define OFPR_24_36_FRAME_POWER_MASK     BITS(16,22)
#define OFPR_12_18_FRAME_POWER_MASK     BITS(8,14)
#define OFPR_6_9_FRAME_POWER_MASK       BITS(0,6)

#define OFPR_48_54_FRAME_POWER_STARTBIT 24
#define OFPR_24_36_FRAME_POWER_STARTBIT 16
#define OFPR_12_18_FRAME_POWER_STARTBIT 8
#define OFPR_6_9_FRAME_POWER_STARTBIT 0


//3 CFPR
#define CFPR_FRAME_POWER_MASK           BITS(0,6)
#define CFPR_FRAME_POWER_STARTBIT       0



#define PTA_HW_DISABLE              (0)
#define PTA_HW_ENABLE               (1)
/*BTCER0*/
#define PTA_BTCER0_COEXIST_EN      BIT(0)
#define PTA_BTCER0_WLAN_ACT_POL    BIT(1)
#define PTA_BTCER0_BURST_MODE      BIT(2)
#define PTA_BTCER0_WLAN_ACK        BIT(3)
#define PTA_BTCER0_TX_MODE         BITS(4,5)
#define PTA_BTCER0_RX_MODE         BIT(6)
#define PTA_BTCER0_WLAN_AC0        BIT(8)
#define PTA_BTCER0_WLAN_AC1        BIT(9)
#define PTA_BTCER0_WLAN_AC2        BIT(10)
#define PTA_BTCER0_WLAN_AC3        BIT(11)
#define PTA_BTCER0_WLAN_BCN        BIT(12)
#define PTA_BTCER0_WLAN_AC4        BIT(13)
#define PTA_BTCER0_WLAN_RX         BIT(14)
#define PTA_BTCER0_WLAN_CTRL       BIT(15)
#define PTA_BTCER0_BCN_TIMEOUT_EN  BIT(16)
#define PTA_BTCER0_QCFP_TIMEOUT_EN BIT(17)
#define PTA_BTCER0_SP_TIMEOUT_EN   BIT(18)
#define PTA_BTCER0_REMAIN_TIME     BITS(24,31)
/*BTCER1*/
#define PTA_BTCER1_BT_2ND_SAMPLE_TIME       BITS(0,4)
#define PTA_BTCER1_BT_1ST_SAMPLE_TIME       BITS(5,7)
#define PTA_BTCER1_1ST_SAMPLE_MODE          BITS(8,9)
#define PTA_BTCER1_2ND_SAMPLE_MODE          BITS(10,11)
#define PTA_BTCER1_BT_PRI_MODE              BIT(12)
#define PTA_BTCER1_BT_TR_MODE               BIT(13)
#define PTA_BTCER1_CONCATE_MODE             BIT(14)
#define PTA_BTCER1_T6_PERIOD                BITS(16,20)
#define PTA_BTCER1_SINGLE_ANT               BIT(28)
#define PTA_BTCER1_WIRE_MODE                BITS(29,30)

#define PTA_BTCER1_1_WIRE_MODE              0x00000000
#define PTA_BTCER1_2_WIRE_MODE              BIT(30)
#define PTA_BTCER1_3_WIRE_MODE             (BIT(30) | BIT(29))
#define PTA_BTCER1_4_WIRE_MODE              PTA_BTCER1_3_WIRE_MODE

/*BTCER2*/
#define PTA_BTCER2_QUOTA_LIMIT              BITS(0,15)
#define PTA_BTCER2_FAIRNESS_PERIOD          BITS(16,31)

/*BTCER3*/
#define PTA_BTCER3_SP_TIMEOUT               BITS(0,7)
#define PTA_BTCER3_BCN_QCFP_TIMEOUT         BITS(8,15)
#define PTA_BTCER3_BT_GUARD_ITVAL           BITS(16,23)
#define PTA_BTCER3_QUOTA_CTRL               BIT(24)
#define PTA_BTCER3_QUOTA_EN                 BIT(25)



//4 2006/09/11, mikewu, <todo>BTCER1-3, BRUR0-1, BRDR, BPDR, CDTR, ODTR
#define BPDR_SLOTTIME_MASK              BITS(17,21)
#define BPDR_SLOTTIME_OFFSET            17



//3 RCCR
//4 Long Preamble
#define RCCR_RC_BR_CCK_LONG_1M          0
#define RCCR_RC_BR_CCK_LONG_2M          BIT(24)
#define RCCR_RC_BR_CCK_LONG_5p5M        BIT(25)
#define RCCR_RC_BR_CCK_LONG_11M         BITS(24,25)

//4 Short Preamble
#define RCCR_RC_BR_CCK_SHORT_2M         (BIT(28) | BIT(24))
#define RCCR_RC_BR_CCK_SHORT_5p5M       (BIT(28) | BIT(25))
#define RCCR_RC_BR_CCK_SHORT_11M        (BIT(28) | BITS(24,25))

#define RCCR_RC_BR_OFDM_6M              (BIT(29) | BIT(27) | BITS(24,25))
#define RCCR_RC_BR_OFDM_9M              (BIT(29) | BITS(24,27))
#define RCCR_RC_BR_OFDM_12M             (BIT(29) | BIT(27) | BIT(25))
#define RCCR_RC_BR_OFDM_18M             (BIT(29) | BITS(25,27))
#define RCCR_RC_BR_OFDM_24M             (BIT(29) | BIT(27) | BIT(24))
#define RCCR_RC_BR_OFDM_36M             (BIT(29) | BIT(24) | BITS(26,27))
#define RCCR_RC_BR_OFDM_48M             (BIT(29) | BIT(27))
#define RCCR_RC_BR_OFDM_54M             (BIT(29) | BITS(26,27))
#define RCCR_RC_BR_MASK                 (BITS(24,29))
#define RCCR_RC_BR_OFFSET                24

#define RCCR_BR_MASK                     BITS(16,21)
#define RCCR_BR_CCK_2M                  (BIT(20) | BIT(24))
#define RCCR_BR_CCK_5p5M                (BIT(20) | BIT(25))
#define RCCR_BR_CCK_11M                 (BIT(20) | BITS(24,25))

#define RCCR_BR_OFDM_6M                 (BIT(21) | BIT(19) | BITS(16,17))
#define RCCR_BR_OFDM_9M                 (BIT(21) | BITS(16,19))
#define RCCR_BR_OFDM_12M                (BIT(21) | BIT(19) | BIT(17))
#define RCCR_BR_OFDM_18M                (BIT(21) | BITS(17,19))
#define RCCR_BR_OFDM_16M                (BIT(21) | BIT(19) | BIT(16))
#define RCCR_BR_OFDM_36M                (BIT(21) | BIT(16) | BITS(18,19))
#define RCCR_BR_OFDM_48M                (BIT(21) | BIT(19))
#define RCCR_BR_OFDM_54M                (BIT(21) | BITS(18,19))
#define RCCR_CTS_PROTECTION              BIT(15)
#define RCCR_CTS_MODE_PROPRIETARY        BIT(14)
#define RCCR_RTS_THRESHOLD_MASK          BITS(0,11)
//2 RMAC

//3 RFCR
#define RFCR_RX_SAMEBSSIDPRORESP_CTRL   BIT(28)
#define RFCR_RX_DIFFBSSIDPRORESP_CTRL   BIT(27)
#define RFCR_RX_SAMEBSSIDATIM_CTRL      BIT(26)
#define RFCR_RX_DIFFBSSIDATIM_CTRL      BIT(25)
#define RFCR_RX_SAMEBSSIDNULL_CTRL      BIT(24)
#define RFCR_RX_DIFFBSSIDNULL_CTRL      BIT(23)
#define RFCR_RX_SAMEBSSIDBCN_CTRL       BIT(22)
#define RFCR_RX_DIFFBSSIDBCN_CTRL       BIT(21)
#define RFCR_RX_PROREQ_CTRL             BIT(20)
#define RFCR_DROP_VERSIONNOT0_CTRL      BIT(19)
#define RFCR_RX_NOACK_CTRL              BIT(18)
#define RFCR_RX_MGMT_FRAME_CTRL         BIT(17)
#define RFCR_RX_DATA_FRAME_CTRL         BIT(16)
#define RFCR_DROP_DIFFBSSIDMGT_CTRL     BIT(15)
#define RFCR_DROP_ADDR3OWNSA_CTRL       BIT(14)
#define RFCR_DROP_DIFFBSSIDA3_CTRL      BIT(13)
#define RFCR_DROP_DIFFBSSIDA2_CTRL      BIT(12)
#define RFCR_RX_BCFRAME_CTRL            BIT(11)
#define RFCR_MCTABLE_NOCHK_CTRL         BIT(10)
#define RFCR_RX_MCFRAME_CTRL            BIT(9)
#define RFCR_RX_PROMISCUOUSFRAME_CTRL   BIT(1)
#define RFCR_DROP_FCSERRORFRAME_CTRL    BIT(0)

//3 SSCR
#define SSCR_MIN_RCPI               BITS(24,31)
#define SSCR_EXT_SCAN_TIME          BITS(16,23)
#define SSCR_MIN_SCAN_TIME          BITS(8,15)
#define SSCR_EXT_TIME_CTRL          BIT(5)
#define SSCR_SSID_START             BIT(4)
#define SSCR_SSID_NUMBER            BITS(0,3)


//3 MUAR
#define MUAR1_SEARCH_IDX_MASK       BITS(24,29)
#define MUAR1_READ                  0
#define MUAR1_WRITE                 BIT(17)
#define MUAR1_ACCESS_START          BIT(16)
#define MUAR1_ADDR_INDEX_OFFSET     24


//3 NSUR
#define NSUR_NAV_UPDATE             BIT(31)
#define NSUR_NAV_UPDATE_VALUE_MASK  BITS(0,25)  /* NAV updated value */

//3 ALCR
#define ALCR_ALC_CALCULATION_EN     BIT(31)
#define ALCR_ALC_TRIGGER            BIT(30)
#define ALCR_ALC_BUSY               BIT(28)
#define ALCR_ALC_AR_FACTOR_MASK     BITS(24, 25)
#define ALCR_AR_PARM_1_OF_32        0
#define ALCR_AR_PARM_1_OF_16        BIT(24)
#define ALCR_AR_PARM_1_OF_4         BIT(25)
#define ALCR_AR_PARM_1_OF_1         BITS(24,25)

#define ALCR_ALC_MIN_THRESHOLD      BITS(18,23)
#define ALCR_ALC_MAX_THRESHOLD      BITS(12,17)
#define ALCR_ALC_CAL_VALUE_MASK     BITS(6,11)
#define ALCR_ALC_RAW_VALUE_MASK     BITS(0,5)


#define ALCR_SET_MOV_AVE_PARA(_movAvePara) \
                    (((_movAvePara) << 0) & ALCR_ALC_AR_FACTOR_MASK)
#define ALCR_SET_MIN_THRESHOLD(_alcMinThreshold) \
                (((_alcMinThreshold) << 18) & ALCR_ALC_MIN_THRESHOLD)
#define ALCR_SET_MAX_THRESHOLD(_alcMaxThreshold) \
                (((_alcMaxThreshold) << 12) & ALCR_ALC_MAX_THRESHOLD)

#define ALCR_SET_ALC_INIT_VALUE(_alcInitVal) \
                (((_alcInitVal) << 6) & ALCR_ALC_CAL_VALUE_MASK)

#define ALCR_GET_ALC_CAL_VALUE(_alcr) \
                ((ALC_VAL)(((_alcr) & ALCR_ALC_CAL_VALUE_MASK)>>6))
#define ALCR_GET_ALC_RAW_VALUE(_alcr) \
                    ((ALC_VAL)(((_alcr) & ALCR_ALC_RAW_VALUE_MASK)>>0))

//2 LP
//3 LTTR

//3 PIR
#define PIR_BC_PATTERN_SRCH_EN          BIT(22)
#define PIR_MC_PATTERN_SRCH_EN          BIT(21)
#define PIR_UC_PATTERN_SRCH_EN          BIT(20)
#define PIR_BC_MATCHING_OPERATION       BIT(18)
#define PIR_MC_MATCHING_OPERATION       BIT(17)
#define PIR_UC_MATCHING_OPERATION       BIT(16)
#define PIR_IPV6_FRAME_MATCH_CTRL       BIT(12)
#define PIR_PATTERN_WRITE_MODE          BIT(7)
#define PIR_PATTERN_INDEX               BITS(0, 4)

//3 PMR
#define PIR_PATTERN_BC_CHECK            BIT(24)
#define PIR_PATTERN_MC_CHECK            BIT(23)
#define PIR_PATTERN_UC_CHECK            BIT(22)
#define PIR_IPV4_IP_CTRL                BIT(21)
#define PIR_IPV6_ICMP_CTRL              BIT(20)
#define PIR_GARP_IP_EQUAL_CTRL          BIT(19)
#define PIR_ARP_PATTERN_CTRL            BIT(18)
#define PIR_AND_OR_OPERAND              BIT(17)
#define PIR_NOT_OPERAND                 BIT(16)
#define PIR_PATTERN_BYTE_MASK           BITS(8, 15)
#define PIR_PATTERN_OFFSET              BITS(0, 7)

//3 PPR

//3 LPWSAR

//3 MPTCR
#define MPTCR_BMC_TIMEOUT_EN            BIT(31)
#define MPTCR_MORE_TRIG_EN              BIT(30)
#define MPTCR_TX_DONE_SLEEP_CTRL        BIT(29)
#define MPTCR_TX_PSPOLL_TIMEOUT_EN      BIT(28)
#define MPTCR_BCN_CONTENT_CHK_EN        BIT(27)
#define MPTCR_APSD_TIMEOUT_EN           BIT(25)
#define MPTCR_BCN_TIMEOUT_EN            BIT(24)
#define MPTCR_SCAN_EN                   BIT(23)
#define MPTCR_SSID_SRCH                 BIT(22)
#define MPTCR_CFPPERIODTIMEREN          BIT(21)
#define MPTCR_T3_TIMER_EN               BIT(20)
#define MPTCR_T2_TIMER_EN               BIT(19)
#define MPTCR_T1_PERIOD_TIMER_EN        BIT(18)
#define MPTCR_T0_PERIOD_TIMER_EN        BIT(17)
#define MPTCR_TBTT_PERIOD_TIMER_EN      BIT(16)
#define MPTCR_BCN_UC_EN                 BIT(15)
#define MPTCR_TX_NULL_EN                BIT(14)
#define MPTCR_BCN_BMC_EN                BIT(13)
#define MPTCR_BCN_PARSE_TIM_EN          BIT(12)
#define MPTCR_Q_CFP_NO_DATA_EN          BIT(10)
#define MPTCR_HCCA_TXOP_SHORT_EN        BIT(9)
#define MPTCR_PREDTIM_TIMEUP_EN         BIT(8)
#define MPTCR_PRETBTT_TIMEUP_EN         BIT(7)
#define MPTCR_TBTT_TIMEUP_EN            BIT(6)
#define MPTCR_GPIO2_TRIGGER             BIT(5)
#define MPTCR_GPIO1_TRIGGER             BIT(4)
#define MPTCR_GPIO0_TRIGGER             BIT(3)
#define MPTCR_RX_BMC_MGT_EN             BIT(2)
#define MPTCR_PREDTIM_TRIG_EN           BIT(1)
#define MPTCR_PRETBTT_TRIG_EN           BIT(0)

#define MPTCR_PRETBTT_PREDTIM_TRIG_EN   BITS(0, 1)


//3 ACWR
#define ACWR_ATIM_WINDOW_MASK       BITS(0,15)

//3 BSSSAR
#define BSSSAR_CHNL_INDEX_MASK      BITS(24,31)
#define BSSSAR_BAND_INDEX_MASK      BITS(16,23)
#define BSSSAR_CHNL_INDEX_OFFSET    24
#define BSSSAR_BAND_INDEX_OFFSET    16

//3 TCR, 0x019C
#define TCR_BCN_FRAME_TYPE_PSPOLL       0
#define TCR_BCN_FRAME_TYPE_QoS_TRIG_FRM BIT(20)
#define TCR_TIMER_SEL_T0                0
#define TCR_TIMER_SEL_T1                BIT(0)

//4 T2, T3 can be configured period only
#define TCR_TIMER_SEL_T2                BIT(1)
#define TCR_TIMER_SEL_T3                BITS(0,1)
#define TCR_TIME_VALUE_CTRL_PERIOD      BIT(2)
#define TCR_TIME_VALUE_CTRL_START_TIME  0

#define TCR_T0_CTRL_TS                  BIT(5)
#define TCR_T0_CTRL_NO_TRIG_FRAME       BITS(3,4)
#define TCR_T0_CTRL_PSPOLL              BIT(4)
#define TCR_T0_CTRL_QOS_TRIG            BIT(3)
#define TCR_T0_CTRL_TSF_TIMER           0


#define TCR_T1_CTRL_NO_TRIG_FRAME       BITS(6,7)
#define TCR_T1_CTRL_PSPOLL              BIT(7)
#define TCR_T1_CTRL_QOS_TRIG            BIT(6)
#define TCR_T1_CTRL_TSF_TIMER           0

#define TCR_T2_CTRL_PSPOLL              BIT(9)
#define TCR_T2_CTRL_QOS_TRIG            BIT(8)
#define TCR_T2_CTRL_FREERUN_TIMER       0

#define TCR_T3_CTRL_PSPOLL              BIT(11)
#define TCR_T3_CTRL_QOS_TRIG            BIT(10)
#define TCR_T3_CTRL_FREERUN_TIMER       0

#define TCR_T2_COUNT_UNIT_1US           0
#define TCR_T2_COUNT_UNIT_1TU           BIT(16)

#define TCR_T2_MODE_AUTO_REPEAT         BIT(17)
#define TCR_T2_MODE_ONE_SHOT            0

#define TCR_T3_COUNT_UNIT_1US           0
#define TCR_T3_COUNT_UNIT_1TU           BIT(18)

#define TCR_T3_MODE_AUTO_REPEAT         BIT(19)
#define TCR_T3_MODE_ONE_SHOT            0


//3 TDR, 0x01A0
#define TDR_TIMER_SEL_MASK              BITS(30,31)
#define TDR_TIMER_SEL_T0                (0 << 30)
#define TDR_TIMER_SEL_T1                (1 << 30)
#define TDR_TIMER_SEL_T2                (2 << 30)
#define TDR_TIMER_SEL_T3                (3 << 30)
#define TDR_TIME_VALUE_CTRL_PERIOD      BIT(29)
#define TDR_TIMEUP_ENABLE               BIT(27)
#define TDR_TIMER_CTRL_MASK             BITS(24,26)
#define TDR_TIMER_CTRL_TSF              (0 << 24)
#define TDR_TIMER_CTRL_FREE_RUN         (0 << 24)
#define TDR_TIMER_CTRL_AUTO_TRIGGER     (1 << 24)
#define TDR_TIMER_CTRL_AUTO_PS_POLL     (2 << 24)
#define TDR_TIMER_CTRL_SAPSD            (3 << 24)
#define TDR_TIMER_CTRL_TS               (4 << 24)
#define TDR_TIMER_MODE_AUTO_REPEAT      BIT(23)
#define TDR_TIMER_COUNT_UNIT_TU         BIT(22)
#define TDR_TIME_VALUE_MASK             BITS(0,21)
#define TDR_TIME_VALUE_MAX_T2_T3        BITS(0,15)
#define TDR_TIME_VALUE_MAX_T0_T1        BITS(0,21)

//3 ATCR, 0x01A4
#define ATCR_PSPOLL_END_SP_EN           BIT(31)
#define ATCR_TRIGGER_END_SP_EN          BIT(30)
#define ATCR_PSPOLL_NEW_SP_EN           BIT(28)
#define ATCR_TRIGGER_NEW_SP_EN          BIT(27)
#define ATCR_MNGT_PSPOLL_EN             BIT(26)
#define ATCR_TX_NULL_INTERVAL           BITS(16,25)
#define ATCR_TX_NULL_RESET_CTRL         BIT(15)
#define ATCR_BCN_POLL_QOS_NULL          BIT(14)
#define ATCR_TRIGGER_THRESHOLD          BITS(8,13)
#define ATCR_TIMEOUT_COUNT_LIMIT        BITS(4,7)
#define ATCR_BCN_TIMEOUT_COUNT_LIMIT    BITS(0,3)



//3 TSTR
#define TSTR_PRETBTT_INTERVAL_MASK      BITS(24,31)
#define TSTR_GUARD_INTERVAL_MASK        BITS(16,23)
#define TSTR_TBTTSTARTTIME_MASK         BITS(0,15)


//3 TTPCR
#define TTPCR_TBTT_CAL_ENABLE           BIT(31)
#define TTPCR_DTIM_WAKE_PERIOD_MASK     BITS(28,30)
#define TTPCR_TBTT_WAKE_PERIOD_MASK     BITS(24,27)
#define TTPCR_DTIM_PERIOD_MASK          BITS(16,23)
#define TTPCR_BEACON_PERIOD_MASK        BITS(0,15)


//3 CTPCR

//3 LPCR
#define LPCR_LP_STABLE_TIME_MASK        BITS(24, 30)
#define LPCR_TSF_DRIFT_INTR_EN          BIT(23)
#define LPCR_TX_LIFE_TIME_MASK          BITS(12, 22)
#define LPCR_AID_MASK                   BITS(0, 11)


//3 LPICR

//3 LPIDR

//3 UTTR0

//3 UTTR1

//3 TTAR


#define TTAR_TSF_TSF_DRIFT_WINDOW       BITS(16,23)
#define TTAR_TSF_TIMER_VALUE_CHANGE     BIT(0)
#define TTAR_DISABLE_HWTSF_UPDATE       BIT(1)

//3 LPOSAR

//3 TR
#define TR_BCN_MAX_TIME_LIMIT_MASK          BITS(24,31)
#define TR_BCN_MAX_TIME_LIMIT_VALID         BIT(23)
#define TR_BCN_MIN_TIME_LIMIT_VALID         BIT(22)
#define TR_BCN_MIN_TIME_LIMIT_MASK          BITS(16,21)
#define TR_BCN_MIN_MAX_TIME_LIMIT_MASK      BITS(16,31)

#define TR_MAX_TIME_LIMIT_MASK              BITS(8,15)
#define TR_MAX_TIME_LIMIT_VALID             BIT(7)
#define TR_MIN_TIME_LIMIT_VALID             BIT(6)
#define TR_MIN_TIME_LIMIT_MASK              BITS(0,5)
#define TR_MIN_MAX_TIME_LIMIT_MASK          BITS(0,15)


//3 MTR0, MTR1
#define MTR_ACQ_ADMIT_TIME(ac, value)   (((value) & BITS(0,15)) << ((ac & 1) << 4))
#define MTR_ACQ_USED_TIME(ac, regVal)   (((regVal) >> ((ac & 1) << 4)) & BITS(0,15))

#define MTR0_AC3_ADMIT_TIME_MASK             BITS(16,31)
#define MTR0_AC2_ADMIT_TIME_MASK             BITS(0,15)
#define MTR1_AC1_ADMIT_TIME_MASK             BITS(16,31)
#define MTR1_AC0_ADMIT_TIME_MASK             BITS(0,15)

//3 MTCR0, MTCR1

#define MTCR_MEDIA_TIME_SHIFT               7
#define MTCR_MEDIA_TIME_MASK                0x1UL
#define MTCR_MEDIA_TIME_UNIT(x)             (((x) & MTCR_MEDIA_TIME_MASK) << MTCR_MEDIA_TIME_SHIFT)

#define MTCR_1_SEC                          0
#define MTCR_64_USEC                        1

#define MTCR_AVERAGE_PERIOD_MASK            BITS(0,6)
#define MTCR_ACQ_AVERAGE_PERIOD(ac, value, unit)    (((((value) & MTCR_AVERAGE_PERIOD_MASK) | MTCR_MEDIA_TIME_UNIT(unit)) << ((ac & 1) << 3)) << 16)
#define MTCR_ACQ_ADMIT_TIME_ENABLE(ac)      BIT(ac & 1)


#define MTCR0_AC3_AVERAGE_PERIOD_UNIT_64_US BIT(31)
#define MTCR0_AC3_AVERAGE_PERIOD_MASK       BITS(24,30)
#define MTCR0_AC2_AVERAGE_PERIOD_UNIT_64_US BIT(23)
#define MTCR0_AC2_AVERAGE_PERIOD_MASK       BITS(16,22)
#define MTCR0_AC3_ADMIT_TIME_EN             BIT(1)
#define MTCR0_AC2_ADMIT_TIME_EN             BIT(0)

#define MTCR1_AC1_AVERAGE_PERIOD_UNIT_64_US BIT(31)
#define MTCR1_AC1_AVERAGE_PERIOD_MASK       BITS(24,30)
#define MTCR1_AC0_AVERAGE_PERIOD_UNIT_64_US BIT(23)
#define MTCR1_AC0_AVERAGE_PERIOD_MASK       BITS(16,22)
#define MTCR1_AC1_ADMIT_TIME_EN             BIT(1)
#define MTCR1_AC0_ADMIT_TIME_EN             BIT(0)

//3 SPCR
#define SPCR_NULL_TIMEOUT_LIMIT_MASK        BITS(24,31)
#define SPCR_NULL_MAX_TIME_LIMIT_VALID      BIT(23)
#define SPCR_NULL_MIN_TIME_LIMIT_VALID      BIT(22)
#define SPCR_NULL_MIN_TIME_LIMIT_MASK       BITS(16,21)
#define SPCR_NULL_MIN_MAX_TIME_LIMIT_MASK   BITS(16,31)

#define SPCR_TO_COUNTER_RESET_CTRL          BIT(5)
#define SPCR_BEACON_SP_INVALID_MASK         BIT(4)
#define SPCR_BMC_SP_INVALID_MASK            BIT(3)
#define SPCR_QOS_CFPOLL_SP_INVALID_MASK     BIT(2)
#define SPCR_PSPOLL_SP_INVALID_MASK         BIT(1)
#define SPCR_TRIGGER_SP_INVALID_MASK        BIT(0)
#define SPCR_ALL_SP_INVALID_MASK            BITS(0, 4)

//3 BEIR
#define BEIR_BCN_LOST_THRESHOLD             BITS(16,19)
#define BEIR_BEI_CAL_EN                     BIT(8)
#define BEIR_BCN_EARLIER_INTERVAL_MASK      BITS(0,7)


//2 BB

//4 CR2, BBISR
#define BBISR_RADAR_DETECTION           BIT(1)
#define BBISR_TYPE1_DETECTION           BIT(2)
#define BBISR_TYPE2_DETECTION           BIT(3)
#define BBISR_TYPE3_DETECTION           BIT(4)
#define BBISR_TYPE4_DETECTION           BIT(5)

#define BBISR_RADAR_DETECTION_EN        BIT(9)
#define BBISR_TYPE1_DETECTION_EN        BIT(10)
#define BBISR_TYPE2_DETECTION_EN        BIT(11)
#define BBISR_TYPE3_DETECTION_EN        BIT(12)
#define BBISR_TYPE4_DETECTION_EN        BIT(13)

//4 CR1, BCR
#define BCR_BAND_5G_EN                  BIT(1)
#define BCR_RX_OFDM_DISABLE             BIT(2)
#define BCR_RX_CCK_DISABLE              BIT(3)
#define BCR_RX_ANT_SEL_FIX_1            BITS(4,5)
#define BCR_RX_ANT_SEL_MASK             BITS(4,5)
#define BCR_RX_ANT_SEL_FIX_0            BIT(5)
#define BCR_RX_ANT_SEL_MPDU_BASED       BIT(4)
#define BCR_RX_ANT_SEL_AGC_BASED        0

#define BCR_CCA_METHOD_ED_OR_EARLYCS    BITS(9,10)
#define BCR_CCA_METHOD_ED_OR_CS         BIT(8) | BIT(10)
#define BCR_CCA_METHOD_ED_AND_EARLYCS   BIT(10)
#define BCR_CCA_METHOD_ED_AND_CS        BITS(8,9)
#define BCR_CCA_METHOD_EARLYCS          BIT(9)
#define BCR_CCA_METHOD_ED               BIT(8)
#define BCR_CCA_METHOD_CS               0
#define BCR_CCA_METHOD_MASK             BITS(8,10)

//4 CR65, BCCMR1
#define BCR_BBCMR1_TXF_JP_CH            BIT(6)

//2 RF
#define RFCR4_XO_TRIM_MASK              BITS(21, 27)
#define RFCR4_XO_TRIM_OFFSET            21

#define RFCR4_EREFS                     BITS(0, 2)

#endif /* _MT592X_REG_H */

