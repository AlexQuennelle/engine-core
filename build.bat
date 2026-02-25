@echo off
for %%I in (.) do set parent=%%~nI%%~xI
setlocal ENABLEDELAYEDEXPANSION
set buildType=%1
if "%~1" == "" set /p "buildType=Build type: "
if not exist "bin" mkdir bin
set "CMAKE_BUILD_TYPE_VAL="
if /i "%buildType%" == "release" (
	set buildType=Release
) else (
	set buildType=Debug
)
if not exist "build.win" mkdir "build.win"
cd build.win
cmake -DCMAKE_BUILD_TYPE="!buildType!" .. -G "Ninja"
echo Building Executable
Ninja
echo !buildType!
if /i "!buildType!" == "Debug" (
	cd ../bin
	start %parent%
)
endlocal
pause
exit
