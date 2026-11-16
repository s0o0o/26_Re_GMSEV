@echo off
setlocal
pushd "%~dp0.."
if not exist build\tests mkdir build\tests
cl /nologo /EHsc /std:c++17 tests\town_walk_quest_test.cpp /Fo:build\tests\ /Fe:build\tests\town_walk_quest_test.exe
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
build\tests\town_walk_quest_test.exe
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
call tests\run-exploration-guidance.cmd
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
popd
exit /b 0
:failed
popd
exit /b 1
