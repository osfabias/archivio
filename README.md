# Archivio
Archivio is a C library that provides a cross-platform multithreadead
logging system for developers.

Currently supported platforms:
- Windows;
- Linux;
- MacOS.

Platform to be supported (SOON):
- Android;
- iOS.

## How to build
Requirements:
- CMake (version 2.16+)

### 1. Clone project.
```shell
git clone git@github.com:osfabias/archivio.git
cd archivio
```

### 2. Configure and build project via CMake.
```shell
# from archivio/
mkdir build
cd build
cmake ..
cmake --build .
```

### 3. Try run examples.
```shell
# from archivio/build/
cd bin
./example
```
You're done!

## Documentation
```shell
# from archivio/
doxygen
cd doc/html
open index.html
```
