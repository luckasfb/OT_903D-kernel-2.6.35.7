

#ifndef _CFG_AUDIO_FILE_H
#define _CFG_AUDIO_FILE_H

#define CUSTOM_VOLUME_STEP (7)
#define NB_FIR_NUM (45)
#define NB_FIR_INDEX_NUM   (6)
#define SPEECH_COMMON_NUM (12)
#define SPEECH_PARA_MODE_NUM     (8)
#define SPEECH_PARA_NUM                (16)
#define AUDIO_EQ_PARAM_NUM         (8)

#define WB_FIR_NUM (90)
#define WB_FIR_INDEX_NUM   (6)

/* audio nvram structure definition*/
typedef enum
{
    VOL_NORMAL   = 0 ,
    VOL_HEADSET      ,
    VOL_HANDFREE     ,
    MAX_VOL_CATE
} volume_category_enum;

typedef enum
{
    VOL_TYPE_TON  = 0 ,
    VOL_TYPE_KEY      ,
    VOL_TYPE_MIC      ,
    VOL_TYPE_FMR      ,
    VOL_TYPE_SPH      ,
    VOL_TYPE_SID	    ,
    VOL_TYPE_MEDIA    ,
    MAX_VOL_TYPE
} volume_type_enum;

#define     NUM_ABF_PARAM 44
#define     NUM_ABFWB_PARAM 76

typedef struct _AUDIO_CUSTOM_EXTRA_PARAM_STRUCT
{
   /* ABF parameters */
   unsigned short ABF_para[NUM_ABF_PARAM + NUM_ABFWB_PARAM];    //with WB;
} AUDIO_CUSTOM_EXTRA_PARAM_STRUCT;

#define CFG_FILE_SPEECH_DUAL_MIC_SIZE    sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT)
#define CFG_FILE_SPEECH_DUAL_MIC_TOTAL   1

typedef struct _AUDIO_CUSTOM_PARAM_STRUCT
{
    /* volume setting */
    unsigned char volume[MAX_VOL_CATE][MAX_VOL_TYPE];
    /* speech enhancement */
    unsigned short speech_common_para[SPEECH_COMMON_NUM];
    unsigned short speech_mode_para[SPEECH_PARA_MODE_NUM][SPEECH_PARA_NUM];
    unsigned short speech_volume_para[4];//in the feature, should extend to [MAX_VOL_CATE][MAX_VOL_TYPE][4]
    /* debug info */
    unsigned short debug_info[16];
   /* speech input FIR */
   short          sph_in_fir[NB_FIR_INDEX_NUM][NB_FIR_NUM];
   /* speech output FIR */
   short          sph_out_fir[NB_FIR_INDEX_NUM][NB_FIR_NUM];
   /* digital gain of DL speech */
   unsigned short Digi_DL_Speech;
   /* digital gain of uplink speech */
   unsigned short Digi_Microphone;
   /* FM record volume*/
   unsigned short FM_Record_Volume;
   /* user mode : normal mode, earphone mode, loud speaker mode */
   unsigned short Bluetooth_Sync_Type;
   unsigned short Bluetooth_Sync_Length;
   unsigned short bt_pcm_in_vol;
   unsigned short bt_pcm_out_vol;
   unsigned short user_mode;
   /* auto VM record setting */
   unsigned short bSupportVM;
   unsigned short bAutoVM;
   // mic bias
   unsigned short uMicbiasVolt;

} AUDIO_CUSTOM_PARAM_STRUCT;

#define CFG_FILE_SPEECH_REC_SIZE        sizeof(AUDIO_CUSTOM_PARAM_STRUCT)
#define CFG_FILE_SPEECH_REC_TOTAL   1

typedef struct _AUDIO_CUSTOM_WB_A2M_PARAM_STRUCT_
{
   /* WB speech enhancement */
   unsigned short speech_mode_wb_para[SPEECH_PARA_MODE_NUM][SPEECH_PARA_NUM];
   /* WB speech input/output FIR */
   short          sph_wb_fir[WB_FIR_INDEX_NUM][WB_FIR_NUM];
   /* in_out flag */
   short          input_out_fir_flag; // 0: input, 1: output
} AUDIO_CUSTOM_WB_A2M_PARAM_STRUCT;

typedef struct _AUDIO_CUSTOM_WB_PARAM_STRUCT
{
   /* WB speech enhancement */
   unsigned short speech_mode_wb_para[SPEECH_PARA_MODE_NUM][SPEECH_PARA_NUM];
   /* WB speech input FIR */
   short          sph_wb_in_fir[WB_FIR_INDEX_NUM][WB_FIR_NUM];
   /* WB speech output FIR */
   short          sph_wb_out_fir[WB_FIR_INDEX_NUM][WB_FIR_NUM];
} AUDIO_CUSTOM_WB_PARAM_STRUCT;

#define CFG_FILE_WB_SPEECH_REC_SIZE        sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT)
#define CFG_FILE_WB_SPEECH_REC_TOTAL   1

typedef struct _AUDIO_ACF_CUSTOM_PARAM_STRUCT
{
   /* Compensation Filter HSF coeffs       */
   /* BesLoudness also uses this coeffs    */
   unsigned int bes_loudness_hsf_coeff[9][4];

   /* Compensation Filter BPF coeffs       */
   unsigned int bes_loudness_bpf_coeff[4][6][3];
   unsigned int bes_loudness_DRC_Forget_Table[9][2];
   unsigned int bes_loudness_WS_Gain_Max;
   unsigned int bes_loudness_WS_Gain_Min;
   unsigned int bes_loudness_Filter_First;
   char bes_loudness_Gain_Map_In[5];
   char bes_loudness_Gain_Map_Out[5];

} AUDIO_ACF_CUSTOM_PARAM_STRUCT;

#define CFG_FILE_AUDIO_COMPFLT_REC_SIZE        sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT)
#define CFG_FILE_AUDIO_COMPFLT_REC_TOTAL   1

typedef struct _AUDIO_EFFECT_CUSTOM_PARAM_STRUCT
{
    //surround parameters
    int bsrd_level;
    unsigned int Distance1;
    unsigned int Distance2;
    int bsrd_band_select;

    //bass parameters
    unsigned int bass_CutoffFreq;
    int bass_IsVB;

    //EQ parameters
    short Normal_Gain_dB_level[AUDIO_EQ_PARAM_NUM];
    short Dance_Gain_dB_level[AUDIO_EQ_PARAM_NUM];
    short Bass_Gain_dB_level[AUDIO_EQ_PARAM_NUM];
    short Classical_Gain_dB_level[AUDIO_EQ_PARAM_NUM];
    short Treble_Gain_dB_level[AUDIO_EQ_PARAM_NUM];
    short Party_Gain_dB_level[AUDIO_EQ_PARAM_NUM];
    short Pop_Gain_dB_level[AUDIO_EQ_PARAM_NUM];
    short Rock_Gain_dB_level[AUDIO_EQ_PARAM_NUM];

    //loudness mode
    int LoudEnhancemode;

    // time stretch
    int Time_TD_FD;
    int Time_TS_Ratio;

} AUDIO_EFFECT_CUSTOM_PARAM_STRUCT;

#define CFG_FILE_AUDIO_EFFECT_REC_SIZE        sizeof(AUDIO_EFFECT_CUSTOM_PARAM_STRUCT)
#define CFG_FILE_AUDIO_EFFECT_REC_TOTAL   1

typedef struct _AUDIO_PARAM_MED_STRUCT
{
    short speech_input_FIR_coeffs[SPEECH_PARA_MODE_NUM][NB_FIR_NUM];
    short speech_output_FIR_coeffs[SPEECH_PARA_MODE_NUM][NB_FIR_INDEX_NUM][NB_FIR_NUM];
    short select_FIR_output_index[SPEECH_PARA_MODE_NUM];
    short select_FIR_intput_index[SPEECH_PARA_MODE_NUM];
    short speech_mode_para[SPEECH_PARA_MODE_NUM][SPEECH_PARA_NUM];
} AUDIO_PARAM_MED_STRUCT;

#define CFG_FILE_AUDIO_PARAM_MED_REC_SIZE        sizeof(AUDIO_PARAM_MED_STRUCT)
#define CFG_FILE_AUDIO_PARAM_MED_REC_TOTAL   1


typedef struct _AUDIO_VOLUME_CUSTOM_STRUCT
{
   unsigned char audiovolume_ring[MAX_VOL_CATE][CUSTOM_VOLUME_STEP];
   unsigned char audiovolume_key[MAX_VOL_CATE][CUSTOM_VOLUME_STEP];
   unsigned char audiovolume_mic[MAX_VOL_CATE][CUSTOM_VOLUME_STEP];
   unsigned char audiovolume_fmr[MAX_VOL_CATE][CUSTOM_VOLUME_STEP];
   unsigned char audiovolume_sph[MAX_VOL_CATE][CUSTOM_VOLUME_STEP];
   unsigned char audiovolume_sid[MAX_VOL_CATE][CUSTOM_VOLUME_STEP];
   unsigned char audiovolume_media[MAX_VOL_CATE][CUSTOM_VOLUME_STEP];
   unsigned char audiovolume_matv[MAX_VOL_CATE][CUSTOM_VOLUME_STEP];
} AUDIO_VOLUME_CUSTOM_STRUCT;

#define CFG_FILE_AUDIO_VOLUME_CUSTOM_REC_SIZE        sizeof(AUDIO_VOLUME_CUSTOM_STRUCT)
#define CFG_FILE_AUDIO_VOLUME_CUSTOM_REC_TOTAL   1

#endif

