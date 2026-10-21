@echo off
setlocal
where cl >nul 2>nul
if errorlevel 1 (
    echo Run this script from an x64 Native Tools Command Prompt.
    exit /b 1
)
pushd "%~dp0.."
if not exist build\tests mkdir build\tests
if not exist build\tests goto failed
cl /nologo /EHsc /std:c++17 tests\login_flow_test.cpp /Fo:build\tests\ /Fe:build\tests\login_flow_test.exe
if not "%errorlevel%"=="0" goto failed
build\tests\login_flow_test.exe
if not "%errorlevel%"=="0" goto failed
cl /nologo /EHsc /std:c++17 tests\login_input_test.cpp /Fo:build\tests\ /Fe:build\tests\login_input_test.exe
if not "%errorlevel%"=="0" goto failed
build\tests\login_input_test.exe
if not "%errorlevel%"=="0" goto failed
cl /nologo /EHsc /std:c++17 tests\login_lookup_test.cpp /Fo:build\tests\ /Fe:build\tests\login_lookup_test.exe
if not "%errorlevel%"=="0" goto failed
build\tests\login_lookup_test.exe
if not "%errorlevel%"=="0" goto failed
cl /nologo /EHsc /std:c++17 tests\load_test_config_test.cpp /Fo:build\tests\ /Fe:build\tests\load_test_config_test.exe
if not "%errorlevel%"=="0" goto failed
build\tests\load_test_config_test.exe
if not "%errorlevel%"=="0" goto failed
cl /nologo /EHsc /std:c++17 tests\pathfinder_test.cpp TP_2023192003\TP_2023192003\pathfinder.cpp TP_2023192003\TP_2023192003\world_map.cpp /Fo:build\tests\ /Fe:build\tests\pathfinder_test.exe
if not "%errorlevel%"=="0" goto failed
build\tests\pathfinder_test.exe
if not "%errorlevel%"=="0" goto failed
cl /nologo /EHsc /std:c++17 tests\session_packets_test.cpp TP_2023192003\TP_2023192003\session_packets.cpp TP_2023192003\TP_2023192003\session_registry.cpp TP_2023192003\TP_2023192003\world_map.cpp /Fo:build\tests\ /Fe:build\tests\session_packets_test.exe /link ws2_32.lib
if not "%errorlevel%"=="0" goto failed
build\tests\session_packets_test.exe
if not "%errorlevel%"=="0" goto failed
cl /nologo /EHsc /std:c++17 tests\stress_error_test.cpp /Fo:build\tests\ /Fe:build\tests\stress_error_test.exe /link user32.lib
if not "%errorlevel%"=="0" goto failed
build\tests\stress_error_test.exe
if not "%errorlevel%"=="0" goto failed
cl /nologo /EHsc /std:c++17 tests\database_write_test.cpp /Fo:build\tests\ /Fe:build\tests\database_write_test.exe
if not "%errorlevel%"=="0" goto failed
build\tests\database_write_test.exe
if not "%errorlevel%"=="0" goto failed
cl /nologo /EHsc /std:c++17 tests\session_transport_test.cpp TP_2023192003\TP_2023192003\game_session.cpp /Fo:build\tests\ /Fe:build\tests\session_transport_test.exe /link ws2_32.lib
if not "%errorlevel%"=="0" goto failed
build\tests\session_transport_test.exe
if not "%errorlevel%"=="0" goto failed
echo All 9 regression programs passed.
popd
exit /b 0
:failed
echo Regression check failed.
popd
exit /b 1
