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

set(exe_list 
   "symtraverse_all_step1.cpp" 
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
   "utils/logger.cpp")

add_executable(symtraverse_all_step1 ${exe_list})

set(lib_list 
    "libsmt-switch-btor.a"
    "libsmt-switch.a"
    "pthread")

target_link_libraries(symtraverse_all_step1 ${lib_list})

# generate dependent header
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config/testpath.h.in"  "${CMAKE_CURRENT_SOURCE_DIR}/config/testpath.h")

