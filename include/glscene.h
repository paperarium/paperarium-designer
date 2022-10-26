#ifndef GLSCENE_H
#define GLSCENE_H

#include <QString>
#include "mainwindow.h"

class OpenGLWidget;
class GLObject;

class GLScene {
public: // METHODS
    GLScene();
    ~GLScene();

    void Clear();
    void Draw(OpenGLWidget* renderer);

    GLObject* AddGLObject(QString name, GLObject* parent = nullptr);
    void RemoveEntity(int id);

public: // MEMBERS
    GLObject* root = nullptr;
    MainWindow* mainWindow;
    int gloId = 0;
    bool loadNewModel = false;
    QString newModelPath;
};

#endif // GLSCENE_H
