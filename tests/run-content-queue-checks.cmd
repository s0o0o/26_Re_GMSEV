@echo off
setlocal
pushd "%~dp0.."
if not exist build\tests mkdir build\tests
call tests\run-regressions.cmd > build\tests\resident-queue-regressions.log 2>&1
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
for %%T in (pashmina-experience content-boundaries resident-experience resident-experience-guidance field-journal field-content chat town-walk-quest field-map sprite-frames wardrobe-ui display travel-client) do (
 call tests\run-%%T.cmd > build\tests\resident-queue-%%T.log 2>&1
 if errorlevel 1 goto failed
 if not errorlevel 0 goto failed
)
for %%C in (Debug Release) do (
 msbuild TP_2023192003_CLIENT\TP_2023192003_CLIENT.sln /p:Configuration=%%C /p:Platform=x64 /v:minimal > build\tests\resident-queue-native-%%C.log 2>&1
 if errorlevel 1 goto failed
 if not errorlevel 0 goto failed
)
echo Content queue regression/window checks and client Debug/Release builds PASS. No live DB run.
popd
exit /b 0
:failed
echo Content queue verification failed. Inspect build/tests/resident-queue-*.log
popd
exit /b 1
