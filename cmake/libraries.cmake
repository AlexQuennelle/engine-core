include(FetchContent)

message("Fetching glfw")
if(NOT EMSCRIPTEN)
    set(GLFW_VERSION 3.4)
    find_package(glfw ${GLFW_VERSION} QUIET)
    if (NOT glfw_FOUND)
        FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG ${GLFW_VERSION}
            GIT_SHALLOW ON
            GIT_PRORGRESS ON
        )
        FetchContent_MakeAvailable(glfw)
        FetchContent_GetProperties(glfw SOURCE_DIR GLFW_DIR)
        # target_include_directories(
        # engine-core
        # SYSTEM PRIVATE "${glfw_SOURCE_DIR}/include/GLFW/"
        # )
    endif()
    target_link_libraries(engine-core PUBLIC glfw)
else()
    message("Using port")
    target_link_options(
        engine-core
        PUBLIC
        "--use-port=contrib.glfw3"
    )
    target_compile_options(
        engine-core
        PUBLIC
        "--use-port=contrib.glfw3"
    )
endif()
message("Done!")

message("Fetching bgfx")
set(BGFX_BUILD_EXAMPLES OFF)
FetchContent_Declare(
    bgfx
    GIT_REPOSITORY https://github.com/bkaradzic/bgfx.cmake.git
    GIT_TAG v1.140.9174-515
    GIT_SHALLOW ON
    GIT_PROGRESS ON
)
FetchContent_MakeAvailable(bgfx)
target_link_libraries(engine-core PUBLIC bgfx)
# target_include_directories(
#     "engine-core" PUBLIC
#     "${CMAKE_BINARY_DIR}/_deps/bgfx-src/bgfx/"
# )
target_include_directories(
    engine-core SYSTEM PUBLIC
    "${CMAKE_BINARY_DIR}/_deps/bgfx-src/bx/include/"
    "${CMAKE_BINARY_DIR}/_deps/bgfx-src/bimg/include/"
)
message("Done!")

message("Fetching assimp")
FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG v6.0.4
    GIT_SHALLOW ON
    GIT_PROGRESS ON
)
FetchContent_MakeAvailable(assimp)
target_link_libraries(engine-core PUBLIC assimp)
target_include_directories(
    "engine-core" SYSTEM PUBLIC
    "${CMAKE_BINARY_DIR}/_deps/assimp-src/include/"
)
message("Done!")

message("Fetching imgui")
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.6
    GIT_SHALLOW ON
    GIT_PROGRESS ON
)
FetchContent_MakeAvailable(imgui)
add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imconfig.h
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui.h
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_internal.h
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imstb_rectpack.h
        ${imgui_SOURCE_DIR}/imstb_textedit.h
        ${imgui_SOURCE_DIR}/imstb_truetype.h
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.h
        # ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
        # ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.h
)
target_link_libraries(engine-core PUBLIC imgui)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
target_include_directories(
        imgui
        PUBLIC "${glfw_SOURCE_DIR}/include/"
)
message("Done!")
