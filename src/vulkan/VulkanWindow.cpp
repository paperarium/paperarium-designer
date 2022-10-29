#include "VulkanWindow.h"

namespace VulkanEngine {

VulkanWindow::VulkanWindow(QWidget* parent): QWindow() {

    // begin the render timer
    renderTimer.setInterval(16);
    QObject::connect(&renderTimer, &QTimer::timeout, this, &VulkanWindow::drawFrame);
    renderTimer.start();
}

void VulkanWindow::drawFrame() {
    Q_ASSERT(render != nullptr);
    render->drawFrame();
}

}
