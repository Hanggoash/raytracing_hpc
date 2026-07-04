@echo off
setlocal
cd /d "%~dp0"

if not exist results mkdir results

echo Building raytracer.exe ...
g++ -O2 -std=c++17 -fopenmp -Iinclude src\main.cpp src\image.cpp src\renderer.cpp src\scene_generator.cpp -o raytracer.exe

if errorlevel 1 (
    echo Build failed. Please check that g++ is installed and supports -fopenmp.
    exit /b 1
)

echo Build succeeded: raytracer.exe
exit /b 0
