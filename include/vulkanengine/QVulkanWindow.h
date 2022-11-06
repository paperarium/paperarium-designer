#ifndef QVULKAN_WINDOW_H
#define QVULKAN_WINDOW_H

#include <QWindow>
#include <QMouseEvent>
#include "VulkanBaseEngine.h"

class QVulkanWindow : public QWindow {
Q_OBJECT
public:
    QVulkanWindow(QWidget* parent = Q_NULLPTR);
    virtual ~QVulkanWindow();
    void setVulkanPtr(VulkanEngine::VulkanBaseEngine *vulkan);

public slots:

    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void showEvent(QShowEvent* event);
    void closeEvent(QCloseEvent *event);

protected:
    VulkanEngine::VulkanBaseEngine *m_vulkan = nullptr;
    glm::vec2 *m_mousePos = nullptr;
};

#endif // QVULKAN_WINDOW_H
