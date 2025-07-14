# Tinh Linh Programming Language (`Linh.cpp`)

Tinh Linh (Linh) is a new programming language developed by the Sao Tin Developer Team, aiming for simplicity, modern syntax, and extensibility. This project is the reference implementation in C++.

## Features

- Modern, readable syntax.
- Extended type system (int, float, str, array, map, etc.).
- Strong static type checking.
- Supports `vas`, `const`, and `var` declarations.
- Semantic analyzer and sample bytecode compiler.

# Release Notes

**Version 0.01 Released!**

Today marks the release of version 0.01 of the Tinh Linh Programming Language.
This release also celebrates the 6-month anniversary since the project began (from January 15, 2025 to July 15, 2025).

Thank you for your support and feedback!

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
script\build\Release.bat
```

This script will:

- Configure CMake in the `build` directory
- Build the project in Debug mode
- Run the resulting executable automatically if the build succeeds

### **Option 2: Using the build script (macOS/Linux)**

Open a terminal in the project root and run:

```sh
bash script/build/Release.sh
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
- Facebook: [Tinh Linh Lang](https://www.facebook.com/people/Tinh-Linh-Lang/61576609030665/)
- GitHub: [https://github.com/LinhProgrammingLanguage/Linh](https://github.com/LinhProgrammingLanguage/Linh)

---

**NOTICE:**

This project was originally licensed under the custom "Tinh Linh - MIT" license.

As of July 14, 2025, the license has been officially changed to the Apache License, Version 2.0.

All new contributions and distributions from this date forward are governed by the Apache License 2.0.

---

© 2025 Sao Tin Developer Team. See [LICENSE](LICENSE.md) for license details.
