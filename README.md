# cs202-dungeon-game

This is the GitHub repository for Nguyễn Tấn Minh and Châu Tấn Phát's CS202 game project.

---

## Setup Instructions

Follow these steps to set up, build, and run the project on your local machine.

### 1. Prerequisites
Ensure you have the following installed and configured in your system `PATH`:
* **C++ Compiler:** Supporting C++20 (e.g., GCC/MinGW-w64 14.2.0 or newer).
* **CMake:** Version 3.25 or newer.
* **SFML 3.0.x:** Download the package matching your compiler/architecture from the [SFML Downloads Page](https://www.sfml-dev.org/download.php).

### 2. Configure SFML Path
Open [CMakeLists.txt](file:///CMakeLists.txt) and locate the following line:
```cmake
set(SFML_ROOT "C:/Program Files/SFML/SFML-3.0.2")
```
Update `"C:/Program Files/SFML/SFML-3.0.2"` to the directory where you extracted your local SFML 3.0.x installation.

### 3. Build the Project
Open a terminal in the root directory of the repository and execute:

```powershell
# Configure the build files
cmake -S . -B build -G "MinGW Makefiles"

# Compile the project
cmake --build build
```

> [!NOTE]
> The build system automatically copies the required SFML dynamic link libraries (`.dll` files) to the output binary folder upon building, so you don't need to do it manually.

### 4. Run the Game
Once built, run the executable generated in the output directory:
```powershell
.\build\game.exe
```
