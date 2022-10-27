#include "glscene.h"
#include "globject.h"
#include "glmesh.h"

#include <QApplication>

GLScene::GLScene() {
    root = new GLObject("root");
}

GLScene::~GLScene() {
    Clear();
    delete root;
}

void GLScene::Clear() {
    delete root;
    root = new GLObject("root");
}

void GLScene::Draw(OpenGLWidget* renderer) {
    root->Draw(renderer);
    // if a new model needs to be loaded, do so on a draw call
    if (loadNewModel) {
        GLObject* loadedModelGLO = AddGLObject("untitled_glo");
        GLMesh* loadedModelMesh = new GLMesh(loadedModelGLO);
        loadedModelMesh->importModel(newModelPath, renderer);
        loadNewModel = false;
    }
}

GLObject *GLScene::AddGLObject(QString name, GLObject *parent) {
    if (parent == nullptr) parent = root;
    GLObject* ret = new GLObject(name, parent);
    ret->id = gloId;
    gloId++;
    return ret;
}

void GLScene::RemoveEntity(int id) {
    // ! TODO: Reference-count resources to free unusued only
}

GLMesh* GLScene::InitDemo(OpenGLWidget* renderer) {
    qDebug() << "HERE" << qApp->applicationDirPath();
    gloDemo = AddGLObject("Demo");
    GLMesh* mesh = new GLMesh(gloDemo);
    mesh->importModel("/Users/evan/Desktop/evan/paperarium/paperarium-design/resources/demo/square.obj", renderer);
    return nullptr;
}
