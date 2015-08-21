export TICOMPONENTSBASE=$PWD/../../../

export STARTERWAREDIRNAME=$PWD/../../../ti_components/drivers/starterware_01_03_00_09

export Gcc_Tools_Path=$TICOMPONENTSBASE/ti_components/cg_tools/linux/gcc-arm-none-eabi-4_7-2013q3/bin

#REM <blank> or _opp_od or _opp_high 
export OPPMODE=

export Sbl_Tools_Path=$STARTERWAREDIRNAME/bootloader/Tools/tiimage
export Sbl_Elf_Path=$STARTERWAREDIRNAME/binary/sbl_sd$OPPMODE/bin/tda2ex
export Sbl_Out_Path=$PWD/mlo_tda2ex

if [ -d $Sbl_Out_Path ]
then
    echo "$Sbl_Out_Path exists"
else
    mkdir $Sbl_Out_Path
fi

rm -rf $Sbl_Out_Path/MLO

"$Gcc_Tools_Path/arm-none-eabi-objcopy" --gap-fill=0xff -O binary $Sbl_Elf_Path/sbl_sd$OPPMODE\_a15host_release.xa15fg $Sbl_Out_Path/SBL.bin

"$Sbl_Tools_Path/tiimage" 0x40300000 LE $Sbl_Out_Path/SBL.bin $Sbl_Out_Path/SBL.tiimage

mv $Sbl_Out_Path/SBL.tiimage $Sbl_Out_Path/MLO

rm -f $Sbl_Out_Path/SBL.bin


