@echo off
setlocal enabledelayedexpansion

:: 配置版本和路径
set ASTYLE_VERSION_TAG=3.6.9
set "SOURCE_DIR=%cd%"
set "APP_VERSION="
set "APP_VERSION_SUFFIX="
for /f "tokens=*" %%i in (version.inc) do set "%%i"
if defined APP_VERSION_SUFFIX set "APP_VERSION=%APP_VERSION%%APP_VERSION_SUFFIX%"

:: 默认参数
set CLEAN=0
set CHECK_DEPS=1
set TARGET_DIR=%SOURCE_DIR%\dist
set MSYSTEM=MINGW64
set COMPILER_MINGW64=1

:: 解析命令行参数
:parse_args
if "%1"=="" goto end_parse
if "%1"=="-c" set CLEAN=1 & shift & goto parse_args
if "%1"=="--clean" set CLEAN=1 & shift & goto parse_args
if "%1"=="-t" set TARGET_DIR=%2 & shift & shift & goto parse_args
if "%1"=="--target-dir" set TARGET_DIR=%2 & shift & shift & goto parse_args
if "%1"=="--mingw64" set COMPILER_MINGW64=1 & shift & goto parse_args
echo Unknown argument: %1
exit /b 1
:end_parse

:: 配置目录
set "BUILD_DIR=%TEMP%\redpanda-msvc-build"
set "ASTYLE_BUILD_DIR=%BUILD_DIR%\astyle"
set "PACKAGE_DIR=%TEMP%\redpanda-msvc-pkg"
set "ASSETS_DIR=%SOURCE_DIR%\assets"
set "QMAKE=%Qt6_DIR%\bin\qmake.exe"
set "JOM=%Qt6_DIR%\..\..\Tools\QtCreator\bin\jom.exe"
set "NSIS=%ProgramFiles(x86)%\NSIS\makensis.exe"
set "7Z=7z.exe"

:: 架构配置
set NSIS_ARCH=x64
set "PACKAGE_BASENAME=RedPanda.C++.%APP_VERSION%.win64.MingW64"

:: 创建目录
if %CLEAN% equ 1 (
    rmdir /s /q "%BUILD_DIR%" 2>nul
    rmdir /s /q "%PACKAGE_DIR%" 2>nul
)
mkdir "%BUILD_DIR%" "%PACKAGE_DIR%" "%TARGET_DIR%" "%ASTYLE_BUILD_DIR%" "%ASSETS_DIR%" 2>nul

:: 打印进度函数
:fn_print_progress
echo.
echo %1
goto :eof

:: 检查依赖
call :fn_print_progress "Checking dependencies..."
if %CHECK_DEPS% equ 1 (
    where msbuild >nul 2>nul || (echo MSBuild not found & exit /b 1)
    where %QMAKE% >nul 2>nul || (echo qmake not found & exit /b 1)
    where %JOM% >nul 2>nul || (echo jom not found & exit /b 1)
    where %NSIS% >nul 2>nul || (echo NSIS not found & exit /b 1)
    where %7Z% >nul 2>nul || (echo 7-Zip not found & exit /b 1)
)

:: 准备astyle
call :fn_print_progress "Preparing astyle..."
if not exist "%ASSETS_DIR%\astyle" (
    git clone --bare https://gitlab.com/saalen/astyle "%ASSETS_DIR%\astyle" || exit /b 1
)
pushd "%ASSETS_DIR%\astyle"
git fetch --all --tags || exit /b 1
popd
git --work-tree="%ASTYLE_BUILD_DIR%" -C "%ASSETS_DIR%\astyle" checkout -f %ASTYLE_VERSION_TAG% || exit /b 1

:: 编译astyle
call :fn_print_progress "Building astyle..."
pushd "%ASTYLE_BUILD_DIR%"
cmake . -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXE_LINKER_FLAGS="/MT" || exit /b 1
msbuild /m /p:Configuration=Release AStyle.sln || exit /b 1
copy /y Release\AStyle.exe "%PACKAGE_DIR%\astyle.exe" || exit /b 1
popd

:: 构建主应用
call :fn_print_progress "Building main application..."
pushd "%BUILD_DIR%"
%QMAKE% PREFIX="%PACKAGE_DIR%" -spec win32-msvc "DEFINES+=BUILD_INCLUDE_OPENSSL BUILD_MODERN" "%SOURCE_DIR%\Red_Panda_Cpp.pro" || exit /b 1
%JOM% -j%NUMBER_OF_PROCESSORS% || exit /b 1
%JOM% install || exit /b 1
popd

:: 部署Qt依赖
call :fn_print_progress "Deploying Qt dependencies..."
pushd "%PACKAGE_DIR%"
"%Qt6_DIR%\bin\windeployqt.exe" --release RedPandaIDE.exe || exit /b 1
popd

:: 准备打包资源
call :fn_print_progress "Preparing packaging resources..."
copy /y "%SOURCE_DIR%\platform\windows\qt.conf" "%PACKAGE_DIR%" || exit /b 1
copy /y "%SOURCE_DIR%\platform\windows\installer-scripts\*.nsh" "%PACKAGE_DIR%" || exit /b 1

:: 处理MinGW64
if %COMPILER_MINGW64% equ 1 (
    call :fn_print_progress "Processing MinGW64..."
    set "MINGW64_ARCHIVE=x86_64-15.1.0-release-posix-seh-msvcrt-rt_v12-rev0_2.zip"
    set "MINGW64_URL=https://github.com/C14147/RedPandaIDE-Extensions/releases/download/mingw64-15.1-compilers/!MINGW64_ARCHIVE!"
    
    if not exist "%BUILD_DIR%\!MINGW64_ARCHIVE!" (
        curl -L -o "%BUILD_DIR%\!MINGW64_ARCHIVE!" "!MINGW64_URL!" || exit /b 1
    )
    if not exist "%PACKAGE_DIR%\mingw64" (
        %7Z% x -y "%BUILD_DIR%\!MINGW64_ARCHIVE!" -o"%PACKAGE_DIR%" || exit /b 1
        ren "%PACKAGE_DIR%\x86_64-15.1.0-release-posix-seh-msvcrt-rt_v12-rev0" mingw64 || exit /b 1
    )
)

:: 生成安装包
call :fn_print_progress "Building installer..."
pushd "%PACKAGE_DIR%"
set "SETUP_NAME=%PACKAGE_BASENAME%.Setup.exe"
set "PORTABLE_NAME=%PACKAGE_BASENAME%.Portable.7z"

%NSIS% /DAPP_VERSION=%APP_VERSION% /DARCH=%NSIS_ARCH% /DFINALNAME="%SETUP_NAME%" /DHAVE_MINGW64 redpanda.nsi || exit /b 1

:: 生成便携版
%7Z% x -y "%SETUP_NAME%" -o"RedPanda-CPP" -xr!$PLUGINSDIR -x!uninstall.exe || exit /b 1
%7Z% a -mmt -mx9 -ms=on -mqs=on -mf=BCJ2 "%PORTABLE_NAME%" "RedPanda-CPP" || exit /b 1

:: 移动产物
move /y "%SETUP_NAME%" "%TARGET_DIR%" || exit /b 1
move /y "%PORTABLE_NAME%" "%TARGET_DIR%" || exit /b 1
popd

call :fn_print_progress "Build completed successfully!"
endlocal