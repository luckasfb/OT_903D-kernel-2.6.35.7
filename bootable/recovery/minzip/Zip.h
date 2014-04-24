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
 * Copyright 2006 The Android Open Source Project
 *
 * Simple Zip archive support.
 */
#ifndef _MINZIP_ZIP
#define _MINZIP_ZIP

#include "inline_magic.h"

#include <stdlib.h>
#include <utime.h>

#include "Hash.h"
#include "SysUtil.h"

/*
 * One entry in the Zip archive.  Treat this as opaque -- use accessors below.
 *
 * TODO: we're now keeping the pages mapped so we don't have to copy the
 * filename.  We can change the accessors to retrieve the various pieces
 * directly from the source file instead of copying them out, for a very
 * slight speed hit and a modest reduction in memory usage.
 */
typedef struct ZipEntry {
    unsigned int fileNameLen;
    const char*  fileName;       // not null-terminated
    long         offset;
    long         compLen;
    long         uncompLen;
    int          compression;
    long         modTime;
    long         crc32;
    int          versionMadeBy;
    long         externalFileAttributes;
} ZipEntry;

/*
 * One Zip archive.  Treat as opaque.
 */
typedef struct ZipArchive {
    int         fd;
    unsigned int numEntries;
    ZipEntry*   pEntries;
    HashTable*  pHash;          // maps file name to ZipEntry
    MemMapping  map;
} ZipArchive;

/*
 * Represents a non-NUL-terminated string,
 * which is how entry names are stored.
 */
typedef struct {
    const char *str;
    size_t len;
} UnterminatedString;

/*
 * Open a Zip archive.
 *
 * On success, returns 0 and populates "pArchive".  Returns nonzero errno
 * value on failure.
 */
int mzOpenZipArchive(const char* fileName, ZipArchive* pArchive);

/*
 * Close archive, releasing resources associated with it.
 *
 * Depending on the implementation this could unmap pages used by classes
 * stored in a Jar.  This should only be done after unloading classes.
 */
void mzCloseZipArchive(ZipArchive* pArchive);


/*
 * Find an entry in the Zip archive, by name.
 */
const ZipEntry* mzFindZipEntry(const ZipArchive* pArchive,
        const char* entryName);

/*
 * Get the number of entries in the Zip archive.
 */
INLINE unsigned int mzZipEntryCount(const ZipArchive* pArchive) {
    return pArchive->numEntries;
}

/*
 * Get an entry by index.  Returns NULL if the index is out-of-bounds.
 */
INLINE const ZipEntry*
mzGetZipEntryAt(const ZipArchive* pArchive, unsigned int index)
{
    if (index < pArchive->numEntries) {
        return pArchive->pEntries + index;
    }
    return NULL;
}

/*
 * Get the index number of an entry in the archive.
 */
INLINE unsigned int
mzGetZipEntryIndex(const ZipArchive *pArchive, const ZipEntry *pEntry) {
    return pEntry - pArchive->pEntries;
}

/*
 * Simple accessors.
 */
INLINE UnterminatedString mzGetZipEntryFileName(const ZipEntry* pEntry) {
    UnterminatedString ret;
    ret.str = pEntry->fileName;
    ret.len = pEntry->fileNameLen;
    return ret;
}
INLINE long mzGetZipEntryOffset(const ZipEntry* pEntry) {
    return pEntry->offset;
}
INLINE long mzGetZipEntryUncompLen(const ZipEntry* pEntry) {
    return pEntry->uncompLen;
}
INLINE long mzGetZipEntryModTime(const ZipEntry* pEntry) {
    return pEntry->modTime;
}
INLINE long mzGetZipEntryCrc32(const ZipEntry* pEntry) {
    return pEntry->crc32;
}
bool mzIsZipEntrySymlink(const ZipEntry* pEntry);


/*
 * Type definition for the callback function used by
 * mzProcessZipEntryContents().
 */
typedef bool (*ProcessZipEntryContentsFunction)(const unsigned char *data,
    int dataLen, void *cookie);

/*
 * Stream the uncompressed data through the supplied function,
 * passing cookie to it each time it gets called.  processFunction
 * may be called more than once.
 *
 * If processFunction returns false, the operation is abandoned and
 * mzProcessZipEntryContents() immediately returns false.
 *
 * This is useful for calculating the hash of an entry's uncompressed contents.
 */
bool mzProcessZipEntryContents(const ZipArchive *pArchive,
    const ZipEntry *pEntry, ProcessZipEntryContentsFunction processFunction,
    void *cookie);

/*
 * Read an entry into a buffer allocated by the caller.
 */
bool mzReadZipEntry(const ZipArchive* pArchive, const ZipEntry* pEntry,
        char* buf, int bufLen);

/*
 * Check the CRC on this entry; return true if it is correct.
 * May do other internal checks as well.
 */
bool mzIsZipEntryIntact(const ZipArchive *pArchive, const ZipEntry *pEntry);

/*
 * Inflate and write an entry to a file.
 */
bool mzExtractZipEntryToFile(const ZipArchive *pArchive,
    const ZipEntry *pEntry, int fd);

/*
 * Inflate and write an entry to a memory buffer, which must be long
 * enough to hold mzGetZipEntryUncomplen(pEntry) bytes.
 */
bool mzExtractZipEntryToBuffer(const ZipArchive *pArchive,
    const ZipEntry *pEntry, unsigned char* buffer);

/*
 * Inflate all entries under zipDir to the directory specified by
 * targetDir, which must exist and be a writable directory.
 *
 * The immediate children of zipDir will become the immediate
 * children of targetDir; e.g., if the archive contains the entries
 *
 *     a/b/c/one
 *     a/b/c/two
 *     a/b/c/d/three
 *
 * and mzExtractRecursive(a, "a/b/c", "/tmp", ...) is called, the resulting
 * files will be
 *
 *     /tmp/one
 *     /tmp/two
 *     /tmp/d/three
 *
 * flags is zero or more of the following:
 *
 *     MZ_EXTRACT_FILES_ONLY - only unpack files, not directories or symlinks
 *     MZ_EXTRACT_DRY_RUN - don't do anything, but do invoke the callback
 *
 * If timestamp is non-NULL, file timestamps will be set accordingly.
 *
 * If callback is non-NULL, it will be invoked with each unpacked file.
 *
 * Returns true on success, false on failure.
 */
enum { MZ_EXTRACT_FILES_ONLY = 1, MZ_EXTRACT_DRY_RUN = 2 };
bool mzExtractRecursive(const ZipArchive *pArchive,
        const char *zipDir, const char *targetDir,
        int flags, const struct utimbuf *timestamp,
        void (*callback)(const char *fn, void*), void *cookie);

#endif /*_MINZIP_ZIP*/
