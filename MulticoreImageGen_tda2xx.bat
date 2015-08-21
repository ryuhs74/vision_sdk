REM @echo off
REM Define Device Id Vayu - 55 - choosen random value will be updated
REM Device ID & CPU ID should be in sync with SBL. Refer SBL user guide for values

REM Set local environment for the variables defined in this file. This will
REM ensure correct behavior when this batch file is run multiple times from the
REM same DOS window.
setlocal

set Dev_ID=55
set MPU_CPU0_ID=0
set MPU_CPU1_ID=1
set	IPU1_CPU0_ID=2
set IPU1_CPU1_ID=3
set IPU1_CPU_SMP_ID=4
set	IPU2_CPU0_ID=5
set	IPU2_CPU1_ID=6
set IPU2_CPU_SMP_ID=7
set DSP1_ID=8
set	DSP2_ID=9
set	EVE1_ID=10
set	EVE2_ID=11
set	EVE3_ID=12
set	EVE4_ID=13
set MPU_SMP_ID=14

REM Define Output file path
set Out_Path=%CD%\binaries\vision_sdk\bin\tda2xx-evm\sbl_boot
set Tools_path=%CD%\..\ti_components\drivers\starterware_01_03_00_09\bootloader\Tools

REM Use profile passed from command line, else use default as release
set profile=%1
IF %1.==. (
set profile=release
)

IF NOT EXIST %Out_Path%\ mkdir %Out_Path%


REM Define Input file paths; To skip the core leave it blank
set App_MPU_CPU0=%CD%\binaries\vision_sdk\bin\tda2xx-evm\vision_sdk_a15_0_debug.xa15fg
set App_MPU_CPU1=
REM App_MPU_SMP is to define MPU SMP application, To enable SMP mode - remove "App_MPU_CPU0" & set "APP_MPU_SMP"
REM set APP_MPU_SMP=%CD%\binaries\vision_sdk\bin\tda2xx-evm\vision_sdk_a15_0_debug.xa15fg
set App_IPU1_CPU0=%CD%\binaries\vision_sdk\bin\tda2xx-evm\vision_sdk_ipu1_0_%profile%.xem4
set App_IPU1_CPU1=%CD%\binaries\vision_sdk\bin\tda2xx-evm\vision_sdk_ipu1_1_%profile%.xem4
REM App_IPU1_CPU_SMP is to define the IPU SMP application
set App_IPU1_CPU_SMP=
set App_IPU2_CPU0=
set App_IPU2_CPU1=
REM App_IPU2_CPU_SMP is to define the IPU SMP application
set App_IPU2_CPU_SMP=
set App_DSP1=%CD%\binaries\vision_sdk\bin\tda2xx-evm\vision_sdk_c66xdsp_1_%profile%.xe66
set App_DSP2=%CD%\binaries\vision_sdk\bin\tda2xx-evm\vision_sdk_c66xdsp_2_%profile%.xe66
set App_EVE1=%CD%\binaries\vision_sdk\bin\tda2xx-evm\vision_sdk_arp32_1_%profile%.xearp32F
set App_EVE2=%CD%\binaries\vision_sdk\bin\tda2xx-evm\vision_sdk_arp32_2_%profile%.xearp32F
set App_EVE3=%CD%\binaries\vision_sdk\bin\tda2xx-evm\vision_sdk_arp32_3_%profile%.xearp32F
set App_EVE4=%CD%\binaries\vision_sdk\bin\tda2xx-evm\vision_sdk_arp32_4_%profile%.xearp32F

if defined App_MPU_CPU0 (
set MPU_CPU0=%MPU_CPU0_ID%
set image_gen=1
set App_MPU_CPU0_RPRC=%App_MPU_CPU0%.rprc )
if defined App_MPU_CPU0 (
"%Tools_path%\out2rprc\out2rprc.exe" %App_MPU_CPU0% %App_MPU_CPU0_RPRC% )

if defined App_MPU_CPU1 (
set MPU_CPU1=%MPU_CPU1_ID%
set image_gen=1
set App_MPU_CPU1_RPRC=%App_MPU_CPU1%.rprc )
if defined App_MPU_CPU1 (
"%Tools_path%\out2rprc\out2rprc.exe" %App_MPU_CPU1% %App_MPU_CPU1_RPRC% )

if defined APP_MPU_SMP (
set MPU_SMP=%MPU_SMP_ID%
set image_gen=1
set App_MPU_SMP_RPRC=%APP_MPU_SMP%.rprc )
if defined APP_MPU_SMP (
"%Tools_path%\out2rprc\out2rprc.exe" %APP_MPU_SMP% %App_MPU_SMP_RPRC% )

if defined App_IPU1_CPU0 (
set IPU1_CPU0=%IPU1_CPU0_ID%
set image_gen=1
set App_IPU1_CPU0_RPRC=%App_IPU1_CPU0%.rprc )
if defined App_IPU1_CPU0 (
"%Tools_path%\out2rprc\out2rprc.exe" %App_IPU1_CPU0% %App_IPU1_CPU0_RPRC% )

if defined App_IPU1_CPU1 (
set IPU1_CPU1=%IPU1_CPU1_ID%
set image_gen=1
set App_IPU1_CPU1_RPRC=%App_IPU1_CPU1%.rprc )
if defined App_IPU1_CPU1 (
"%Tools_path%\out2rprc\out2rprc.exe" %App_IPU1_CPU1% %App_IPU1_CPU1_RPRC% )

if defined App_IPU1_CPU_SMP (
set IPU1_CPU_SMP=%IPU1_CPU_SMP_ID%
set image_gen=1
set App_IPU1_CPU_SMP_RPRC=%App_IPU1_CPU_SMP%.rprc )
if defined App_IPU1_CPU_SMP (
"%Tools_path%\out2rprc\out2rprc.exe" %App_IPU1_CPU_SMP% %App_IPU1_CPU_SMP_RPRC% )

if defined App_IPU2_CPU0 (
set IPU2_CPU0=%IPU2_CPU0_ID%
set image_gen=1
set App_IPU2_CPU0_RPRC=%App_IPU2_CPU0%.rprc )
if defined App_IPU2_CPU0 (
"%Tools_path%\out2rprc\out2rprc.exe" %App_IPU2_CPU0% %App_IPU2_CPU0_RPRC% )

if defined App_IPU2_CPU1 (
set IPU2_CPU1=%IPU2_CPU1_ID%
set image_gen=1
set App_IPU2_CPU1_RPRC=%App_IPU2_CPU1%.rprc )
if defined App_IPU2_CPU1 (
"%Tools_path%\out2rprc\out2rprc.exe" %App_IPU2_CPU1% %App_IPU2_CPU1_RPRC% )

if defined App_IPU2_CPU_SMP (
set IPU2_CPU_SMP=%IPU2_CPU_SMP_ID%
set image_gen=1
set App_IPU2_CPU_SMP_RPRC=%App_IPU2_CPU_SMP%.rprc )
if defined App_IPU2_CPU_SMP (
"%Tools_path%\out2rprc\out2rprc.exe" %App_IPU2_CPU_SMP% %App_IPU2_CPU_SMP_RPRC% )

if defined App_DSP1 (
set DSP1_CPU=%DSP1_ID%
set image_gen=1
set App_DSP1_RPRC=%App_DSP1%.rprc )
if defined App_DSP1 (
"%Tools_path%\out2rprc\out2rprc.exe" %App_DSP1% %App_DSP1_RPRC% )

if defined App_DSP2 (
set DSP2_CPU=%DSP2_ID%
set image_gen=1
set App_DSP2_RPRC=%App_DSP2%.rprc )
if defined App_DSP2 (
"%Tools_path%\out2rprc\out2rprc.exe" %App_DSP2% %App_DSP2_RPRC% )

if defined App_EVE1 (
set EVE1_CPU=%EVE1_ID%
set image_gen=1
set App_EVE1_RPRC=%App_EVE1%.rprc )
if defined App_EVE1 (
"%Tools_path%\out2rprc\out2rprc.exe" %App_EVE1% %App_EVE1_RPRC% )

if defined App_EVE2 (
set EVE2_CPU=%EVE2_ID%
set image_gen=1
set App_EVE2_RPRC=%App_EVE2%.rprc )
if defined App_EVE2 (
"%Tools_path%\out2rprc\out2rprc.exe" %App_EVE2% %App_EVE2_RPRC% )

if defined App_EVE3 (
set EVE3_CPU=%EVE3_ID%
set image_gen=1
set App_EVE3_RPRC=%App_EVE3%.rprc )
if defined App_EVE3 (
"%Tools_path%\out2rprc\out2rprc.exe" %App_EVE3% %App_EVE3_RPRC% )

if defined App_EVE4 (
set EVE4_CPU=%EVE4_ID%
set image_gen=1
set App_EVE4_RPRC=%App_EVE4%.rprc )
if defined App_EVE4 (
"%Tools_path%\out2rprc\out2rprc.exe" %App_EVE4% %App_EVE4_RPRC% )

REM ImageGen
if defined image_gen (

REM Gerating MulticoreImage Gen
"%Tools_path%\multicore_image_generator\MulticoreImageGen.exe" LE %Dev_ID% %Out_Path%\AppImage_LE %MPU_CPU0% %App_MPU_CPU0_RPRC%  %MPU_CPU1% %App_MPU_CPU1_RPRC% %MPU_SMP% %App_MPU_SMP_RPRC% %IPU1_CPU0% %App_IPU1_CPU0_RPRC% %IPU1_CPU1% %App_IPU1_CPU1_RPRC% %IPU1_CPU_SMP% %IPU1_CPU_SMP_RPRC% %IPU2_CPU0% %App_IPU2_CPU0_RPRC% %IPU2_CPU1% %App_IPU2_CPU1_RPRC% %IPU2_CPU_SMP% %IPU2_CPU_SMP_RPRC% %DSP1_CPU% %App_DSP1_RPRC% %DSP2_CPU% %App_DSP2_RPRC% %EVE1_CPU% %App_EVE1_RPRC% %EVE2_CPU% %App_EVE2_RPRC% %EVE3_CPU% %App_EVE3_RPRC% %EVE4_CPU% %App_EVE4_RPRC%

"%Tools_path%\multicore_image_generator\MulticoreImageGen.exe" BE %Dev_ID% %Out_Path%\AppImage_BE %MPU_CPU0% %App_MPU_CPU0_RPRC%  %MPU_CPU1% %App_MPU_CPU1_RPRC% %MPU_SMP% %App_MPU_SMP_RPRC% %IPU1_CPU0% %App_IPU1_CPU0_RPRC% %IPU1_CPU1% %App_IPU1_CPU1_RPRC% %IPU1_CPU_SMP% %IPU1_CPU_SMP_RPRC% %IPU2_CPU0% %App_IPU2_CPU0_RPRC% %IPU2_CPU1% %App_IPU2_CPU1_RPRC% %IPU2_CPU_SMP% %IPU2_CPU_SMP_RPRC% %DSP1_CPU% %App_DSP1_RPRC% %DSP2_CPU% %App_DSP2_RPRC% %EVE1_CPU% %App_EVE1_RPRC% %EVE2_CPU% %App_EVE2_RPRC% %EVE3_CPU% %App_EVE3_RPRC% %EVE4_CPU% %App_EVE4_RPRC%
)
copy %Out_Path%\AppImage_LE %Out_Path%\AppImage

REM copy %Out_Path%\AppImage F:

pause
