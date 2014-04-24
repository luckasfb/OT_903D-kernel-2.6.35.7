



#ifndef _AUDDRV_IOCTL_MSG_H
#define _AUDDRV_IOCTL_MSG_H





//below is control message
#define AUD_DRV_IOC_MAGIC 'C'

#define SET_AUDSYS_REG         _IOWR(AUD_DRV_IOC_MAGIC, 0x00, Register_Control*)
#define GET_AUDSYS_REG         _IOWR(AUD_DRV_IOC_MAGIC, 0x01, Register_Control*)
#define SET_ANAAFE_REG         _IOWR(AUD_DRV_IOC_MAGIC, 0x02, Register_Control*)
#define GET_ANAAFE_REG         _IOWR(AUD_DRV_IOC_MAGIC, 0x03, Register_Control*)

#define GET_AFE_BUFFER_SIZE    _IOWR(AUD_DRV_IOC_MAGIC, 0x11, unsigned int)
#define SET_SPEAKER_VOL        _IOW(AUD_DRV_IOC_MAGIC, 0x12, int)
#define SET_SPEAKER_ON         _IOW(AUD_DRV_IOC_MAGIC, 0x13, int)
#define SET_SPEAKER_OFF        _IOW(AUD_DRV_IOC_MAGIC, 0x14, int)
#define SET_HEADSET_           _IOW(AUD_DRV_IOC_MAGIC, 0x15, int)

#define OPEN_DL1_STREAM        _IOWR(AUD_DRV_IOC_MAGIC, 0x20, int)
#define CLOSE_DL1_STREAM       _IOWR(AUD_DRV_IOC_MAGIC, 0x21, int)
#define INIT_DL1_STREAM        _IOWR(AUD_DRV_IOC_MAGIC, 0x22, int)
#define START_DL1_STREAM       _IOWR(AUD_DRV_IOC_MAGIC, 0x23, int)
#define STANDBY_DL1_STREAM     _IOWR(AUD_DRV_IOC_MAGIC, 0x24, int)
#define SET_DL1_AFE_BUFFER     _IOWR(AUD_DRV_IOC_MAGIC, 0x25, int)
#define SET_DL1_SLAVE_MODE     _IOWR(AUD_DRV_IOC_MAGIC, 0x26, int)
#define GET_DL1_SLAVE_MODE     _IOWR(AUD_DRV_IOC_MAGIC, 0x27, int)

#define OPEN_I2S_INPUT_STREAM       _IOWR(AUD_DRV_IOC_MAGIC, 0x30, int)
#define CLOSE_I2S_INPUT_STREAM      _IOWR(AUD_DRV_IOC_MAGIC, 0x31, int)
#define START_I2S_INPUT_STREAM      _IOWR(AUD_DRV_IOC_MAGIC, 0x33, int)
#define STANDBY_I2S_INPUT_STREAM    _IOWR(AUD_DRV_IOC_MAGIC, 0x34, int)
#define SET_I2S_Input_BUFFER        _IOWR(AUD_DRV_IOC_MAGIC, 0x35, int)
#define SET_I2S_Output_BUFFER       _IOWR(AUD_DRV_IOC_MAGIC, 0x36, int)
#define SET_AWB_INPUT_STREAM_STATE  _IOWR(AUD_DRV_IOC_MAGIC, 0x37, int)

#define SET_2IN1_SPEAKER          _IOW(AUD_DRV_IOC_MAGIC, 0x41, int)
#define SET_AUDIO_STATE           _IOWR(AUD_DRV_IOC_MAGIC, 0x42 ,SPH_Control*)
#define GET_AUDIO_STATE           _IOWR(AUD_DRV_IOC_MAGIC, 0x43, SPH_Control*)

#define AUD_SET_LINE_IN_CLOCK     _IOWR(AUD_DRV_IOC_MAGIC, 0x50, int)
#define AUD_SET_CLOCK             _IOWR(AUD_DRV_IOC_MAGIC, 0x51, int)
#define AUD_SET_26MCLOCK          _IOWR(AUD_DRV_IOC_MAGIC, 0x52, int)
#define AUD_SET_ADC_CLOCK         _IOWR(AUD_DRV_IOC_MAGIC, 0x53, int)
#define AUD_SET_I2S_CLOCK         _IOWR(AUD_DRV_IOC_MAGIC, 0x54, int)

#define AUDDRV_RESET_BT_FM_GPIO   _IOWR(AUD_DRV_IOC_MAGIC, 0x60, int)
#define AUDDRV_SET_BT_PCM_GPIO    _IOWR(AUD_DRV_IOC_MAGIC, 0x61, int)
#define AUDDRV_SET_FM_I2S_GPIO    _IOWR(AUD_DRV_IOC_MAGIC, 0x62, int)
#define AUDDRV_MT6573_CHIP_VER    _IOWR(AUD_DRV_IOC_MAGIC, 0x63, int)

#define AUDDRV_ENABLE_ATV_I2S_GPIO   _IOWR(AUD_DRV_IOC_MAGIC, 0x65, int)
#define AUDDRV_DISABLE_ATV_I2S_GPIO  _IOWR(AUD_DRV_IOC_MAGIC, 0x66, int)

#define AUDDRV_FM_ANALOG_PATH        _IOWR(AUD_DRV_IOC_MAGIC, 0x67, int)

#define YUSU_INFO_FROM_USER        _IOWR(AUD_DRV_IOC_MAGIC, 0x71, struct _Info_Data*)      //by HP

#define AUDDRV_START_DAI_OUTPUT    _IOWR(AUD_DRV_IOC_MAGIC, 0x81, int)
#define AUDDRV_STOP_DAI_OUTPUT     _IOWR(AUD_DRV_IOC_MAGIC, 0x82, int)

// used for debug
#define AUDDRV_LOG_PRINT          _IOWR(AUD_DRV_IOC_MAGIC, 0xFD, int)
#define AUDDRV_ASSERT_IOCTL       _IOW(AUD_DRV_IOC_MAGIC, 0xFE, int)
#define AUDDRV_BEE_IOCTL          _IOW(AUD_DRV_IOC_MAGIC, 0xFF, int)

// below defines the YUSU_INFO_FROM_USER message
#define INFO_U2K_MATV_AUDIO_START   0x1001
#define INFO_U2K_MATV_AUDIO_STOP    0x1002


//#ifdef HEARING_AID_KERNEL
#define INFO_U2K_HEARINGAID_ENABLE 0x1003
#define INFO_U2K_HEARINGAID_DISABLE 0x1004
//#endif

typedef struct
{
    UINT32 offset;
    UINT32 value;
    UINT32 mask;
}Register_Control;

typedef struct
{
   int bSpeechFlag;
   int bBgsFlag;
   int bRecordFlag;
   int bTtyFlag;
   int bVT;
   int bAudioPlay;   
}SPH_Control;

typedef struct       //HP
{
    UINT32 info;
    UINT32 param1;
    UINT32 param2;
}_Info_Data;



#endif

