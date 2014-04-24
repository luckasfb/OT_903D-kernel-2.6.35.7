#!/bin/bash
# ##########################################################
# ALPS(Android2.3 based) build environment profile setting
# ##########################################################
# Overwrite JAVA_HOME environment variable setting if already exists
JAVA_HOME=/usr/local/jdk1.6.0_29
export JAVA_HOME

# Overwrite ANDROID_JAVA_HOME environment variable setting if already exists
ANDROID_JAVA_HOME=/usr/local/jdk1.6.0_29
export ANDROID_JAVA_HOME

# LuckAs
JRE_HOME=/usr/local/jre1.6.0_29
export JRE_HOME

CLASSPATH=$JAVA_HOME/lib:$JRE_HOME/lib:$CLASSPATH
export CLASSPATH

# Overwrite PATH environment setting for JDK & arm-eabi if already exists
PATH=$JAVA_HOME/bin:$JRE_HOME/bin:./prebuilt/linux-x86/toolchain/arm-eabi-4.4.3/bin:$PATH
#PATH=$JAVA_HOME/bin:$JRE_HOME/bin:/usr/local/arm-eabi-4.4.3/bin:$PATH
#PATH=/usr/java/jdk1.6.0_29/bin:/usr/local/arm-eabi-4.4.3/bin:$PATH
#PATH=/opt/jdk1.6.0_29/bin:./prebuilt/linux-x86/toolchain/arm-eabi-4.4.3/bin:$PATH
export PATH

# Add MediaTek developed Python libraries path into PYTHONPATH
if [ -z "$PYTHONPATH" ]; then
  PYTHONPATH=$PWD/mediatek/build/tools
else
  PYTHONPATH=$PWD/mediatek/build/tools:$PYTHONPATH
fi
export PYTHONPATH
