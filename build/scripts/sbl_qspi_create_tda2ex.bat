
set TICOMPONENTSBASE=%CD%\..\..\..\

set STARTERWAREDIRNAME=%TICOMPONENTSBASE%\ti_components\drivers\starterware_01_03_00_09

set Gcc_Tools_Path=%TICOMPONENTSBASE%\ti_components\cg_tools\windows\gcc-arm-none-eabi-4_7-2013q3\bin

REM <blank> or _opp_od or _opp_high 
set OPPMODE=

set Sbl_Tools_Path=%STARTERWAREDIRNAME%\bootloader\Tools\tiimage
set Sbl_Elf_Path=%STARTERWAREDIRNAME%\binary\sbl_qspi%OPPMODE%\bin\tda2ex
set Sbl_Out_Path=%CD%\qspi_tda2ex

IF NOT EXIST %Sbl_Out_Path%\ mkdir %Sbl_Out_Path%

del %Sbl_Out_Path%\sbl_qspi

"%Gcc_Tools_Path%\arm-none-eabi-objcopy.exe" --gap-fill=0xff -O binary %Sbl_Elf_Path%\sbl_qspi%OPPMODE%_a15host_release.xa15fg %Sbl_Out_Path%\SBL.bin

"%Sbl_Tools_Path%\tiimage.exe" 0x40300000 BE %Sbl_Out_Path%\SBL.bin %Sbl_Out_Path%\SBL.tiimage

ren %Sbl_Out_Path%\SBL.tiimage sbl_qspi

del %Sbl_Out_Path%\SBL.bin

copy %STARTERWAREDIRNAME%\binary\qspiFlashWriter\bin\tda2xx\qspiFlashWriter_m4_release.xem4 %Sbl_Out_Path%\qspiFlashWriter_m4_release.xem4

pause