#!/bin/bash
# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.

# MediaTek Inc. (C) 2011. All rights reserved.
#
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
# RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
# AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#
# The following software/firmware and/or related documentation ("MediaTek Software")
# have been modified by MediaTek Inc. All revisions are subject to any receiver's
# applicable license agreements with MediaTek Inc.


##############################################################
# Program:
# Program to create ALPS preloader binary
#

function build_preloader () {

    if [ "$1" != "" ]; then export TARGET_PRODUCT=$1; fi

    source ../../../mediatek/build/shell.sh ../../../ preloader
    CUR_DIR=`pwd`

    ##############################################################
    # Variable Initialization
    #

    PL_CAN_USE_SIZE=19000 #(0x90020000-0x90007000)

    PL_IMAGE=bin/preloader_${MTK_PROJECT}.bin
    PL_ELF_IMAGE=bin/preloader_${MTK_PROJECT}.elf

    ##############################################################
    # Binary Generation
    #

    make

    if [ ! -f "${PL_IMAGE}" ]; then echo "BUILD FAIL."; exit 1; fi

    PL_FUN_MAP=function.map
    
    ##############################################################
    # Binary Size Check
    #

    PL_SIZE=$(stat -c%s "${PL_IMAGE}")    
    if [ $PL_SIZE -gt 68500 ]; then
        echo "===================== Building Fail ========================"
        echo "---------------------------------------------------------------"
        echo " image size : ${PL_SIZE} cannot be greater than 68500 bytes !! "
        echo " please reduce your code size first then compile again !! "
        echo "==============================================================="
        size `ls out/*.o` > pl-code-size-report.txt
        echo "---------------------------------------------------------------"
        echo "                      CODE SIZE REPORT                         "
        echo "---------------------------------------------------------------"
        echo "size(bytes)     file  ( size > 2500 bytes )"
        echo "---------------------------------------------------------------"               
        #awk {'if ($4>2500) print $4 "\t\t" $6}' < pl-code-size-report.txt | sort -rn        
        rm ${PL_IMAGE}
        echo "BUILD FAIL."
        exit 1;
    fi
    
    ##############################################################
    # BSS Size Check :  read (_bss_end - _ _bss_start) + pre-loader.bin size 
    #

    BSS_USE_SIZE=$(awk '{if($3=="_bss_start") {BSS_START= strtonum("0x" $1)} ;
                 if($3=="_bss_end") {BSS_END=strtonum("0x" $1)}}
                 END{printf("%d\n",BSS_END-BSS_START)}' ${PL_FUN_MAP})

    USE_SIZE=`expr $BSS_USE_SIZE + $PL_SIZE`
    ((TOTAL_CAN_USE_SIZE=16#$PL_CAN_USE_SIZE))
    if [ $USE_SIZE -gt $TOTAL_CAN_USE_SIZE ]; then
        echo "===================== Building Fail ==========================="
        echo "---------------------------------------------------------------"
        echo " Now PL use space size: ${USE_SIZE} bytes (pl.bin size=${PL_SIZE} bytes, bss size=${BSS_USE_SIZE}) bytes"
        echo " PL can use space size: ${TOTAL_CAN_USE_SIZE} bytes(0x90020000--0x90007000)"
        echo " preloader size cannot be greater than Preloader can use size"
        echo " please reduce your bss size first then compile again !!       "
        echo "==============================================================="
        echo " Please check the report file pl_bss_size_report.txt" 
        echo " [${CUR_DIR}] "
        echo "								     "
        echo "		    BSS SIZE REPORT			     " > pl_bss_size_report.txt
       	echo "file 			        (bss size > 0 bytes )" >> pl_bss_size_report.txt

        for objfile in `ls out/*o`
        do
		echo  "							       " >> pl_bss_size_report.txt
		echo  "--------------------------------------------------------" >> pl_bss_size_report.txt
		echo  "The bss size of $objfile :" | sed 's/\.o/.c/g' | sed 's/out\///g' >> pl_bss_size_report.txt
                readelf -SW $objfile | grep "bss" | sed 's/\[\s/[/g'| awk '{if ($6 > 0)  printf "%-40s\t%8s\n",$2, $6}' |
		sort -k2 >> pl_bss_size_report.txt
        done
        rm -f ${PL_IMAGE}
        echo "BUILD FAIL."
        exit 1;
    fi
}

function ns_chip () {

    ##############################################################
    # Only Support Non-Secure Chip
    #        

    echo ""
    echo "[ Only for Non-Secure Chip ]"
    echo "============================================"
    
    GFH_PATH=${CHIP_CONFIG_PATH}/ns/  
    
    ##############################################################
    # INITIALIZE GFH
    #
    
    if [ "${MTK_EMMC_SUPPORT}" == "yes" ]; then
        GFH_INFO=${GFH_PATH}/GFH_INFO_EMMC.txt
    else
        GFH_INFO=${GFH_PATH}/GFH_INFO.txt
    fi
    GFH_HASH=${GFH_PATH}/GFH_HASH.txt

    ##############################################################
    # ATTACH GFH
    #

    echo ""      
    echo "[ Attach ${MTK_PLATFORM} GFH ]"
    echo "============================================"
    echo " : GFH_INFO             - ${GFH_INFO}"
    echo " : GFH_HASH             - ${GFH_HASH}"

    chmod u+w ${PL_IMAGE}
    mv -f ${PL_IMAGE} ${PL_IMAGE/%.bin/_NO_GFH.bin}
    cp -f ${GFH_INFO} ${PL_IMAGE}	    
    chmod u+w ${PL_IMAGE} ${PL_IMAGE/%.bin/_NO_GFH.bin}
    cat ${PL_IMAGE/%.bin/_NO_GFH.bin} >> ${PL_IMAGE}
    cat ${GFH_HASH} >> ${PL_IMAGE}

    ##############################################################
    # PROCESS BOOT LOADER
    #

    chmod 777 ${PBP_TOOL}/PBP.exe
    
    WINEPATH=`which wine`

    if [ -n "$WINEPATH" ]; then	
	wine ${PBP_TOOL}/PBP.exe ${PL_IMAGE}
    else
    	echo "===BUILD FAIL. need wine to execution PBP.exe==="
	echo "===Please check your environment variable!===" 
	exit 1;
    fi 
}

function s_chip_support () {

    ##############################################################
    # Can Support Secure Chip
    #

    echo ""
    echo "[ Enable Secure Chip Support ]"
    echo "============================================"

    GFH_PATH=${CHIP_CONFIG_PATH}/s/gfh    
    CONFIG_PATH=${CHIP_CONFIG_PATH}/s/cfg
    KEY_PATH=${CHIP_CONFIG_PATH}/s/key
    
    ##############################################################
    # INITIALIZE CONFIG and KEY
    #
    
    CHIP_CONFIG=${CONFIG_PATH}/CHIP_CONFIG.ini
    CHIP_KEY=${KEY_PATH}/CHIP_TEST_KEY.ini
    
    ##############################################################
    # INITIALIZE GFH
    #
    
    if [ "${MTK_EMMC_SUPPORT}" == "yes" ]; then
        GFH_INFO=${GFH_PATH}/GFH_INFO_EMMC.txt
    else
    	GFH_INFO=${GFH_PATH}/GFH_INFO.txt
    fi
    GFH_SEC_KEY=${GFH_PATH}/GFH_SEC_KEY.txt
    GFH_ANTI_CLONE=${GFH_PATH}/GFH_ANTI_CLONE.txt
    GFH_HASH_SIGNATURE=${GFH_PATH}/GFH_HASH_AND_SIG.txt
    GFH_PADDING=${GFH_PATH}/GFH_PADDING.txt

    source ${CONFIG_PATH}/SECURE_JTAG_CONFIG.ini
    if [ "${SECURE_JTAG_ENABLE}" == "TRUE" ]; then
        SECURE_JTAG_GFH=${GFH_PATH}/GFH_SEC_CFG_JTAG_ON.txt
        echo " : SECURE_JTAG_ENABLE - TRUE"
    elif [ "${SECURE_JTAG_ENABLE}" == "FALSE" ]; then
        SECURE_JTAG_GFH=${GFH_PATH}/GFH_SEC_CFG_JTAG_OFF.txt
        echo " : SECURE_JTAG_ENABLE - FALSE"
    else
        echo "BUILD FAIL. SECURE_JTAG_ENABLE not defined in ${CONFIG_PATH}/SECURE_JTAG_CONFIG.ini"
        exit 1;
    fi

    ##############################################################
    # ATTACH GFH
    #

    echo ""      
    echo "[ Attach ${MTK_PLATFORM} GFH ]"
    echo "============================================"
    echo " : GFH_INFO             - ${GFH_INFO}"
    echo " : GFH_SEC_KEY          - ${GFH_SEC_KEY}"
    echo " : GFH_ANTI_CLONE       - ${GFH_ANTI_CLONE}"
    echo " : GFH_JTAG_CFG         - ${SECURE_JTAG_GFH}"
    echo " : GFH_PADDING          - ${GFH_PADDING}"
    echo " : GFH_HASH_SIGNATURE   - ${GFH_HASH_SIGNATURE}"

    chmod u+w ${PL_IMAGE}
    mv -f ${PL_IMAGE} ${PL_IMAGE/%.bin/_NO_GFH.bin}
    cp -f ${GFH_INFO} ${PL_IMAGE}	    
    chmod 777 ${PL_IMAGE}
    cat ${GFH_SEC_KEY} >> ${PL_IMAGE}
    cat ${GFH_ANTI_CLONE} >> ${PL_IMAGE}
    cat ${SECURE_JTAG_GFH} >> ${PL_IMAGE}
    cat ${GFH_PADDING} >> ${PL_IMAGE}
    chmod u+w ${PL_IMAGE} ${PL_IMAGE/%.bin/_NO_GFH.bin}
    cat ${PL_IMAGE/%.bin/_NO_GFH.bin} >> ${PL_IMAGE}
    cat ${GFH_HASH_SIGNATURE} >> ${PL_IMAGE}

    echo ""
    echo "[ Load Configuration ]"
    echo "============================================"
    echo " : CONFIG               - ${CHIP_CONFIG}"
    echo " : RSA KEY              - ${CHIP_KEY}"	
    echo " : AC_K                 - ${CHIP_KEY}"

    ##############################################################
    # PROCESS BOOT LOADER
    #

    chmod 777 ${PBP_TOOL}/PBP.exe

    WINEPATH=`which wine` 
    if [ -n "$WINEPATH" ]; then
        wine ${PBP_TOOL}/PBP.exe -m ${CHIP_CONFIG} -i ${CHIP_KEY} ${PL_IMAGE}
    else            
    	echo "===BUILD FAIL. need wine to execution PBP.exe==="
	echo "===Please check your environment variable!===" 
	exit 1;
    fi
} 

function key_encode () {

    ##############################################################
    # Encode Key
    #

    KEY_ENCODE_TOOL=tools/ke/KeyEncode
    if [ -e ${KEY_ENCODE_TOOL} ]; then    

        ./${KEY_ENCODE_TOOL} ${PL_IMAGE} KEY_ENCODED_PL
        
        if [ -e KEY_ENCODED_PL ]; then    
            rm ${PL_IMAGE}
            mv KEY_ENCODED_PL ${PL_IMAGE}
        fi
    fi
}

function post_process () {

    ##############################################################
    # Binary Secure Postprocessing
    #        

    PBP_TOOL=tools/pbp
    CUSTOM_PATH=${MTK_ROOT_CUSTOM}/${MTK_PROJECT}/security
    CHIP_CONFIG_PATH=${CUSTOM_PATH}/chip_config
    WINEPATH=`which wine`
    if [ -e ${PBP_TOOL}/PBP.exe ]; then

        echo ""
        echo "[ Pre-loader Post Processing ]"
        echo "============================================"
        if [ -z "$WINEPATH" ]; then
            echo "===BUILD FAIL. need wine tool to sign pre-loader binary==="
	    echo "===Please check your environment variable!===" 
            exit 1;
        fi

        ##############################################################
        # ENCODE KEY FIRST
        #        
        key_encode;

        ##############################################################
        # CHECK CHIP TYPE
        #        

        if [ -e ${CHIP_CONFIG_PATH}/s/gfh/GFH_INFO.txt ] || [ -e ${CHIP_CONFIG_PATH}/s/gfh/GFH_INFO_EMMC.txt ]; then
            
            echo ""
            echo "[ Load Chip Config. ]"
            echo "============================================"                        
            echo " : MTK_SEC_CHIP_SUPPORT - ${MTK_SEC_CHIP_SUPPORT}"                 

            if [ "${MTK_SEC_CHIP_SUPPORT}" == "no" ]; then

                ##############################################################
                # ONLY SUPPORT NON-SECURE CHIP
                #

                CHIP_CONFIG_PATH=${MTK_PATH_PLATFORM}/gfh/default
                ns_chip;
                
            elif [ "${MTK_SEC_CHIP_SUPPORT}" == "yes" ]; then

                ##############################################################
                # CAN SUPPORT SECURE CHIP
                #
                
                CHIP_CONFIG_PATH=${CUSTOM_PATH}/chip_config
                s_chip_support;
                
            else
   
                echo "BUILD FAIL. MTK_SEC_CHIP_SUPPORT not defined in ProjectConfig.mk"
                exit 1;
            fi
            
        else

            ##############################################################
            # NO CONFIGURATION IS FOUND. APPLY DEFAULT SETTING
            #
            
            echo "${CHIP_CONFIG_PATH}/s/gfh/GFH_INFO.txt not found."
            echo "Suppose it is non-secure chip and apply default config."
            CHIP_CONFIG_PATH=${MTK_PATH_PLATFORM}/gfh/default
            ns_chip;
        fi            
    fi
}

function dump_build_info () {

    ##############################################################
    # Dump Message
    #

    echo ""          
    echo "============================================"
    echo "${MTK_PROJECT} preloader load"
    echo "${PL_IMAGE} built at"
    echo "time : $(date)"
    echo "img size : $(stat -c%s "${PL_IMAGE}")" byte
    echo "bss size : 0x$(readelf -SW "${PL_ELF_IMAGE}"|grep "bss" | awk '{print $6}')" byte
    echo "============================================"

    PL_ELF_IMAGE=bin/preloader_${MTK_PROJECT}.elf

    chmod a+w ${PL_IMAGE}
    cp -f ${PL_IMAGE} .
}

function copy_binary () {

    ##############################################################
    # Copy Binary to Output Direcory
    #

    copy_to_legacy_download_flash_folder   ${PL_IMAGE}
}


##############################################################
# Main Flow
#
build_preloader;
post_process;
dump_build_info;
copy_binary;
