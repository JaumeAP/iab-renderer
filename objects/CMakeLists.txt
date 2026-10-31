# Object pans, movements and automation, on top of IABLib and kept apart from it (nothing in IABLib depends on this
# folder). Not a project of its own: New-Renderer's adm/ and Cinema-MXF's mxf/ add it with add_subdirectory after
# they add IABLib, and link the objects library.
# Compiled with the C++ standard of the part that adds it (mxf: 11, adm: 14), so it uses C++11 only.

if(NOT TARGET IABLib)
    message(FATAL_ERROR "objects/ must be added after IABLib")
endif()

add_library(objects STATIC
    object_types.h
    iab_pan_reader.h
    iab_pan_reader.cpp
    object_block_reader.h
    object_block_reader.cpp
    pan_profile.h
    pan_profile.cpp
    iab_pan_writer.h
    iab_pan_writer.cpp
)
set_target_properties(objects PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(objects PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_include_directories(objects PRIVATE
    ${IABLib_HEADER_PATH}
)
target_link_libraries(objects PUBLIC
    IABLib
)

add_executable(objects_pan_reader_selfcheck
    ${CMAKE_CURRENT_SOURCE_DIR}/selfchecks/objects_pan_reader_selfcheck.cpp
)
target_link_libraries(objects_pan_reader_selfcheck
    objects
)
target_include_directories(objects_pan_reader_selfcheck PRIVATE
    ${IABLib_HEADER_PATH}
)

add_executable(objects_block_reader_selfcheck
    ${CMAKE_CURRENT_SOURCE_DIR}/selfchecks/objects_block_reader_selfcheck.cpp
)
target_link_libraries(objects_block_reader_selfcheck
    objects
)

add_executable(objects_pan_profile_selfcheck
    ${CMAKE_CURRENT_SOURCE_DIR}/selfchecks/objects_pan_profile_selfcheck.cpp
)
target_link_libraries(objects_pan_profile_selfcheck
    objects
)
