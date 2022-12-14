#include "texture/VulkanTexture2D.h"
#include <stb_image.h>

namespace VulkanEngine {

void VulkanTexture2D::loadFromFile(std::string file, VkFormat format,
                                   vks::VulkanDevice* device, VkQueue copyQueue,
                                   VkImageUsageFlags imageUsageFlags,
                                   VkImageLayout imageLayout,
                                   bool forceLinear) {
  int w, h, c = 0;
  FILE* imgFile = fopen(file.c_str(), "rb");
  unsigned char* imgData = stbi_load_from_file(imgFile, &w, &h, &c, 0);
  this->device = device;
  width = static_cast<uint32_t>(w);
  height = static_cast<uint32_t>(h);
  channels = static_cast<uint32_t>(c);
  mipLevels = static_cast<uint32_t>(1);
  m_size = width * height * channels * mipLevels;

  unsigned char* newImgData = nullptr;
  bool newImgDataMalloc = false;

  if (channels == 3 && format == VK_FORMAT_R8G8B8A8_UNORM) {
    channels = 4;
    m_size = width * height * channels * mipLevels;
    newImgDataMalloc = true;
    newImgData = new uint8_t[m_size];
    for (int i = 0; i < width * height; i++) {
      newImgData[4 * i] = imgData[3 * i];
      newImgData[4 * i + 1] = imgData[3 * i + 1];
      newImgData[4 * i + 2] = imgData[3 * i + 2];
      newImgData[4 * i + 3] = 255;
    }
  } else {
    newImgData = imgData;
  }

  // get device properties for the requested texture format
  VkFormatProperties formatProperties;
  vkGetPhysicalDeviceFormatProperties(device->physicalDevice, format,
                                      &formatProperties);

  // only use linear tiling if requested (and supported by device)
  // support for linear tiling is mostly limited, so prefer to use optimal
  // tiling instead. on most implementations linear tiling will only support a
  // very limited amount of formats and features (mip maps, cubemaps, arrays..)
  VkBool32 useStaging = !forceLinear;

  VkMemoryAllocateInfo memAllocInfo = vks::initializers::memoryAllocateInfo();
  VkMemoryRequirements memReqs;

  // Use a separate command buffer for texture loading
  VkCommandBuffer copyCmd =
      device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
  // if we're staging our images on CPU and transferring to GPU
  if (useStaging) {
    // Create a host-visible staging buffer with the raw image data
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;
    VkBufferCreateInfo bufferCreateInfo = vks::initializers::bufferCreateInfo();
    bufferCreateInfo.size = m_size;
    // this buffer is used as a transfer source for the buffer copy
    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VK_CHECK_RESULT(vkCreateBuffer(device->logicalDevice, &bufferCreateInfo,
                                   nullptr, &stagingBuffer));

    // get memory requirements for the staging buffer (alignment, m type bits).
    // also get memory type index for a host visible buffer.
    vkGetBufferMemoryRequirements(device->logicalDevice, stagingBuffer,
                                  &memReqs);
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = device->getMemoryType(
        memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(device->logicalDevice, &memAllocInfo,
                                     nullptr, &stagingMemory));
    VK_CHECK_RESULT(vkBindBufferMemory(device->logicalDevice, stagingBuffer,
                                       stagingMemory, 0));

    // copy texture data into staging buffer
    uint8_t* data;
    VK_CHECK_RESULT(vkMapMemory(device->logicalDevice, stagingMemory, 0,
                                memReqs.size, 0, (void**)&data));
    memcpy(data, newImgData, m_size);
    vkUnmapMemory(device->logicalDevice, stagingMemory);

    // setup buffer copy regions for each mip level
    std::vector<VkBufferImageCopy> bufferCopyRegions;
    uint32_t offset = 0;
    for (uint32_t i = 0; i < mipLevels; i++) {
      VkBufferImageCopy bufferCopyRegion = {};
      bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      bufferCopyRegion.imageSubresource.mipLevel = i;
      bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
      bufferCopyRegion.imageSubresource.layerCount = 1;
      bufferCopyRegion.imageExtent.width = static_cast<uint32_t>(width);
      bufferCopyRegion.imageExtent.height = static_cast<uint32_t>(height);
      bufferCopyRegion.imageExtent.depth = 1;
      bufferCopyRegion.bufferOffset = offset;
      bufferCopyRegions.push_back(bufferCopyRegion);
      offset += static_cast<uint32_t>(m_size);
    }

    // create optimal tiled target image
    VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = format;
    imageCreateInfo.mipLevels = mipLevels;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.extent = {width, height, 1};
    imageCreateInfo.usage = imageUsageFlags;
    // ensure that TRANSFER_DST bit is set for staging
    if (!(imageCreateInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
      imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VK_CHECK_RESULT(vkCreateImage(device->logicalDevice, &imageCreateInfo,
                                  nullptr, &image));

    // allocate and bind memory for the target image
    vkGetImageMemoryRequirements(device->logicalDevice, image, &memReqs);
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = device->getMemoryType(
        memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(device->logicalDevice, &memAllocInfo,
                                     nullptr, &deviceMemory));
    VK_CHECK_RESULT(
        vkBindImageMemory(device->logicalDevice, image, deviceMemory, 0));

    // create image subresources for mip maps
    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = mipLevels;
    subresourceRange.layerCount = 1;

    // image barrier for optimal image (target)
    // optimal image will be used as destination for the copy
    vks::tools::setImageLayout(copyCmd, image, VK_IMAGE_LAYOUT_UNDEFINED,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               subresourceRange);

    // copy mip levels from staging buffer
    vkCmdCopyBufferToImage(copyCmd, stagingBuffer, image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           static_cast<uint32_t>(bufferCopyRegions.size()),
                           bufferCopyRegions.data());

    // change texture image layout to shader read after all mip levels have been
    // copied
    this->imageLayout = imageLayout;
    vks::tools::setImageLayout(copyCmd, image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               imageLayout, subresourceRange);

    // perform the commands
    device->flushCommandBuffer(copyCmd, copyQueue);

    // clean up staging resources. have been transferred to GPU now.
    vkFreeMemory(device->logicalDevice, stagingMemory, nullptr);
    vkDestroyBuffer(device->logicalDevice, stagingBuffer, nullptr);
    // if we're not staging, and keeping the image on CPU and mappiing to GPU
  } else {
    // Prefer using optimal tiling, as linear tiling
    // may support only a small set of features
    // depending on implementation (e.g. no mip maps, only one layer, etc.)

    // check if this support is supported for linear tiling
    assert(formatProperties.linearTilingFeatures *
           VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);

    VkImage mappableImage;
    VkDeviceMemory mappableMemory;

    VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = format;
    imageCreateInfo.extent = {width, height, 1};
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
    imageCreateInfo.usage = imageUsageFlags;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // load mip map level 0 to linear tiling image
    VK_CHECK_RESULT(vkCreateImage(device->logicalDevice, &imageCreateInfo,
                                  nullptr, &mappableImage));

    // get memory requirements for image, like size and alignment
    vkGetImageMemoryRequirements(device->logicalDevice, mappableImage,
                                 &memReqs);
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = device->getMemoryType(
        memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // allocate and bind host memory
    VK_CHECK_RESULT(vkAllocateMemory(device->logicalDevice, &memAllocInfo,
                                     nullptr, &mappableMemory));
    VK_CHECK_RESULT(vkBindImageMemory(device->logicalDevice, mappableImage,
                                      mappableMemory, 0));

    // get sub resource layout, i.e. mip map count, array layer...
    VkImageSubresource subRes = {};
    subRes.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subRes.mipLevel = 0;
    VkSubresourceLayout subResLayout;
    void* data;
    vkGetImageSubresourceLayout(device->logicalDevice, mappableImage, &subRes,
                                &subResLayout);

    // map image memory and copy into it
    VK_CHECK_RESULT(vkMapMemory(device->logicalDevice, mappableMemory, 0,
                                memReqs.size, 0, &data));
    memcpy(data, newImgData, m_size);
    vkUnmapMemory(device->logicalDevice, mappableMemory);

    // linear tiled images don't need to be staged
    image = mappableImage;
    deviceMemory = mappableMemory;
    this->imageLayout = imageLayout;

    // set up image memory barrier
    vks::tools::setImageLayout(copyCmd, image, VK_IMAGE_ASPECT_COLOR_BIT,
                               VK_IMAGE_LAYOUT_UNDEFINED, imageLayout);
    device->flushCommandBuffer(copyCmd, copyQueue);
  }

  // create a default sampler
  VkSamplerCreateInfo samplerCreateInfo = {};
  samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
  samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
  samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerCreateInfo.mipLodBias = 0.0f;
  samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
  samplerCreateInfo.minLod = 0.0f;
  // max level-of-detail should match mip level count
  samplerCreateInfo.maxLod = (useStaging) ? (float)mipLevels : 0.0f;
  // only enable anisotropic filtering if enabled on the device
  samplerCreateInfo.maxAnisotropy =
      device->enabledFeatures.samplerAnisotropy
          ? device->properties.limits.maxSamplerAnisotropy
          : 1.0f;
  samplerCreateInfo.maxAnisotropy = device->enabledFeatures.samplerAnisotropy;
  samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
  VK_CHECK_RESULT(vkCreateSampler(device->logicalDevice, &samplerCreateInfo,
                                  nullptr, &sampler));

  // create the image view itself
  // textures are not directly accessed by the shaders and are abstracted by
  // image views containing additional information and sub resource ranges
  VkImageViewCreateInfo viewCreateInfo = {};
  viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewCreateInfo.format = format;
  viewCreateInfo.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
                               VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
  viewCreateInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
  // linear tiling usually won't support mip maps, only set mip map count if
  // optimal tiling is used
  viewCreateInfo.subresourceRange.levelCount = (useStaging) ? mipLevels : 1;
  viewCreateInfo.image = image;
  VK_CHECK_RESULT(vkCreateImageView(device->logicalDevice, &viewCreateInfo,
                                    nullptr, &view));

  // update descriptor image info member to be be used for descriptor sets
  updateDescriptor();
  // free the STB image data
  stbi_image_free(imgData);
  fclose(imgFile);

  // delete any new mallocs
  if (newImgDataMalloc) {
    delete[] newImgData;
  }
}

}  // namespace VulkanEngine