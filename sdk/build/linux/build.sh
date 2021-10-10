#!/bin/bash

#export VERBOSE=1

source ./common.sh

if [ ! -d $CFG_PROJECT ]; then
    mkdir $CFG_PROJECT
fi

if [ "$BOOTLOADER" = "" ]; then
    export BOOTLOADER=0
fi

if [ "$CODEC" = "" ]; then
    export CODEC=0
fi

if [ "$CODEC_EX" = "" ]; then
    export CODEC_EX=0
fi

if [ "$CODEC_IT9910" = "" ]; then
    export CODEC_IT9910=0
fi

if [ "$CODEC_EX_IT9910" = "" ]; then
    export CODEC_EX_IT9910=0
fi


if [ "$TARGET" = "" ]; then
    export TARGET=$CFG_PROJECT
fi

if [ "$AUTOBUILD" = "" ]; then
    export AUTOBUILD=0
fi

export CFG_DEVELOP=0
export BUILD_CMD=./post_build.sh

if [ $BOOTLOADER = 1 ]; then
    export PRESETTING="--loadcfg $CMAKE_SOURCE_DIR/build/_presettings/_config_bootloader"
else
    export PRESETTING=""
fi

if [ -f $CMAKE_SOURCE_DIR/project/$CFG_PROJECT/config.cmake ]; then
    source ./post_build.sh
elif [ $AUTOBUILD = 1 ]; then
    mconf --autowrite --prefix "CFG_" --cmakefile $CFG_PROJECT/config.cmake --cfgfile $CFG_PROJECT/.config $PRESETTING $CMAKE_SOURCE_DIR/project/$CFG_PROJECT/Kconfig
    source ./post_build.sh
else
    qconf --fontsize 11 --prefix "CFG_" --cmakefile $CFG_PROJECT/config.cmake --cfgfile $CFG_PROJECT/.config $PRESETTING $CMAKE_SOURCE_DIR/project/$CFG_PROJECT/Kconfig
fi

