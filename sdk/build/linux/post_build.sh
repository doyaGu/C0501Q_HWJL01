#!/bin/bash

export PROJECT_NAME=$CFG_PROJECT

if [ "${MAKEJOBS}" = "" ]; then
    export MAKEJOBS=1
fi

function buildProject {
    export CFG_PROJECT=$1

    if [ ! -d $CFG_PROJECT ]; then
        mkdir $CFG_PROJECT
    fi

    pushd $CFG_PROJECT
    if [ -f CMakeCache.txt ]; then
        cmake -G"Unix Makefiles" $CMAKE_SOURCE_DIR
    else
        cmake -G"Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=$CFG_TOOLCHAIN_FILE $CMAKE_SOURCE_DIR
    fi

    if [ $? != 0 ]; then
        echo "execute cmake error."
        exit
    fi

    if [ "${MAKECLEAN}" = "1" ]; then
        echo "Clean build..."
        make clean
    fi

    make -j $MAKEJOBS VERBOSE=$VERBOSE
    popd
}

if [ -f $CFG_PROJECT/project/$TARGET/$TARGET ]; then
    rm $CFG_PROJECT/project/$TARGET/$TARGET
fi

if [ "$CODEC" = "1" ]; then
    buildProject "risc1_code"
    buildProject "codec"
fi

if [ "$CODEC_EX" = "1" ]; then
    buildProject "codec_ex"
fi

if [ "$CODEC_IT9910" = "1" ]; then
    buildProject "codec_it9910"
fi

if [ "$CODEC_EX_IT9910" = "1" ]; then
    buildProject "codec_ex_it9910"
fi

if [ "$CODEC_IT9850" = "1" ]; then
    buildProject "codec_it9850"
fi

if [ "$CODEC_EX_IT9850" = "1" ]; then
    buildProject "codec_ex_it9850"
fi

if [ "$RISC_TEST" = "1" ]; then
    buildProject "risc1_code"
    buildProject "risc2_code"
    buildProject "risc1_code_it9850"
    buildProject "risc2_code_it9850"
fi

buildProject "$PROJECT_NAME"

