#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "file_utils.h"
#include "renderer.h"
#include "scene_generator.h"

struct Options {
    std::string mode = "openmp";
    int width = 800;
    int height = 600;
    int threads = 4;
    std::string schedule = "static";
    std::string output = "results/demo.ppm";
    int maxDepth = 2;
    bool benchmark = false;
    bool benchmarkFull = false;
    bool benchmarkQuick = false;
    int benchmarkWidth = 1280;
    int benchmarkHeight = 720;
    int sceneId = 1;
    int sceneCount = 3;
    int sceneWidth = 800;
    int sceneHeight = 600;
    SceneSize sceneSize = SceneSize::Medium;
    unsigned int sceneSeed = 24281092U;
};

struct RenderResult {
    double seconds;
    int objectCount;
    int lightCount;
};

struct BenchmarkRow {
    std::string testType;
    int sceneId;
    std::string sceneSize;
    int width;
    int height;
    std::string mode;
    int threads;
    std::string schedule;
    double timeSeconds;
    double speedup;
    double efficiency;
    int objectCount;
    int lightCount;
    std::string outputImage;
};

std::string toLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

int parseInt(const std::string& value, const std::string& name) {
    try {
        int parsed = std::stoi(value);
        if (parsed <= 0) {
            throw std::invalid_argument("non-positive");
        }
        return parsed;
    } catch (...) {
        throw std::invalid_argument("Invalid value for " + name + ": " + value);
    }
}

unsigned int parseSeed(const std::string& value, const std::string& name) {
    try {
        unsigned long parsed = std::stoul(value);
        if (parsed > std::numeric_limits<unsigned int>::max()) {
            throw std::out_of_range("seed too large");
        }
        return static_cast<unsigned int>(parsed);
    } catch (...) {
        throw std::invalid_argument("Invalid value for " + name + ": " + value);
    }
}

std::string formatSceneDirectory(int sceneId) {
    std::ostringstream out;
    out << "results/scene_" << std::setw(3) << std::setfill('0') << sceneId;
    return out.str();
}

std::string openmpImageName(const std::string& directory, const std::string& schedule, int threads) {
    return directory + "/openmp_" + schedule + "_t" + std::to_string(threads) + ".ppm";
}

void printUsage() {
    std::cout
        << "OpenMP Ray Tracing Renderer\n\n"
        << "Render examples:\n"
        << "  raytracer.exe --mode serial --scene-id 2 --scene-size medium --width 1280 --height 720 --output results/serial.ppm\n"
        << "  raytracer.exe --mode openmp --scene-id 2 --scene-size medium --width 1280 --height 720 --threads 8 --schedule static --output results/openmp_static.ppm\n"
        << "  raytracer.exe --mode openmp --scene-id 2 --scene-size large --width 1280 --height 720 --threads 8 --schedule dynamic --output results/openmp_dynamic.ppm\n\n"
        << "Benchmark:\n"
        << "  raytracer.exe --benchmark\n"
        << "  raytracer.exe --benchmark --benchmark-full --scene-size large\n"
        << "  raytracer.exe --benchmark --benchmark-quick\n\n"
        << "Options:\n"
        << "  --mode serial|openmp\n"
        << "  --width N\n"
        << "  --height N\n"
        << "  --threads N\n"
        << "  --schedule static|dynamic\n"
        << "  --output path.ppm\n"
        << "  --max-depth N\n"
        << "  --scene-id N\n"
        << "  --scene-size small|medium|large\n"
        << "  --scene-count N\n"
        << "  --scene-width N\n"
        << "  --scene-height N\n"
        << "  --scene-seed N\n";
}

Options parseOptions(int argc, char** argv) {
    Options options;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        auto requireValue = [&](const std::string& name) -> std::string {
            if (i + 1 >= argc) {
                throw std::invalid_argument("Missing value after " + name);
            }
            return argv[++i];
        };

        if (arg == "--help" || arg == "-h") {
            printUsage();
            std::exit(0);
        } else if (arg == "--benchmark") {
            options.benchmark = true;
        } else if (arg == "--benchmark-full") {
            options.benchmark = true;
            options.benchmarkFull = true;
        } else if (arg == "--benchmark-quick") {
            options.benchmark = true;
            options.benchmarkQuick = true;
        } else if (arg == "--mode") {
            options.mode = toLower(requireValue(arg));
        } else if (arg == "--width") {
            options.width = parseInt(requireValue(arg), arg);
        } else if (arg == "--height") {
            options.height = parseInt(requireValue(arg), arg);
        } else if (arg == "--threads") {
            options.threads = parseInt(requireValue(arg), arg);
        } else if (arg == "--schedule") {
            options.schedule = toLower(requireValue(arg));
        } else if (arg == "--output") {
            options.output = requireValue(arg);
        } else if (arg == "--max-depth") {
            options.maxDepth = parseInt(requireValue(arg), arg);
        } else if (arg == "--benchmark-width") {
            options.benchmarkWidth = parseInt(requireValue(arg), arg);
            options.benchmark = true;
        } else if (arg == "--benchmark-height") {
            options.benchmarkHeight = parseInt(requireValue(arg), arg);
            options.benchmark = true;
        } else if (arg == "--scene-id") {
            options.sceneId = parseInt(requireValue(arg), arg);
        } else if (arg == "--scene-size") {
            options.sceneSize = parseSceneSize(requireValue(arg));
        } else if (arg == "--scene-count") {
            options.sceneCount = parseInt(requireValue(arg), arg);
            options.benchmark = true;
        } else if (arg == "--scene-width") {
            options.sceneWidth = parseInt(requireValue(arg), arg);
            options.benchmark = true;
        } else if (arg == "--scene-height") {
            options.sceneHeight = parseInt(requireValue(arg), arg);
            options.benchmark = true;
        } else if (arg == "--scene-seed") {
            options.sceneSeed = parseSeed(requireValue(arg), arg);
        } else {
            throw std::invalid_argument("Unknown argument: " + arg);
        }
    }

    if (options.mode != "serial" && options.mode != "openmp") {
        throw std::invalid_argument("--mode must be serial or openmp.");
    }
    if (options.schedule != "static" && options.schedule != "dynamic") {
        throw std::invalid_argument("--schedule must be static or dynamic.");
    }

    return options;
}

RenderResult renderOnce(const std::string& mode, int width, int height, int threads,
                        const std::string& schedule, const std::string& outputPath,
                        int maxDepth, int sceneId, SceneSize sceneSize, unsigned int sceneSeed) {
    Renderer renderer = SceneGenerator::createScene(width, height, maxDepth, sceneId, sceneSize, sceneSeed);
    double seconds = 0.0;
    if (mode == "serial") {
        seconds = renderer.serialRender(outputPath);
    } else {
        seconds = renderer.openmpRender(outputPath, threads, schedule);
    }
    return {seconds, static_cast<int>(renderer.objectCount()), static_cast<int>(renderer.lightCount())};
}

void writeBenchmarkHeader(std::ofstream& csv) {
    csv << "test_type,scene_id,scene_size,width,height,mode,threads,schedule,"
        << "time_seconds,speedup,efficiency,object_count,light_count,output_image\n";
}

void writeBenchmarkRow(std::ofstream& csv, const BenchmarkRow& row) {
    csv << row.testType << ','
        << row.sceneId << ','
        << row.sceneSize << ','
        << row.width << ','
        << row.height << ','
        << row.mode << ','
        << row.threads << ','
        << row.schedule << ','
        << std::fixed << std::setprecision(6) << row.timeSeconds << ','
        << std::fixed << std::setprecision(6) << row.speedup << ','
        << std::fixed << std::setprecision(6) << row.efficiency << ','
        << row.objectCount << ','
        << row.lightCount << ','
        << row.outputImage << '\n';
}

double safeSpeedup(double serialTime, double parallelTime) {
    return parallelTime > 0.0 ? serialTime / parallelTime : 0.0;
}

std::vector<int> benchmarkThreadCounts(bool quick) {
    (void)quick;
    return {1, 2, 4, 8};
}

void runSceneBenchmark(std::ofstream& csv, const Options& options,
                       const std::vector<std::string>& schedules,
                       const std::vector<int>& threadCounts, int benchmarkDepth) {
    int sceneCount = options.sceneCount;
    if (options.benchmarkQuick) {
        sceneCount = std::min(sceneCount, 2);
    } else if (options.benchmarkFull) {
        sceneCount = std::max(sceneCount, 5);
    }

    SceneSize sceneSize = options.benchmarkQuick ? SceneSize::Small : options.sceneSize;
    int width = options.benchmarkQuick ? 480 : (options.benchmarkFull ? options.benchmarkWidth : options.sceneWidth);
    int height = options.benchmarkQuick ? 270 : (options.benchmarkFull ? options.benchmarkHeight : options.sceneHeight);
    std::string sizeName = sceneSizeName(sceneSize);

    std::cout << "[benchmark] Scene benchmark: scenes=" << sceneCount
              << ", size=" << sizeName << ", resolution=" << width << "x" << height << "\n";

    for (int offset = 0; offset < sceneCount; ++offset) {
        int sceneId = options.sceneId + offset;
        std::string directory = formatSceneDirectory(sceneId);
        if (!ensureDirectory(directory)) {
            throw std::runtime_error("Failed to create scene output directory: " + directory);
        }

        std::string serialImage = directory + "/serial.ppm";
        std::cout << "[benchmark] scene=" << sceneId << ", serial\n";
        RenderResult serial = renderOnce("serial", width, height, 1, "static", serialImage,
                                         benchmarkDepth, sceneId, sceneSize, options.sceneSeed);
        writeBenchmarkRow(csv, {"scene_best", sceneId, sizeName, width, height, "serial", 1, "none",
                                serial.seconds, 1.0, 1.0, serial.objectCount, serial.lightCount,
                                serialImage});

        for (const std::string& schedule : schedules) {
            double bestTime = std::numeric_limits<double>::max();
            int bestThreads = 1;
            RenderResult bestResult{0.0, serial.objectCount, serial.lightCount};
            std::string bestImage;

            for (int threads : threadCounts) {
                std::string image = openmpImageName(directory, schedule, threads);
                std::cout << "[benchmark] scene=" << sceneId
                          << ", " << schedule << ", threads=" << threads << "\n";
                RenderResult result = renderOnce("openmp", width, height, threads, schedule,
                                                 image, benchmarkDepth, sceneId, sceneSize,
                                                 options.sceneSeed);
                double speedup = safeSpeedup(serial.seconds, result.seconds);
                double efficiency = speedup / static_cast<double>(threads);
                writeBenchmarkRow(csv, {"scene_thread", sceneId, sizeName, width, height,
                                        "openmp", threads, schedule, result.seconds, speedup,
                                        efficiency, result.objectCount, result.lightCount, image});

                if (result.seconds < bestTime) {
                    bestTime = result.seconds;
                    bestThreads = threads;
                    bestResult = result;
                    bestImage = image;
                }
            }

            double bestSpeedup = safeSpeedup(serial.seconds, bestTime);
            double bestEfficiency = bestSpeedup / static_cast<double>(bestThreads);
            writeBenchmarkRow(csv, {"scene_best", sceneId, sizeName, width, height,
                                    "openmp", bestThreads, schedule, bestTime, bestSpeedup,
                                    bestEfficiency, bestResult.objectCount, bestResult.lightCount,
                                    bestImage});
        }
    }
}

void runBenchmark(const Options& options) {
    if (!ensureDirectory("results")) {
        throw std::runtime_error("Failed to create results directory.");
    }

    int benchmarkDepth = options.benchmarkQuick ? 1 : options.maxDepth;
    std::vector<std::string> schedules = {"static", "dynamic"};
    std::vector<int> threadCounts = benchmarkThreadCounts(options.benchmarkQuick);

    std::ofstream csv("results/benchmark.csv");
    if (!csv) {
        throw std::runtime_error("Failed to create results/benchmark.csv");
    }
    writeBenchmarkHeader(csv);

    runSceneBenchmark(csv, options, schedules, threadCounts, benchmarkDepth);

    std::cout << "[benchmark] CSV written to results/benchmark.csv\n";
}

int main(int argc, char** argv) {
    try {
        Options options = parseOptions(argc, argv);

        if (options.benchmark) {
            runBenchmark(options);
            return 0;
        }

        std::cout << "Rendering " << options.width << "x" << options.height
                  << ", mode=" << options.mode
                  << ", scene-id=" << options.sceneId
                  << ", scene-size=" << sceneSizeName(options.sceneSize);
        if (options.mode == "openmp") {
            std::cout << ", threads=" << options.threads << ", schedule=" << options.schedule;
        }
        std::cout << "\n";

        RenderResult result = renderOnce(options.mode, options.width, options.height, options.threads,
                                         options.schedule, options.output, options.maxDepth,
                                         options.sceneId, options.sceneSize, options.sceneSeed);
        std::cout << "Scene objects: " << result.objectCount << ", lights: " << result.lightCount << "\n";
        std::cout << "Output: " << options.output << "\n";
        std::cout << "Render time: " << std::fixed << std::setprecision(6) << result.seconds << " seconds\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        std::cerr << "Use --help to show available options.\n";
        return 1;
    }
}
