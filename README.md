# Tinh Linh Programming Language (`Linh.cpp`)

Tinh Linh (Linh) is a new programming language developed by the Sao Tin Developer Team, aiming for simplicity, modern syntax, and extensibility. This project is the reference implementation in C++.

## Features

- Modern, readable syntax.
- Extended type system (int, float, str, array, map, etc.).
- Strong static type checking.
- Supports `vas`, `const`, and `var` declarations.
- Semantic analyzer and sample bytecode compiler.

## Requirements

- CMake >= 3.15
- C++17 compiler (MSVC, GCC, Clang)
- Git (for fetching dependencies via CPM.cmake)

## Build Instructions

You can build the project manually with CMake, or use the provided build scripts:

### **Option 1: Using the build script (Windows)**

#### **Windows**

Open a terminal in the project root and run:

```sh
script\build\build.bat
```

This script will:

- Configure CMake in the `build` directory
- Build the project in Debug mode
- Run the resulting executable automatically if the build succeeds

### **Option 2: Using the build script (macOS/Linux)**

Open a terminal in the project root and run:

```sh
bash script/build/build.sh
```

This script will:

- Configure CMake in the `build` directory
- Build the project in Debug mode
- Run the resulting executable automatically if the build succeeds

### **Option 3: Manual build with CMake**

```sh
git clone https://github.com/LinhProgrammingLanguage/Linh.git
cd Linh.cpp
mkdir build
cd build
cmake ..
cmake --build .
```

## Running

- Place your Linh source code in `test.li` (or modify Main.cpp to use another file).
- Run the executable:

```sh
./LinhApp
```

## Project Structure

- `LinhC/Parsing/` - Parser, AST, and semantic analysis components.
- `LinhC/Bytecode/` - Bytecode emitter.
- `test.li` - Sample Linh language test file.

## Contributing

Contributions, bug reports, and ideas are welcome! Please open an issue or pull request on GitHub.

## Contact

- Website: [https://linh.kesug.com](https://linh.kesug.com)
- Email: linhprogramminglanguage@gmail.com
- Facebook: [Tinh Linh Programming Language](https://www.facebook.com/linhprogramminglanguage)
- GitHub: [https://github.com/LinhProgrammingLanguage/Linh](https://github.com/LinhProgrammingLanguage/Linh)

---

© 2025 Sao Tin Developer Team. See [LICENSE](LICENSE) for license details.
