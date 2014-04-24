/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#include "mt6573.h"
#include "mt6573_typedefs.h"
#include "mt6573_emi_reg.h"
#include "custom_emi.h"

extern int _EmiDataTrain( EMI_DATA_TRAIN_REG_t* pResult, int SDRAM_CS);

/*------------------------------------------------------------------------------
    Set MBIST TEST BUFFER Start Address == End Address,
    This will consume 1KB buffer for MBIST R/W Test
  ------------------------------------------------------------------------------*/
kal_uint32 __EmiDataTRainMbistTest(unsigned long MBIST_TEST_BUFFER_START_ADDR, unsigned long MBIST_TEST_BUFFER_END_ADDR )
{
    #define __EMIDATATRAINMBISTTEST_FULL
    
    unsigned long   EMI_DRCT_bakval;
    kal_uint32      ret_val = 0;        /*Return value 0: SUCCESS 1: FAIL 2: TIMEOUT*/
    unsigned long   timeout = 0;
    int             i, j, k, m, n;
    unsigned int    error_address, delay, test_mode;

    

    /*Backup EMI_DRCT*/
    EMI_DRCT_bakval = *EMI_DRCT;

    /* MUST DISABLE DUMMUY READ when MBIST*/
    *EMI_DRCT &= ~(0x1);

    #if defined(__EMIDATATRAINMBISTTEST_FULL)
    for(i=0; i<3 /*4*/; i++)        //test BIST_MODE /*No mode b11 *//*ORIGINAL*/
    #else
    for(i=2; i <= 2  /*4*/; i++)      //Use this combination to mbist data train. i=0 has problem   
    #endif
    {
        #if defined(__EMIDATATRAINMBISTTEST_FULL)
        for(j=0; j<1 /*5*/; j++)    //test BIST_ADSCR /*Fix Address*//*ORIGINAL*/
        #else
        for(j=0; j<= 0 /*5*/; j++)   
        #endif
        {
            #if defined(__EMIDATATRAINMBISTTEST_FULL)
            for(k=0; k<7; k++)      //test BIST_DTSCR/*ORIGINAL*/
            #else
            for(k=4; k<=4; k++)      
            #endif
            {
                // MBIST reset
                *EMI_MBISTA = 0x0;
                // MBIST data setting
                *EMI_MBISTB = 0xFFFF0000 | ((0x0000A55A) >> (i+j+k));   /*CLS:Mark Pattern 2*/

                // MBIST starting address
                *EMI_MBISTC = MBIST_TEST_BUFFER_START_ADDR >> 10;
                // MBIST end address
                *EMI_MBISTD = MBIST_TEST_BUFFER_END_ADDR >> 10;

                *EMI_MBISTA &= 0xFFFF000F;

                //for(delay=0; delay<0x5fff; delay++); //for DDR2 /8
                *EMI_MBISTA = *EMI_MBISTA | 0x00220000 | i<<4 | j<<12 | k<<8 | 1; //enable first
                
                //for(delay=0; delay<0x5fff; delay++); //for DDR2 /8
                *EMI_MBISTA = *EMI_MBISTA | 2;      //trigger start
                
                
                //for(delay=0; delay<0x5fff; delay++); //for DDR2 /8
                // polling the mbist status
                while(!(*EMI_MBISTE&0x0002))
                {
                    if( timeout++ >= 0xFFFFFFFF )
                    {
                        ret_val = 2;    /*** RETURN FAIL:Timeout ***/
                        goto    __EmiDataTRainMbistTest_END;
                    }
                }
                
                // check the MBIST result if fail
                if( *EMI_MBISTE&0x0001 )
                {
                    ret_val = 1;    /*** RETURN FAIL:Test Fail ***/
                    goto    __EmiDataTRainMbistTest_END;
                }

            }
        }
    }


__EmiDataTRainMbistTest_END:

    if( ret_val == 1 ) //MBIST Test Fail
    {
    #if 0
            
        unsigned long address, wdataL, wdataH, rdataL, rdataH;

        //Addr[15:00] //SPEC Wrong
        *EMI_MBISTA = (*EMI_MBISTA & ~(0xF << 28)) | ( 0x0 << 28 ) ;
        address = ( *EMI_MBISTE & 0xFFFF0000 ) >> 16;
        //addr[31:16] //SPEC Wrong
        *EMI_MBISTA = (*EMI_MBISTA & ~(0xF << 28)) | ( 0x1 << 28 ) ;
        address |= ( *EMI_MBISTE & 0xFFFF0000 );

        //wdataL[15:00]
        *EMI_MBISTA = (*EMI_MBISTA & ~(0xF << 28)) | ( 0x4 << 28 ) ;
        wdataL = ( *EMI_MBISTE & 0xFFFF0000 ) >> 16;
        //wdataL[31:16]
        *EMI_MBISTA = (*EMI_MBISTA & ~(0xF << 28)) | ( 0x5 << 28 ) ;
        wdataL |= ( *EMI_MBISTE & 0xFFFF0000 );

        //wdataH[47:32]
        *EMI_MBISTA = (*EMI_MBISTA & ~(0xF << 28)) | ( 0x6 << 28 ) ;
        wdataH = ( *EMI_MBISTE & 0xFFFF0000 ) >> 16;
        //wdataH[63:48]
        *EMI_MBISTA = (*EMI_MBISTA & ~(0xF << 28)) | ( 0x7 << 28 ) ;
        wdataH |= ( *EMI_MBISTE & 0xFFFF0000 );


        //rdataL[15:00]
        *EMI_MBISTA = (*EMI_MBISTA & ~(0xF << 28)) | ( 0x8 << 28 ) ;
        rdataL = ( *EMI_MBISTE & 0xFFFF0000 ) >> 16;
        //rdataL[31:16]
        *EMI_MBISTA = (*EMI_MBISTA & ~(0xF << 28)) | ( 0x9 << 28 ) ;
        rdataL |= ( *EMI_MBISTE & 0xFFFF0000 );

        //rdataH[47:32]
        *EMI_MBISTA = (*EMI_MBISTA & ~(0xF << 28)) | ( 0xA << 28 ) ;
        rdataH = ( *EMI_MBISTE & 0xFFFF0000 ) >> 16;
        //rdataH[63:48]
        *EMI_MBISTA = (*EMI_MBISTA & ~(0xF << 28)) | ( 0xB << 28 ) ;
        rdataH |= ( *EMI_MBISTE & 0xFFFF0000 );


        dbg_print("\n\r<MBIST Error mode:%d ad:%d da:%d>\n\r", i, j, k);
        dbg_print("ADDR:0x%08x\n\r", address);
        dbg_print("WDATA:0x%08x%08x\n\r", wdataH, wdataL);
        dbg_print("RDATA:0x%08x%08x\n\r", rdataH, rdataL);
    #endif
    }

    
    *EMI_DRCT = EMI_DRCT_bakval;    //Restore EMI_DRCT
    *EMI_MBISTA = 0x0;              //MBIST reset

    return ret_val; 
    
}

void __EmiDataTrainRegRead( EMI_DATA_TRAIN_REG_t* pREG )
{
    if( pREG != 0 )
    {
        pREG->EMI_CONN_regval = *EMI_CONN;

        pREG->EMI_DQSA_regval = *EMI_DQSA;
        pREG->EMI_DQSB_regval = *EMI_DQSB; // VST added
        pREG->EMI_DQSE_regval = *EMI_DQSE;
        
        pREG->EMI_IDLA_regval = *EMI_IDLA;
        pREG->EMI_IDLB_regval = *EMI_IDLB;
        pREG->EMI_IDLC_regval = *EMI_IDLC;
        pREG->EMI_IDLD_regval = *EMI_IDLD;
        pREG->EMI_IDLE_regval = *EMI_IDLE;
        pREG->EMI_IDLF_regval = *EMI_IDLF;
        pREG->EMI_IDLG_regval = *EMI_IDLG;
        pREG->EMI_IDLH_regval = *EMI_IDLH;
        //pREG->EMI_IDLI_regval = *EMI_IDLI;
        
        pREG->EMI_CALA_regval = *EMI_CALA;
        pREG->EMI_CALB_regval = *EMI_CALB; // VST added
        pREG->EMI_CALE_regval = *EMI_CALE;
        pREG->EMI_CALF_regval = *EMI_CALF; // VST added
        pREG->EMI_CALI_regval = *EMI_CALI;
        pREG->EMI_CALJ_regval = *EMI_CALJ; // VST added
        pREG->EMI_CALP_regval = *EMI_CALP;
    }
    
}

void __EmiDataTrainRegWrite( EMI_DATA_TRAIN_REG_t* pREG )
{
    if( pREG != 0 )
    {
        *EMI_CONN = pREG->EMI_CONN_regval;

        *EMI_DQSA = pREG->EMI_DQSA_regval;
        *EMI_DQSB = pREG->EMI_DQSB_regval; // VST added
        *EMI_DQSE = pREG->EMI_DQSE_regval;
                    
        *EMI_IDLA = pREG->EMI_IDLA_regval;
        *EMI_IDLB = pREG->EMI_IDLB_regval;
        *EMI_IDLC = pREG->EMI_IDLC_regval;
        *EMI_IDLD = pREG->EMI_IDLD_regval;
        *EMI_IDLE = pREG->EMI_IDLE_regval;
        *EMI_IDLF = pREG->EMI_IDLF_regval;
        *EMI_IDLG = pREG->EMI_IDLG_regval;
        *EMI_IDLH = pREG->EMI_IDLH_regval;
        //*EMI_IDLI = pREG->EMI_IDLI_regval;
                    
        *EMI_CALA = pREG->EMI_CALA_regval;
        *EMI_CALB = pREG->EMI_CALB_regval; // VST added
        *EMI_CALE = pREG->EMI_CALE_regval;
        *EMI_CALF = pREG->EMI_CALF_regval; // VST added
        *EMI_CALI = pREG->EMI_CALI_regval;
        *EMI_CALJ = pREG->EMI_CALJ_regval; // VST added
        *EMI_CALP = pREG->EMI_CALP_regval;
    }
    
}

// TODO : use generated emi setting
int _EmiDataTrain( EMI_DATA_TRAIN_REG_t* pResult, int SDRAM_CS )
{
    #define ___EMIDATATRAIN_MBIST_DATATRAIN__
    
    unsigned long MBIST_TEST_BUFFER_START_ADDR;
    unsigned long MBIST_TEST_BUFFER_END_ADDR;
    kal_int32 prev_emi_dqsa;
    kal_int32 prev_emi_dqsb;    
    kal_int32 delay, value;
    kal_int32 test_result;
    kal_int32 bytex_dly_mod, bytex_setup_mod, dqy_in_del, dqsix_dlysel;
    kal_int32 bytex_dly_mod_start = 0, bytex_setup_mod_start = 0, dqy_in_del_start = 0, dqsix_dlysel_start = 0;    
    kal_int32 prev_emi_idl, prev_emi_cala, prev_emi_calb, prev_emi_cale, prev_emi_calf, prev_emi_cali, prev_emi_calj; // VST added
    kal_int32 prev_dwnd_size = 0, dwnd_size, lbound, rbound;
    kal_int32 lbound_finding;
    kal_uint32 DQSI_center = 0x0, DQSI_start = 0x00, DQSI_end = 0xFF; //Use to record the DQSI start and end value
    kal_int32 DATA_TUNING_STEP = 1;
    kal_int8 DQSI_TUNING_STEP = 4;    
    kal_int8 WINDOW_SIZE_THRESHOLD = 3;
    unsigned dqs_center_point;
    unsigned dll_value;
    
    int b_wnd_found = 0;
    //EMI_DATA_TRAIN_REG_t REG_BAK;
    static EMI_DATA_TRAIN_REG_t REG_BAK = {1, 2, 3}; /*use static to avoid stack overflow*/

    if (SDRAM_CS == 0)
    {
        MBIST_TEST_BUFFER_START_ADDR  =0x410000;
        MBIST_TEST_BUFFER_END_ADDR    =0x410000;
    }
    else if (SDRAM_CS == 1)
    {        
        MBIST_TEST_BUFFER_START_ADDR  =0x10410000;
        MBIST_TEST_BUFFER_END_ADDR    =0x10410000;
    }    
    
    /*------------------------------------------------------------------------------
        Backup modified register at entry
      ------------------------------------------------------------------------------*/
    __EmiDataTrainRegRead( &REG_BAK );
    

    /*------------------------------------------------------------------------------
        Disable 
            1. "Data auto tracking" & "Setup/Hold max value"
            2. "1/5T DLL" 
            3. "Mask auto-tracking"
            before data training
      ------------------------------------------------------------------------------*/
    if (SDRAM_CS == 0)
    {
        *EMI_CALP &= ~0xFFFF0001;
    }
    else if (SDRAM_CS == 1)
    {
        *EMI_CALP &= ~0xFFFF0002;
    }
       
    *EMI_CONN &= ~CAL_EN;
    *EMI_DQSE &= ~0xFFFF;
    

    
    for(dqsix_dlysel=0x0; dqsix_dlysel<=0xFF; dqsix_dlysel+=DQSI_TUNING_STEP/* 8 */)
    {
        /*DEBUG*/
            //dbg_print("\n\rDQSV=%x ", dqsix_dlysel);
        
        
        /*Mask Auto Tracking Init Value*/        
        if (SDRAM_CS == 0)
        {
            *EMI_DQSA = dqsix_dlysel<<24 | dqsix_dlysel<<16 | dqsix_dlysel<<8 | dqsix_dlysel;
        }
        else if (SDRAM_CS == 1)
        {
            *EMI_DQSB = dqsix_dlysel<<24 | dqsix_dlysel<<16 | dqsix_dlysel<<8 | dqsix_dlysel;
        }        

        lbound_finding = 1;

        /*byte_delay = 0*/
        bytex_dly_mod = 0;
        *EMI_IDLI = 0;

        /*byte_setup = 0*/
        bytex_setup_mod = 0;
        *EMI_CALE = 0;
        *EMI_CALF = 0; // VST added

        /*Reset CALA/CALB/CALI/CALJ*/
        *EMI_CALA = 0;
        *EMI_CALB = 0; // VST added
        *EMI_CALI = 0;
        *EMI_CALJ = 0; // VST added

        /*Iterate dq_in delay 0x1F ~ 0*/
        for(dqy_in_del=0x1F; dqy_in_del>=0; dqy_in_del-=DATA_TUNING_STEP)
        {
            *EMI_IDLA = *EMI_IDLB = *EMI_IDLC = *EMI_IDLD = *EMI_IDLE = *EMI_IDLF = *EMI_IDLG = *EMI_IDLH =
            dqy_in_del<<24 | dqy_in_del<<16 | dqy_in_del<<8 | dqy_in_del;

            // Clear DDRFIFO
            *EMI_CALP |= 0x00000100;
            *EMI_CALP &= 0xFFFFFEFF;

            /* do DQS calibration */
            #if defined( ___EMIDATATRAIN_MBIST_DATATRAIN__ )
            test_result = __EmiDataTRainMbistTest( MBIST_TEST_BUFFER_START_ADDR, MBIST_TEST_BUFFER_END_ADDR );
            #else
            store_8word(0x0, 0x12345678);
            test_result = load_8word(0x0, 0x12345678);
            #endif

            /*DEBUG*/
            //dbg_print("%d ", (test_result == 0)?1:0 );

            /* R/W ok & during boundary finding ==> 0->1 , Record the start boundary*/
            if(lbound_finding==1 && test_result == 0 )
            {
                dqy_in_del_start = dqy_in_del;
                bytex_dly_mod_start = bytex_dly_mod;
                bytex_setup_mod_start = bytex_setup_mod;
                
                lbound_finding = 0;

            }
            /* R/W failk & not during boundary finding ==> 1->0 */
            else if(lbound_finding==0 && test_result != 0 )
            {
                goto window_found;
            }

            
        }

        dqy_in_del = 0; /*This value should be already be 0*/
        *EMI_IDLA = *EMI_IDLB = *EMI_IDLC = *EMI_IDLD = *EMI_IDLE = *EMI_IDLF = *EMI_IDLG = *EMI_IDLH = 0;
                
        for(bytex_setup_mod=0; bytex_setup_mod<=0x1F; bytex_setup_mod+=DATA_TUNING_STEP)
        {  
            *EMI_CALE = bytex_setup_mod<<24 | bytex_setup_mod<<16 | bytex_setup_mod<<8 | bytex_setup_mod;
            *EMI_CALF = bytex_setup_mod<<24 | bytex_setup_mod<<16 | bytex_setup_mod<<8 | bytex_setup_mod; // VST added

            // Clear DDRFIFO
            *EMI_CALP |= 0x00000100;
            *EMI_CALP &= 0xFFFFFEFF;

            /* do DQS calibration */
            #if defined( ___EMIDATATRAIN_MBIST_DATATRAIN__ )
            test_result = __EmiDataTRainMbistTest( MBIST_TEST_BUFFER_START_ADDR, MBIST_TEST_BUFFER_END_ADDR );
            #else
            store_8word(0x0, 0x12345678);
            test_result = load_8word(0x0, 0x12345678);
            #endif

            /*DEBUG*/
            //dbg_print("%d ", (test_result == 0)?1:0 );
            

            if(lbound_finding==1 && test_result == 0 )
            {
                bytex_dly_mod_start = bytex_dly_mod;
                bytex_setup_mod_start = bytex_setup_mod;
                dqy_in_del_start = dqy_in_del;

                lbound_finding = 0;

            }
            else if(lbound_finding==0 && test_result != 0 )
            {
                goto window_found;
            }

        }
        bytex_setup_mod=0x1F; /*This value should be already be 0x1F*/
        *EMI_CALE = bytex_setup_mod<<24 | bytex_setup_mod<<16 | bytex_setup_mod<<8 | bytex_setup_mod;
        *EMI_CALF = bytex_setup_mod<<24 | bytex_setup_mod<<16 | bytex_setup_mod<<8 | bytex_setup_mod; // VST added

        for(bytex_dly_mod=0; bytex_dly_mod<=0x1F; bytex_dly_mod+=DATA_TUNING_STEP)
        {
            //*EMI_CALA = bytex_dly_mod<<24 | bytex_dly_mod<<16 | bytex_dly_mod<<8 | bytex_dly_mod;
            *EMI_IDLI = bytex_dly_mod<<24 | bytex_dly_mod<<16 | bytex_dly_mod<<8 | bytex_dly_mod;

            // Clear DDRFIFO
            *EMI_CALP |= 0x00000100;
            *EMI_CALP &= 0xFFFFFEFF;

            /* do DQS calibration */
            #if defined( ___EMIDATATRAIN_MBIST_DATATRAIN__ )
            test_result = __EmiDataTRainMbistTest( MBIST_TEST_BUFFER_START_ADDR, MBIST_TEST_BUFFER_END_ADDR );
            #else
            store_8word(0x0, 0x12345678);
            test_result = load_8word(0x0, 0x12345678);
            #endif

            /*DEBUG*/
            //dbg_print("%d ", (test_result == 0)?1:0 );

            if(lbound_finding==1 && test_result == 0 )
            {
                bytex_dly_mod_start = bytex_dly_mod;
                bytex_setup_mod_start = bytex_setup_mod;
                dqy_in_del_start = dqy_in_del;

                lbound_finding = 0;

            }
            else if(lbound_finding==0 && test_result != 0 )
            {
                goto window_found;
            }

        }


        /*Find a windows that only have one-end boundary,ex. 000111111...*/
        if(lbound_finding == 0)
        {
            goto window_found;
        }

        /*window is not found, but previous windows found, it's also a shrink case, goto windows_found*/
        /*In this case, found a window size = 0 , ex. 00000000... */
        if( ( lbound_finding == 1 ) && ( b_wnd_found == 1 ) )
        {
#if 0
            debug_var = 3;
#endif
            if (DQSI_end == 0xff) DQSI_end = dqsix_dlysel-DQSI_TUNING_STEP;//IvanTseng: record the last DQSI value

            if (SDRAM_CS == 0)
            {
                *EMI_DQSA = prev_emi_dqsa;
            }
            else if (SDRAM_CS == 1)
            {
                *EMI_DQSB = prev_emi_dqsb;
            }          
            
            *EMI_IDLA = *EMI_IDLB = *EMI_IDLC = *EMI_IDLD = *EMI_IDLE = *EMI_IDLF = *EMI_IDLG = *EMI_IDLH = prev_emi_idl;
            *EMI_CALA = prev_emi_cala;
            *EMI_CALB = prev_emi_calb; // VST added
            *EMI_CALE = prev_emi_cale;
            *EMI_CALF = prev_emi_calf; // VST added
            *EMI_CALI = prev_emi_cali;
            *EMI_CALJ = prev_emi_calj; // VST added

            break;
        }
        

        /*window is not found, use next mask setting*/
        continue;

    window_found:

        
        


        if(bytex_dly_mod>0x1F)
        {
            // This is an unexpected case
            bytex_dly_mod = 0x1F;
        }

        if(bytex_setup_mod>0x1F)
        {
            // This is an unexpected case
            bytex_setup_mod = 0x1F;
        }

        if(dqy_in_del<0)
        {
            // This is an unexpected case       
            dqy_in_del = 0;
        }

        if(dqsix_dlysel>0xFF)
        {
            // This is an unexpected case
            dqsix_dlysel = 0xFF;
        }

        dwnd_size = (dqy_in_del_start-dqy_in_del)+(bytex_setup_mod-bytex_setup_mod_start)+(bytex_dly_mod-bytex_dly_mod_start);


        /*If windows <= 10, ignore this windows found,maybe it's a noise*/
        if( dwnd_size <= 10 )
        {
            continue;
        }
        else 
        {
            b_wnd_found = 1; //it means the DQSI is found
            
            #if DEBUG_MODE
                dbg_print("Window size = %d, DQSI=0x%x\n\r", dwnd_size, dqsix_dlysel);
            #endif
        }

        /*DEBUG*/
        //dbg_print(": %x\n\r",dwnd_size );
        if (DQSI_start==0x0) DQSI_start = dqsix_dlysel; //Record the 1st DQSI value

        if(prev_dwnd_size && (prev_dwnd_size > (dwnd_size+WINDOW_SIZE_THRESHOLD)))
        {
            DQSI_end = dqsix_dlysel-DQSI_TUNING_STEP;//IvanTseng: record the last DQSI value
            if (SDRAM_CS == 0)
            {
                *EMI_DQSA = prev_emi_dqsa;
            }
            else if (SDRAM_CS == 1)
            {
                *EMI_DQSB = prev_emi_dqsb;
            }
            
            *EMI_IDLA = *EMI_IDLB = *EMI_IDLC = *EMI_IDLD = *EMI_IDLE = *EMI_IDLF = *EMI_IDLG = *EMI_IDLH = prev_emi_idl;
            *EMI_CALA = prev_emi_cala;
            *EMI_CALB = prev_emi_calb; // VST added
            *EMI_CALE = prev_emi_cale;
            *EMI_CALF = prev_emi_calf; // VST added
            *EMI_CALI = prev_emi_cali;
            *EMI_CALJ = prev_emi_calj; // VST added

            
            /*------------------------------------------------------------------------------
                BELOW INFORMATION to be confirm:
                Once find a windows less or equal previous one, use:
                    1. Previous delay setting
                    2. current mask setting ( in case the previous one is in mask boundary )
                 and finish data training.
              ------------------------------------------------------------------------------*/
            break;
        }

        prev_dwnd_size = dwnd_size;

        /*Use only for a "valid windows size" shrink to "zero windows size" immmediately.*/
        if (SDRAM_CS == 0)
        {    
            prev_emi_dqsa = *EMI_DQSA;
        }
        else if (SDRAM_CS == 1)
        {        
            prev_emi_dqsb = *EMI_DQSB;
        }

        dqs_center_point = bytex_setup_mod_start + (dwnd_size/2) - 3;

        dll_value = *EMI_DLLV;
        dll_value = (((dll_value & 0x1F000000) >> 24) + ((dll_value & 0x001F0000) >> 16) +
                     ((dll_value & 0x00001F00) >>  8) + ((dll_value & 0x0000001F) >>  0)) / 4;

        // Data in delay
        if (dll_value > dqs_center_point)
          value = dll_value - dqs_center_point;
        else
          value = 0;

        *EMI_IDLA = *EMI_IDLB = *EMI_IDLC = *EMI_IDLD = *EMI_IDLE = *EMI_IDLF = *EMI_IDLG = *EMI_IDLH = prev_emi_idl =
          value<<24 | value<<16 | value<<8 | value;

        // DQS delay
        if (dll_value > dqs_center_point)
          value = 0;
        else
          value = dqs_center_point - dll_value;

        *EMI_CALA = prev_emi_cala = value<<24 | value<<16 | value<<8 | value;
        *EMI_CALB = prev_emi_calb = value<<24 | value<<16 | value<<8 | value; // VST added

        // DQS setup
        value = dll_value;
        *EMI_CALE = prev_emi_cale = value<<24 | value<<16 | value<<8 | value;
        *EMI_CALF = prev_emi_calf = value<<24 | value<<16 | value<<8 | value; // VST added

        // DQS hold
        value = ( dwnd_size/2 > 31 ) ? 31 : dwnd_size/2;
        *EMI_CALI = prev_emi_cali = value<<24 | value<<16 | value<<8 | value; 
        *EMI_CALJ = prev_emi_calj = value<<24 | value<<16 | value<<8 | value; 

        /*Go next mask setting*/
        
    }

    /* IvanTseng : Get the proper DQSI value here */
    DQSI_center = (DQSI_start + DQSI_end)/2;

    if (SDRAM_CS == 0)
    {
        *EMI_DQSA = DQSI_center<<24 | DQSI_center<<16 | DQSI_center<<8 | DQSI_center;
    }
    else if (SDRAM_CS == 1)
    {
        *EMI_DQSB = DQSI_center<<24 | DQSI_center<<16 | DQSI_center<<8 | DQSI_center;
    }

    /*------------------------------------------------------------------------------
        Set up MAX "Data Setup" & " Data Hold"
      ------------------------------------------------------------------------------*/
     value = ( prev_dwnd_size/2 > 31 ) ? 31 : prev_dwnd_size/2;
    *EMI_CALP &= 0x0000FFFF;
    *EMI_CALP |= ( 1 << 31 ) | ( value << 24) | ( 1 << 23 ) | ( value << 16 );


#if 0
    /*------------------------------------------------------------------------------
        For Debugging
      ------------------------------------------------------------------------------*/
      if( (*EMI_DQSA & 0xFF ) == 0x78 )
      {
        debug_var = 2;
      }
#endif          


     /*------------------------------------------------------------------------------
         Return Training Result and Restore Register
       ------------------------------------------------------------------------------*/
     __EmiDataTrainRegRead( pResult );      /*Return Training Result*/
     __EmiDataTrainRegWrite( &REG_BAK );    /*Restore Register*/


    /*Print Training Result*/
    #if 0
    __EmiDataTrainRegRead( &REG_BAK ); //Test
    dbg_print("\n\r");
    dbg_print("*EMI_CONL=0x%x\n\r", *EMI_CONL );
    dbg_print("*EMI_CONN=0x%x\n\r", pResult->EMI_CONN_regval );
    dbg_print("EMI_DQSA=0x%x\n\r", pResult->EMI_DQSA_regval );
    dbg_print("EMI_DQSA=0x%x\n\r", pResult->EMI_DQSB_regval );
    dbg_print("*EMI_DQSE=0x%x\n\r", pResult->EMI_DQSE_regval );
    dbg_print("EMI_IDLA=0x%x\n\r", pResult->EMI_IDLA_regval );
    dbg_print("EMI_IDLB=0x%x\n\r", pResult->EMI_IDLB_regval );
    dbg_print("EMI_IDLC=0x%x\n\r", pResult->EMI_IDLC_regval );
    dbg_print("EMI_IDLD=0x%x\n\r", pResult->EMI_IDLD_regval );
    dbg_print("EMI_IDLE=0x%x\n\r", pResult->EMI_IDLE_regval );
    dbg_print("EMI_IDLF=0x%x\n\r", pResult->EMI_IDLF_regval );
    dbg_print("EMI_IDLG=0x%x\n\r", pResult->EMI_IDLG_regval );
    dbg_print("EMI_IDLH=0x%x\n\r", pResult->EMI_IDLH_regval );
    dbg_print("EMI_IDLI=0x%x\n\r", pResult->EMI_IDLI_regval );
    dbg_print("EMI_CALA=0x%x\n\r", pResult->EMI_CALA_regval );
    dbg_print("EMI_CALB=0x%x\n\r", pResult->EMI_CALB_regval ); // VST added
    dbg_print("EMI_CALE=0x%x\n\r", pResult->EMI_CALE_regval );
    dbg_print("EMI_CALF=0x%x\n\r", pResult->EMI_CALF_regval ); // VST added
    dbg_print("EMI_CALI=0x%x\n\r", pResult->EMI_CALI_regval );
    dbg_print("EMI_CALJ=0x%x\n\r", pResult->EMI_CALJ_regval ); // VST added
    dbg_print("EMI_CALP=0x%x\n\r", pResult->EMI_CALP_regval );
    dbg_print("\n\r");
    #endif
     
    return  b_wnd_found;
    
}