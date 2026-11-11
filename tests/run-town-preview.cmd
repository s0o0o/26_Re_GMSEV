@echo off
setlocal
pushd "%~dp0.."
if not exist build\tests mkdir build\tests
cl /nologo /EHsc /MD /std:c++17 /DSFML_STATIC /I TP_2023192003_CLIENT\packages\SFML-cpp.2.5.1\build\native\include tests\town_npcs_preview.cpp /Fo:build\tests\ /Fe:build\tests\town_npcs_preview.exe /link /LIBPATH:TP_2023192003_CLIENT\packages\SFML-cpp.2.5.1\build\native\lib\x64\Release sfml-graphics-s.lib sfml-window-s.lib sfml-system-s.lib freetype.lib opengl32.lib winmm.lib gdi32.lib user32.lib advapi32.lib
if not "%errorlevel%"=="0" goto failed
build\tests\town_npcs_preview.exe
if not "%errorlevel%"=="0" goto failed
cl /nologo /EHsc /MD /std:c++17 /permissive- /DSFML_STATIC /I TP_2023192003_CLIENT\packages\SFML-cpp.2.5.1\build\native\include tests\town_client_test.cpp TP_2023192003_CLIENT\client_sample\hamster_customizer.cpp TP_2023192003_CLIENT\client_sample\background_blur.cpp /Fo:build\tests\ /Fe:build\tests\town_client_test.exe /link /LIBPATH:TP_2023192003_CLIENT\packages\SFML-cpp.2.5.1\build\native\lib\x64\Release sfml-network-s.lib sfml-graphics-s.lib sfml-window-s.lib sfml-system-s.lib freetype.lib opengl32.lib winmm.lib gdi32.lib user32.lib advapi32.lib ws2_32.lib
if not "%errorlevel%"=="0" goto failed
build\tests\town_client_test.exe
if not "%errorlevel%"=="0" goto failed
popd
exit /b 0
:failed
popd
exit /b 1
