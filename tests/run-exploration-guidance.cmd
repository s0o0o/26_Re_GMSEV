@echo off
setlocal
pushd "%~dp0.."
if not exist build\tests mkdir build\tests
cl /nologo /EHsc /std:c++17 tests\exploration_guidance_test.cpp /Fo:build\tests\ /Fe:build\tests\exploration_guidance_test.exe
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
build\tests\exploration_guidance_test.exe
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
call tests\run-town-dialogue.cmd
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
popd
exit /b 0
:failed
popd
exit /b 1
