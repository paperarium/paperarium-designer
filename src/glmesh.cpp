#include "glmesh.h"
#include "glcomponent.h"
#include "globject.h"
#include "gltransform.h"
#include "openglwidget.h"

GLMesh::GLMesh(GLObject* glo, bool isActive): GLComponent(MESH, glo, isActive) {
    if (glo != nullptr) glo->components.push_back(this);
}

GLMesh::~GLMesh() { CleanUp(); }

void GLMesh::Draw(OpenGLWidget* renderer) {
    renderer->DrawMesh(this);
}

void GLMesh::Save(QDataStream &stream) {}
void GLMesh::Load(QDataStream &stream) {}
void GLMesh::CleanUp() {}

void GLMesh::importModel(QString path, OpenGLWidget* renderer) {
    Assimp::Importer import;

    // use ASSIMP to read the scene in from the file
    const aiScene *scene = import.ReadFile(
                path.toStdString(),
                aiProcess_CalcTangentSpace |
                aiProcess_Triangulate |
                aiProcess_GenSmoothNormals |
                aiProcess_FixInfacingNormals |
                aiProcess_JoinIdenticalVertices |
                aiProcess_PreTransformVertices |
                aiProcess_FlipUVs |
                aiProcess_OptimizeMeshes);

    // if there was an error loading the scene, log it to the debug log
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        qDebug() << "ERROR::ASSIMP::" << import.GetErrorString() << Qt::endl;
        return;
    }

    // get the name of the file
    bool extension = true;
    QString name;
    directory = path;
    for (int i = directory.length() - 1; i > 0; i--) {
        if (directory[i] != '/') {
            if (!extension) name = directory[i] + name;
            if (directory[i] == '.') extension = false;
            directory.remove(i, 1);
        } else break;
    }

    qDebug() << "Mesh at:" << directory;

    // add the ASSIMP mesh to the current object
    glObject->name = name;
    processNode(scene->mRootNode, scene, renderer);
}

void GLMesh::processNode(aiNode *node, const aiScene* scene, OpenGLWidget* renderer) {
    // read the transformation from the model
    aiMatrix4x4 t = node->mTransformation;
    aiVector3D pos, scale;
    aiQuaternion rot;
    t.Decompose(scale, rot, pos);
    // and apply the values to our own GLObject
    glObject->transform->Translate({pos.x, pos.y, pos.z});
    glObject->transform->RotateQ({rot.x, rot.y, rot.z, rot.w});
    glObject->transform->RotateZ(-180.0f); // flip mesh
    glObject->transform->SetScale({scale.x, scale.y, scale.z});

    // now add each sub mesh to this mesh
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        sub_meshes.push_back(processMesh(mesh, scene, renderer));
    }

    // lastly, process each child node
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene, renderer);
    }
}

GLSubMesh* GLMesh::processMesh(aiMesh* mesh, const aiScene *scene, OpenGLWidget* renderer) {
    // create a new submesh for this mesh
    GLSubMesh* subMesh = new GLSubMesh();

    // process each vertex, getting position, normal, texcoords, and tangent/bitangent
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        GLVertex vertex;
        // read in vertex position
        vertex.Position.setX(mesh->mVertices[i].x);
        vertex.Position.setY(mesh->mVertices[i].y);
        vertex.Position.setZ(mesh->mVertices[i].z);
        // read in vertex normal
        vertex.Normal.setX(mesh->mNormals[i].x);
        vertex.Normal.setY(mesh->mNormals[i].y);
        vertex.Normal.setZ(mesh->mNormals[i].z);
        // read in texture coords, if they exist
        if (mesh->HasTextureCoords(0)) {
            vertex.TexCoords.setX(mesh->mTextureCoords[0][i].x);
            vertex.TexCoords.setY(mesh->mTextureCoords[0][i].y);
        } else vertex.TexCoords = QVector2D(0.0f, 0.0f);
        // read in tangents and bitangents
        if (mesh->HasTangentsAndBitangents()) {
            vertex.Tangent.setX(mesh->mTangents[i].x);
            vertex.Tangent.setY(mesh->mTangents[i].y);
            vertex.Tangent.setZ(mesh->mTangents[i].z);
            vertex.Bitangent.setX(mesh->mBitangents[i].x);
            vertex.Bitangent.setY(mesh->mBitangents[i].y);
            vertex.Bitangent.setZ(mesh->mBitangents[i].z);
        } else {
            vertex.Tangent.setX(1);
            vertex.Tangent.setY(0);
            vertex.Tangent.setZ(0);
            vertex.Bitangent.setX(0);
            vertex.Bitangent.setY(1);
            vertex.Bitangent.setZ(0);
        }
        // now add the vertex to the sub mesh
        subMesh->vertices.push_back(vertex);
    }

    // update the sizes for the GL data QVectors
    subMesh->num_vertices = static_cast<int>(mesh->mNumVertices);
    subMesh->vertex_data.resize(subMesh->num_vertices * 3);
    subMesh->normal_data.resize(subMesh->num_vertices * 3);
    subMesh->texcoord_data.resize(subMesh->num_vertices * 2);
    subMesh->tangent_data.resize(subMesh->num_vertices * 3);
    subMesh->bitangent_data.resize(subMesh->num_vertices * 3);

    // and load the data into the GL float array (i think?)
    for (int i = 0; i < subMesh->num_vertices; i++) {
        GLfloat* float_p = subMesh->vertex_data.data() + (3 * i);
        // load position
        *float_p++ = subMesh->vertices[i].Position.x();
        *float_p++ = subMesh->vertices[i].Position.y();
        *float_p++ = subMesh->vertices[i].Position.z();
        // load normals
        float_p = subMesh->normal_data.data() + (3 * i);
        *float_p++ = subMesh->vertices[i].Normal.x();
        *float_p++ = subMesh->vertices[i].Normal.y();
        *float_p++ = subMesh->vertices[i].Normal.z();
        // load texcoords
        float_p = subMesh->texcoord_data.data() + (2 * i);
        *float_p++ = subMesh->vertices[i].TexCoords.x();
        *float_p++ = subMesh->vertices[i].TexCoords.y();
        // load tangent
        float_p = subMesh->tangent_data.data() + (3 * i);
        *float_p++ = subMesh->vertices[i].Tangent.x();
        *float_p++ = subMesh->vertices[i].Tangent.y();
        *float_p++ = subMesh->vertices[i].Tangent.z();
        // load bitangent
        float_p = subMesh->bitangent_data.data() + (3 * i);
        *float_p++ = subMesh->vertices[i].Bitangent.x();
        *float_p++ = subMesh->vertices[i].Bitangent.y();
        *float_p++ = subMesh->vertices[i].Bitangent.z();
    }

    // process each mesh face, 3 vertices each
    subMesh->num_faces = static_cast<int>(mesh->mNumFaces);
    subMesh->index_data.resize(subMesh->num_faces * 3);
    for (int i = 0; i < subMesh->num_faces; i++) {
        aiFace face = mesh->mFaces[i];
        GLuint* int_p = subMesh->index_data.data() + (3 * i);
        *int_p++ = static_cast<GLuint>(face.mIndices[0]);
        *int_p++ = static_cast<GLuint>(face.mIndices[1]);
        *int_p++ = static_cast<GLuint>(face.mIndices[2]);
    }

    // process each material
    if (mesh->mMaterialIndex > 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        QVector<GLTexture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        subMesh->textures << diffuseMaps;
    }

    // done! log the submesh
    qDebug() << "Loading Mesh:" << mesh->mName.C_Str() << "with" << subMesh->vertices.count() << "vertices";
    renderer->LoadSubMesh(subMesh);
    return subMesh;
}

QVector<GLTexture> GLMesh::loadMaterialTextures(aiMaterial *mat, aiTextureType type, QString typeName) {
    QVector<GLTexture> textures;
    // read in all the textures matching the desired type
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        // iterate through, checking if the texture has already been loaded
        // ! TODO: This might catch all textures contained in the name of another texture.
        for (int j = 0; j < texturesLoaded.size(); j++) {
            if (texturesLoaded[j].path.contains(str.C_Str())) {
                textures.push_back(texturesLoaded[j]);
                skip = true;
                break;
            }
        }
        // if no texture was found, load and add this one
        if (!skip) {
            GLTexture texture;
            texture.glTexture = new QOpenGLTexture(QImage(directory + str.C_Str()));
            qDebug() << "Loading Texture:"<< str.C_Str();
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            texturesLoaded.push_back(texture);
        }
    }
    // return the loaded textures
    return textures;
}
