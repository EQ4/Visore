@echo off
setlocal enableextensions enabledelayedexpansion

set mingw_bin_directory=C:\QtSDK\mingw\bin
set qt_core_directory=C:\QtSDK\Desktop\Qt\4.7.4\mingw\include\QtCore

set source_directory="C:\Users\GOOmuckel\Documents\Uni\Masters\code\Visore\code"
set build_directory=%source_directory%\..\..\build
set install_directory=%source_directory%\..\..\install

set PATH=%PATH%;%mingw_bin_directory%;%qt_core_directory%

rmdir /S /Q %build_directory%
mkdir %build_directory%
cd %build_directory%

rmdir /S /Q %install_directory%
mkdir %install_directory%

cmake -G "MinGW Makefiles" --build %build_directory% -D ARCHITECTURE=32bit -D CMAKE_INSTALL_PREFIX=%install_directory% -DCMAKE_BUILD_TYPE=Release %source_directory%

cd %build_directory%
mingw32-make.exe install

cd %install_directory%
for /L %%n in (1 1 500) do if "!__cd__:~%%n,1!" neq "" set /a "len=%%n+1"
for /D /r %%G in ("*") do (
	set relative_path=%%G
	set relative_path=!relative_path:~%len%!
	set folders=!folders!!relative_path!;
)

echo @echo off > %install_directory%\StartVisore.bat
echo set PATH=%%PATH%%;%folders% >> %install_directory%\StartVisore.bat
echo start visore.exe >> %install_directory%\StartVisore.bat