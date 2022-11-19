#include "ThirdPersonEngine.h"
#include <QtDebug>

namespace VulkanEngine {

/**
 * @brief Update the camera based on mouse input
 */
void ThirdPersonEngine::updateCamera() {
  if (m_mouseButtons.left) {
    m_camera.m_rotation.y +=
        (m_mousePos.x - m_mousePosOld.x) / m_viewportSensitivity;
    m_camera.m_rotation.x +=
        (m_mousePos.y - m_mousePosOld.y) / m_viewportSensitivity;
  }
  m_distance += m_scroll / m_scrollSensitivity;
  m_camera.m_zoom += m_distance;
  m_distance = 0.f;
  m_mousePosOld = m_mousePos;
}

}  // namespace VulkanEngine