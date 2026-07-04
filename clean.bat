@echo off
setlocal
cd /d "%~dp0"

if exist raytracer.exe del /q raytracer.exe
if exist results\*.ppm del /q results\*.ppm
if exist results\*.png del /q results\*.png
if exist results\*.csv del /q results\*.csv
if exist results\scene_* (
    for /d %%D in (results\scene_*) do rd /s /q "%%D"
)

echo Clean finished.
exit /b 0
