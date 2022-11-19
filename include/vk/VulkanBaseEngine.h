#ifndef VULKAN_BASE_ENGINE_H
#define VULKAN_BASE_ENGINE_H

#include "VulkanBase.h"
#include "VulkanContext.h"
#include "VulkanDescriptorSet.h"
#include "VulkanPipelines.h"
#include "VulkanUIOverlay.h"
#include "VulkanVertexDescriptions.h"

namespace VulkanEngine {

class VULKANENGINE_EXPORT_API VulkanBaseEngine : public VulkanBase {
 public:
  VulkanBaseEngine() = default;
  virtual ~VulkanBaseEngine();

  virtual void initWindow();
  virtual void prepare() override;
  virtual void render() override;
  virtual void updateOverlay() override;
  virtual void drawUI(const VkCommandBuffer commandBuffer);
  virtual void OnUpdateUIOverlay(vks::UIOverlay* overlay){};
  virtual void processPrepareCallback(){};
  virtual void updateCommand() override;

  void renderAsyncThread();
  void renderJoin();

 protected:
  void prepareImGui();
  void prepareDescriptorSets();
  void prepareVertexDescriptions();
  void prepareBasePipelines();
  void prepareContext();

  virtual void prepareMyObjects(){};
  virtual void buildCommandBuffersBeforeMainRenderPass(VkCommandBuffer& cmd){};
  virtual void buildCommandBuffers() override;
  virtual void buildCommandBuffersAfterMainRenderPass(VkCommandBuffer& cmd){};
  virtual void setViewPorts(VkCommandBuffer& cmd);
  virtual void buildMyObjects(VkCommandBuffer& cmd){};

  template <class T>
  void REGISTER_OBJECT(std::shared_ptr<T>& obj) {
    obj = VkObject::New<T>(m_context);
    m_objs.emplace_back(obj);
  }

  void destroyObjects() {
    for (auto& obj : m_objs) {
      obj = nullptr;
    }
  }

  VkPipelineShaderStageCreateInfo loadShader(
      std::string const& fileName, VkShaderStageFlagBits const& stage);

 protected:
  int m_maxSets = 1;
  struct Settings {
    bool overlay = true;
  } m_settings;
  bool m_rebuild = false;

  std::vector<std::shared_ptr<VkObject>> m_objs;
  std::thread* m_thread = nullptr;

  vks::UIOverlay m_UIOverlay;

  VulkanDescriptorSet* m_vulkanDescriptorSet = nullptr;
  VulkanVertexDescriptions* m_vulkanVertexDescriptions = nullptr;
  VulkanPipelines* m_pipelines = nullptr;
  VulkanContext* m_context = nullptr;
  VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
};

}  // namespace VulkanEngine

#endif  // VULKAN_BASE_ENGINE_H
