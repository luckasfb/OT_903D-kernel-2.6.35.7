/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   AudioMeta.cpp
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   Audio HW Testing Tool for META
 *
 * Author:
 * -------
 *   Stan Huang
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * 06 03 2011 donglei.ji
 * [ALPS00051363] [Critical Patch][Dual Mic] Calibration Meta Tool Support On GB FDD
 * Meta tool dual mic support check in - MM handle framework.
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/


/*****************************************************************************
*                E X T E R N A L   R E F E R E N C E S
******************************************************************************
*/

#include <AudioYusuLad.h>
#include <AudioYusuCcci.h>
#include <AudioYusuVolumeController.h>
#include <AudioYusuDef.h>
#include <AudioYusuHeadsetMessage.h>
#include <AudioYusuCustParam.h>
#include <AudioSystem.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <../utils/threads.h>

#include <AudioAfe.h>
#include <AudioAnalogAfe.h>
#include <AudioIoctl.h>
#include <AudioMeta.h>

//ACF
#include "AudioCompensationFilter.h"
#include "AudioCompFltCustParam.h"
//for log
#include "WM2Linux.h"
using namespace android;

/*****************************************************************************
*                     C O M P I L E R   F L A G S
******************************************************************************
*/

#define TEMP_FOR_DUALMIC
#define  AUD_DL1_USE_SLAVE

/*****************************************************************************
*                          C O N S T A N T S
******************************************************************************
*/
#define AUDIO_APPLY_MAX_GAIN (0xffff)
#define AUDIO_APPLY_BIG_GAIN (0xcccc)

#define MIC1_OFF  0
#define MIC1_ON   1
#define MIC2_OFF  2
#define MIC2_ON   3

#define PEER_BUF_SIZE 2*1024
#define ADVANCED_META_MODE 5
/*****************************************************************************
*                         D A T A   T Y P E S
******************************************************************************
*/
enum audio_devices {
	// output devices
	OUT_EARPIECE = 0,
	OUT_SPEAKER = 1,
	OUT_WIRED_HEADSET = 2,
	DEVICE_OUT_WIRED_HEADPHONE = 3,
	DEVICE_OUT_BLUETOOTH_SCO = 4
};


/*****************************************************************************
*                   G L O B A L      V A R I A B L E
******************************************************************************
*/
static android::AudioAfe *mAsmReg = NULL;
static android::AudioAnalog *mAnaReg = NULL;
static int mFd = 0;
static bool bMetaAudioInited = false;
static bool bMetaAudioRecording = false;
static android::LAD *mLad = NULL;
static android::AudioYusuVolumeController *mVolumeController = NULL;

Mutex mLock;
Mutex mLockStop;
Condition mWaitWorkCV;
Condition mWaitStopCV;

static int META_SetEMParameter( void *audio_par );
static int META_GetEMParameter( void *audio_par );
static void Audio_Set_Speaker_Vol(int level);
static void Audio_Set_Speaker_On(int Channel);
static void Audio_Set_Speaker_Off(int Channel);

#ifndef GENERIC_AUDIO

/*****************************************************************************
*                        F U N C T I O N   D E F I N I T I O N
******************************************************************************
*/

static void *AudioRecordControlLoop(void *arg)
{
   FT_L4AUD_REQ *pArg = (FT_L4AUD_REQ *)arg;
   FT_L4AUD_CNF audio_cnf;

   memset(&audio_cnf, 0, sizeof(FT_L4AUD_CNF));
   audio_cnf.header.id = FT_L4AUD_CNF_ID;
   audio_cnf.header.token = pArg->header.token;
   audio_cnf.op = pArg->op;
   audio_cnf.status = META_SUCCESS;
   LOGD("AudioRecordControlLoop in");

   switch (pArg->op)
   {
     case FT_L4AUD_OP_DUALMIC_RECORD:
	 {
		LOGD("AudioRecordControlLoop begine to wait");
	 	mWaitWorkCV.waitRelative(mLock, milliseconds(pArg->req.dualmic_record.duration+500));
		AudioSystem::setParameters(0, String8("META_DUAL_MIC_RECORD=0"));
		AudioSystem::setParameters(0, String8("META_SET_DUAL_MIC_FLAG=0"));
		break;
	 }
	 case FT_L4AUD_OP_PLAYBACK_DUALMICRECORD:
	 {
	 	LOGD("AudioRecordControlLoop begine to wait");
	 	mWaitWorkCV.waitRelative(mLock, milliseconds(pArg->req.playback_dualmic_record.recordDuration+500));
		AudioSystem::setParameters(0, String8("META_DUAL_MIC_REC_PLAY=0"));
		AudioSystem::setParameters(0, String8("META_SET_DUAL_MIC_FLAG=0"));
		break;
	 }
	 case FT_L4AUD_OP_PLAYBACK_DUALMICRECORD_HS:
	 {
	 	LOGD("AudioRecordControlLoop begine to wait");
	 	mWaitWorkCV.waitRelative(mLock, milliseconds(pArg->req.playback_dualmic_record_hs.recordDuration+500));
		AudioSystem::setParameters(0, String8("META_DUAL_MIC_REC_PLAY_HS=0"));
		AudioSystem::setParameters(0, String8("META_SET_DUAL_MIC_FLAG=0"));
		break;
	 }
     default:
	break;
   }

//   usleep(1000000);
   bMetaAudioRecording = false;
   audio_cnf.cnf.dualmic_record_cnf.state = RECORD_END;

   META_LOG("Record end, audio_cnf.status = %d \r", audio_cnf.status);
   WriteDataToPC(&audio_cnf, sizeof(FT_L4AUD_CNF), NULL, 0);
   mWaitStopCV.signal();

   return NULL;
}
static void Audio_Set_Speaker_Vol(int level)
{
   if(mFd){
      ioctl(mFd,SET_SPEAKER_VOL,level);// init speaker volume level 0
      LOGD("Audio_Set_Speaker_Vol with level = %d",level);
   }
   else{
      LOGD("Set_Speaker_Vol with mFd not init\n");
   }
}

static void Audio_Set_Speaker_On(int Channel)
{
   META_LOG("Audio_Set_Speaker_On Channel = %d\n",Channel);
   if (mFd){
      ioctl(mFd,SET_SPEAKER_ON,Channel);// enable speaker
   }
   else{
      LOGD("Audio_Set_Speaker_On with mFd not init\n");
   }
}

static void Audio_Set_Speaker_Off(int Channel)
{
   META_LOG("Audio_Set_Speaker_Off Channel = %d\n",Channel);
   if (mFd){
      ioctl(mFd,SET_SPEAKER_OFF,Channel);// disable speaker
   }
   else{
      LOGD("Audio_Set_Speaker_Off with mFd not init\n");
   }
}

bool META_Audio_init(void)
{
   META_LOG("+META_Audio_init");

   int bootMode = getBootMode();
   if(bootMode==ADVANCED_META_MODE) {
   	  META_LOG("META_Audio_init, the mode is advanced meta mode");
   	  bMetaAudioInited = true;
   	  return true;
   }
   if(bMetaAudioInited == true){
      META_LOG("META_Audio_init, Already init");
      return true;
   }
   META_LOG("META_Audio_init bMetaAudioInited=%d",bMetaAudioInited);

   mFd = ::open("/dev/eac", O_RDWR);

   int err =0;
   /*  Init asm */
   mAsmReg = new android::AudioAfe(NULL);
   err = mAsmReg->Afe_Init(mFd);
   if(err == false){
   	LOGD("Afe_Init error");
   	return false;
   }

   /* init analog  */
   mAnaReg = new android::AudioAnalog(NULL);
   err = mAnaReg->AnalogAFE_Init(mFd);

   if(err == false){
   	LOGD("AnalogAFE_Init error");
   	return false;
   }
#ifdef  AUD_DL1_USE_SLAVE
            ioctl(mFd,SET_DL1_SLAVE_MODE, 1);  //Set Audio DL1 slave mode
            ioctl(mFd,INIT_DL1_STREAM,0x2700);    // init AFE
#else
            ioctl(mFd,SET_DL1_SLAVE_MODE, 0);  //Set Audio DL1 master mode
            ioctl(mFd,INIT_DL1_STREAM,0x3000);    // init AFE
#endif

   ioctl(mFd,START_DL1_STREAM,0);        // init memory

   /* create LAD */
   // lad will only be create once
   if(mLad == NULL)
   {
      mLad = new android::LAD(NULL);
      if( !mLad->LAD_Initial() ){
         LOGD("LAD_Initial error!");
         return false;
      }
      if(mLad->mHeadSetMessager->SetHeadInit() == false)
         LOGE("Common_Audio_init SetHeadInit error");

//      mLad->LAD_SwitchVCM(true);
      fcntl(mLad->pCCCI->GetRxFd(), F_SETFD, FD_CLOEXEC);
      fcntl(mLad->pCCCI->GetTxFd(), F_SETFD, FD_CLOEXEC);

      LOGD("LAD create success!");
   }


   /* new an volume controller */
   mVolumeController = new android::AudioYusuVolumeController(mFd,NULL,mAsmReg,mAnaReg);
   mVolumeController->InitVolumeController ();
   mVolumeController->ApplyGain(AUDIO_APPLY_BIG_GAIN,OUT_SPEAKER);  //apply volume
   bMetaAudioInited = true;

   Audio_Set_Speaker_Vol(12);
   Audio_Set_Speaker_Off(Channel_Stereo);
   mAnaReg->SetAnaReg(VAUDN_CON0,0x0001,0xffff);
   mAnaReg->SetAnaReg(VAUDP_CON0,0x0001,0xffff);
   mAnaReg->SetAnaReg(VAUDP_CON1,0x0000,0xffff);
   mAnaReg->SetAnaReg(VAUDP_CON2,0x012B,0xffff);

   META_LOG("-META_Audio_init");
   return true;
}


bool META_Audio_deinit()
{
   META_LOG("META_Audio_deinit bMetaAudioInited = %d",bMetaAudioInited);
   int bootMode = getBootMode();
   if(bootMode==ADVANCED_META_MODE) {
   	  META_LOG("META_Audio_deinit, the mode is advanced meta mode");
   	  bMetaAudioInited = false;
   	  return true;
   }

   if(bMetaAudioInited == true){
      bMetaAudioInited = false;
      ioctl(mFd,STANDBY_DL1_STREAM,0);
   }
   else{
      return true;
   }
   if(mVolumeController)
   {
      delete mVolumeController;
      mVolumeController = NULL;
      LOGD("delete mVolumeController");
   }
   if(mAsmReg){
      mAsmReg->Afe_Deinit();
      delete mAsmReg;
      mAsmReg = NULL;
      LOGD("delete mAsmReg");
   }
   if(mAnaReg){
      mAnaReg->AnalogAFE_Deinit();
      delete mAnaReg;
      mAnaReg = NULL;
      LOGD("delete mAnaReg");
   }
   if(mLad){
      mLad->LAD_Deinitial();
      delete mLad;
      mLad = NULL;
      LOGD("delete mLad");
   }
   return true;
}

bool RecieverLoopbackTest(char echoflag)
{
   META_LOG("RecieverLoopbackTest echoflag=%d",echoflag);
   int ret =0;
//   AUDIO_CUSTOM_PARAM_STRUCT msndparam;
//   android::GetCustParamFromNV (&msndparam);
//   unsigned int outputVolume = 0xFFFFFFFF - (msndparam.volume[0][4])<<1;
//   unsigned int micgain = 0xFFFFFFFF - (msndparam.volume[0][2])<<1;
//   LOGD("MT657x outputVolume=%x, MicGain=%x",outputVolume,micgain);
   ret = mLad->LAD_SetAfeLoopback(echoflag);

//   mLad->LAD_SetOutputVolume (outputVolume);
//   mLad->LAD_SetMicrophoneVolume (micgain);

//   mLad->LAD_SetOutputVolume(0xFFFFFFE8);      //0x18=0x00011000=24, (2's complement+1)=0xE8, 24/2 = -12dB
   mLad->LAD_SetOutputVolume(0xFFFFFFF4);        //0x0C=0x00001100=12, (2's complement+1)=0xF4, 12/2 = -6dB

//   mLad->LAD_SetMicrophoneVolume(0xFFFFFF9E);  //0x62=0x01100010=98, (2's complement+1)=0x9E, 98/2 = -49dB
//   mLad->LAD_SetMicrophoneVolume(0xFFFFFFC4);  //0x3C=0x00111100=60, (2's complement+1)=0xC4, 60/2 = -30dB
   mLad->LAD_SetMicrophoneVolume(0xFFFFFFE8);    //0x18=0x00011000=24, (2's complement+1)=0xE8, 24/2 = -12dB

   mLad->LAD_SetSidetoneVolume (0xFFFFFFFF);

   return true;
}

bool RecieverLoopbackTest_Mic2(char echoflag)
{
   LOGD("RecieverLoopbackTest_Mic2 echoflag=%d",echoflag);
   int ret =0;
//   SPEECH_CUSTOM_PARAM_STRUCT msndparam;
//   android::GetCustParamFromNV (&msndparam);
//   unsigned int outputVolume = 0xFFFFFFFF - (msndparam.volume[0][4])<<1;
//   unsigned int micgain = 0xFFFFFFFF - (msndparam.volume[0][2])<<1;
//   LOGD("MT657x outputVolume=%x, MicGain=%x",outputVolume,micgain);

   char mic2_flag;

   if(echoflag == true)
   {
      mic2_flag = MIC2_ON;
   }
   else
   {
      mic2_flag = MIC2_OFF;
   }

   ret = mLad->LAD_SetAfeLoopback(mic2_flag);
#ifdef TEMP_FOR_DUALMIC
   if(mic2_flag == MIC2_ON)
   {
      mAnaReg->SetAnaReg(AUDIO_CON23,0x0800,0x0800);
   }
   else if(mic2_flag == MIC2_OFF)
   {
      mAnaReg->SetAnaReg(AUDIO_CON23,0x0000,0x0800);
   }
#endif

//   mLad->LAD_SetOutputVolume (outputVolume);
//   mLad->LAD_SetMicrophoneVolume (micgain);

//   mLad->LAD_SetOutputVolume(0xFFFFFFE8);      //0x18=0x00011000=24, (2's complement+1)=0xE8, 24/2 = -12dB
   mLad->LAD_SetOutputVolume(0xFFFFFFF4);        //0x0C=0x00001100=12, (2's complement+1)=0xF4, 12/2 = -6dB

//   mLad->LAD_SetMicrophoneVolume(0xFFFFFF9E);  //0x62=0x01100010=98, (2's complement+1)=0x9E, 98/2 = -49dB
//   mLad->LAD_SetMicrophoneVolume(0xFFFFFFC4);  //0x3C=0x00111100=60, (2's complement+1)=0xC4, 60/2 = -30dB
   mLad->LAD_SetMicrophoneVolume(0xFFFFFFE8);    //0x18=0x00011000=24, (2's complement+1)=0xE8, 24/2 = -12dB

   mLad->LAD_SetSidetoneVolume (0xFFFFFFFF);

   return true;
}


bool RecieverTest(char receiver_test)
{
   META_LOG("RecieverTest receiver_test=%d",receiver_test);
   if(receiver_test)
   {
      mLad->LAD_SetOutputDevice(android::LADOUT_SPEAKER1);
      mLad->LAD_SetOutputVolume(0xFFFFFFE8);
      mLad->LAD_SwitchVCM(true);
      mAnaReg->FTM_AnaLpk_on();
      mAnaReg->SetAnaReg(AUDIO_CON3,0x01C0,0xffff);
      mAnaReg->SetAnaReg(AUDIO_CON5,0x4000,0xffff);
      mAsmReg->Afe_DL_Unmute(AFE_MODE_DAC);
      mAsmReg->Afe_Enable_SineWave(true);
   }
   else
   {
      mLad->LAD_SwitchVCM(false);
      mAsmReg->Afe_Enable_SineWave(false);
      mAnaReg->FTM_AnaLpk_off();
   }

   return true;
}

bool LouderSPKTest(char left_channel, char right_channel)
{
   META_LOG("LouderSPKTest left_channel=%d, right_channel=%d",left_channel,right_channel);

   int Speaker_Channel =0;
   if( left_channel == 0 && right_channel == 0)
   {
      mAsmReg->Afe_Enable_SineWave(false);
      mAnaReg->FTM_AnaLpk_off();
      Audio_Set_Speaker_Off(Channel_Stereo);
   }
   else
   {
      mAnaReg->FTM_AnaLpk_on();
      mVolumeController->ApplyGain(AUDIO_APPLY_BIG_GAIN,OUT_SPEAKER);  //apply volume
      mAsmReg->Afe_DL_Unmute(AFE_MODE_DAC);
      mAsmReg->Afe_Enable_SineWave(true);
      if(left_channel ==1 && right_channel == 1)
      {
         Audio_Set_Speaker_On(Channel_Stereo);
      }
      else if(right_channel ==1){
         Audio_Set_Speaker_On(Channel_Right);
      }
      else if(left_channel == 1){
         Audio_Set_Speaker_On(Channel_Left);
      }
   }
   return true;
}

bool EarphoneLoopbackTest(char bEnable)
{
   META_LOG("EarphoneLoopbackTest bEnable=%d",bEnable);

   Audio_Set_Speaker_Off(Channel_Stereo);
   mLad->LAD_SetAfeLoopback(bEnable);
   if(bEnable)
   {
      mLad->LAD_SetInputSource(android::LADIN_Microphone2);
      mVolumeController->ApplyGain(AUDIO_APPLY_BIG_GAIN,OUT_SPEAKER);  //apply volume
      mAsmReg->Afe_DL_Unmute(AFE_MODE_DAC);
   }
   else
   {
      mLad->LAD_SetInputSource(android::LADIN_Microphone1);
      mAsmReg->Afe_DL_Mute(AFE_MODE_DAC);
   }

   return true;
}

bool EarphoneTest(char bEnable)
{
   META_LOG("EarphoneTest bEnable=%d",bEnable);

   Audio_Set_Speaker_Off(Channel_Stereo);
   if(bEnable)
   {
      mLad->LAD_SetInputSource(android::LADIN_Microphone2);
      mAnaReg->FTM_AnaLpk_on();
      mVolumeController->ApplyGain(AUDIO_APPLY_BIG_GAIN,OUT_SPEAKER);  //apply volume
      mAsmReg->Afe_DL_Unmute(AFE_MODE_DAC);
      mAsmReg->Afe_Enable_SineWave(true);
   }
   else
   {
      mAsmReg->Afe_Enable_SineWave(false);
      mAnaReg->FTM_AnaLpk_off();
      Audio_Set_Speaker_Off(Channel_Stereo);
   }

   return true;
}

bool FMLoopbackTest(char bEnable)
{
   META_LOG("FMLoopbackTest bEnable = %d",bEnable);

   // ToDo...
   Audio_Set_Speaker_Off(Channel_Stereo);
   if(bEnable)
   {
      // enable Digital AFE
      // ...
      // enable Analog AFE
      mAnaReg->SetAnaReg(PLL_CON2,0x20,0x20);          // turn on AFE (26M Clock)
      mAnaReg->SetAnaReg(AUDIO_CON1,0x0C0C,MASK_ALL);
      mAnaReg->SetAnaReg(AUDIO_CON2,0x000C,MASK_ALL);
      mAnaReg->SetAnaReg(AUDIO_CON3,0x0070,MASK_ALL);  // enable voice buffer, audio left/right buffer
      mAnaReg->SetAnaReg(AUDIO_CON5,0x0220,MASK_ALL);  // FM mono playback (analog line in)

      mAsmReg->Afe_DL_Unmute(AFE_MODE_DAC);
      Audio_Set_Speaker_On(Channel_Stereo);
   }
   else
   {
      mAsmReg->Afe_DL_Mute(AFE_MODE_DAC);
      Audio_Set_Speaker_Off(Channel_Stereo);
      // disable Digital AFE
      // ...
      // disable Analog AFE
      mAnaReg->SetAnaReg(AUDIO_CON3,0x0000,0x00f0);  // disable voice buffer, audio left/right buffer
      mAnaReg->SetAnaReg(AUDIO_CON5,0x0440,0x0ff0);  // disable FM mono playback (analog line in)
      mAnaReg->SetAnaReg(PLL_CON2,0x0,0x20);         // turn off AFE (26M Clock)
   }
   return true;
}

int Audio_I2S_Play(int enable_flag)
{
   LOGD("[META] Audio_I2S_Play");
   if(enable_flag == true)
   {
      LOGD("[META] +Audio_I2S_Play true");
      mAnaReg->Meta_Open_Analog(AUDIO_PATH);
      mAsmReg->Afe_DL_Start(AFE_MODE_FTM_I2S);
      mAsmReg->Afe_Set_Stream_Gain(0xffff); // hardcore AFE gain
      Audio_Set_Speaker_On(Channel_Stereo);
      ::ioctl(mFd,AUDDRV_SET_FM_I2S_GPIO,0);// enable FM use I2S
      LOGD("[META] -Audio_I2S_Play true");
   }
   else
   {
      LOGD("[META] +Audio_I2S_Play false");
      Audio_Set_Speaker_Off(Channel_Stereo);
      mAsmReg->Afe_DL_Stop(AFE_MODE_FTM_I2S);
      mAnaReg->Meta_Close_Analog();
      ::ioctl(mFd,AUDDRV_RESET_BT_FM_GPIO,0);// Reset GPIO pin mux
      LOGD("[META] -Audio_I2S_Play false");

   }
   return true;
}

int Audio_FMTX_Play(bool Enable, unsigned int Freq)
{
   LOGD("Audio_FMTX_Play : Enable =%d, Freq=%d ", Enable, Freq);
   if(Enable)
   {
       mAnaReg->SetAnaReg(AUDIO_CON1, 0x000C, 0xffff);
       mAnaReg->SetAnaReg(AUDIO_CON2, 0x000C, 0xffff);
       mAnaReg->SetAnaReg(AUDIO_CON3, 0x01B0, 0xffff);
       mAsmReg->Afe_DL_Unmute(AFE_MODE_DAC);
   }
   else
   {
       mAnaReg->SetAnaReg(AUDIO_CON1, 0, 0xffff);
       mAnaReg->SetAnaReg(AUDIO_CON2, 0, 0xffff);
       mAnaReg->SetAnaReg(AUDIO_CON3, 0, 0xffff);
       mAsmReg->Afe_DL_Mute(AFE_MODE_DAC);
   }
   return mAsmReg->Afe_FmTx_SineWave(Enable, Freq);
}

bool EarphoneMicbiasEnable(bool bMicEnable)
{
   META_LOG("EarphoneMicbiasEnable bEnable = %d",bMicEnable);

   mLad->LAD_SwitchMicBias((/*int32*/signed long)bMicEnable);
   return true;
}

static int META_SetEMParameter( void *audio_par )
{
   int WriteCount = 0;
   android::SetCustParamToNV( (AUDIO_CUSTOM_PARAM_STRUCT *)audio_par);
   return WriteCount;
}

static int META_GetEMParameter( void *audio_par )
{
   int ReadConut = 0;
   android::GetCustParamFromNV( (AUDIO_CUSTOM_PARAM_STRUCT *)audio_par);

   return ReadConut;
}

static int META_SetACFParameter( void *audio_par )
{
   int WriteCount = 0;
   android::SetAudioCompFltCustParamToNV( (AUDIO_ACF_CUSTOM_PARAM_STRUCT *)audio_par);
   return WriteCount;
}

static int META_GetACFParameter( void *audio_par )
{
   int ReadConut = 0;
   android::GetAudioCompFltCustParamFromNV( (AUDIO_ACF_CUSTOM_PARAM_STRUCT *)audio_par);

   return ReadConut;
}

static int META_SetACFPreviewParameter( void *audio_par )
{
   int WriteCount = 0;
   //set to working buffer

   //Tina todo acf feature
   android::AudioSystem::SetACFPreviewParameter( (AUDIO_ACF_CUSTOM_PARAM_STRUCT *)audio_par, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
   return WriteCount;
}

static void META_Load_Volume(int var)
{
    android::AudioSystem::SetAudioCommand(0x50,0x0);
    return;
}


//<--- add for dual mic support on advanced meta mode
static META_BOOL SetPlaybackFile(const char *fileName)
{
	 FILE *fp;
	 LOGD("SetPlaybackFile() file name %s", fileName);
	 fp = fopen(fileName, "wb+");
	 if (fp==NULL) {
	 	 LOGE("SetPlaybackFile() open file failed");
		 return false;
	 }

	 fclose(fp);
	 return true;
}

static META_BOOL DownloadDataToFile(const char * fileName, char *data, unsigned short size)
{
	 FILE *fp;
	 LOGV("DownloadDataToFile() file name %s, data 0x%x, size %d", fileName, data, size);
   if (NULL==data || 0==size)
  	 return false;

	 fp = fopen(fileName, "ab+");
	 if (fp==NULL) {
	 	 LOGE("DownloadDataToFile() open file failed");
		 return false;
	 }

	 if (1!=fwrite(data, size, 1, fp)) {
		 LOGE("DownloadDataToFile failed, fwrite failed, file name:%s", fileName);
		 fclose(fp);
		 return false;
	 }

	 fclose(fp);
	 return true;
}

static META_BOOL DualMICRecorder(FT_L4AUD_REQ *req, FT_L4AUD_CNF *audio_par)
{
    int ret = 0;
    char mPlaybackFileNameParam[256];
    char mRecordFileNameParam[256];
    pthread_t s_tid_audio;
    META_BOOL playbackPlusRecord = false;

    AudioSystem::setParameters(0, String8("META_SET_DUAL_MIC_FLAG=1"));

    switch (req->op)
    {
     case FT_L4AUD_OP_DUALMIC_RECORD:
	   {
	 	   sprintf(mRecordFileNameParam, "META_DUAL_MIC_OUT_FILE_NAME=%s", req->req.dualmic_record.filename);
	     LOGV("DualMICRecorder(FT_L4AUD_OP_DUALMIC_RECORD): file name parameters : %s", mRecordFileNameParam);
	     AudioSystem::setParameters(0, String8(mRecordFileNameParam));
	     AudioSystem::setParameters(0, String8("HQA_RDMIC_P1=3"));
	     AudioSystem::setParameters(0, String8("META_DUAL_MIC_RECORD=1"));
	     playbackPlusRecord = false;
		   break;
	   }
	   case FT_L4AUD_OP_PLAYBACK_DUALMICRECORD:
	   {
	 	   sprintf(mRecordFileNameParam, "META_DUAL_MIC_OUT_FILE_NAME=%s", req->req.playback_dualmic_record.recordingFilename);
	 	   sprintf(mPlaybackFileNameParam, "META_DUAL_MIC_IN_FILE_NAME=%s", req->req.playback_dualmic_record.playbackFilename);
	     LOGV("DualMICRecorder(FT_L4AUD_OP_PLAYBACK_DUALMICRECORD): recording file name: %s, playback file name: %s", mRecordFileNameParam, mPlaybackFileNameParam);
	     AudioSystem::setParameters(0, String8(mRecordFileNameParam));
	     AudioSystem::setParameters(0, String8(mPlaybackFileNameParam));
	     AudioSystem::setParameters(0, String8("HQA_RDMIC_P1=3"));
		   AudioSystem::setParameters(0, String8("META_DUAL_MIC_REC_PLAY=1"));
		   playbackPlusRecord = true;
		   break;
	   }
	   case FT_L4AUD_OP_PLAYBACK_DUALMICRECORD_HS:
	   {
	   	 sprintf(mRecordFileNameParam, "META_DUAL_MIC_OUT_FILE_NAME=%s", req->req.playback_dualmic_record_hs.recordingFilename);
	 	   sprintf(mPlaybackFileNameParam, "META_DUAL_MIC_IN_FILE_NAME=%s", req->req.playback_dualmic_record_hs.playbackFilename);
	     LOGV("DualMICRecorder(FT_L4AUD_OP_PLAYBACK_DUALMICRECORD_HS): recording file name: %s, playback file name: %s", mRecordFileNameParam, mPlaybackFileNameParam);
	     AudioSystem::setParameters(0, String8(mRecordFileNameParam));
	     AudioSystem::setParameters(0, String8(mPlaybackFileNameParam));
	     AudioSystem::setParameters(0, String8("HQA_RDMIC_P1=3"));
		   AudioSystem::setParameters(0, String8("META_DUAL_MIC_REC_PLAY_HS=1"));
		   playbackPlusRecord = true;
		   break;
	   }
     default:
	     break;
    }

	  ret = pthread_create(&s_tid_audio, NULL, AudioRecordControlLoop, (void *)req);
    if (ret != 0) {
        LOGE("Fail to create record control thread ");
        if (playbackPlusRecord)
		        AudioSystem::setParameters(0, String8("META_DUAL_MIC_REC_PLAY=0"));
	      else
		        AudioSystem::setParameters(0, String8("META_DUAL_MIC_RECORD=0"));

        AudioSystem::setParameters(0, String8("META_SET_DUAL_MIC_FLAG=0"));
		    return false;
    }

    audio_par->cnf.dualmic_record_cnf.state = RECORD_START;
    bMetaAudioRecording = true;

	  return true;

}

static META_BOOL StopDualMICRecorder()
{
	  mWaitWorkCV.signal(); // signal thread to stop record
	  mWaitStopCV.waitRelative(mLockStop, milliseconds(2000)); //waiting thread exit
    LOGV("StopDualMICRecorder():Stop dual mic recording ");
	  return true;
}

static META_BOOL UplinkDataToPC(ft_l4aud_ul_data_package_req &uplike_par, void *audio_par, unsigned char *pBuff)
{
	FILE *fp;
	int uplinkdatasize = 0;
	static long mCurrFilePosition = 0;
	static char mLastFileName[256] = "hello";
	ft_l4aud_ul_data_package_cnf *rec_data_cnf = (ft_l4aud_ul_data_package_cnf *)audio_par;

	if (uplike_par.size>PEER_BUF_SIZE) {
		LOGE("UplinkDataToPC():required data size more than limitation, ReadSize=%d, Limiation=%d", uplike_par.size, PEER_BUF_SIZE);
		return false;
	}

	fp = fopen(uplike_par.filename, "rb+");
	if (NULL==fp)
		return false;

  if (0!=strcmp(mLastFileName, uplike_par.filename)) {
		mCurrFilePosition = 0;
		strcpy(mLastFileName, uplike_par.filename);
		LOGD("UplinkDataToPC():read different file, from beginning of the file ");
	}

	if (1==(uplike_par.flag&0x01)) {
		LOGD("UplinkDataToPC():read data from beginnig of the file ");
		mCurrFilePosition = 0;
	}

	fseek(fp, mCurrFilePosition, SEEK_SET);
	uplinkdatasize = fread(pBuff, sizeof(char), uplike_par.size, fp);
	rec_data_cnf->size = uplinkdatasize;
	rec_data_cnf->flag = 0;

	if (uplinkdatasize<uplike_par.size && !feof(fp)) {
		fclose(fp);
		mCurrFilePosition = 0;
		return false;
	}else if (feof(fp)){
		mCurrFilePosition = 0;
		rec_data_cnf->flag = 1;
		LOGD("UplinkDataToPC():read data to end of the file ");
	}else {
		mCurrFilePosition = ftell(fp);
	}

	fclose(fp);
	return true;
}

static META_BOOL setParameters(ft_l4aud_dualmic_set_params_req &set_par)
{
	char mParams[128];
	if (0==strlen(set_par.param)) {
		LOGE("parameters name is null");
		return false;
	}

	sprintf(mParams, "%s=%d", set_par.param, set_par.value);
	AudioSystem::setParameters(0, String8(mParams));
	return true;
}

static META_BOOL getParameters(ft_l4aud_dualmic_get_params_req &get_par, void *audio_par)
{
	char mParams[128];
	char *pParamName;
	char *pParamValue;
	ft_l4aud_dualmic_get_param_cnf *get_param_cnf = (ft_l4aud_dualmic_get_param_cnf *)audio_par;

	if (0==strlen(get_par.param)) {
		LOGE("parameters name is null");
		return false;
	}

	String8 mValue = AudioSystem::getParameters(0,String8(get_par.param));
  LOGV("getParameters:getParameters the parameters is %s", mValue.string());
  strcpy(mParams, mValue.string());
  pParamValue = mParams;
	pParamName = strsep(&pParamValue, "=");
	if (NULL==pParamName || NULL==pParamValue)
		return false;

	strcpy(get_param_cnf->param_name, pParamName);
	get_param_cnf->value = atoi(pParamValue);
	LOGD("getParameters: param name %s, param value %d", get_param_cnf->param_name, get_param_cnf->value);
	return true;
}
//---> add for dual mic support on advanced meta mode

void META_Audio_OP(FT_L4AUD_REQ *req, char *peer_buff, unsigned short peer_len)
{
   META_BOOL ret = true;
   unsigned char pBuff[PEER_BUF_SIZE];
   unsigned short mReadSize = 0;
   FT_L4AUD_CNF audio_cnf;
   memset(&audio_cnf, 0, sizeof(FT_L4AUD_CNF));
   audio_cnf.header.id = FT_L4AUD_CNF_ID;
   audio_cnf.header.token = req->header.token;
   audio_cnf.op = req->op;
   audio_cnf.status = META_SUCCESS;

   if(bMetaAudioInited == FALSE)
   {
      META_LOG("META_Audio_OP not initialed \r");
      audio_cnf.status = META_FAILED;
      WriteDataToPC(&audio_cnf, sizeof(FT_L4AUD_CNF), NULL, 0);
   }

	META_LOG("META_Audio_OP req->op=%d \r",req->op);

	switch(req->op)
	{
      case FT_L4AUD_OP_SET_PARAM_SETTINGS_0809:
      {
         ft_l4aud_set_param_0809 *par;
         META_LOG("META_Audio_OP, Audio Set Param Req \r");

         META_SetEMParameter((void *)&req->req );
         par = (ft_l4aud_set_param_0809 *)&req->req;
         break;
      }

	   case FT_L4AUD_OP_GET_PARAM_SETTINGS_0809:
	   {
         ft_l4aud_set_param_0809 *par;
         META_LOG("META_Audio_OP, Audio Get Param Req\r\n");

         META_GetEMParameter( (void *)&audio_cnf.cnf );
         par = (ft_l4aud_set_param_0809 *)&audio_cnf.cnf;
         break;
	   }
	   case FT_L4AUD_OP_SET_ACF_COEFFS:
	   {
         ft_l4aud_set_acf_param_req *par;
         META_LOG("META_Audio_OP, Audio Set ACF Param Req \r");

         META_SetACFParameter((void *)&req->req );
         par = (ft_l4aud_set_acf_param_req *)&req->req;
         break;
	   }

	   case FT_L4AUD_OP_GET_ACF_COEFFS:
	   {
         ft_l4aud_get_acf_param_cnf *par;
         META_LOG("META_Audio_OP, Audio Get ACF Param Req\r\n");

         META_GetACFParameter( (void *)&audio_cnf.cnf );
         par = (ft_l4aud_get_acf_param_cnf *)&audio_cnf.cnf;
         break;
	   }
	   case FT_L4AUD_OP_SET_PREVIEW_ACF_COEFFS:
	   {
         ft_l4aud_set_acf_param_req *par;
         META_LOG("META_Audio_OP, Audio Set ACF Preview Param Req\r\n");

         META_SetACFPreviewParameter( (void *)&req->req );
         par = (ft_l4aud_set_acf_param_req *)&req->req;
         break;
	   }

	   case FT_L4AUD_OP_SET_ECHO:
	   {
         META_LOG("META_Audio_OP, Loopback test \r\n");
         ft_l4aud_set_echo *par;
         par = (ft_l4aud_set_echo *)&req->req;
         RecieverLoopbackTest(par->echoflag);
         break;
	   }
	   case FT_L4AUD_OP_MIC2_LOOPBACK:
	   {
         LOGD("META_Audio_OP, MIC2 Loopback test \r\n");
         ft_l4aud_set_echo *par;
         par = (ft_l4aud_set_echo *)&req->req;
         RecieverLoopbackTest_Mic2(par->echoflag);
         break;
	   }
	   case FT_L4AUD_OP_RECEIVER_TEST:
	   {
         META_LOG("META_Audio_OP, Receiver test \r\n");
         ft_l4aud_receiver_test *par;
         par = (ft_l4aud_receiver_test *)&req->req;
         RecieverTest(par->receiver_test);
         break;
	   }
	   case FT_L4AUD_OP_LOUDSPK_TEST:
	   {
         META_LOG("META_Audio_OP, LoudSpk test \r\n");
         ft_l4aud_loudspk *par;
         par = (ft_l4aud_loudspk *)&req->req;
         LouderSPKTest(par->left_channel, par->right_channel);
         break;
	   }
      case FT_L4AUD_OP_EARPHONE_TEST:
      {
         META_LOG("META_Audio_OP, Earphone test \r\n");
         EarphoneTest(req->req.eaphone_test.bEnable);
         break;
      }

      case FT_L4AUD_OP_HEADSET_LOOPBACK_TEST:
      {
         META_LOG("META_Audio_OP, Headset loopback test \r\n");
         EarphoneLoopbackTest(req->req.headset_loopback_test.bEnable);
         break;
      }

      case FT_L4AUD_OP_FM_LOOPBACK_TEST:
      {
         META_LOG("META_Audio_OP, FM loopback test \r\n");
          // Need to check FM function is ready
//			 FMLoopbackTest(req->req.fm_loopback_test.bEnable);
         break;
      }

	  case FT_L4AUD_OP_SET_PLAYBACK_FILE:
	  {
	  	   META_LOG("META_Audio_OP, set playback file \r\n");
		     ret = SetPlaybackFile(req->req.dl_playback_file.filename);
		     break;
	  }

	  case FT_L4AUD_OP_DL_DATA_PACKAGE:
	  {
	  	   META_LOG("META_Audio_OP, down link data pakage \r\n");
	  	   ret = DownloadDataToFile(req->req.dl_data_package.filename, peer_buff, peer_len);
		     break;
	  }

	  case FT_L4AUD_OP_DUALMIC_RECORD:
	  {
	       META_LOG("META_Audio_OP, dual mic recording \r\n");
		     ret = false;
		     if (!bMetaAudioRecording)
		         ret = DualMICRecorder(req, &audio_cnf);
		     break;
	  }

	  case FT_L4AUD_OP_PLAYBACK_DUALMICRECORD:
	  {
	       META_LOG("META_Audio_OP, playback and dual mic recording \r\n");
		     ret = false;
		     if (!bMetaAudioRecording)
  	         ret = DualMICRecorder(req, &audio_cnf);

		     break;
	  }

	  case FT_L4AUD_OP_PLAYBACK_DUALMICRECORD_HS:
	  {
	  	   META_LOG("META_Audio_OP, headset playback and dual mic recording \r\n");
		     ret = false;
		     if (!bMetaAudioRecording)
  	         ret = DualMICRecorder(req, &audio_cnf);

		     break;
	  }

      case FT_L4AUD_OP_STOP_DUALMIC_RECORD:
	  {
	  	   META_LOG("META_Audio_OP, stop dual mic recording \r\n");
	  	   ret = false;
	  	   if (bMetaAudioRecording)
		         ret = StopDualMICRecorder();

		     break;
	  }

	  case FT_L4AUD_OP_UL_DATA_PACKAGE:
	  {
	  	   META_LOG("META_Audio_OP, uplink data package \r\n");
	  	   ret = false;
	  	   if (PEER_BUF_SIZE>=req->req.ul_data_package.size) {
		         ret = UplinkDataToPC(req->req.ul_data_package, (void *)&audio_cnf.cnf, pBuff);
             mReadSize = ret==true?audio_cnf.cnf.ul_data_package_cnf.size:0;
		         LOGV("uplink data package  size = %d", mReadSize);
		         LOGV("uplink data package  flag = %d", audio_cnf.cnf.ul_data_package_cnf.flag);
		     }

		     break;
	  }

	  case FT_L4AUD_OP_DUALMIC_SET_PARAMS:
	  {
	  	META_LOG("META_Audio_OP, set parameters \r\n");
	  	setParameters(req->req.dualmic_set_params);
	  	break;
	  }

	  case FT_L4AUD_OP_DUALMIC_GET_PARAMS:
	  {
	  	META_LOG("META_Audio_OP, get parameters \r\n");
	  	getParameters(req->req.dualmic_get_params, (void *)&audio_cnf.cnf);
	  	break;
	  }

      case FT_L4AUD_OP_LOAD_VOLUME:{
          META_Load_Volume(NULL);
          META_LOG("META_Audio_OP, FT_L4AUD_OP_LOAD_VOLUME\r\n");
          ret = true;
          break;
      }

      default:
         audio_cnf.status = META_FAILED;
         break;
   }

   if (!ret)
	     audio_cnf.status = META_FAILED;

   META_LOG("META_Audio_OP, audio_cnf.status = %d \r", audio_cnf.status);
   WriteDataToPC(&audio_cnf, sizeof(FT_L4AUD_CNF), pBuff, mReadSize);

}

#else  // #ifndef GENERIC_AUDIO

void META_Audio_OP(FT_L4AUD_REQ *req, char *peer_buff, unsigned short peer_len)
{
	META_LOG("META_Audio_OP with generic Audio , no test");
}

bool META_Audio_init(void)
{

    META_LOG("META_Audio_init");
    return true;
}

bool META_Audio_deinit()
{
    META_LOG("META_Audio_deinit");
    return true;
}

bool RecieverLoopbackTest(char echoflag)
{
    return true;
}

bool RecieverTest(char receiver_test)
{
    return true;
}

bool LouderSPKTest(char left_channel, char right_channel)
{
    return true;
}

bool EarphoneLoopbackTest(char bEnable)
{
    return true;
}

bool EarphoneTest(char bEnable)
{
    return true;
}

bool FMLoopbackTest(char bEnable)
{
    return true;
}

bool EarphoneMicbiasEnable(bool bMicEnable)
{
    return true;
}

int Audio_I2S_Play(int enable_flag)
{
   return true;
}

int Audio_FMTX_Play(bool Enable, unsigned int Freq)
{
    return true;
}

#endif  // #ifndef GENERIC_AUDIO



