@echo off
setlocal
pushd "%~dp0.."
if not exist build\tests mkdir build\tests
cl /nologo /EHsc /std:c++17 tests\region_sector_index_test.cpp /Fo:build\tests\ /Fe:build\tests\region_sector_index_test.exe
if not "%errorlevel%"=="0" goto failed
build\tests\region_sector_index_test.exe
if not "%errorlevel%"=="0" goto failed
cl /nologo /EHsc /MD /std:c++17 /permissive- /DGMSEV_MULTIWORKER_TEST /DGMSEV_SECTOR_TEST tests\server_worker_test.cpp TP_2023192003\TP_2023192003\game_session.cpp TP_2023192003\TP_2023192003\session_packets.cpp TP_2023192003\TP_2023192003\session_registry.cpp TP_2023192003\TP_2023192003\world_map.cpp TP_2023192003\TP_2023192003\pathfinder.cpp /Fo:build\tests\ /Fe:build\tests\server_sector_test.exe /link /LIBPATH:TP_2023192003\TP_2023192003
if not "%errorlevel%"=="0" goto failed
build\tests\server_sector_test.exe
if not "%errorlevel%"=="0" goto failed
popd
exit /b 0
:failed
popd
exit /b 1
