#!/usr/bin/python2.4
# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
#
# MediaTek Inc. (C) 2010. All rights reserved.
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



"""A simple script to convert asset images to BMP files, that supports
RGBA image."""

import struct
import Image
import sys

infile = sys.argv[1]
outfile = sys.argv[2]

if not outfile.endswith(".bmp"):
  print >> sys.stderr, "Warning: I'm expecting to write BMP files."

im = Image.open(infile)
if im.mode == 'RGB':
  im.save(outfile)
elif im.mode == 'RGBA':
  # Python Imaging Library doesn't write RGBA BMP files, so we roll
  # our own.

  BMP_HEADER_FMT = ("<"      # little-endian
                    "H"      # signature
                    "L"      # file size
                    "HH"     # reserved (set to 0)
                    "L"      # offset to start of bitmap data)
                    )

  BITMAPINFO_HEADER_FMT= ("<"      # little-endian
                          "L"      # size of this struct
                          "L"      # width
                          "L"      # height
                          "H"      # planes (set to 1)
                          "H"      # bit count
                          "L"      # compression (set to 0 for minui)
                          "L"      # size of image data (0 if uncompressed)
                          "L"      # x pixels per meter (1)
                          "L"      # y pixels per meter (1)
                          "L"      # colors used (0)
                          "L"      # important colors (0)
                          )

  fileheadersize = struct.calcsize(BMP_HEADER_FMT)
  infoheadersize = struct.calcsize(BITMAPINFO_HEADER_FMT)

  header = struct.pack(BMP_HEADER_FMT,
                       0x4d42,   # "BM" in little-endian
                       (fileheadersize + infoheadersize +
                        im.size[0] * im.size[1] * 4),
                       0, 0,
                       fileheadersize + infoheadersize)

  info = struct.pack(BITMAPINFO_HEADER_FMT,
                     infoheadersize,
                     im.size[0],
                     im.size[1],
                     1,
                     32,
                     0,
                     0,
                     1,
                     1,
                     0,
                     0)

  f = open(outfile, "wb")
  f.write(header)
  f.write(info)
  data = im.tostring()
  for j in range(im.size[1]-1, -1, -1):   # rows bottom-to-top
    for i in range(j*im.size[0]*4, (j+1)*im.size[0]*4, 4):
      f.write(data[i+2])    # B
      f.write(data[i+1])    # G
      f.write(data[i+0])    # R
      f.write(data[i+3])    # A
  f.close()
else:
  print >> sys.stderr, "Don't know how to handle image mode '%s'." % (im.mode,)
