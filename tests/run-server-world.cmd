@echo off
setlocal
where cl >nul 2>nul
if errorlevel 1 exit /b 1
pushd "%~dp0.."
if not exist build\tests mkdir build\tests
cl /nologo /EHsc /MD /std:c++17 /permissive- /DGMSEV_MULTIWORKER_TEST /DGMSEV_WORLD_TEST tests\server_worker_test.cpp TP_2023192003\TP_2023192003\game_session.cpp TP_2023192003\TP_2023192003\session_packets.cpp TP_2023192003\TP_2023192003\session_registry.cpp TP_2023192003\TP_2023192003\world_map.cpp TP_2023192003\TP_2023192003\pathfinder.cpp /Fo:build\tests\ /Fe:build\tests\server_world_test.exe /link /LIBPATH:TP_2023192003\TP_2023192003
if not "%errorlevel%"=="0" goto failed
build\tests\server_world_test.exe
if not "%errorlevel%"=="0" goto failed
popd
exit /b 0
:failed
popd
exit /b 1
