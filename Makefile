CXX := g++
CXXFLAGS := -O2 -std=c++17 -fopenmp -Iinclude
SOURCES := src/main.cpp src/image.cpp src/renderer.cpp src/scene_generator.cpp
TARGET := raytracer
PYTHON ?= python3

ifeq ($(OS),Windows_NT)
TARGET := raytracer.exe
endif

.PHONY: all run serial omp benchmark plot convert clean

all: $(TARGET)

$(TARGET): $(SOURCES) include/*.h
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

run: omp

serial: $(TARGET)
	./$(TARGET) --mode serial --width 800 --height 600 --output results/serial.ppm

omp: $(TARGET)
	./$(TARGET) --mode openmp --width 800 --height 600 --threads 4 --schedule static --output results/demo.ppm

benchmark: $(TARGET)
	$(PYTHON) scripts/run_benchmark.py

plot:
	$(PYTHON) scripts/plot_result.py

convert:
	$(PYTHON) scripts/convert_images.py

clean:
	rm -f $(TARGET)
	rm -f results/*.ppm results/*.png results/*.csv
	rm -rf results/scene_*
