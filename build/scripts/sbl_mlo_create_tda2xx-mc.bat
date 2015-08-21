
set TICOMPONENTSBASE=%CD%\..\..\..\

set STARTERWAREDIRNAME=%TICOMPONENTSBASE%\ti_components\drivers\starterware_01_03_00_09

set Gcc_Tools_Path=%TICOMPONENTSBASE%\ti_components\cg_tools\windows\gcc-arm-none-eabi-4_7-2013q3\bin

REM <blank> or _opp_od or _opp_high 
set OPPMODE=

set Sbl_Tools_Path=%STARTERWAREDIRNAME%\bootloader\Tools\tiimage
set Sbl_Elf_Path=%STARTERWAREDIRNAME%\binary\sbl_sd%OPPMODE%\bin\tda2xx-mc
set Sbl_Out_Path=%CD%\mlo_tda2xx-mc

IF NOT EXIST %Sbl_Out_Path%\ mkdir %Sbl_Out_Path%

del %Sbl_Out_Path%\MLO

"%Gcc_Tools_Path%\arm-none-eabi-objcopy.exe" --gap-fill=0xff -O binary %Sbl_Elf_Path%\sbl_sd%OPPMODE%_a15host_release.xa15fg %Sbl_Out_Path%\SBL.bin

"%Sbl_Tools_Path%\tiimage.exe" 0x40300000 LE %Sbl_Out_Path%\SBL.bin %Sbl_Out_Path%\SBL.tiimage

ren %Sbl_Out_Path%\SBL.tiimage MLO

del %Sbl_Out_Path%\SBL.bin

pause
