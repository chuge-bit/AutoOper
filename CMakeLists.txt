cmake_minimum_required(VERSION 3.10)

project(AutoOper)

add_executable(AutoOper src/main.cpp)

# O2
target_compile_options(AutoOper PRIVATE
    $<$<CONFIG:Release>:-O2>
)

# C++23
target_compile_features(AutoOper PRIVATE cxx_std_23)


add_subdirectory(third_party/imgui)

target_link_libraries(AutoOper PRIVATE
    imgui
    stdc++exp
)

# 打包用
target_link_options(AutoOper PRIVATE
    -static-libgcc
    -static-libstdc++
)

# target_include_directories(AutoOper PRIVATE
#     ${CMAKE_SOURCE_DIR}/third_party/imgui
# )