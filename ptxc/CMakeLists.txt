cmake_minimum_required(VERSION 3.20)
project(ptxc CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Compiler warnings ──────────────────────────────────────────────
add_compile_options(-Wall -Wextra -Wshadow -Wno-unused-parameter)

# ── Main library: ptxc_lib ─────────────────────────────────────────
file(GLOB_RECURSE PTXC_LIB_SOURCES src/*.cpp)
add_library(ptxc_lib STATIC ${PTXC_LIB_SOURCES})
target_include_directories(ptxc_lib PUBLIC include)

# ── Executable: ptxc ───────────────────────────────────────────────
add_executable(ptxc tools/ptxc/main.cpp)
target_link_libraries(ptxc PRIVATE ptxc_lib)

# ── Optional: CUDA / nvPTXCompiler ─────────────────────────────────
find_package(CUDAToolkit QUIET)
if(CUDAToolkit_FOUND)
    message(STATUS "CUDA Toolkit found — enabling nvPTXCompiler support")
    target_compile_definitions(ptxc_lib PUBLIC PTXC_HAVE_NVPTXCOMPILER=1)
    target_link_libraries(ptxc_lib PUBLIC CUDA::nvptxcompiler)
else()
    message(STATUS "CUDA Toolkit not found — nvPTXCompiler support disabled")
endif()

# ── Tests (Custom SimpleTest) ──────────────────────────────────────
enable_testing()

file(GLOB_RECURSE PTXC_TEST_SOURCES test/*.cpp test/unit/*.cpp test/integration/*.cpp)
add_executable(ptxc_tests ${PTXC_TEST_SOURCES})
target_link_libraries(ptxc_tests PRIVATE ptxc_lib)
target_include_directories(ptxc_tests PRIVATE include test)

# Copy test fixtures so tests can locate them relative to binary dir
file(COPY test/fixtures DESTINATION ${CMAKE_BINARY_DIR}/test)

add_test(NAME ptxc_unit_tests COMMAND ptxc_tests)

# ── Benchmarks ─────────────────────────────────────────────────────
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/bench_optimizer.cpp)
    add_executable(ptxc_bench benchmarks/bench_optimizer.cpp)
    target_link_libraries(ptxc_bench PRIVATE ptxc_lib)
endif()
