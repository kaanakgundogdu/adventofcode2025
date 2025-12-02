# Advent of Code 2025 (C++)

This project contains C++ solutions for Advent of Code 2025, built using CMake.

### 🛠️ Build Instructions

I used an "out-of-source" build to keep the main directory clean. 

Run the following commands from the root of the project.

Create the build directory:

```
mkdir build
cd build/
```

Generate build files & Compile:
(This step creates the Makefiles and executables)

```
cmake ..
make
```

### 🚀 Running Solutions

After compiling, the executables are located in their respective subdirectories inside the build/ folder.

To run Day 2:

```
cd day_2/
./day2
```

To run Day 1:

```
cd ../day_1/   # (If you are currently in day_2)
./day1_p1
```

### 📂 Project Structure (Build Artifacts)

Inside the build/ folder, the structure mirrors your source code:

```
CMakeCache.txt / Makefile: CMake configuration files.

day_1/: Contains the executable for Day 1.

day_2/: Contains the executable for Day 2.

```
