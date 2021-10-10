#!/bin/bash

#export AUTOBUILD=1

# !!! Not support win32 build yet !!!
#pushd ../win32
#export NO_PAUSE=1
#source ./sdk_doorbell_win32.sh
#export NO_PAUSE=
#export COMMON_DEFINED=
#export TARGET=
#popd

export MAKEJOBS=4
export CFG_PROJECT=risc1_code
source ./build.sh

export MAKEJOBS=4
export CFG_PROJECT=codec
source ./build.sh

export MAKEJOBS=4
export CFG_PROJECT=codec_ex
source ./build.sh

export MAKEJOBS=4
export CFG_PROJECT=codec_it9910
source ./build.sh

export MAKEJOBS=4
export CFG_PROJECT=codec_ex_it9910
source ./build.sh

export MAKEJOBS=4
export CFG_PROJECT=codec_it9850
source ./build.sh

export MAKEJOBS=4
export CFG_PROJECT=codec_ex_it9850
source ./build.sh

export MAKEJOBS=4
export CFG_PROJECT=sdk_doorbell_sm32
source ./build.sh

export CFG_PROJECT=`basename $0 .sh`
export COMMON_DEFINED=
source ./build.sh

