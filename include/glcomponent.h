#ifndef COMPONENT_H
#define COMPONENT_H

#include <QDataStream>

// forward declarations
class GLObject;
class OpenGLWidget;

/**
 * @brief Types of components in the OpenGL view
 *
 * Possible values:
 *  - EMPTY (0)
 *  - TRANSFORM (1)
 *  - MESH (2)
 *  - LIGHT (3)
 */
enum GLComponentType {
    EMPTY,
    TRANSFORM,
    MESH,
    LIGHT
};

/**
 * @brief A component in an OpenGL view
 *
 * Defines a serializable entity in the OpenGL view, allowing saving to a file
 * and restoring from said file.
 */
class GLComponent
{
public: // METHODS
    GLComponent(GLComponentType type = EMPTY, GLObject* gameObject = nullptr, bool isActive = true);
    virtual ~GLComponent();

    // Functions for saving / restoring state
    virtual void Save(QDataStream& stream) = 0;
    virtual void Load(QDataStream& stream) = 0;
    virtual void CleanUp() = 0;
    virtual void Draw(OpenGLWidget *renderer) {}

public: // MEMBERS
    GLComponentType type = EMPTY;
    GLObject* glObject = nullptr;
    bool isActive = true;
};

#endif // COMPONENT_H
