#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

// Inspired by: https://github.com/cumus/QT-3D-Model-Viewer/blob/master/QT3DModelViewer/myopenglwidget.h

#include <QVector3D>
#include <QPoint>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QMouseEvent>
#include <QTimer>

class GLTransform;
class GLMesh;
class GLSubMesh;
class GLScene;

/**
 * @brief A shader to use in the Renderer
 *
 * Possible values:
 *  - DEFAULT (0)
 *  - SINGLE_COLOR (1)
 *  - FRAMEBUFFER_TO_SCREEN (2)
 *  - GRAPHIC_BUFFER (3)
 *  - DEFERRED_SHADING (4)
 *  - DEFERRED_LIGHT (5)
 *  - SKYBOX (6)
 */
enum GLSHADER_TYPE: int {
    DEFAULT = 0,
    SINGLE_COLOR,
    FRAMEBUFFER_TO_SCREEN,
    GRAPHIC_BUFFER,
    DEFERRED_SHADING,
    DEFERRED_LIGHT,
    SKYBOX
};

/**
 * @brief An OpenGLWidget's render state.
 *
 * Possible values:
 *  - WIDGET_CREATED (0)
 *  - INITIALIZING (1)
 *  - INITIALIZED (2)
 *  - PREPARING_TO_DRAW (3)
 *  - MODELS (4)
 *  - BORDERED_MODELS (5)
 *  - BORDERS (6)
 *  - POST_PROCESSING (7)
 *  - FINISHED (8)
 */
enum RENDER_STATE : int {
    WIDGET_CREATED,
    INITIALIZING,
    INITIALIZED,
    PREPARING_TO_DRAW,
    MODELS,
    BORDERED_MODELS,
    BORDERS,
    POST_PROCESSING,
    FINISHED
};

struct GLLight {
    bool isActive;
    QVector3D Position;
    QVector3D Color;
    float Intensity;
    float MinRange;
    float Constant;
    float Linear;
    float Quadratic;
    // dependent values
    float radius;
    float maxBrightness;
    bool updated;
};

/**
 * @brief The OpenGLWidget class
 *
 * A custom OpenGLWidget in which to display the papercraft's model.
 */
class OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public: // METHODS
    OpenGLWidget(QWidget *parent = nullptr);
    ~OpenGLWidget() override;

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    void Tick();

    void DrawMesh(GLMesh* mesh = nullptr, GLSHADER_TYPE shader = DEFAULT);
    void LoadSubMesh(GLSubMesh* mesh = nullptr);

    void ResetLights();
    void AddLightAtCamPos();

protected: // METHODS
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private: // METHODS

    void RenderQuad();
//    void RenderCube();
//    void RenderSkybox();
//    int LoadSkyboxes(const QVector<QString> paths);
    void LoadShaders();
    void LoadFramebuffer();
    void DrawBordered();
//    void PostProcessDeferredLights();

public: // MEMBERS

    // Scene
    GLScene* scene = nullptr;

    // Scene camera
    GLTransform* camera = nullptr;
    QVector3D cam_focus;
    float zoom = 45.0f;

    // Scene light
    QList<GLLight> lights;
    bool camera_light_follow = true;
    int current_light = 1;

    // Rendering options
    bool use_deferred = false;
    bool draw_borders = true;
    float mode = 8;
    int current_skybox = 0;

    // Stencil border
    float border_scale = 1.1f;
    QVector3D border_color;
    float border_alpha = 0.8f;
    bool border_over_borderless = true;

    // Skybox
    bool renderSkybox = true;

private: // MEMBERS

    // Shaders
    QVector<QOpenGLShaderProgram*> programs;

    // OpenGL render state
    RENDER_STATE state;

    // Camera
    QPointF mouse_pos;
    bool cam_dir[6];
    bool orbiting = false;

    // Time control
    QTimer* timer = nullptr;
    int tick_count = 0;
    float tick_period = 3.0f;

    // Context dimensions
    int width, height;

    // Skybox
    QVector<QOpenGLTexture*> skyboxes;

    // Frame buffer
    unsigned int fbo;
    unsigned int gPosition, gNormal, gAlbedoSpec;
    unsigned int rboDepth;

    // screen quad
    unsigned int quadVAO = 0;
    unsigned int quadVBO;

    // Border stack vector
    QVector<GLMesh*> border_meshes;

    // Shader uniform locations
    QMatrix4x4 m_proj;
    // Default shader values
    int d_mvMatrixLoc;
    int d_normalMatrixLoc;
    int d_projMatrixLoc;
    int d_lightPosLoc;
    int d_lightIntensityLoc;
    int d_textureLoc;
    int d_modeLoc;
    // Single Color
    int sc_modelView;
    int sc_proj;
    int sc_color;
    int sc_alpha;
    // Framebuffer to screen
    int fs_screenTexture;
    // Graphic Buffer
    // Deferred Shading
    // Lighting Pass
    int def_posLoc;
    int def_normalLoc;
    int def_albedospecLoc;

    // Max Lights
    const int NR_LIGHTS = 32;
};

#endif // OPENGLWIDGET_H
