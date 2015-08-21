#!/bin/bash

# Device Id for TDA3xx - 77
# Device ID & CPU ID should be in sync with SBL. Refer to SBL user guide for values
export Dev_ID=77

export CORE_ID_IPU1_CPU0=2
export CORE_ID_IPU1_CPU1=3
export CORE_ID_DSP1=6
export CORE_ID_DSP2=7
export CORE_ID_EVE1=8
export CORE_ID_IPU1=14

#
# Creates two app images one for UCEarly containing only ipu and one for other cores.
#
export FAST_BOOT=yes

# Define Output file path
# Define Output file path
export Out_Path=$PWD/binaries/vision_sdk/bin/tda3xx-evm/sbl_boot
export Tools_path=$PWD/../ti_components/drivers/starterware_01_03_00_09/bootloader/Tools

# Use profile passed from command line, else use default as release
export profile=$1
if [ -z $profile ]
then
    export profile=release
fi


if [ -d $Out_Path ]
then
    echo "$Out_Path exists"
else
    mkdir $Out_Path
fi


# Define Input file paths; To skip the core leave it blank
export App_IPU1_CPU0=$PWD/binaries/vision_sdk/bin/tda3xx-evm/vision_sdk_ipu1_0_$profile.xem4
export App_IPU1_CPU1=$PWD/binaries/vision_sdk/bin/tda3xx-evm/vision_sdk_ipu1_1_$profile.xem4
export App_DSP1=$PWD/binaries/vision_sdk/bin/tda3xx-evm/vision_sdk_c66xdsp_1_$profile.xe66
export App_DSP2=$PWD/binaries/vision_sdk/bin/tda3xx-evm/vision_sdk_c66xdsp_2_$profile.xe66
export App_EVE1=$PWD/binaries/vision_sdk/bin/tda3xx-evm/vision_sdk_arp32_1_$profile.xearp32F

echo $App_IPU1_CPU0
echo $App_IPU1_CPU1
echo $App_DSP1
echo $App_DSP2
echo $App_EVE1
# App_IPU1 is to define the IPU SMP application
export App_IPU1=

if [ ! -z $App_IPU1_CPU0 ]
then
    export IPU1_CPU0=$CORE_ID_IPU1_CPU0
    export image_gen=1
    export App_IPU1_CPU0_RPRC=$App_IPU1_CPU0.rprc
fi

if [ ! -z $App_IPU1_CPU0 ]
then
    echo mono $Tools_path/out2rprc/out2rprc.exe $App_IPU1_CPU0 $App_IPU1_CPU0_RPRC
    mono $Tools_path/out2rprc/out2rprc.exe $App_IPU1_CPU0 $App_IPU1_CPU0_RPRC
fi


if [ ! -z $App_IPU1_CPU1 ]
then
    export IPU1_CPU1=$CORE_ID_IPU1_CPU1
    export image_gen=1
    export App_IPU1_CPU1_RPRC=$App_IPU1_CPU1.rprc
fi

if [ ! -z $App_IPU1_CPU1 ]
then
    echo mono $Tools_path/out2rprc/out2rprc.exe $App_IPU1_CPU1 $App_IPU1_CPU1_RPRC
    mono $Tools_path/out2rprc/out2rprc.exe $App_IPU1_CPU1 $App_IPU1_CPU1_RPRC
fi


if [ ! -z $App_DSP1 ]
then
    export DSP1_CPU=$CORE_ID_DSP1
    export image_gen=1
    export App_DSP1_RPRC=$App_DSP1.rprc
fi

if [ ! -z $App_DSP1 ]
then
    echo mono $Tools_path/out2rprc/out2rprc.exe $App_DSP1 $App_DSP1_RPRC
    mono $Tools_path/out2rprc/out2rprc.exe $App_DSP1 $App_DSP1_RPRC
fi

if [ ! -z $App_DSP2 ]
then
    export DSP2_CPU=$CORE_ID_DSP2
    export image_gen=1
    export App_DSP2_RPRC=$App_DSP2.rprc
fi

if [ ! -z $App_DSP2 ]
then
    echo mono $Tools_path/out2rprc/out2rprc.exe $App_DSP2 $App_DSP2_RPRC
    mono $Tools_path/out2rprc/out2rprc.exe $App_DSP2 $App_DSP2_RPRC
fi


if [ ! -z $App_EVE1 ]
then
    export EVE1_CPU=$CORE_ID_EVE1
    export image_gen=1
    export App_EVE1_RPRC=$App_EVE1.rprc
fi

if [ ! -z $App_EVE1 ]
then
    echo mono $Tools_path/out2rprc/out2rprc.exe $App_EVE1 $App_EVE1_RPRC
    mono $Tools_path/out2rprc/out2rprc.exe $App_EVE1 $App_EVE1_RPRC
fi


if [ ! -z $App_IPU1 ]
then
    export IPU1_CPU_SMP=$CORE_ID_IPU1
    export image_gen=1
    export App_IPU1_RPRC=$App_IPU1.rprc
fi

if [ ! -z $App_IPU1 ]
then
    echo mono $Tools_path/out2rprc/out2rprc.exe $App_IPU1 $App_IPU1_RPRC
    mono $Tools_path/out2rprc/out2rprc.exe $App_IPU1 $App_IPU1_RPRC
fi


if [ ! -z $image_gen ]
then
    if [ $FAST_BOOT == "yes" ]
    then
         # Generate MulticoreImage Gen for UCearly (IPU1_0 only)
        "$Tools_path/multicore_image_generator/MulticoreImageGen" BE $Dev_ID $Out_Path/AppImage_UcEarly_BE $IPU1_CPU0 $App_IPU1_CPU0_RPRC $IPU1_CPU1 $App_IPU1_CPU1_RPRC

        # Generate MulticoreImage Gen for UClate (core other than IPU1_0)
        "$Tools_path/multicore_image_generator/MulticoreImageGen" BE $Dev_ID $Out_Path/AppImage_UcLate_BE  $DSP1_CPU $App_DSP1_RPRC $DSP2_CPU $App_DSP2_RPRC $EVE1_CPU $App_EVE1_RPRC $IPU1_CPU_SMP $App_IPU1_RPRC
    else
        # Generate MulticoreImage Gen
        "$Tools_path/multicore_image_generator/MulticoreImageGen" LE $Dev_ID $Out_Path/AppImage_LE $IPU1_CPU0 $App_IPU1_CPU0_RPRC $IPU1_CPU1 $App_IPU1_CPU1_RPRC $DSP1_CPU $App_DSP1_RPRC $DSP2_CPU $App_DSP2_RPRC $EVE1_CPU $App_EVE1_RPRC $IPU1_CPU_SMP $App_IPU1_RPRC
        "$Tools_path/multicore_image_generator/MulticoreImageGen" BE $Dev_ID $Out_Path/AppImage_BE $IPU1_CPU0 $App_IPU1_CPU0_RPRC $IPU1_CPU1 $App_IPU1_CPU1_RPRC $DSP1_CPU $App_DSP1_RPRC $DSP2_CPU $App_DSP2_RPRC $EVE1_CPU $App_EVE1_RPRC $IPU1_CPU_SMP $App_IPU1_RPRC
    fi
fi


if [ $FAST_BOOT == "no" ]
then
cp $Out_Path/AppImage_LE $Out_Path/AppImage
fi

# copy $Out_Path\AppImage F:\

