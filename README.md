# What?

Moonshine is a C2 framework with a custom Lua interpreter (called Moon, see [here](extern/moon/README.md) for further details) runtime at it's core. The runtime is used in the implants to execute scripts on the remote host, with the option of loading Lua C or script modules to provide additional functionality. The communication channels for both implant and server are implemented as Lua scripts, allowing for complete control and flexibility.

The goals for the framework are:
- To allow for rapid prototyping of new adversarial techniques.
- To allow for simulation of adversarial techniques.
- To have an implant that is easily expandable and customisable, with a minimal core.
- To have an implant that works cross-platform.
- To have a server that can be accessed via a REST based API, so the framework can easily be automated or integrated with other tools.

# Use?

## Build

A Makefile is also included that not only provides a convenience wrapper around CMake, but also for Docker which enables the framework to be cross-compiled within a container containing all the pre-requisite development tools. The Makefile implements the following common targets:

```bash
$ make (debug*|release|install|dist|check|clean|distclean)
```

\* default target if no arguments are supplied.

For example to compile the framework using native development tools already installed on the host, execute:

`make install`

The resultant build artifacts can then be found within the `_install` subdirectory.

The project also includes a test harness. To execute, use the standard CMake commands or use the provided Makefile (`make check`).

To use within another CMake based project, simply include using the method of choice for the project e.g. '[add_subdirectory](https://cmake.org/cmake/help/latest/command/add_subdirectory.html)', '[ExternalProject](https://cmake.org/cmake/help/latest/module/ExternalProject.html)', '[FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html)', '[CPM](https://github.com/cpm-cmake/CPM.cmake)'.

### Building using a Docker container

The Makefile wrapper can also be used to initiate cross-compilation to both the Linux and Windows platforms using a Docker container that has the required development tools pre-installed:

```bash
cd <moon directory>
make TARGET=linux install

-or-

make TARGET=mingw install
```

Unfortunately due to the requirement of being able to execute compiled binaries during the build process, cross-compiling the framework to target the macOS platform is not possible within a Docker container.

### Native build instructions

To build, at a minimum you'll need to install the [CMake](https://cmake.org/) build system and version 14 of the [Clang](https://clang.llvm.org/) compiler. All dependant libraries are automatically downloaded and compiled by the build system. Where possible, libraries are statically linked into the resultant artifacts at compile time rather than requiring libraries to be available on the local system during runtime.

#### Windows

For a less arduous setup, use [choco](https://chocolatey.org/). Alternatively, see the "Cross-compiling" section below.

```bash
Set-ExecutionPolicy Bypass -Scope Process -Force;
[Net.ServicePointManager]::SecurityProtocol = "tls12"  # Only needed on Windows < 10
iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
choco feature enable -n allowGlobalConfirmation
choco install git
choco install cmake
choco install make
```

```bash
(new-object System.Net.WebClient).DownloadFile('https://github.com/mstorsjo/llvm-mingw/releases/download/20220323/llvm-mingw-20220323-msvcrt-x86_64.zip','C:\llvm-mingw-20220323-msvcrt-x86_64.zip')
Expand-Archive -Force C:\llvm-mingw-20220323-msvcrt-x86_64.zip C:\LLVM
```

```bash
cd <moon directory>
make install
```

Build artifacts can be found within the `_install` subdirectory.

#### macOS

```bash
xcode-select --install
```

For a less arduous setup, use [Homebrew](https://brew.sh/).

```bash
brew install cmake
```

```bash
cd <moon directory>
make install
```

Build artifacts can be found within the `_install` subdirectory.

#### Linux

Download a recent version of cmake from [here](https://cmake.org/download/) or install via the [Kitware APT Repository](https://apt.kitware.com/), and make sure that the cmake binary is accessible via the `$PATH`.

```bash
sudo apt-get install build-essential wget
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 14
```

```bash
cd <moon directory>
make install
```

Build artifacts can be found within the `_install` subdirectory.

## Try

### Server

```bash
C:\Users\Tester\Desktop\moonshine\bin> server.windows.x86_64.exe -h
Moonshine Server
Usage:
  server [OPTION...]

  -a, --address arg      Management interface bind address. (default:
                         127.0.0.1)
  -p, --port arg         Management interface listening port. (default: 9000)
  -r, --resource arg     Resource root directory. (default:
                         C:\Users\Tester\Desktop\moonshine\share\moonshine\resources)
  -i, --implants arg     Implants root directory. (default:
                         C:\Users\Tester\Desktop\moonshine\share\moonshine\implants)
  -d, --database arg     Database path. (default:
                         C:\Users\Tester\Desktop\moonshine\bin\moonshine.sqlite)
  -c, --certificate arg  Certificate, private key, and dhparam root
                         directory. (default: C:\Users\Tester\Desktop\moonshine\bin)
  -l, --loglevel arg     Log level (>=). (default: 2)
  -v, --version          Print version.
  -h, --help             Print usage.
```

First, the server component will need to be executed. The server provides a REST API and also a WebSocket interface for clients (i.e. operators) to interact with the server. The REST API will allow an operator to create listeners, generate implants artifacts, and send tasks to connected implants.

The server publishes a Swagger / OpenAPI based interface that provides details of the REST API in both a human-readable format (website), and also as a json descriptor file for tools to consume. As such, in conjunction with a tool such as the [openapi-generator](https://github.com/OpenAPITools/openapi-generator), SDK client libraries can be easily generated for many programming languages. See the `examples/client-sdk/python` subdirectory for a Python SDK that has been generated using an openapi generator.

A Jupyter Notebook named `api.ipynb` can also be found within the `examples` repositor.  This uses the generated Python SDK and provides example usage of the framework.

Lua modules and scripts for use in the framework can also be found in `examples/scripts`.
