#!/bin/bash
. ~/.bashrc

targetPath=$1

if [ -z $targetPath ]; then
    echo "[ERROR] Please input the target path..."
    exit
fi

if [ ! -d $targetPath ]; then
    echo "[ERROR] Can not do code stripping, the target path does not exist~!"
    exit
fi

find $targetPath -name '*.java' -o -name '*.cpp' -o -name '*.c' -o -name '*.h' | xargs sed '/MTK_SWIP_PROTECT_START/,/MTK_SWIP_PROTECT_END/ d' -i

