@echo off

:: make sure game server is not running by killing it
Taskkill /IM InstanceServerG.exe /F 2> nul
Taskkill /IM ClientG.exe /F 2> nul

pushd %1

:: TODO: reset log file

:: start game like the launcher does
START /B InstanceServerG.exe GAME_INDEX=0 ^
  CLIENT_PORT=16000 ^
  RUN_AS_APP ^
  NO_AUTO_EXIT ^
  LAUNCH_FROM_CLIENT= &

popd
