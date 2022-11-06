#include "01_statictriangle/objects/TriangleShader.h"

namespace VulkanEngine {

/**
 * @brief Load the static triangle shader
 */
void TriangleShader::prepareShaders() {
  m_shaderStages.push_back(loadShader(":/shaders/01_statictriangle/statictriangle.vert.spv", VK_SHADER_STAGE_VERTEX_BIT));
  m_shaderStages.push_back(loadShader(":/shaders/01_statictriangle/statictriangle.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT));
}

}
