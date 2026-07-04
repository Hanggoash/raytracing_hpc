# 基于 OpenMP 的光线追踪渲染算法并行优化研究

本项目是一个使用 C++17 实现的光线追踪渲染器及其并行计算优化。程序计算每个像素颜色，输出 PPM 图片，并提供串行版本、OpenMP 并行版本、benchmark 统计、性能图绘制和 PPM 转 PNG 的 python 脚本。

项目为基础的简单场景光线追踪算法（平面、球体、光源、相机的简单场景）的并行优化渲染，所以没有使用 CUDA、OpenCV、OpenGL、Vulkan、Qt 或第三方 C++ 图形库。C++ 主程序只依赖 C++17 标准库和 OpenMP。

## 文件结构

```text
raytracing_hpc/
├── include/
│   ├── vec3.h              # 三维向量、点积、叉积、反射、颜色 clamp
│   ├── ray.h               # 光线结构
│   ├── material.h          # 材质参数
│   ├── object.h            # 物体抽象接口和 HitRecord
│   ├── sphere.h            # 球体求交
│   ├── plane.h             # 平面求交
│   ├── light.h             # 点光源
│   ├── camera.h            # 相机和像素光线生成
│   ├── image.h             # 图像像素存储和 PPM 输出接口
│   ├── renderer.h          # 渲染器接口
│   ├── scene_generator.h   # 多场景生成器接口
│   └── file_utils.h        # Windows/MinGW 目录创建工具
├── src/
│   ├── main.cpp            # 命令行参数、渲染调度、benchmark
│   ├── image.cpp           # P6 PPM 文件输出
│   ├── renderer.cpp        # 串行/OpenMP 渲染、光照、阴影、反射
│   └── scene_generator.cpp # 固定主体 + 随机场景生成逻辑
├── scripts/
│   ├── run_benchmark.py    # 检查编译并运行 benchmark
│   ├── plot_result.py      # 生成性能折线图和多场景热力图
│   └── convert_images.py   # 递归转换 results 下的 PPM 为 PNG
├── results/                # 输出图片、CSV、性能图和多场景子目录
├── build.bat               # Windows 编译脚本
├── run_benchmark.bat       # Windows benchmark 脚本
├── clean.bat               # 清理生成文件
├── Makefile                # 可选 make 构建
└── README.md
```

## 测试环境

实验测试环境：

- Windows 11
- MinGW-w64 提供的 `g++`
- 支持 OpenMP 的 `g++`
- Python 3

Python 依赖：

```bat
python -m pip install pandas matplotlib pillow
```

## 运行方法

### 编译

CMD：

```bat
build.bat
```

PowerShell：

```powershell
.\build.bat
```

核心编译命令为（在 build.bat 脚本中自动执行）：

```bat
g++ -O2 -std=c++17 -fopenmp -Iinclude src\main.cpp src\image.cpp src\renderer.cpp src\scene_generator.cpp -o raytracer.exe
```

编译成功后会生成：

```text
raytracer.exe
```

### 运行串行未优化版本

```bat
raytracer.exe --mode serial --scene-id 1 --scene-size medium --width 1280 --height 720 --output results\serial.ppm
```

PowerShell 中如果直接运行当前目录下的 exe，需要写成：

```powershell
.\raytracer.exe --mode serial --scene-id 1 --scene-size medium --width 1280 --height 720 --output results\serial.ppm
```

### 运行 OpenMP 版本

static 调度：

```bat
raytracer.exe --mode openmp --scene-id 1 --scene-size medium --width 1280 --height 720 --threads 8 --schedule static --output results\openmp_static.ppm
```

dynamic 调度：

```bat
raytracer.exe --mode openmp --scene-id 1 --scene-size medium --width 1280 --height 720 --threads 8 --schedule dynamic --output results\openmp_dynamic.ppm
```

### 多场景渲染参数

场景相关参数：

- `--scene-id N`：场景编号。同一个编号会生成稳定一致的随机场景。
- `--scene-size small|medium|large`：场景规模。
- `--scene-seed N`：随机种子，默认固定为20260704，便于复现实验。

场景规模说明：

```text
small   约 16 个物体
medium  约 38 个物体
large   约 102 个物体
```

示例：渲染一个较大规模随机场景。

```bat
raytracer.exe --mode openmp --scene-id 7 --scene-size large --width 1280 --height 720 --threads 8 --schedule dynamic --output results\scene7_large.ppm
```

### 运行 benchmark

最简单方式：

```bat
run_benchmark.bat
```

也可以直接使用 Python 脚本：

```bat
python scripts\run_benchmark.py
```

以上两种方式会随机生成三个场景。每个场景都会分别运行串行版本、OpenMP static 调度和 OpenMP dynamic 调度，并在不同线程数下计时。最终性能图由所有场景的测试数据综合统计得到。

benchmark 常用参数：

- `--scenes N`：控制多场景 benchmark 中生成并测试的随机场景数量。例如 `--scenes 3` 会生成并测试 `scene_001`、`scene_002`、`scene_003`。
- `--quick`：快速测试模式，用于检查程序是否能正常运行。该模式会降低渲染分辨率、降低反射深度，并且多场景测试最多跑 2 个 small 场景。
- `--full`：完整测试模式，用于更正式地观察 OpenMP 加速效果。该模式会使用更大的默认渲染分辨率，并且多场景测试至少跑 5 个场景。
- `--no-plot`：跳过性能图生成。
- `--no-convert`：跳过 PPM 到 PNG 的自动转换。

快速测试：

```bat
python scripts\run_benchmark.py --quick --scenes 2
```

正式测试：

```bat
python scripts\run_benchmark.py --scenes 3 --scene-size medium --scene-width 800 --scene-height 600
```

较大规模测试，用于更明显观察 OpenMP 加速效果：

```bat
python scripts\run_benchmark.py --full --scenes 5 --scene-size large
```

benchmark 会生成：

```text
results\benchmark.csv
results\time_vs_threads.png
results\speedup_vs_threads.png
results\efficiency_vs_threads.png
results\scene_best_time_heatmap.png
```

benchmark 结束后，`scripts\run_benchmark.py` 会自动调用 `scripts\plot_result.py` 生成性能图，并自动调用 `scripts\convert_images.py` 将 `results` 下的 PPM 图片递归转换为 PNG。

多场景输出会分别存放在：

```text
results\scene_001\
results\scene_002\
results\scene_003\
```

每个场景目录中会包含类似文件：

```text
serial.ppm
openmp_static_t1.ppm
openmp_static_t2.ppm
openmp_static_t4.ppm
openmp_static_t8.ppm
openmp_dynamic_t1.ppm
openmp_dynamic_t2.ppm
openmp_dynamic_t4.ppm
openmp_dynamic_t8.ppm
```

执行图片转换后，会在相同目录下生成对应的 `.png` 文件。

### 清理生成文件

CMD：

```bat
clean.bat
```

PowerShell：

```powershell
.\clean.bat
```

该脚本会删除 `raytracer.exe`，以及 `results` 下生成的 `.ppm`、`.png`、`.csv` 和 `scene_*` 多场景目录。

## 实验结果与分析

### 串行版本与 OpenMP 版本

串行版本是普通单线程渲染流程，核心逻辑是逐行、逐像素计算颜色：

```text
for y
  for x
    generateRay
    traceRay
    setPixel
```

OpenMP 版本对外层像素循环进行并行化：

```cpp
#pragma omp parallel for schedule(static)
```

或：

```cpp
#pragma omp parallel for schedule(dynamic, 1)
```

由于每个像素的光线追踪计算基本相互独立，因此该任务适合使用 OpenMP 多线程并行。并行版本不会改变渲染结果，主要区别是多个线程同时计算不同像素，从而减少总运行时间。

### 统计指标

benchmark 输出文件：

```text
results\benchmark.csv
```

CSV 主要字段：

```text
test_type,scene_id,scene_size,width,height,mode,threads,schedule,time_seconds,speedup,efficiency,object_count,light_count,output_image
```

指标含义：

- `time_seconds`：渲染耗时。
- `speedup`：加速比，计算公式为 `serial_time / parallel_time`。
- `efficiency`：并行效率，计算公式为 `speedup / thread_count`。
- `object_count`：当前场景中的物体数量。
- `light_count`：当前场景中的光源数量。

### 性能图说明

`results\time_vs_threads.png`：

展示多个随机场景综合后的平均运行时间，并用所有场景的串行平均时间作为基准参考。

`results\speedup_vs_threads.png`：

展示多个随机场景综合后的平均加速比。每个场景先用自己的串行时间计算加速比，再对同一线程数和调度策略求平均。

`results\efficiency_vs_threads.png`：

展示多个随机场景综合后的平均并行效率。串行版本本身是基准，不作为并行效率曲线绘制。

`results\scene_best_time_heatmap.png`：

用于多场景比较。横轴是场景编号，纵轴是 `serial`、`openmp_static`、`openmp_dynamic`，每个格子显示该场景下的渲染时间。OpenMP 格子中还会标注该调度方式下测试得到的最优线程数。
