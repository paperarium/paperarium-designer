#include "openglwidget.h"
#include "gltransform.h"
#include "glscene.h"
#include "glmesh.h"
#include "globject.h"

/**
 * @brief OpenGLWidget::OpenGLWidget
 *
 * Initializes the OpenGL widget with a camera and retrieves its dimensions.
 */
OpenGLWidget::OpenGLWidget(QWidget *parent): QOpenGLWidget(parent), tick_count(0) {
    setFocusPolicy(Qt::ClickFocus);

    // initialize the camera at 0,0,5
    camera = new GLTransform(nullptr, true, {0,0,5});
    for (int i = 0; i < 6; i++) cam_dir[i] = false;
    cam_focus = {0,0,0};

    // set the border color
    border_color = QVector3D(1,0.27f,0);
    border_meshes.clear();

    // set the size hint
    QSize size = sizeHint();
    width = size.width();
    height = size.height();

    // finally, set the state
    state = WIDGET_CREATED;
}

OpenGLWidget::~OpenGLWidget() {
     makeCurrent();
     if (scene != nullptr) scene->Clear();
     doneCurrent();
}

QSize OpenGLWidget::minimumSizeHint() const {
    return QSize(100,100);
}

QSize OpenGLWidget::sizeHint() const {
    return QSize(563, 453);
}

void OpenGLWidget::Tick() {
    tick_count += tick_period;
    // update camera transform
    if (cam_dir[0]) camera->TranslateForward(0.01f * tick_period);
    if (cam_dir[1]) camera->TranslateForward(-0.01f * tick_period);
    if (cam_dir[2]) camera->TranslateLeft(0.01f * tick_period);
    if (cam_dir[3]) camera->TranslateLeft(-0.01f * tick_period);
    if (cam_dir[4]) camera->TranslateUp(0.01f * tick_period);
    if (cam_dir[5]) camera->TranslateUp(-0.01f * tick_period);
    if (camera_light_follow) lights[0].Position = camera->GetPos();
    // update the renderer
    update();
}

void OpenGLWidget::initializeGL() {
    // initialize the GL instance
    state = INITIALIZING;
    initializeOpenGLFunctions();

    // Shaders
    LoadShaders();
    // Framebuffer
    LoadFramebuffer();
    // Lights
    ResetLights();
    lights[0].isActive = true;
    lights[0].Intensity = 3;

    // GL: Enable depth buffer
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    // GL: Enable back face culling ( might not want this one)
    glEnable(GL_CULL_FACE);
    // GL: Enable transparency blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    // GL: Enable stencil
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // begin the tick timer for ticking the OpenGL scene
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &OpenGLWidget::Tick);
    timer->start(static_cast<int>(tick_period));

    // lastly, update the state
    state = INITIALIZED;
}

void OpenGLWidget::paintGL() {
    // prepare the GL widget for drawing
    state = PREPARING_TO_DRAW;

    // choose target frame buffer
    if (use_deferred) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glDisable(GL_BLEND); // cannot blend with deferred lighting
    } else {
        QOpenGLFramebufferObject::bindDefault();
        glEnable(GL_BLEND);
    }

    // reset the buffer
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glStencilMask(0x00);
    border_meshes.clear();

    // prepare to draw the models
    state = MODELS;

    // draw the scene, then skybox, then borders, then deferred lights
    if (scene != nullptr) {
        scene->Draw(this);
//        if (renderSkybox) RenderSkybox();
        DrawBordered();
//        PostProcessDeferredLights();
    }

    // done with draw call!
    state = FINISHED;
}

void OpenGLWidget::DrawMesh(GLMesh* mesh, GLSHADER_TYPE shader) {
    // ensure we have a mesh and a shader program
    if (mesh == nullptr || programs.isEmpty()) return;
    // if we're drawing deferred, render to graphics buffer
    else if (use_deferred) shader = GRAPHIC_BUFFER;
    // if we're drawing borders, just save the mesh for its eventual border
    else if (draw_borders && mesh->draw_border && state == MODELS) {
        border_meshes.push_back(mesh);
        return;
    }

    // make sure our world matrix is up-to-date
    QMatrix4x4 m_world = mesh->glObject->transform->GetWorldMatrix();

    // use our desired shader program
    QOpenGLShaderProgram* program = programs[static_cast<int>(shader)];
    program->bind();

    // now draw mesh based on shader type
    switch (shader) {
    // for the default shader, just draw the mesh normally, with textures
    case DEFAULT: {
        // pass in meta information about the render pass to the shader
        program->setUniformValue(d_projMatrixLoc, m_proj);
        program->setUniformValue(d_mvMatrixLoc, camera->GetWorldMatrix().inverted() * m_world);
        program->setUniformValue(d_normalMatrixLoc, m_world.normalMatrix());
        program->setUniformValue("modelMatrix", m_world);
        program->setUniformValue("cameraPos", camera->GetPos());
        program->setUniformValue(d_modeLoc, mode);

        // now pass in each mesh to the shader
        for (int i = 0; i < mesh->sub_meshes.size(); i++) {
            GLSubMesh* sub = mesh->sub_meshes[i];
            // if we have the vertex array object already
            if (sub->vao.isCreated()) {
                // if skybox exists, bind it to the shader
                if (skyboxes[current_skybox] != nullptr)
                    skyboxes[current_skybox]->bind();
                unsigned int diffuseNr = 1;
                unsigned int specularNr = 1;
                // load the textures into the shader
                for (int i = 0; i < sub->textures.size(); i++) {
                    glActiveTexture(GL_TEXTURE0 + static_cast<unsigned int>(i));
                    // generate a name for the texture based on diffuse vs. specular
                    QString number;
                    QString name = sub->textures[i].type;
                    if (name == "texture_diffuse") number = QString::number(diffuseNr++);
                    else if (name == "texture_specular") number = QString::number(specularNr++);
                    // pass the name for the texture into the shader program
                    program->setUniformValue((name + number).toStdString().c_str(), static_cast<unsigned int>(i));
                    // and bind the texture to the shader
                    sub->textures[i].glTexture->bind();
                }
                // load the vertices into the shader and draw them
                QOpenGLVertexArrayObject::Binder vaoBinder(&sub->vao);
                if (sub->num_faces > 0) glDrawElements(GL_TRIANGLES, sub->num_faces * 3, GL_UNSIGNED_INT, nullptr);
                else glDrawArrays(GL_TRIANGLES, 0, sub->num_vertices);
                // lastly, release the textures we used in this draw call
                for (int i = 0; i < sub->textures.count(); i++)
                    if (sub->textures[i].glTexture != nullptr && sub->textures[i].glTexture->isCreated())
                        sub->textures[i].glTexture->release();
            }
        }
        break;
    }
    // for the single color shader, draw the mesh normally, without textures
    case SINGLE_COLOR: {
        m_world.scale(border_scale);
        // pass in meta information about the render pass to the shader
        program->setUniformValue(sc_proj, m_proj);
        program->setUniformValue(sc_modelView, camera->GetWorldMatrix().inverted() * m_world);
        program->setUniformValue(sc_color, border_color);
        program->setUniformValue(sc_alpha, border_alpha);

        // now pass in each mesh to the shader
        for(int i = 0; i < mesh->sub_meshes.size(); i++) {
            GLSubMesh* sub = mesh->sub_meshes[i];
            // if we have the vertex array object already
            if(sub->vao.isCreated()) {
                // load the vertices into the shader and draw them
                QOpenGLVertexArrayObject::Binder vaoBinder(&sub->vao);
                if(sub->num_faces > 0) glDrawElements(GL_TRIANGLES, sub->num_faces * 3, GL_UNSIGNED_INT, nullptr);
                else glDrawArrays(GL_TRIANGLES, 0, sub->num_vertices);
            }
        }
        break;
    }
    case GRAPHIC_BUFFER: {
        if (state == BORDERS) m_world.scale(border_scale);
        program->setUniformValue("use_flat_color", state == BORDERS);
        program->setUniformValue("projection", m_proj);
        program->setUniformValue("view", camera->GetWorldMatrix().inverted());
        program->setUniformValue("model", m_world);
        program->setUniformValue("modelInv", m_world.inverted());

        // now pass in each mesh to the shader
        for (int i = 0; i < mesh->sub_meshes.size(); i++) {
            GLSubMesh* sub = mesh->sub_meshes[i];
            // if we have the vertex array object already
            if (sub->vao.isCreated()) {
                // bind appropriate textures as single colors
                unsigned int diffuseNr = 1;
                unsigned int specularNr = 1;
                for (int i = 0; i < sub->textures.size(); i++) {
                    glActiveTexture(GL_TEXTURE0 + static_cast<unsigned int>(i));
                    // generate a name for the texture based on diffuse vs. specular
                    QString number;
                    QString name = sub->textures[i].type;
                    if (name == "texture_diffuse") number = QString::number(diffuseNr++);
                    else if (name == "texture_specular") number = QString::number(specularNr++);
                    // pass the name for the texture into the shader program
                    program->setUniformValue((name + number).toStdString().c_str(), static_cast<unsigned int>(i));
                    // and bind the texture to the shader
                    sub->textures[i].glTexture->bind();
                }
                // load the vertices into the shader and draw them
                QOpenGLVertexArrayObject::Binder vaoBinder(&sub->vao);
                if (sub->num_faces > 0) glDrawElements(GL_TRIANGLES, sub->num_faces * 3, GL_UNSIGNED_INT, nullptr);
                else glDrawArrays(GL_TRIANGLES, 0, sub->num_vertices);
                // lastly, release the textures we used in this draw call
                for (int i = 0; i < sub->textures.count(); i++)
                    if (sub->textures[i].glTexture != nullptr && sub->textures[i].glTexture->isCreated())
                        sub->textures[i].glTexture->release();
            }
        }
        break;
    }
    default: break;
    }
    // release the resources being used for the mesh rendering
    program->release();
}

void OpenGLWidget::LoadSubMesh(GLSubMesh* mesh) {
    // ensure the submesh actually has topology
    if (mesh == nullptr || mesh->num_vertices <= 0) return;

    // now load the opengl array objects using the default shader program
    programs[DEFAULT]->bind();

    // build the Vertex Array Object
    mesh->vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&mesh->vao);

    // load our POSITIONS
    mesh->vbo.create();
    mesh->vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    mesh->vbo.bind();
    mesh->vbo.allocate(mesh->vertex_data.constData(), 3 * mesh->num_vertices * static_cast<int>(sizeof(GLfloat)));
    programs[DEFAULT]->enableAttributeArray(0);
    programs[DEFAULT]->setAttributeBuffer(0, GL_FLOAT, 0, 3);

    // load our NORMALS
    mesh->nbo.create();
    mesh->nbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    mesh->nbo.bind();
    mesh->nbo.allocate(mesh->normal_data.constData(), 3 * mesh->num_vertices * static_cast<int>(sizeof(GLfloat)));
    programs[DEFAULT]->enableAttributeArray(1);
    programs[DEFAULT]->setAttributeBuffer(1, GL_FLOAT, 0, 3);

    // load our TEXTURE COORDS
    mesh->tbo.create();
    mesh->tbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    mesh->tbo.bind();
    mesh->tbo.allocate(mesh->texcoord_data.constData(), 3 * mesh->num_vertices * static_cast<int>(sizeof(GLfloat)));
    programs[DEFAULT]->enableAttributeArray(2);
    programs[DEFAULT]->setAttributeBuffer(2, GL_FLOAT, 0, 2);

    // load our TANGENTS
    mesh->tnbo.create();
    mesh->tnbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    mesh->tnbo.bind();
    mesh->tnbo.allocate(mesh->tangent_data.constData(), 3 * mesh->num_vertices * static_cast<int>(sizeof(GLfloat)));
    programs[DEFAULT]->enableAttributeArray(3);
    programs[DEFAULT]->setAttributeBuffer(3, GL_FLOAT, 0, 3);

    // load our BITANGENTS
    mesh->btnbo.create();
    mesh->btnbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    mesh->btnbo.bind();
    mesh->btnbo.allocate(mesh->bitangent_data.constData(), 3 * mesh->num_vertices * static_cast<int>(sizeof(GLfloat)));
    programs[DEFAULT]->enableAttributeArray(4);
    programs[DEFAULT]->setAttributeBuffer(4, GL_FLOAT, 0, 3);

    // setup our INDICES
    if (mesh->num_faces > 0) {
        mesh->ibo.create();
        mesh->ibo.setUsagePattern(QOpenGLBuffer::StaticDraw);
        mesh->ibo.bind();
        mesh->ibo.allocate(mesh->index_data.constData(), 3 * mesh->num_faces * static_cast<int>(sizeof(GLint)));
    }

    // release the shader program
    programs[DEFAULT]->release();
}

void OpenGLWidget::ResetLights() {
    // clears the light array
    lights.clear();
    for (unsigned int i = 0; i < NR_LIGHTS; i++) {
        GLLight light;
        light.isActive = false;
        light.Position = {0,0,0};
        light.Color = {1,1,1};
        light.Intensity = 1.0f;
        light.MinRange = 1.0f;
        light.Constant = 1.0f;
        light.Linear = 0.7f;
        light.Quadratic = 1.8f;
        light.updated = false;
        lights.push_back(light);
    }
}

void OpenGLWidget::resizeGL(int width, int height) {
    // reset the camera
    m_proj.setToIdentity();
    m_proj.perspective(zoom, GLfloat(width) / height, 0.1f, 100.0f);
    this->width = width;
    this->height = height;

    // ! ТОDO: Create new frame buffer with new target size?
}

void OpenGLWidget::mousePressEvent(QMouseEvent *event) {
    mouse_pos = event->pos();
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent *event) {
    if (orbiting) {
        camera->Orbit(
                    static_cast<float>((mouse_pos - event->position()).x()) * 0.5f,
                    static_cast<float>((mouse_pos - event->position()).y()) * 0.5f,
                    cam_focus);
        camera->RotateAxisForward(-camera->GetRot().z()); // correct roll
    } else {
        camera->RotateAxisUp(static_cast<float>((mouse_pos - event->position()).x()));
        camera->RotateAxisLeft(static_cast<float>((mouse_pos - event->position()).y()));
        camera->RotateAxisForward(-camera->GetRot().z()); // correct roll
    }
    mouse_pos = event->position();
}

void OpenGLWidget::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_W: cam_dir[0] = true; break;
    case Qt::Key_S: cam_dir[1] = true; break;
    case Qt::Key_A: cam_dir[2] = true; break;
    case Qt::Key_D: cam_dir[3] = true; break;
    case Qt::Key_E: cam_dir[4] = true; break;
    case Qt::Key_Q: cam_dir[5] = true; break;
    case Qt::Key_Alt: orbiting = true; break;
    case Qt::Key_F: camera->Focus(cam_focus); break;
    case Qt::Key_T: camera_light_follow = !camera_light_follow; break;
    case Qt::Key_R: lights[0].isActive = !lights[0].isActive; break;
    case Qt::Key_C: ResetLights(); break;
    case Qt::Key_Space: {
        current_light + 1 == NR_LIGHTS ? current_light = 1 : current_light++;
        lights[current_light].isActive = true;
        lights[current_light].Position = camera->GetPos();
        break;
    }
    case Qt::Key_X: mode = mode+1.f>9.f?0:mode+1; break;
    case Qt::Key_G: use_deferred = !use_deferred; break;
    default: break;
    }
}

void OpenGLWidget::keyReleaseEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_W: cam_dir[0] = false; break;
    case Qt::Key_S: cam_dir[1] = false; break;
    case Qt::Key_A: cam_dir[2] = false; break;
    case Qt::Key_D: cam_dir[3] = false; break;
    case Qt::Key_E: cam_dir[4] = false; break;
    case Qt::Key_Q: cam_dir[5] = false; break;
    case Qt::Key_Alt: orbiting = false; break;
    default: break;
    }
}

void OpenGLWidget::wheelEvent(QWheelEvent *event) {
    zoom -= event->angleDelta().y() * 0.01f;
    if (zoom < 1) zoom = 1;
    else if (zoom > 160) zoom = 160;
    m_proj.setToIdentity();
    m_proj.perspective(zoom, GLfloat(width) / height, 0.1f, 100.0f);
}

void OpenGLWidget::RenderQuad() {
    if (quadVAO == 0) {
        // draw a screen quad
        float quadVertices[] = {
            // positions        // texture coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
             1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
        };
        // set up plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1,2,GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void OpenGLWidget::LoadShaders() {
    // Load default shader
    QOpenGLShaderProgram* d_shader = new QOpenGLShaderProgram();
    if (!d_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/default.vert"))
        qDebug() << "Error loading default.vert shader";
    if (!d_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/default.frag"))
        qDebug() << "Error loading default.frag shader";
    d_shader->bindAttributeLocation("vertex", 0);
    d_shader->bindAttributeLocation("normal", 1);
    d_shader->bindAttributeLocation("texCoord", 2);
    d_shader->bindAttributeLocation("tangent", 3);
    d_shader->bindAttributeLocation("bitangent", 4);
    if (!d_shader->link()) qDebug() << "Error linking default shader!";
    if (!d_shader->bind()) qDebug() << "Error binding default shader!";
    d_projMatrixLoc = d_shader->uniformLocation("projMatrix");
    d_mvMatrixLoc = d_shader->uniformLocation("mvMatrix");
    d_normalMatrixLoc = d_shader->uniformLocation("normalMatrix");
    d_lightPosLoc = d_shader->uniformLocation("lightPos");
    d_lightIntensityLoc = d_shader->uniformLocation("light_intensity");
    d_modeLoc = d_shader->uniformLocation("mode");
    d_textureLoc = d_shader->uniformLocation("texture");
    programs.push_back(d_shader);
    d_shader->release();

   // Load single color shader
   QOpenGLShaderProgram* sc_shader = new QOpenGLShaderProgram();
   if (!sc_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/singlecolor.vert"))
       qDebug() << "Error loading singecolor.vert shader";
   if (!sc_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/singlecolor.frag"))
       qDebug() << "Error loading singecolor.frag shader";
    sc_shader->bindAttributeLocation("aPos", 0);
    sc_shader->bindAttributeLocation("texCoord", 1);
    if (!sc_shader->link()) qDebug() << "Error linking single color shader!";
    if (!sc_shader->bind()) qDebug() << "Error binding single color shader!";
    sc_modelView = sc_shader->uniformLocation("modelView");
    sc_proj = sc_shader->uniformLocation("projection");
    sc_color = sc_shader->uniformLocation("flat_color");
    sc_alpha = sc_shader->uniformLocation("alpha");
    programs.push_back(sc_shader);
    sc_shader->release();

    // Load framebuffer to screen shader
    QOpenGLShaderProgram* fs_shader = new QOpenGLShaderProgram();
    if (!fs_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/framebuffertoscreen.vert"))
        qDebug() << "Error loading framebuffertoscreen.vert shader";
    if (!fs_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/framebuffertoscreen.frag"))
        qDebug() << "Error loading framebuffertoscreen.frag shader";
    fs_shader->bindAttributeLocation("vertex", 0);
    fs_shader->bindAttributeLocation("texCoord", 1);
    if (!fs_shader->link()) qDebug() << "Error linking framebuffer to screen shader!";
    if (!fs_shader->bind()) qDebug() << "Error binding framebuffer to screen shader!";
    fs_screenTexture = fs_shader->uniformLocation("screenTexture");
    fs_shader->setUniformValue(fs_screenTexture, 0);
    programs.push_back(fs_shader);
    fs_shader->release();

    // Load graphics buffer shader
    QOpenGLShaderProgram* gb_shader = new QOpenGLShaderProgram();
    if (!gb_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/graphic_buffer.vert"))
        qDebug() << "Error loading graphic_buffer.vert shader";
    if (!gb_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/graphic_buffer.frag"))
        qDebug() << "Error loading graphic_buffer.frag shader";
    gb_shader->bindAttributeLocation("gPosition", 0);
    gb_shader->bindAttributeLocation("gNormal", 1);
    gb_shader->bindAttributeLocation("gAlbedoSpec", 2);
    if (!gb_shader->link()) qDebug() << "Error linking graphic buffer shader!";
    if (!gb_shader->bind()) qDebug() << "Error binding graphic buffer shader!";
    programs.push_back(gb_shader);
    gb_shader->release();

    // Load deferred shading shader
    QOpenGLShaderProgram* def_shader = new QOpenGLShaderProgram();
    if (!def_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/deferred_shading.vert"))
        qDebug() << "Error loading deferred_shading.vert shader";
    if (!def_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/deferred_shading.frag"))
        qDebug() << "Error loading deferred_shading.frag shader";
    if (!def_shader->link()) qDebug() << "Error linking deferred shading shader!";
    if (!def_shader->bind()) qDebug() << "Error binding deferred shading shader!";
    def_posLoc = def_shader->uniformLocation("gPosition");
    def_normalLoc = def_shader->uniformLocation("gNormal");
    def_albedospecLoc = def_shader->uniformLocation("gAlbedoSpec");
    def_shader->setUniformValue(def_posLoc, 0);
    def_shader->setUniformValue(def_normalLoc, 1);
    def_shader->setUniformValue(def_albedospecLoc, 2);
    programs.push_back(def_shader);
    def_shader->release();

    // Load deferred lighting shader
    QOpenGLShaderProgram* dl_shader = new QOpenGLShaderProgram();
    if (!dl_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/deferred_light.vert"))
        qDebug() << "Error loading deferred_light.vert shader";
    if (!dl_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/deferred_light.frag"))
        qDebug() << "Error loading deferred_light.frag shader";
    if (!dl_shader->link()) qDebug() << "Error linking deferred lighting shader!";
    if (!dl_shader->bind()) qDebug() << "Error binding deferred lighting shader!";
    programs.push_back(dl_shader);
    dl_shader->release();

    // Skybox shader
    QOpenGLShaderProgram* sb_shader = new QOpenGLShaderProgram();
    if (!sb_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/skybox.vert"))
        qDebug() << "Error loading skybox.vert shader";
    if (!sb_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/skybox.frag"))
        qDebug() << "Error loading skybox.frag shader";
    if (!sb_shader->link()) qDebug() << "Error linking skybox shader!";
    if (!sb_shader->bind()) qDebug() << "Error binding skybox shader!";
    programs.push_back(sb_shader);
    sb_shader->release();
}

void OpenGLWidget::LoadFramebuffer() {
    // Framebuffer configuration
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Position buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    // Normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    // Color + specular color buffer
    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

    // Color attachments
    GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, buffers);

    // Depth attachment
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    // Check framebuffer status
    switch(glCheckFramebufferStatus(GL_FRAMEBUFFER)){
    case GL_FRAMEBUFFER_COMPLETE: qInfo() << "Framebuffer Complete"; break; // Everything's OK
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: qInfo() << "Framebuffer ERROR: GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: qInfo() << "Framebuffer ERROR: GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: qInfo() << "Framebuffer ERROR: GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: qInfo() << "Framebuffer ERROR: GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"; break;
    case GL_FRAMEBUFFER_UNSUPPORTED: qInfo() << "Framebuffer ERROR: GL_FRAMEBUFFER_UNSUPPORTED"; break;
    default: qInfo() << "Framebuffer ERROR: Unknown ERROR"; }

    // Finally, bind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLWidget::DrawBordered() {
    // If no bordered meshes to draw, return.
    if (border_meshes.isEmpty()) return;

    // set our state
    state = BORDERED_MODELS;

    // Draw mesh and write to stencil
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);
    for (int i = 0; i < border_meshes.count(); i++)
        DrawMesh(border_meshes[i]);

    // we are now drawing the borders of those meshes
    state = BORDERS;

    // Draw border from stencil
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    if (border_over_borderless) glDisable(GL_DEPTH_TEST);
    for (int i = 0; i < border_meshes.count(); i++)
        DrawMesh(border_meshes[i], SINGLE_COLOR);
    glStencilMask(0xFF);
    if (border_over_borderless) glEnable(GL_DEPTH_TEST);

    // consume the border-drawing meshes
    border_meshes.clear();
}
