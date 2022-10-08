cmake_minimum_required(VERSION 3.1)

project(btorpaser)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g")

set(CMAKE_BUILD_TYPE Debug)

set(SMT_SWITCH_DIR "${PROJECT_SOURCE_DIR}/deps/smt-switch")

include_directories(${SMT_SWITCH_DIR}/local/include)
include_directories(${PROJECT_SOURCE_DIR}/framework)
include_directories(${PROJECT_SOURCE_DIR}/deps/btor2tools/src)
include_directories(${PROJECT_SOURCE_DIR}/utils)
# include_directories(${PROJECT_SOURCE_DIR}/fmt)

install(DIRECTORY "${PROJECT_SOURCE_DIR}/fmt/"
        DESTINATION include/btor-paser-cpp/fmt
        FILES_MATCHING PATTERN "*.h")




link_directories(${SMT_SWITCH_DIR}/local/lib)

set(framework
    "framework/ts.cpp"
    "framework/btor2_encoder.cpp"
    "framework/symsim.cpp"
    "framework/independence_check.cpp"
    "framework/term_manip.cpp"
    "framework/tracemgr.cpp"
    "framework/symtraverse.cpp"
    "framework/traverse_manip.cpp"
    "framework/sygus_simplify.cpp"
    "framework/state_simplify.cpp"
    "utils/logger.cpp"
)

set(lib_list 
    "libsmt-switch-btor.a"
    "libsmt-switch.a"
    "pthread")

add_library(wasim ${framework})
target_link_libraries(wasim ${lib_list})

add_executable(symtraverse_all_step1 "symtraverse_all_step1.cpp")
target_link_libraries(symtraverse_all_step1 wasim)

add_executable(independence_check_test "independence_check_test.cpp")
target_link_libraries(independence_check_test wasim)

add_executable(solver_test "solver_test.cpp")
target_link_libraries(solver_test wasim)

add_executable(symsim_test "symsim_test.cpp")
target_link_libraries(symsim_test wasim)

# generate dependent header
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config/testpath.h.in"  "${CMAKE_CURRENT_SOURCE_DIR}/config/testpath.h")

