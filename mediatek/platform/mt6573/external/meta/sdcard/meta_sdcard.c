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


#include "meta_sdcard.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define FREEIF(p)   do { if(p) free(p); (p) = NULL; } while(0)
#define CHKERR(x)   do { if((x) < 0) goto error; } while(0)

static char *path = NULL;
static SDCARD_CNF_CB cnf_cb = NULL;

static int meta_sdcard_read_info(const char *filename, void *buf, ssize_t bufsz)
{
    int fd, rsize;

    if ((fd = open(filename, O_RDONLY)) < 0) {
        return -1;
    }

    rsize = read(fd, buf, bufsz);

    close(fd);

    return rsize;
}

static void meta_sdcard_send_resp(SDCARD_CNF *cnf)
{
    if (cnf_cb)
        cnf_cb(cnf);
    else
        WriteDataToPC(cnf, sizeof(SDCARD_CNF), NULL, 0);
}

void Meta_SDcard_Register(SDCARD_CNF_CB callback)
{
    cnf_cb = callback;
}

bool Meta_SDcard_Init(SDCARD_REQ *req)
{
    int id = (int)req->dwSDHCIndex;
    char name[20];
    char *ptr;
    DIR *dp;
    struct dirent *dirp;

    if (id < MIN_SDCARD_IDX || id > MAX_SDCARD_IDX)
        return false;

    if (!path && NULL == (path = malloc(512))) {
        perror("No memory\n");
        return false;
    }

    sprintf(name, "mmc%d", id - MIN_SDCARD_IDX);

    ptr = path;
    ptr += sprintf(ptr, "/sys/class/mmc_host/%s", name);

    if (NULL == (dp = opendir(path)))
        goto error;

    while (NULL != (dirp = readdir(dp))) {
        if (strstr(dirp->d_name, name)) {
            ptr += sprintf(ptr, "/%s", dirp->d_name);
            break;
        }
    }

    closedir(dp);

    if (!dirp)
        goto error;

    return true;

error:
    printf("[META_SD] path not found: %s/%s\n", path, name); 

    FREEIF(path);
    return false;
}

bool Meta_SDcard_Deinit(void)
{
    FREEIF(path);    
    return true;
}


void Meta_SDcard_OP(SDCARD_REQ *req, char *peer_buf, unsigned short peer_len)
{
    SDCARD_CNF cnf;
    int bufsz = 512;
    char fname[512];
    char buf[512];

    memset(&cnf, 0, sizeof(SDCARD_CNF));

    cnf.header.id = FT_SDCARD_CNF_ID;
    cnf.header.token = req->header.token;
    cnf.status = META_SUCCESS;

    sprintf(fname, "%s/bus_width", path);
    CHKERR(meta_sdcard_read_info(fname, buf, bufsz));
    cnf.DatBusWidth = atoi(buf);

    sprintf(fname, "%s/sec_mode", path);
    CHKERR(meta_sdcard_read_info(fname, buf, bufsz));
    cnf.IsSecuredMode = atoi(buf);

    sprintf(fname, "%s/card_type", path);
    CHKERR(meta_sdcard_read_info(fname, buf, bufsz));
    cnf.CardTpye = atoi(buf);

    sprintf(fname, "%s/prot_area_sz", path);
    CHKERR(meta_sdcard_read_info(fname, buf, bufsz));
    cnf.SizeOfProtectErea = atoi(buf);

    sprintf(fname, "%s/spd_class", path);
    CHKERR(meta_sdcard_read_info(fname, buf, bufsz));
    cnf.SpeedClass = atoi(buf);

    sprintf(fname, "%s/perf_move", path);
    CHKERR(meta_sdcard_read_info(fname, buf, bufsz));
    cnf.Performance_move = atoi(buf);

    sprintf(fname, "%s/au_size", path);
    CHKERR(meta_sdcard_read_info(fname, buf, bufsz));
    cnf.AUSize = atoi(buf);

    meta_sdcard_send_resp(&cnf);
    return;

error:
    cnf.status = META_FAILED;
    meta_sdcard_send_resp(&cnf);
    return;
}

