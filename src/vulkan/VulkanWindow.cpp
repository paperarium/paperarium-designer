#include "VulkanWindow.h"

VulkanWindow::VulkanWindow(): QWindow() {

    // begin the render timer
    renderTimer.setInterval(16);
    QObject::connect(&renderTimer, &QTimer::timeout, this, &VKWindow::drawFrame);
    renderTimer.start();
}

void VKWindow::drawFrame() {
    Q_ASSERT(render != nullptr);
    render->drawFrame();
}
