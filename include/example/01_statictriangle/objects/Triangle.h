#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "mesh/MeshObject.h"
#include "vertex_struct.h"

namespace VulkanEngine {

class Triangle: public MeshObject {
public:

  Triangle() = default;
  virtual ~Triangle();

  void generateVertex() override;
  void updateVertex() override {}; // <-- we don't ever update the vertices
};

}


#endif /* TRIANGLE_H */
