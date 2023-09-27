##############################################
##### Build

# Define a name that will be used for the target
set(TARGET "shared")

# Add a library to the project.
# https://cmake.org/cmake/help/latest/command/add_library.html
add_library(${TARGET} OBJECT)

# Specify sources to use when building the target.
# https://cmake.org/cmake/help/latest/command/target_sources.html
target_sources(${TARGET}
        PRIVATE
          encryption.cpp
          metadata.cpp
          message.cpp
          worker.cpp
          ${monocyphercpp_SOURCE_DIR}/vendor/monocypher/src/monocypher.c
          ${monocyphercpp_SOURCE_DIR}/vendor/monocypher/src/optional/monocypher-ed25519.c
        PUBLIC
          $<TARGET_OBJECTS:moon::object>)

# Set library properties that affect how it is built
# https://cmake.org/cmake/help/latest/command/set_target_properties.html
set_target_properties(${TARGET} PROPERTIES LINKER_LANGUAGE CXX)

# Specify the include directories to use when compiling the target.
# https://cmake.org/cmake/help/latest/command/target_include_directories.html
target_include_directories(${TARGET}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            $<BUILD_INTERFACE:${monocyphercpp_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${monocyphercpp_SOURCE_DIR}/vendor/monocypher/src>
            $<BUILD_INTERFACE:${monocyphercpp_SOURCE_DIR}/vendor/monocypher/src/optional>
        )

# Define expected compiler features
# https://cmake.org/cmake/help/latest/command/target_compile_features.html
#target_compile_features(${TARGET} PRIVATE cxx_std_17)

# Specify libraries or flags to use when linking the executable.
# https://cmake.org/cmake/help/latest/command/target_link_libraries.html
target_link_dependencies(${TARGET}
        PRIVATE
            hydrogen
            Threads::Threads
            yas::yas
        PUBLIC
            #moon::static
            moon::object
            #luasocket::headers
            #luasec::headers
            #copas::headers
            #binaryheap::headers
            #coxpcall::headers
            #timerwheel::headers
            #luv::headers
            miniz
            thewisp::signals
        )



# Specify compile time options when compiling the target.
# https://cmake.org/cmake/help/latest/command/target_compile_options.html
target_compile_options(${TARGET} PRIVATE)

# Specify options to the link step for an target.
# https://cmake.org/cmake/help/git-stage/command/target_link_options.html
target_link_options(${TARGET} PRIVATE)

# Add compile definitions to a target.
# https://cmake.org/cmake/help/latest/command/target_compile_definitions.html
target_compile_definitions(${TARGET} PUBLIC LUA_CORE)
