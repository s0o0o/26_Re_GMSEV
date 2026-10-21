@echo off
setlocal
where cl >nul 2>nul
if errorlevel 1 (
    echo Run from an x64 Native Tools Command Prompt.
    exit /b 1
)
pushd "%~dp0.."
if not exist build\tests mkdir build\tests
if not exist build\tests goto failed
cl /nologo /EHsc /MD /std:c++17 /DSFML_STATIC /I TP_2023192003_CLIENT\packages\SFML-cpp.2.5.1\build\native\include tools\make_hamster_layers.cpp /Fo:build\tests\ /Fe:build\tests\make_hamster_layers.exe /link /LIBPATH:TP_2023192003_CLIENT\packages\SFML-cpp.2.5.1\build\native\lib\x64\Release sfml-graphics-s.lib sfml-window-s.lib sfml-system-s.lib freetype.lib opengl32.lib winmm.lib gdi32.lib user32.lib advapi32.lib
if not "%errorlevel%"=="0" goto failed
build\tests\make_hamster_layers.exe
if not "%errorlevel%"=="0" goto failed
popd
exit /b 0
:failed
popd
exit /b 1
