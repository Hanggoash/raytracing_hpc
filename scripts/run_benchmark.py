from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
EXE = ROOT / ("raytracer.exe" if os.name == "nt" else "raytracer")
CSV = ROOT / "results" / "benchmark.csv"


def prepare_results() -> None:
    results = ROOT / "results"
    results.mkdir(exist_ok=True)

    for pattern in ("bench_*.ppm", "bench_*.png"):
        for file_path in results.glob(pattern):
            file_path.unlink(missing_ok=True)

    for file_name in (
        "benchmark.csv",
        "time_vs_threads.png",
        "speedup_vs_threads.png",
        "efficiency_vs_threads.png",
        "scene_best_time_heatmap.png",
    ):
        (results / file_name).unlink(missing_ok=True)

    for scene_dir in results.glob("scene_*"):
        if scene_dir.is_dir():
            shutil.rmtree(scene_dir)


def run_command(command: list[str], cwd: Path) -> int:
    print(" ".join(str(part) for part in command))
    completed = subprocess.run(command, cwd=str(cwd))
    return completed.returncode


def build_if_needed() -> None:
    if EXE.exists():
        return

    print("raytracer executable was not found. Building first...")
    if os.name == "nt":
        build_script = ROOT / "build.bat"
        if not build_script.exists():
            raise FileNotFoundError("build.bat was not found.")
        code = run_command(["cmd", "/c", str(build_script)], ROOT)
    else:
        code = run_command(["make"], ROOT)

    if code != 0 or not EXE.exists():
        raise RuntimeError("Build failed. Please check whether g++ and OpenMP are installed.")


def main() -> int:
    parser = argparse.ArgumentParser(description="Run raytracer benchmark and create benchmark.csv.")
    parser.add_argument("--quick", action="store_true", help="Run a smaller benchmark for smoke testing.")
    parser.add_argument("--full", action="store_true", help="Run a larger multi-scene benchmark.")
    parser.add_argument("--plot", action="store_true", help="Deprecated: plotting now runs by default.")
    parser.add_argument("--convert", action="store_true", help="Deprecated: PPM conversion now runs by default.")
    parser.add_argument("--no-plot", action="store_true", help="Skip matplotlib figure generation after benchmark.")
    parser.add_argument("--no-convert", action="store_true", help="Skip PPM to PNG conversion after benchmark.")
    parser.add_argument("--scenes", type=int, help="Number of random scenes for the multi-scene benchmark.")
    parser.add_argument("--scene-size", choices=["small", "medium", "large"], help="Scene complexity for generated scenes.")
    parser.add_argument("--scene-width", type=int, help="Width for the multi-scene benchmark.")
    parser.add_argument("--scene-height", type=int, help="Height for the multi-scene benchmark.")
    parser.add_argument("--scene-seed", type=int, help="Base random seed for deterministic scene generation.")
    args = parser.parse_args()

    try:
        prepare_results()
        build_if_needed()

        command = [str(EXE), "--benchmark"]
        if args.quick:
            command.append("--benchmark-quick")
        if args.full:
            command.append("--benchmark-full")
        if args.scenes is not None:
            command.extend(["--scene-count", str(args.scenes)])
        if args.scene_size is not None:
            command.extend(["--scene-size", args.scene_size])
        if args.scene_width is not None:
            command.extend(["--scene-width", str(args.scene_width)])
        if args.scene_height is not None:
            command.extend(["--scene-height", str(args.scene_height)])
        if args.scene_seed is not None:
            command.extend(["--scene-seed", str(args.scene_seed)])

        code = run_command(command, ROOT)
        if code != 0:
            raise RuntimeError("Benchmark command failed.")
        if not CSV.exists():
            raise RuntimeError("results/benchmark.csv was not generated.")

        print(f"Benchmark finished: {CSV}")

        if not args.no_plot:
            plot_script = ROOT / "scripts" / "plot_result.py"
            code = run_command([sys.executable, str(plot_script)], ROOT)
            if code != 0:
                print("Plot generation failed or dependencies are missing. Benchmark CSV is still available.")

        if not args.no_convert:
            convert_script = ROOT / "scripts" / "convert_images.py"
            code = run_command([sys.executable, str(convert_script)], ROOT)
            if code != 0:
                print("PNG conversion failed or Pillow is missing. PPM files are still available.")

        return 0
    except Exception as exc:
        print(f"Error: {exc}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
