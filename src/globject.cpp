#include "globject.h"
#include "gltransform.h"
#include "openglwidget.h"

GLObject::GLObject(QString name,
                   GLObject* parent,
                   bool isActive,
                   QVector3D pos,
                   QVector3D rot,
                   QVector3D scale) :
        name(name),
        id(-1),
        parent(parent) {

    // ensure children and components are empty
    children.clear();
    components.clear();
    // append this object to the parent's children
    if (parent != nullptr) parent->children.push_back(this);
    // add in the transform for the object
    transform = new GLTransform(this, isActive, pos, rot, scale);
    components.push_back(transform);
}

GLObject::~GLObject() { CleanUp(); }

void GLObject::Draw(OpenGLWidget* renderer) {
    // check if the object is active
    if (renderer == nullptr || transform == nullptr) return;

    // iterate over each component and draw them
    QVector<GLComponent*>::iterator comp = components.begin();
    for (; comp != components.end(); comp++) {
        if ((*comp)->type == MESH && (*comp)->isActive) {
            (*comp)->Draw(renderer);
        }
    }

    // draw each child GLObject
    QVector<GLObject*>::iterator child = children.begin();
    for (; child != children.end(); child++) {
        if ((*child)->transform->isActive) {
            (*child)->Draw(renderer);
        }
    }
}

void GLObject::Save(QDataStream& stream) {}
void GLObject::Load(QDataStream& stream) {}

void GLObject::CleanUp() {
    components.clear();
    children.clear();
}
