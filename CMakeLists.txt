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

# --------------------------------- SOURCES --------------------------------- #

# Locate different types of files
file(GLOB PAPERARIUM_DESIGN_SRCS src/*.cpp)
file(GLOB PAPERARIUM_DESIGN_HEADERS include/*.h include/vulkanengine/*.h)
file(GLOB PAPERARIUM_DESIGN_UIS ui/*.ui)
file(GLOB PAPERARIUM_DESIGN_TSS lang/*.ts)
file(GLOB PAPERARIUM_DESIGN_RESOURCES resources/*/*.qrc)
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
find_package(Vulkan COMPONENTS glslc REQUIRED)      # Vulkan
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
