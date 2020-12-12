cmake_minimum_required(VERSION 3.5)

project(data_desk C)

add_executable(data_desk "source/data_desk_main.c")
target_link_libraries(data_desk ${CMAKE_DL_LIBS})
if (UNIX)
    target_compile_definitions(data_desk PRIVATE BUILD_LINUX=1 BUILD_WIN32=0)
elseif (WIN32)
    target_compile_definitions(data_desk PRIVATE BUILD_LINUX=0 BUILD_WIN32=1)
endif()

set_property(GLOBAL PROPERTY DATA_DESK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

function(data_desk_add_custom_library)
    cmake_parse_arguments(DD "" "LIBNAME" "SRC" ${ARGN})
    add_library("${DD_LIBNAME}" SHARED ${DD_SRC})
    get_property(DATA_DESK_ROOT GLOBAL PROPERTY DATA_DESK_ROOT)
    target_include_directories("${DD_LIBNAME}" PUBLIC "${DATA_DESK_ROOT}/source")
endfunction()

function(data_desk_invoke)
    cmake_parse_arguments(DD "" "LIBNAME;WORKING_DIRECTORY" "DATASPECS;OUTPUTS" ${ARGN})
    add_custom_command(
        OUTPUT ${DD_OUTPUTS}
        COMMAND $<TARGET_FILE:data_desk> --custom $<TARGET_FILE:${DD_LIBNAME}> ${DD_DATASPECS}
        WORKING_DIRECTORY "${DD_WORKING_DIRECTORY}"
        DEPENDS data_desk ${DD_LIBNAME} ${DD_DATASPECS}
        COMMENT "Run data_desk with ${DD_LIBNAME} on ${DD_DATASPECS}."
        VERBATIM)
endfunction()
