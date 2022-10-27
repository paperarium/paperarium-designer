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

    // Try loading the Demo
    scene->InitDemo(this);

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
    glClearColor(1,1,1,1);
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
//        RenderCube();
        DrawBordered();
        PostProcessDeferredLights();
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

    shader = SINGLE_COLOR;

    // now draw mesh based on shader type
    switch (shader) {
    // for the default shader, just draw the mesh normally, with textures
    case DEFAULT: {
        // pass in meta information about the render pass to the shader
        program->setUniformValue("projMatrix", m_proj);
        program->setUniformValue("mvMatrix", camera->GetWorldMatrix().inverted() * m_world);
        program->setUniformValue("normalMatrix", m_world.normalMatrix());
        program->setUniformValue("modelMatrix", m_world);
        program->setUniformValue("cameraPos", camera->GetPos());
        program->setUniformValue("mode", 1);

        // now pass in each mesh to the shader
        for (int i = 0; i < mesh->sub_meshes.size(); i++) {
            GLSubMesh* sub = mesh->sub_meshes[i];
            // if we have the vertex array object already
            if (sub->vao.isCreated()) {
                // search for the diffuse texture
                GLTexture* diffuseTexture = nullptr;
                for (int i = 0; i < sub->textures.size() && diffuseTexture == nullptr; i++)
                    if (sub->textures[i].type == "texture_diffuse")
                        diffuseTexture = &sub->textures[i];
                // if we found a diffuse texture, activate it
                if (diffuseTexture != nullptr) {
                    glActiveTexture(GL_TEXTURE0);
                    program->setUniformValue("diffuseTexture", 0);
                    diffuseTexture->glTexture->bind();
                }
                // load the vertices into the shader and draw them
                QOpenGLVertexArrayObject::Binder vaoBinder(&sub->vao);
                if (sub->num_faces > 0) glDrawElements(GL_TRIANGLES, sub->num_faces * 3, GL_UNSIGNED_INT, nullptr);
                else glDrawArrays(GL_TRIANGLES, 0, sub->num_vertices);
                // finally, release the texture
                if (diffuseTexture != nullptr) diffuseTexture->glTexture->release();
            }
        }
        break;
    }
    // for the single color shader, draw the mesh normally, without textures
    case SINGLE_COLOR: {
        m_world.scale(border_scale);
        // pass in meta information about the render pass to the shader
        program->setUniformValue("projection", m_proj);
        program->setUniformValue("modelView", camera->GetWorldMatrix().inverted() * m_world);
        program->setUniformValue("flatColor", border_color);
        program->setUniformValue("alpha", border_alpha);

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
        program->setUniformValue("useFlatColor", state == BORDERS);
        program->setUniformValue("projection", m_proj);
        program->setUniformValue("view", camera->GetWorldMatrix().inverted());
        program->setUniformValue("model", m_world);
        program->setUniformValue("modelInv", m_world.inverted());

        // now pass in each mesh to the shader
        for (int i = 0; i < mesh->sub_meshes.size(); i++) {
            GLSubMesh* sub = mesh->sub_meshes[i];
            // if we have the vertex array object already
            if (sub->vao.isCreated()) {
                // search for the diffuse texture
                GLTexture* diffuseTexture = nullptr;
                for (int i = 0; i < sub->textures.size() && diffuseTexture == nullptr; i++)
                    if (sub->textures[i].type == "texture_diffuse")
                        diffuseTexture = &sub->textures[i];
                // if we found a diffuse texture, activate it
                if (diffuseTexture != nullptr) {
                    glActiveTexture(GL_TEXTURE0);
                    program->setUniformValue("diffuseTexture", 0);
                    diffuseTexture->glTexture->bind();
                }
                // load the vertices into the shader and draw them
                QOpenGLVertexArrayObject::Binder vaoBinder(&sub->vao);
                if (sub->num_faces > 0) glDrawElements(GL_TRIANGLES, sub->num_faces * 3, GL_UNSIGNED_INT, nullptr);
                else glDrawArrays(GL_TRIANGLES, 0, sub->num_vertices);
                // finally, release the texture
                if (diffuseTexture != nullptr) diffuseTexture->glTexture->release();
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
    mesh->vbo.release();

    // load our NORMALS
    mesh->nbo.create();
    mesh->nbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    mesh->nbo.bind();
    mesh->nbo.allocate(mesh->normal_data.constData(), 3 * mesh->num_vertices * static_cast<int>(sizeof(GLfloat)));
    programs[DEFAULT]->enableAttributeArray(1);
    programs[DEFAULT]->setAttributeBuffer(1, GL_FLOAT, 0, 3);
    mesh->nbo.release();

    // load our TEXTURE COORDS
    mesh->tbo.create();
    mesh->tbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    mesh->tbo.bind();
    mesh->tbo.allocate(mesh->texcoord_data.constData(), 2 * mesh->num_vertices * static_cast<int>(sizeof(GLfloat)));
    programs[DEFAULT]->enableAttributeArray(2);
    programs[DEFAULT]->setAttributeBuffer(2, GL_FLOAT, 0, 2);
    mesh->tbo.release();

    // load our TANGENTS
    mesh->tnbo.create();
    mesh->tnbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    mesh->tnbo.bind();
    mesh->tnbo.allocate(mesh->tangent_data.constData(), 3 * mesh->num_vertices * static_cast<int>(sizeof(GLfloat)));
    programs[DEFAULT]->enableAttributeArray(3);
    programs[DEFAULT]->setAttributeBuffer(3, GL_FLOAT, 0, 3);
    mesh->tnbo.release();

    // load our BITANGENTS
    mesh->btnbo.create();
    mesh->btnbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    mesh->btnbo.bind();
    mesh->btnbo.allocate(mesh->bitangent_data.constData(), 3 * mesh->num_vertices * static_cast<int>(sizeof(GLfloat)));
    programs[DEFAULT]->enableAttributeArray(4);
    programs[DEFAULT]->setAttributeBuffer(4, GL_FLOAT, 0, 3);
    mesh->btnbo.release();

    // setup our INDICES
    if (mesh->num_faces > 0) {
        mesh->ibo.create();
        mesh->ibo.setUsagePattern(QOpenGLBuffer::StaticDraw);
        mesh->ibo.bind();
        mesh->ibo.allocate(mesh->index_data.constData(), 3 * mesh->num_faces * static_cast<int>(sizeof(GLint)));
        mesh->ibo.release();
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
    qDebug() << camera->GetPos();
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
    // The ordered list of shaders we want to load
    QVector<QString> shaders{
        "default",
        "single_color",
        "framebuffer_to_screen",
        "graphic_buffer",
        "deferred_shading",
        "deferred_light"
    };

    // Iterate over and load each shader from their sources
    for (const auto& shader : shaders) {
        QOpenGLShaderProgram* shader_program = new QOpenGLShaderProgram();
        if (!shader_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/" + shader + ".vert"))
            qDebug() << "Error loading " + shader + ".vert shader";
        if (!shader_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/" + shader + ".frag"))
            qDebug() << "Error loading " + shader + ".frag shader";
        if (!shader_program->link()) qDebug() << "Error linking " + shader + " shader!";
        if (!shader_program->bind()) qDebug() << "Error binding " + shader + " shader!";
        programs.push_back(shader_program);
        shader_program->release();
    }
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
    glGenTextures(1, &gAlbedo);
    glBindTexture(GL_TEXTURE_2D, gAlbedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);

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
    case GL_FRAMEBUFFER_COMPLETE: break; // Everything's OK
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

void OpenGLWidget::PostProcessDeferredLights() {
    if (!use_deferred) return;
    state = POST_PROCESSING;
    QOpenGLFramebufferObject::bindDefault();

    // lighting pass
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    programs[DEFERRED_SHADING]->bind();
    programs[DEFERRED_SHADING]->setUniformValue("viewPos", camera->GetPos());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gAlbedo);

    // Update and set scene lights
    for (int i = 0; i < lights.count(); ++i) {
        if (!lights[i].updated) {
            lights[i].maxBrightness = std::fmaxf(std::fmaxf(lights[i].Color.x(), lights[i].Color.y()), lights[i].Color.z());
            lights[i].radius = (-lights[i].Linear + std::sqrt(lights[i].Linear * lights[i].Linear - 4 * lights[i].Quadratic * (lights[i].Constant - (256.0f / 5.0f) * lights[i].maxBrightness))) / (2.0f * lights[i].Quadratic);
            lights[i].updated = true;
        }
        programs[DEFERRED_SHADING]->setUniformValue(QString("lights[" + QString::number(i) + "].isActive").toStdString().c_str(), lights[i].isActive);
        programs[DEFERRED_SHADING]->setUniformValue(QString("lights[" + QString::number(i) + "].Position").toStdString().c_str(), lights[i].Position);
        programs[DEFERRED_SHADING]->setUniformValue(QString("lights[" + QString::number(i) + "].Color").toStdString().c_str(), lights[i].Color);
        programs[DEFERRED_SHADING]->setUniformValue(QString("lights[" + QString::number(i) + "].Intensity").toStdString().c_str(), lights[i].Intensity);
        programs[DEFERRED_SHADING]->setUniformValue(QString("lights[" + QString::number(i) + "].Range").toStdString().c_str(), lights[i].MinRange);
        programs[DEFERRED_SHADING]->setUniformValue(QString("lights[" + QString::number(i) + "].Linear").toStdString().c_str(), lights[i].Linear);
        programs[DEFERRED_SHADING]->setUniformValue(QString("lights[" + QString::number(i) + "].Quadratic").toStdString().c_str(), lights[i].Quadratic);
        programs[DEFERRED_SHADING]->setUniformValue(QString("lights[" + QString::number(i) + "].Radius").toStdString().c_str(), lights[i].radius);
    }

    RenderQuad();
    programs[DEFERRED_SHADING]->release();

    // copy geometry depth buffer to framebuffer's depth buffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // render lights on top of scene
    programs[DEFERRED_LIGHT]->bind();
    programs[DEFERRED_LIGHT]->setUniformValue("projection", m_proj);

    for (int i = 0; i < lights.count(); i++) {
        QMatrix4x4 w;
        w.setToIdentity();
        w.translate(lights[i].Position);
        programs[DEFERRED_LIGHT]->setUniformValue("mv", camera->GetWorldMatrix().inverted() * w);
        programs[DEFERRED_LIGHT]->setUniformValue("lightColor", lights[i].Color);
        RenderCube();
    }
    programs[DEFERRED_LIGHT]->release();
}

void OpenGLWidget::RenderCube()
{
    if (cubeVAO == 0)
    {
        GLfloat vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(GLfloat)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}
