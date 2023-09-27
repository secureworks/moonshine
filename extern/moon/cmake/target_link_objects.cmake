# When linking OBJECTS -> OBJECTS -> LIB/EXE, the "results" from the first OBJECT are not propagated to the
# final target. This is apparently by design to avoid accidentally linking the same OBJECT multiple times.
# A workaround for this was found in the link below, is entails using custom functions that "...record object
# lists in a custom target property and then use target_sources() to add the list to the first non-object
# library...". However "a clear drawback of this approach is that the order in which libraries are linked
# becomes important."
# https://gitlab.kitware.com/cmake/cmake/-/issues/18090#note_861617

function(target_link_objects TARGET MODE LIBRARY)
    # When linking two OBJECT libraries together, record the input library objects in
    #   a custom target property "LINKED_OBJECTS" together with any other existing ones
    #   from the input library's LINKED_OBJECTS property.
    # Accumulate LINKED_OBJECTS until reaching a non-object target, and add them as
    #   extra sources - this will de-duplicate the list and link it into the target.
    get_target_property(TARGET_TYPE ${TARGET} TYPE)
    get_target_property(LIBRARY_TYPE ${LIBRARY} TYPE)

    if(TARGET_TYPE STREQUAL "INTERFACE_LIBRARY")
        message(FATAL_ERROR "OBJECT to INTERFACE library linking is not supported.")
    endif()

    if(LIBRARY_TYPE STREQUAL "OBJECT_LIBRARY")
        get_target_property(LIBRARY_LINKED_OBJECTS ${LIBRARY} LINKED_OBJECTS)
        if(LIBRARY_LINKED_OBJECTS MATCHES "-NOTFOUND")
            set(LIBRARY_LINKED_OBJECTS)
        endif()
        if(TARGET_TYPE STREQUAL "OBJECT_LIBRARY")
            set_property(TARGET ${TARGET} APPEND PROPERTY
                    LINKED_OBJECTS ${LIBRARY_LINKED_OBJECTS} $<TARGET_OBJECTS:${LIBRARY}>)
        else()
            target_sources(${TARGET} PRIVATE ${LIBRARY_LINKED_OBJECTS})
        endif()
    endif()
endfunction()

# Behaves like target_link_libraries, but propagates OBJECT libraries' objects
#   up to the first non-object library.
function(target_link_dependencies TARGET)
    set(MODES PUBLIC PRIVATE INTERFACE)
    set(MODE PUBLIC)
    foreach(ARG ${ARGN})
        if(ARG IN_LIST MODES)
            set(MODE ${ARG})
            continue()
        endif()

        if(TARGET "${ARG}")
            target_link_objects(${TARGET} ${MODE} "${ARG}")
        endif()
        target_link_libraries(${TARGET} ${MODE} "${ARG}")
    endforeach()
endfunction()
