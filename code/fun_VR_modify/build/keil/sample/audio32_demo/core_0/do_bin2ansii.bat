@echo off

set core_0_path=%cd%

:loop
if not exist %cd%\tool (
    cd ..
    goto loop
)

set SDK_ROOT=%cd%
set PATH=%SDK_ROOT%\tool\bin;%PATH%

set core1_img=%core_0_path%\core_1_img.dat

if exist %core1_img% del /F/Q %core1_img%

::bin2ascii %core_0_path%\core1.bin %core1_img%


bin2ascii D:\Voice_Recognition\SNC7320\snc7320_sdk_v0.00.001_0720_RealSys_for_core1\build\keil\dual_core_Template\core1.bin %core1_img%