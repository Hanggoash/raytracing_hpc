#!/usr/bin/env bash
set -euo pipefail

sudo apt update
sudo apt install -y build-essential make python3 python3-pip python3-venv

if python3 -m pip install pandas matplotlib pillow; then
    echo "Python packages installed globally."
else
    echo "Global pip install failed. Creating a local virtual environment in .venv."
    python3 -m venv .venv
    . .venv/bin/activate
    pip install --upgrade pip
    pip install pandas matplotlib pillow
    echo "Virtual environment ready. Before running Python scripts, use: source .venv/bin/activate"
fi
