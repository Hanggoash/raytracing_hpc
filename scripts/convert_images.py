from __future__ import annotations

from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
RESULTS = ROOT / "results"


def main() -> int:
    try:
        from PIL import Image
    except ImportError as exc:
        print("Missing Pillow dependency.")
        print("Install with: pip install pillow")
        print(f"Details: {exc}")
        return 1

    RESULTS.mkdir(exist_ok=True)
    ppm_files = sorted(RESULTS.rglob("*.ppm"))
    if not ppm_files:
        print("No PPM images found in results/.")
        return 0

    for ppm_file in ppm_files:
        png_file = ppm_file.with_suffix(".png")
        with Image.open(ppm_file) as image:
            image.save(png_file)
        print(f"Converted {ppm_file.relative_to(RESULTS)} -> {png_file.relative_to(RESULTS)}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
