# What?

Moon is a [Lua](https://www.lua.org/) derivative with changes to the module loading system and some additional functionality. It aims to be API and ABI compatible with LuaJIT, or Lua 5.3/5.4, so existing Lua scripts and modules should work as-is.

# Why?

The novell feature of the library is that it allows Lua C based modules to be loaded directly from memory, avoiding disk access. This can be achieved on all three major operating systems (Linux, macOS, and Windows). Standard operating system libraries (.so / .dll) can also be loaded. For Windows a basic "library manager" has also been implemented that records all modules loaded by the runtime allowing any export and import functions to be appropriately mapped when loading additional libraries.

# How?

Lua C modules are loaded using the following techniques that have been abstracted into a library called [llwythwr](extern/llwythwr/README.md).

## Windows

Reimplements the Win32 LoadLibrary API. In a nutshell it...
* Allocates memory within the local process virtual address space.
* Parses the PE header and map each section into memory.
* Performs base relocations if required.
* Processes import and delayed import tables, and if necessary, loads any dependant DLLs. Modify import address tables accordingly.
* Execute any TLS callbacks.
* Sets the correct permission on each page mapped into memory according to the section’s characteristic requirements.

Some attempts have been made to mask the use of Windows functions by using features provided by the [deinamig](extern/deinamig/README.md) library.

## Linux

Uses the [memfd_create](https://man7.org/linux/man-pages/man2/memfd_create.2.html) syscall to create an anonymous file that is stored in memory. The file descriptor returned can then be pass to the standard [dlopen](https://man7.org/linux/man-pages/man3/dlopen.3.html) syscall to load the library.

## macOS:

Uses the (Apple depreciated, but working) NSCreateObjectFileImageFromMemory, NSLinkModule and NSAddressOfSymbol syscalls. The same in-memory loading technique was used by the Lazarus Group.

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

The Makefile wrapper can also be used to initiate cross-compilation to both the Linux and Windows platforms using a Docker container with the required development tools pre-installed:

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

In addition to the standard Lua functions, Moon adds the following global functions:
```lua
-- Returns a table containing a list of all available modules.
preloaded() -> table

-- Returns a table containing a list of all modules currently loaded into memory.
loaded()  -> table

-- Preloads a module ready for loading using 'require('name').
preload(type, name, bytes, compressed) -> boolean

-- Unloads module from memory.
unrequire(name)

-- Dynamically load a library and (optionally) return a Lua function [cfunction] referenced by the symbol name provided.
loadlibm(name, bytes, compressed, sym) -> boolean|cfunction|nil, [errormsg, errorsrc]
```

In addition to the library, the project also includes an executable, `moonlight`, that can be used to execute Lua scripts and C or script based modules from the command line. It also has an interactive shell that can be accessed when a script argument is not provided.

As `moonlight` is simply a light wrapper around the Moon library it may prove useful during the development of scripts and modules, and for testing functionality before scripts are used with other tools such as Moonshine.

```bash
$ moonlight -h
Usage:
  moonlight [options] [script [args]]

Available options are:
  -m path   load library (.lua / .dll/ .zip) from 'path'.
  -e stat   execute string 'stat'
  -c        output script (and/or library) embedded in a C array header file.
  -z        compress input before generating header file.
  -p        add a prefix to header file variables.
  -i        enter interactive mode after executing 'script'
  -l mod    require library 'mod' into global 'mod'
  -l g=mod  require library 'mod' into global 'g'
  -v        show version information
  -E        ignore environment variables
  -W        turn warnings on
  --        stop handling options
  -         stop handling options and execute stdin
```

The `-m`, `-l`, and `-c` flags can be used multiple times.
