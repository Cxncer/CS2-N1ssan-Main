@echo off
rem Build and run the n1ssan status protocol harness.
rem Requires the Visual Studio 2022 x64 developer environment.
setlocal
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b 1

pushd "%~dp0"
cl /nologo /EHsc /std:c++17 main.cpp /Fe:main.exe
if errorlevel 1 (popd & exit /b 1)

.\main.exe
set RESULT=%errorlevel%
del /q main.obj main.exe 2>nul
popd
exit /b %RESULT%
