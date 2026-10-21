@echo off
setlocal
where cl >nul 2>nul
if errorlevel 1 exit /b 1
pushd "%~dp0.."
if not exist build\tests mkdir build\tests
if not exist build\tests goto failed
set "CHARACTER_INC=TP_2023192003_CLIENT\packages\SFML-cpp.2.5.1\build\native\include"
set "CHARACTER_LIB=TP_2023192003_CLIENT\packages\SFML-cpp.2.5.1\build\native\lib\x64\Release"
set "CHARACTER_LINK=sfml-graphics-s.lib sfml-window-s.lib sfml-system-s.lib freetype.lib opengl32.lib winmm.lib gdi32.lib user32.lib advapi32.lib"
cl /nologo /EHsc /MD /std:c++17 /DSFML_STATIC /I %CHARACTER_INC% tools\extract_character_sheets.cpp /Fo:build\tests\ /Fe:build\tests\extract_character_sheets.exe /link /LIBPATH:%CHARACTER_LIB% %CHARACTER_LINK%
if not "%errorlevel%"=="0" goto failed
build\tests\extract_character_sheets.exe
if not "%errorlevel%"=="0" goto failed
cl /nologo /EHsc /MD /std:c++17 /DSFML_STATIC /I %CHARACTER_INC% tools\make_hamster_layers.cpp /Fo:build\tests\ /Fe:build\tests\make_hamster_layers.exe /link /LIBPATH:%CHARACTER_LIB% %CHARACTER_LINK%
if not "%errorlevel%"=="0" goto failed
build\tests\make_hamster_layers.exe TP_2023192003_CLIENT/client_sample/assets/characters/hamtaro/character.png TP_2023192003_CLIENT/client_sample/assets/characters/hamtaro/ cropped
if not "%errorlevel%"=="0" goto failed
build\tests\make_hamster_layers.exe TP_2023192003_CLIENT/client_sample/assets/characters/bijou/character.png TP_2023192003_CLIENT/client_sample/assets/characters/bijou/ cropped
if not "%errorlevel%"=="0" goto failed
popd
exit /b 0
:failed
popd
exit /b 1
