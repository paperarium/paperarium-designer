#include "glcomponent.h"

GLComponent::GLComponent(GLComponentType type, GLObject* glObject, bool isActive):
    type(type), glObject(glObject), isActive(isActive) {}
GLComponent::~GLComponent() {}
