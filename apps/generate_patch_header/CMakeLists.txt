# Define a name that will be used for the executable
set(TARGET "generate_patch_header")
add_executable(${TARGET} IMPORTED GLOBAL)
install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/generate_patch_header.lua DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT ${PROJECT_NAME})
