#ifndef GLOBJECT_H
#define GLOBJECT_H

#include <QString>
#include <QVector>
#include <QVector3D>
#include <qtreewidget.h>
#include <QTreeWidgetItem>

class GLComponent;
class GLTransform;
class OpenGLWidget;

/**
 * @brief The GLObject class
 */
class GLObject
{
public: // METHODS
    GLObject(QString name = "untitled_glo",
             GLObject* parent = nullptr,
             bool isActive = true,
             QVector3D pos = QVector3D(0,0,0),
             QVector3D rot = QVector3D(0,0,0),
             QVector3D scale = QVector3D(1,1,1));
    ~GLObject();

    void Draw(OpenGLWidget* renderer = nullptr);
    void Save(QDataStream& stream);
    void Load(QDataStream& stream);
    void CleanUp();

public: // MEMBERS
    QString name = "untitled_glo";
    int id = -1;
    QTreeWidgetItem *hierarchyItem;

    GLObject* parent = nullptr;
    QVector<GLObject*> children;
    QVector<GLComponent*> components;
    GLTransform* transform = nullptr;
};

#endif // GLOBJECT_H
