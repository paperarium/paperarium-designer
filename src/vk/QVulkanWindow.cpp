#include "QVulkanWindow.h"
#include "02_assimpmodel/AssimpModel.h"

QVulkanWindow::QVulkanWindow(QWidget* parent) : QWindow() {
  setSurfaceType(QSurface::VulkanSurface);
  m_vulkan = std::make_unique<VulkanEngine::AssimpModel>();
  m_vulkan->setWindow(winId());
}

QVulkanWindow::~QVulkanWindow() {
  m_vulkan->quit();
  m_vulkan->renderJoin();
}

void QVulkanWindow::mousePressEvent(QMouseEvent* event) {
  if (event->button() & Qt::LeftButton) {
    m_vulkan->setMouseButtonLeft(true);
  } else if (event->button() & Qt::RightButton) {
    m_vulkan->setMouseButtonRight(true);
  } else if (event->button() & Qt::MiddleButton) {
    m_vulkan->setMouseButtonMiddle(true);
  }
  m_vulkan->handleMouseMove(event->pos().x(), event->pos().y());
}

void QVulkanWindow::mouseReleaseEvent(QMouseEvent* event) {
  if (event->button() & Qt::LeftButton) {
    m_vulkan->setMouseButtonLeft(false);
  } else if (event->button() & Qt::RightButton) {
    m_vulkan->setMouseButtonRight(false);
  } else if (event->button() & Qt::MiddleButton) {
    m_vulkan->setMouseButtonMiddle(false);
  }
  m_vulkan->handleMouseMove(event->pos().x(), event->pos().y());
}

void QVulkanWindow::mouseMoveEvent(QMouseEvent* event) {
  m_vulkan->handleMouseMove(event->pos().x(), event->pos().y());
}

void QVulkanWindow::wheelEvent(QWheelEvent* event) {
  QPoint angleDelta = event->angleDelta();
  if (!angleDelta.isNull()) {
    m_vulkan->setMouseWheelScroll(static_cast<float>(angleDelta.y()));
  }
}

void QVulkanWindow::showEvent(QShowEvent* event) {
  if (!m_vulkan->getPrepared()) {
    m_vulkan->initVulkan();
    m_vulkan->prepare();
    m_vulkan->renderAsyncThread();
  }
}

void QVulkanWindow::resizeEvent(QResizeEvent* event) {
  m_vulkan->m_destWidth = width();
  m_vulkan->m_destHeight = height();
  QWindow::resizeEvent(event);
}
