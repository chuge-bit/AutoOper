# imgui
add_library(imgui 
    imgui.cpp
    imgui_draw.cpp
    imgui_impl_dx11.cpp
    imgui_impl_win32.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
)

target_include_directories(imgui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(imgui PUBLIC
    d3d11
    dxgi
    d3dcompiler
    dwmapi
)