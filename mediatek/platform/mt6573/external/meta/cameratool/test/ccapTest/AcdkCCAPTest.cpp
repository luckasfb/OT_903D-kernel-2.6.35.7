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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////
// AcdkCCAPTest.cpp  $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkCCAPTest.cpp
//! \brief

#define LOG_TAG "ACDKCCAPTest"
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <unistd.h>



//#include "mcu_hal.h"

extern "C" {
#include <pthread.h>
}

#include "AcdkTypes.h"
#include "camera_custom_nvram.h"
#include "AcdkIF.h"

#define MEDIA_PATH "//data"
#define PROJECT_NAME "Yusu"

/////////////////////////////////////////////////////////////////////////
//
//! @brief insert the PMEM driver module shell command
//
/////////////////////////////////////////////////////////////////////////
static  const char*  const g_mkPMEMNod_arg_list[] = {
    "mknod",
    "/dev/pmem_multimedia",
    "c",
    "10",
    "0",
    NULL
};

/////////////////////////////////////////////////////////////////////////
//
//! @brief mount SD shell command
//
/////////////////////////////////////////////////////////////////////////
static  const char* const g_mountSD_arg_list[]  = {
       "mount",
       "-t",
       "vfat",
       "/dev/mmcblk0p1",
       "/sdcard",
       NULL
};

/////////////////////////////////////////////////////////////////////////
//
//! @brief unmount SD shell command
//
/////////////////////////////////////////////////////////////////////////
static  const char*  const g_unMountSD_arg_list[] = {
    "umount",
    "/sdcard",
    NULL
};

/////////////////////////////////////////////////////////////////////////
//
//! @brief sync shell command
//
/////////////////////////////////////////////////////////////////////////
static  const char* const g_sync_arg_list[]  = {
    "sync",
    NULL
};


/////////////////////////////////////////////////////////////////////////
//  Global Variable for the the thread
/////////////////////////////////////////////////////////////////////////
static BOOL g_bIsCLITest = TRUE;
static pthread_t g_CliKeyThreadHandle;



/////////////////////////////////////////////////////////////////////////
//
//  thread_exit_handler () -
//! @brief the CLI key input thread, wait for CLI command
//! @param sig: The input arguments
/////////////////////////////////////////////////////////////////////////
void thread_exit_handler(MINT32 a_u4Sig)
{
    ACDK_LOGD("This signal is %d \n", a_u4Sig);
    pthread_exit(0);
}


/////////////////////////////////////////////////////////////////////////
//
//  vExecProgram () -
//! @brief execute the external program
//! @param pProgram: program name
//! @param ppArgList: Arguments
/////////////////////////////////////////////////////////////////////////
VOID vExecProgram(const char *pProgram, const char * const ppArgList[])
{
    pid_t childPid;

    //Duplicate this process
    childPid = fork ();

    if (childPid != 0)
    {
        return;
    }
    else
    {
        //execute the program, searching for it in the path
        execvp(pProgram, (char **)ppArgList);
        abort();
    }
}

/////////////////////////////////////////////////////////////////////////
//
//  vSkipSpace () -
//! @brief skip the space of the input string
//! @param ppInStr: The point of the input string
/////////////////////////////////////////////////////////////////////////
void vSkipSpace(char **ppInStr)
{
    char *s = *ppInStr;

    while (( *s == ' ' ) || ( *s == '\t' ) || ( *s == '\r' ) || ( *s == '\n' ))
    {
        s++;
    }

    *ppInStr = s;
}


/////////////////////////////////////////////////////////////////////////
//
//  getHexToken () -
//! @brief skip the space of the input string
//! @param ppInStr: The point of the input string
/////////////////////////////////////////////////////////////////////////
char* getHexToken(char *inStr, MUINT32 *outVal)
{
    MUINT32 thisVal, tVal;
    char x;
    char *thisStr = inStr;

    thisVal = 0;

    // If first character is ';', we have a comment, so
    // get out of here.

    if (*thisStr == ';')
    {
        return (thisStr);
    }
        // Process hex characters.

    while (*thisStr)
    {
        // Do uppercase conversion if necessary.

        x = *thisStr;
        if ((x >= 'a') && (x <= 'f'))
        {
            x &= ~0x20;
        }
        // Check for valid digits.

        if ( !(((x >= '0') && (x <= '9')) || ((x >= 'A') && (x <= 'F'))))
        {
            break;
        }
        // Hex ASCII to binary conversion.

        tVal = (MUINT32)(x - '0');
        if (tVal > 9)
        {
            tVal -= 7;
        }

        thisVal = (thisVal * 16) + tVal;

        thisStr++;
    }

        // Return updated pointer and decoded value.

    *outVal = thisVal;
    return (thisStr);
}

static MUINT32 g_u4ImgCnt = 0;

/////////////////////////////////////////////////////////////////////////
//
//   mrSaveRAWImg () -
//!
//!  brief for geneirc function to save image file
//
/////////////////////////////////////////////////////////////////////////
BOOL bRAW10To8(char *a_pInBuf,  MUINT32 a_u4Width, MUINT32 a_u4Height, MUINT32 a_u4Size, UINT8 a_uBitDepth,  char *a_pOutBuf)
{
    if (a_uBitDepth != 10)
    {
        ACDK_LOGE("Not support bitdepth");
        return FALSE;
    }

    MUINT32 *pu4SrcBuf = (MUINT32 *) a_pInBuf;
    //char *pucBuf = (UCHAR *) malloc (a_u4Width * a_u4Height  * 2 * sizeof(UCHAR));

    char *puDestBuf = (char *)a_pOutBuf;

    while (puDestBuf < (char *)a_pOutBuf + a_u4Width * a_u4Height)
    {
        MUINT32 u4Pixel = *(pu4SrcBuf++);
        *(puDestBuf++) = (char)((u4Pixel & 0x03FF) >> 2);
        *(puDestBuf++) = (char)(((u4Pixel >> 10) & 0x03FF) >> 2);
        *(puDestBuf++) = (char)(((u4Pixel >> 20) & 0x03FF) >> 2);
    }


    return TRUE;
}


/////////////////////////////////////////////////////////////////////////
//
//   mrSaveRAWImg () -
//!
//!  brief for geneirc function to save image file
//
/////////////////////////////////////////////////////////////////////////
BOOL bSaveRAWImg(char *a_pBuf,  MUINT32 a_u4Width, MUINT32 a_u4Height, MUINT32 a_u4Size, UINT8 a_uBitDepth, UINT8 a_uBayerStart)
{
    char szFileName[256];

    UINT8 uBayerStart = 0;

    //convert the bayerstart, //TODO
    switch (a_uBayerStart)
    {
        case 0xB4:
            uBayerStart = 3;
    	    break;
    }

    sprintf(szFileName, "%s//%04d_%s_%dx%d_%d_%d.raw" , MEDIA_PATH,
                                                          g_u4ImgCnt,
                                                          PROJECT_NAME,
                                                          a_u4Width,
                                                          a_u4Height,
                                                          a_uBitDepth,
                                                          uBayerStart);

    MUINT32 *pu4SrcBuf = (MUINT32 *) a_pBuf;
    char *pucBuf = (char *) malloc (a_u4Width * a_u4Height  * 2 * sizeof(char));

    UINT16 *pu2DestBuf = (UINT16 *)pucBuf;

    while (pu2DestBuf < (UINT16 *)pucBuf + a_u4Width * a_u4Height)
    {
        MUINT32 u4Pixel = *(pu4SrcBuf++);
        *(pu2DestBuf++) = (UINT16)(u4Pixel & 0x03FF);
        *(pu2DestBuf++) = (UINT16)((u4Pixel >> 10) & 0x03FF);
        *(pu2DestBuf++) = (UINT16)((u4Pixel >> 20) & 0x03FF);
    }


    FILE *pFp = fopen(szFileName, "wb");

    if (NULL == pFp )
    {
        ACDK_LOGE("Can't open file to save Image\n");
        return FALSE;
    }

    MINT32 i4WriteCnt = fwrite(pucBuf, 1, a_u4Width * a_u4Height  * 2  , pFp);

    ACDK_LOGD("Save image file name:%s\n", szFileName);

    fclose(pFp);
    sync();
    free(pucBuf);
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////
//
//   mrSaveJPEGImg () -
//!
//!  brief for geneirc function to save image file
//
/////////////////////////////////////////////////////////////////////////
BOOL bSaveJPEGImg(char *a_pBuf,  MUINT32 a_u4Size)
{
    char szFileName[256];

    sprintf(szFileName, "%s//%04d_%s.jpg" , MEDIA_PATH, g_u4ImgCnt, PROJECT_NAME);

    FILE *pFp = fopen(szFileName, "wb");

    if (NULL == pFp )
    {
        ACDK_LOGE("Can't open file to save Image\n");
        return FALSE;
    }

	MINT32 i4WriteCnt = fwrite(a_pBuf, 1, a_u4Size , pFp);

    ACDK_LOGD("Save image file name:%s\n", szFileName);

    fclose(pFp);
    sync();
    return TRUE;
}


/////////////////////////////////////////////////////////////////////////
//! CCAP CLI Test Command
//! For test CCAP -> ACDK interface
/////////////////////////////////////////////////////////////////////////
static BOOL g_bAcdkOpend = FALSE;
static bool bSendDataToACDK(MINT32   FeatureID,
						    MVOID*					pInAddr,
						    MUINT32					nInBufferSize,
                            MVOID*                  pOutAddr, 
						    MUINT32					nOutBufferSize,
						    MUINT32*				pRealOutByeCnt)
{
    ACDK_FEATURE_INFO_STRUCT rAcdkFeatureInfo;

    rAcdkFeatureInfo.puParaIn = (MUINT8*)pInAddr;
    rAcdkFeatureInfo.u4ParaInLen = nInBufferSize;
    rAcdkFeatureInfo.puParaOut = (MUINT8*)pOutAddr;
    rAcdkFeatureInfo.u4ParaOutLen = nOutBufferSize;
    rAcdkFeatureInfo.pu4RealParaOutLen = pRealOutByeCnt;


    return (MDK_IOControl(FeatureID, &rAcdkFeatureInfo));
}

static BOOL bCapDone = FALSE;


/////////////////////////////////////////////////////////////////////////
// vPrvCb
//! @brief capture callback function for ACDK to callback the capture buffer
//! @param a_pParam: the callback image buffer info
//!                              the buffer info will be
//!                              if the buffer type is JPEG, it use CapBufInfo
//!                              if the buffer type is RAW, it use RAWBufInfo
//!
/////////////////////////////////////////////////////////////////////////
static VOID vCapCb(VOID *a_pParam)
{
    ACDK_LOGD("Capture Callback \n");

    ImageBufInfo *pImgBufInfo = (ImageBufInfo *)a_pParam;

    ACDK_LOGD("Buffer Type:%d\n",  pImgBufInfo->eImgType);

    BOOL bRet = TRUE;

    if (pImgBufInfo->eImgType == RAW_TYPE)
    {
        //! currently the RAW buffer type is packed buffer
        //! The packed format is the same as MT6516 ISP format <00 Pixel1, Pixel2, Pixel3 > in 4bytes
        ACDK_LOGD("Size:%d\n", pImgBufInfo->rRawBufInfo.u4ImgSize);
        ACDK_LOGD("Width:%d\n", pImgBufInfo->rRawBufInfo.rRawImgInfo.u2Width);
        ACDK_LOGD("Height:%d\n", pImgBufInfo->rRawBufInfo.rRawImgInfo.u2Height);
        ACDK_LOGD("BitDepth:%d\n", pImgBufInfo->rRawBufInfo.rRawImgInfo.uBitDepth);
        ACDK_LOGD("Bayer Start:%d\n", pImgBufInfo->rRawBufInfo.rRawImgInfo.eColorOrder);

#if 1
        bRet = bSaveRAWImg((char *)pImgBufInfo->rRawBufInfo.pucRawBuf,
                                            pImgBufInfo->rRawBufInfo.rRawImgInfo.u2Width,
                                            pImgBufInfo->rRawBufInfo.rRawImgInfo.u2Height,
                                            pImgBufInfo->rRawBufInfo.u4ImgSize,
                                            pImgBufInfo->rRawBufInfo.rRawImgInfo.uBitDepth,
                                            pImgBufInfo->rRawBufInfo.rRawImgInfo.eColorOrder);
#else  //RAW8 Save
        UCHAR *pBuf = (UCHAR *) malloc (pImgBufInfo->rRawBufInfo.rRawImgInfo.u2Width * pImgBufInfo->rRawBufInfo.rRawImgInfo.u2Height * 1);

        bRet = bRAW10To8(pImgBufInfo->rRawBufInfo.pucRawBuf,
                                       pImgBufInfo->rRawBufInfo.rRawImgInfo.u2Width,
                                       pImgBufInfo->rRawBufInfo.rRawImgInfo.u2Height,
                                       pImgBufInfo->rRawBufInfo.u4ImgSize,
                                       pImgBufInfo->rRawBufInfo.rRawImgInfo.uBitDepth,
                                       pBuf);
        FILE *pFp = fopen("/data/test8.raw", "wb");

        if (NULL == pFp )
        {
            ACDK_LOGE("Can't open file to save Image\n");
        }

        MINT32 i4WriteCnt = fwrite(pBuf, 1, pImgBufInfo->rRawBufInfo.rRawImgInfo.u2Width * pImgBufInfo->rRawBufInfo.rRawImgInfo.u2Height  * 1  , pFp);

        fclose(pFp);
        sync();
        free(pBuf);
#endif

    }
    else if (pImgBufInfo->eImgType == JPEG_TYPE)
   {
        ACDK_LOGD("Size:%d\n", pImgBufInfo->rCapBufInfo.u4ImgSize);
        ACDK_LOGD("Width:%d\n", pImgBufInfo->rCapBufInfo.u2ImgXRes);
        ACDK_LOGD("Height:%d\n", pImgBufInfo->rCapBufInfo.u2ImgYRes)

        bRet = bSaveJPEGImg((char *)pImgBufInfo->rCapBufInfo.pucImgBuf,
                                             pImgBufInfo->rCapBufInfo.u4ImgSize);
   }
   else
   {
        ACDK_LOGD("UnKnow Format \n");
   }

    bCapDone = TRUE;
    g_u4ImgCnt ++;
}

/////////////////////////////////////////////////////////////////////////
// vPrvCb
//! @brief preview callback function for ACDK to callback the preview buffer
//! @param a_pParam: the callback image buffer info
//!                              the buffer info will be PrvVDOBufInfo structure
//!
/////////////////////////////////////////////////////////////////////////
static VOID vPrvCb(VOID *a_pParam)
{
    //ACDK_LOGD("Preview Callback \n");

    ImageBufInfo *pImgBufInfo = (ImageBufInfo *)a_pParam;

    //ACDK_LOGD("Buffer Type:%d\n",  pImgBufInfo->eImgType);
    //ACDK_LOGD("Size:%d\n", pImgBufInfo->rPrvVDOBufInfo.u4ImgSize);
    //ACDK_LOGD("Width:%d\n", pImgBufInfo->rPrvVDOBufInfo.u2ImgXRes);
    //ACDK_LOGD("Height:%d\n", pImgBufInfo->rPrvVDOBufInfo.u2ImgYRes);
}




/////////////////////////////////////////////////////////////////////////
// FT_CCT_OP_PREVIEW_LCD_START
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPPreviewStart_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{

    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("FT_CCT_OP_PREVIEW_LCD_START\n");

    ACDK_CCT_CAMERA_PREVIEW_STRUCT rCCTPreviewConfig;

    rCCTPreviewConfig.fpPrvCB = vPrvCb;
    rCCTPreviewConfig.u2PreviewWidth = 320;
    rCCTPreviewConfig.u2PreviewHeight = 240;

    MUINT32 u4RetLen = 0;


    BOOL bRet = bSendDataToACDK (ACDK_CCT_OP_PREVIEW_LCD_START, (UINT8 *)&rCCTPreviewConfig,
                                                                                                                sizeof(ACDK_CCT_CAMERA_PREVIEW_STRUCT),
                                                                                                                NULL,
                                                                                                                0,
                                                                                                                &u4RetLen);
    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;

}

/////////////////////////////////////////////////////////////////////////
// FT_CCT_OP_PREVIEW_LCD_STOP
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPPreviewStop_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("FT_CCT_OP_PREVIEW_LCD_STOP\n");

    MUINT32 u4RetLen = 0;
    BOOL bRet = bSendDataToACDK(ACDK_CCT_OP_PREVIEW_LCD_STOP, NULL, 0, NULL, 0, &u4RetLen);

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// FT_CCT_OP_SINGLE_SHOT_CAPTURE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSingleShot_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{

    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("FT_CCT_OP_SINGLE_SHOT_CAPTURE\n");
    if (a_u4Argc != 2 && a_u4Argc != 4)
    {
        ACDK_LOGD("Usage: cap <mode, prv:0, cap:1> <format, 1:raw, 0:jpg> <width (Option)> <height (Option)>\n");
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_CCT_STILL_CAPTURE_STRUCT rCCTStillCapConfig;

    rCCTStillCapConfig.eCameraMode = (eCAMERA_MODE)atoi((char *)a_pprArgv[0]);
    if (atoi((char *)a_pprArgv[1]) == 0)
    {
        rCCTStillCapConfig.eOutputFormat = OUTPUT_JPEG;
    }
    else
    {
        rCCTStillCapConfig.eOutputFormat = OUTPUT_EXT_RAW_10BITS;
    }

    if  (a_u4Argc == 4)
    {
        rCCTStillCapConfig.u2JPEGEncWidth = atoi((char *)a_pprArgv[2]);
        rCCTStillCapConfig.u2JPEGEncHeight = atoi((char *)a_pprArgv[3]);
    }
    else
    {
        rCCTStillCapConfig.u2JPEGEncWidth = 0;
        rCCTStillCapConfig.u2JPEGEncHeight = 0;
    }
    rCCTStillCapConfig.fpCapCB = vCapCb;
    MUINT32 u4RetLen = 0;

    bCapDone = FALSE;
    BOOL bRet = bSendDataToACDK(ACDK_CCT_OP_SINGLE_SHOT_CAPTURE_EX, (UINT8 *)&rCCTStillCapConfig,
                                                                                                                     sizeof(ACDK_CCT_STILL_CAPTURE_STRUCT),
                                                                                                                     NULL,
                                                                                                                     0,
                                                                                                                     &u4RetLen);

    //wait JPEG Done;
    while (!bCapDone)
    {
        usleep(1000);
    }


    ACDK_LOGD("FT_CCT_OP_PREVIEW_LCD_START\n");

    ACDK_CCT_CAMERA_PREVIEW_STRUCT rCCTPreviewConfig;

    rCCTPreviewConfig.fpPrvCB = vPrvCb;
    rCCTPreviewConfig.u2PreviewWidth = 320;
    rCCTPreviewConfig.u2PreviewHeight = 240;

    bRet = bSendDataToACDK (ACDK_CCT_OP_PREVIEW_LCD_START, (UINT8 *)&rCCTPreviewConfig,
                                                                                                                sizeof(ACDK_CCT_CAMERA_PREVIEW_STRUCT),
                                                                                                                NULL,
                                                                                                                0,
                                                                                                                &u4RetLen);
    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// FT_CCT_OP_MULTI_SHOT_CAPTURE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPMultiShot_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("FT_CCT_OP_MULTI_SHOT_CAPTURE\n");

    if (a_u4Argc != 3)
    {
        ACDK_LOGD("Usage: cap <mode, prv:0, cap:1> <format, 1:raw, 0:jpg> <count> \n");
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_CCT_MULTI_SHOT_CAPTURE_STRUCT rCCTMutiShotConfig;

    rCCTMutiShotConfig.eCameraMode = (eCAMERA_MODE)atoi((char *)a_pprArgv[0]);
    if (atoi((char *)a_pprArgv[1]) == 0)
    {
        rCCTMutiShotConfig.eOutputFormat = OUTPUT_JPEG;
    }
    else
    {
        rCCTMutiShotConfig.eOutputFormat = OUTPUT_EXT_RAW_10BITS;
    }
    rCCTMutiShotConfig.u2JPEGEncWidth = 2560;
    rCCTMutiShotConfig.u2JPEGEncHeight = 1920;
    rCCTMutiShotConfig.fpCapCB = vCapCb;
    rCCTMutiShotConfig.u4CapCount = atoi((char *)a_pprArgv[2]);
    MUINT32 u4RetLen = 0;

    bCapDone = FALSE;
    BOOL bRet = bSendDataToACDK(ACDK_CCT_OP_MULTI_SHOT_CAPTURE_EX, (UINT8 *)&rCCTMutiShotConfig,
                                                                                                                     sizeof(ACDK_CCT_MULTI_SHOT_CAPTURE_STRUCT),
                                                                                                                     NULL,
                                                                                                                     0,
                                                                                                                     &u4RetLen);

    //wait JPEG Done;
    while (!bCapDone)
    {
        usleep(1000);
    }

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_LOAD_FROM_NVRAM
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPLoadFromNvram_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_LOAD_FROM_NVRAM\n");

    BOOL bRet = bSendDataToACDK(ACDK_CCT_OP_LOAD_FROM_NVRAM, NULL, 0, NULL, 0, NULL);

    if (!bRet)
    {
        ACDK_LOGE("ACDK_CCT_OP_LOAD_FROM_NVRAM Fail\n");
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_SAVE_TO_NVRAM
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSaveToNvram_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_SAVE_TO_NVRAM\n");

    BOOL bRet = bSendDataToACDK(ACDK_CCT_OP_SAVE_TO_NVRAM, NULL, 0, NULL, 0, NULL);

    if (!bRet)
    {
        ACDK_LOGE("ACDK_CCT_OP_SAVE_TO_NVRAM Fail\n");
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_ISP_LOAD_FROM_NVRAM
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPISPLoadFromNvram_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_ISP_LOAD_FROM_NVRAM\n");

    BOOL bRet = ::bSendDataToACDK(ACDK_CCT_OP_ISP_LOAD_FROM_NVRAM, NULL, 0, NULL, 0, NULL);

    if (!bRet)
    {
        ACDK_LOGE("ACDK_CCT_OP_ISP_LOAD_FROM_NVRAM Fail\n");
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_ISP_SAVE_TO_NVRAM
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPISPSaveToNvram_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_ISP_SAVE_TO_NVRAM\n");

    BOOL bRet = ::bSendDataToACDK(ACDK_CCT_OP_ISP_SAVE_TO_NVRAM, NULL, 0, NULL, 0, NULL);

    if (!bRet)
    {
        ACDK_LOGE("ACDK_CCT_OP_ISP_SAVE_TO_NVRAM Fail\n");
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_ISP_READ_REG
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPReadISPReg_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: rIspReg <addr>\n");
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_CCT_REG_RW_STRUCT ACDK_reg_read;
    memset(&ACDK_reg_read,0, sizeof(ACDK_CCT_REG_RW_STRUCT));

    MUINT32 u4InRegAddr = 0;

    ACDK_LOGD("ACDK_CCT_OP_ISP_READ_REG\n");

    getHexToken((char *)a_pprArgv[0], &u4InRegAddr);

    ACDK_reg_read.RegAddr	= u4InRegAddr;

    MUINT32 u4RetLen = 0;

    BOOL bRet = bSendDataToACDK(ACDK_CCT_OP_ISP_READ_REG,
                                (UINT8 *)&ACDK_reg_read,
                                sizeof(ACDK_CCT_REG_RW_STRUCT),
                                (UINT8 *)&ACDK_reg_read,
                                sizeof(ACDK_CCT_REG_RW_STRUCT),
                                &u4RetLen);

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    ACDK_LOGD("Read Addr:0x%X\n", ACDK_reg_read.RegAddr);
    ACDK_LOGD("Read Data:0x%X\n", ACDK_reg_read.RegData);

    return S_ACDK_CCAP_OK;

}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_ISP_WRITE_REG
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPWriteISPReg_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (a_u4Argc != 2)
    {
        ACDK_LOGD("Usage: wIspReg <addr> <data>\n");
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_CCT_REG_RW_STRUCT ACDK_reg_write;
    memset(&ACDK_reg_write,0, sizeof(ACDK_CCT_REG_RW_STRUCT));

    MUINT32 u4InRegAddr = 0;
    MUINT32 u4InRegData = 0;

    ACDK_LOGD("ACDK_CCT_OP_ISP_WRITE_REG\n");

    getHexToken((char *)a_pprArgv[0], &u4InRegAddr);
    getHexToken((char *)a_pprArgv[1], &u4InRegData);


    ACDK_reg_write.RegAddr	= u4InRegAddr;
    ACDK_reg_write.RegData = u4InRegData;

    MUINT32 u4RetLen = 0;

    BOOL bRet = bSendDataToACDK(ACDK_CCT_OP_ISP_WRITE_REG,
                                (UINT8 *)&ACDK_reg_write,
                                sizeof(ACDK_CCT_REG_RW_STRUCT),
                                (UINT8 *)&ACDK_reg_write,
                                sizeof(ACDK_CCT_REG_RW_STRUCT),
                                &u4RetLen);

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    ACDK_LOGD("Write Addr:0x%X\n", ACDK_reg_write.RegAddr);
    ACDK_LOGD("Write Data:0x%X\n", ACDK_reg_write.RegData);

    return S_ACDK_CCAP_OK;

}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_QUERY_ISP_ID
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPQueryISPID_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_QUERY_ISP_ID\n");

    MUINT32 u4ID = 0;
    MUINT32 u4RetLen = 0;

    BOOL bRet = bSendDataToACDK(ACDK_CCT_OP_QUERY_ISP_ID,
                                NULL,
                                0,
                                (UINT8*)&u4ID,
                                4,
                                &u4RetLen);

    ACDK_LOGD("ISP ID:0x%X\n", u4ID);

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_ISP_SET_TUNING_INDEX
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetTuningIndex_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_ISP_SET_TUNING_INDEX\n");

    ACDK_CCT_ISP_REG_CATEGORY const aryCategory[EIsp_Num_Of_Category] = 
    {
        EIsp_Category_OB, 
        EIsp_Category_DM, 
        EIsp_Category_DP, 
        EIsp_Category_NR1, 
        EIsp_Category_NR2, 
        EIsp_Category_EE, 
        EIsp_Category_Saturation, 
        EIsp_Category_Contrast, 
        EIsp_Category_Hue
    };

    MUINT32 const aryIndex[EIsp_Num_Of_Category] =
    {
        0, 0, 1, 2, 3, 3, 2, 1, 0
    };


    ACDK_CCT_ISP_ACCESS_NVRAM_REG_INDEX acdk_cct_reg_idx;
    ::memset(&acdk_cct_reg_idx, 0, sizeof(acdk_cct_reg_idx));

    for (MUINT32 i = 0; i < EIsp_Num_Of_Category; i++)
    {
        acdk_cct_reg_idx.u4Index    = aryIndex[i];
        acdk_cct_reg_idx.eCategory  = aryCategory[i];
        MBOOL bRet = ::bSendDataToACDK  (
                ACDK_CCT_V2_OP_ISP_SET_TUNING_INDEX, 
                (MUINT8*)&acdk_cct_reg_idx, sizeof(ACDK_CCT_ISP_ACCESS_NVRAM_REG_INDEX), 
                NULL, 0, NULL
            );
        if  ( ! bRet )
        {
            return E_ACDK_CCAP_API_FAIL;
        }
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_ISP_GET_TUNING_INDEX
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetTuningIndex_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_ISP_GET_TUNING_INDEX\n");

    ACDK_CCT_ISP_REG_CATEGORY const aryCategory[EIsp_Num_Of_Category] = 
    {
        EIsp_Category_OB, 
        EIsp_Category_DM, 
        EIsp_Category_DP, 
        EIsp_Category_NR1, 
        EIsp_Category_NR2, 
        EIsp_Category_EE, 
        EIsp_Category_Saturation, 
        EIsp_Category_Contrast, 
        EIsp_Category_Hue
    };

    MUINT32 u4RetLen = 0;
    MUINT32 u4Index = 0xFFFFFFFF;

    for (MUINT32 i = 0; i < EIsp_Num_Of_Category; i++)
    {
        ACDK_CCT_ISP_REG_CATEGORY eCategory = aryCategory[i];
        u4Index = 0xFFFFFFFF;
        MBOOL bRet = ::bSendDataToACDK  (
                ACDK_CCT_V2_OP_ISP_GET_TUNING_INDEX, 
                &eCategory, sizeof(eCategory), 
                &u4Index, sizeof(u4Index), 
                &u4RetLen
            );

        printf("Get: (bRet, u4RetLen)=(%d, %d) (Category, Index)=(%d, %d)\n", bRet, u4RetLen, eCategory, u4Index);
        if  ( ! bRet )
        {
            return E_ACDK_CCAP_API_FAIL;
        }
    }

    return S_ACDK_CCAP_OK;
}

#if 0
static WINMO_CCT_ISP_TUNING_CMD g_WinMo_CCT_Test_Para =
{
    //NR1
    {
        {1, 8, 4, 4, 120, 6, 9, 14, 1, 2, 3, 4, 5, 6, 7, 8, 2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10, 4, 5, 6, 7, 8, 9, 10, 11, 1, 2, 7, 9},
        {0, 4, 3, 1, 121, 3, 4, 5, 2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10, 4, 5, 6, 7, 8, 9, 10, 11, 5, 6, 7, 8, 9, 10, 11, 12, 0, 1, 10, 22},
        {1, 5, 1, 2, 123, 4, 5, 6, 1, 2, 3, 4, 5, 6, 7, 8, 2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 10, 28},
        {0, 3, 2, 3, 124, 5, 6, 7, 2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10, 4, 5, 6, 7, 8, 9, 10, 11, 5, 6, 7, 8, 9, 10, 11, 12, 0, 1, 11, 27},
        {0, 2, 3, 2, 125, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8, 2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10, 4, 5, 6, 7, 8, 9, 10, 11, 0, 0, 12, 26},
        {0, 1, 2, 1, 126, 7, 8, 9, 2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10, 4, 5, 6, 7, 8, 9, 10, 11, 5, 6, 7, 8, 9, 10, 11, 12, 1, 0, 13, 25},
        {1, 0, 1, 4, 127, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10, 4, 5, 6, 7, 8, 9, 10, 11, 1, 1, 14, 24},
    }
};
#endif


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_ISP_GET_TUNING_PARAS
/////////////////////////////////////////////////////////////////////////
template <class T, MUINT32 n>
void Dump_ACDK_CCT_ISP_NVRAM_REG(T (& rParams)[n], char const*const pcszTitle)
{
    printf("< %s > \n", pcszTitle);
    for (MUINT32 i = 0; i < n; i++)
    {
        printf(" [%d] \n", i);

        T& rReg = rParams[i];
        for (MUINT32 iSet = 0; iSet < sizeof(rReg.set)/sizeof(MUINT32); iSet++)
        {
            printf("  set[%d]:\t 0x%08X\n", iSet, rReg.set[iSet]);
        }
    }
}

MRESULT mrCCAPGetTuningParas_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_ISP_GET_TUNING_PARAS\n");

    MUINT32 u4RetLen = 0;
    ACDK_CCT_ISP_GET_TUNING_PARAS   param;
    ::memset(&param, 0, sizeof(param));

    MBOOL bRet = ::bSendDataToACDK(
        ACDK_CCT_V2_OP_ISP_GET_TUNING_PARAS,
        NULL, 0, 
        (MUINT8*)&param, sizeof(param),
        &u4RetLen
    );


    ACDK_LOGD("(bRet, u4RetLen, OutBufSize)=(%d, %d, %d)\n", bRet, u4RetLen, sizeof(param));
    ACDK_CCT_ISP_NVRAM_REG& rRegs = param.stIspNvramRegs;

    Dump_ACDK_CCT_ISP_NVRAM_REG(rRegs.OB,           "OB");
    Dump_ACDK_CCT_ISP_NVRAM_REG(rRegs.DM,           "DM");
    Dump_ACDK_CCT_ISP_NVRAM_REG(rRegs.DP,           "DP");
    Dump_ACDK_CCT_ISP_NVRAM_REG(rRegs.NR1,          "NR1");
    Dump_ACDK_CCT_ISP_NVRAM_REG(rRegs.NR2,          "NR2");
    Dump_ACDK_CCT_ISP_NVRAM_REG(rRegs.EE,           "EE");
    Dump_ACDK_CCT_ISP_NVRAM_REG(rRegs.Saturation,   "Saturation");
    Dump_ACDK_CCT_ISP_NVRAM_REG(rRegs.Contrast,     "Contrast");
    Dump_ACDK_CCT_ISP_NVRAM_REG(rRegs.Hue,          "Hue");

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_ISP_SET_TUNING_PARAS
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetTuningParas_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_ISP_SET_TUNING_PARAS\n");

    ACDK_CCT_ISP_SET_TUNING_PARAS   param;
    ::memset(&param, 0xFF, sizeof(param));  //  Set all to 0xFFFFFFFF

    MUINT32 u4Category = 0;
    MUINT32 u4Index = 0;
    
    if  ( a_u4Argc < 2 )
    {
        printf("a_u4Argc(%d) < 2: (please give \"category\" & \"index\")  \n", a_u4Argc);
        return E_ACDK_IF_API_FAIL;
    }

    ::sscanf((char*)a_pprArgv[0], "%d", &u4Category);
    ::sscanf((char*)a_pprArgv[1], "%d", &u4Index);

    printf("(u4Category, u4Index) = (%d, %d) \n", u4Category, u4Index);
    if  ( EIsp_Num_Of_Category <= u4Category )
        return E_ACDK_CCAP_API_FAIL;

    param.u4Index   = u4Index;
    param.eCategory = (ACDK_CCT_ISP_REG_CATEGORY)u4Category;

    MBOOL bRet = ::bSendDataToACDK(
        ACDK_CCT_V2_OP_ISP_SET_TUNING_PARAS,
        (MUINT8*)&param, sizeof(param),
        NULL, 0, NULL
    );

    if (!bRet)
    {
        printf("Fail\n");
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_OP_ISP_SET_SHADING_ON_OFF
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetShadingOnOff_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("FT_MSDK_CCT_V2_OP_ISP_SET_SHADING_ON_OFF\n");
    if (a_u4Argc != 2)
    {
        ACDK_LOGD("Usage: setShading <Mode: prv:0, cap:1, bin:2 > <On/Off, ON:1, OFF:0>\n");
        return E_ACDK_IF_API_FAIL;
    }


    ACDK_CCT_MODULE_CTRL_STRUCT ACDK_Data;
    memset(&ACDK_Data,0,sizeof(ACDK_CCT_MODULE_CTRL_STRUCT));

    ACDK_Data.Mode   = (CAMERA_TUNING_SET_ENUM)atoi((char *)a_pprArgv[0]);
    ACDK_Data.Enable = atoi((char *)a_pprArgv[1]);

    MUINT32 u4RetLen = 0;

    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_ISP_SET_SHADING_ON_OFF,
                                                       (UINT8 *)&ACDK_Data,
                                                       sizeof(ACDK_CCT_MODULE_CTRL_STRUCT),
                                                       NULL,
                                                       0,
                                                       &u4RetLen);

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    ACDK_LOGD("CCAP Set Shading On/Off Success, Mode:%d    On/Off:%d\n", ACDK_Data.Mode,  ACDK_Data.Enable);
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_OP_ISP_GET_SHADING_ON_OFF
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetShadingOnOff_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("FT_MSDK_CCT_V2_OP_ISP_GET_SHADING_ON_OFF\n");
    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: getShading <Mode: prv:0, cap:1, bin:2 >\n");
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_CCT_MODULE_CTRL_STRUCT ACDK_Data;
    memset(&ACDK_Data,0,sizeof(ACDK_CCT_MODULE_CTRL_STRUCT));

    ACDK_Data.Mode = (CAMERA_TUNING_SET_ENUM)atoi((char *)a_pprArgv[0]);

    MUINT32 u4RetLen = 0;

    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_ISP_GET_SHADING_ON_OFF,
                                                       (UINT8 *)&ACDK_Data,
                                                       sizeof(ACDK_CCT_MODULE_CTRL_STRUCT),
                                                       (UINT8 *)&ACDK_Data,
                                                       sizeof(ACDK_CCT_MODULE_CTRL_STRUCT),
                                                       &u4RetLen);

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    ACDK_LOGD("CCAP Get Shading On/Off Success, Mode:%d    On/Off:%d\n", ACDK_Data.Mode,  ACDK_Data.Enable);
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_OP_ISP_SET_SHADING_PARA
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetShadingPara_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("FT_MSDK_CCT_V2_OP_ISP_SET_SHADING_PARA\n");
    ACDK_CCT_SHADING_COMP_STRUCT ACDK_data;
    memset(&ACDK_data,0,sizeof(ACDK_CCT_SHADING_COMP_STRUCT));
    ACDK_data.SHADING_MODE = 0;

    winmo_cct_shading_comp_struct Cct_shading;
    memset(&Cct_shading,0,sizeof(winmo_cct_shading_comp_struct)); 
    Cct_shading.SDBLK_TRIG = 0;
    Cct_shading.SHADING_EN = 1;
    Cct_shading.SHADINGBLK_XNUM = 7;
    Cct_shading.SHADINGBLK_YNUM = 6;
    Cct_shading.SD_TABLE_SIZE = 1024;
    Cct_shading.SDBLK_RATIO00 = 32;
    Cct_shading.SDBLK_RATIO01 = 32;
    Cct_shading.SDBLK_RATIO10 = 32;
    Cct_shading.SDBLK_RATIO11 = 32;
    Cct_shading.SHADINGBLK_WIDTH = 92;
    Cct_shading.SHADINGBLK_HEIGHT = 81;
    Cct_shading.SD_LWIDTH = 87;
    Cct_shading.SD_LHEIGHT = 72;    
    
    //memcpy(&Cct_shading,(const cct_shading_comp_struct *)&pREQ->cmd.set_shading_para,sizeof(cct_shading_comp_struct));

    ACDK_data.pShadingComp = &Cct_shading;
    MUINT32 u4RetLen = 0;

    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_ISP_SET_SHADING_PARA,
                                                       (UINT8 *)&ACDK_data,
                                                       sizeof(ACDK_CCT_SHADING_COMP_STRUCT),
                                                       NULL,
                                                       0,
                                                       &u4RetLen);
    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_OP_ISP_GET_SHADING_PARA
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetShadingPara_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("FT_MSDK_CCT_V2_OP_ISP_GET_SHADING_PARA\n");
    winmo_cct_shading_comp_struct CCT_shading;
    memset(&CCT_shading,0,sizeof(winmo_cct_shading_comp_struct));

    //CAMERA_TUNING_SET_ENUM
    ACDK_CCT_SHADING_COMP_STRUCT ACDK_Output;
    memset(&ACDK_Output,0,sizeof(ACDK_CCT_SHADING_COMP_STRUCT));

    UINT8 uCompMode = CAMERA_TUNING_PREVIEW_SET;
    ACDK_Output.pShadingComp = &CCT_shading;
    MUINT32 u4RetLen = 0;
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_ISP_GET_SHADING_PARA,
                                                       (UINT8*) &uCompMode,
                                                       sizeof(UINT8),
                                                       (UINT8*)&ACDK_Output,
                                                       sizeof(ACDK_CCT_SHADING_COMP_STRUCT),
                                                       &u4RetLen);

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    ACDK_LOGD("CAMERA_TUNING_PREVIEW_SET\n");
    ACDK_LOGD("SDBLK_TRIG:%d\n", CCT_shading.SDBLK_TRIG);
    ACDK_LOGD("SHADING_EN:%d\n", CCT_shading.SHADING_EN);
    ACDK_LOGD("SHADINGBLK_XOFFSET:%d\n", CCT_shading.SHADINGBLK_XOFFSET);
    ACDK_LOGD("SHADINGBLK_YOFFSET:%d\n", CCT_shading.SHADINGBLK_YOFFSET);
    ACDK_LOGD("SHADINGBLK_XNUM:%d\n", CCT_shading.SHADINGBLK_XNUM);
    ACDK_LOGD("SHADINGBLK_YNUM:%d\n", CCT_shading.SHADINGBLK_YNUM);
    ACDK_LOGD("SHADINGBLK_WIDTH:%d\n", CCT_shading.SHADINGBLK_WIDTH);
    ACDK_LOGD("SHADINGBLK_HEIGHT:%d\n", CCT_shading.SHADINGBLK_HEIGHT);
    ACDK_LOGD("SHADING_RADDR:%d\n", CCT_shading.SHADING_RADDR);
    ACDK_LOGD("SD_LWIDTH:%d\n", CCT_shading.SD_LWIDTH);
    ACDK_LOGD("SD_LHEIGHT:%d\n", CCT_shading.SD_LHEIGHT);
    ACDK_LOGD("SDBLK_RATIO00:%d\n", CCT_shading.SDBLK_RATIO00);
    ACDK_LOGD("SDBLK_RATIO01:%d\n", CCT_shading.SDBLK_RATIO01);
    ACDK_LOGD("SDBLK_RATIO10:%d\n", CCT_shading.SDBLK_RATIO10);
    ACDK_LOGD("SDBLK_RATIO11:%d\n", CCT_shading.SDBLK_RATIO11);
    ACDK_LOGD("SD_TABLE_SIZE:%d\n",  CCT_shading.SD_TABLE_SIZE);

    uCompMode = CAMERA_TUNING_CAPTURE_SET;
    bRet = bSendDataToACDK(ACDK_CCT_V2_OP_ISP_GET_SHADING_PARA,
                                                       (UINT8*) &uCompMode,
                                                       sizeof(UINT8),
                                                       (UINT8*)&ACDK_Output,
                                                       sizeof(ACDK_CCT_SHADING_COMP_STRUCT),
                                                       &u4RetLen);

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    ACDK_LOGD("CAMERA_TUNING_CAPTURE_SET\n");
    ACDK_LOGD("SDBLK_TRIG:%d\n", CCT_shading.SDBLK_TRIG);
    ACDK_LOGD("SHADING_EN:%d\n", CCT_shading.SHADING_EN);
    ACDK_LOGD("SHADINGBLK_XOFFSET:%d\n", CCT_shading.SHADINGBLK_XOFFSET);
    ACDK_LOGD("SHADINGBLK_YOFFSET:%d\n", CCT_shading.SHADINGBLK_YOFFSET);
    ACDK_LOGD("SHADINGBLK_XNUM:%d\n", CCT_shading.SHADINGBLK_XNUM);
    ACDK_LOGD("SHADINGBLK_YNUM:%d\n", CCT_shading.SHADINGBLK_YNUM);
    ACDK_LOGD("SHADINGBLK_WIDTH:%d\n", CCT_shading.SHADINGBLK_WIDTH);
    ACDK_LOGD("SHADINGBLK_HEIGHT:%d\n", CCT_shading.SHADINGBLK_HEIGHT);
    ACDK_LOGD("SHADING_RADDR:%d\n", CCT_shading.SHADING_RADDR);
    ACDK_LOGD("SD_LWIDTH:%d\n", CCT_shading.SD_LWIDTH);
    ACDK_LOGD("SD_LHEIGHT:%d\n", CCT_shading.SD_LHEIGHT);
    ACDK_LOGD("SDBLK_RATIO00:%d\n", CCT_shading.SDBLK_RATIO00);
    ACDK_LOGD("SDBLK_RATIO01:%d\n", CCT_shading.SDBLK_RATIO01);
    ACDK_LOGD("SDBLK_RATIO10:%d\n", CCT_shading.SDBLK_RATIO10);
    ACDK_LOGD("SDBLK_RATIO11:%d\n", CCT_shading.SDBLK_RATIO11);
    ACDK_LOGD("SD_TABLE_SIZE:%d\n",  CCT_shading.SD_TABLE_SIZE);


    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_ISP_DEFECT_TABLE_ON
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPDefectTblOn_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("FT_MSDK_CCT_V2_ISP_DEFECT_TABLE_ON\n");

    ACDK_CCT_MODULE_CTRL_STRUCT ACDK_MODULE_ctrl_struct;
    memset(&ACDK_MODULE_ctrl_struct,0,sizeof(ACDK_CCT_MODULE_CTRL_STRUCT));

    ACDK_MODULE_ctrl_struct.Enable = TRUE;
    MUINT32 u4RetLen = 0;

    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_ISP_DEFECT_TABLE_ON,
                                                       (UINT8 *)&ACDK_MODULE_ctrl_struct,
                                                       sizeof(ACDK_CCT_MODULE_CTRL_STRUCT),
                                                       NULL,
                                                       0,
                                                       &u4RetLen);
    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_ISP_DEFECT_TABLE_OFF
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPDefectTblOff_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("FT_MSDK_CCT_V2_ISP_DEFECT_TABLE_OFF\n");

    ACDK_CCT_MODULE_CTRL_STRUCT ACDK_MODULE_ctrl_struct;
    memset(&ACDK_MODULE_ctrl_struct,0,sizeof(ACDK_CCT_MODULE_CTRL_STRUCT));

    ACDK_MODULE_ctrl_struct.Enable = FALSE;
    MUINT32 u4RetLen = 0;

    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_ISP_DEFECT_TABLE_OFF,
                                                       (UINT8 *)&ACDK_MODULE_ctrl_struct,
                                                       sizeof(ACDK_CCT_MODULE_CTRL_STRUCT),
                                                       NULL,
                                                       0,
                                                       &u4RetLen);
    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_OP_DEFECT_TABLE_CAL
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPDefectTblCal_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }
    ACDK_LOGD("FT_MSDK_CCT_OP_DEFECT_TABLE_CAL\n");
    //ToDo

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_OP_ISP_ENABLE_DYNAMIC_BYPASS_MODE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPEnableDynamicBypass(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("FT_MSDK_CCT_V2_OP_ISP_ENABLE_DYNAMIC_BYPASS_MODE\n");

    MUINT32 u4RetLen = 0;
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_ISP_ENABLE_DYNAMIC_BYPASS_MODE,
                                                       NULL,
                                                       0,
                                                       NULL,
                                                       0,
                                                       &u4RetLen);

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_OP_ISP_DISABLE_DYNAMIC_BYPASS_MODE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPDisableDynamicBypass(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("FT_MSDK_CCT_V2_OP_ISP_DISABLE_DYNAMIC_BYPASS_MODE\n");
    MUINT32 u4RetLen = 0;
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_ISP_DISABLE_DYNAMIC_BYPASS_MODE,
                                                       NULL,
                                                       0,
                                                       NULL,
                                                       0,
                                                       &u4RetLen);

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    return S_ACDK_CCAP_OK;
}




/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_OP_ISP_GET_DYNAMIC_BYPASS_MODE_ON_OFF
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetDynamicMode_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("FT_MSDK_CCT_V2_OP_ISP_GET_DYNAMIC_BYPASS_MODE_ON_OFF\n");
    ACDK_CCT_FUNCTION_ENABLE_STRUCT ACDK_get_bypass_onoff;
    memset(&ACDK_get_bypass_onoff,0,sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT));

    MUINT32 u4RetLen = 0;

    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_ISP_GET_DYNAMIC_BYPASS_MODE_ON_OFF,
                                                        NULL,
                                                        0,
                                                        (UINT8 *)&ACDK_get_bypass_onoff,
                                                        sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT),
                                                        &u4RetLen);

    ACDK_LOGD("Dynamic Mode Bypass:%d\n", ACDK_get_bypass_onoff.Enable);

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    return S_ACDK_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_V3
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetShadingTableV3_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }
    ACDK_LOGD("FT_MSDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_V3\n");

    UINT8 uShadingTable[MAX_SVD_SHADING_SIZE];
    memset(uShadingTable, 0, MAX_SVD_SHADING_SIZE);

    ACDK_CCT_TABLE_SET_STRUCT  shadingTable;
    memset (&shadingTable, 0, sizeof(ACDK_CCT_TABLE_SET_STRUCT));
    shadingTable.Length = MAX_SVD_SHADING_SIZE;
    shadingTable.Offset = 0;
    shadingTable.pBuffer = (MUINT32 *)&uShadingTable[0];

    MUINT32 u4RetLen = 0;


    ACDK_LOGD("Get Shading Preview Table \n");
    for (MINT32 i = 0; i < 3; i++)
    {
        shadingTable.Mode = CAMERA_TUNING_PREVIEW_SET;
        shadingTable.ColorTemp = i;
         BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_V3,
                                                            NULL,
                                                            0,
                                                            (UINT8 *)&shadingTable,
                                                            sizeof(ACDK_CCT_TABLE_SET_STRUCT),
                                                            &u4RetLen);

        if (!bRet)
        {
            ACDK_LOGD("Get Shading Preview Table Fail, Color Temp:%d\n", i);
            return E_ACDK_CCAP_API_FAIL;
        }
        ACDK_LOGD("Get Shading Preview Table_ColorTemp:%d\n", i);
        for (MINT32 j = 0; j  < 675; j++)
        {
            printf("%02x", uShadingTable[j]);
        }
        printf("\n\n");
    }

    ACDK_LOGD("Get Shading Capture Table \n");
    for (MINT32 i = 0; i < 3; i++)
    {
        shadingTable.Mode = CAMERA_TUNING_CAPTURE_SET;
        shadingTable.ColorTemp = i;
         BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_V3,
                                                            NULL,
                                                            0,
                                                            (UINT8 *)&shadingTable,
                                                            sizeof(ACDK_CCT_TABLE_SET_STRUCT),
                                                            &u4RetLen);

        if (!bRet)
        {
            ACDK_LOGD("Get Shading Capture Table Fail, Color Temp:%d\n", i);
            return E_ACDK_CCAP_API_FAIL;
        }
        ACDK_LOGD("Get Shading Capture Table_ColorTemp:%d\n", i);
        for (MINT32 j = 0; j  < 675; j++)
        {
            printf("%02x", uShadingTable[j]);
        }
        printf("\n\n");
    }
    return S_ACDK_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetShadingTableV3_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("FT_MSDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3\n");

    // this table is for ov5642 capture , 15x15 blocks setting
    // preview should show some artifact after apply this table.
    UINT8 uShadingTable[MAX_SVD_SHADING_SIZE] ={
                0x3B, 0xF0, 0x4F, 0xF0, 0x43, 0x00, 0x59, 0x00, 
                0x07, 0x08,
                0x89, 0x00, 0x00, 0x00, 0x02, 0x3C, 0x00, 0x00, 0x00, 0x02, 
                0xEB, 0xFF, 0xFF, 0xFF, 0x02, 0xF2, 0xFF, 0xFF, 0xFF, 0x03, 
                0xED, 0xFF, 0xFF, 0xFF, 0x02, 0xF2, 0xFF, 0xFF, 0xFF, 0x03, 
                0xE5, 0xFF, 0xFF, 0xFF, 0x02, 0xF8, 0xFF, 0xFF, 0xFF, 0x03, 
                0xF2, 0xFF, 0xFF, 0xFF, 0x03, 0xF5, 0xFF, 0xFF, 0xFF, 0x03, 
                0xEA, 0xFF, 0xFF, 0xFF, 0x03, 0xF7, 0xFF, 0xFF, 0xFF, 0x03, 
                0xEF, 0xFF, 0xFF, 0xFF, 0x03, 0xF8, 0xFF, 0xFF, 0xFF, 0x03, 
                0xDE, 0xB4, 0x3D, 0xD6, 0x88, 0xDC, 0xF4, 
                0xB5, 0xAC, 0x70, 0x99, 0x08, 0xDC, 0x05, 
                0x8A, 0x96, 0x83, 0x5B, 0x5E, 0x06, 0xA0, 
                0x63, 0x6A, 0x83, 0x5F, 0xBC, 0xA0, 0x63, 
                0x42, 0x4C, 0x79, 0x54, 0xC3, 0xB7, 0x95, 
                0x27, 0x34, 0x7A, 0x6D, 0x99, 0xD6, 0x9B, 
                0x13, 0x20, 0x75, 0x57, 0x92, 0xF8, 0x79, 
                0x09, 0x12, 0x6D, 0x54, 0x62, 0xCF, 0xA0, 
                0x05, 0x01, 0x54, 0x6D, 0x4D, 0xB6, 0xA8, 
                0x02, 0x03, 0x48, 0x71, 0x42, 0xBC, 0x91, 
                0x01, 0x03, 0x33, 0x7B, 0x45, 0x99, 0x7F, 
                0x07, 0x10, 0x2C, 0x7F, 0x3A, 0x80, 0x9D, 
                0x15, 0x29, 0x2A, 0x7E, 0x4D, 0x93, 0x80, 
                0x2B, 0x3F, 0x17, 0x71, 0x74, 0x94, 0x53, 
                0x46, 0x5E, 0x0A, 0x65, 0xCE, 0xB1, 0x51, 
                0x61, 0x7C, 0x02, 0x4B, 0xCA, 0xAB, 0x7F, 
                0x70, 0xB6, 0x2C, 0x03, 0x04, 0xE6, 0xC0, 
                0x96, 0x9F, 0x9F, 0xAD, 0x73, 0x80, 0x80, 0x8D, 0x51, 0x5E, 0x5E, 0x69, 0x38, 0x43, 0x43, 0x4B, 0x23, 0x2E, 0x2E, 0x33, 0x15, 0x1D, 0x1D, 0x21, 0x0B, 0x12, 0x12, 0x15, 0x05, 0x0A, 0x0A, 0x0D, 0x03, 0x07, 0x07, 0x0A, 0x05, 0x08, 0x08, 0x0D, 0x0B, 0x0F, 0x0F, 0x14, 0x16, 0x1A, 0x1A, 0x20, 0x25, 0x29, 0x29, 0x32, 0x3A, 0x3F, 0x3E, 0x4B, 0x54, 0x58, 0x58, 0x69, 0x76, 0x7B, 0x7A, 0x90, 0x9A, 0x9C, 0x9C, 0xB7, 
                0x0E, 0x15, 0x0C, 0x6E, 0x1B, 0x24, 0x23, 0x71, 0x0F, 0x28, 0x2C, 0x7A, 0x07, 0x3A, 0x3E, 0x90, 0x1D, 0x58, 0x5D, 0xAB, 0x38, 0x7D, 0x7D, 0xC6, 0x66, 0xA5, 0xA2, 0xE9, 0x77, 0xBC, 0xBC, 0xEF, 0x84, 0xC4, 0xC5, 0xF1, 0x81, 0xC1, 0xC2, 0xF3, 0x77, 0xAF, 0xB3, 0xE1, 0x60, 0x9C, 0x9D, 0xD0, 0x5B, 0x6F, 0x75, 0xBF, 0x52, 0x61, 0x69, 0xA5, 0x55, 0x3F, 0x46, 0x7E, 0x59, 0x47, 0x44, 0x6C, 0x6F, 0x53, 0x53, 0x79, 
                0xA3, 0xB5, 0xBB, 0xB2, 0x81, 0x96, 0xA9, 0x72, 0x68, 0x80, 0x8D, 0x5A, 0x51, 0x92, 0x92, 0x5B, 0x46, 0x9C, 0x9F, 0x6D, 0x41, 0x99, 0x99, 0x77, 0x41, 0x99, 0x9A, 0x73, 0x39, 0x95, 0x98, 0x71, 0x44, 0x9B, 0xA0, 0x6A, 0x37, 0x92, 0x96, 0x64, 0x39, 0x8A, 0x88, 0x66, 0x2F, 0x80, 0x7D, 0x56, 0x16, 0x6B, 0x6D, 0x49, 0x01, 0x58, 0x69, 0x3D, 0x00, 0x61, 0x5D, 0x2E, 0x1F, 0x70, 0x77, 0x59, 0x53, 0x89, 0x87, 0x67, 
                0x17, 0x38, 0x35, 0x47, 0x09, 0x16, 0x1B, 0x30, 0x24, 0x2F, 0x29, 0x0B, 0x23, 0x20, 0x21, 0x16, 0x2D, 0x26, 0x26, 0x08, 0x31, 0x34, 0x34, 0x15, 0x3D, 0x47, 0x49, 0x19, 0x40, 0x3E, 0x3C, 0x34, 0x46, 0x5A, 0x59, 0x2D, 0x42, 0x4E, 0x4D, 0x2E, 0x48, 0x5A, 0x57, 0x31, 0x41, 0x46, 0x47, 0x13, 0x3B, 0x40, 0x3F, 0x1F, 0x48, 0x3D, 0x47, 0x02, 0x3F, 0x45, 0x45, 0x0E, 0x74, 0x4E, 0x5D, 0x3E, 0xDB, 0xBD, 0xBF, 0xB6, 
                0x9A, 0x63, 0x61, 0x3C, 0x44, 0x52, 0x44, 0x0D, 0x80, 0x52, 0x59, 0x38, 0x81, 0x59, 0x5D, 0x39, 0x79, 0x54, 0x55, 0x41, 0x7C, 0x6B, 0x6B, 0x40, 0x8E, 0x73, 0x6F, 0x5F, 0x84, 0x73, 0x79, 0x4C, 0x74, 0x79, 0x7E, 0x5D, 0x7C, 0x6E, 0x73, 0x5C, 0x86, 0x63, 0x71, 0x65, 0x6E, 0x60, 0x5C, 0x44, 0x7D, 0x4A, 0x51, 0x4F, 0x51, 0x57, 0x40, 0x2E, 0x5D, 0x4C, 0x4C, 0x35, 0x5B, 0x29, 0x4C, 0x03, 0x82, 0x48, 0x39, 0x02, 
                0x2B, 0x24, 0x35, 0x06, 0x4A, 0x4C, 0x51, 0x83, 0x4D, 0x56, 0x37, 0x4E, 0x86, 0x58, 0x58, 0x63, 0x72, 0x6A, 0x6A, 0x6A, 0x5A, 0x5A, 0x5A, 0x66, 0x51, 0x53, 0x52, 0x43, 0x66, 0x33, 0x33, 0x59, 0x42, 0x53, 0x52, 0x3B, 0x3C, 0x46, 0x45, 0x39, 0x46, 0x4B, 0x49, 0x4D, 0x1D, 0x1A, 0x1D, 0x2F, 0x33, 0x4C, 0x50, 0x5D, 0x57, 0x32, 0x39, 0x67, 0x20, 0x1F, 0x27, 0x32, 0x3D, 0x20, 0x4B, 0x10, 0x5F, 0x53, 0x5C, 0x6F, 
                0x55, 0x40, 0x3F, 0x8A, 0x2E, 0x05, 0x11, 0x25, 0x39, 0x52, 0x5A, 0x4F, 0x42, 0x4F, 0x4D, 0x4A, 0x61, 0x57, 0x5D, 0x29, 0x59, 0x3A, 0x3A, 0x4B, 0x5B, 0x29, 0x30, 0x53, 0x36, 0x3C, 0x3D, 0x2C, 0x11, 0x38, 0x34, 0x34, 0x3E, 0x35, 0x31, 0x40, 0x36, 0x35, 0x2D, 0x3F, 0x50, 0x44, 0x49, 0x62, 0x1E, 0x37, 0x33, 0x64, 0x35, 0x49, 0x3A, 0x4A, 0x49, 0x45, 0x47, 0x60, 0x06, 0x1E, 0x09, 0x19, 0x66, 0x34, 0x4A, 0x70           
        };

    ACDK_CCT_TABLE_SET_STRUCT  shadingTable;
    memset (&shadingTable, 0, sizeof(ACDK_CCT_TABLE_SET_STRUCT));
    shadingTable.Length = MAX_SVD_SHADING_SIZE;
    shadingTable.Offset = 0;
    shadingTable.pBuffer = (MUINT32 *)&uShadingTable[0];

    MUINT32 u4RetLen = 0;

    ACDK_LOGD("Set Shading Preview Table \n");
    for (MINT32 i = 0; i < 3; i++)
    {
        shadingTable.Mode = CAMERA_TUNING_PREVIEW_SET;
        shadingTable.ColorTemp = i;
         BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3,
                                                             (UINT8 *)&shadingTable,
                                                            sizeof(ACDK_CCT_TABLE_SET_STRUCT),
                                                            NULL,
                                                            0,
                                                            &u4RetLen);

        if (!bRet)
        {
            ACDK_LOGD("Set Shading Preview Table Fail, Color Temp:%d\n", i);
            return E_ACDK_CCAP_API_FAIL;
        }
    }

    ACDK_LOGD("Set Shading Capture Table \n");
    for (MINT32 i = 0; i < 3; i++)
    {
        shadingTable.Mode = CAMERA_TUNING_CAPTURE_SET;
        shadingTable.ColorTemp = i;
         BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3,
                                                             (UINT8 *)&shadingTable,
                                                            sizeof(ACDK_CCT_TABLE_SET_STRUCT),
                                                            NULL,
                                                            0,
                                                            &u4RetLen);

        if (!bRet)
        {
            ACDK_LOGD("Set Shading Capture Table Fail, Color Temp:%d\n", i);
            return E_ACDK_CCAP_API_FAIL;
        }
    }
    return S_ACDK_CCAP_OK;

}

/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_POLYCOEF
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetShadingTablePOLYCOEF_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }
    ACDK_LOGD("ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_POLYCOEF\n");

    UINT32 uShadingTable[MAX_SHADING_Capture_SIZE] = {0};

    ACDK_CCT_TABLE_SET_STRUCT  shadingTable;
    memset (&shadingTable, 0, sizeof(ACDK_CCT_TABLE_SET_STRUCT));
    shadingTable.Length = MAX_SHADING_Preview_SIZE;
    shadingTable.Offset = 0;
    shadingTable.pBuffer = (MUINT32 *)&uShadingTable[0];

    MUINT32 u4RetLen = 0;


    ACDK_LOGD("Get Shading Preview Table Poly Coef\n");
    for (MINT32 i = 0; i < 3; i++)
    {
        shadingTable.Mode = CAMERA_TUNING_PREVIEW_SET;
        shadingTable.ColorTemp = i;
         BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_POLYCOEF,
                                                            NULL,
                                                            0,
                                                            (UINT8 *)&shadingTable,
                                                            sizeof(ACDK_CCT_TABLE_SET_STRUCT),
                                                            &u4RetLen);

        if (!bRet)
        {
            ACDK_LOGD("Get Shading Preview Table Poly Coef Fail, Color Temp:%d\n", i);
            return E_ACDK_CCAP_API_FAIL;
        }
        ACDK_LOGD("Get Shading Preview Table_ColorTemp:%d\n", i);
        for (MINT32 j = 0; j  < MAX_SHADING_Preview_SIZE/8; j++)
        {
            printf("%08x", uShadingTable[j]);
        }
        printf("\n\n");
    }

    ACDK_LOGD("Get Shading Capture Table Poly Coef \n");
    shadingTable.Length = MAX_SHADING_Capture_SIZE;
    
    for (MINT32 i = 0; i < 3; i++)
    {
        shadingTable.Mode = CAMERA_TUNING_CAPTURE_SET;
        shadingTable.ColorTemp = i;
         BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_POLYCOEF,
                                                            NULL,
                                                            0,
                                                            (UINT8 *)&shadingTable,
                                                            sizeof(ACDK_CCT_TABLE_SET_STRUCT),
                                                            &u4RetLen);

        if (!bRet)
        {
            ACDK_LOGD("Get Shading Capture Table Poly Coef Fail, Color Temp:%d\n", i);
            return E_ACDK_CCAP_API_FAIL;
        }
        ACDK_LOGD("Get Shading Capture Table_ColorTemp:%d\n", i);
        for (MINT32 j = 0; j  < MAX_SHADING_Capture_SIZE/8; j++)
        {
            printf("%08x", uShadingTable[j]);
        }
        printf("\n\n");
    }
    return S_ACDK_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetShadingTablePOLYCOEF_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF\n");

    // this table is for ov5642 capture , 15x15 blocks setting
    // preview should show some artifact after apply this table.
    UINT32 uShadingTable[MAX_SHADING_Capture_SIZE] ={0};

    ACDK_CCT_TABLE_SET_STRUCT  shadingTable;
    memset (&shadingTable, 0, sizeof(ACDK_CCT_TABLE_SET_STRUCT));
    shadingTable.Length = MAX_SHADING_Preview_SIZE;
    shadingTable.Offset = 0;
    shadingTable.pBuffer = (MUINT32 *)&uShadingTable[0];

    MUINT32 u4RetLen = 0;

    ACDK_LOGD("Set Shading Preview Table Poly Coef\n");
    for (MINT32 i = 0; i < 3; i++)
    {
        shadingTable.Mode = CAMERA_TUNING_PREVIEW_SET;
        shadingTable.ColorTemp = i;
         BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF,
                                                             (UINT8 *)&shadingTable,
                                                            sizeof(ACDK_CCT_TABLE_SET_STRUCT),
                                                            NULL,
                                                            0,
                                                            &u4RetLen);

        if (!bRet)
        {
            ACDK_LOGD("Set Shading Preview Table Poly Coef Fail, Color Temp:%d\n", i);
            return E_ACDK_CCAP_API_FAIL;
        }
    }

    ACDK_LOGD("Set Shading Capture Table Poly Coef\n");
    shadingTable.Length = MAX_SHADING_Capture_SIZE;
    for (MINT32 i = 0; i < 3; i++)
    {
        shadingTable.Mode = CAMERA_TUNING_CAPTURE_SET;
        shadingTable.ColorTemp = i;
         BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF,
                                                             (UINT8 *)&shadingTable,
                                                            sizeof(ACDK_CCT_TABLE_SET_STRUCT),
                                                            NULL,
                                                            0,
                                                            &u4RetLen);

        if (!bRet)
        {
            ACDK_LOGD("Set Shading Capture Table Fail, Color Temp:%d\n", i);
            return E_ACDK_CCAP_API_FAIL;
        }
    }
    return S_ACDK_CCAP_OK;

}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_ISP_GET_SHADING_NVRAM
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetShadingNVRAM_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }
    ACDK_LOGD("ACDK_CCT_V2_OP_ISP_GET_SHADING_NVRAM\n");

    UINT32 uShadingSTR[sizeof(ISP_SHADING_STRUCT)] = {0};
    ISP_SHADING_STRUCT * pshadistr;

    ACDK_CCT_NVRAM_SET_STRUCT  ShadingData;
    memset (&ShadingData, 0, sizeof(ACDK_CCT_NVRAM_SET_STRUCT));
    ShadingData.Mode = CAMERA_NVRAM_SHADING_STRUCT;
    ShadingData.pBuffer = (MUINT32 *)&uShadingSTR[0];

    MUINT32 u4RetLen = 0;

    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_ISP_GET_NVRAM_DATA,
                                                        NULL,
                                                        0,
                                                        (UINT8 *)&ShadingData,
                                                        sizeof(ACDK_CCT_NVRAM_SET_STRUCT),
                                                        &u4RetLen);

    if (!bRet)
    {
          ACDK_LOGD("Get Shading NVRAM data Fail\n");
        return E_ACDK_CCAP_API_FAIL;
    }

    pshadistr =  reinterpret_cast<ISP_SHADING_STRUCT*> (ShadingData.pBuffer) ;

    ACDK_LOGD("Buffer size :%d\n", sizeof(ISP_SHADING_STRUCT));  
    ACDK_LOGD("PreviewSize :%d\n", pshadistr->PreviewSize);
    ACDK_LOGD("CaptureSize :%d\n", pshadistr->CaptureSize);
    ACDK_LOGD("Pre SVD Size :%d\n", pshadistr->PreviewSVDSize);
    ACDK_LOGD("Cap SVD Size :%d\n", pshadistr->CaptureSVDSize);
    ACDK_LOGD("Data Size  :%d\n", u4RetLen);
    ACDK_LOGD("NVRAM Data :%d\n", pshadistr->PreviewTable[0][0]);
    
    return S_ACDK_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_SHADING_CAL
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPCalShading_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_SHADING_CAL\n");
    if (a_u4Argc != 2)
    {
        ACDK_LOGD("Usage: calShading <mode, prv:0, cap:1> <index: 0::2900K, 1:4000K, 2:6300K>\n");
        return E_ACDK_IF_API_FAIL;
    }


    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    MUINT32 u4RetLen = 0;
    ACDK_CCT_LSC_CAL_SET_STRUCT rLSCCalSet;

    rLSCCalSet.mode = (CAMERA_TUNING_SET_ENUM)(UINT8)atoi((char *)a_pprArgv[0]);
    rLSCCalSet.colorTemp = (UINT8) atoi((char *)a_pprArgv[1]);
    rLSCCalSet.boundaryEndX = 40;
    rLSCCalSet.boundaryEndY = 40;
    rLSCCalSet.boundaryStartX = 40;
    rLSCCalSet.boundaryStartY = 40;
    rLSCCalSet.attnRatio = 1;
    rLSCCalSet.u1FixShadingIndex = 0;

    ACDK_LOGD("rLSCCalSet.mode :%d\n", rLSCCalSet.mode);
    ACDK_LOGD("rLSCCalSet.colorTemp  :%d\n", rLSCCalSet.colorTemp);

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_SHADING_CAL,
                                                        (UINT8 *)&rLSCCalSet,
                                                        sizeof(ACDK_CCT_LSC_CAL_SET_STRUCT),
                                                        NULL,
                                                        0,
                                                        &u4RetLen);

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_SHADING_VERIFY
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPVerifyShading_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_DEFECT_VERIFY
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPVerifyDefect_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_READ_SENSOR_REG
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPReadSensorReg_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: rsensorReg <addr>\n");
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_CCT_REG_RW_STRUCT ACDK_reg_read;
    memset(&ACDK_reg_read,0, sizeof(ACDK_CCT_REG_RW_STRUCT));

    MUINT32 u4InRegAddr = 0;

    ACDK_LOGD("ACDK_CCT_OP_READ_SENSOR_REG\n");

    getHexToken((char *)a_pprArgv[0], &u4InRegAddr);

    ACDK_reg_read.RegAddr	= u4InRegAddr;

    MUINT32 u4RetLen = 0;

    BOOL bRet = bSendDataToACDK(ACDK_CCT_OP_READ_SENSOR_REG,
                                (UINT8 *)&ACDK_reg_read,
                                sizeof(ACDK_CCT_REG_RW_STRUCT),
                                (UINT8 *)&ACDK_reg_read,
                                sizeof(ACDK_CCT_REG_RW_STRUCT),
                                &u4RetLen);

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    ACDK_LOGD("Read Addr:0x%X\n", ACDK_reg_read.RegAddr);
    ACDK_LOGD("Read Data:0x%X\n", ACDK_reg_read.RegData);

    return S_ACDK_CCAP_OK;

}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_WRITE_SENSOR_REG
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPWriteSensorReg_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (a_u4Argc != 2)
    {
        ACDK_LOGD("Usage: wsensorReg <addr> <data>\n");
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_CCT_REG_RW_STRUCT ACDK_reg_write;
    memset(&ACDK_reg_write,0, sizeof(ACDK_CCT_REG_RW_STRUCT));

    MUINT32 u4InRegAddr = 0;
    MUINT32 u4InRegData = 0;

    ACDK_LOGD("ACDK_CCT_OP_WRITE_SENSOR_REG\n");

    getHexToken((char *)a_pprArgv[0], &u4InRegAddr);
    getHexToken((char *)a_pprArgv[1], &u4InRegData);


    ACDK_reg_write.RegAddr	= u4InRegAddr;
    ACDK_reg_write.RegData = u4InRegData;

    MUINT32 u4RetLen = 0;

    BOOL bRet = bSendDataToACDK(ACDK_CCT_OP_WRITE_SENSOR_REG,
                                (UINT8 *)&ACDK_reg_write,
                                sizeof(ACDK_CCT_REG_RW_STRUCT),
                                (UINT8 *)&ACDK_reg_write,
                                sizeof(ACDK_CCT_REG_RW_STRUCT),
                                &u4RetLen);

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    ACDK_LOGD("Write Addr:0x%X\n", ACDK_reg_write.RegAddr);
    ACDK_LOGD("Write Data:0x%X\n", ACDK_reg_write.RegData);

    return S_ACDK_CCAP_OK;

}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_GET_SENSOR_RESOLUTION
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetSensorRes_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_CCT_SENSOR_RESOLUTION_STRUCT  SensorResolution;

    memset(&SensorResolution,0,sizeof(ACDK_CCT_SENSOR_RESOLUTION_STRUCT));

    MUINT32 u4RetLen = 0;

    ACDK_LOGD("ACDK_CCT_V2_OP_GET_SENSOR_RESOLUTION\n");

    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_GET_SENSOR_RESOLUTION,
                                                        NULL,
                                                        0,
                                                        (UINT8 *)&SensorResolution,
                                                        sizeof(ACDK_CCT_SENSOR_RESOLUTION_STRUCT),
                                                        &u4RetLen);

    if (!bRet)
    {
        ACDK_LOGE("Get Sensor Resolution Fail \n");
        return E_ACDK_CCAP_API_FAIL;
    }

    ACDK_LOGD("Sensor Resolution:\n");
    ACDK_LOGD("Preview Width:%d\n", SensorResolution.SensorPreviewWidth);
    ACDK_LOGD("Preview Height:%d\n", SensorResolution.SensorPreviewHeight);
    ACDK_LOGD("Full Width:%d\n", SensorResolution.SensorFullWidht);
    ACDK_LOGD("Full Height:%d\n", SensorResolution.SensorFullHeight);
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_GET_LSC_SENSOR_RESOLUTION
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetLSCSensorRes_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_CCT_SENSOR_RESOLUTION_STRUCT  SensorResolution;

    memset(&SensorResolution,0,sizeof(ACDK_CCT_SENSOR_RESOLUTION_STRUCT));

    MUINT32 u4RetLen = 0;

    ACDK_LOGD("ACDK_CCT_OP_GET_LSC_SENSOR_RESOLUTION\n");

    BOOL bRet = bSendDataToACDK(ACDK_CCT_OP_GET_LSC_SENSOR_RESOLUTION,
                                                        NULL,
                                                        0,
                                                        (UINT8 *)&SensorResolution,
                                                        sizeof(ACDK_CCT_SENSOR_RESOLUTION_STRUCT),
                                                        &u4RetLen);

    if (!bRet)
    {
        ACDK_LOGE("Get Sensor Resolution Fail \n");
        return E_ACDK_CCAP_API_FAIL;
    }

    ACDK_LOGD("LSC Sensor Resolution:\n");
    ACDK_LOGD("Preview Width:%d\n", SensorResolution.SensorPreviewWidth);
    ACDK_LOGD("Preview Height:%d\n", SensorResolution.SensorPreviewHeight);
    ACDK_LOGD("Full Width:%d\n", SensorResolution.SensorFullWidht);
    ACDK_LOGD("Full Height:%d\n", SensorResolution.SensorFullHeight);
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_QUERY_SENSOR
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPQuerySensor_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_CCT_SENSOR_INFO_STRUCT ACDK_Sensor;

    memset(&ACDK_Sensor,0,sizeof(ACDK_CCT_SENSOR_INFO_STRUCT));

    MUINT32 u4RetLen = 0;

    ACDK_LOGD("ACDK_CCT_OP_QUERY_SENSOR\n");

    BOOL bRet = bSendDataToACDK(ACDK_CCT_OP_QUERY_SENSOR,
                                                        NULL,
                                                        0,
                                                        (UINT8 *)&ACDK_Sensor,
                                                        sizeof(ACDK_CCT_SENSOR_INFO_STRUCT),
                                                        &u4RetLen);


    if (!bRet)
    {
        ACDK_LOGE("CCAP Query Sensor Fail \n");
        return E_ACDK_CCAP_API_FAIL;
    }


    ACDK_LOGD("Query Sensor Info:\n");
    ACDK_LOGD("Type:%d\n", ACDK_Sensor.Type);
    ACDK_LOGD("DeviceId:%d\n", ACDK_Sensor.DeviceId);
    ACDK_LOGD("StartPixelBayerPtn:%d\n", ACDK_Sensor.StartPixelBayerPtn);
    ACDK_LOGD("GrabXOffset:%d\n", ACDK_Sensor.GrabXOffset);
    ACDK_LOGD("GrabYOffset:%d\n", ACDK_Sensor.GrabYOffset);

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// MSDK_CCT_OP_GET_ENG_SENSOR_PARA
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetEngSensorPara_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    //!
    //! Please ref mrCCAPGetSensorPreGain_Cmd();
    //!

    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("Please ref mrCCAPGetSensorPreGain_Cmd(); \n");

    return S_ACDK_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// MSDK_CCT_OP_SET_ENG_SENSOR_PARA
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetEngSensorPara_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    //!
    //! Please ref mrCCAPSetSensorPreGain_Cmd();
    //!

    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("Please ref mrCCAPSetSensorPreGain_Cmd(); \n");
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// MSDK_CCT_OP_GET_ENG_SENSOR_GROUP_PARA
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetEngSensorGroupPara_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("MSDK_CCT_OP_GET_ENG_SENSOR_GROUP_PARA\n");

    UINT8 group_name[64] = {0};
    ACDK_SENSOR_GROUP_INFO_STRUCT ACDK_GroupName;
    memset(&ACDK_GroupName,0,sizeof(ACDK_SENSOR_GROUP_INFO_STRUCT));
    ACDK_GroupName.GroupNamePtr = group_name;

    MUINT32 nIndate = 0;
    MUINT32 u4RetLen = 0;

    BOOL bRet = bSendDataToACDK(ACDK_CCT_OP_GET_ENG_SENSOR_GROUP_PARA,
                                                       (UINT8 *)&nIndate,
                                                       sizeof(MUINT32),
                                                       (UINT8 *)&ACDK_GroupName,
                                                       sizeof(ACDK_SENSOR_GROUP_INFO_STRUCT),
                                                       &u4RetLen);

    if (!bRet)
    {
        ACDK_LOGE("CCAP Get Eng Sensor Group Para Fail \n");
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_GET_ENG_SENSOR_GROUP_COUNT
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetEngSensorGroupCnt_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_GET_ENG_SENSOR_GROUP_COUNT\n");
    MUINT32 uGroupCnt = 0;

    MUINT32 u4RetLen = 0;

    BOOL bRet = bSendDataToACDK(ACDK_CCT_OP_GET_ENG_SENSOR_GROUP_COUNT,
                                                        NULL,
                                                        0,
                                                        (UINT8 *)&uGroupCnt,
                                                        sizeof(MUINT32),
                                                        &u4RetLen);


    if (!bRet)
    {
        ACDK_LOGE("Get Sensor Group Count Fail \n");
        return E_ACDK_CCAP_API_FAIL;
    }

    ACDK_LOGD("Get Sensor Group Count:%d\n", uGroupCnt);

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_OP_GET_SENSOR_PREGAIN
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetSensorPreGain_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("FT_MSDK_CCT_OP_GET_SENSOR_PREGAIN\n");


    ACDK_SENSOR_ITEM_INFO_STRUCT ACDK_Data;
    memset(&ACDK_Data,0,sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT));

    MUINT32 u4RetLen = 0;
    BOOL bRet = TRUE;

    //Pregain R
    ACDK_Data.GroupIdx = 0;           //g_FT_CCT_StateMachine.sensor_eng_group_idx; PRE_GAIN
    ACDK_Data.ItemIdx = 1;             //g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_R

    bRet &= bSendDataToACDK(ACDK_CCT_OP_GET_ENG_SENSOR_PARA,
                                                (UINT8*)&ACDK_Data,
                                                sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
                                                (UINT8*)&ACDK_Data,
                                                 sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
                                                &u4RetLen);

    ACDK_LOGD("CCAP GET_SENSOR_PREGAIN_R:%d, %d, %d\n", ACDK_Data.ItemValue,
                                                                                                  ACDK_Data.Min,
                                                                                                  ACDK_Data.Max);


    //Pregain Gr
    ACDK_Data.ItemIdx = 2;            //g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_Gr

    ACDK_LOGD("CCAP GET_SENSOR_PREGAIN_Gr:%d, %d, %d\n", ACDK_Data.ItemValue,
                                                                                                   ACDK_Data.Min,
                                                                                                  ACDK_Data.Max);

    bRet &= bSendDataToACDK(ACDK_CCT_OP_GET_ENG_SENSOR_PARA,
                                                (UINT8*)&ACDK_Data,
                                                sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
                                                (UINT8*)&ACDK_Data,
                                                sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
                                                &u4RetLen);

    //Pregain Gb
    ACDK_Data.ItemIdx = 3;            //g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_Gr
    ACDK_Data.ItemValue = 0x20;   //pREQ->cmd.set_sensor_pregain.pregain_r.value;

    ACDK_LOGD("CCAP GET_SENSOR_PREGAIN_Gb:%d, %d, %d\n", ACDK_Data.ItemValue,
                                                                                                   ACDK_Data.Min,
                                                                                                   ACDK_Data.Max);

    bRet &= bSendDataToACDK(ACDK_CCT_OP_GET_ENG_SENSOR_PARA,
                                                (UINT8*)&ACDK_Data,
                                                sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
                                                 (UINT8*)&ACDK_Data,
                                                sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
                                                &u4RetLen);



    //Pregain B
    ACDK_Data.ItemIdx = 4;            //g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_Gr
    ACDK_Data.ItemValue = 0x20;   //pREQ->cmd.set_sensor_pregain.pregain_r.value;

    ACDK_LOGD("CCAP GET_SENSOR_PREGAIN_B:%d, %d, %d\n", ACDK_Data.ItemValue,
                                                                                                   ACDK_Data.Min,
                                                                                                   ACDK_Data.Max);

    bRet &= bSendDataToACDK(ACDK_CCT_OP_GET_ENG_SENSOR_PARA,
                                                (UINT8*)&ACDK_Data,
                                                sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
                                                 (UINT8*)&ACDK_Data,
                                                sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
                                                &u4RetLen);


    if (!bRet)
    {
        ACDK_LOGE("Get Sensor PreGain Fail \n");
        return E_ACDK_CCAP_API_FAIL;
    }
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_OP_SET_SENSOR_PREGAIN
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetSensorPreGain_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{

    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("FT_MSDK_CCT_OP_SET_SENSOR_PREGAIN\n");
    ACDK_SENSOR_ITEM_INFO_STRUCT ACDK_Item;
    memset(&ACDK_Item,0,sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT));

    MUINT32 u4RetLen = 0;
    BOOL bRet = TRUE;

    //Pregain R
    ACDK_Item.GroupIdx = 0;           //g_FT_CCT_StateMachine.sensor_eng_group_idx; PRE_GAIN
    ACDK_Item.ItemIdx = 1;             //g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_R
    ACDK_Item.ItemValue = 0x20;    //pREQ->cmd.set_sensor_pregain.pregain_r.value;
    ACDK_LOGD("CCAP SET_SENSOR_PREGAIN_R:%d, %d, %d\n", ACDK_Item.GroupIdx,
                                                                                                  ACDK_Item.ItemIdx,
                                                                                                  ACDK_Item.ItemValue);




    bRet &= bSendDataToACDK(ACDK_CCT_OP_SET_ENG_SENSOR_PARA,
                                                (UINT8*)&ACDK_Item,
                                                sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
                                                NULL,
                                                0,
                                                &u4RetLen);

    //Pregain Gr
    ACDK_Item.ItemIdx = 2;            //g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_Gr
    ACDK_Item.ItemValue = 0x20;   //pREQ->cmd.set_sensor_pregain.pregain_r.value;

    ACDK_LOGD("CCAP SET_SENSOR_PREGAIN_Gr:%d, %d, %d\n", ACDK_Item.GroupIdx,
                                                                                                   ACDK_Item.ItemIdx,
                                                                                                   ACDK_Item.ItemValue);

    bRet &= bSendDataToACDK(ACDK_CCT_OP_SET_ENG_SENSOR_PARA,
                                                (UINT8*)&ACDK_Item,
                                                sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
                                                NULL,
                                                0,
                                                &u4RetLen);

    //Pregain Gb
    ACDK_Item.ItemIdx = 3;            //g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_Gr
    ACDK_Item.ItemValue = 0x20;   //pREQ->cmd.set_sensor_pregain.pregain_r.value;

    ACDK_LOGD("CCAP SET_SENSOR_PREGAIN_Gb:%d, %d, %d\n", ACDK_Item.GroupIdx,
                                                                                                   ACDK_Item.ItemIdx,
                                                                                                   ACDK_Item.ItemValue);

    bRet &= bSendDataToACDK(ACDK_CCT_OP_SET_ENG_SENSOR_PARA,
                                                (UINT8*)&ACDK_Item,
                                                sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
                                                NULL,
                                                0,
                                                &u4RetLen);



    //Pregain B
    ACDK_Item.ItemIdx = 4;            //g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_Gr
    ACDK_Item.ItemValue = 0x20;   //pREQ->cmd.set_sensor_pregain.pregain_r.value;

    ACDK_LOGD("CCAP SET_SENSOR_PREGAIN_B:%d, %d, %d\n", ACDK_Item.GroupIdx,
                                                                                                   ACDK_Item.ItemIdx,
                                                                                                   ACDK_Item.ItemValue);

    bRet &= bSendDataToACDK(ACDK_CCT_OP_SET_ENG_SENSOR_PARA,
                                                (UINT8*)&ACDK_Item,
                                                sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
                                                NULL,
                                                0,
                                                &u4RetLen);


    if (!bRet)
    {
        ACDK_LOGE("Set Sensor PreGain Fail \n");
        return E_ACDK_CCAP_API_FAIL;
    }
    return S_ACDK_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_AE_ENABLE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAEEnable_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend)
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_AE_ENABLE\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_OP_AE_ENABLE,
                                NULL,
                                0,
                                NULL,
                                0,
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_AE_DISABLE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAEDisable_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_AE_DISABLE\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_OP_AE_DISABLE,
                                NULL,
                                0,
                                NULL,
                                0,
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_AE_GET_ENABLE_INFO
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetAEInfo_cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_AE_GET_ENABLE_INFO\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    MUINT32 u4RetLen = 0;
    MUINT32 u4AEEnableInfo = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_OP_AE_GET_ENABLE_INFO,
                                NULL,
                                0,
                                (UINT8 *)&u4AEEnableInfo,
                                sizeof(u4AEEnableInfo),
                                &u4RetLen);

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    ACDK_LOGD("AE enable Mode = %d\n", u4AEEnableInfo);

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_DEV_AE_SET_SCENE_MODE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAESetSceneMode_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_DEV_AE_SET_SCENE_MODE\n");

    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: AE scene mode value\n");
        return E_ACDK_IF_API_FAIL;
    }
    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MUINT32 u4RetLen = 0;
    MINT32 i4AEMode;

    i4AEMode = (MINT32) atoi((char *)a_pprArgv[0]);    // Off:0 Auto:1 Night:7 Action:8 Beach:9 Candlelight:10 Fireworks:11 Landscape:12 Portrait:13
                                                      // Night portrait:14 Party:15 Snow:16 Sports:17 Steadyphoto:18 Sunset:19 Theatre:20 ISO Anti shake:21

    ACDK_LOGD("AE scene mode = %d\n", i4AEMode);

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_OP_DEV_AE_SET_SCENE_MODE,
                                (UINT8 *)&i4AEMode,
                                sizeof(MINT32),
                                NULL,
                                0,
                                &u4RetLen);

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_DEV_AE_GET_INFO
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAEGetInfo_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    AE_NVRAM_T rAENVRAM;

    memset(&rAENVRAM,0, sizeof(AE_NVRAM_T));

    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_DEV_AE_GET_INFO\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_OP_DEV_AE_GET_INFO,
                                NULL,
                                0,
                                (UINT8 *)&rAENVRAM,
                                sizeof(AE_NVRAM_T),
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    // TEST ONLY (check AE parameter)
    ACDK_LOGD("u4MinGain = %d\n", rAENVRAM.rDevicesInfo.u4MinGain);
    ACDK_LOGD("u4MaxGain = %d\n", rAENVRAM.rDevicesInfo.u4MaxGain);
    ACDK_LOGD("u4MiniISOGain = %d\n", rAENVRAM.rDevicesInfo.u4MiniISOGain);
    ACDK_LOGD("u4GainStepUnit = %d\n", rAENVRAM.rDevicesInfo.u4GainStepUnit);
    ACDK_LOGD("u4PreExpUnit = %d\n", rAENVRAM.rDevicesInfo.u4PreExpUnit);
    ACDK_LOGD("u4PreMaxFrameRate = %d\n", rAENVRAM.rDevicesInfo.u4PreMaxFrameRate);
    ACDK_LOGD("u4VideoExpUnit = %d\n", rAENVRAM.rDevicesInfo.u4VideoExpUnit);
    ACDK_LOGD("u4VideoMaxFrameRate = %d\n", rAENVRAM.rDevicesInfo.u4VideoMaxFrameRate);
    ACDK_LOGD("u4Video2PreRatio = %d\n", rAENVRAM.rDevicesInfo.u4Video2PreRatio);
    ACDK_LOGD("u4CapExpUnit = %d\n", rAENVRAM.rDevicesInfo.u4CapExpUnit);
    ACDK_LOGD("u4CapMaxFrameRate = %d\n", rAENVRAM.rDevicesInfo.u4CapMaxFrameRate);
    ACDK_LOGD("u4Cap2PreRatio = %d\n", rAENVRAM.rDevicesInfo.u4Cap2PreRatio);
    ACDK_LOGD("u4LensFno = %d\n", rAENVRAM.rDevicesInfo.u4LensFno);

    ACDK_LOGD("u4HistHighThres = %d\n", rAENVRAM.rHistConfig.u4HistHighThres);
    ACDK_LOGD("u4HistLowThres = %d\n", rAENVRAM.rHistConfig.u4HistLowThres);
    ACDK_LOGD("u4MostBrightRatio = %d\n", rAENVRAM.rHistConfig.u4MostBrightRatio);
    ACDK_LOGD("u4MostDarkRatio = %d\n", rAENVRAM.rHistConfig.u4MostDarkRatio);
    ACDK_LOGD("u4CentralHighBound = %d\n", rAENVRAM.rHistConfig.u4CentralHighBound);
    ACDK_LOGD("u4CentralLowBound = %d\n", rAENVRAM.rHistConfig.u4CentralLowBound);
    ACDK_LOGD("u4OverExpThres[5] = {%d, %d, %d, %d, %d}\n", rAENVRAM.rHistConfig.u4OverExpThres[0], rAENVRAM.rHistConfig.u4OverExpThres[1],
                                     rAENVRAM.rHistConfig.u4OverExpThres[2], rAENVRAM.rHistConfig.u4OverExpThres[3], rAENVRAM.rHistConfig.u4OverExpThres[4]);
    ACDK_LOGD("u4HistStretchThres[5] = {%d, %d, %d, %d, %d}\n", rAENVRAM.rHistConfig.u4HistStretchThres[0], rAENVRAM.rHistConfig.u4HistStretchThres[1],
                                     rAENVRAM.rHistConfig.u4HistStretchThres[2], rAENVRAM.rHistConfig.u4HistStretchThres[3], rAENVRAM.rHistConfig.u4HistStretchThres[4]);
    ACDK_LOGD("u4BlackLightThres[5] = {%d, %d, %d, %d, %d}\n", rAENVRAM.rHistConfig.u4BlackLightThres[0], rAENVRAM.rHistConfig.u4BlackLightThres[1],
                                     rAENVRAM.rHistConfig.u4BlackLightThres[2], rAENVRAM.rHistConfig.u4BlackLightThres[3], rAENVRAM.rHistConfig.u4BlackLightThres[4]);
    ACDK_LOGD("bEnableBlackLight = %d\n", rAENVRAM.rCCTConfig.bEnableBlackLight);
    ACDK_LOGD("bEnableHistStretch = %d\n", rAENVRAM.rCCTConfig.bEnableHistStretch);
    ACDK_LOGD("bEnableAntiOverExposure = %d\n", rAENVRAM.rCCTConfig.bEnableAntiOverExposure);
    ACDK_LOGD("bEnableTimeLPF = %d\n", rAENVRAM.rCCTConfig.bEnableTimeLPF);
    ACDK_LOGD("bEnableCaptureThres = %d\n", rAENVRAM.rCCTConfig.bEnableCaptureThres);
    ACDK_LOGD("u4AETarget = %d\n", rAENVRAM.rCCTConfig.u4AETarget);
    ACDK_LOGD("u4InitIndex = %d\n", rAENVRAM.rCCTConfig.u4InitIndex);
    ACDK_LOGD("u4BackLightWeight = %d\n", rAENVRAM.rCCTConfig.u4BackLightWeight);
    ACDK_LOGD("u4HistStretchWeight = %d\n", rAENVRAM.rCCTConfig.u4HistStretchWeight);
    ACDK_LOGD("u4AntiOverExpWeight = %d\n", rAENVRAM.rCCTConfig.u4AntiOverExpWeight);
    ACDK_LOGD("u4BlackLightStrengthIndex = %d\n", rAENVRAM.rCCTConfig.u4BlackLightStrengthIndex);
    ACDK_LOGD("u4HistStretchStrengthIndex = %d\n", rAENVRAM.rCCTConfig.u4HistStretchStrengthIndex);
    ACDK_LOGD("u4AntiOverExpStrengthIndex = %d\n", rAENVRAM.rCCTConfig.u4AntiOverExpStrengthIndex);
    ACDK_LOGD("u4TimeLPFStrengthIndex = %d\n", rAENVRAM.rCCTConfig.u4TimeLPFStrengthIndex);
    ACDK_LOGD("u4LPFConvergeLevel[5] = {%d, %d, %d, %d, %d}\n", rAENVRAM.rCCTConfig.u4LPFConvergeLevel[0], rAENVRAM.rCCTConfig.u4LPFConvergeLevel[1],
                                     rAENVRAM.rCCTConfig.u4LPFConvergeLevel[2], rAENVRAM.rCCTConfig.u4LPFConvergeLevel[3], rAENVRAM.rCCTConfig.u4LPFConvergeLevel[4]);
    ACDK_LOGD("u4InDoorEV = %d\n", rAENVRAM.rCCTConfig.u4InDoorEV);
    ACDK_LOGD("u4BVOffset = %d\n", rAENVRAM.rCCTConfig.u4BVOffset);
    ACDK_LOGD("u4PreviewFlareOffset = %d\n", rAENVRAM.rCCTConfig.u4PreviewFlareOffset);
    ACDK_LOGD("u4CaptureFlareOffset = %d\n", rAENVRAM.rCCTConfig.u4CaptureFlareOffset);
    ACDK_LOGD("u4CaptureFlareThres = %d\n", rAENVRAM.rCCTConfig.u4CaptureFlareThres);

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AE_GET_SCENE_MODE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAEGetSceneMode_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AE_GET_SCENE_MODE\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    MINT32 i4AEMode = 0;
    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_AE_GET_SCENE_MODE,
                                NULL,
                                0,
                                (UINT8 *)&i4AEMode,
                                sizeof(i4AEMode),
                                &u4RetLen);

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    ACDK_LOGD("AE Mode = %d\n", i4AEMode);

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AE_SET_METERING_MODE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAESetMeteringMode_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AE_SET_METERING_MODE\n");

    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: AE metering mode value\n");
        return E_ACDK_IF_API_FAIL;
    }
    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MUINT32 u4RetLen = 0;
    MINT32 i4AEMeteringMode;

    i4AEMeteringMode = (MINT32) atoi((char *)a_pprArgv[0]);    // Central weighting : 0    Spot : 1    Average : 2

    ACDK_LOGD("AE metering mode = %d\n", i4AEMeteringMode);

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_AE_SET_METERING_MODE,
                                (UINT8 *)&i4AEMeteringMode,
                                sizeof(MINT32),
                                NULL,
                                0,
                                &u4RetLen);

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AE_APPLY_EXPO_INFO
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAEApplyExpoInfo_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AE_APPLY_EXPO_INFO\n");

    if (a_u4Argc != 5)
    {
        ACDK_LOGD("Usage: Exposure AfeGain PreFlare CapFlare bFlareAuto\n");
        return E_ACDK_IF_API_FAIL;
    }

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    MUINT32 u4RetLen = 0, u4FrameRate;
    ACDK_AE_MODE_CFG_T rAEExpPara;

    rAEExpPara.u4Eposuretime = (MUINT32)atoi((char *)a_pprArgv[0]);
    rAEExpPara.u4AfeGain = (MUINT32)atoi((char *)a_pprArgv[1]);
    rAEExpPara.uFlareValue= (UINT8)atoi((char *)a_pprArgv[2]);
    rAEExpPara.uFlareGain = 128*255 / (255 - rAEExpPara.uFlareValue);
    u4FrameRate = 1000000 / rAEExpPara.u4Eposuretime;
    rAEExpPara.uCaptureFlareValue= (UINT8)atoi((char *)a_pprArgv[3]);
    rAEExpPara.uCaptureFlareGain = 128*255 / (255 - rAEExpPara.uFlareValue);
    rAEExpPara.bFlareAuto = (UINT8)atoi((char *)a_pprArgv[4]);
    if(u4FrameRate >= 30)
    {
        u4FrameRate = 30;
    }
    rAEExpPara.uFrameRate = u4FrameRate;

    ACDK_LOGD("Expoure time:%d\n", rAEExpPara.u4Eposuretime);
    ACDK_LOGD("AFE Gain:%d\n", rAEExpPara.u4AfeGain);
    ACDK_LOGD("Flare:%d Flare Gain:%d\n",rAEExpPara.uFlareValue, rAEExpPara.uFlareGain);
    ACDK_LOGD("Capture Flare:%d Capture Flare Gain:%d\n",rAEExpPara.uCaptureFlareValue, rAEExpPara.uCaptureFlareGain);
    ACDK_LOGD("bFlareAuto:%d\n", rAEExpPara.bFlareAuto);
    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_AE_APPLY_EXPO_INFO,
                                (UINT8 *)&rAEExpPara,
                                sizeof(ACDK_AE_MODE_CFG_T),
                                NULL,
                                0,
                                &u4RetLen);

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AE_SELECT_BAND
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAESelectBand_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AE_SELECT_BAND\n");

    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: Flicker mode value\n");
        return E_ACDK_IF_API_FAIL;
    }
    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MUINT32 u4RetLen = 0;
    MINT32 i4AEFlickerMode;

    i4AEFlickerMode = (MINT32) atoi((char *)a_pprArgv[0]);    // 60Hz : 0    50Hz : 1

    ACDK_LOGD("AE Flicker mode = %d\n", i4AEFlickerMode);

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_AE_SELECT_BAND,
                               (UINT8 *)&i4AEFlickerMode,
                               sizeof(MINT32),
                               NULL,
                               0,
                               &u4RetLen);

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AE_GET_AUTO_EXPO_PARA
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAEGetAutoExpoPara_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AE_GET_AUTO_EXPO_PARA\n");

    if (a_u4Argc != 1)
    {
        ACDK_LOGD("[Usage] GainMode = 0: AFE Gain, GainMode = 1: ISO\n");
        return E_ACDK_IF_API_FAIL;
    }
    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MUINT32 u4RetLen = 0;
    ACDK_AE_MODE_CFG_T rAEExpPara;
    MUINT32 u4GainMode;

    u4GainMode = (MUINT32) atoi((char *)a_pprArgv[0]);

    ACDK_LOGD("Gain mode = %d\n", u4GainMode);

    memset(&rAEExpPara,0, sizeof(ACDK_AE_MODE_CFG_T));

    rAEExpPara.u4GainMode = u4GainMode;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_AE_GET_AUTO_EXPO_PARA,
                                (UINT8 *)&rAEExpPara,
                                sizeof(ACDK_AE_MODE_CFG_T),
                                (UINT8 *)&rAEExpPara,
                                sizeof(ACDK_AE_MODE_CFG_T),
                                &u4RetLen);

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    ACDK_LOGD("Expoure time:%d\n", rAEExpPara.u4Eposuretime);
    ACDK_LOGD("Gain Mode:%d\n", rAEExpPara.u4GainMode);
    ACDK_LOGD("AFE Gain:%d\n", rAEExpPara.u4AfeGain);
    ACDK_LOGD("ISO:%d\n", rAEExpPara.u4ISO);
    ACDK_LOGD("Preview Flare:%d Flare Gain:%d\n",rAEExpPara.uFlareValue, rAEExpPara.uFlareGain);
    ACDK_LOGD("Capture Flare:%d Flare Gain:%d\n",rAEExpPara.uCaptureFlareValue, rAEExpPara.uCaptureFlareGain);
    ACDK_LOGD("bFlareAuto:%d\n",rAEExpPara.bFlareAuto);

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AE_GET_BAND
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAEGetBand_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
     if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AE_GET_BAND\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    MUINT32 u4RetLen = 0;
    MINT32 i4AEFlickerMode = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_AE_GET_BAND,
                                NULL,
                                0,
                                (UINT8 *)&i4AEFlickerMode,
                                sizeof(MINT32),
                                &u4RetLen);

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    ACDK_LOGD("AE Flicker Mode = %d\n", i4AEFlickerMode);

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AE_GET_METERING_RESULT
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAEGetMeteringResoult_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
     if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AE_GET_METERING_RESULT\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    MUINT32 u4RetLen = 0;
    MINT32 i4AEMeteringMode = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_AE_GET_METERING_RESULT,
                                NULL,
                                0,
                                (UINT8 *)&i4AEMeteringMode,
                                sizeof(MINT32),
                                &u4RetLen);

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    ACDK_LOGD("AE Metering Mode = %d\n", i4AEMeteringMode);
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_DEV_AE_APPLY_INFO
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAEApplyInfo_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    // AE NVRAM test data
    AE_NVRAM_T rAENVRAMTestData;

    // TEST ONLY (check AE parameter)
    rAENVRAMTestData.rDevicesInfo.u4MinGain = 1200;
    rAENVRAMTestData.rDevicesInfo.u4MaxGain = 20480;
    rAENVRAMTestData.rDevicesInfo.u4MiniISOGain = 60;
    rAENVRAMTestData.rDevicesInfo.u4GainStepUnit = 256;
    rAENVRAMTestData.rDevicesInfo.u4PreExpUnit = 62;
    rAENVRAMTestData.rDevicesInfo.u4PreMaxFrameRate = 15;
    rAENVRAMTestData.rDevicesInfo.u4VideoExpUnit = 62;
    rAENVRAMTestData.rDevicesInfo.u4VideoMaxFrameRate = 10;
    rAENVRAMTestData.rDevicesInfo.u4Video2PreRatio = 1024;
    rAENVRAMTestData.rDevicesInfo.u4CapExpUnit = 116;
    rAENVRAMTestData.rDevicesInfo.u4CapMaxFrameRate = 4;
    rAENVRAMTestData.rDevicesInfo.u4Cap2PreRatio = 1024;
    rAENVRAMTestData.rDevicesInfo.u4LensFno = 56;

    rAENVRAMTestData.rHistConfig.u4HistHighThres = 4;
    rAENVRAMTestData.rHistConfig.u4HistLowThres = 20;
    rAENVRAMTestData.rHistConfig.u4MostBrightRatio = 4;
    rAENVRAMTestData.rHistConfig.u4MostDarkRatio = 2;
    rAENVRAMTestData.rHistConfig.u4CentralHighBound = 180;
    rAENVRAMTestData.rHistConfig.u4CentralLowBound = 40;
    rAENVRAMTestData.rHistConfig.u4OverExpThres[0] = 245;
    rAENVRAMTestData.rHistConfig.u4OverExpThres[1] = 235;
    rAENVRAMTestData.rHistConfig.u4OverExpThres[2] = 225;
    rAENVRAMTestData.rHistConfig.u4OverExpThres[3] = 215;
    rAENVRAMTestData.rHistConfig.u4OverExpThres[4] = 205;
    rAENVRAMTestData.rHistConfig.u4HistStretchThres[0] = 80;
    rAENVRAMTestData.rHistConfig.u4HistStretchThres[1] = 90;
    rAENVRAMTestData.rHistConfig.u4HistStretchThres[2] = 100;
    rAENVRAMTestData.rHistConfig.u4HistStretchThres[3] = 110;
    rAENVRAMTestData.rHistConfig.u4HistStretchThres[4] = 120;
    rAENVRAMTestData.rHistConfig.u4BlackLightThres[0] = 14;
    rAENVRAMTestData.rHistConfig.u4BlackLightThres[1] = 16;
    rAENVRAMTestData.rHistConfig.u4BlackLightThres[2] = 18;
    rAENVRAMTestData.rHistConfig.u4BlackLightThres[3] = 20;
    rAENVRAMTestData.rHistConfig.u4BlackLightThres[4] = 22;
    rAENVRAMTestData.rCCTConfig.bEnableBlackLight = FALSE;
    rAENVRAMTestData.rCCTConfig.bEnableHistStretch = FALSE;
    rAENVRAMTestData.rCCTConfig.bEnableAntiOverExposure = TRUE;
    rAENVRAMTestData.rCCTConfig.bEnableTimeLPF = FALSE;
    rAENVRAMTestData.rCCTConfig.bEnableCaptureThres = FALSE;
    rAENVRAMTestData.rCCTConfig.u4AETarget = 50;
    rAENVRAMTestData.rCCTConfig.u4InitIndex = 40;
    rAENVRAMTestData.rCCTConfig.u4BackLightWeight = 2;
    rAENVRAMTestData.rCCTConfig.u4HistStretchWeight = 16;
    rAENVRAMTestData.rCCTConfig.u4AntiOverExpWeight = 2;
    rAENVRAMTestData.rCCTConfig.u4BlackLightStrengthIndex = 0;
    rAENVRAMTestData.rCCTConfig.u4HistStretchStrengthIndex = 0;
    rAENVRAMTestData.rCCTConfig.u4AntiOverExpStrengthIndex = 0;
    rAENVRAMTestData.rCCTConfig.u4TimeLPFStrengthIndex = 0;
    rAENVRAMTestData.rCCTConfig.u4LPFConvergeLevel[0] = 1;
    rAENVRAMTestData.rCCTConfig.u4LPFConvergeLevel[1] = 2;
    rAENVRAMTestData.rCCTConfig.u4LPFConvergeLevel[2] = 3;
    rAENVRAMTestData.rCCTConfig.u4LPFConvergeLevel[3] = 4;
    rAENVRAMTestData.rCCTConfig.u4LPFConvergeLevel[4] = 5;
    rAENVRAMTestData.rCCTConfig.u4InDoorEV = 100;
    rAENVRAMTestData.rCCTConfig.u4BVOffset = 47;
    rAENVRAMTestData.rCCTConfig.u4PreviewFlareOffset = 2;
    rAENVRAMTestData.rCCTConfig.u4CaptureFlareOffset = 0;
    rAENVRAMTestData.rCCTConfig.u4CaptureFlareThres = 2;

     if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_DEV_AE_APPLY_INFO\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_OP_DEV_AE_APPLY_INFO,
                                (UINT8 *)&rAENVRAMTestData, // TEST ONLY
                                sizeof(AE_NVRAM_T),
                                NULL,
                                0,
                                &u4RetLen);

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_DEV_AE_SAVE_INFO_NVRAM
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAESaveInfoToNVRAM_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_DEV_AE_SAVE_INFO_NVRAM\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_OP_DEV_AE_SAVE_INFO_NVRAM,
                                NULL,
                                0,
                                NULL,
                                0,
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_DEV_AE_GET_EV_CALIBRATION
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAEGetEVCalibration_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
     if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_DEV_AE_GET_EV_CALIBRATION\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MUINT32 u4RetLen = 0;
    MINT32 i4AEcurrentEVValue = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_OP_DEV_AE_GET_EV_CALIBRATION,
                                NULL,
                                0,
                                (UINT8 *)&i4AEcurrentEVValue,
                                sizeof(MINT32),
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    ACDK_LOGD("AE current EV value = %d\n", i4AEcurrentEVValue);

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_AE_SET_SENSOR_EXP_LINE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAESetExpLine_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
     if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_AE_SET_SENSOR_EXP_LINE\n");

    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: AE exposure line\n");
        return E_ACDK_IF_API_FAIL;
    }
    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MUINT32 u4RetLen = 0;
    MINT32 i4AEExpLine;

    i4AEExpLine = (MINT32) atoi((char *)a_pprArgv[0]);

    ACDK_LOGD("AE exposure line = %d\n", i4AEExpLine);

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_OP_AE_SET_SENSOR_EXP_LINE,
                                (UINT8 *)&i4AEExpLine,
                                sizeof(MINT32),
                                NULL,
                                0,
                                &u4RetLen);

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAWBEnableAutoRun_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN,
                                NULL,
                                0,
                                NULL,
                                0,
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAWBDisableAutoRun_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN,
                                NULL,
                                0,
                                NULL,
                                0,
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_GET_AUTO_RUN_INFO
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAWBGetEnableInfo_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_GET_AUTO_RUN_INFO\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    MUINT32 u4RetLen = 0;
    MINT32 i4AWBEnable;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_AWB_GET_AUTO_RUN_INFO,
                                NULL,
                                0,
                                (UINT8 *)&i4AWBEnable,
                                sizeof(MINT32),
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    ACDK_LOGD("i4AWBEnable = %d\n", i4AWBEnable);

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_GET_GAIN
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAWBGetGain_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_GET_GAIN\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    MUINT32 u4RetLen = 0;
    AWB_GAIN_T rAWBGain;

    memset(&rAWBGain,0, sizeof(rAWBGain));

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_AWB_GET_GAIN,
                                NULL,
                                0,
                                (UINT8 *)&rAWBGain,
                                sizeof(AWB_GAIN_T),
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    ACDK_LOGD("AWB Rgain = %d\n", rAWBGain.u4R);
    ACDK_LOGD("AWB Ggain = %d\n", rAWBGain.u4G);
    ACDK_LOGD("AWB Bgain = %d\n", rAWBGain.u4B);

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_SET_GAIN
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAWBSetGain_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_SET_GAIN\n");

    if (a_u4Argc != 3)
    {
        ACDK_LOGD("Usage: sAWBGain <Rgain> <Ggain> <Bgain>\n");
        return E_ACDK_IF_API_FAIL;
    }

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    MUINT32 u4RetLen = 0;
    AWB_GAIN_T rAWBGain;

    rAWBGain.u4R = (MUINT32)atoi((char *)a_pprArgv[0]);
    rAWBGain.u4G = (MUINT32)atoi((char *)a_pprArgv[1]);
    rAWBGain.u4B = (MUINT32)atoi((char *)a_pprArgv[2]);

    ACDK_LOGD("AWB Rgain = %d\n", rAWBGain.u4R);
    ACDK_LOGD("AWB Ggain = %d\n", rAWBGain.u4G);
    ACDK_LOGD("AWB Bgain = %d\n", rAWBGain.u4B);

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_AWB_SET_GAIN,
                                (UINT8 *)&rAWBGain,
                                sizeof(AWB_GAIN_T),
                                NULL,
                                0,
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_APPLY_CAMERA_PARA2
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAWBApplyCameraPara_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
// AWB NVRAM test data
AWB_NVRAM_T rAWBNVRAMTestData =
{
	// AWB calibration data
	{
		// rCalGain (calibration gain: 1.0 = 512)
		{
			0,	// u4R
			0,	// u4G
			0	// u4B
		},
		// rDefGain (Default calibration gain: 1.0 = 512)
		{
			0,	// u4R
			0,	// u4G
			0   // u4B
		},
		// rD65Gain (D65 WB gain: 1.0 = 512)
		{
			512,//961,	// u4R
			512,	// u4G
			512//528	    // u4B
		}
	},
	// Original XY coordinate of AWB light source
	{
		// Horizon
		{
			-484,	// i4X
			-290	// i4Y
		},
		// A
		{
			-307,	// i4X
			-300	// i4Y
		},
		// TL84
		{
			-64,	// i4X
			-304	// i4Y
		},
		// CWF
		{
			-52,	// i4X
			-379	// i4Y
		},
		// DNP
		{
			100,	// i4X
			-341	// i4Y
		},
		// D65
		{
			260,	// i4X
			-287	// i4Y
		},
		// D75
		{
			395,	// i4X
			-283	// i4Y
		}
	},
	// Rotated XY coordinate of AWB light source
	{
		// Horizon
		{
			-489,	// i4X
			-282	// i4Y
		},
		// A
		{
			-312,	// i4X
			-295	// i4Y
		},
		// TL84
		{
			-69,	// i4X
			-303	// i4Y
		},
		// CWF
		{
			-58,	// i4X
			-378	// i4Y
		},
		// DNP
		{
			95,	// i4X
			-343	// i4Y
		},
		// D65
		{
			256,	// i4X
			-291	// i4Y
		},
		// D75
		{
			391,	// i4X
			-289	// i4Y
		}
	},
	// Rotation matrix parameter
	{
		1,	// i4RotationAngle
		128,	// i4H11
		2,	// i4H12
		-2,	// i4H21
		128	// i4H22
	},
	// Daylight locus parameter
	{
		-132,	// i4SlopeNumerator
		128	// i4SlopeDenominator
	},
	// AWB light area
	{
		// Tungsten
		{
			-119,	// i4RightBound
			-769,	// i4LeftBound
			-239,	// i4UpperBound
			-339	// i4LowerBound
		},
		// Warm fluorescent
		{
			-119,	// i4RightBound
			-769,	// i4LeftBound
			-339,	// i4UpperBound
			-459	// i4LowerBound
		},
		// Fluorescent
		{
			45,	// i4RightBound
			-119,	// i4LeftBound
			-238,	// i4UpperBound
			-341	// i4LowerBound
		},
		// CWF
		{
			45,	// i4RightBound
			-119,	// i4LeftBound
			-341,	// i4UpperBound
			-428	// i4LowerBound
		},
		// Daylight
		{
			324,	// i4RightBound
			45,	// i4LeftBound
			-211,	// i4UpperBound
			-371	// i4LowerBound
		},
		// Shade
		{
			684,	// i4RightBound
			324,	// i4LeftBound
			-209,	// i4UpperBound
			-369	// i4LowerBound
		}
	},
	// PWB light area
	{
		// Reference area
		{
			684,	// i4RightBound
			-769,	// i4LeftBound
			-209,	// i4UpperBound
			-459	// i4LowerBound
		},
		// Daylight
		{
			324,	// i4RightBound
			45,	// i4LeftBound
			-211,	// i4UpperBound
			-371	// i4LowerBound
		},
		// Cloudy daylight
		{
			471,	// i4RightBound
			324,	// i4LeftBound
			-209,	// i4UpperBound
			-369	// i4LowerBound
		},
		// Shade
		{
			631,	// i4RightBound
			471,	// i4LeftBound
			-209,	// i4UpperBound
			-369	// i4LowerBound
		},
		// Twilight
		{
			45,	// i4RightBound
			-115,	// i4LeftBound
			-211,	// i4UpperBound
			-371	// i4LowerBound
		},
		// Fluorescent
		{
			306,	// i4RightBound
			-119,	// i4LeftBound
			-253,	// i4UpperBound
			-428	// i4LowerBound
		},
		// Warm fluorescent
		{
			-119,	// i4RightBound
			-362,	// i4LeftBound
			-253,	// i4UpperBound
			-428	// i4LowerBound
		},
		// Incandescent
		{
			-119,	// i4RightBound
			-362,	// i4LeftBound
			-211,	// i4UpperBound
			-371	// i4LowerBound
		},
		// Gray World
		{
			10000,	// i4RightBound
			-10000,	// i4LeftBound
			10000,	// i4UpperBound
			-10000	// i4LowerBound
		}
	},
	// PWB default gain
	{
		// Daylight
		{
			887,	// u4R
			512,	// u4G
			574	// u4B
		},
		// Cloudy daylight
		{
			1126,	// u4R
			512,	// u4G
			446	// u4B
		},
		// Shade
		{
			1340,	// u4R
			512,	// u4G
			373	// u4B
		},
		// Twilight
		{
			691,	// u4R
			512,	// u4G
			742	// u4B
		},
		// Fluorescent
		{
			848,	// u4R
			512,	// u4G
			675	// u4B
		},
		// Warm fluorescent
		{
			580,	// u4R
			512,	// u4G
			998	// u4B
		},
		// Incandescent
		{
			548,	// u4R
			512,	// u4G
			943	// u4B
		},
		// Gray World
		{
			512,	// u4R
			512,	// u4G
			512	// u4B
		}
	},
	// AWB preference color
	{
		// Tungsten
		{
			50,	// i4SliderValue
			4345	// i4OffsetThr
		},
		// Warm fluorescent
		{
			50,	// i4SliderValue
			4345	// i4OffsetThr
		},
		// Shade
		{
			50,	// i4SliderValue
			788	// i4OffsetThr
		},
		// Daylight WB gain
		{
			801,	// u4R
			512,	// u4G
			637	// u4B
		}
	},
	// CCT estimation
	{
		// CCT
		{
			2400,	// i4CCT[0]
			2850,	// i4CCT[1]
			4100,	// i4CCT[2]
			5100,	// i4CCT[3]
			6500,	// i4CCT[4]
			7500	// i4CCT[5]
		},
		// Rotated X coordinate
		{
			-745,	// i4RotatedXCoordinate[0]
			-568,	// i4RotatedXCoordinate[1]
			-325,	// i4RotatedXCoordinate[2]
			-161,	// i4RotatedXCoordinate[3]
			0,	// i4RotatedXCoordinate[4]
			135	// i4RotatedXCoordinate[5]
		}
	}
};

     if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_APPLY_CAMERA_PARA2\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_AWB_APPLY_CAMERA_PARA2,
                                (UINT8 *)&rAWBNVRAMTestData, // TEST ONLY
                                sizeof(AWB_NVRAM_T),
                                NULL,
                                0,
                                &u4RetLen);

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_GET_AWB_PARA
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAWBGetAWBPara_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    AWB_NVRAM_T rAWBNVRAM; // TEST ONLY

    memset(&rAWBNVRAM,0, sizeof(rAWBNVRAM)); // TEST ONLY

    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_GET_AWB_PARA\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_AWB_GET_AWB_PARA,
                                NULL,
                                0,
                                (UINT8 *)&rAWBNVRAM,
                                sizeof(AWB_NVRAM_T),
                                &u4RetLen);

    // TEST ONLY (check AWB parameter)
    ACDK_LOGD("rCalGain.u4R = %d\n", rAWBNVRAM.rCalData.rCalGain.u4R);
    ACDK_LOGD("rCalGain.u4G = %d\n", rAWBNVRAM.rCalData.rCalGain.u4G);
    ACDK_LOGD("rCalGain.u4B = %d\n", rAWBNVRAM.rCalData.rCalGain.u4B);
    ACDK_LOGD("rDefGain.u4R = %d\n", rAWBNVRAM.rCalData.rDefGain.u4R);
    ACDK_LOGD("rDefGain.u4G = %d\n", rAWBNVRAM.rCalData.rDefGain.u4G);
    ACDK_LOGD("rDefGain.u4B = %d\n", rAWBNVRAM.rCalData.rDefGain.u4B);
    ACDK_LOGD("rD65Gain.u4R = %d\n", rAWBNVRAM.rCalData.rD65Gain.u4R);
    ACDK_LOGD("rD65Gain.u4G = %d\n", rAWBNVRAM.rCalData.rD65Gain.u4G);
    ACDK_LOGD("rD65Gain.u4B = %d\n", rAWBNVRAM.rCalData.rD65Gain.u4B);

    ACDK_LOGD("rOriginalXY.rHorizon.i4X = %d\n", rAWBNVRAM.rOriginalXY.rHorizon.i4X);
    ACDK_LOGD("rOriginalXY.rHorizon.i4Y = %d\n", rAWBNVRAM.rOriginalXY.rHorizon.i4Y);
    ACDK_LOGD("rOriginalXY.rA.i4X = %d\n", rAWBNVRAM.rOriginalXY.rA.i4X);
    ACDK_LOGD("rOriginalXY.rA.i4Y = %d\n", rAWBNVRAM.rOriginalXY.rA.i4Y);
    ACDK_LOGD("rOriginalXY.rTL84.i4X = %d\n", rAWBNVRAM.rOriginalXY.rTL84.i4X);
    ACDK_LOGD("rOriginalXY.rTL84.i4Y = %d\n", rAWBNVRAM.rOriginalXY.rTL84.i4Y);
    ACDK_LOGD("rOriginalXY.rCWF.i4X = %d\n", rAWBNVRAM.rOriginalXY.rCWF.i4X);
    ACDK_LOGD("rOriginalXY.rCWF.i4Y = %d\n", rAWBNVRAM.rOriginalXY.rCWF.i4Y);
    ACDK_LOGD("rOriginalXY.rDNP.i4X = %d\n", rAWBNVRAM.rOriginalXY.rDNP.i4X);
    ACDK_LOGD("rOriginalXY.rDNP.i4Y = %d\n", rAWBNVRAM.rOriginalXY.rDNP.i4Y);
    ACDK_LOGD("rOriginalXY.rD65.i4X = %d\n", rAWBNVRAM.rOriginalXY.rD65.i4X);
    ACDK_LOGD("rOriginalXY.rD65.i4Y = %d\n", rAWBNVRAM.rOriginalXY.rD65.i4Y);
    ACDK_LOGD("rOriginalXY.rD75.i4X = %d\n", rAWBNVRAM.rOriginalXY.rD75.i4X);
    ACDK_LOGD("rOriginalXY.rD75.i4Y = %d\n", rAWBNVRAM.rOriginalXY.rD75.i4Y);

    ACDK_LOGD("rRotatedXY.rHorizon.i4X = %d\n", rAWBNVRAM.rRotatedXY.rHorizon.i4X);
    ACDK_LOGD("rRotatedXY.rHorizon.i4Y = %d\n", rAWBNVRAM.rRotatedXY.rHorizon.i4Y);
    ACDK_LOGD("rRotatedXY.rA.i4X = %d\n", rAWBNVRAM.rRotatedXY.rA.i4X);
    ACDK_LOGD("rRotatedXY.rA.i4Y = %d\n", rAWBNVRAM.rRotatedXY.rA.i4Y);
    ACDK_LOGD("rRotatedXY.rTL84.i4X = %d\n", rAWBNVRAM.rRotatedXY.rTL84.i4X);
    ACDK_LOGD("rRotatedXY.rTL84.i4Y = %d\n", rAWBNVRAM.rRotatedXY.rTL84.i4Y);
    ACDK_LOGD("rRotatedXY.rCWF.i4X = %d\n", rAWBNVRAM.rRotatedXY.rCWF.i4X);
    ACDK_LOGD("rRotatedXY.rCWF.i4Y = %d\n", rAWBNVRAM.rRotatedXY.rCWF.i4Y);
    ACDK_LOGD("rRotatedXY.rDNP.i4X = %d\n", rAWBNVRAM.rRotatedXY.rDNP.i4X);
    ACDK_LOGD("rRotatedXY.rDNP.i4Y = %d\n", rAWBNVRAM.rRotatedXY.rDNP.i4Y);
    ACDK_LOGD("rRotatedXY.rD65.i4X = %d\n", rAWBNVRAM.rRotatedXY.rD65.i4X);
    ACDK_LOGD("rRotatedXY.rD65.i4Y = %d\n", rAWBNVRAM.rRotatedXY.rD65.i4Y);
    ACDK_LOGD("rRotatedXY.rD75.i4X = %d\n", rAWBNVRAM.rRotatedXY.rD75.i4X);
    ACDK_LOGD("rRotatedXY.rD75.i4Y = %d\n", rAWBNVRAM.rRotatedXY.rD75.i4Y);

    ACDK_LOGD("rRotationMatrix.i4RotationAngle = %d\n", rAWBNVRAM.rRotationMatrix.i4RotationAngle);
    ACDK_LOGD("rRotationMatrix.i4H11 = %d\n", rAWBNVRAM.rRotationMatrix.i4H11);
    ACDK_LOGD("rRotationMatrix.i4H12 = %d\n", rAWBNVRAM.rRotationMatrix.i4H12);
    ACDK_LOGD("rRotationMatrix.i4H21 = %d\n", rAWBNVRAM.rRotationMatrix.i4H21);
    ACDK_LOGD("rRotationMatrix.i4H22 = %d\n", rAWBNVRAM.rRotationMatrix.i4H22);

    ACDK_LOGD("rDaylightLocus.i4SlopeNumerator = %d\n", rAWBNVRAM.rDaylightLocus.i4SlopeNumerator);
    ACDK_LOGD("rDaylightLocus.i4SlopeDenominator = %d\n", rAWBNVRAM.rDaylightLocus.i4SlopeDenominator);

    ACDK_LOGD("rAWBLightArea.rTungsten.i4RightBound = %d\n", rAWBNVRAM.rAWBLightArea.rTungsten.i4RightBound);
    ACDK_LOGD("rAWBLightArea.rTungsten.i4LeftBound = %d\n", rAWBNVRAM.rAWBLightArea.rTungsten.i4LeftBound);
    ACDK_LOGD("rAWBLightArea.rTungsten.i4UpperBound = %d\n", rAWBNVRAM.rAWBLightArea.rTungsten.i4UpperBound);
    ACDK_LOGD("rAWBLightArea.rTungsten.i4LowerBound = %d\n", rAWBNVRAM.rAWBLightArea.rTungsten.i4LowerBound);
    ACDK_LOGD("rAWBLightArea.rWarmFluorescent.i4RightBound = %d\n", rAWBNVRAM.rAWBLightArea.rWarmFluorescent.i4RightBound);
    ACDK_LOGD("rAWBLightArea.rWarmFluorescent.i4LeftBound = %d\n", rAWBNVRAM.rAWBLightArea.rWarmFluorescent.i4LeftBound);
    ACDK_LOGD("rAWBLightArea.rWarmFluorescent.i4UpperBound = %d\n", rAWBNVRAM.rAWBLightArea.rWarmFluorescent.i4UpperBound);
    ACDK_LOGD("rAWBLightArea.rWarmFluorescent.i4LowerBound = %d\n", rAWBNVRAM.rAWBLightArea.rWarmFluorescent.i4LowerBound);
    ACDK_LOGD("rAWBLightArea.rFluorescent.i4RightBound = %d\n", rAWBNVRAM.rAWBLightArea.rFluorescent.i4RightBound);
    ACDK_LOGD("rAWBLightArea.rFluorescent.i4LeftBound = %d\n", rAWBNVRAM.rAWBLightArea.rFluorescent.i4LeftBound);
    ACDK_LOGD("rAWBLightArea.rFluorescent.i4UpperBound = %d\n", rAWBNVRAM.rAWBLightArea.rFluorescent.i4UpperBound);
    ACDK_LOGD("rAWBLightArea.rFluorescent.i4LowerBound = %d\n", rAWBNVRAM.rAWBLightArea.rFluorescent.i4LowerBound);
    ACDK_LOGD("rAWBLightArea.rCWF.i4RightBound = %d\n", rAWBNVRAM.rAWBLightArea.rCWF.i4RightBound);
    ACDK_LOGD("rAWBLightArea.rCWF.i4LeftBound = %d\n", rAWBNVRAM.rAWBLightArea.rCWF.i4LeftBound);
    ACDK_LOGD("rAWBLightArea.rCWF.i4UpperBound = %d\n", rAWBNVRAM.rAWBLightArea.rCWF.i4UpperBound);
    ACDK_LOGD("rAWBLightArea.rCWF.i4LowerBound = %d\n", rAWBNVRAM.rAWBLightArea.rCWF.i4LowerBound);
    ACDK_LOGD("rAWBLightArea.rDaylight.i4RightBound = %d\n", rAWBNVRAM.rAWBLightArea.rDaylight.i4RightBound);
    ACDK_LOGD("rAWBLightArea.rDaylight.i4LeftBound = %d\n", rAWBNVRAM.rAWBLightArea.rDaylight.i4LeftBound);
    ACDK_LOGD("rAWBLightArea.rDaylight.i4UpperBound = %d\n", rAWBNVRAM.rAWBLightArea.rDaylight.i4UpperBound);
    ACDK_LOGD("rAWBLightArea.rDaylight.i4LowerBound = %d\n", rAWBNVRAM.rAWBLightArea.rDaylight.i4LowerBound);
    ACDK_LOGD("rAWBLightArea.rShade.i4RightBound = %d\n", rAWBNVRAM.rAWBLightArea.rShade.i4RightBound);
    ACDK_LOGD("rAWBLightArea.rShade.i4LeftBound = %d\n", rAWBNVRAM.rAWBLightArea.rShade.i4LeftBound);
    ACDK_LOGD("rAWBLightArea.rShade.i4UpperBound = %d\n", rAWBNVRAM.rAWBLightArea.rShade.i4UpperBound);
    ACDK_LOGD("rAWBLightArea.rShade.i4LowerBound = %d\n", rAWBNVRAM.rAWBLightArea.rShade.i4LowerBound);

    ACDK_LOGD("rPWBLightArea.rReferenceArea.i4RightBound = %d\n", rAWBNVRAM.rPWBLightArea.rReferenceArea.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rReferenceArea.i4LeftBound = %d\n", rAWBNVRAM.rPWBLightArea.rReferenceArea.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rReferenceArea.i4UpperBound = %d\n", rAWBNVRAM.rPWBLightArea.rReferenceArea.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rReferenceArea.i4LowerBound = %d\n", rAWBNVRAM.rPWBLightArea.rReferenceArea.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rDaylight.i4RightBound = %d\n", rAWBNVRAM.rPWBLightArea.rDaylight.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rDaylight.i4LeftBound = %d\n", rAWBNVRAM.rPWBLightArea.rDaylight.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rDaylight.i4UpperBound = %d\n", rAWBNVRAM.rPWBLightArea.rDaylight.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rDaylight.i4LowerBound = %d\n", rAWBNVRAM.rPWBLightArea.rDaylight.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rCloudyDaylight.i4RightBound = %d\n", rAWBNVRAM.rPWBLightArea.rCloudyDaylight.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rCloudyDaylight.i4LeftBound = %d\n", rAWBNVRAM.rPWBLightArea.rCloudyDaylight.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rCloudyDaylight.i4UpperBound = %d\n", rAWBNVRAM.rPWBLightArea.rCloudyDaylight.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rCloudyDaylight.i4LowerBound = %d\n", rAWBNVRAM.rPWBLightArea.rCloudyDaylight.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rShade.i4RightBound = %d\n", rAWBNVRAM.rPWBLightArea.rShade.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rShade.i4LeftBound = %d\n", rAWBNVRAM.rPWBLightArea.rShade.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rShade.i4UpperBound = %d\n", rAWBNVRAM.rPWBLightArea.rShade.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rShade.i4LowerBound = %d\n", rAWBNVRAM.rPWBLightArea.rShade.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rTwilight.i4RightBound = %d\n", rAWBNVRAM.rPWBLightArea.rTwilight.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rTwilight.i4LeftBound = %d\n", rAWBNVRAM.rPWBLightArea.rTwilight.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rTwilight.i4UpperBound = %d\n", rAWBNVRAM.rPWBLightArea.rTwilight.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rTwilight.i4LowerBound = %d\n", rAWBNVRAM.rPWBLightArea.rTwilight.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rFluorescent.i4RightBound = %d\n", rAWBNVRAM.rPWBLightArea.rFluorescent.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rFluorescent.i4LeftBound = %d\n", rAWBNVRAM.rPWBLightArea.rFluorescent.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rFluorescent.i4UpperBound = %d\n", rAWBNVRAM.rPWBLightArea.rFluorescent.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rFluorescent.i4LowerBound = %d\n", rAWBNVRAM.rPWBLightArea.rFluorescent.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rWarmFluorescent.i4RightBound = %d\n", rAWBNVRAM.rPWBLightArea.rWarmFluorescent.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rWarmFluorescent.i4LeftBound = %d\n", rAWBNVRAM.rPWBLightArea.rWarmFluorescent.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rWarmFluorescent.i4UpperBound = %d\n", rAWBNVRAM.rPWBLightArea.rWarmFluorescent.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rWarmFluorescent.i4LowerBound = %d\n", rAWBNVRAM.rPWBLightArea.rWarmFluorescent.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rIncandescent.i4RightBound = %d\n", rAWBNVRAM.rPWBLightArea.rIncandescent.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rIncandescent.i4LeftBound = %d\n", rAWBNVRAM.rPWBLightArea.rIncandescent.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rIncandescent.i4UpperBound = %d\n", rAWBNVRAM.rPWBLightArea.rIncandescent.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rIncandescent.i4LowerBound = %d\n", rAWBNVRAM.rPWBLightArea.rIncandescent.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rGrayWorld.i4RightBound = %d\n", rAWBNVRAM.rPWBLightArea.rGrayWorld.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rGrayWorld.i4LeftBound = %d\n", rAWBNVRAM.rPWBLightArea.rGrayWorld.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rGrayWorld.i4UpperBound = %d\n", rAWBNVRAM.rPWBLightArea.rGrayWorld.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rGrayWorld.i4LowerBound = %d\n", rAWBNVRAM.rPWBLightArea.rGrayWorld.i4LowerBound);

    ACDK_LOGD("rPWBDefaultGain.rDaylight.u4R = %d\n", rAWBNVRAM.rPWBDefaultGain.rDaylight.u4R);
    ACDK_LOGD("rPWBDefaultGain.rDaylight.u4G = %d\n", rAWBNVRAM.rPWBDefaultGain.rDaylight.u4G);
    ACDK_LOGD("rPWBDefaultGain.rDaylight.u4B = %d\n", rAWBNVRAM.rPWBDefaultGain.rDaylight.u4B);
    ACDK_LOGD("rPWBDefaultGain.rCloudyDaylight.u4R = %d\n", rAWBNVRAM.rPWBDefaultGain.rCloudyDaylight.u4R);
    ACDK_LOGD("rPWBDefaultGain.rCloudyDaylight.u4G = %d\n", rAWBNVRAM.rPWBDefaultGain.rCloudyDaylight.u4G);
    ACDK_LOGD("rPWBDefaultGain.rCloudyDaylight.u4B = %d\n", rAWBNVRAM.rPWBDefaultGain.rCloudyDaylight.u4B);
    ACDK_LOGD("rPWBDefaultGain.rShade.u4R = %d\n", rAWBNVRAM.rPWBDefaultGain.rShade.u4R);
    ACDK_LOGD("rPWBDefaultGain.rShade.u4G = %d\n", rAWBNVRAM.rPWBDefaultGain.rShade.u4G);
    ACDK_LOGD("rPWBDefaultGain.rShade.u4B = %d\n", rAWBNVRAM.rPWBDefaultGain.rShade.u4B);
    ACDK_LOGD("rPWBDefaultGain.rTwilight.u4R = %d\n", rAWBNVRAM.rPWBDefaultGain.rTwilight.u4R);
    ACDK_LOGD("rPWBDefaultGain.rTwilight.u4G = %d\n", rAWBNVRAM.rPWBDefaultGain.rTwilight.u4G);
    ACDK_LOGD("rPWBDefaultGain.rTwilight.u4B = %d\n", rAWBNVRAM.rPWBDefaultGain.rTwilight.u4B);
    ACDK_LOGD("rPWBDefaultGain.rFluorescent.u4R = %d\n", rAWBNVRAM.rPWBDefaultGain.rFluorescent.u4R);
    ACDK_LOGD("rPWBDefaultGain.rFluorescent.u4G = %d\n", rAWBNVRAM.rPWBDefaultGain.rFluorescent.u4G);
    ACDK_LOGD("rPWBDefaultGain.rFluorescent.u4B = %d\n", rAWBNVRAM.rPWBDefaultGain.rFluorescent.u4B);
    ACDK_LOGD("rPWBDefaultGain.rWarmFluorescent.u4R = %d\n", rAWBNVRAM.rPWBDefaultGain.rWarmFluorescent.u4R);
    ACDK_LOGD("rPWBDefaultGain.rWarmFluorescent.u4G = %d\n", rAWBNVRAM.rPWBDefaultGain.rWarmFluorescent.u4G);
    ACDK_LOGD("rPWBDefaultGain.rWarmFluorescent.u4B = %d\n", rAWBNVRAM.rPWBDefaultGain.rWarmFluorescent.u4B);
    ACDK_LOGD("rPWBDefaultGain.rIncandescent.u4R = %d\n", rAWBNVRAM.rPWBDefaultGain.rIncandescent.u4R);
    ACDK_LOGD("rPWBDefaultGain.rIncandescent.u4G = %d\n", rAWBNVRAM.rPWBDefaultGain.rIncandescent.u4G);
    ACDK_LOGD("rPWBDefaultGain.rIncandescent.u4B = %d\n", rAWBNVRAM.rPWBDefaultGain.rIncandescent.u4B);
    ACDK_LOGD("rPWBDefaultGain.rGrayWorld.u4R = %d\n", rAWBNVRAM.rPWBDefaultGain.rGrayWorld.u4R);
    ACDK_LOGD("rPWBDefaultGain.rGrayWorld.u4G = %d\n", rAWBNVRAM.rPWBDefaultGain.rGrayWorld.u4G);
    ACDK_LOGD("rPWBDefaultGain.rGrayWorld.u4B = %d\n", rAWBNVRAM.rPWBDefaultGain.rGrayWorld.u4B);

    ACDK_LOGD("rPreferenceColor.rTungsten.i4SliderValue = %d\n", rAWBNVRAM.rPreferenceColor.rTungsten.i4SliderValue);
    ACDK_LOGD("rPreferenceColor.rTungsten.i4OffsetThr = %d\n", rAWBNVRAM.rPreferenceColor.rTungsten.i4OffsetThr);
    ACDK_LOGD("rPreferenceColor.rWarmFluorescent.i4SliderValue = %d\n", rAWBNVRAM.rPreferenceColor.rWarmFluorescent.i4SliderValue);
    ACDK_LOGD("rPreferenceColor.rWarmFluorescent.i4OffsetThr = %d\n", rAWBNVRAM.rPreferenceColor.rWarmFluorescent.i4OffsetThr);
    ACDK_LOGD("rPreferenceColor.rShade.i4SliderValue = %d\n", rAWBNVRAM.rPreferenceColor.rShade.i4SliderValue);
    ACDK_LOGD("rPreferenceColor.rShade.i4OffsetThr = %d\n", rAWBNVRAM.rPreferenceColor.rShade.i4OffsetThr);
    ACDK_LOGD("rPreferenceColor.rDaylightWBGain.u4R = %d\n", rAWBNVRAM.rPreferenceColor.rDaylightWBGain.u4R);
    ACDK_LOGD("rPreferenceColor.rDaylightWBGain.u4G = %d\n", rAWBNVRAM.rPreferenceColor.rDaylightWBGain.u4G);
    ACDK_LOGD("rPreferenceColor.rDaylightWBGain.u4B = %d\n", rAWBNVRAM.rPreferenceColor.rDaylightWBGain.u4B);

    ACDK_LOGD("rCCTEstimation.i4CCT[0] = %d\n", rAWBNVRAM.rCCTEstimation.i4CCT[0]);
    ACDK_LOGD("rCCTEstimation.i4CCT[1] = %d\n", rAWBNVRAM.rCCTEstimation.i4CCT[1]);
    ACDK_LOGD("rCCTEstimation.i4CCT[2] = %d\n", rAWBNVRAM.rCCTEstimation.i4CCT[2]);
    ACDK_LOGD("rCCTEstimation.i4CCT[3] = %d\n", rAWBNVRAM.rCCTEstimation.i4CCT[3]);
    ACDK_LOGD("rCCTEstimation.i4CCT[4] = %d\n", rAWBNVRAM.rCCTEstimation.i4CCT[4]);
    ACDK_LOGD("rCCTEstimation.i4CCT[5] = %d\n", rAWBNVRAM.rCCTEstimation.i4CCT[5]);

    ACDK_LOGD("rCCTEstimation.i4RotatedXCoordinate[0] = %d\n", rAWBNVRAM.rCCTEstimation.i4RotatedXCoordinate[0]);
    ACDK_LOGD("rCCTEstimation.i4RotatedXCoordinate[1] = %d\n", rAWBNVRAM.rCCTEstimation.i4RotatedXCoordinate[1]);
    ACDK_LOGD("rCCTEstimation.i4RotatedXCoordinate[2] = %d\n", rAWBNVRAM.rCCTEstimation.i4RotatedXCoordinate[2]);
    ACDK_LOGD("rCCTEstimation.i4RotatedXCoordinate[3] = %d\n", rAWBNVRAM.rCCTEstimation.i4RotatedXCoordinate[3]);
    ACDK_LOGD("rCCTEstimation.i4RotatedXCoordinate[4] = %d\n", rAWBNVRAM.rCCTEstimation.i4RotatedXCoordinate[4]);
    ACDK_LOGD("rCCTEstimation.i4RotatedXCoordinate[5] = %d\n", rAWBNVRAM.rCCTEstimation.i4RotatedXCoordinate[5]);




    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_SAVE_AWB_PARA
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAWBSaveAWBPara_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_SAVE_AWB_PARA\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_AWB_SAVE_AWB_PARA,
                                NULL,
                                0,
                                NULL,
                                0,
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_AWB_SET_AWB_MODE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAWBSetAWBMode_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_AWB_SET_AWB_MODE\n");

    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: sAWBMode <AWBMode>\n");
        return E_ACDK_IF_API_FAIL;
    }

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    MUINT32 u4RetLen = 0;
    MUINT32 u4AWBMode = (MUINT32) atoi((char *)a_pprArgv[0]);

    ACDK_LOGD("AWBMode = %d\n", u4AWBMode);


    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_OP_AWB_SET_AWB_MODE,
                                (UINT8 *)&u4AWBMode,
                                sizeof(MUINT32),
                                NULL,
                                0,
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_AWB_GET_AWB_MODE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAWBGetAWBMode_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_AWB_GET_AWB_MODE\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    MUINT32 u4RetLen = 0;
    MUINT32 u4AWBMode;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_OP_AWB_GET_AWB_MODE,
                                NULL,
                                0,
                                (UINT8 *)&u4AWBMode,
                                sizeof(MUINT32),
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    ACDK_LOGD("u4AWBMode = %d\n", u4AWBMode);

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AF_OPERATION
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAFOperation_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AF_OPERATION\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================



    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_AF_OPERATION,
                                NULL,
                                0,
                                NULL,
                                0,
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_MF_OPERATION
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPMFOperation_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
     if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_MF_OPERATION\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MINT32 i4MFPos = (MINT32)atoi((char *)a_pprArgv[0]);

    ACDK_LOGD("MF Pos = %d\n", i4MFPos);


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_MF_OPERATION,
						        (UINT8 *)&i4MFPos,
								sizeof(i4MFPos),
                                NULL,
                                0,
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_GET_AF_INFO
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetAFInfo_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
     if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_GET_AF_INFO\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

	ACDK_AF_INFO_T sAFInfo;


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_GET_AF_INFO,
			                    NULL,
								0,
								(UINT8*)&sAFInfo,
								sizeof(sAFInfo),
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    ACDK_LOGD("AF Info : [AFMode] %d, [AFMeter] %d, [Curr Pos] %d\n", sAFInfo.i4AFMode, sAFInfo.i4AFMeter, sAFInfo.i4CurrPos);

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AF_GET_BEST_POS
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAFGetBestPos_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AF_GET_BEST_POS\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

	MINT32 i4AFBestPos;

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_AF_GET_BEST_POS,
                                NULL,
                                0,
                                (UINT8*)&i4AFBestPos,
                                sizeof(i4AFBestPos),
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    ACDK_LOGD("AF Best Pos = %d\n", i4AFBestPos);

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AF_CALI_OPERATION
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAFCaliOperation_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AF_CALI_OPERATION\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    ACDK_AF_CALI_DATA_T sCaliData;

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_AF_CALI_OPERATION,
                                NULL,
                                0,
							    (UINT8*)&sCaliData,
							    sizeof(sCaliData),
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    ACDK_LOGD("AF Best Pos = %d\n", sCaliData.i4BestPos);


    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AF_SET_RANGE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAFSetRange_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AF_SET_RANGE\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

	FOCUS_RANGE_T sFocusRange;

    sFocusRange.i4InfPos = (MINT32)atoi((char *)a_pprArgv[0]);
    sFocusRange.i4MacroPos = (MINT32)atoi((char *)a_pprArgv[1]);

    ACDK_LOGD("Focus Range = %d to %d\n", sFocusRange.i4InfPos, sFocusRange.i4MacroPos);


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_AF_SET_RANGE,
                                (UINT8*)&sFocusRange,
                                sizeof(sFocusRange),
                                NULL,
                                0,
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AF_GET_RANGE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAFGetRange_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
     if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AF_GET_RANGE\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

	FOCUS_RANGE_T sFocusRange;


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_AF_GET_RANGE,
                                NULL,
                                0,
                                (UINT8*)&sFocusRange,
                                sizeof(sFocusRange),
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    ACDK_LOGD("Focus Range = %d to %d\n", sFocusRange.i4InfPos, sFocusRange.i4MacroPos);

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AF_SAVE_TO_NVRAM
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAFSaveToNVRAM_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AF_SAVE_TO_NVRAM\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================



    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_AF_SAVE_TO_NVRAM,
                                NULL,
                                0,
                                NULL,
                                0,
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AF_READ
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAFRead_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AF_READ\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

	NVRAM_LENS_PARA_STRUCT sLensPara;


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_AF_READ,
                                NULL,
                                0,
								(UINT8*)&sLensPara,
								sizeof(sLensPara),
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    ACDK_LOGD("[Read AF Para]\n");
    ACDK_LOGD("[Thres Main]%d\n", sLensPara.rAFNVRAM.i4AF_THRES_MAIN);
    ACDK_LOGD("[Thres Sub]%d\n", sLensPara.rAFNVRAM.i4AF_THRES_SUB);
    ACDK_LOGD("[Thres Offset]%d\n", sLensPara.rAFNVRAM.i4AF_THRES_OFFSET);
    ACDK_LOGD("[Inf Pos]%d\n", sLensPara.rFocusRange.i4InfPos);
    ACDK_LOGD("[Macro Pos]%d\n", sLensPara.rFocusRange.i4MacroPos);

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AF_APPLY
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAFApply_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AF_APPLY\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

	NVRAM_LENS_PARA_STRUCT sLensPara;

    sLensPara.rAFNVRAM.i4AF_THRES_MAIN = (MINT32)atoi((char *)a_pprArgv[0]);
    sLensPara.rAFNVRAM.i4AF_THRES_SUB = (MINT32)atoi((char *)a_pprArgv[1]);


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_AF_APPLY,
							    (UINT8*)&sLensPara,
								sizeof(sLensPara),
                                NULL,
                                0,
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AF_GET_FV
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAFGetFV_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AF_GET_FV\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

	ACDK_AF_POS_T sAFPos;
	ACDK_AF_VLU_T sAFVlu;

	sAFPos.i4Num = 3;
	sAFPos.i4Pos[0] = (MINT32)atoi((char *)a_pprArgv[0]);
	sAFPos.i4Pos[1] = (MINT32)atoi((char *)a_pprArgv[1]);
	sAFPos.i4Pos[2] = (MINT32)atoi((char *)a_pprArgv[2]);
	//sAFPos.i4Pos[3] = (MINT32)atoi(a_pprArgv[3]);
	//sAFPos.i4Pos[4] = (MINT32)atoi(a_pprArgv[4]);
	//sAFPos.i4Pos[5] = (MINT32)atoi(a_pprArgv[5]);

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_AF_GET_FV,
							    (UINT8*)&sAFPos,
								sizeof(sAFPos),
								(UINT8*)&sAFVlu,
								sizeof(sAFVlu),
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

	for (MINT32 i=0; i<sAFVlu.i4Num; i++)
	{
    	ACDK_LOGD("[Pos] %4d, [Vlu] %11u\n", sAFPos.i4Pos[i], sAFVlu.u4Vlu[i]);
	}

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_FLASH_ENABLE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPFLASHEnable_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_FLASH_ENABLE\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_OP_FLASH_ENABLE,
                                NULL,
                                0,
                                NULL,
                                0,
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;

}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_FLASH_DISABLE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPFLASHDisable_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
     if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_FLASH_DISABLE\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================



    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_OP_FLASH_DISABLE,
                                NULL,
                                0,
                                NULL,
                                0,
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_FLASH_GET_INFO
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPFLASHGetInfo_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_FLASH_GET_INFO\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    MUINT32 u4RetLen = 0;
    MINT32 i4FlashEnable;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_OP_FLASH_GET_INFO,
                                NULL,
                                0,
                                (UINT8 *)&i4FlashEnable,
                                sizeof(MINT32),
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    ACDK_LOGD("i4FlashEnable = %d\n", i4FlashEnable);

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_ENABLE_DYNAMIC_CCM
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPEnableDyanmicCCM_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_ENABLE_DYNAMIC_CCM\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToACDK(
        ACDK_CCT_V2_OP_AWB_ENABLE_DYNAMIC_CCM,
        NULL, 0,
        NULL, 0, NULL
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_DISABLE_DYNAMIC_CCM
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPDisableDynamicCCM_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_DISABLE_DYNAMIC_CCM\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToACDK(
        ACDK_CCT_V2_OP_AWB_DISABLE_DYNAMIC_CCM, 
        NULL, 0,
        NULL, 0, NULL
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_GET_CCM_PARA
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetCCMPara_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_GET_CCM_PARA\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_NVRAM_CCM_PARA ccms;
    ::memset(&ccms, 0, sizeof(ccms));

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToACDK(
        ACDK_CCT_V2_OP_AWB_GET_CCM_PARA,
        NULL, 0, 
        &ccms, sizeof(ccms), 
        &u4RetLen
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    ACDK_LOGD("Get CCM Paras\n");
    for (MUINT32 i = 0; i < 3; i++)
    {
        printf("CCM:%d\n", i);
        ACDK_CCT_CCM_STRUCT& ccm = ccms.ccm[i];
        printf("RESERVED0 0x%02X\n", ccm.RESERVED0);
        printf("M11 M12 M13 : 0x%02X 0x%02X 0x%02X\n", ccm.M11, ccm.M12, ccm.M13);
        printf("RESERVED1 0x%02X\n", ccm.RESERVED1);
        printf("M21 M22 M23 : 0x%02X 0x%02X 0x%02X\n", ccm.M21, ccm.M22, ccm.M23);
        printf("RESERVED2 0x%02X\n", ccm.RESERVED2);
        printf("M31 M32 M33 : 0x%02X 0x%02X 0x%02X\n", ccm.M31, ccm.M32, ccm.M33);
        printf("\n");
    }
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_GET_CCM_STATUS
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetCCMStatus_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_GET_CCM_STATUS\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_FUNCTION_ENABLE_STRUCT rCCMStatus;
    ::memset(&rCCMStatus, 0, sizeof(rCCMStatus));

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToACDK(
        ACDK_CCT_V2_OP_AWB_GET_CCM_STATUS,
        NULL, 0, 
        &rCCMStatus, sizeof(rCCMStatus), 
        &u4RetLen
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    ACDK_LOGD("CCM Status:%d\n", rCCMStatus.Enable);

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_GET_CURRENT_CCM
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetCurrentCCM_Cmd(const MUINT32 a_u4Argc, MUINT8*a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_GET_CURRENT_CCM\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_CCM_STRUCT ccm;
    ::memset(&ccm, 0, sizeof(ccm));
    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToACDK(
        ACDK_CCT_V2_OP_AWB_GET_CURRENT_CCM,
        NULL, 0,
        &ccm, sizeof(ccm), &u4RetLen
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    ACDK_LOGD("Get Current CCM \n");
    printf("RESERVED0 0x%02X\n", ccm.RESERVED0);
    printf("M11 M12 M13 : 0x%02X 0x%02X 0x%02X\n", ccm.M11, ccm.M12, ccm.M13);
    printf("RESERVED1 0x%02X\n", ccm.RESERVED1);
    printf("M21 M22 M23 : 0x%02X 0x%02X 0x%02X\n", ccm.M21, ccm.M22, ccm.M23);
    printf("RESERVED2 0x%02X\n", ccm.RESERVED2);
    printf("M31 M32 M33 : 0x%02X 0x%02X 0x%02X\n", ccm.M31, ccm.M32, ccm.M33);
    printf("\n");
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_GET_NVRAM_CCM
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetNVRAMCCM_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_GET_NVRAM_CCM\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    //  in
    MUINT32 u4Index = 1;
    //  out
    ACDK_CCT_CCM_STRUCT ccm;
    ::memset(&ccm, 0, sizeof(ccm));

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToACDK(
        ACDK_CCT_V2_OP_AWB_GET_NVRAM_CCM,
        &u4Index, sizeof(u4Index),
        &ccm, sizeof(ccm),
        &u4RetLen
    );


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    ACDK_LOGD("Get NVram CCM\n");
    ACDK_LOGD("Light Mode:%d\n", u4Index);
    ACDK_LOGD("CCM Matrix\n");
    printf("RESERVED0 0x%02X\n", ccm.RESERVED0);
    printf("M11 M12 M13 : 0x%02X 0x%02X 0x%02X\n", ccm.M11, ccm.M12, ccm.M13);
    printf("RESERVED1 0x%02X\n", ccm.RESERVED1);
    printf("M21 M22 M23 : 0x%02X 0x%02X 0x%02X\n", ccm.M21, ccm.M22, ccm.M23);
    printf("RESERVED2 0x%02X\n", ccm.RESERVED2);
    printf("M31 M32 M33 : 0x%02X 0x%02X 0x%02X\n", ccm.M31, ccm.M32, ccm.M33);
    printf("\n");
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_SET_CURRENT_CCM
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetCurrentCCM_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_SET_CURRENT_CCM\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_CCM_STRUCT ccm;
    ::memset(&ccm, 0, sizeof(ccm));

    ccm.M11 = 0x3B;
    ccm.M12 = 0xA5;
    ccm.M13 = 0x0A;

    ccm.M21 = 0x88;
    ccm.M22 = 0x2d;
    ccm.M23 = 0x85;

    ccm.M31 = 0x84;
    ccm.M32 = 0x97;
    ccm.M33 = 0x3b;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToACDK(
        ACDK_CCT_V2_OP_AWB_SET_CURRENT_CCM,
        &ccm, 
        sizeof(ccm),
        NULL, 0, NULL
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// MSDK_CCT_V2_OP_AWB_SET_NVRAM_CCM
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetNVRAMCCM_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("MSDK_CCT_V2_OP_AWB_SET_NVRAM_CCM\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_SET_NVRAM_CCM  inCCM;
    //! Fixme
    //Just for test, fill the right data from CCT
    inCCM.u4Index = 1;

    ::memset(&inCCM.ccm, 0, sizeof(inCCM.ccm));
    inCCM.ccm.M11 = 0x20;
    inCCM.ccm.M12 = 0x00;
    inCCM.ccm.M13 = 0x00;

    inCCM.ccm.M21 = 0x00;
    inCCM.ccm.M22 = 0x20;
    inCCM.ccm.M23 = 0x00;

    inCCM.ccm.M31 = 0x00;
    inCCM.ccm.M32 = 0x00;
    inCCM.ccm.M33 = 0x20;

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToACDK(
        ACDK_CCT_V2_OP_AWB_SET_NVRAM_CCM,
        &inCCM,
        sizeof(inCCM),
        NULL, 0, NULL
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_UPDATE_CCM_PARA
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPUpdateCCMPara_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_UPDATE_CCM_PARA\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_NVRAM_CCM_PARA ccms;
    ::memset(&ccms, 0, sizeof(ccms));

    //lightsource 1
    ACDK_CCT_CCM_STRUCT& rCcm0 = ccms.ccm[0];
    rCcm0.M11 = 0x3b;
    rCcm0.M12 = 0xa5;
    rCcm0.M13 = 0x0a;
    rCcm0.M21 = 0x88;
    rCcm0.M22 = 0x2d;
    rCcm0.M23 = 0x85;
    rCcm0.M31 = 0x84;
    rCcm0.M32 = 0x97;
    rCcm0.M33 = 0x3b;
    //lightsource 2
    ACDK_CCT_CCM_STRUCT& rCcm1 = ccms.ccm[1];
    rCcm1.M11 = 0x31;
    rCcm1.M12 = 0x8d;
    rCcm1.M13 = 0x84;
    rCcm1.M21 = 0x80;
    rCcm1.M22 = 0x29;
    rCcm1.M23 = 0x89;
    rCcm1.M31 = 0x00;
    rCcm1.M32 = 0x87;
    rCcm1.M33 = 0x26;
    //lightsource 3
    ACDK_CCT_CCM_STRUCT& rCcm2 = ccms.ccm[2];
    rCcm2.M11 = 0x39;
    rCcm2.M12 = 0x91;
    rCcm2.M13 = 0x88;
    rCcm2.M21 = 0x8b;
    rCcm2.M22 = 0x3c;
    rCcm2.M23 = 0x91;
    rCcm2.M31 = 0x86;
    rCcm2.M32 = 0x93;
    rCcm2.M33 = 0x39;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToACDK(
        ACDK_CCT_V2_OP_AWB_UPDATE_CCM_PARA, 
        &ccms, sizeof(ccms), 
        NULL, 0, NULL
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    return S_ACDK_CCAP_OK;

}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_UPDATE_CCM_STATUS
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPUpdateCCMStatus_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_UPDATE_CCM_STATUS\n");
    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: 510 <On/Off, ON:1, OFF:0>\n");
        return E_ACDK_IF_API_FAIL;
    }
    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_FUNCTION_ENABLE_STRUCT ccmStatus;

    if (atoi((char *)a_pprArgv[0]) == 0)
    {
        ccmStatus.Enable = FALSE;
    }
    else
    {
        ccmStatus.Enable = TRUE;
    }

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToACDK(
        ACDK_CCT_V2_OP_AWB_UPDATE_CCM_STATUS,
        &ccmStatus, sizeof(ccmStatus),
        NULL, 0, NULL
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_SET_CCM_MODE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetCCMMode_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_SET_CCM_MODE\n");
    if (1 != a_u4Argc)
    {
        ACDK_LOGD("Usage: setCCMMode <index 0~2>\n");
        return E_ACDK_IF_API_FAIL;
    }
    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MUINT32 u4Index = atoi((char *)a_pprArgv[0]);

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToACDK(
        ACDK_CCT_OP_SET_CCM_MODE,
        &u4Index, sizeof(u4Index),
        NULL, 0, NULL
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_GET_CCM_MODE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetCCMMode_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_GET_CCM_MODE\n");
    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MUINT32 u4Index = 0;

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToACDK(
        ACDK_CCT_OP_GET_CCM_MODE,
        NULL, 0, 
        &u4Index, sizeof(u4Index), &u4RetLen
    );
    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    ACDK_LOGD("CCM Index = %d\n", u4Index);

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_OP_AE_SET_GAMMA_BYPASS
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetGammaBypass_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("FT_MSDK_CCT_V2_OP_AE_SET_GAMMA_BYPASS\n");
    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: 601 <On/Off, ON:1, OFF:0>\n");
        return E_ACDK_IF_API_FAIL;
    }
    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_FUNCTION_ENABLE_STRUCT gammaCtrl;
    memset(&gammaCtrl, 0, sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT));

    if (atoi((char *)a_pprArgv[0]) == 0)
    {
        gammaCtrl.Enable = FALSE;
    }
    else
    {
        gammaCtrl.Enable = TRUE;
    }

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_AE_SET_GAMMA_BYPASS,
                                                        (UINT8*)&gammaCtrl,
                                                        sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT),
                                                        NULL,
                                                        0,
                                                        &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_OP_AE_GET_GAMMA_BYPASS_FLAG
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetGammaBypassFlag_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("FT_MSDK_CCT_V2_OP_AE_GET_GAMMA_BYPASS_FLAG\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_FUNCTION_ENABLE_STRUCT gammCtrlStatus;
    memset(&gammCtrlStatus, 0, sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT));

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_AE_GET_GAMMA_BYPASS_FLAG,
                                                        NULL,
                                                        0,
                                                        (UINT8*)&gammCtrlStatus,
                                                        sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT),
                                                        &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    ACDK_LOGD("Gamma Bypass:%d\n", gammCtrlStatus.Enable);

    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_OP_AE_GET_GAMMA_TABLE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetGammaTable_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("FT_MSDK_CCT_V2_OP_AE_GET_GAMMA_TABLE\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_GAMMA_TABLE_STRUCT gammaTable;
    memset(&gammaTable, 0, sizeof(ACDK_CCT_GAMMA_TABLE_STRUCT));

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_AE_GET_GAMMA_TABLE,
                                                        NULL,
                                                        0,
                                                        (UINT8 *)&gammaTable,
                                                        sizeof(ACDK_CCT_GAMMA_TABLE_STRUCT),
                                                        &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
   ACDK_LOGD("Gamma Table:\n");
   for (int i = 0; i < GAMMA_STEP_NO; i++)
   {
        printf("%d ", gammaTable.gamma[0][i]);
   }
   printf("\n");

   return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AE_SET_GAMMA_TABLE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetGammaTable_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AE_GET_GAMMA_TABLE\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_GAMMA_TABLE_STRUCT gammaTable;
    memset(&gammaTable, 0, sizeof(ACDK_CCT_GAMMA_TABLE_STRUCT));

    gammaTable.gamma[0][0] = 7;
    gammaTable.gamma[0][1] = 17;            //GMA_Y04(32)
    gammaTable.gamma[0][2] = 36;           //GMA_Y08(50)
    gammaTable.gamma[0][3] = 65;            //GMA_Y16(73)
    gammaTable.gamma[0][4] = 87;            //GMA_Y24(88)
    gammaTable.gamma[0][5] = 107;           //GMA_Y32(100)
    gammaTable.gamma[0][6] = 123;           //GMA_Y40(111)
    gammaTable.gamma[0][7] = 136;           //GMA_Y48(120)
    gammaTable.gamma[0][8] = 148;           //GMA_Y56(129)
    gammaTable.gamma[0][9] = 159;           //GMA_Y64(137)
    gammaTable.gamma[0][10] = 178;           //GMA_Y80(151)
    gammaTable.gamma[0][11] = 190;           //GMA_Y96(164)
    gammaTable.gamma[0][12] = 201;           //GMA_Y112(176)
    gammaTable.gamma[0][13] = 210;           //GMA_Y128(187)
    gammaTable.gamma[0][14] = 217;           //GMA_Y144(197)
    gammaTable.gamma[0][15] = 225;           //GMA_Y160(206)
    gammaTable.gamma[0][16] = 234;           //GMA_Y192(224)
    gammaTable.gamma[0][17] = 241;           //GMA_Y208(232)
    gammaTable.gamma[0][18] = 245;           //GMA_Y224(240)
    gammaTable.gamma[0][19] = 250;           //GMA_Y240(248)


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_AE_SET_GAMMA_TABLE,
                                                        (UINT8 *)&gammaTable,
                                                        sizeof(ACDK_CCT_GAMMA_TABLE_STRUCT),
                                                        NULL,
                                                        0,
                                                        &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    return S_ACDK_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_ISP_SET_PCA_TABLE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetPcaTable_Cmd(const MUINT32 a_u4Argc, MUINT8*a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_ISP_SET_PCA_TABLE\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_ACCESS_NVRAM_PCA_TABLE access_set;
    ::memset(&access_set, 0, sizeof(access_set));

    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: setPcaTable <color temperature: 0~2 >\n");
        return E_ACDK_IF_API_FAIL;
    }
    access_set.u4Offset = 0;
    access_set.u4Count  = PCA_BIN_NUM;
    access_set.u8ColorTemperature = ::atoi((char *)a_pprArgv[0]);
    for (MUINT32 i = 0; i < access_set.u4Count; i++)
    {
        ISP_NVRAM_PCA_BIN_T* p = &access_set.buffer[i];
        p->hue_shift= rand();
        p->sat_gain = rand();
        p->y_gain   = rand();
    }

    ACDK_CCT_ACCESS_NVRAM_PCA_TABLE access_get;
    ::memset(&access_get, 0, sizeof(access_get));
    access_get.u4Offset = access_set.u4Offset;
    access_get.u4Count  = access_set.u4Count;
    access_get.u8ColorTemperature = access_set.u8ColorTemperature;

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet_set = ::bSendDataToACDK(
        ACDK_CCT_OP_ISP_SET_PCA_TABLE,
        &access_set, sizeof(access_set), 
        NULL, 0, NULL
    );

    BOOL bRet_get = ::bSendDataToACDK(
        ACDK_CCT_OP_ISP_GET_PCA_TABLE,
        NULL, 0, 
        &access_get, sizeof(access_get), &u4RetLen
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if  (
            ! bRet_set
        ||  ! bRet_get
        ||  sizeof(access_get) != u4RetLen
        )
    {
        ACDK_LOGE("[ERR][ACDK_CCT_OP_ISP_SET_PCA_TABLE] "
        "(bRet_set, bRet_get, u4RetLen, sizeof(access_get))=(%d, %d, %d, %d)\n", 
        bRet_set, bRet_get, u4RetLen, sizeof(access_get));
        return E_ACDK_CCAP_API_FAIL;
    }

    for (MUINT32 i = 0; i < access_set.u4Count; i++)
    {
        ISP_NVRAM_PCA_BIN_T* p_set = &access_set.buffer[i];
        ISP_NVRAM_PCA_BIN_T* p_get = &access_get.buffer[i];

        if  ( ::memcmp(p_set, p_get, sizeof(MUINT32)) )
        {
            printf("[%d] Mismatch \n", i);
            printf("set[%d %d %d %d] ", p_set->reserved, p_set->hue_shift, p_set->sat_gain, p_set->y_gain);
            printf("get[%d %d %d %d] ", p_get->reserved, p_get->hue_shift, p_get->sat_gain, p_get->y_gain);
            return E_ACDK_CCAP_API_FAIL;
        }
    }
    printf("Compare: OK\n");
    return S_ACDK_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_ISP_GET_PCA_TABLE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetPcaTable_Cmd(const MUINT32 a_u4Argc, MUINT8*a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_ISP_GET_PCA_TABLE\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MUINT32 u4RetLen = 0;
    ACDK_CCT_ACCESS_NVRAM_PCA_TABLE access;
    ::memset(&access, 0, sizeof(access));

    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: getPcaTable <color temperature: 0~2 >\n");
        return E_ACDK_IF_API_FAIL;
    }
    access.u4Offset = 0;
    access.u4Count  = PCA_BIN_NUM;
    access.u8ColorTemperature = ::atoi((char *)a_pprArgv[0]);

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToACDK(
        ACDK_CCT_OP_ISP_GET_PCA_TABLE,
        NULL, 0,
        &access, sizeof(access), &u4RetLen
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if  (
            ! bRet
        ||  sizeof(access) != u4RetLen
        )
    {
        ACDK_LOGE("[ERR][ACDK_CCT_OP_ISP_GET_PCA_TABLE] (bRet, u4RetLen, sizeof(access))=(%d, %d, %d)\n", bRet, u4RetLen, sizeof(access));
        return E_ACDK_CCAP_API_FAIL;
    }

    for (MUINT32 i = 0; i < access.u4Count; i++)
    {
        if  (0==i%10)
            printf("\n");

        ISP_NVRAM_PCA_BIN_T* p = &access.buffer[i];
        printf("[%08X] ", *((MUINT32*)p));
    }
    printf("\n");

    return S_ACDK_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_ISP_SET_PCA_PARA
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetPcaPara_Cmd(const MUINT32 a_u4Argc, MUINT8*a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_ISP_SET_PCA_PARA\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_ACCESS_PCA_CONFIG access;
    ::memset(&access, 0, sizeof(access));

    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: setPcaPara <On/Off, ON:1, OFF:0>\n");
        return E_ACDK_IF_API_FAIL;
    }

    access.EN = ::atoi((char *)a_pprArgv[0]);

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToACDK(
        ACDK_CCT_OP_ISP_SET_PCA_PARA,
        &access, sizeof(access), 
        NULL, 0, NULL
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if  (! bRet)
    {
        ACDK_LOGE("[ERR][ACDK_CCT_OP_ISP_SET_PCA_PARA] (bRet)=(%d)\n", bRet);
        return E_ACDK_CCAP_API_FAIL;
    }

    printf("PCA EN : %d\n", access.EN);
    printf("\n");

    return S_ACDK_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_ISP_GET_PCA_PARA
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetPcaPara_Cmd(const MUINT32 a_u4Argc, MUINT8*a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_ISP_GET_PCA_PARA\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MUINT32 u4RetLen = 0;
    ACDK_CCT_ACCESS_PCA_CONFIG access;
    ::memset(&access, 0, sizeof(access));

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToACDK(
        ACDK_CCT_OP_ISP_GET_PCA_PARA,
        NULL, 0,
        &access, sizeof(access), &u4RetLen
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if  (
            ! bRet
        ||  sizeof(access) != u4RetLen
        )
    {
        ACDK_LOGE("[ERR][ACDK_CCT_OP_ISP_GET_PCA_PARA] (bRet, u4RetLen, sizeof(access))=(%d, %d, %d)\n", bRet, u4RetLen, sizeof(access));
        return E_ACDK_CCAP_API_FAIL;
    }

    printf("PCA EN : %d\n", access.EN);
    printf("\n");

    return S_ACDK_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_SET_OB_ON_OFF
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetOBOnOff_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_SET_OB_ON_OFF\n");
    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: setOB <On/Off, ON:1, OFF:1>\n");
        return E_ACDK_IF_API_FAIL;
    }

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_FUNCTION_ENABLE_STRUCT OBCtrl;
    ::memset(&OBCtrl, 0, sizeof(OBCtrl));
    OBCtrl.Enable = ::atoi((char *)a_pprArgv[0]);

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToACDK(
        ACDK_CCT_V2_OP_SET_OB_ON_OFF, 
        &OBCtrl, sizeof(OBCtrl), 
        NULL, 0, NULL
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    return S_ACDK_CCAP_OK;
}
/*
/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_SAVE_OB_ON_OFF
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSaveOBOnOff_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_SAVE_OB_ON_OFF\n");
    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: saveOB value\n");
        return E_ACDK_IF_API_FAIL;
    }

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MUINT32 u4RetLen = 0;
    MUINT32 u4SaveOBValue = 0;

    u4SaveOBValue = atoi((char *)a_pprArgv[0]);

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_SAVE_OB_ON_OFF,
                                                        (UINT8 *)&u4SaveOBValue,
                                                        sizeof(MUINT32),
                                                        NULL,
                                                        0,
                                                        &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    ACDK_LOGD("Save OB Value:%d\n", u4SaveOBValue);

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    return S_ACDK_CCAP_OK;
}
*/
/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_GET_OB_ON_OFF
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetOBOnOff_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_GET_OB_ON_OFF\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_FUNCTION_ENABLE_STRUCT OBCtrl;
    ::memset(&OBCtrl, 0, sizeof(OBCtrl));

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToACDK(
        ACDK_CCT_V2_OP_GET_OB_ON_OFF, 
        NULL, 0, 
        &OBCtrl, sizeof(OBCtrl), 
        &u4RetLen
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    ACDK_LOGD("OB Mode:%d\n", OBCtrl.Enable);
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_SET_NR_ON_OFF
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetNROnOff_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDk_CCT_V2_OP_SET_NR_ON_OFF\n");
    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: setNR <On/Off, ON:1, OFF:1>\n");
        return E_ACDK_IF_API_FAIL;
    }

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_FUNCTION_ENABLE_STRUCT NRCtrl;
    NRCtrl.Enable = ::atoi((char *)a_pprArgv[0]);

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToACDK(
        ACDK_CCT_V2_OP_SET_NR_ON_OFF, 
        &NRCtrl, sizeof(NRCtrl), 
        NULL, 0, NULL
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_GET_NR_ON_OFF
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetNROnOff_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_GET_NR_ON_OFF\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_FUNCTION_ENABLE_STRUCT NRCtrl;
    ::memset(&NRCtrl, 0, sizeof(NRCtrl));

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToACDK(
        ACDK_CCT_V2_OP_GET_NR_ON_OFF, 
        NULL, 0, 
        &NRCtrl, sizeof(NRCtrl), 
        &u4RetLen
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    ACDK_LOGD("NR Mode:%d\n", NRCtrl.Enable);
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_SET_EE_ON_OFF
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetEEOnOff_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_SET_EE_ON_OFF\n");
    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: setEE <On/Off, ON:1, OFF:1>\n");
        return E_ACDK_IF_API_FAIL;
    }

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_FUNCTION_ENABLE_STRUCT EECtrl;
    EECtrl.Enable = ::atoi((char *)a_pprArgv[0]);

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToACDK(
        ACDK_CCT_V2_OP_SET_EE_ON_OFF, 
        &EECtrl, sizeof(EECtrl), 
        NULL, 0, NULL
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_GET_EE_ON_OFF
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetEEOnOff_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_GET_EE_ON_OFF\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_FUNCTION_ENABLE_STRUCT EECtrl;
    ::memset(&EECtrl, 0, sizeof(EECtrl));

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToACDK(
        ACDK_CCT_V2_OP_GET_EE_ON_OFF, 
        NULL, 0, 
        &EECtrl, sizeof(EECtrl), 
        &u4RetLen
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    ACDK_LOGD("(EECtrl.Enable, u4RetLen)=(%d, %d)\n", EECtrl.Enable, u4RetLen);
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_SET_ISP_ON
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetIspOn_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_SET_ISP_ON\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    if (1 != a_u4Argc)
    {
        ACDK_LOGD("Usage: setIspOnOn <category> \n");
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_CCT_ISP_REG_CATEGORY eCategory = static_cast<ACDK_CCT_ISP_REG_CATEGORY>(::atoi((char *)a_pprArgv[0]));

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToACDK(
        ACDK_CCT_OP_SET_ISP_ON, 
        &eCategory, sizeof(eCategory), 
        NULL, 0, NULL
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    ACDK_LOGD("(eCategory)=(%d)\n", eCategory);
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_SET_ISP_OFF
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetIspOff_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_SET_ISP_OFF\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    if (1 != a_u4Argc)
    {
        ACDK_LOGD("Usage: setIspOnOff <category> \n");
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_CCT_ISP_REG_CATEGORY eCategory = static_cast<ACDK_CCT_ISP_REG_CATEGORY>(::atoi((char *)a_pprArgv[0]));

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToACDK(
        ACDK_CCT_OP_SET_ISP_OFF, 
        &eCategory, sizeof(eCategory), 
        NULL, 0, NULL
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    ACDK_LOGD("(eCategory)=(%d)\n", eCategory);
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_GET_ISP_ON_OFF
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetIspOnOff_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_GET_ISP_ON_OFF\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    if (1 != a_u4Argc)
    {
        ACDK_LOGD("Usage: getIspOnOff <category> \n");
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_CCT_ISP_REG_CATEGORY eCategory = static_cast<ACDK_CCT_ISP_REG_CATEGORY>(::atoi((char *)a_pprArgv[0]));
    ACDK_CCT_FUNCTION_ENABLE_STRUCT Ctrl;
    ::memset(&Ctrl, 0, sizeof(Ctrl));

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToACDK(
        ACDK_CCT_OP_GET_ISP_ON_OFF, 
        &eCategory, sizeof(eCategory), 
        &Ctrl, sizeof(Ctrl), 
        &u4RetLen
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    ACDK_LOGD("(u4RetLen, eCategory, Ctrl.Enable)=(%d, %d, %d)\n", u4RetLen, eCategory, Ctrl.Enable);
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_ISP_SET_SHADING_INDEX
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetShadingIndex_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
     if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_ISP_SET_SHADING_INDEX\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: setLSCIndex <Index> \n");
        return E_ACDK_IF_API_FAIL;
    }

    UINT8 index = (UINT8)atoi((char *)a_pprArgv[0]);

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_ISP_SET_SHADING_INDEX,
                                                        (UINT8*)&index,
                                                        sizeof(UINT8),
                                                        NULL,
                                                        0,
                                                        &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }
    return S_ACDK_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_ISP_GET_SHADING_INDEX
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetShadingIndex_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
     if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_ISP_GET_SHADING_INDEX\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    UINT8 index = 0;

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_V2_OP_ISP_GET_SHADING_INDEX,
                                                        NULL,
                                                        0,
                                                        (UINT8*)&index,
                                                        sizeof(UINT8),
                                                        &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    ACDK_LOGD("Shading Index:%d\n", index);
    return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_CDVT_SENSOR_TEST
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPCDVTSensorTest_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
     if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_CDVT_SENSOR_TEST\n");

    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: SensorTest <Test Item: 0 ~ 4>\n");
		ACDK_LOGD("[0] Exposure Linearity \n");
		ACDK_LOGD("[1] Gain Linearity (Gain Config)\n");
		ACDK_LOGD("[2] Gain Linearity (Gain Table)\n");
		ACDK_LOGD("[3] OB Stability (Gain Config)\n");
		ACDK_LOGD("[4] OB Stability (Gain Table)\n");
        return E_ACDK_IF_API_FAIL;
    }

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MUINT32 u4RetLen = 0;
    MUINT32 u4TestItem = (MUINT32) atoi((char *)a_pprArgv[0]);

    if ((u4TestItem > 4))
    {
        ACDK_LOGD("Un-support test item\n");
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_CDVT_SENSOR_TEST_INPUT_T rSensorTestInput;

    switch(u4TestItem)
    {
    case 0: // [0] Exposure Linearity
    	ACDK_LOGD("[0] Exposure Linearity \n");
        rSensorTestInput.eTestItem = ACDK_CDVT_TEST_EXPOSURE_LINEARITY;
	    rSensorTestInput.eSensorMode = ACDK_CDVT_SENSOR_MODE_PREVIEW;
	    rSensorTestInput.rExpLinearity.eExpMode = ACDK_CDVT_EXP_MODE_TIME;
	    rSensorTestInput.rExpLinearity.i4Gain = 1024; // 1x
	    rSensorTestInput.rExpLinearity.i4ExpStart = 33333; // 1/30 sec
	    rSensorTestInput.rExpLinearity.i4ExpEnd = 66666; // 1/15 sec
	    rSensorTestInput.rExpLinearity.i4ExpInterval = 33333; // 1/30 sec
        break;
	case 1:	// [1] Gain Linearity (Gain Config)
		ACDK_LOGD("[1] Gain Linearity (Gain Config)\n");
	    rSensorTestInput.eTestItem = ACDK_CDVT_TEST_GAIN_LINEARITY;
	    rSensorTestInput.eSensorMode = ACDK_CDVT_SENSOR_MODE_PREVIEW;
		rSensorTestInput.rGainLinearityOBStability.eGainControlMode = ACDK_CDVT_GAIN_CONFIG;
		rSensorTestInput.rGainLinearityOBStability.i4ExpTime = 33333; // 1/30 sec
		rSensorTestInput.rGainLinearityOBStability.i4GainStart = 1024; // 1x
		rSensorTestInput.rGainLinearityOBStability.i4GainEnd = 2048; // 2x
		rSensorTestInput.rGainLinearityOBStability.i4GainInterval = 1024; // 1x
		break;
	case 2:	// [2] Gain Linearity (Gain Table)
		ACDK_LOGD("[2] Gain Linearity (Gain Table)\n");
	    rSensorTestInput.eTestItem = ACDK_CDVT_TEST_GAIN_LINEARITY;
	    rSensorTestInput.eSensorMode = ACDK_CDVT_SENSOR_MODE_PREVIEW;
		rSensorTestInput.rGainLinearityOBStability.eGainControlMode = ACDK_CDVT_GAIN_TABLE;
		rSensorTestInput.rGainLinearityOBStability.i4ExpTime = 33333; // 1/30 sec
		rSensorTestInput.rGainLinearityOBStability.i4GainTableSize = 2;
		rSensorTestInput.rGainLinearityOBStability.i4GainTable[0] = 1024;
		rSensorTestInput.rGainLinearityOBStability.i4GainTable[1] = 2048;
		break;
	case 3:	// [3] OB Stability (Gain Config)
		ACDK_LOGD("[3] OB Stability (Gain Config)\n");
	    rSensorTestInput.eTestItem = ACDK_CDVT_TEST_OB_STABILITY;
	    rSensorTestInput.eSensorMode = ACDK_CDVT_SENSOR_MODE_PREVIEW;
		rSensorTestInput.rGainLinearityOBStability.eGainControlMode = ACDK_CDVT_GAIN_CONFIG;
		rSensorTestInput.rGainLinearityOBStability.i4ExpTime = 33333; // 1/30 sec
		rSensorTestInput.rGainLinearityOBStability.i4GainStart = 1024; // 1x
		rSensorTestInput.rGainLinearityOBStability.i4GainEnd = 2048; // 2x
		rSensorTestInput.rGainLinearityOBStability.i4GainInterval = 1024; // 1x
		break;
	case 4:	// [4] OB Stability (Gain Table)
	    ACDK_LOGD("[4] OB Stability (Gain Table)\n");
	    rSensorTestInput.eTestItem = ACDK_CDVT_TEST_OB_STABILITY;
	    rSensorTestInput.eSensorMode = ACDK_CDVT_SENSOR_MODE_PREVIEW;
		rSensorTestInput.rGainLinearityOBStability.eGainControlMode = ACDK_CDVT_GAIN_TABLE;
		rSensorTestInput.rGainLinearityOBStability.i4ExpTime = 33333; // 1/30 sec
		rSensorTestInput.rGainLinearityOBStability.i4GainTableSize = 2;
		rSensorTestInput.rGainLinearityOBStability.i4GainTable[0] = 1024;
		rSensorTestInput.rGainLinearityOBStability.i4GainTable[1] = 2048;
		break;
    }

	ACDK_CDVT_SENSOR_TEST_OUTPUT_T rSensorTestOutput;
    memset (&rSensorTestOutput, 0, sizeof(ACDK_CDVT_SENSOR_TEST_OUTPUT_T));

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_OP_CDVT_SENSOR_TEST,
                                (UINT8*)&rSensorTestInput,
                                sizeof(ACDK_CDVT_SENSOR_TEST_INPUT_T),
                                (UINT8*)&rSensorTestOutput,
                                sizeof(ACDK_CDVT_SENSOR_TEST_OUTPUT_T),
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

    ACDK_LOGD("rSensorTestOutput.i4ErrorCode = %d\n", rSensorTestOutput.i4ErrorCode);
    ACDK_LOGD("rSensorTestOutput.i4TestCount = %d\n", rSensorTestOutput.i4TestCount);

	for (MINT32 i = 0; i < rSensorTestOutput.i4TestCount; i++)
	{
		ACDK_LOGD("[%d] R = %4.2f; Gr = %4.2f; Gb = %4.2f; B = %4.2f; Median = %d\n",
			      i,
                  rSensorTestOutput.rRAWAnalysisResult[i].fRAvg,
                  rSensorTestOutput.rRAWAnalysisResult[i].fGrAvg,
                  rSensorTestOutput.rRAWAnalysisResult[i].fGbAvg,
                  rSensorTestOutput.rRAWAnalysisResult[i].fBAvg,
                  rSensorTestOutput.rRAWAnalysisResult[i].u4Median);
	}

	return S_ACDK_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_CDVT_SENSOR_CALIBRATION
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPCDVTSensorCalibration_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
     if (!g_bAcdkOpend )
    {
        return E_ACDK_IF_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_CDVT_SENSOR_CALIBRATION\n");

	if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: SensorCal <Calibration Item: 0 ~ 3>\n");
		ACDK_LOGD("[0] OB Calibration \n");
		ACDK_LOGD("[1] Minimum ISO Calibration\n");
		ACDK_LOGD("[2] Minimum Saturation Gain Calibration\n");
        return E_ACDK_IF_API_FAIL;
    }

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MUINT32 u4RetLen = 0;
    MUINT32 u4CalItem = (MUINT32) atoi((char *)a_pprArgv[0]);

    if ((u4CalItem > 2))
    {
        ACDK_LOGD("Un-support calibration item\n");
        return E_ACDK_IF_API_FAIL;
    }


    ACDK_CDVT_SENSOR_CALIBRATION_INPUT_T rSensorCalibrationInput;

	switch(u4CalItem)
    {
    case 0: // [0] OB Calibration
		ACDK_LOGD("[0] OB Calibration \n");
        rSensorCalibrationInput.eCalibrationItem = ACDK_CDVT_CALIBRATION_OB;
	    rSensorCalibrationInput.eSensorMode = ACDK_CDVT_SENSOR_MODE_PREVIEW;
	    rSensorCalibrationInput.rOB.i4ExpTime = 33333; // 1/30 sec
	    rSensorCalibrationInput.rOB.i4Gain = 1024; // 1x
	    rSensorCalibrationInput.rOB.i4RepeatTimes = 10;
        break;
	case 1:	// [1] Minimum ISO Calibration
		ACDK_LOGD("[1] Minimum ISO Calibration\n");
        rSensorCalibrationInput.eCalibrationItem = ACDK_CDVT_CALIBRATION_MIN_ISO;
	    rSensorCalibrationInput.eSensorMode = ACDK_CDVT_SENSOR_MODE_PREVIEW;
	    rSensorCalibrationInput.rMinISO.i4LV = 90; // LV 9
        rSensorCalibrationInput.rMinISO.i4FNumber = 28; // F2.8
        rSensorCalibrationInput.rMinISO.eFlicker = ACDK_CDVT_FLICKER_60_HZ;
		rSensorCalibrationInput.rMinISO.i4OB = 42;
		break;
	case 2:	// [2] Minimum Saturation Gain Calibration
		ACDK_LOGD("[2] Minimum Saturation Gain Calibration\n");
        rSensorCalibrationInput.eCalibrationItem = ACDK_CDVT_CALIBRATION_MIN_SAT_GAIN;
	    rSensorCalibrationInput.eSensorMode = ACDK_CDVT_SENSOR_MODE_PREVIEW;
	    rSensorCalibrationInput.rMinSatGain.i4TargetDeclineRate = 10; // 10%
	    rSensorCalibrationInput.rMinSatGain.i4GainBuffer = 0; // 0%
	    rSensorCalibrationInput.rMinSatGain.eFlicker = ACDK_CDVT_FLICKER_60_HZ;
		rSensorCalibrationInput.rMinSatGain.i4OB = 42;
		break;
    }

	ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T rSensorCalibrationOutput;
    memset (&rSensorCalibrationOutput, 0, sizeof(ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T));

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToACDK(ACDK_CCT_OP_CDVT_SENSOR_CALIBRATION,
                                (UINT8*)&rSensorCalibrationInput,
                                sizeof(ACDK_CDVT_SENSOR_CALIBRATION_INPUT_T),
                                (UINT8*)&rSensorCalibrationOutput,
                                sizeof(ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T),
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_ACDK_CCAP_API_FAIL;
    }

	ACDK_LOGD("rSensorCalibrationOutput.i4ErrorCode = %d\n", rSensorCalibrationOutput.i4ErrorCode);
    ACDK_LOGD("rSensorCalibrationOutput.i4OB = %d\n", rSensorCalibrationOutput.i4OB);
	ACDK_LOGD("rSensorCalibrationOutput.i4MinISO = %d\n", rSensorCalibrationOutput.i4MinISO);
	ACDK_LOGD("rSensorCalibrationOutput.i4MinSatGain = %d\n", rSensorCalibrationOutput.i4MinSatGain);

    return S_ACDK_CCAP_OK;
}




/////////////////////////////////////////////////////////////////////////
//
//  ACDK CCAP CLI Command List  () -
//! @brief
//! @param
/////////////////////////////////////////////////////////////////////////
static const Acdk_CLICmd g_prAcdkCCAPCLICmds [] =
{
    //Camera Control
    {"prvstart",                      "FT_MSDK_CCT_OP_PREVIEW_LCD_START",       mrCCAPPreviewStart_Cmd},
    {"prvstop",                      "FT_MSDK__CCT_OP_PREVIEW_LCD_STOP",        mrCCAPPreviewStop_Cmd},
    {"cap",                            "FT_MSDK_CCT_OP_SINGLE_SHOT_CAPTURE, (cap <mode> <type>  <width (Option)> <height (Option)>)",   mrCCAPSingleShot_Cmd},
    {"multiCap",                    "FT_MSDK_CCT_OP_MULTI_SHOT_CAPTURE, (cap <mode <type> <cnt>)",     mrCCAPMultiShot_Cmd},
    {"lnvram",              "ACDK_CCT_OP_LOAD_FROM_NVRAM",          mrCCAPLoadFromNvram_Cmd},
    {"Snvram",              "ACDK_CCT_OP_SAVE_TO_NVRAM",            mrCCAPSaveToNvram_Cmd},
    //ISP Control
    {"qIspID",              "ACDK_CCT_OP_QUERY_ISP_ID",                 mrCCAPQueryISPID_Cmd},
    {"rIspReg",             "ACDK_CCT_OP_ISP_READ_REG",                 mrCCAPReadISPReg_Cmd},
    {"wIspReg",             "ACDK_CCT_OP_ISP_WRITE_REG",                mrCCAPWriteISPReg_Cmd},
    {"setIndex",            "ACDK_CCT_V2_OP_ISP_SET_TUNING_INDEX",      mrCCAPSetTuningIndex_Cmd},
    {"getIndex",            "ACDK_CCT_V2_OP_ISP_GET_TUNING_INDEX",      mrCCAPGetTuningIndex_Cmd},
    {"setParas",            "ACDK_CCT_V2_OP_ISP_SET_TUNING_PARAS",      mrCCAPSetTuningParas_Cmd},
    {"getParas",            "ACDK_CCT_V2_OP_ISP_GET_TUNING_PARAS",      mrCCAPGetTuningParas_Cmd},
    {"setShading",                "FT_MSDK_CCT_V2_OP_ISP_SET_SHADING_ON_OFF", mrCCAPSetShadingOnOff_Cmd},
    {"getShading",                "FT_MSDK_CCT_V2_OP_ISP_GET_SHADING_ON_OFF", mrCCAPGetShadingOnOff_Cmd},
    {"setShadingPara",          "FT_MSDK_CCT_V2_OP_ISP_SET_SHADING_PARA", mrCCAPSetShadingPara_Cmd},
    {"getShadingPara",          "FT_MSDK_CCT_V2_OP_ISP_GET_SHADING_PARA", mrCCAPGetShadingPara_Cmd},
    {"setDefectOn",               "FT_MSDK_CCT_V2_ISP_DEFECT_TABLE_ON", mrCCAPDefectTblOn_Cmd},
    {"setDefectOff",               "FT_MSDK_CCT_V2_ISP_DEFECT_TABLE_OFF", mrCCAPDefectTblOff_Cmd },
    {"enBypass",            "FT_MSDK_CCT_V2_OP_ISP_ENABLE_DYNAMIC_BYPASS_MODE", mrCCAPEnableDynamicBypass},
    {"disBypass",           "FT_MSDK_CCT_V2_OP_ISP_DISABLE_DYNAMIC_BYPASS_MODE", mrCCAPDisableDynamicBypass },
    {"getBypass",           "FT_MSDK_CCT_V2_OP_ISP_GET_DYNAMIC_BYPASS_MODE_ON_OFF", mrCCAPGetDynamicMode_Cmd },
    {"lnvramisp",           "ACDK_CCT_OP_ISP_LOAD_FROM_NVRAM",          mrCCAPISPLoadFromNvram_Cmd},
    {"snvramisp",           "ACDK_CCT_OP_ISP_SAVE_TO_NVRAM",            mrCCAPISPSaveToNvram_Cmd},
    //Calibration
    {"calShading",                  "MSDK_CCT_V2_OP_SHADING_CAL", mrCCAPCalShading_Cmd},
    {"verifyShading",              "MSDK_CCT_V2_OP_SHADING_VERIFY", mrCCAPVerifyShading_Cmd},
    {"verifyDefect",                "MSDK_CCT_V2_OP_DEFECT_VERIFY", mrCCAPVerifyDefect_Cmd},
    {"calDefect",                   "FT_MSDK_CCT_OP_DEFECT_TABLE_CAL", mrCCAPDefectTblCal_Cmd},

    //Sensor Control
    {"rsensorReg",       "ACDK_CCT_OP_READ_SENSOR_REG",        mrCCAPReadSensorReg_Cmd},
    {"wsensorReg",       "ACDK_CCT_OP_WRITE_SENSOR_REG",       mrCCAPWriteSensorReg_Cmd},
    {"getres",           "ACDK_CCT_V2_OP_GET_SENSOR_RESOLUTION", mrCCAPGetSensorRes_Cmd},
    {"getlscres",        "ACDK_CCT_OP_GET_LSC_SENSOR_RESOLUTION", mrCCAPGetLSCSensorRes_Cmd},
    {"qsensor",                      "MSDK_CCT_OP_QUERY_SENSOR", mrCCAPQuerySensor_Cmd},
    {"gsensorPara",               "MSDK_CCT_OP_GET_ENG_SENSOR_PARA", mrCCAPGetEngSensorPara_Cmd},
    {"ssensorPara",               "MSDK_CCT_OP_SET_ENG_SENSOR_PARA", mrCCAPSetEngSensorPara_Cmd},
    {"gsensorGPara",             "MSDK_CCT_OP_GET_ENG_SENSOR_GROUP_PARA", mrCCAPGetEngSensorGroupPara_Cmd},
    {"gsensorGCnt",              "MSDK_CCT_OP_GET_ENG_SENSOR_GROUP_COUNT", mrCCAPGetEngSensorGroupCnt_Cmd},
    {"gsenPreGain",               "FT_MSDK_CCT_OP_GET_SENSOR_PREGAIN",             mrCCAPGetSensorPreGain_Cmd},
    {"ssenPreGain",               "FT_MSDK_CCT_OP_SET_SENSOR_PREGAIN",              mrCCAPSetSensorPreGain_Cmd},

    //AE
    {"enAE",             "ACDK_CCT_OP_AE_ENABLE",                 mrCCAPAEEnable_Cmd},
    {"disAE",            "ACDK_CCT_OP_AE_DISABLE",                mrCCAPAEDisable_Cmd},
    {"getAE",            "ACDK_CCT_OP_AE_GET_ENABLE_INFO",        mrCCAPGetAEInfo_cmd},
    {"sAEMode",          "ACDK_CCT_OP_DEV_AE_SET_SCENE_MODE",     mrCCAPAESetSceneMode_Cmd},
    {"gAEParam",         "ACDK_CCT_OP_DEV_AE_GET_INFO",           mrCCAPAEGetInfo_Cmd},
    {"gAEMode",          "ACDK_CCT_V2_OP_AE_GET_SCENE_MODE",      mrCCAPAEGetSceneMode_Cmd},
    {"sAEMeter",         "ACDK_CCT_V2_OP_AE_SET_METERING_MODE",   mrCCAPAESetMeteringMode_Cmd},
    {"sExpPara",         "ACDK_CCT_V2_OP_AE_APPLY_EXPO_INFO",     mrCCAPAEApplyExpoInfo_Cmd},
    {"sAEFlicker",       "ACDK_CCT_V2_OP_AE_SELECT_BAND",         mrCCAPAESelectBand_Cmd},
    {"gExpPara",         "ACDK_CCT_V2_OP_AE_GET_AUTO_EXPO_PARA",  mrCCAPAEGetAutoExpoPara_Cmd},
    {"gAEFlicker",       "ACDK_CCT_V2_OP_AE_GET_BAND",            mrCCAPAEGetBand_Cmd},
    {"gAEMeter",         "ACDK_CCT_V2_OP_AE_GET_METERING_RESULT", mrCCAPAEGetMeteringResoult_Cmd},
    {"aAEInfo",          "ACDK_CCT_OP_DEV_AE_APPLY_INFO",         mrCCAPAEApplyInfo_Cmd},
    {"sAEParam",         "ACDK_CCT_OP_DEV_AE_SAVE_INFO_NVRAM",    mrCCAPAESaveInfoToNVRAM_Cmd},
    {"gAECaliEV",        "ACDK_CCT_OP_DEV_AE_GET_EV_CALIBRATION", mrCCAPAEGetEVCalibration_Cmd},
    {"sAEExpLine",       "ACDK_CCT_OP_AE_SET_SENSOR_EXP_LINE",    mrCCAPAESetExpLine_Cmd},

    //AWB
    {"enAWB",             "ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN",   mrCCAPAWBEnableAutoRun_Cmd},
    {"disAWB",            "ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN",  mrCCAPAWBDisableAutoRun_Cmd},
    {"gAWBEnableInfo",    "ACDK_CCT_V2_OP_AWB_GET_AUTO_RUN_INFO", mrCCAPAWBGetEnableInfo_Cmd},
    {"gAWBGain",          "ACDK_CCT_V2_OP_AWB_GET_GAIN",          mrCCAPAWBGetGain_Cmd},
    {"sAWBGain",          "ACDK_CCT_V2_OP_AWB_SET_GAIN (sAWBGain <Rgain> <Ggain> <Bgain>)", mrCCAPAWBSetGain_Cmd},
    {"aAWBParam",         "ACDK_CCT_V2_OP_AWB_APPLY_CAMERA_PARA2", mrCCAPAWBApplyCameraPara_Cmd},
    {"gAWBParam",         "ACDK_CCT_V2_OP_AWB_GET_AWB_PARA",       mrCCAPAWBGetAWBPara_Cmd},
    {"sAWBParam",         "ACDK_CCT_V2_OP_AWB_SAVE_AWB_PARA",      mrCCAPAWBSaveAWBPara_Cmd},
    {"sAWBMode",          "ACDK_CCT_OP_AWB_SET_AWB_MODE (sAWBMode <AWBMode>)", mrCCAPAWBSetAWBMode_Cmd},
    {"gAWBMode",          "ACDK_CCT_OP_AWB_GET_AWB_MODE",          mrCCAPAWBGetAWBMode_Cmd},

    //AF
    {"301",               "ACDK_CCT_V2_OP_AF_OPERATION",           mrCCAPAFOperation_Cmd},
    {"302",               "ACDK_CCT_V2_OP_MF_OPERATION",           mrCCAPMFOperation_Cmd},
    {"303",               "ACDK_CCT_V2_OP_GET_AF_INFO",            mrCCAPGetAFInfo_Cmd},
    {"304",               "ACDK_CCT_V2_OP_AF_GET_BEST_POS",        mrCCAPAFGetBestPos_Cmd},
    {"305",               "ACDK_CCT_V2_OP_AF_CALI_OPERATION",      mrCCAPAFCaliOperation_Cmd},
    {"306",               "ACDK_CCT_V2_OP_AF_SET_RANGE",           mrCCAPAFSetRange_Cmd},
    {"307",               "ACDK_CCT_V2_OP_AF_GET_RANGE",           mrCCAPAFGetRange_Cmd},
    {"308",               "ACDK_CCT_V2_OP_AF_SAVE_TO_NVRAM",       mrCCAPAFSaveToNVRAM_Cmd},
    {"309",               "ACDK_CCT_V2_OP_AF_READ",                mrCCAPAFRead_Cmd},
    {"310",               "ACDK_CCT_V2_OP_AF_APPLY",               mrCCAPAFApply_Cmd},
    {"311",               "ACDK_CCT_V2_OP_AF_GET_FV",              mrCCAPAFGetFV_Cmd},

    //FLASH
    {"enFlash",           "ACDK_CCT_OP_FLASH_ENABLE",              mrCCAPFLASHEnable_Cmd},
    {"disFlash",          "ACDK_CCT_OP_FLASH_DISABLE",             mrCCAPFLASHDisable_Cmd},
    {"gFlashEnableInfo",  "ACDK_CCT_OP_FLASH_GET_INFO",            mrCCAPFLASHGetInfo_Cmd},

    //CCM
    {"501",                 "ACDK_CCT_V2_OP_AWB_ENABLE_DYNAMIC_CCM",    mrCCAPEnableDyanmicCCM_Cmd}, 
    {"502",                 "ACDK_CCT_V2_OP_AWB_DISABLE_DYNAMIC_CCM",   mrCCAPDisableDynamicCCM_Cmd}, 
    {"503",                 "ACDK_CCT_V2_OP_AWB_GET_CCM_PARA",          mrCCAPGetCCMPara_Cmd}, 
    {"504",                 "ACDK_CCT_V2_OP_AWB_GET_CCM_STATUS",        mrCCAPGetCCMStatus_Cmd}, 
    {"505",                 "ACDK_CCT_V2_OP_AWB_GET_CURRENT_CCM",       mrCCAPGetCurrentCCM_Cmd}, 
    {"506",                 "ACDK_CCT_V2_OP_AWB_GET_NVRAM_CCM",         mrCCAPGetNVRAMCCM_Cmd}, 
    {"507",                 "ACDK_CCT_V2_OP_AWB_SET_CURRENT_CCM",       mrCCAPSetCurrentCCM_Cmd}, 
    {"508",                 "ACDK_CCT_V2_OP_AWB_SET_NVRAM_CCM",         mrCCAPSetNVRAMCCM_Cmd}, 
    {"509",                 "ACDK_CCT_V2_OP_AWB_UPDATE_CCM_PARA",       mrCCAPUpdateCCMPara_Cmd}, 
    {"510",                 "ACDK_CCT_V2_OP_AWB_UPDATE_CCM_STATUS",     mrCCAPUpdateCCMStatus_Cmd}, 
    {"setCCMMode",          "ACDK_CCT_OP_SET_CCM_MODE",                 mrCCAPSetCCMMode_Cmd}, 
    {"getCCMMode",          "ACDK_CCT_OP_GET_CCM_MODE",                 mrCCAPGetCCMMode_Cmd}, 

    //Gamma
    {"601",                 "FT_MSDK_CCT_V2_OP_AE_SET_GAMMA_BYPASS",             mrCCAPSetGammaBypass_Cmd},
    {"602",                 "FT_MSDK_CCT_V2_OP_AE_GET_GAMMA_BYPASS_FLAG",    mrCCAPGetGammaBypassFlag_Cmd},
    {"603",                 "FT_MSDK_CCT_V2_OP_AE_GET_GAMMA_TABLE",                mrCCAPGetGammaTable_Cmd},
    {"604",                 "ACDK_CCT_V2_OP_AE_SET_GAMMA_TABLE",                     mrCCAPSetGammaTable_Cmd},

    //PCA
    {"setPcaTable",         "ACDK_CCT_OP_ISP_SET_PCA_TABLE",            mrCCAPSetPcaTable_Cmd},
    {"getPcaTable",         "ACDK_CCT_OP_ISP_GET_PCA_TABLE",            mrCCAPGetPcaTable_Cmd},
    {"setPcaPara",          "ACDK_CCT_OP_ISP_SET_PCA_PARA",             mrCCAPSetPcaPara_Cmd},
    {"getPcaPara",          "ACDK_CCT_OP_ISP_GET_PCA_PARA",             mrCCAPGetPcaPara_Cmd},

    //Module Control
    {"setOB",               "ACDK_CCT_V2_OP_SET_OB_ON_OFF",             mrCCAPSetOBOnOff_Cmd},
    {"getOB",               "ACDk_CCT_V2_OP_GET_OB_ON_OFF",             mrCCAPGetOBOnOff_Cmd},
    {"setNR",               "ACDk_CCT_V2_OP_SET_NR_ON_OFF",             mrCCAPSetNROnOff_Cmd},
    {"getNR",               "ACDk_CCT_V2_OP_GET_NR_ON_OFF",             mrCCAPGetNROnOff_Cmd},
    {"setEE",               "ACDK_CCT_V2_OP_SET_EE_ON_OFF",             mrCCAPSetEEOnOff_Cmd},
    {"getEE",               "ACDK_CCT_V2_OP_GET_EE_ON_OFF",             mrCCAPGetEEOnOff_Cmd},
    {"setIspOn",            "ACDK_CCT_OP_SET_ISP_ON",                   mrCCAPSetIspOn_Cmd},
    {"setIspOff",           "ACDK_CCT_OP_SET_ISP_OFF",                  mrCCAPSetIspOff_Cmd},
    {"getIspOnOff",         "ACDK_CCT_OP_GET_ISP_ON_OFF",               mrCCAPGetIspOnOff_Cmd},

    //Shading Table
    {"getLSCTbl",         "ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_V3",       mrCCAPGetShadingTableV3_Cmd},
    {"setLSCTbl",         "ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3",        mrCCAPSetShadingTableV3_Cmd},
    {"getLSCTblp",         "ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_POLYCOEF",       mrCCAPGetShadingTablePOLYCOEF_Cmd},
    {"setLSCTblp",         "ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF",        mrCCAPSetShadingTablePOLYCOEF_Cmd},
    {"getLSCNvram",         "ACDK_CCT_V2_OP_ISP_GET_SHADING_NVRAM",       mrCCAPGetShadingNVRAM_Cmd},
    {"setLSCIndex",     "ACDK_CCT_V2_OP_ISP_SET_SHADING_INDEX",             mrCCAPSetShadingIndex_Cmd},
    {"getLSCIndex",     "ACDK_CCT_V2_OP_ISP_GET_SHADING_INDEX",             mrCCAPGetShadingIndex_Cmd},
    // CDVT
    {"SensorTest", "ACDK_CCT_OP_CDVT_SENSOR_TEST", mrCCAPCDVTSensorTest_Cmd},
    {"SensorCal", "ACDK_CCT_OP_CDVT_SENSOR_CALIBRATION", mrCCAPCDVTSensorCalibration_Cmd},

    NULL_CLI_CMD,
};

/////////////////////////////////////////////////////////////////////////
//
//  vHelp () -
//! @brief skip the space of the input string
//! @param ppInStr: The point of the input string
/////////////////////////////////////////////////////////////////////////
VOID vHelp()
{
    if (g_prAcdkCCAPCLICmds == NULL)
    {
        ACDK_LOGE("Null Acdk Support Cmds \n");
        return;
    }

    printf("\n***********************************************************\n");
    printf("* ACDK CLI Test                                                  *\n");
    printf("* Current Support Commands                                *\n");
    printf("===========================================================\n");
    MUINT32 i = 0;
    while(g_prAcdkCCAPCLICmds[ i].pucCmdStr != NULL)
    {
        printf("%s    [%s]\n", g_prAcdkCCAPCLICmds[ i].pucCmdStr,
                                g_prAcdkCCAPCLICmds[ i].pucHelpStr);
        i++;
    }
    printf("help/h    [Help]\n");
    printf("exit/q    [Exit]\n");
}

/////////////////////////////////////////////////////////////////////////
//
//  cliKeyThread () -
//! @brief the CLI key input thread, wait for CLI command
//! @param a_pArg: The input arguments
/////////////////////////////////////////////////////////////////////////
VOID* cliKeyThread (VOID *a_pArg)
{
    char urCmds[256] = {0};

    //! ************************************************
    //! Set the signal for kill self thread
    //! this is because android don't support thread_kill()
    //! So we need to creat a self signal to receive signal
    //! to kill self
    //! ************************************************
    struct sigaction actions;
    memset(&actions, 0, sizeof(actions));
    sigemptyset(&actions.sa_mask);
    actions.sa_flags = 0;
    actions.sa_handler = thread_exit_handler;
    int rc = sigaction(SIGUSR1,&actions,NULL);

    while (1)
    {
        printf("Input Cmd#");
        fgets(urCmds, 256, stdin);

        //remove the '\n'
        urCmds[strlen(urCmds)-1] = '\0';
        char *pCmds = &urCmds[0];
        //remove the space in the front of the string
        vSkipSpace(&pCmds);

        //Ignore blank command
        if (*pCmds == '\0')
        {
            continue;
        }

        //Extract the Command  and arguments where the argV[0] is the command
        MUINT32 u4ArgCount = 0;
        MUINT8  *pucStrToken, *pucCmdToken;
        MUINT8  *pucArgValues[MAX_CLI_CMD_ARGS];

        pucStrToken = (MUINT8*)strtok(pCmds, " ");
        while (pucStrToken != NULL)
        {
            pucArgValues[u4ArgCount++] = pucStrToken;
            pucStrToken = (MUINT8*)strtok (NULL, " ");
        }

        if (u4ArgCount == 0)
        {
            continue;
        }

        pucCmdToken = pucArgValues[0];

        //parse the command
        if ((strcmp((char*)pucCmdToken, "help") == 0) ||
            (strcmp((char *)pucCmdToken, "h") == 0))
        {
            vHelp();
        }
        else if ((strcmp((char*)pucCmdToken, "exit") == 0) ||
                  (strcmp((char *)pucCmdToken, "q") == 0))
        {
            ACDK_LOGD("Exit From CLI\n");
            g_bIsCLITest = FALSE;
        }
        else
        {
            MINT32 i4CmdIndex = 0;
            BOOL bIsFoundCmd = FALSE;
            while (g_prAcdkCCAPCLICmds[i4CmdIndex].pucCmdStr != NULL)
            {
                if (strcmp((char*)pucCmdToken, g_prAcdkCCAPCLICmds[i4CmdIndex].pucCmdStr) == 0)
                {
                    bIsFoundCmd = TRUE;
                    g_prAcdkCCAPCLICmds[i4CmdIndex].handleCmd(u4ArgCount - 1, &pucArgValues[1]);
                    break;
                }
                i4CmdIndex++;
            }
            if (bIsFoundCmd == FALSE)
            {
                printf("Invalid Command\n");
            }
        }

    }

    return 0;
}


/////////////////////////////////////////////////////////////////////////
//
//  main () -
//! @brief The main function for sensor test, it will create two thread
//!        one is for CLI command
//!        the other is for keypad input command
//! @param argc: The number of the input argument
//! @param argv: The input arguments
/////////////////////////////////////////////////////////////////////////
int main (int argc, char **argv)
{
    MUINT32 u4RealOutLen = 0;


    ACDK_LOGD(" Acdk CCAP CLI Test\n");

    ACDK_LOGD(" Insert the PMEM device \n");
    //! *************************************************
    //! insert the PMEM device,
    //! *************************************************
    if (open("/dev/pmem_multimedia", O_RDWR) < 0)
    {
        vExecProgram("mknod", g_mkPMEMNod_arg_list);
    }

    //!***************************************************
    //! mount the SD Card to SDCard
    //!***************************************************
    ACDK_LOGD(" Mount SD Card to /sdcard folder \n");
    vExecProgram("mount", g_mountSD_arg_list);

    sleep(1);

    //! *************************************************
    //! Create the related object and init/enable it
    //! *************************************************
    if (MDK_Open() == FALSE)
    {
        ACDK_LOGE("MDK_Open() Fail \n");
        goto Exit;
    }


    if (MDK_Init() == FALSE)
    {
        ACDK_LOGE("MDK_Init() Fail \n");
        goto Exit;
    }

    g_bAcdkOpend = TRUE;

    //! *************************************************
    //! Create the CLI command thread to listen input CLI command
    //! *************************************************
    ACDK_LOGD(" Create the CLI thread \n");
    vHelp();
    pthread_create(& g_CliKeyThreadHandle, NULL, cliKeyThread, NULL);

    //!***************************************************
    //! Main thread wait for exit
    //!***************************************************
    while (g_bIsCLITest== TRUE)
    {
        sleep(1);
    }

    //!***************************************************
    //! Receive the exit command, cancel the two thread
    //!***************************************************
    int status;
    ACDK_LOGD("Cancel cli key thread\n");
    if ((status = pthread_kill(g_CliKeyThreadHandle, SIGUSR1)) != 0)
    {
         ACDK_LOGE("Error cancelling thread %d, error = %d (%s)\n", (int)g_CliKeyThreadHandle,
               status, (char*)strerror);
    }


Exit:

    //!***************************************************
    //! Exit
    //! 1. DeInit ACDK device and close it
    //! 2. Umount the SD card and sync the file to ensure
    //!    all files are written to SD card
    //! 3. Sync all file to SD card to ensure the files are saving in SD
    //!***************************************************
    MDK_DeInit();
    MDK_Close();
    ACDK_LOGD("umount SDCard file system\n");
    vExecProgram("sync", g_sync_arg_list);     //sync all file tor sdcard first
    vExecProgram("umount", g_unMountSD_arg_list);

    return 0;
}


