#ifndef GLMESH_H
#define GLMESH_H

#include "glcomponent.h"
#include <qopengl.h>
#include <QVector>
#include <QVector2D>
#include <QVector3D>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// forward declared classes here
class GLObject;
class OpenGLWidget;
class QOpenGLShaderProgram;

struct GLVertex {
    QVector3D Position;
    QVector3D Normal;
    QVector2D TexCoords;
    QVector3D Tangent;
    QVector3D Bitangent;
};

struct GLTexture {
    QOpenGLTexture* glTexture;
    QString type;
    QString path;
};

class GLSubMesh {
public:
    GLSubMesh():
        vbo(QOpenGLBuffer::VertexBuffer),
        nbo(QOpenGLBuffer::VertexBuffer),
        tbo(QOpenGLBuffer::VertexBuffer),
        tnbo(QOpenGLBuffer::VertexBuffer),
        btnbo(QOpenGLBuffer::VertexBuffer),
        ibo(QOpenGLBuffer::VertexBuffer){}

    // High-level information about the mesh
    int num_vertices = 0;
    int num_faces = 0;
    QVector<GLVertex> vertices;
    QVector<unsigned int> indices;
    QVector<GLTexture> textures;

    // Low-level OpenGL information about the mesh
    QVector<GLfloat> vertex_data;
    QVector<GLfloat> normal_data;
    QVector<GLfloat> texcoord_data;
    QVector<GLfloat> tangent_data;
    QVector<GLfloat> bitangent_data;
    QVector<GLuint> index_data;

    // OpenGL buffers for render data
    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer vbo;      // vertex buffer object
    QOpenGLBuffer nbo;      // normal buffer object
    QOpenGLBuffer tbo;      // texcoord buffer object
    QOpenGLBuffer tnbo;     // tangent buffer object
    QOpenGLBuffer btnbo;    // bitangent buffer object
    QOpenGLBuffer ibo;      // index buffer object

};

class GLMesh : public GLComponent
{
public: // METHODS
    GLMesh(GLObject* glo = nullptr, bool isActive = true);
    ~GLMesh() override;

    void Draw(OpenGLWidget* renderer) override;

    void Save(QDataStream& stream) override;
    void Load(QDataStream& stream) override;
    void CleanUp() override;

    void importModel(QString path, OpenGLWidget* renderer = nullptr);
    void processNode(aiNode *node, const aiScene *scene, OpenGLWidget* renderer = nullptr);
    GLSubMesh* processMesh(aiMesh *aiMesh, const aiScene *scene, OpenGLWidget* renderer = nullptr);
    QVector<GLTexture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, QString typeName);

public: // MEMBERS

    QVector<GLSubMesh*> sub_meshes;
    QVector<GLTexture> texturesLoaded;

    bool draw_border = false;
private:
    QString directory;
};

#endif // GLMESH_H
