#include "VulkanBuffer.h"

namespace VulkanEngine {

VulkanBuffer::~VulkanBuffer() {
  m_uniformBuffer.unmap();
  m_uniformBuffer.destroy();
}

void VulkanBuffer::prepare() {
  prepareUniformBuffers();
}

void VulkanBuffer::update() {
  updateUniformBuffers();
}

}