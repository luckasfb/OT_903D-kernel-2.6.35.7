





#ifndef _BB_MT5921_H
#define _BB_MT5921_H







/* MT5921 Baseband control register (BBCR) definition */

/* CR2 bit definitions */
#define CCA_METHOD_CS                   0   /* CS (15us) */
#define CCA_METHOD_ED                   1   /* ED (4us) */
#define CCA_METHOD_EARLY_CS             2   /* early_CS (4us) */
#define CCA_METHOD_ED_AND_CS            3   /* ED and CS (15us) */
#define CCA_METHOD_ED_AND_EARLY_CS      4   /* ED and early_CS (4us) */
#define CCA_METHOD_ED_OR_CS             5   /* ED or CS (4us) */
#define CCA_METHOD_ED_OR_EARLY_CS       6   /* ED or early_CS (4us) */

#define CR2_WATCHDOG_EN                 BIT(4)
#define CR2_CCA_METHOD                  BITS(1, 3)
#define CR2_CCA_METHOD_CS               (CCA_METHOD_CS << 1)
#define CR2_CCA_METHOD_ED               (CCA_METHOD_ED << 1)
#define CR2_CCA_METHOD_EARLY_CS         (CCA_METHOD_EARLY_CS << 1)
#define CR2_CCA_METHOD_ED_AND_CS        (CCA_METHOD_ED_AND_CS << 1)
#define CR2_CCA_METHOD_ED_AND_EARLY_CS  (CCA_METHOD_ED_AND_EARLY_CS << 1)
#define CR2_CCA_METHOD_ED_OR_CS         (CCA_METHOD_ED_OR_CS << 1)
#define CR2_CCA_METHOD_ED_OR_EARLY_CS   (CCA_METHOD_ED_OR_EARLY_CS << 1)
#define CR2_RADAR_DETECTION_EN          BIT(0)

/* CR3 bit definitions */
#define CR3_OFDM_MDRDY_CTRL             BIT(7)
#define CR3_CCK_MDRDY_CTRL              BIT(6)
#define CR3_RX_ANT_SELECTION_MODE       BITS(4,5)
#define CR3_RX_ANT_DIVERSITY_AGC        (RX_ANT_MODE_DIVERSITY_AGC << 4)
#define CR3_RX_ANT_DIVERSITY_MPDU       (RX_ANT_MODE_DIVERSITY_MPDU << 4)
#define CR3_RX_ANT_FIXED_0              (RX_ANT_MODE_FIXED_0 << 4)
#define CR3_RX_ANT_FIXED_1              (RX_ANT_MODE_FIXED_1 << 4)
#define CR3_RX_CCK_DIS                  BIT(3)
#define CR3_RX_OFDM_DIS                 BIT(2)
#define CR3_BAND_5G_EN                  BIT(1)
#define CR3_OFDM_ENCODER_INIT_STATE     BIT(0)

/* CR4 bit definitions */
#define CR4_RADAR_DETECTION             BIT(7)      /* radar detection */
#define CR4_BBP_RX_DEADLOCK             BIT(6)      /* BBP Rx deadlock */

/* CR29 bit definitions */
#define CR29_RX_ANT                     BIT(7)
#define CR29_LNA_STATUS                 BITS(5,6)
#define CR29_LNA_LOW                    BITS(5)
#define CR29_LNA_MID                    BIT(6)
#define CR29_LNA_HIGH                   BIT(5,6)
#define CR29_VGA_GAIN                   BITS(0,4)

/* CR97 bit definitions */
#define CR97_COUNTER_EN                 BIT(7)
#define CR97_COUNTER_RST                BIT(6)

/* CR24 bit definitions */
#define CR24_RX_MODULATION_TYPE         BIT(7)
#define CR24_RX_OFDM                    BIT(7)
#define CR24_RX_CCK                     0x00
#define CR24_RX_PREAMBLE_TYPE           BIT(6)
#define CR24_RX_SHORT_PREAMBLE          BIT(6)
#define CR24_RX_LONG_PREAMBLE           0x00
#define CR24_RX_RATE                    BITS(2,5)
#define CR24_RX_RATE_1M                 (RATE_CODE_1M << 2)
#define CR24_RX_RATE_2M                 (RATE_CODE_2M << 2)
#define CR24_RX_RATE_5_5M               (RATE_CODE_5_5M << 2)
#define CR24_RX_RATE_11M                (RATE_CODE_11M << 2)
#define CR24_RX_RATE_6M                 (RATE_CODE_6M << 2)
#define CR24_RX_RATE_9M                 (RATE_CODE_9M << 2)
#define CR24_RX_RATE_12M                (RATE_CODE_12M << 2)
#define CR24_RX_RATE_18M                (RATE_CODE_18M << 2)
#define CR24_RX_RATE_24M                (RATE_CODE_24M << 2)
#define CR24_RX_RATE_36M                (RATE_CODE_36M << 2)
#define CR24_RX_RATE_48M                (RATE_CODE_48M << 2)
#define CR24_RX_RATE_54M                (RATE_CODE_54M << 2)


#define BBCR_CR0                        0
#define BBCR_CR1                        1
#define BBCR_CR2                        2
#define BBCR_CR3                        3
#define BBCR_CR4                        4
#define BBCR_CR5                        5
#define BBCR_CR6                        6
#define BBCR_CR7                        7
#define BBCR_CR8                        8
#define BBCR_CR9                        9
#define BBCR_CR10                       10
#define BBCR_CR11                       11
#define BBCR_CR12                       12
#define BBCR_CR13                       13
#define BBCR_CR14                       14
#define BBCR_CR15                       15
#define BBCR_CR16                       16
#define BBCR_CR17                       17
#define BBCR_CR18                       18
#define BBCR_CR19                       19
#define BBCR_CR20                       20
#define BBCR_CR21                       21
#define BBCR_CR22                       22
#define BBCR_CR23                       23
#define BBCR_CR24                       24
#define BBCR_CR25                       25
#define BBCR_CR26                       26
#define BBCR_CR27                       27
#define BBCR_CR28                       28
#define BBCR_CR29                       29
#define BBCR_CR30                       30
#define BBCR_CR31                       31
#define BBCR_CR32                       32
#define BBCR_CR33                       33
#define BBCR_CR34                       34
#define BBCR_CR35                       35
#define BBCR_CR36                       36
#define BBCR_CR37                       37
#define BBCR_CR38                       38
#define BBCR_CR39                       39
#define BBCR_CR45                       45
#define BBCR_CR46                       46
#define BBCR_CR65                       65
#define BBCR_CR80                       80
#define BBCR_CR81                       81
#define BBCR_CR82                       82
#define BBCR_CR83                       83
#define BBCR_CR84                       84
#define BBCR_CR85                       85
#define BBCR_CR86                       86
#define BBCR_CR87                       87
#define BBCR_CR88                       88
#define BBCR_CR89                       89
#define BBCR_CR90                       90
#define BBCR_CR91                       91
#define BBCR_CR92                       92
#define BBCR_CR93                       93
#define BBCR_CR94                       94
#define BBCR_CR95                       95
#define BBCR_CR96                       96
#define BBCR_CR97                       97
#define BBCR_CR98                       98
#define BBCR_CR99                       99
#define BBCR_CR100                      100
#define BBCR_CR101                      101
#define BBCR_CR102                      102
#define BBCR_CR103                      103
#define BBCR_CR104                      104
#define BBCR_CR105                      105
#define BBCR_CR106                      106
#define BBCR_CR107                      107
#define BBCR_CR108                      108
#define BBCR_CR109                      109
#define BBCR_CR110                      110
#define BBCR_CR111                      111
#define BBCR_CR112                      112
#define BBCR_CR113                      113
#define BBCR_CR114                      114
#define BBCR_CR115                      115
#define BBCR_CR116                      116
#define BBCR_CR117                      117
#define BBCR_CR118                      118
#define BBCR_CR119                      119
#define BBCR_CR120                      120
#define BBCR_CR121                      121
#define BBCR_CR122                      122
#define BBCR_CR123                      123
#define BBCR_CR124                      124
#define BBCR_CR125                      125
#define BBCR_CR126                      126
#define BBCR_CR127                      127
#define BBCR_CR128                      128
#define BBCR_CR129                      129


/* Write MT5921 baseband */
#define HAL_BBCR_WR(prAdapter, u4Offset, u4Value) \
    HAL_MCR_WR(prAdapter, 0x200+u4Offset*4, u4Value)

/* Read MT5921 baseband */
#define HAL_BBCR_RD(prAdapter, u4Offset, pu4Value) \
    HAL_MCR_RD(prAdapter, 0x200+u4Offset*4, pu4Value)


VOID
halBBInit (
    IN P_ADAPTER_T prAdapter
    );

VOID
halBBChangeBand (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_BAND_T eBand
    );

VOID
halBBAdoptTempChange (
    IN P_ADAPTER_T          prAdapter,
    IN ENUM_THERMO_STATE_T  rState
    );
VOID
halBBEnableFixedRxGain (
    IN P_ADAPTER_T          prAdapter,
    IN BOOL                 fgEnableFixedRxGain
    );

#endif /* _BB_MT5921_H */

