#!/bin/bash

# Device Id for Vayu - 55
# Device ID & CPU ID should be in sync with SBL. Refer to SBL user guide for values
export Dev_ID=55
export MPU_CPU0_ID=0
export MPU_CPU1_ID=1
export IPU1_CPU0_ID=2
export IPU1_CPU1_ID=3
export IPU1_CPU_SMP_ID=4
export IPU2_CPU0_ID=5
export IPU2_CPU1_ID=6
export IPU2_CPU_SMP_ID=7
export DSP1_ID=8
export DSP2_ID=9
export EVE1_ID=10
export EVE2_ID=11
export EVE3_ID=12
export EVE4_ID=13
export MPU_SMP_ID=14

export Out_Path=$PWD/binaries/vision_sdk/bin/tda2xx-evm/sbl_boot
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
export App_MPU_CPU0=$PWD/binaries/vision_sdk/bin/tda2xx-evm/vision_sdk_a15_0_debug.xa15fg
export App_MPU_CPU1=
# App_MPU_SMP is to define MPU SMP application, To enable SMP mode - remove "App_MPU_CPU0" & set "APP_MPU_SMP"
# export APP_MPU_SMP=$PWD/binaries/vision_sdk/bin/tda2xx-evm/vision_sdk_a15_0_debug.xa15fg
export App_IPU1_CPU0=$PWD/binaries/vision_sdk/bin/tda2xx-evm/vision_sdk_ipu1_0_debug.xem4
export App_IPU1_CPU1=$PWD/binaries/vision_sdk/bin/tda2xx-evm/vision_sdk_ipu1_1_debug.xem4
# App_IPU1_CPU_SMP is to define the IPU SMP application
export App_IPU1_CPU_SMP=
export App_IPU2_CPU0=
export App_IPU2_CPU1=
# App_IPU2_CPU_SMP is to define the IPU SMP application
export App_IPU2_CPU_SMP=
export App_DSP1=$PWD/binaries/vision_sdk/bin/tda2xx-evm/vision_sdk_c66xdsp_1_$profile.xe66
export App_DSP2=$PWD/binaries/vision_sdk/bin/tda2xx-evm/vision_sdk_c66xdsp_2_$profile.xe66
export App_EVE1=$PWD/binaries/vision_sdk/bin/tda2xx-evm/vision_sdk_arp32_1_$profile.xearp32F
export App_EVE2=$PWD/binaries/vision_sdk/bin/tda2xx-evm/vision_sdk_arp32_2_$profile.xearp32F
export App_EVE3=$PWD/binaries/vision_sdk/bin/tda2xx-evm/vision_sdk_arp32_3_$profile.xearp32F
export App_EVE4=$PWD/binaries/vision_sdk/bin/tda2xx-evm/vision_sdk_arp32_4_$profile.xearp32F

if [ ! -z $App_MPU_CPU0  ]
then
    export MPU_CPU0=$MPU_CPU0_ID
    export image_gen=1
    export App_MPU_CPU0_RPRC=$App_MPU_CPU0.rprc
fi
if [ ! -z $App_MPU_CPU0  ]
then
    mono "$Tools_path/out2rprc/out2rprc.exe" $App_MPU_CPU0 $App_MPU_CPU0_RPRC
fi

if [ ! -z $App_MPU_CPU1  ]
then
    export MPU_CPU1=$MPU_CPU1_ID
    export image_gen=1
    export App_MPU_CPU1_RPRC=$App_MPU_CPU1.rprc
fi
if [ ! -z $App_MPU_CPU1  ]
then
    mono "$Tools_path/out2rprc/out2rprc.exe" $App_MPU_CPU1 $App_MPU_CPU1_RPRC
fi

if [ ! -z $APP_MPU_SMP  ]
then
    export MPU_SMP=$MPU_SMP_ID
    export image_gen=1
    export App_MPU_SMP_RPRC=$App_MPU_SMP.rprc
fi
if [ ! -z $App_MPU_SMP  ]
then
    mono "$Tools_path/out2rprc/out2rprc.exe" $App_MPU_SMP $App_MPU_SMP_RPRC
fi

if [ ! -z $App_IPU1_CPU0  ]
then
    export IPU1_CPU0=$IPU1_CPU0_ID
    export image_gen=1
    export App_IPU1_CPU0_RPRC=$App_IPU1_CPU0.rprc
fi
if [ ! -z $App_IPU1_CPU0  ]
then
    mono "$Tools_path/out2rprc/out2rprc.exe" $App_IPU1_CPU0 $App_IPU1_CPU0_RPRC
fi

if [ ! -z $App_IPU1_CPU1  ]
then
    export IPU1_CPU1=$IPU1_CPU1_ID
    export image_gen=1
    export App_IPU1_CPU1_RPRC=$App_IPU1_CPU1.rprc
fi
if [ ! -z $App_IPU1_CPU1  ]
then
    mono "$Tools_path/out2rprc/out2rprc.exe" $App_IPU1_CPU1 $App_IPU1_CPU1_RPRC
fi

if [ ! -z $App_IPU1_CPU_SMP  ]
then
    export IPU1_CPU_SMP=$IPU1_CPU_SMP_ID
    export image_gen=1
    export App_IPU1_CPU_SMP_RPRC=$App_IPU1_CPU_SMP.rprc
fi
if [ ! -z $App_IPU1_CPU_SMP  ]
then
    mono "$Tools_path/out2rprc/out2rprc.exe" $App_IPU1_CPU_SMP $App_IPU1_CPU_SMP_RPRC
fi

if [ ! -z $App_IPU2_CPU0  ]
then
    export IPU2_CPU0=$IPU2_CPU0_ID
    export image_gen=1
    export App_IPU2_CPU0_RPRC=$App_IPU2_CPU0.rprc
fi
if [ ! -z $App_IPU2_CPU0  ]
then
    mono "$Tools_path/out2rprc/out2rprc.exe" $App_IPU2_CPU0 $App_IPU2_CPU0_RPRC
fi

if [ ! -z $App_IPU2_CPU1  ]
then
    export IPU2_CPU1=$IPU2_CPU1_ID
    export image_gen=1
    export App_IPU2_CPU1_RPRC=$App_IPU2_CPU1.rprc
fi
if [ ! -z $App_IPU2_CPU1  ]
then
    mono "$Tools_path/out2rprc/out2rprc.exe" $App_IPU2_CPU1 $App_IPU2_CPU1_RPRC
fi

if [ ! -z $App_IPU2_CPU_SMP  ]
then
    export IPU2_CPU_SMP=$IPU2_CPU_SMP_ID
    export image_gen=1
    export App_IPU2_CPU_SMP_RPRC=$App_IPU2_CPU_SMP.rprc
fi
if [ ! -z $App_IPU2_CPU_SMP  ]
then
    mono "$Tools_path/out2rprc/out2rprc.exe" $App_IPU2_CPU_SMP $App_IPU2_CPU_SMP_RPRC
fi

if [ ! -z $App_DSP1  ]
then
    export DSP1_CPU=$DSP1_ID
    export image_gen=1
    export App_DSP1_RPRC=$App_DSP1.rprc
fi
if [ ! -z $App_DSP1  ]
then
    mono "$Tools_path/out2rprc/out2rprc.exe" $App_DSP1 $App_DSP1_RPRC
fi

if [ ! -z $App_DSP2  ]
then
    export DSP2_CPU=$DSP2_ID
    export image_gen=1
    export App_DSP2_RPRC=$App_DSP2.rprc
fi
if [ ! -z $App_DSP2  ]
then
    mono "$Tools_path/out2rprc/out2rprc.exe" $App_DSP2 $App_DSP2_RPRC
fi

if [ ! -z $App_EVE1  ]
then
    export EVE1_CPU=$EVE1_ID
    export image_gen=1
    export App_EVE1_RPRC=$App_EVE1.rprc
fi
if [ ! -z $App_EVE1  ]
then
    mono "$Tools_path/out2rprc/out2rprc.exe" $App_EVE1 $App_EVE1_RPRC
fi

if [ ! -z $App_EVE2  ]
then
    export EVE2_CPU=$EVE2_ID
    export image_gen=1
    export App_EVE2_RPRC=$App_EVE2.rprc
fi
if [ ! -z $App_EVE2  ]
then
    mono "$Tools_path/out2rprc/out2rprc.exe" $App_EVE2 $App_EVE2_RPRC
fi

if [ ! -z $App_EVE3  ]
then
    export EVE3_CPU=$EVE3_ID
    export image_gen=1
    export App_EVE3_RPRC=$App_EVE3.rprc
fi
if [ ! -z $App_EVE3  ]
then
    mono "$Tools_path/out2rprc/out2rprc.exe" $App_EVE3 $App_EVE3_RPRC
fi

if [ ! -z $App_EVE4  ]
then
    export EVE4_CPU=$EVE4_ID
    export image_gen=1
    export App_EVE4_RPRC=$App_EVE4.rprc
fi
if [ ! -z $App_EVE4  ]
then
    mono "$Tools_path/out2rprc/out2rprc.exe" $App_EVE4 $App_EVE4_RPRC
fi

# ImageGen
if [ ! -z $image_gen  ]
then
    # Gerating MulticoreImage Gen
    "$Tools_path/multicore_image_generator/MulticoreImageGen" LE $Dev_ID $Out_Path/AppImage_LE $MPU_CPU0 $App_MPU_CPU0_RPRC  $MPU_CPU1 $App_MPU_CPU1_RPRC $MPU_SMP $App_MPU_SMP_RPRC $IPU1_CPU0 $App_IPU1_CPU0_RPRC $IPU1_CPU1 $App_IPU1_CPU1_RPRC $IPU1_CPU_SMP $IPU1_CPU_SMP_RPRC $IPU2_CPU0 $App_IPU2_CPU0_RPRC $IPU2_CPU1 $App_IPU2_CPU1_RPRC $IPU2_CPU_SMP $IPU2_CPU_SMP_RPRC $DSP1_CPU $App_DSP1_RPRC $DSP2_CPU $App_DSP2_RPRC $EVE1_CPU $App_EVE1_RPRC $EVE2_CPU $App_EVE2_RPRC $EVE3_CPU $App_EVE3_RPRC $EVE4_CPU $App_EVE4_RPRC

    "$Tools_path/multicore_image_generator/MulticoreImageGen" BE $Dev_ID $Out_Path/AppImage_BE $MPU_CPU0 $App_MPU_CPU0_RPRC  $MPU_CPU1 $App_MPU_CPU1_RPRC $MPU_SMP $App_MPU_SMP_RPRC $IPU1_CPU0 $App_IPU1_CPU0_RPRC $IPU1_CPU1 $App_IPU1_CPU1_RPRC $IPU1_CPU_SMP $IPU1_CPU_SMP_RPRC $IPU2_CPU0 $App_IPU2_CPU0_RPRC $IPU2_CPU1 $App_IPU2_CPU1_RPRC $IPU2_CPU_SMP $IPU2_CPU_SMP_RPRC $DSP1_CPU $App_DSP1_RPRC $DSP2_CPU $App_DSP2_RPRC $EVE1_CPU $App_EVE1_RPRC $EVE2_CPU $App_EVE2_RPRC $EVE3_CPU $App_EVE3_RPRC $EVE4_CPU $App_EVE4_RPRC
fi

cp $Out_Path/AppImage_LE $Out_Path/AppImage

# copy $Out_Path/AppImage F:

