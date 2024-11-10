@echo on
chcp 1251 >nul
:: 0. support local builds too
if not defined BUILD_NUMBER (
set BUILD_NUMBER=1
)
if not defined JOB_NAME (
set JOB_NAME=localbuild-genesis-gui
)

set > bin\release\set0.log

:: Copy latest versions of solvers in redist
:: set PATH=C:\Python37;%PATH%
:: set PYTHONPATH=C:\Python37
:: python copy_dll.py

:: 1. VS 2019 environment
if not defined VCToolsVersion (
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64 || goto :error
)

set > bin\release\set1.log

:: 2. clean up binaries
rd bin\release /S /Q
mkdir bin\release || goto :error

:: 3. evironment
echo Using QT6_DIR:
echo %QT6_DIR%

:: 4.1.1 qmake recursively
cd genesis || goto :error
%QT6_DIR%\bin\qmake.exe -r -tp vc || goto :error

:: 4.1.2 build solution
"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\amd64\MSBuild.exe" /target:Build /property:Configuration=Release /maxcpucount:12 || goto :error
cd %~dp0



:: 7. make publish dir
if exist C:\Builds\%JOB_NAME%\%BUILD_NUMBER% rd /q /s C:\Builds\%JOB_NAME%\%BUILD_NUMBER%
mkdir C:\Builds\%JOB_NAME%\%BUILD_NUMBER% || goto :error


:: 9. copy all artefacts
copy bin\release\*.exe C:\Builds\%JOB_NAME%\%BUILD_NUMBER% /Y || goto :error


:: 10. copy binaries and installers
xcopy /s /y /i installer\common           C:\Builds\%JOB_NAME%\%BUILD_NUMBER%\extern_libs\installer\common
xcopy /s /y /i installer\languages        C:\Builds\%JOB_NAME%\%BUILD_NUMBER%\extern_libs\installer\languages
xcopy /s /y /i installer\resource         C:\Builds\%JOB_NAME%\%BUILD_NUMBER%\extern_libs\installer\resource
xcopy /s /y /i installer\vc_redist        C:\Builds\%JOB_NAME%\%BUILD_NUMBER%\extern_libs\installer\vc_redist
xcopy /s /y /i installer\common_genesis C:\Builds\%JOB_NAME%\%BUILD_NUMBER%\extern_libs\installer\common_genesis
xcopy /s /y /i installer\genesis          C:\Builds\%JOB_NAME%\%BUILD_NUMBER%\extern_libs\installer\genesis

:: Dll's for installer
mkdir C:\Builds\%JOB_NAME%\%BUILD_NUMBER%\extern_libs\psql\bin
mkdir C:\Builds\%JOB_NAME%\%BUILD_NUMBER%\extern_libs\openssl\bin64
mkdir C:\Builds\%JOB_NAME%\%BUILD_NUMBER%\extern_libs\python_toolkit
copy extern\common_thirdparty\psql\bin\libeay32.dll           C:\Builds\%JOB_NAME%\%BUILD_NUMBER%\extern_libs\psql\bin\libeay32.dll /Y
copy extern\common_thirdparty\psql\bin\libiconv-2.dll         C:\Builds\%JOB_NAME%\%BUILD_NUMBER%\extern_libs\psql\bin\libiconv-2.dll /Y
copy extern\common_thirdparty\psql\bin\libintl-9.dll          C:\Builds\%JOB_NAME%\%BUILD_NUMBER%\extern_libs\psql\bin\libintl-9.dll /Y
copy extern\common_thirdparty\psql\bin\libpq.dll              C:\Builds\%JOB_NAME%\%BUILD_NUMBER%\extern_libs\psql\bin\libpq.dll /Y
copy extern\common_thirdparty\psql\bin\libwinpthread-1.dll    C:\Builds\%JOB_NAME%\%BUILD_NUMBER%\extern_libs\psql\bin\libwinpthread-1.dll /Y
copy extern\common_thirdparty\psql\bin\ssleay32.dll           C:\Builds\%JOB_NAME%\%BUILD_NUMBER%\extern_libs\psql\bin\ssleay32.dll /Y

copy extern\common_thirdparty\openssl\bin64\libcryptoMD.dll   C:\Builds\%JOB_NAME%\%BUILD_NUMBER%\extern_libs\openssl\bin64\libcryptoMD.dll /Y
copy extern\common_thirdparty\openssl\bin64\libsslMD.dll      C:\Builds\%JOB_NAME%\%BUILD_NUMBER%\extern_libs\openssl\bin64\libsslMD.dll /Y

:: Dll's for execute from folder

copy extern\common_thirdparty\psql\bin\libeay32.dll           C:\Builds\%JOB_NAME%\%BUILD_NUMBER%\libeay32.dll /Y
copy extern\common_thirdparty\psql\bin\libiconv-2.dll         C:\Builds\%JOB_NAME%\%BUILD_NUMBER%\libiconv-2.dll /Y
copy extern\common_thirdparty\psql\bin\libintl-9.dll          C:\Builds\%JOB_NAME%\%BUILD_NUMBER%\libintl-9.dll /Y
copy extern\common_thirdparty\psql\bin\libpq.dll              C:\Builds\%JOB_NAME%\%BUILD_NUMBER%\libpq.dll /Y
copy extern\common_thirdparty\psql\bin\libwinpthread-1.dll    C:\Builds\%JOB_NAME%\%BUILD_NUMBER%\libwinpthread-1.dll /Y
copy extern\common_thirdparty\psql\bin\ssleay32.dll           C:\Builds\%JOB_NAME%\%BUILD_NUMBER%\ssleay32.dll /Y

copy extern\common_thirdparty\openssl\bin64\libcryptoMD.dll   C:\Builds\%JOB_NAME%\%BUILD_NUMBER%\libcryptoMD.dll /Y
copy extern\common_thirdparty\openssl\bin64\libsslMD.dll      C:\Builds\%JOB_NAME%\%BUILD_NUMBER%\libsslMD.dll /Y

::11 windeploy
%QT6_DIR%\bin\windeployqt.exe C:\Builds\%JOB_NAME%\%BUILD_NUMBER%\client.exe --no-compiler-runtime || goto :error
cd %~dp0

:: Done
goto :EOF

:: Failed
:error
echo Failed with error #%errorlevel%.
exit /b %errorlevel%
