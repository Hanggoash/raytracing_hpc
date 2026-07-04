from __future__ import annotations

from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
CSV = ROOT / "results" / "benchmark.csv"
RESULTS = ROOT / "results"


def require_dependencies():
    try:
        import pandas as pd
        import matplotlib.pyplot as plt
        return pd, plt
    except ImportError as exc:
        print("Missing Python plotting dependency.")
        print("Install with: pip install pandas matplotlib")
        print(f"Details: {exc}")
        return None, None


def plot_metric(df, plt, metric: str, ylabel: str, output_name: str) -> None:
    thread_df = df[(df["test_type"] == "scene_thread") & (df["mode"] == "openmp")].copy()
    if thread_df.empty:
        print(f"No scene thread benchmark rows found for {metric}.")
        return

    grouped = (
        thread_df.groupby(["schedule", "threads"], as_index=False)[metric]
        .mean()
        .sort_values(["schedule", "threads"])
    )

    plt.figure(figsize=(8, 5))
    for schedule, group in grouped.groupby("schedule"):
        plt.plot(group["threads"], group[metric], marker="o", label=f"OpenMP {schedule}")

    serial_rows = df[(df["test_type"] == "scene_best") & (df["mode"] == "serial")]
    if metric == "time_seconds" and not serial_rows.empty:
        serial_time = float(serial_rows["time_seconds"].mean())
        plt.axhline(serial_time, linestyle="--", color="gray", label="serial average")
    elif metric in {"speedup", "efficiency"}:
        plt.axhline(1.0, linestyle="--", color="gray", linewidth=1.0, label="serial baseline")

    plt.xlabel("Threads")
    plt.ylabel(ylabel)
    plt.title(f"Average {ylabel} across generated scenes")
    plt.grid(True, alpha=0.3)
    plt.legend()
    plt.tight_layout()
    out = RESULTS / output_name
    plt.savefig(out, dpi=160)
    plt.close()
    print(f"Saved {out}")


def plot_scene_best_heatmap(df, plt) -> None:
    required = {"test_type", "scene_id", "mode", "schedule", "threads", "time_seconds"}
    if not required.issubset(set(df.columns)):
        print("CSV does not contain multi-scene benchmark columns.")
        return

    scene_df = df[df["test_type"] == "scene_best"].copy()
    if scene_df.empty:
        print("No scene_best rows found. Run the new benchmark first.")
        return

    def method_name(row):
        if row["mode"] == "serial":
            return "serial"
        return f"openmp_{row['schedule']}"

    scene_df["method"] = scene_df.apply(method_name, axis=1)
    method_order = ["serial", "openmp_static", "openmp_dynamic"]
    scene_ids = sorted(scene_df["scene_id"].unique())
    matrix = scene_df.pivot_table(index="method", columns="scene_id",
                                  values="time_seconds", aggfunc="min")
    matrix = matrix.reindex(method_order).reindex(columns=scene_ids)

    thread_matrix = scene_df.pivot_table(index="method", columns="scene_id",
                                         values="threads", aggfunc="first")
    thread_matrix = thread_matrix.reindex(method_order).reindex(columns=scene_ids)

    plt.figure(figsize=(max(7, len(scene_ids) * 1.2), 4.8))
    ax = plt.gca()
    image = ax.imshow(matrix.values, cmap="YlOrRd")

    ax.set_xticks(range(len(scene_ids)))
    ax.set_xticklabels([str(scene_id) for scene_id in scene_ids])
    ax.set_yticks(range(len(method_order)))
    ax.set_yticklabels(method_order)
    ax.set_xlabel("Scene ID")
    ax.set_ylabel("Render mode")
    ax.set_title("Best render time by generated scene")

    for y, method in enumerate(method_order):
        for x, scene_id in enumerate(scene_ids):
            value = matrix.loc[method, scene_id]
            threads = thread_matrix.loc[method, scene_id]
            if value != value:
                label = ""
            elif method == "serial":
                label = f"{value:.3f}s"
            else:
                label = f"{value:.3f}s\nT={int(threads)}"
            ax.text(x, y, label, ha="center", va="center", fontsize=9, color="black")

    colorbar = plt.colorbar(image, ax=ax)
    colorbar.set_label("Time (seconds)")
    plt.tight_layout()
    out = RESULTS / "scene_best_time_heatmap.png"
    plt.savefig(out, dpi=160)
    plt.close()
    print(f"Saved {out}")


def main() -> int:
    if not CSV.exists():
        print("results/benchmark.csv not found. Run run_benchmark.bat first.")
        return 1

    pd, plt = require_dependencies()
    if pd is None:
        return 1

    RESULTS.mkdir(exist_ok=True)
    df = pd.read_csv(CSV)

    plot_metric(df, plt, "time_seconds", "Time (seconds)", "time_vs_threads.png")
    plot_metric(df, plt, "speedup", "Speedup", "speedup_vs_threads.png")
    plot_metric(df, plt, "efficiency", "Parallel efficiency", "efficiency_vs_threads.png")
    plot_scene_best_heatmap(df, plt)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
