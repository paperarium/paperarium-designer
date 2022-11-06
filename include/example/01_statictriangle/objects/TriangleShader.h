#ifndef TRIANGLE_SHADER_H
#define TRIANGLE_SHADER_H

#include "VulkanShader.h"

namespace VulkanEngine {

class TriangleShader : public VulkanShader {
public:

  TriangleShader() = default;
  ~TriangleShader() = default;
  virtual void prepareShaders() override;
};

}

#endif /* TRIANGLE_SHADER_H */
