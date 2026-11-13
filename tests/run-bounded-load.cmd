@echo off
setlocal
pushd "%~dp0.."
if not exist build\tests mkdir build\tests
cl /nologo /EHsc /std:c++17 /W4 tests\bounded_load_test.cpp /Fo:build\tests\ /Fe:build\tests\bounded_load_test.exe
if errorlevel 1 goto failed
build\tests\bounded_load_test.exe
if errorlevel 1 goto failed
popd
exit /b 0
:failed
popd
exit /b 1
