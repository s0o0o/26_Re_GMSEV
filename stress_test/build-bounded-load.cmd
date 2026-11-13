@echo off
setlocal
pushd "%~dp0.."
if not exist build\tests mkdir build\tests
cl /nologo /EHsc /std:c++17 /W4 stress_test\STRESS_TEST\bounded_load.cpp /Fo:build\tests\ /Fe:build\tests\bounded_load.exe
set result=%errorlevel%
popd
exit /b %result%
