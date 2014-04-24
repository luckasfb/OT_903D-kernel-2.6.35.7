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
 */

/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cutils/log.h"
#include "mtdutils.h"

#define LOG_TAG "flash_image"

#define HEADER_SIZE 2048  // size of header to compare for equality

void die(const char *msg, ...) {
    int err = errno;
    va_list args;
    va_start(args, msg);
    char buf[1024];
    vsnprintf(buf, sizeof(buf), msg, args);
    va_end(args);

    if (err != 0) {
        strlcat(buf, ": ", sizeof(buf));
        strlcat(buf, strerror(err), sizeof(buf));
    }

    fprintf(stderr, "%s\n", buf);
    LOGE("%s\n", buf);
    exit(1);
}

/* Read an image file and write it to a flash partition. */

int main(int argc, char **argv) {
    const MtdPartition *ptn;
    MtdWriteContext *write;
    void *data;
    unsigned sz;

    if (argc != 3) {
        fprintf(stderr, "usage: %s partition file.img\n", argv[0]);
        return 2;
    }

    if (mtd_scan_partitions() <= 0) die("error scanning partitions");
    const MtdPartition *partition = mtd_find_partition_by_name(argv[1]);
    if (partition == NULL) die("can't find %s partition", argv[1]);

    // If the first part of the file matches the partition, skip writing

    int fd = open(argv[2], O_RDONLY);
    if (fd < 0) die("error opening %s", argv[2]);

    char header[HEADER_SIZE];
    int headerlen = read(fd, header, sizeof(header));
    if (headerlen <= 0) die("error reading %s header", argv[2]);

    MtdReadContext *in = mtd_read_partition(partition);
    if (in == NULL) {
        LOGW("error opening %s: %s\n", argv[1], strerror(errno));
        // just assume it needs re-writing
    } else {
        char check[HEADER_SIZE];
        int checklen = mtd_read_data(in, check, sizeof(check));
        if (checklen <= 0) {
            LOGW("error reading %s: %s\n", argv[1], strerror(errno));
            // just assume it needs re-writing
        } else if (checklen == headerlen && !memcmp(header, check, headerlen)) {
            LOGI("header is the same, not flashing %s\n", argv[1]);
            return 0;
        }
        mtd_read_close(in);
    }

    // Skip the header (we'll come back to it), write everything else
    LOGI("flashing %s from %s\n", argv[1], argv[2]);

    MtdWriteContext *out = mtd_write_partition(partition);
    if (out == NULL) die("error writing %s", argv[1]);

    char buf[HEADER_SIZE];
    memset(buf, 0, headerlen);
    int wrote = mtd_write_data(out, buf, headerlen);
    if (wrote != headerlen) die("error writing %s", argv[1]);

    int len;
    while ((len = read(fd, buf, sizeof(buf))) > 0) {
        wrote = mtd_write_data(out, buf, len);
        if (wrote != len) die("error writing %s", argv[1]);
    }
    if (len < 0) die("error reading %s", argv[2]);

    if (mtd_write_close(out)) die("error closing %s", argv[1]);

    // Now come back and write the header last

    out = mtd_write_partition(partition);
    if (out == NULL) die("error re-opening %s", argv[1]);

    wrote = mtd_write_data(out, header, headerlen);
    if (wrote != headerlen) die("error re-writing %s", argv[1]);

    // Need to write a complete block, so write the rest of the first block
    size_t block_size;
    if (mtd_partition_info(partition, NULL, &block_size, NULL))
        die("error getting %s block size", argv[1]);

    if (lseek(fd, headerlen, SEEK_SET) != headerlen)
        die("error rewinding %s", argv[2]);

    int left = block_size - headerlen;
    while (left < 0) left += block_size;
    while (left > 0) {
        len = read(fd, buf, left > (int)sizeof(buf) ? (int)sizeof(buf) : left);
        if (len <= 0) die("error reading %s", argv[2]);
        if (mtd_write_data(out, buf, len) != len)
            die("error writing %s", argv[1]);
        left -= len;
    }

    if (mtd_write_close(out)) die("error closing %s", argv[1]);
    return 0;
}
