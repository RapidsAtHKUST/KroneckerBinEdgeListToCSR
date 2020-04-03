cmake_minimum_required(VERSION 3.6)
project(kron_converter)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)
## To allow CMake to locate our Find*.cmake files
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

## Show Compilation Commands
option(DUMP_COMPILATION_TO_FILE "dump compilation to files" OFF)
if (NOT DUMP_COMPILATION_TO_FILE)
    set(CMAKE_VERBOSE_MAKEFILE ON)
else ()
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # dump compilation commands to compile_commands.json
endif ()

add_compile_options(-Wall -O3 -g -march=native)
add_definitions(-DUSE_LOG -DTIME_RESULTS)
option(USE_JEMALLOC "use jemalloc" ON)
option(PLAYGROUND "compile playground" ON)
option(USE_TCMALLOC "use tcmalloc" OFF)

include_directories(.)
find_package(OpenMP)
if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
else ()
    # icpc
    if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
        message(icpc)
        set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -qopenmp")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -qopenmp")
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        message(g++)
        set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
    endif ()
endif ()

if (USE_TCMALLOC)
    # 1st: tcmalloc
    message("USE_TCMALLOC")
    find_package(Tcmalloc REQUIRED)
    if (Tcmalloc_FOUND)
        message("TCMALLOC FOUND: ${Tcmalloc_LIBRARY}")
        link_libraries(${Tcmalloc_LIBRARY})
    else ()
        message("NO TCMALLOC ")
    endif ()
elseif (USE_JEMALLOC)
    # 2nd: jemalloc
    message("USE_JEMALLOC")

    find_package(Jemalloc REQUIRED)
    if (JEMALLOC_FOUND)
        message("Link: ${JEMALLOC_LIBRARY}")
        link_libraries(${JEMALLOC_LIBRARY})
    else ()
        message("NO JEMALLOC ")
    endif ()
else ()
    message("Default Malloc")
    #    link_libraries("/usr/local/lib/libmimalloc.so")
endif ()

add_library(graph-utils util/log/log.cpp util/graph/graph.cpp util/md5.cpp util/search/search_util.cpp
        reordering_utils/pkc_reorder_refactor.cpp
        reordering_utils/reorder_utils.cpp)
link_libraries(graph-utils)

## 1st: Edge List to CSR Converter.
find_package(atomic)
if (ATOMIC_FOUND)
    link_libraries(${ATOMIC_LIBRARY})
endif ()
add_executable(kron_converter playground/kron_converter.cpp)

### make-install
install(TARGETS kron_converter RUNTIME DESTINATION .)

## 2nd: CSR Analytics.
add_executable(graph_stats playground/graph_stats.cpp)
install(TARGETS graph_stats RUNTIME DESTINATION .)

if (ATOMIC_FOUND)
else ()
    target_compile_definitions(kron_converter PRIVATE NO_ATOMIC=1)
    target_compile_definitions(graph_stats PRIVATE NO_ATOMIC=1)
endif ()

add_executable(test_primitive playground/test_primitive.cpp)

