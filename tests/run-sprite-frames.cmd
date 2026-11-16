@echo off
setlocal
pushd "%~dp0.."
if not exist build\tests mkdir build\tests
cl /nologo /EHsc /MD /std:c++17 /DSFML_STATIC /I TP_2023192003_CLIENT\packages\SFML-cpp.2.5.1\build\native\include tests\sprite_frame_audit.cpp /Fo:build\tests\ /Fe:build\tests\sprite_frame_audit.exe /link /LIBPATH:TP_2023192003_CLIENT\packages\SFML-cpp.2.5.1\build\native\lib\x64\Release sfml-graphics-s.lib sfml-window-s.lib sfml-system-s.lib freetype.lib opengl32.lib winmm.lib gdi32.lib user32.lib advapi32.lib
if errorlevel 1 goto failed
build\tests\sprite_frame_audit.exe
if errorlevel 1 goto failed
popd
exit /b 0
:failed
popd
exit /b 1
