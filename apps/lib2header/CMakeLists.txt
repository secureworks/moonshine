# Define a name that will be used for the executable
set(TARGET "lib2header")
add_executable(${TARGET} IMPORTED GLOBAL)
install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/lib2header.lua DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT ${PROJECT_NAME})
