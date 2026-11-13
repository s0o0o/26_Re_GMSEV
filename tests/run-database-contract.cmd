@echo off
setlocal
pushd "%~dp0.."
if not exist build\tests mkdir build\tests
cl /nologo /EHsc /std:c++17 tests\database_contract_test.cpp /Fo:build\tests\ /Fe:build\tests\database_contract_test.exe
if not "%errorlevel%"=="0" goto failed
build\tests\database_contract_test.exe
if not "%errorlevel%"=="0" goto failed
cl /nologo /EHsc /std:c++17 tests\database_position_test.cpp /Fo:build\tests\ /Fe:build\tests\database_position_test.exe
if not "%errorlevel%"=="0" goto failed
build\tests\database_position_test.exe
if not "%errorlevel%"=="0" goto failed
popd
exit /b 0
:failed
popd
exit /b 1
