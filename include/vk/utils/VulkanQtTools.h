#ifndef VULKANQTTOOLS_H
#define VULKANQTTOOLS_H

#include <QResource>
#include <QFile>
#include <stdbool.h>
#include <string>

namespace VulkanEngine::QtTools {

QByteArray readResource(const char *resourcePath);

}

#endif // VULKANQTTOOLS_H
