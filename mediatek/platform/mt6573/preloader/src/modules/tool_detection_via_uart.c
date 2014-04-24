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

#include <mt6573.h>
#include "mt6573_typedefs.h"
#include "mt6573_download.h"
#include "mt6573_rtc.h"
#include "mt65xx_meta.h"
#include "boot_mode.h"
#include "uart.h"
#include "cust.h"

#define MOD                 "<TOOL>"

#define META_STR_READY      "READY"         /* Ready Signal */

/**************************************************************************
 *  DEBUG FLAG
 **************************************************************************/
#define META_DEBUG

/**************************************************************************
 *  FUNCTION IMPLEMENTATION
 **************************************************************************/

//======================================================================
//  HANDSHAKE SYNC TIME
//======================================================================
#define META_SYNCH_TIME      250    /* in ms */


static ulong g_meta_ready_start_time = 0;

/******************************************************************************
 * listen_tool
 * 
 * DESCRIPTION:
 *
******************************************************************************/
static bool meta_listen_tool(int listen_byte_cnt, int retry_cnt, int wait_time, char* buf)
{
    unsigned int  start_time = get_timer(0);  /* calculate wait time */
    char          c = 0;
    int           i = 0;    
    int           rx_cnt = 0;
    ulong         begin_time = 0;

    
    for(i=0;i<retry_cnt;i++)
    {
        begin_time = get_timer(0);

        do {
            c = serial_nonblock_getc();
            
            if(c!=NULL)
            {
                buf[rx_cnt]=c;                
                rx_cnt++;
            }
        
            if (rx_cnt == listen_byte_cnt)
            {   mt6573_serial_set_current_uart(UART4);
                dbg_print ("%s : sync time %d ms\n", MOD, get_timer (start_time));
                return TRUE;    
            }
        } while(get_timer(begin_time) < wait_time);
    }

    return FALSE;
}

/** 
 * If UART log port feature is not switched, 
 * function "uart1_as_tool_port_accord_dynamic_setting()" 
 * will alway return 1.
 */
extern unsigned int uart1_as_tool_port_accord_dynamic_setting();
static void check_meta_respon_value(char *print_value)
{
    int i = 0;

    if( (CFG_UART_LOG_PORT == UART4)||uart1_as_tool_port_accord_dynamic_setting() )
    {
        mt6573_serial_set_current_uart(UART1);
        dbg_print(print_value);
        for(i=0;i<200;i++);        
        mt6573_serial_set_current_uart(UART4);        
    }
    else if (CFG_UART_LOG_PORT == UART1)
    {
        mt6573_serial_set_current_uart(UART4);
        dbg_print(print_value);
        for(i=0;i<200;i++);        
        mt6573_serial_set_current_uart(UART1);        
    }
    else
    {
        dbg_print("Error: uart config\n");
    }
}

void meta_send_ready(void)
{
    /* send meta ready to tool and tool can start sending handshake pattern */
    if( (CFG_UART_LOG_PORT == UART4)||uart1_as_tool_port_accord_dynamic_setting() )
    {
        mt6573_serial_set_current_uart(UART1);
        dbg_print(META_STR_READY);
        mt6573_serial_set_current_uart(UART4);
    }
    else if(CFG_UART_LOG_PORT == UART1)
    {
        mt6573_serial_set_current_uart(UART4);
        dbg_print(META_STR_READY);
        mt6573_serial_set_current_uart(UART1);
    }
    g_meta_ready_start_time = get_timer(0);
}

/******************************************************************************
 * meta_check_pc_trigger
******************************************************************************/
BOOL meta_check_pc_trigger(void)
{
    ulong   begin = 0;
    int     i = 0, j =0;
    char    hpbuf[META_SZ_MAX_PBUF] = { '\0' };  /* hpbuf = handshake protocol buffer */
    int     meta_lock = 0;
    int     wait_time = META_SYNCH_TIME - get_timer(g_meta_ready_start_time);

    wait_time = wait_time <= 0 ? 5 : wait_time;
    wait_time = wait_time > META_SYNCH_TIME ? META_SYNCH_TIME : wait_time;
    dbg_print("%s : wait sync time %dms\n", MOD, wait_time);

    /* detect tool existence */ 
    if( (CFG_UART_LOG_PORT == UART4)||uart1_as_tool_port_accord_dynamic_setting() )
    {
        mt6573_serial_set_current_uart(UART1);
        //dbg_print(META_STR_READY);
        meta_listen_tool(DM_SZ_DWN_REQ_STR, 1, wait_time, hpbuf);
        mt6573_serial_set_current_uart(UART4);
        dbg_print("\nfrom UART1: %s\n",hpbuf);          
    }
    else if(CFG_UART_LOG_PORT == UART1)
    {
        mt6573_serial_set_current_uart(UART4);
        //dbg_print(META_STR_READY);
        meta_listen_tool(DM_SZ_DWN_REQ_STR, 1, wait_time, hpbuf);
        mt6573_serial_set_current_uart(UART1);
        dbg_print("\nfrom UART4: %s\n",hpbuf);
    }
    else
    {
        dgb_print("\nError: uart port \n");
        return FALSE;
    }

    
    if(hpbuf==NULL)
    {   
        dbg_print("meta timeout\n");
        return FALSE;
    }
        
    /* check "METAMETA", judge whether META mode indicated */    
    if(!strncmp ((const u8 *) hpbuf, META_STR_REQ, strlen(META_STR_REQ)))
    {
        dbg_print ("%s : meta detected !\n", MOD);
        /* return "ATEMATEM" to ack tool */
        check_meta_respon_value(META_STR_ACK);	
        g_boot_mode = META_BOOT;
        return TRUE;
    }

    /* check "ADVMETA", judge whether META mode indicated */
    if(!strncmp ((const u8 *) hpbuf, META_ADV_REQ, strlen(META_ADV_REQ)))    
    {
        dbg_print ("%s : adv meta detected !\n", MOD);
        /* return "ATEMVDA" to ack tool */
        check_meta_respon_value(META_ADV_ACK);
        g_boot_mode = ADVMETA_BOOT;
        return TRUE;
    }

    /* check "FACTORYM", judge whether ATE mode indicated */
    if(!strncmp ((const u8 *) hpbuf, ATE_STR_REQ, strlen(ATE_STR_REQ)))            
    {            
        dbg_print ("%s : ate detected !\n", MOD);  
        /* return "MYROTCAF" to ack tool */
        check_meta_respon_value(ATE_STR_ACK);
        g_boot_mode = ATE_FACTORY_BOOT;
        return TRUE;
    }
    else
    {   
        dbg_print ("%s : unknown string %s !\n", MOD, hpbuf);
    }

    return FALSE;
}
