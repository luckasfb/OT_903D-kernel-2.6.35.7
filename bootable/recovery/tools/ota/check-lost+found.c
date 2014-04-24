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

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/klog.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "private/android_filesystem_config.h"

// Sentinel file used to track whether we've forced a reboot
static const char *kMarkerFile = "/data/misc/check-lost+found-rebooted-2";

// Output file in tombstones directory (first 8K will be uploaded)
static const char *kOutputDir = "/data/tombstones";
static const char *kOutputFile = "/data/tombstones/check-lost+found-log";

// Partitions to check
static const char *kPartitions[] = { "/system", "/data", "/cache", NULL };

/*
 * 1. If /data/misc/forced-reboot is missing, touch it & force "unclean" boot.
 * 2. Write a log entry with the number of files in lost+found directories.
 */

int main(int argc, char **argv) {
    mkdir(kOutputDir, 0755);
    chown(kOutputDir, AID_SYSTEM, AID_SYSTEM);
    FILE *out = fopen(kOutputFile, "a");
    if (out == NULL) {
        fprintf(stderr, "Can't write %s: %s\n", kOutputFile, strerror(errno));
        return 1;
    }

    // Note: only the first 8K of log will be uploaded, so be terse.
    time_t start = time(NULL);
    fprintf(out, "*** check-lost+found ***\nStarted: %s", ctime(&start));

    struct stat st;
    if (stat(kMarkerFile, &st)) {
        // No reboot marker -- need to force an unclean reboot.
        // But first, try to create the marker file.  If that fails,
        // skip the reboot, so we don't get caught in an infinite loop.

        int fd = open(kMarkerFile, O_WRONLY|O_CREAT, 0444);
        if (fd >= 0 && close(fd) == 0) {
            fprintf(out, "Wrote %s, rebooting\n", kMarkerFile);
            fflush(out);
            sync();  // Make sure the marker file is committed to disk

            // If possible, dirty each of these partitions before rebooting,
            // to make sure the filesystem has to do a scan on mount.
            int i;
            for (i = 0; kPartitions[i] != NULL; ++i) {
                char fn[PATH_MAX];
                snprintf(fn, sizeof(fn), "%s/%s", kPartitions[i], "dirty");
                fd = open(fn, O_WRONLY|O_CREAT, 0444);
                if (fd >= 0) {  // Don't sweat it if we can't write the file.
                    write(fd, fn, sizeof(fn));  // write, you know, some data
                    close(fd);
                    unlink(fn);
                }
            }

            reboot(RB_AUTOBOOT);  // reboot immediately, with dirty filesystems
            fprintf(out, "Reboot failed?!\n");
            exit(1);
        } else {
            fprintf(out, "Can't write %s: %s\n", kMarkerFile, strerror(errno));
        }
    } else {
        fprintf(out, "Found %s\n", kMarkerFile);
    }

    int i;
    for (i = 0; kPartitions[i] != NULL; ++i) {
        char fn[PATH_MAX];
        snprintf(fn, sizeof(fn), "%s/%s", kPartitions[i], "lost+found");
        DIR *dir = opendir(fn);
        if (dir == NULL) {
            fprintf(out, "Can't open %s: %s\n", fn, strerror(errno));
        } else {
            int count = 0;
            struct dirent *ent;
            while ((ent = readdir(dir))) {
                if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, ".."))
                    ++count;
            }
            closedir(dir);
            if (count > 0) {
                fprintf(out, "OMGZ FOUND %d FILES IN %s\n", count, fn);
            } else {
                fprintf(out, "%s is clean\n", fn);
            }
        }
    }

    char dmesg[131073];
    int len = klogctl(KLOG_READ_ALL, dmesg, sizeof(dmesg) - 1);
    if (len < 0) {
        fprintf(out, "Can't read kernel log: %s\n", strerror(errno));
    } else {  // To conserve space, only write lines with certain keywords
        fprintf(out, "--- Kernel log ---\n");
        dmesg[len] = '\0';
        char *saveptr, *line;
        int in_yaffs = 0;
        for (line = strtok_r(dmesg, "\n", &saveptr); line != NULL;
             line = strtok_r(NULL, "\n", &saveptr)) {
            if (strstr(line, "yaffs: dev is")) in_yaffs = 1;

            if (in_yaffs ||
                    strstr(line, "yaffs") ||
                    strstr(line, "mtd") ||
                    strstr(line, "msm_nand")) {
                fprintf(out, "%s\n", line);
            }

            if (strstr(line, "yaffs_read_super: isCheckpointed")) in_yaffs = 0;
        }
    }

    return 0;
}
