##############################################
##### Build

# Define a name that will be used for the target.
set(TARGET "moon_object")

# Here we define the target.
# https://cmake.org/cmake/help/latest/command/add_library.html
add_library(${TARGET} OBJECT)
add_library(moon::object ALIAS ${TARGET})

# Here we define the source files.

# Moon source files
SET(MOON_SRC
        maux.c
        mcompress.c
        mloader.c
        moutput.c
        mstate.c
        mutil.c
        mzloader.c
        mcompat.c)

IF(WIN32 OR MINGW)
  LIST(APPEND MOON_SRC mwinapi.c)
ENDIF()

# External source/object files
SET(EXT_SRC $<TARGET_OBJECTS:apultra>)

if (MOON_USE_LUAJIT)
  include(CheckLibraryExists)
  include(CheckCSourceCompiles)
  include(CheckCSourceCompiles)
  include(CheckTypeSize)

  SET(CMAKE_REQUIRED_INCLUDES
          ${PROJECT_SOURCE_DIR}/extern/luajit
          ${PROJECT_SOURCE_DIR}/extern/luajit/src
          ${PROJECT_SOURCE_DIR}/extern/luajit/dynasm
          ${CMAKE_CURRENT_BINARY_DIR}
          )

  if (WIN32)
    set (LJVM_MODE peobj)
  elseif (APPLE )
    set (LJVM_MODE machasm)
  else ()
    set (LJVM_MODE elfasm)
  endif ()

  MACRO(LJ_TEST_ARCH stuff)
    CHECK_C_SOURCE_COMPILES("
#undef ${stuff}
#include \"lj_arch.h\"
#if ${stuff}
int main() { return 0; }
#else
#error \"not defined\"
#endif
" ${stuff})
  ENDMACRO()

  MACRO(LJ_TEST_ARCH_VALUE stuff value)
    CHECK_C_SOURCE_COMPILES("
#undef ${stuff}
#include \"lj_arch.h\"
#if ${stuff} == ${value}
int main() { return 0; }
#else
#error \"not defined\"
#endif
" ${stuff}_${value})
  ENDMACRO()

  FOREACH(arch X64 X86 ARM ARM64 PPC PPCSPE MIPS)
    LJ_TEST_ARCH(LJ_TARGET_${arch})
    if(LJ_TARGET_${arch})
      STRING(TOLOWER ${arch} TARGET_LJARCH)
      MESSAGE(STATUS "LuaJIT Target: ${TARGET_LJARCH}")
      BREAK()
    ENDIF()
  ENDFOREACH()

  IF(NOT TARGET_LJARCH)
    MESSAGE(FATAL_ERROR "architecture not supported")
  ELSE()
    MESSAGE(STATUS "LuaJIT target ${TARGET_LJARCH}")
  ENDIF()

  FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/jit)
  FILE(GLOB jit_files ${PROJECT_SOURCE_DIR}/extern/luajit/src/jit/*.lua)
  FILE(COPY ${jit_files} DESTINATION ${CMAKE_BINARY_DIR}/jit)

  SET(DASM_ARCH ${TARGET_LJARCH})
  SET(DASM_FLAGS)
  SET(TARGET_ARCH)
  LIST(APPEND TARGET_ARCH "LUAJIT_TARGET=LUAJIT_ARCH_${TARGET_LJARCH}")
  LJ_TEST_ARCH_VALUE(LJ_LE 1)
  IF(LJ_LE_1)
    SET(DASM_FLAGS ${DASM_FLAGS} -D ENDIAN_LE)
  ELSE()
    SET(DASM_FLAGS ${DASM_FLAGS} -D ENDIAN_BE)
  ENDIF()
  LJ_TEST_ARCH_VALUE(LJ_ARCH_BITS 64)
  IF(LJ_ARCH_BITS_64)
    SET(DASM_FLAGS ${DASM_FLAGS} -D P64)
  ENDIF()
  LJ_TEST_ARCH_VALUE(LJ_HASJIT 1)
  IF(LJ_HASJIT_1)
    SET(DASM_FLAGS ${DASM_FLAGS} -D JIT)
  ENDIF()
  LJ_TEST_ARCH_VALUE(LJ_HASFFI 1)
  IF(LJ_HASFFI_1)
    SET(DASM_FLAGS ${DASM_FLAGS} -D FFI)
  ENDIF()
  LJ_TEST_ARCH_VALUE(LJ_DUALNUM 1)
  IF(LJ_DUALNUM_1)
    SET(DASM_FLAGS ${DASM_FLAGS} -D DUALNUM)
  ENDIF()
  LJ_TEST_ARCH_VALUE(LJ_ARCH_HASFPU 1)
  IF(LJ_ARCH_HASFPU_1)
    SET(DASM_FLAGS ${DASM_FLAGS} -D FPU)
    LIST(APPEND TARGET_ARCH "LJ_ARCH_HASFPU=1")
  ELSE()
    LIST(APPEND TARGET_ARCH "LJ_ARCH_HASFPU=0")
  ENDIF()
  LJ_TEST_ARCH_VALUE(LJ_ABI_SOFTFP 1)
  IF(NOT LJ_ABI_SOFTFP_1)
    SET(DASM_FLAGS ${DASM_FLAGS} -D HFABI)
    LIST(APPEND TARGET_ARCH "LJ_ABI_SOFTFP=0")
  ELSE()
    LIST(APPEND TARGET_ARCH "LJ_ABI_SOFTFP=1")
  ENDIF()
  LJ_TEST_ARCH_VALUE(LJ_NO_UNWIND 1)
  IF(LJ_NO_UNWIND_1)
    SET(DASM_FLAGS ${DASM_FLAGS} -D NO_UNWIND)
    LIST(APPEND TARGET_ARCH "LUAJIT_NO_UNWIND=1")
  ENDIF()
  IF(WIN32)
    SET(DASM_FLAGS ${DASM_FLAGS} -LN -D WIN)
  ENDIF()
  IF(TARGET_LJARCH STREQUAL "x86")
    LJ_TEST_ARCH_VALUE(__SSE2__ 1)
    IF(__SSE2__1)
      SET(DASM_FLAGS ${DASM_FLAGS} -D SSE)
    ENDIF()
  ENDIF()
  IF(TARGET_LJARCH STREQUAL "ppc")
    LJ_TEST_ARCH_VALUE(LJ_ARCH_SQRT 1)
    IF(NOT LJ_ARCH_SQRT_1)
      SET(DASM_FLAGS ${DASM_FLAGS} -D SQRT)
    ENDIF()
    LJ_TEST_ARCH_VALUE(LJ_ARCH_PPC64 1)
    IF(NOT LJ_ARCH_PPC64_1)
      SET(DASM_FLAGS ${DASM_FLAGS} -D GPR64)
    ENDIF()
  ENDIF()

  IF(APPLE)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLUAJIT_UNWIND_EXTERNAL")
  ELSEIF(NOT WIN32)
    IF(NOT LJ_NO_UNWIND_1)
      # Find out whether the target toolchain always generates unwind tables.
      MESSAGE(STATUS "Checking if the toolchain always generates unwind tables")
      SET(TOOLCHAIN_GENERATES_UNWIND_TABLES 0)
      SET(TMPUNWIND_DIR "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp")
      file(WRITE "${TMPUNWIND_DIR}/tmpunwind.c" "extern void b(void);int a(void){b();return 0;}")
      # This seems flimsy; it would be better to use something like CMake's try_compile but
      # there doesn't seem to be a way to avoid linking when using try_compile so this particular
      # code can't use it (at least without modifying the test code)
      execute_process(COMMAND "${CMAKE_C_COMPILER}" -c -x c "${TMPUNWIND_DIR}/tmpunwind.c" -o "${TMPUNWIND_DIR}/tmpunwind.o"
              RESULT_VARIABLE UNWIND_TEST_ERRORED)
      IF(UNWIND_TEST_ERRORED EQUAL 0)
        # Use STRINGS here so that CMake doesn't stop reading the file once it hits a NUL character.
        # Note: STRINGS skips all non-ASCII/binary bytes, but that's okay since we're only checking
        #       for the presence of some ASCII strings.
        file(STRINGS "${TMPUNWIND_DIR}/tmpunwind.o" TMPUNWIND_O)
        string(FIND "${TMPUNWIND_O}" "eh_frame" EH_FRAME_FOUND)
        string(FIND "${TMPUNWIND_O}" "__unwind_info" UNWIND_INFO_FOUND)
        IF(EH_FRAME_FOUND GREATER -1 OR UNWIND_INFO_FOUND GREATER -1)
          SET(TOOLCHAIN_GENERATES_UNWIND_TABLES 1)
        ENDIF()
      ENDIF()

      IF(TOOLCHAIN_GENERATES_UNWIND_TABLES)
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLUAJIT_UNWIND_EXTERNAL")
        MESSAGE(STATUS "Toolchain does generate unwind tables")
      ELSE()
        MESSAGE(STATUS "Toolchain does not generate unwind tables")
      ENDIF()
    ENDIF()
  ENDIF()

  add_executable(minilua ${PROJECT_SOURCE_DIR}/extern/luajit/src/host/minilua.c)
  SET_TARGET_PROPERTIES(minilua PROPERTIES COMPILE_DEFINITIONS "${TARGET_ARCH}")
  if(UNIX)
    find_library(LIBM m)
    if(LIBM)
      target_link_dependencies(minilua m)
    endif()
  endif()

  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/buildvm_arch.h
          COMMAND minilua ${PROJECT_SOURCE_DIR}/extern/luajit/dynasm/dynasm.lua ${DASM_FLAGS} -o ${CMAKE_CURRENT_BINARY_DIR}/buildvm_arch.h ${PROJECT_SOURCE_DIR}/extern/luajit/src/vm_${DASM_ARCH}.dasc
          DEPENDS ${PROJECT_SOURCE_DIR}/extern/luajit/dynasm/dynasm.lua minilua
          )

  ## Source Lists
  SET(SRC_LJLIB
          ${PROJECT_SOURCE_DIR}/extern/luajit/src/lib_base.c
          ${PROJECT_SOURCE_DIR}/extern/luajit/src/lib_math.c
          ${PROJECT_SOURCE_DIR}/extern/luajit/src/lib_bit.c
          ${PROJECT_SOURCE_DIR}/extern/luajit/src/lib_string.c
          ${PROJECT_SOURCE_DIR}/extern/luajit/src/lib_table.c
          ${PROJECT_SOURCE_DIR}/extern/luajit/src/lib_io.c
          ${PROJECT_SOURCE_DIR}/extern/luajit/src/lib_os.c
          ${PROJECT_SOURCE_DIR}/extern/luajit/src/lib_package.c
          ${PROJECT_SOURCE_DIR}/extern/luajit/src/lib_debug.c
          ${PROJECT_SOURCE_DIR}/extern/luajit/src/lib_jit.c
          ${PROJECT_SOURCE_DIR}/extern/luajit/src/lib_ffi.c
          ${PROJECT_SOURCE_DIR}/extern/luajit/src/lib_buffer.c
          )

  SET(SRC_LIBAUX
          ${PROJECT_SOURCE_DIR}/extern/luajit/src/lib_aux.c
          ${PROJECT_SOURCE_DIR}/extern/luajit/src/lib_init.c
          )
  file (GLOB_RECURSE SRC_LJCORE "${PROJECT_SOURCE_DIR}/extern/luajit/src/lj_*.c")
  list (APPEND SRC_LJCORE ${SRC_LJLIB} ${SRC_LIBAUX})
  file (GLOB_RECURSE SRC_BUILDVM  "${PROJECT_SOURCE_DIR}/extern/luajit/src/host/buildvm*.c")
  list (APPEND SRC_BUILDVM ${CMAKE_CURRENT_BINARY_DIR}/buildvm_arch.h)

  ## GENERATE
  ADD_EXECUTABLE(buildvm ${SRC_BUILDVM})
  SET_TARGET_PROPERTIES(buildvm PROPERTIES COMPILE_DEFINITIONS "${TARGET_ARCH}")
  target_include_directories(buildvm
          PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/extern/luajit/src>
          PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

  macro(add_buildvm_target _target _mode)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_target}
            COMMAND buildvm ARGS -m ${_mode} -o ${CMAKE_CURRENT_BINARY_DIR}/${_target} ${ARGN}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS buildvm ${ARGN}
            )
  endmacro(add_buildvm_target)

  if (WIN32)
    add_buildvm_target ( lj_vm.obj peobj )
    set (LJ_VM_SRC ${CMAKE_CURRENT_BINARY_DIR}/lj_vm.obj)
    target_link_libraries(${TARGET} PRIVATE ${LJ_VM_SRC})
    set_source_files_properties( ${LJ_VM_SRC} PROPERTIES EXTERNAL_OBJECT true GENERATED true)
  else ()
    add_buildvm_target ( lj_vm.S ${LJVM_MODE} )
    set (LJ_VM_SRC ${CMAKE_CURRENT_BINARY_DIR}/lj_vm.S)
  endif ()
  add_buildvm_target ( lj_ffdef.h   ffdef   ${SRC_LJLIB} )
  add_buildvm_target ( lj_bcdef.h  bcdef  ${SRC_LJLIB} )
  add_buildvm_target ( lj_folddef.h folddef ${PROJECT_SOURCE_DIR}/extern/luajit/src/lj_opt_fold.c )
  add_buildvm_target ( lj_recdef.h  recdef  ${SRC_LJLIB} )
  add_buildvm_target ( lj_libdef.h  libdef  ${SRC_LJLIB} )
  add_buildvm_target ( vmdef.lua  vmdef  ${SRC_LJLIB} )

  SET(DEPS
          ${LJ_VM_SRC}
          ${CMAKE_CURRENT_BINARY_DIR}/lj_ffdef.h
          ${CMAKE_CURRENT_BINARY_DIR}/lj_bcdef.h
          ${CMAKE_CURRENT_BINARY_DIR}/lj_libdef.h
          ${CMAKE_CURRENT_BINARY_DIR}/lj_recdef.h
          ${CMAKE_CURRENT_BINARY_DIR}/lj_folddef.h
          ${CMAKE_CURRENT_BINARY_DIR}/vmdef.lua
          )

  set(LUA_SRC
          ${SRC_LJCORE}
          ${DEPS}
          )
else()
  # Lua source files
  set(LUA_SRC
          "${PROJECT_SOURCE_DIR}/extern/lua/lapi.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/lcode.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/lctype.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/ldebug.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/ldo.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/ldump.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/lfunc.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/lgc.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/llex.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/lmem.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/lobject.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/lopcodes.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/lparser.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/lstate.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/lstring.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/ltable.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/ltm.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/lundump.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/lvm.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/lzio.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/lauxlib.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/lbaselib.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/lcorolib.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/ldblib.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/liolib.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/lmathlib.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/loadlib.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/loslib.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/lstrlib.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/ltablib.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/lutf8lib.c"
          "${PROJECT_SOURCE_DIR}/extern/lua/linit.c"
          )
endif()

if(MOON_BUILD_AS_CXX)
  SET_SOURCE_FILES_PROPERTIES(${MOON_SRC} ${LUA_SRC} PROPERTIES LANGUAGE CXX )
endif()

######
# Specify sources to use when building the target.
######
# https://cmake.org/cmake/help/latest/command/target_sources.html
target_sources(${TARGET}
        PRIVATE ${LUA_SRC}
        PRIVATE ${MOON_SRC}
        PUBLIC ${EXT_SRC})

######
# Set library properties that affect the target is built.
# https://cmake.org/cmake/help/latest/command/set_target_properties.html
######

if(MOON_BUILD_AS_CXX)
  set_target_properties(${TARGET} PROPERTIES LINKER_LANGUAGE CXX)
else()
  set_target_properties(${TARGET} PROPERTIES LINKER_LANGUAGE C)
endif()

######
# Specify libraries or flags to use when linking the object.
# https://cmake.org/cmake/help/latest/command/target_link_libraries.html
######

target_link_dependencies(${TARGET}
        PRIVATE apultra
        PRIVATE miniz
        PRIVATE llwythwr
        PRIVATE deinamig)

if(UNIX)
  find_library(LIBM m)
  if(NOT LIBM)
    message(FATAL_ERROR "libm not found and is required by lua")
  endif()
  target_link_dependencies(${TARGET} INTERFACE m)
  if(NOT APPLE)
    target_link_dependencies(${TARGET} INTERFACE dl)
  endif()
endif()

######
# Specify the include directories to use when compiling the target.
# https://cmake.org/cmake/help/latest/command/target_include_directories.html
######

target_include_directories(${TARGET}
        PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>)

if (MOON_USE_LUAJIT)
  target_include_directories(${TARGET}
          PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/extern/luajit/src>
          PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
else()
  target_include_directories(${TARGET}
          PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/extern/lua>)
endif()

######
# Specify options to the link step for an target.
# https://cmake.org/cmake/help/latest/command/target_link_options.html
######

target_link_options(${TARGET} PUBLIC)

######
# Add compile definitions to a target.
# https://cmake.org/cmake/help/latest/command/target_compile_definitions.html
######

if (MOON_USE_LUAJIT)
  target_compile_definitions(${TARGET} PUBLIC LUAJIT_ENABLE_LUA52COMPAT)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_compile_definitions(${TARGET} PUBLIC _FILE_OFFSET_BITS=64)
    target_compile_definitions(${TARGET} PUBLIC _LARGEFILE_SOURCE)
  endif()
else()
  target_compile_definitions(${TARGET} PUBLIC LUA_COMPAT_5_3)
endif()

if(WIN32 OR MINGW)
  target_compile_definitions(${TARGET}
          PUBLIC LUA_BUILD_AS_DLL
          PUBLIC _CRT_SECURE_NO_WARNINGS
          PUBLIC LUAJIT_OS=LUAJIT_OS_WINDOWS)
elseif(UNIX)
  if(APPLE)
    target_compile_definitions(${TARGET} PUBLIC LUA_USE_MACOSX LUAJIT_OS=LUAJIT_OS_OSX TARGET_OS_IPHONE=0)
  else()
    target_compile_definitions(${TARGET} PUBLIC LUA_USE_LINUX LUAJIT_OS=LUAJIT_OS_LINUX)
  endif()
endif()

if(MOON_SYSTEM_GETPROCADDRESS)
  target_compile_definitions(${TARGET} PUBLIC SYSTEM_GETPROCADDRESS)
endif()

if(MOON_SYSTEM_GETMODULEHANDLE)
  target_compile_definitions(${TARGET} PUBLIC SYSTEM_GETMODULEHANDLE)
endif()

if(MOON_USE_LUAJIT)
  target_compile_definitions(${TARGET} PUBLIC MOON_USE_LUAJIT)
endif()
