# Use recommended CMake version and initialize project
# set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/lib/vcpkg/scripts/buildsystems/vcpkg.cmake)
cmake_minimum_required(VERSION 3.5)
project(PaperariumDesign VERSION 0.1 LANGUAGES CXX)

# Set CMake variables
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Turn on Qt-related auto compiler options
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ui/)
set(AUTOMOC_PATH_PREFIX ON)

# Include Qt packages
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Gui)
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
# Apple / macOS
elseif(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_MACOS_MVK -DVK_EXAMPLE_XCODE_GENERATED")
endif(WIN32)

# Set preprocessor defines
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX -D_USE_MATH_DEFINES")

# Clang specific stuff
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-switch-enum")
endif()

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

# --------------------------------- SOURCES --------------------------------- #

# Locate different types of files
set(PAPERARIUM_DESIGN_SRCS
    src/mainwindow.cpp
    src/vulkan/VulkanBase.cpp
    src/vulkan/VulkanWindow.cpp
)
set(PAPERARIUM_DESIGN_HEADERS
    include/mainwindow.h
    include/vulkanengine/render_common.h
    include/vulkanengine/vulkan_macro.h
    include/vulkanengine/VulkanContext.h
    include/vulkanengine/VulkanBase.h
    include/vulkanengine/VulkanObject.h
    include/vulkanengine/VulkanWindow.h
    include/vulkanengine/utils/keycodes.hpp
    include/vulkanengine/utils/VulkanAndroid.h
    include/vulkanengine/utils/VulkanBuffer.hpp
    include/vulkanengine/utils/VulkanDebug.h
    include/vulkanengine/utils/VulkanDevice.hpp
    include/vulkanengine/utils/VulkanInitializers.hpp
    include/vulkanengine/utils/VulkanSwapChain.hpp
    include/vulkanengine/utils/VulkanTools.h
)
set(PAPERARIUM_DESIGN_UIS
    ui/mainwindow.ui
)
set(PAPERARIUM_DESIGN_TSS
    lang/PaperariumDesign_en_US.ts
)
set(PAPERARIUM_DESIGN_RESOURCES)
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
    include/vulkanengine
    include/vulkanengine/utils
    ${Vulkan_INCLUDE_DIRS}
    ${ASSIMP_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    ${OPENMP_INCLUDE_DIRS})

# ----------------------------- LINK LIBRARIES ------------------------------ #

# link all our libraries
target_link_libraries(
    PaperariumDesign
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Gui
    ${ASSIMP_LIBRARIES}
    ${Vulkan_LIBRARIES}
    ${GLM_LIBRARIES}
    ${OPENMP_LIBRARIES})

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
