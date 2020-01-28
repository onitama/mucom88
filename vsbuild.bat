REM Batch build script for Visual Studio 2017/2019
echo off
cd src
MSBuild mucom88.sln -t:Rebuild -p:Configuration=Release;Platform="Win32"
cd ..
cd hspplugin
MSBuild hspmucom.sln -t:Rebuild -p:Configuration=Release;Platform="Win32"
cd ..
pause
