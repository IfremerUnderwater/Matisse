Matisse
=====================================

Build instructions
------------------

Required tools:

- CMake
- Git
- C/C++ compiler (GCC or Visual Studio)



Compiling on Linux (ubuntu 20.04 recommanded)
-------------------
<a name="linux"></a>

1. Install the required external libraries.

download and execute the dependencies installation script : [install_dep.sh](https://gist.github.com/IfremerUnderwater/566f3d47f769173a10e1b93639cfc667)


2. Checkout Matisse.
```shell
$ git clone https://github.com/IfremerUnderwater/Matisse.git
$ cd Matisse && mkdir Build && cd Build
```

3. Configure and build
```shell
$ cmake -DCMAKE_BUILD_TYPE=RELEASE ../Src/
$ cmake --build .
```

Compiling on Windows
---------------------

instructions coming soon (vcpkg recommanded)