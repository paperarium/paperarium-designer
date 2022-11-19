# from: https://stackoverflow.com/questions/57478571/why-wont-cmake-build-my-vulkan-spirv-shaders

# Find the glslc executable
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)

# Set shader 
set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)

# -- COMPILE SHADERS --
# file(GLOB SHADERS
#   ${SHADER_SOURCE_DIR}/shaders/*/glsl/*.vert
#   ${SHADER_SOURCE_DIR}/shaders/*/glsl/*.frag)

# # for each source in the shaders list
# foreach(source IN LISTS SHADERS)
#   get_filename_component(FILENAME ${source} NAME)
#   get_filename_component(INNERDIR ${source} DIRECTORY)
#   get_filename_component(FOLDER ${source} INNERDIR)
#   add_custom_command(
#     COMMAND
#     ${glslc_executable}
#     -o 
#   )
# endforeach()