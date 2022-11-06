#include "VulkanQtTools.h"

namespace VulkanEngine::QtTools {


/**
 * @brief Reads a Qt resource binary into a normal char array
 * @param resourcePath
 * @param device
 * @return
 */
QByteArray readResource(const char *resourcePath) {
    QFile resourceFile(resourcePath);
    resourceFile.open(QFile::ReadOnly);
    return resourceFile.readAll();
}

}
