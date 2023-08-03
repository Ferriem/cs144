## CS 144 Networking lab

- [Lab0-note](./writeups/check0.md) 
- [Lab1-note](./writeups/check1.md)
- [Lab2-note](./writeups/check2.md)
- [Lab3-note](./writeups/check3.md)
- [Lab4-note](./writeups/check4.md)
- [Lab5-note](./writeups/check5.md)

You can find the needed header for each lab in [header](./writeups/header.md)

[Website]( https://cs144.stanford.edu)

To set up the build system: `cmake -S . -B build`

To compile: `cmake --build build`

To run tests: `cmake --build build --target test`

To run speed benchmarks: `cmake --build build --target speed`

To run clang-tidy (which suggests improvements): `cmake --build build --target tidy`

To format code: `cmake --build build --target format`