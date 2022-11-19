#include "ThirdPersonEngine.h"

namespace VulkanEngine {

/**
 * @brief Update the camera based on mouse input
 */
void ThirdPersonEngine::updateCamera() {
  if (m_mouseButtons.left) {
    if (m_mousePos.x > m_mousePosOld.x) {
      m_camera.m_rotation.y += 1.f;
    } else if (m_mousePos.x < m_mousePosOld.y) {
      m_camera.m_rotation.y -= 1.f;
    }
    if (m_mousePos.y > m_mousePosOld.y) {
      m_camera.m_rotation.x += 1.f;
    } else if (m_mousePos.y < m_mousePosOld.y) {
      m_camera.m_rotation.x -= 1.f;
    }
  }
  if (m_scroll.up)
    m_distance += 0.1f;
  else if (m_scroll.down)
    m_distance -= 0.1f;
  m_camera.m_zoom += m_distance;
  m_distance = 0.f;
  m_mousePosOld = m_mousePos;
}

}  // namespace VulkanEngine