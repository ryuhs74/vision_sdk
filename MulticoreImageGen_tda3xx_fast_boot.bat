REM Device Id for TDA3xx - 77
REM Device ID & CPU ID should be in sync with SBL. Refer to SBL user guide for values
set Dev_ID=77

set CORE_ID_IPU1_CPU0=2
set CORE_ID_IPU1_CPU1=3
set CORE_ID_DSP1=6
set CORE_ID_DSP2=7
set CORE_ID_EVE1=8
set CORE_ID_IPU1=14

REM Define Output file path
REM Define Output file path
set Out_Path=%CD%\binaries\vision_sdk\bin\tda3xx-evm\sbl_boot
set Tools_path=%CD%\..\ti_components\drivers\starterware_01_03_00_09\bootloader\Tools

REM Use profile passed from command line, else use default as release
set profile=%1
IF %1.==. (
set profile=release
)

IF NOT EXIST %Out_Path%\ mkdir %Out_Path%

REM Define Input file paths; To skip the core leave it blank
set App_IPU1_CPU0=%CD%\binaries\vision_sdk\bin\tda3xx-evm\vision_sdk_ipu1_0_%profile%.xem4
set App_IPU1_CPU1=%CD%\binaries\vision_sdk\bin\tda3xx-evm\vision_sdk_ipu1_1_%profile%.xem4
set App_DSP1=%CD%\binaries\vision_sdk\bin\tda3xx-evm\vision_sdk_c66xdsp_1_%profile%.xe66
set App_DSP2=%CD%\binaries\vision_sdk\bin\tda3xx-evm\vision_sdk_c66xdsp_2_%profile%.xe66
set App_EVE1=%CD%\binaries\vision_sdk\bin\tda3xx-evm\vision_sdk_arp32_1_%profile%.xearp32F
REM App_IPU1 is to define the IPU SMP application
set App_IPU1=

if defined App_IPU1_CPU0 (
set IPU1_CPU0=%CORE_ID_IPU1_CPU0%
set image_gen=1
set App_IPU1_CPU0_RPRC=%App_IPU1_CPU0%.rprc )
if defined App_IPU1_CPU0 (
"%Tools_path%\out2rprc\out2rprc.exe" %App_IPU1_CPU0% %App_IPU1_CPU0_RPRC% )

if defined App_IPU1_CPU1 (
set IPU1_CPU1=%CORE_ID_IPU1_CPU1%
set image_gen=1
set App_IPU1_CPU1_RPRC=%App_IPU1_CPU1%.rprc )
if defined App_IPU1_CPU1 (
"%Tools_path%\out2rprc\out2rprc.exe" %App_IPU1_CPU1% %App_IPU1_CPU1_RPRC% )

if defined App_DSP1 (
set DSP1_CPU=%CORE_ID_DSP1%
set image_gen=1
set App_DSP1_RPRC=%App_DSP1%.rprc )
if defined App_DSP1 (
"%Tools_path%\out2rprc\out2rprc.exe" %App_DSP1% %App_DSP1_RPRC% )

if defined App_DSP2 (
set DSP2_CPU=%CORE_ID_DSP2%
set image_gen=1
set App_DSP2_RPRC=%App_DSP2%.rprc )
if defined App_DSP2 (
"%Tools_path%\out2rprc\out2rprc.exe" %App_DSP2% %App_DSP2_RPRC% )

if defined App_EVE1 (
set EVE1_CPU=%CORE_ID_EVE1%
set image_gen=1
set App_EVE1_RPRC=%App_EVE1%.rprc )
if defined App_EVE1 (
"%Tools_path%\out2rprc\out2rprc.exe" %App_EVE1% %App_EVE1_RPRC% )

if defined App_IPU1 (
set IPU1_CPU_SMP=%CORE_ID_IPU1%
set image_gen=1
set App_IPU1_RPRC=%App_IPU1%.rprc )
if defined App_IPU1 (
"%Tools_path%\out2rprc\out2rprc.exe" %App_IPU1% %App_IPU1_RPRC% )

if defined image_gen (
REM Generate MulticoreImage Gen
"%Tools_path%\multicore_image_generator\MulticoreImageGen.exe" BE %Dev_ID% %Out_Path%\AppImage_UcEarly_BE %IPU1_CPU0% %App_IPU1_CPU0_RPRC% %IPU1_CPU1% %App_IPU1_CPU1_RPRC%

"%Tools_path%\multicore_image_generator\MulticoreImageGen.exe" BE %Dev_ID% %Out_Path%\AppImage_UcLate_BE %DSP1_CPU% %App_DSP1_RPRC% %DSP2_CPU% %App_DSP2_RPRC% %EVE1_CPU% %App_EVE1_RPRC% %IPU1_CPU_SMP% %App_IPU1_RPRC%
)

pause