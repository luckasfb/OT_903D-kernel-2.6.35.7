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
// AcdkCLITest.cpp  $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkCLITest.cpp
//! \brief

#define LOG_TAG "ACDKCliTest"
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

#include "AcdkIF.h"

#include "AcdkCCTFeature.h"

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
static MBOOL g_bIsCLITest = TRUE;
static pthread_t g_InputKeyThreadHandle; 
static pthread_t g_CliKeyThreadHandle; 

static Acdk_CLICmd *g_prAcdkSupportCLICmds = NULL; 

static UINT32 g_u4CLICmdCnt = 0; 
/////////////////////////////////////////////////////////////////////////
//
//  thread_exit_handler () - 
//! @brief the CLI key input thread, wait for CLI command 
//! @param sig: The input arguments 
/////////////////////////////////////////////////////////////////////////
void thread_exit_handler(INT32 a_u4Sig)
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
void vExecProgram(const char *pProgram, const char * const ppArgList[])
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
//  vHelp () - 
//! @brief skip the space of the input string 
//! @param ppInStr: The point of the input string 
/////////////////////////////////////////////////////////////////////////
void vHelp()
{
    if (g_prAcdkSupportCLICmds == NULL)
    {
        ACDK_LOGE("Null Acdk Support Cmds \n");
        return; 
    }

    printf("\n***********************************************************\n");
    printf("* ACDK CLI Test                                                  *\n");
    printf("* Current Support Commands                                *\n"); 
    printf("===========================================================\n");    

    printf("help/h    [Help]\n");
    printf("exit/q    [Exit]\n");

    UINT32 i = 0; 
    for (i = 0; i < g_u4CLICmdCnt; i++)
    {
        printf("%s    [%s]\n", g_prAcdkSupportCLICmds[ i].pucCmdStr, 
                                g_prAcdkSupportCLICmds[ i].pucHelpStr);        
    }
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
        UINT32 u4ArgCount = 0;
        MINT8  *pucStrToken, *pucCmdToken;
        MUINT8  *pucArgValues[MAX_CLI_CMD_ARGS];
        
        pucStrToken = (MINT8 *)strtok(pCmds, " ");
        while (pucStrToken != NULL)
        {
            pucArgValues[u4ArgCount++] =(MUINT8*) pucStrToken;
            pucStrToken = (MINT8*)strtok (NULL, " ");                
        }

        if (u4ArgCount == 0)
        {
            continue; 
        }
        
        pucCmdToken = (MINT8*) pucArgValues[0]; 

        //parse the command 
        if ((strcmp((char *)pucCmdToken, "help") == 0) ||
            (strcmp((char *)pucCmdToken, "h") == 0))
        {
            vHelp(); 
        }
        else if ((strcmp((char *)pucCmdToken, "exit") == 0) || 
                  (strcmp((char *)pucCmdToken, "q") == 0))
        {
            ACDK_LOGD("Exit From CLI\n"); 
            g_bIsCLITest = FALSE; 
        }
        else
        {
            MBOOL bIsFoundCmd = FALSE;
            for (UINT32 u4CmdIndex = 0; u4CmdIndex < g_u4CLICmdCnt; u4CmdIndex++)
            {
                if (strcmp((char *)pucCmdToken, g_prAcdkSupportCLICmds[u4CmdIndex].pucCmdStr) == 0)
                {
                    bIsFoundCmd = TRUE; 
                    g_prAcdkSupportCLICmds[u4CmdIndex].handleCmd(u4ArgCount - 1, &pucArgValues[1]);                     
                    break;
                }                
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
    UINT32 u4RealOutLen = 0; 


    ACDK_LOGD(" Acdk CLI Test\n"); 
    
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

    //vExecProgram("mknod", g_mkInputNode_arg_list); 

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

    //MDK_IOControl(UINT32 a_u4Ioctl, UINT8 * a_pBufIn, UINT32 a_u4BufInLen, UINT8 * a_pBufOut, UINT32 a_u4BufOutLen, UINT32 * a_pu4ActualOutLen)

    //mrRet = pAcdkCamCtrlObj->mrStartPreview(320, 240,NULL);

    ACDK_FEATURE_INFO_STRUCT rAcdkFeatureInfo; 
    rAcdkFeatureInfo.puParaIn = NULL; 
    rAcdkFeatureInfo.u4ParaInLen = 0; 
    rAcdkFeatureInfo.puParaOut = (UINT8 *)&g_u4CLICmdCnt; 
    rAcdkFeatureInfo.u4ParaOutLen = sizeof(UINT32); 
    rAcdkFeatureInfo.pu4RealParaOutLen = &u4RealOutLen; 
    
    MDK_IOControl(ACDK_CCT_FEATURE_QUERY_CLI_CMD_CNT,  &rAcdkFeatureInfo);  
    
    ACDK_LOGD("Total CLI Cmd Cnt:%d\n", g_u4CLICmdCnt); 

    g_prAcdkSupportCLICmds = (Acdk_CLICmd *) malloc (g_u4CLICmdCnt * sizeof (Acdk_CLICmd)); 

    rAcdkFeatureInfo.puParaIn = NULL; 
    rAcdkFeatureInfo.u4ParaInLen = 0; 
    rAcdkFeatureInfo.puParaOut = (UINT8 *)g_prAcdkSupportCLICmds; 
    rAcdkFeatureInfo.u4ParaOutLen = sizeof(Acdk_CLICmd) *g_u4CLICmdCnt ; 
    rAcdkFeatureInfo.pu4RealParaOutLen = &u4RealOutLen; 

    MDK_IOControl(ACDK_CCT_FEATURE_QUERY_CLI_CMDS,  &rAcdkFeatureInfo);

    vHelp(); 

    ACDK_LOGD(" Create the CLI thread \n");
    pthread_create(& g_CliKeyThreadHandle, NULL, cliKeyThread, NULL); 


    ACDK_CCT_CAMERA_PREVIEW_STRUCT rCCTPreviewConfig;

    rCCTPreviewConfig.fpPrvCB = NULL;
    rCCTPreviewConfig.u2PreviewWidth = 320;
    rCCTPreviewConfig.u2PreviewHeight = 240;

    rAcdkFeatureInfo.puParaIn = (MUINT8*)&rCCTPreviewConfig; 
    rAcdkFeatureInfo.u4ParaInLen = sizeof(ACDK_CCT_CAMERA_PREVIEW_STRUCT); 
    rAcdkFeatureInfo.puParaOut = NULL; 
    rAcdkFeatureInfo.u4ParaOutLen = 0; 
    rAcdkFeatureInfo.pu4RealParaOutLen = &u4RealOutLen; 


    MDK_IOControl(ACDK_CCT_OP_PREVIEW_LCD_START,  &rAcdkFeatureInfo);

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
    /*
    MANU_LOGD("Cancel input key thread\n"); 
    if ( (status = pthread_kill(g_InputKeyThreadHandle, SIGUSR1)) != 0) 
    {    
        MANU_LOGE("Error cancelling thread %d, error = %d (%s)\n", (int)g_InputKeyThreadHandle, 
               status, (char*)strerror);
    } 
    */

    ACDK_LOGD("Cancel cli key thread\n");     
    if ((status = pthread_kill(g_CliKeyThreadHandle, SIGUSR1)) != 0)
    {
         ACDK_LOGE("Error cancelling thread %d, error = %d (%s)\n", (int)g_CliKeyThreadHandle, 
               status, (char*)strerror);    
    }


Exit:

    //!***************************************************
    //! Exit 
    //! 1. Umount the SD card and sync the file to ensure 
    //!    all files are written to SD card 
    //! 2. Disable the camera 
    //! 3. delete all object 
    //!***************************************************         
    MDK_DeInit(); 
    MDK_Close();     
    ACDK_LOGD("umount SDCard file system\n");     
    vExecProgram("sync", g_sync_arg_list);     //sync all file tor sdcard first 
    vExecProgram("umount", g_unMountSD_arg_list);

    if (g_prAcdkSupportCLICmds != NULL)
    {
        free(g_prAcdkSupportCLICmds); 
    }
    
    return 0;     
}

