#include "QVulkanWindow.h"

QVulkanWindow::QVulkanWindow(QWidget* parent): QWindow() {
  setSurfaceType(QSurface::VulkanSurface);
}

QVulkanWindow::~QVulkanWindow() {}

void QVulkanWindow::setVulkanPtr(VulkanEngine::VulkanBaseEngine *vulkan) {
  m_vulkan = vulkan;
  m_vulkan->setWindow(winId());
}

void QVulkanWindow::mousePressEvent(QMouseEvent *event) {
  if (!m_vulkan) return;
  if (event->button() & Qt::LeftButton) {
    m_vulkan->setMouseButtonLeft(true);
  } else if (event->button() & Qt::RightButton) {
    m_vulkan->setMouseButtonRight(true);
  } else if (event->button() & Qt::MiddleButton) {
    m_vulkan->setMouseButtonMiddle(true);
  }
  m_vulkan->handleMouseMove(event->pos().x(), event->pos().y());
}

void QVulkanWindow::mouseReleaseEvent(QMouseEvent *event) {
  if (!m_vulkan) return;
  if (event->button() & Qt::LeftButton) {
    m_vulkan->setMouseButtonLeft(false);
  } else if (event->button() & Qt::RightButton) {
    m_vulkan->setMouseButtonRight(false);
  } else if (event->button() & Qt::MiddleButton) {
    m_vulkan->setMouseButtonMiddle(false);
  }
  m_vulkan->handleMouseMove(event->pos().x(), event->pos().y());
}

void QVulkanWindow::mouseMoveEvent(QMouseEvent *event) {
  if (m_vulkan) {
    m_vulkan->handleMouseMove(event->pos().x(), event->pos().y());
  }
}

void QVulkanWindow::showEvent(QShowEvent *event) {
  if (m_vulkan && !m_vulkan->getPrepared()) {
// #if defined(VK_USE_PLATFORM_XCB_KHR)
//     m_vulkan->initxcbConnection();
// #endif
    m_vulkan->initVulkan();
    // m_vulkan->setWindow(uint32_t(winId()));
    m_vulkan->prepare();
    m_vulkan->renderAsyncThread();
  }
}

void QVulkanWindow::closeEvent(QCloseEvent *event) {
  if (m_vulkan) {
    m_vulkan->quit();
    m_vulkan->renderJoin();
  }
}
