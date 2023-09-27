##############################################
##### Build

# Define a name that will be used for the target
set(TARGET "implant_core")

# Add a library to the project.
# https://cmake.org/cmake/help/latest/command/add_library.html
add_library(${TARGET} OBJECT)

# Specify sources to use when building the target.
# https://cmake.org/cmake/help/latest/command/target_sources.html
target_sources(${TARGET} PRIVATE
        state.cpp
        implant.cpp
        dispatcher.cpp
        messages.cpp
        handlers/hello.cpp
        handlers/task.cpp
        handlers/job.cpp
        workers/job.cpp
        $<TARGET_OBJECTS:moon::object>
        $<TARGET_OBJECTS:shared>
        )

# Set library properties that affect how it is built
# https://cmake.org/cmake/help/latest/command/set_target_properties.html
set_target_properties(${TARGET} PROPERTIES LINKER_LANGUAGE CXX)

# Specify the include directories to use when compiling the target.
# https://cmake.org/cmake/help/latest/command/target_include_directories.html
target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Define expected compiler features
# https://cmake.org/cmake/help/latest/command/target_compile_features.html
target_compile_features(${TARGET} PRIVATE cxx_std_17)

# Specify libraries or flags to use when linking the target.
# https://cmake.org/cmake/help/latest/command/target_link_libraries.html
#target_link_libraries(${TARGET} PRIVATE)
target_link_dependencies(${TARGET}
        PRIVATE
            #luv::headers
            luasocket::headers
            luasec::headers
            copas::headers
            binaryheap::headers
            coxpcall::headers
            timerwheel::headers
            struct::headers
        PUBLIC
            shared
            thewisp::signals
            moon::object)

# Specify compile time options when compiling the target.
# https://cmake.org/cmake/help/latest/command/target_compile_options.html
target_compile_options(${TARGET} PRIVATE)

# Specify options to the link step for an target.
# https://cmake.org/cmake/help/git-stage/command/target_link_options.html
target_link_options(${TARGET} PRIVATE)

# Add a dependency between top-level targets.
# https://cmake.org/cmake/help/latest/command/add_dependencies.html
#add_dependencies(${TARGET} socket.core)
