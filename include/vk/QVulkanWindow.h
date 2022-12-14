#ifndef QVULKAN_WINDOW_H
#define QVULKAN_WINDOW_H

#include "VulkanBaseEngine.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QWindow>

class QVulkanWindow : public QWindow {
  Q_OBJECT
 public:
  QVulkanWindow(QWidget* parent = Q_NULLPTR);
  virtual ~QVulkanWindow();

 protected:
  // event listeners we will override
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;
  void showEvent(QShowEvent* event) override;
  // void closeEvent(QCloseEvent *event) override;
  void resizeEvent(QResizeEvent* event) override;

  std::unique_ptr<VulkanEngine::VulkanBaseEngine> m_vulkan;
  glm::vec2* m_mousePos = nullptr;
};

#endif  // QVULKAN_WINDOW_H
