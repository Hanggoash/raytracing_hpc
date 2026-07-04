@echo off
setlocal
cd /d "%~dp0"

python scripts\run_benchmark.py
if errorlevel 1 (
    echo Benchmark failed.
    exit /b 1
)

echo Benchmark CSV written to results\benchmark.csv
echo Performance figures and PNG images were generated under results.
exit /b 0
