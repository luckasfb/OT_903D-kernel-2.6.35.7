/* Copyright Statement:
 * 
 * This software/firmware and related documentation ("MediaTek Software") are 
 * protected under relevant copyright laws. The information contained herein is 
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without 
 * the prior written permission of MediaTek inc. and/or its licensors, any 
 * reproduction, modification, use or disclosure of MediaTek Software, and 
 * information contained herein, in whole or in part, shall be strictly 
 * prohibited.  
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES 
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") 
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON 
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER 
 * DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE SOFTWARE OF 
 * ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE 
 * MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTY FOR 
 * ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT 
 * IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER 
 * LICENSES CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE 
 * RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO RECEIVER'S 
 * SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM. 
 * RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE 
 * LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 *  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, 
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO 
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */
/* libs/diskconfig/diskutils.c
 *
 * Copyright 2008, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "diskutils"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <cutils/log.h>

#include "diskconfig.h"

int
write_raw_image(const char *dst, const char *src, loff_t offset, int test)
{
    int dst_fd = -1;
    int src_fd = -1;
    uint8_t buffer[2048];
    int nr_bytes;
    int tmp;
    int done = 0;
    uint64_t total = 0;

    LOGI("Writing RAW image '%s' to '%s' (offset=%llu)", src, dst, offset);
    if ((src_fd = open(src, O_RDONLY)) < 0) {
        LOGE("Could not open %s for reading (errno=%d).", src, errno);
        goto fail;
    }

    if (!test) {
        if ((dst_fd = open(dst, O_RDWR)) < 0) {
            LOGE("Could not open '%s' for read/write (errno=%d).", dst, errno);
            goto fail;
        }

        if (lseek64(dst_fd, offset, SEEK_SET) != offset) {
            LOGE("Could not seek to offset %lld in %s.", offset, dst);
            goto fail;
        }
    }

    while (!done) {
        if ((nr_bytes = read(src_fd, buffer, sizeof(buffer))) < 0) {
            /* XXX: Should we not even bother with EINTR? */
            if (errno == EINTR)
                continue;
            LOGE("Error (%d) while reading from '%s'", errno, src);
            goto fail;
        }

        if (!nr_bytes) {
            /* we're done. */
            done = 1;
            break;
        }

        total += nr_bytes;

        /* skip the write loop if we're testing */
        if (test)
            nr_bytes = 0;

        while (nr_bytes > 0) {
            if ((tmp = write(dst_fd, buffer, nr_bytes)) < 0) {
                /* XXX: Should we not even bother with EINTR? */
                if (errno == EINTR)
                    continue;
                LOGE("Error (%d) while writing to '%s'", errno, dst);
                goto fail;
            }
            if (!tmp)
                continue;
            nr_bytes -= tmp;
        }
    }

    if (!done) {
        LOGE("Exited read/write loop without setting flag! WTF?!");
        goto fail;
    }

    if (dst_fd >= 0)
        fsync(dst_fd);

    LOGI("Wrote %llu bytes to %s @ %lld", total, dst, offset);

    close(src_fd);
    if (dst_fd >= 0)
        close(dst_fd);
    return 0;

fail:
    if (dst_fd >= 0)
        close(dst_fd);
    if (src_fd >= 0)
        close(src_fd);
    return 1;
}
