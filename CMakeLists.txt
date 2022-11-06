# Use recommended CMake version and initialize project
# set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/lib/vcpkg/scripts/buildsystems/vcpkg.cmake)
cmake_minimum_required(VERSION 3.5)
project(PaperariumDesign VERSION 0.1 LANGUAGES CXX)

# Set CMake variables
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Turn on Qt-related auto compiler options
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ui/)
set(AUTOMOC_PATH_PREFIX ON)

# Add some definitions
add_definitions(-DSTBI_NO_DDS)
add_definitions(-DUSE_STD_FILESYSTEM)  # For Old Compilers as GCC 7 change to -DUSE_STD_EXPERIMENTAL_FILESYSTEM
add_definitions(-D_USE_MATH_DEFINES)
add_definitions(-DPROJECT_ABSOLUTE_PATH="${PROJECT_SOURCE_DIR}")

# Include Qt packages
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Gui)
# Include ImGui for GUIs in Vulkan
find_package(imgui REQUIRED)
# Include Vulkan so we can determine how to bundle
find_package(Vulkan COMPONENTS glslc REQUIRED)

# ------------------------ ADD COMPILER DEFINITIONS -------------------------- #

# Set target definitions
if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

# Windows
set(ADDITIONAL_LIBRARIES)
set(ADDITIONAL_INCLUDE_DIRS)
if(WIN32)
    if (NOT Vulkan_FOUND)
        find_library(Vulkan_LIBRARY NAMES vulkan-1 vulkan PATHS ${CMAKE_SOURCE_DIR}/libs/vulkan)
        if (Vulkan_LIBRARY)
            set(Vulkan_FOUND ON)
            message("Using bundled Vulkan library version")
        endif()
    endif()
# Linux
elseif(LINUX)
    if (NOT Vulkan_FOUND)
        find_library(Vulkan_LIBRARY NAMES vulkan HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
        if (Vulkan_LIBRARY)
            set(Vulkan_FOUND ON)
            message("Using bundled Vulkan library version")
        endif()
    endif()
    find_package(Threads REQUIRED)
    if (USE_D2D_WSI)
        message("Using direct to display extension...")
        add_definitions(-D_DIRECT2DISPLAY)
    elseif(USE_DIRECTFB_WSI)
        find_package(DirectFB REQUIRED)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_DIRECTFB_EXT")
        set(ADDITIONAL_INCLUDE_DIRS
            ${ADDITIONAL_INCLUDE_DIRS}
            ${DIRECTFB_INCLUDE_DIR})
    elseif(USE_WAYLAND_WSI)
        find_program(PKG_CONFIG pkg-config)
        if (NOT PKG_CONFIG)
            message(FATAL_ERROR "pkg-config binary not found")
        endif()
        find_package(Wayland REQUIRED)
        if (NOT WAYLAND_FOUND)
            message(FATAL_ERROR "Wayland development package not found")
        endif()
        pkg_check_modules(WAYLAND_PROTOCOLS REQUIRED wayland-protocols)
        if (NOT WAYLAND_PROTOCOLS_FOUND)
            message(FATAL_ERROR "Wayland protocols package not found")
        endif()
        find_program(WAYLAND_SCANNER wayland-scanner)
        if(NOT WAYLAND_SCANNER)
            message(FATAL_ERROR "wayland-scanner binary not found")
        endif()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WAYLAND_KHR")
        set(ADDITIONAL_INCLUDE_DIRS
            ${ADDITIONAL_INCLUDE_DIRS}
            ${WAYLAND_INCLUDE_DIR})
        pkg_get_variable(protocol_dir wayland-protocols pkgdatadir)
        execute_process(COMMAND ${WAYLAND_SCANNER} client-header ${protocol_dir}/stable/xdg-shell/xdg-shell.xml ${CMAKE_BINARY_DIR}/xdg-shell-client-protocol.h
                        COMMAND ${WAYLAND_SCANNER} private-code ${protocol_dir}/stable/xdg-shell/xdg-shell.xml ${CMAKE_BINARY_DIR}/xdg-shell-protocol.c)
        set(ADDITIONAL_INCLUDE_DIRS
            ${ADDITIONAL_INCLUDE_DIRS}
            ${CMAKE_BINARY_DIR})
    elseif(USE_HEADLESS)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_HEADLESS_EXT")
    else(USE_D2D_WSI)
        find_package(XCB REQUIRED)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_XCB_KHR")
    endif(USE_D2D_WSI)
    set(ADDITIONAL_LIBRARIES ${XCB_LIBRARIES} ${DIRECTFB_LIBRARIES} ${WAYLAND_CLIENT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
# Apple / macOS
elseif(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_MACOS_MVK -DVK_EXAMPLE_XCODE_GENERATED")
    # set(ADDITIONAL_LIBRARIES ${ADDITIONAL_LIBRARIES} "-framework AppKit" "-framework QuartzCore")
endif(WIN32)

# Set preprocessor defines
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX -D_USE_MATH_DEFINES")

# Clang specific stuff
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-switch-enum")
endif()

add_definitions(-D_CRT_SECURE_NO_WARNINGS)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler specific stuff
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
elseif (APPLE)
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-arc -xobjective-c++")
endif (MSVC)

# find XCB on Linux systems
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    message("Linux build needs libxcb-dev.")
    include(FindPkgConfig)
    pkg_check_modules(XCB xcb)
    if (XCB_FOUND)
        message("Found XCB ${XCB_LDFLAGS}")
    else()
        message("You may need to install libxcb-dev!")
    endif()
endif()

# --------------------------------- SOURCES --------------------------------- #

# Locate different types of files
set(PAPERARIUM_DESIGN_SRCS
    src/example/01_statictriangle/StaticTriangle.cpp
    src/example/01_statictriangle/objects/Triangle.cpp
    src/example/01_statictriangle/objects/TriangleShader.cpp
    src/example/01_statictriangle/objects/TriangleUniform.cpp
    src/vulkan/core/VulkanDescriptorSet.cpp
    src/vulkan/core/VulkanPipelines.cpp
    src/vulkan/template/camera/UniformCamera.cpp
    src/vulkan/template/mesh/MeshObject.cpp
    src/vulkan/template/VulkanBuffer.cpp
    src/vulkan/template/VulkanShader.cpp
    src/vulkan/utils/VulkanUIOverlay.cpp
    src/vulkan/utils/VulkanTools.cpp
    src/vulkan/utils/VulkanQtTools.cpp
    src/vulkan/VulkanBase.cpp
    src/vulkan/VulkanBaseEngine.cpp
    src/vulkan/QVulkanWindow.cpp
    src/mainwindow.cpp
)

set(PAPERARIUM_DESIGN_HEADERS
    include/example/01_statictriangle/StaticTriangle.h
    include/example/01_statictriangle/objects/Triangle.h
    include/example/01_statictriangle/objects/TriangleShader.h
    include/example/01_statictriangle/objects/TriangleUniform.h
    include/vulkanengine/common/base_template.h
    include/vulkanengine/common/render_common.h
    include/vulkanengine/common/vertex_struct.h
    include/vulkanengine/common/vulkan_macro.h
    include/vulkanengine/core/VulkanContext.h
    include/vulkanengine/core/VulkanDescriptorSet.h
    include/vulkanengine/core/VulkanPipelines.h
    include/vulkanengine/core/VulkanVertexDescriptions.h
    include/vulkanengine/template/camera/UniformCamera.h
    include/vulkanengine/template/camera/ThirdPersonCamera.h
    include/vulkanengine/template/mesh/MeshObject.h
    include/vulkanengine/template/VkObject.h
    include/vulkanengine/template/VulkanBuffer.h
    include/vulkanengine/template/VulkanShader.h
    include/vulkanengine/utils/keycodes.hpp
    include/vulkanengine/utils/VulkanAndroid.h
    include/vulkanengine/utils/VulkanBuffer.hpp
    include/vulkanengine/utils/VulkanDebug.h
    include/vulkanengine/utils/VulkanDevice.hpp
    include/vulkanengine/utils/VulkanInitializers.hpp
    include/vulkanengine/utils/VulkanSwapChain.hpp
    include/vulkanengine/utils/VulkanTools.h
    include/vulkanengine/utils/VulkanQtTools.h
    include/vulkanengine/utils/VulkanUIOverlay.h
    include/vulkanengine/VulkanBase.h
    include/vulkanengine/VulkanBaseEngine.h
    include/vulkanengine/QVulkanWindow.h
    include/mainwindow.h
)
set(PAPERARIUM_DESIGN_UIS
    ui/mainwindow.ui
)
set(PAPERARIUM_DESIGN_TSS
    lang/PaperariumDesign_en_US.ts
)
set(PAPERARIUM_DESIGN_RESOURCES
    resources/shaders/shaders.qrc
    resources/fonts/fonts.qrc)
# Concatenate all into project sources
set(PAPERARIUM_DESIGN_ALL_SOURCES
    main.cpp
    ${PAPERARIUM_DESIGN_SRCS}
    ${PAPERARIUM_DESIGN_HEADERS}
    ${PAPERARIUM_DESIGN_UIS}
    ${PAPERARIUM_DESIGN_TSS}
    ${PAPERARIUM_DESIGN_RESOURCES})

# ------------------------------- EXECUTION -------------------------------- #

# Add executable as PaperariumDesign
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(PaperariumDesign MANUAL_FINALIZATION ${PAPERARIUM_DESIGN_ALL_SOURCES})
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${PAPERARIUM_DESIGN_TSS})
else()
    add_executable(PaperariumDesign ${PAPERARIUM_DESIGN_ALL_SOURCES})
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${PAPERARIUM_DESIGN_TSS})
endif()

# --------------------------- INCLUDE LIBRARIES ---------------------------- #

# Locate all the required packages
find_package(ASSIMP REQUIRED)                       # ASSIMP
find_package(GLM REQUIRED)                          # GLM
find_package(OpenMP REQUIRED)                       # OpenMP
if (OPENMP_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

# local includes are all in the include/ folder
target_include_directories(
    PaperariumDesign
    PRIVATE
    include
    include/example
    include/vulkanengine
    include/vulkanengine/common
    include/vulkanengine/core
    include/vulkanengine/template
    include/vulkanengine/utils
    ${Vulkan_INCLUDE_DIRS}
    ${ASSIMP_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    ${OPENMP_INCLUDE_DIRS}
    ${IMGUI_INCLUDE_DIRS}
    ${ADDITIONAL_INCLUDE_DIRS})

# ----------------------------- LINK LIBRARIES ------------------------------ #

# link all our libraries
target_link_libraries(
    PaperariumDesign
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Gui
    imgui::imgui
    ${ASSIMP_LIBRARIES}
    ${Vulkan_LIBRARIES}
    ${GLM_LIBRARIES}
    ${OPENMP_LIBRARIES}
    ${ADDITIONAL_LIBRARIES})

# ---------------------------- BUNDLE BUILD + INSTALL ----------------------- #

# Define the target's properties
set_target_properties(PaperariumDesign PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Install libraries
install(TARGETS PaperariumDesign BUNDLE DESTINATION . LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# Finalize executable if necessary
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(PaperariumDesign)
endif()
