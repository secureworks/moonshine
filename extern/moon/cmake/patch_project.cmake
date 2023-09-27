function(patch_project DIRECTORY PATCH_FILE)
  if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${PATCH_FILE})
    message(FATAL_ERROR "Failed to locate ${CMAKE_CURRENT_SOURCE_DIR}/${PATCH_FILE}")
  endif()

  message(STATUS "Patching \"${CMAKE_CURRENT_SOURCE_DIR}/${DIRECTORY}\" with \"${CMAKE_CURRENT_SOURCE_DIR}/${PATCH_FILE}\"")

  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${DIRECTORY}/.git)
    find_package(Git)
    set(REVERT_COMMAND ${GIT_EXECUTABLE} checkout .)
    set(PATCH_COMMAND ${GIT_EXECUTABLE} apply -v --whitespace=fix ${CMAKE_CURRENT_SOURCE_DIR}/${PATCH_FILE}) #--ignore-space-change --ignore-whitespace
  elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${DIRECTORY}/.hg)
    set(REVERT_COMMAND ${HG_EXECUTABLE} revert --all)
    set(PATCH_COMMAND ${HG_EXECUTABLE} import --no-commit ${CMAKE_CURRENT_SOURCE_DIR}/${PATCH_FILE})
  elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${DIRECTORY}/.svn)
    set(REVERT_COMMAND ${SVN_EXECUTABLE} revert --recursive .)
    set(PATCH_COMMAND ${SVN_EXECUTABLE} patch ${CMAKE_CURRENT_SOURCE_DIR}/${PATCH_FILE})
  else()
    set(REVERT_COMMAND ${CMAKE_COMMAND} -E echo "Skipping revert, \"${CMAKE_CURRENT_SOURCE_DIR}/${DIRECTORY}\" is not under git/hg/svn version control")
    set(PATCH_COMMAND ${PATCH_EXECUTABLE} --input=${CMAKE_CURRENT_SOURCE_DIR}/${PATCH_FILE})
  endif()

  execute_process(COMMAND ${REVERT_COMMAND} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${DIRECTORY})

  if(DEFINED PATCH_FILE)
    execute_process(COMMAND ${PATCH_COMMAND} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${DIRECTORY})
  endif()
endfunction()
