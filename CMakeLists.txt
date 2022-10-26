# Use recommended CMake version and initialize project
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
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools OpenGL OpenGLWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools OpenGL OpenGLWidgets)

# --------------------------------- SOURCES --------------------------------- #

# Locate different types of files
file(GLOB PAPERARIUM_DESIGN_SRCS src/*.cpp)
file(GLOB PAPERARIUM_DESIGN_HEADERS include/*.h)
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

# -- assimp
find_package(assimp REQUIRED)
if(ASSIMP_FOUND)
    target_include_directories(PaperariumDesign ${ASSIMP_LIBRARIES})
endif()

# local includes are all in the include/ folder
target_include_directories(PaperariumDesign PRIVATE include)

# ----------------------------- LINK LIBRARIES ------------------------------ #

# link all our libraries
target_link_libraries(
    PaperariumDesign
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::OpenGL
    Qt${QT_VERSION_MAJOR}::OpenGLWidgets
    ${ASSIMP_LIBRARIES})

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
