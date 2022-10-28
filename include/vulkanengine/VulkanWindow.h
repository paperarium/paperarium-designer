#ifndef VKWINDOW_H
#define VKWINDOW_H

#include <QResizeEvent>
#include <QWindow>
#include <QTimer>

namespace VulkanEngine {

class VulkanWindow : public QWindow {
Q_OBJECT
public:
    VulkanWindow(QWidget* parent = Q_NULLPTR);
    virtual ~VulkanWindow();
    void setVulkanPtr(VulkanEngine::VulkanBaseEngine *vulkan);

    void resize(int width, int height);
    void drawFrame();

public slots:

    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

protected:
    void resizeEvent(QResizeEvent* event);

    QTimer renderTimer;

    uint32_t windowWidth = 800;
    uint32_t windowHeight = 600;
};

}

#endif // VKWINDOW_H
