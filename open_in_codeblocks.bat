@echo off
rem BlockCraft by Md Rejaul Korim Sadi (231-115-349)
rem Double-click to open BlockCraft in Code::Blocks. The first time, it installs whatever is missing.
setlocal
cd /d "%~dp0"
set "CB=C:\Program Files\CodeBlocks"
set "MROOT=C:\msys64"

if not exist "%CB%\codeblocks.exe" (
    echo Installing Code::Blocks with its MinGW compiler, about 400 MB ...
    winget install --id CodeBlocks.CodeBlocks.MinGW -e --accept-source-agreements --accept-package-agreements
)

if not exist "%MROOT%\ucrt64\include\GL\freeglut.h" (
    if not exist "%MROOT%\usr\bin\bash.exe" (
        echo Installing MSYS2, used only to download freeglut ...
        winget install --id MSYS2.MSYS2 -e --accept-source-agreements --accept-package-agreements
    )
    echo Downloading freeglut ...
    "%MROOT%\usr\bin\bash.exe" -lc "pacman -Sy --needed --noconfirm --disable-download-timeout mingw-w64-ucrt-x86_64-freeglut"
)

if not exist "%MROOT%\ucrt64\include\GL\freeglut.h" (
    echo freeglut could not be downloaded. Check the internet connection and run this file again.
    pause
    exit /b 1
)

mkdir deps\include\GL deps\lib bin 2>nul
copy /y "%MROOT%\ucrt64\include\GL\*.h" deps\include\GL\ >nul
copy /y "%MROOT%\ucrt64\lib\libfreeglut.dll.a" deps\lib\ >nul
copy /y "%MROOT%\ucrt64\bin\libfreeglut.dll" bin\ >nul
for %%d in (libgcc_s_seh-1.dll libwinpthread-1.dll) do (
    if exist "%CB%\MinGW\bin\%%d" copy /y "%CB%\MinGW\bin\%%d" bin\ >nul
)

echo Opening BlockCraft. Press F9 in Code::Blocks to build and play.
start "" "%CB%\codeblocks.exe" "%cd%\BlockCraft.cbp"
