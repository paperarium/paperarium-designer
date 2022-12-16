# Use recommended CMake version and initialize project
# set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/lib/vcpkg/scripts/buildsystems/vcpkg.cmake)
cmake_minimum_required(VERSION 3.5)
project(PaperariumDesign VERSION 0.1 LANGUAGES CXX)

# Add resource subdirectories

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
add_definitions(-DSTB_IMAGE_IMPLEMENTATION)
add_definitions(-DUSE_STD_FILESYSTEM)  # For Old Compilers as GCC 7 change to -DUSE_STD_EXPERIMENTAL_FILESYSTEM
add_definitions(-D_USE_MATH_DEFINES)
add_definitions(-DPROJECT_ABSOLUTE_PATH="${PROJECT_SOURCE_DIR}")

# Include Qt packages
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Gui)
# Include ImGui for GUIs in Vulkan
find_package(imgui REQUIRED)
# Include Vulkan so we can determine how to bundle
find_package(Vulkan REQUIRED COMPONENTS glslc)
# Grab the STB image aug header
find_path(STB_INCLUDE_DIRS "stb_image.h")

# Add the resources directory for compiling shaders
add_subdirectory(resources)

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
set(CMAKE_CXX_STANDARD 17)
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

# header files
set(PAPERARIUM_DESIGN_HEADERS
    include/example/01_statictriangle/StaticTriangle.h
    include/example/01_statictriangle/objects/Triangle.h
    include/example/01_statictriangle/objects/TriangleShader.h
    include/example/01_statictriangle/objects/TriangleUniform.h
    include/example/02_assimpmodel/AssimpModel.h
    include/example/ThirdPersonEngine.h
    include/vk/common/base_template.h
    include/vk/common/render_common.h
    include/vk/common/vertex_struct.h
    include/vk/common/vulkan_macro.h
    include/vk/template/camera/ShadowCamera.h
    include/vk/template/camera/ThirdPersonCamera.h
    include/vk/template/camera/UniformCamera.h
    include/vk/template/mesh/AssimpObject.h
    include/vk/template/mesh/MeshObject.h
    include/vk/template/mesh/VulkanCube.h
    include/vk/template/mesh/VulkanPlane.h
    include/vk/template/texture/VulkanTexture.h
    include/vk/template/texture/VulkanTexture2D.h
    include/vk/utils/keycodes.hpp
    include/vk/utils/VulkanAndroid.h
    include/vk/utils/VulkanBuffer.hpp
    include/vk/utils/VulkanDebug.h
    include/vk/utils/VulkanDevice.hpp
    include/vk/utils/VulkanInitializers.hpp
    include/vk/utils/VulkanSwapChain.h
    include/vk/utils/VulkanTools.h
    include/vk/utils/VulkanQtTools.h
    include/vk/utils/VulkanUIOverlay.h
    include/vk/QVulkanWindow.h
    include/vk/VkObject.h
    include/vk/VulkanBase.h
    include/vk/VulkanBaseEngine.h
    include/vk/VulkanBuffer.h
    include/vk/VulkanContext.h
    include/vk/VulkanDescriptorSet.h
    include/vk/VulkanFrameBuffer.h
    include/vk/VulkanPipelines.h
    include/vk/VulkanRenderPass.h
    include/vk/VulkanShader.h
    include/vk/VulkanVertexDescriptions.h
    include/mainwindow.h
    ${STB_INCLUDE_DIRS}
)

# source files
set(PAPERARIUM_DESIGN_SRCS
    src/example/01_statictriangle/StaticTriangle.cpp
    src/example/01_statictriangle/objects/Triangle.cpp
    src/example/01_statictriangle/objects/TriangleShader.cpp
    src/example/01_statictriangle/objects/TriangleUniform.cpp
    src/example/02_assimpmodel/AssimpModel.cpp
    src/example/ThirdPersonEngine.cpp
    src/vk/template/camera/ShadowCamera.cpp
    src/vk/template/camera/UniformCamera.cpp
    src/vk/template/mesh/AssimpObject.cpp
    src/vk/template/mesh/MeshObject.cpp
    src/vk/template/mesh/VulkanCube.cpp
    src/vk/template/mesh/VulkanPlane.cpp
    src/vk/template/texture/VulkanTexture2D.cpp
    src/vk/QVulkanWindow.cpp
    src/vk/VulkanBase.cpp
    src/vk/VulkanBaseEngine.cpp
    src/vk/VulkanBuffer.cpp
    src/vk/VulkanDescriptorSet.cpp
    src/vk/VulkanFrameBuffer.cpp
    src/vk/VulkanPipelines.cpp
    src/vk/VulkanQtTools.cpp
    src/vk/VulkanRenderPass.cpp
    src/vk/VulkanShader.cpp
    src/vk/VulkanSwapChain.cpp
    src/vk/VulkanTools.cpp
    src/vk/VulkanUIOverlay.cpp
    src/mainwindow.cpp
)

# Qt UI files
set(PAPERARIUM_DESIGN_UIS
    ui/mainwindow.ui
)

# localization files
set(PAPERARIUM_DESIGN_TSS
    lang/PaperariumDesign_en_US.ts
)

# resource files
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

# local includes are all in the include/ folder
target_include_directories(
    PaperariumDesign
    PRIVATE
    include
    include/example
    include/vk
    include/vk/common
    include/vk/template
    include/vk/utils
    ${Vulkan_INCLUDE_DIRS}
    ${ASSIMP_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    ${IMGUI_INCLUDE_DIRS}
    ${ADDITIONAL_INCLUDE_DIRS}
    ${STB_INCLUDE_DIRS})

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
    ${ADDITIONAL_LIBRARIES})

# ---------------------------- BUNDLE BUILD + INSTALL ----------------------- #

# Define the target's properties
set_target_properties(PaperariumDesign PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER design.paperararium.place
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
