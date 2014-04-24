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
 * Copyright (C) 2007 The Android Open Source Project
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>

#include "DirUtil.h"

typedef enum { DMISSING, DDIR, DILLEGAL } DirStatus;

static DirStatus
getPathDirStatus(const char *path)
{
    struct stat st;
    int err;

    err = stat(path, &st);
    if (err == 0) {
        /* Something's there; make sure it's a directory.
         */
        if (S_ISDIR(st.st_mode)) {
            return DDIR;
        }
        errno = ENOTDIR;
        return DILLEGAL;
    } else if (errno != ENOENT) {
        /* Something went wrong, or something in the path
         * is bad.  Can't do anything in this situation.
         */
        return DILLEGAL;
    }
    return DMISSING;
}

int
dirCreateHierarchy(const char *path, int mode,
        const struct utimbuf *timestamp, bool stripFileName)
{
    DirStatus ds;

    /* Check for an empty string before we bother
     * making any syscalls.
     */
    if (path[0] == '\0') {
        errno = ENOENT;
        return -1;
    }

    /* Allocate a path that we can modify; stick a slash on
     * the end to make things easier.
     */
    size_t pathLen = strlen(path);
    char *cpath = (char *)malloc(pathLen + 2);
    if (cpath == NULL) {
        errno = ENOMEM;
        return -1;
    }
    memcpy(cpath, path, pathLen);
    if (stripFileName) {
        /* Strip everything after the last slash.
         */
        char *c = cpath + pathLen - 1;
        while (c != cpath && *c != '/') {
            c--;
        }
        if (c == cpath) {
//xxx test this path
            /* No directory component.  Act like the path was empty.
             */
            errno = ENOENT;
            free(cpath);
            return -1;
        }
        c[1] = '\0';    // Terminate after the slash we found.
    } else {
        /* Make sure that the path ends in a slash.
         */
        cpath[pathLen] = '/';
        cpath[pathLen + 1] = '\0';
    }

    /* See if it already exists.
     */
    ds = getPathDirStatus(cpath);
    if (ds == DDIR) {
        return 0;
    } else if (ds == DILLEGAL) {
        return -1;
    }

    /* Walk up the path from the root and make each level.
     * If a directory already exists, no big deal.
     */
    char *p = cpath;
    while (*p != '\0') {
        /* Skip any slashes, watching out for the end of the string.
         */
        while (*p != '\0' && *p == '/') {
            p++;
        }
        if (*p == '\0') {
            break;
        }

        /* Find the end of the next path component.
         * We know that we'll see a slash before the NUL,
         * because we added it, above.
         */
        while (*p != '/') {
            p++;
        }
        *p = '\0';

        /* Check this part of the path and make a new directory
         * if necessary.
         */
        ds = getPathDirStatus(cpath);
        if (ds == DILLEGAL) {
            /* Could happen if some other process/thread is
             * messing with the filesystem.
             */
            free(cpath);
            return -1;
        } else if (ds == DMISSING) {
            int err;

            err = mkdir(cpath, mode);
            if (err != 0) {
                free(cpath);
                return -1;
            }
            if (timestamp != NULL && utime(cpath, timestamp)) {
                free(cpath);
                return -1;
            }
        }
        // else, this directory already exists.
        
        /* Repair the path and continue.
         */
        *p = '/';
    }
    free(cpath);

    return 0;
}

int
dirUnlinkHierarchy(const char *path)
{
    struct stat st;
    DIR *dir;
    struct dirent *de;
    int fail = 0;

    /* is it a file or directory? */
    if (lstat(path, &st) < 0) {
        return -1;
    }

    /* a file, so unlink it */
    if (!S_ISDIR(st.st_mode)) {
        return unlink(path);
    }

    /* a directory, so open handle */
    dir = opendir(path);
    if (dir == NULL) {
        return -1;
    }

    /* recurse over components */
    errno = 0;
    while ((de = readdir(dir)) != NULL) {
//TODO: don't blow the stack
        char dn[PATH_MAX];
        if (!strcmp(de->d_name, "..") || !strcmp(de->d_name, ".")) {
            continue;
        }
        snprintf(dn, sizeof(dn), "%s/%s", path, de->d_name);
        if (dirUnlinkHierarchy(dn) < 0) {
            fail = 1;
            break;
        }
        errno = 0;
    }
    /* in case readdir or unlink_recursive failed */
    if (fail || errno < 0) {
        int save = errno;
        closedir(dir);
        errno = save;
        return -1;
    }

    /* close directory handle */
    if (closedir(dir) < 0) {
        return -1;
    }

    /* delete target directory */
    return rmdir(path);
}

int
dirSetHierarchyPermissions(const char *path,
        int uid, int gid, int dirMode, int fileMode)
{
    struct stat st;
    if (lstat(path, &st)) {
        return -1;
    }

    /* ignore symlinks */
    if (S_ISLNK(st.st_mode)) {
        return 0;
    }

    /* directories and files get different permissions */
    if (chown(path, uid, gid) ||
        chmod(path, S_ISDIR(st.st_mode) ? dirMode : fileMode)) {
        return -1;
    }

    /* recurse over directory components */
    if (S_ISDIR(st.st_mode)) {
        DIR *dir = opendir(path);
        if (dir == NULL) {
            return -1;
        }

        errno = 0;
        const struct dirent *de;
        while (errno == 0 && (de = readdir(dir)) != NULL) {
            if (!strcmp(de->d_name, "..") || !strcmp(de->d_name, ".")) {
                continue;
            }

            char dn[PATH_MAX];
            snprintf(dn, sizeof(dn), "%s/%s", path, de->d_name);
            if (!dirSetHierarchyPermissions(dn, uid, gid, dirMode, fileMode)) {
                errno = 0;
            } else if (errno == 0) {
                errno = -1;
            }
        }

        if (errno != 0) {
            int save = errno;
            closedir(dir);
            errno = save;
            return -1;
        }

        if (closedir(dir)) {
            return -1;
        }
    }

    return 0;
}
