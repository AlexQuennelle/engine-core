cmake_minimum_required(VERSION 4.2.1)

# set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_STANDARD 23)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
message("Build type: ${CMAKE_BUILD_TYPE}")

option(CONFIG_USE_WAYLAND "Whether to use Wayland on compatible systems" ON)
option(CONFIG_USE_ASAN "Whether to enable address sanitizer" OFF)
option(CONFIG_USE_UBSAN "Whether to enable undefined behavior sanitizer" OFF)

set(VALID_BACKENDS VULKAN OPEN_GL OPEN_GL_ES DX11 DX12)
set(BACKEND VULKAN CACHE STRING "Graphics API to use")
if(EMSCRIPTEN OR PLATFORM STREQUAL "Web")
    set(EMSCRIPTEN 1)
    message("Emscripten")
    list(REMOVE_ITEM VALID_BACKENDS VULKAN)
    list(REMOVE_ITEM VALID_BACKENDS OPEN_GL_ES)
    list(REMOVE_ITEM VALID_BACKENDS DX11)
    list(REMOVE_ITEM VALID_BACKENDS DX12)
    set(BACKEND OPEN_GL CACHE STRING "Graphics API" FORCE)
elseif(LINUX)
    list(REMOVE_ITEM VALID_BACKENDS DX11)
    list(REMOVE_ITEM VALID_BACKENDS DX12)
elseif(WIN32)
    message("Test")
    add_compile_definitions(NOMINMAX)
    # set(BACKEND OPEN_GL CACHE STRING "Graphics API" FORCE)
endif()
set_property(CACHE BACKEND PROPERTY STRINGS ${VALID_BACKENDS})

if(NOT BACKEND IN_LIST VALID_BACKENDS)
    message(FATAL_ERROR "ERROR: BACKEND must be of of ${VALID_BACKENDS}")
endif()

message("Backend: ${BACKEND}")

if(NOT LINUX)
    set(CONFIG_USE_WAYLAND OFF)
endif()

if(NOT EMSCRIPTEN)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
else()
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)
endif()

if(LINUX AND CONFIG_USE_UBSAN)
    add_compile_options("-fsanitize=undefined")
elseif(LINUX AND CONFIG_USE_ASAN)
    add_compile_options("-fsanitize=address")
endif()

# Change this to match the name of the current folder
project(
  engine-core
  LANGUAGES CXX C
  VERSION 0.0
)

# add_executable("${CMAKE_PROJECT_NAME}")

if(LINUX AND CONFIG_USE_UBSAN)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined")
elseif(LINUX AND CONFIG_USE_ASAN)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif()

add_library(engine-core
    src/imgui/imgui_impl_bgfx.cpp
    src/camera.cpp
    src/mesh.cpp
    src/quaternion.cpp
    src/shader.cpp
    src/vectorN.cpp
    src/window.cpp
)

# Libraries here
set(BGFX_BUILD_EXAMPLES OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_BUILD_SAMPLES OFF)
if(EMSCRIPTEN)
    set(GLFW_BUILD_WAYLAND OFF)
    set(GLFW_BUILD_X11 OFF)
    set(BGFX_CONFIG_MULTITHREADED OFF)
    set(ASSIMP_BUILD_ZLIB ON)
endif()

if(CONFIG_USE_WAYLAND)
    target_compile_definitions(engine-core PUBLIC USE_WAYLAND=true)
else()
    # set(GLFW_BUILD_WAYLAND OFF)
    target_compile_definitions(engine-core PUBLIC USE_WAYLAND=false)
endif()
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/libraries.cmake")

# set(SHADERS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders)
# set(SHADERS_BIN ${CMAKE_BINARY_DIR}/shaders)

# if(NOT EMSCRIPTEN)
#     message("Compiling Shaders")
#     bgfx_compile_shaders(
#         TYPE FRAGMENT
#         SHADERS "${SHADERS_SOURCE}/cubes.frag"
#         VARYING_DEF "${SHADERS_SOURCE}/varying.def.sc"
#         OUTPUT_DIR "${SHADERS_BIN}"
#     )
#     bgfx_compile_shaders(
#         TYPE VERTEX
#         SHADERS "${SHADERS_SOURCE}/cubes.vert"
#         VARYING_DEF "${SHADERS_SOURCE}/varying.def.sc"
#         OUTPUT_DIR "${SHADERS_BIN}"
#     )
#     message("Done!")
# endif()
if(BACKEND STREQUAL "VULKAN")
    set(SHADER_TYPE spirv)
elseif(BACKEND STREQUAL "OPEN_GL")
    set(SHADER_TYPE glsl)
elseif(BACKEND STREQUAL "OPEN_GL_ES")
    set(SHADER_TYPE essl)
else()
    message(FATAL_ERROR "Shader type not handled")
endif()
message("Shader type: ${SHADER_TYPE}")

if(LINUX)
    set(CMAKE_CXX_FLAGS
        "-Wall -Wextra -Wnon-virtual-dtor -Wold-style-cast -Wcast-align\
        -Wunused -Woverloaded-virtual -Wpedantic -Wconversion -Wsign-conversion\
        -Wnull-dereference -Wdouble-promotion -Wformat=2 -Wduplicated-cond\
        -Wduplicated-branches -Wlogical-op -Wuseless-cast"
)
endif()
# file(GLOB_RECURSE
#     MY_SOURCES CONFIGURE_DEPENDS
#      "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
# )
# file(GLOB_RECURSE SHADERS
#     CONFIGURE_DEPENDS
#     "${SHADERS_SOURCE}/*.frag"
#     "${SHADERS_SOURCE}/*.vert"
# )

if(EMSCRIPTEN)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}\
 --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/src/template.html"
  )
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}\
 --js-library ${CMAKE_CURRENT_SOURCE_DIR}/src/lib.js"
  )
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set_target_properties(
    ${CMAKE_PROJECT_NAME}
    PROPERTIES LINK_FLAGS
    "--preload-file ${CMAKE_SOURCE_DIR}/build/shaders/${SHADER_TYPE}@./resources/shaders/${SHADER_TYPE} --preload-file ../resources@./resources\
 --exclude-file ../resources/shaders/ -sALLOW_MEMORY_GROWTH"
  )
endif()
# target_compile_definitions(
#   ${CMAKE_PROJECT_NAME} PUBLIC $<$<CONFIG:Release>:PRODUCTION_BUILD>
#                                NAME="${CMAKE_PROJECT_NAME}"
# )
target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC BACKEND="${BACKEND}")

# set_target_properties(
#   "${CMAKE_PROJECT_NAME}"
#   PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/bin"
#              RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/bin/\
# ${CMAKE_PROJECT_NAME}_${CMAKE_PROJECT_VERSION}"
# )
# if(EMSCRIPTEN)
#     set_target_properties(
#     ${CMAKE_PROJECT_NAME}
#     PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE
#                "${CMAKE_CURRENT_SOURCE_DIR}/bin/\
# ${CMAKE_PROJECT_NAME}_${CMAKE_PROJECT_VERSION}_web"
#   )
# endif()

set(RELEASE_RESOURCE_DIR "${CMAKE_SOURCE_DIR}/bin/\
${CMAKE_PROJECT_NAME}_${CMAKE_PROJECT_VERSION}/resources/"
)
set(DEBUG_RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources/")
# target_compile_definitions(
#     engine-core
#     PUBLIC
#     RESOURCES_PATH=$<IF:$<CONFIG:Release>,"./resources/","${DEBUG_RESOURCE_DIR}">
# )
target_compile_definitions(
    engine-core
    PUBLIC
    SHADERS=$<IF:$<CONFIG:Release>,"./resources/shaders/${SHADER_TYPE}","${SHADERS_BIN}/${SHADER_TYPE}/">
)

target_compile_features(engine-core PRIVATE cxx_std_23)

# target_include_directories(
#     "${CMAKE_PROJECT_NAME}" PRIVATE
#     ${CMAKE_BINARY_DIR}/shaders
# )
# target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${MY_SOURCES})
# target_sources("${CMAKE_PROJECT_NAME}"
#     PRIVATE
#     src/imgui/imgui_impl_bgfx.cpp
#     src/camera.cpp
#     src/mesh.cpp
#     # src/program.cpp
#     src/quaternion.cpp
#     src/shader.cpp
#     src/vectorN.cpp
#     src/window.cpp
#     # src/main.cpp
# )
if(NOT EMSCRIPTEN)
    target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${SHADERS})
endif()

target_include_directories(
    engine-core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/"
)
target_include_directories(
  engine-core PUBLIC "${PROJECT_BINARY_DIR}"
)

if(NOT EMSCRIPTEN)
    add_custom_command(
    TARGET engine-core
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} # -E copy_directory
-DDEST_DIR=$<IF:$<CONFIG:Release>,"${RELEASE_RESOURCE_DIR}","${DEBUG_RESOURCE_DIR}">
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/copyResources.cmake
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      # "${CMAKE_CURRENT_SOURCE_DIR}/resources"
      # $<IF:$<CONFIG:Release>,"${RELEASE_RESOURCE_DIR}","${DEBUG_RESOURCE_DIR}">
  )
endif()
