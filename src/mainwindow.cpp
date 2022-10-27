#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "openglwidget.h"
#include "glscene.h"

/**
 * Builds the window.
 *
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // initialize the UI
    ui->setupUi(this);

    setFocusPolicy(Qt::ClickFocus);

    // window meta information
    setWindowTitle(tr("Paperarium Design"));
    setObjectName("main_window");

    // window attributes
    setAttribute(Qt::WA_QuitOnClose);
    setAttribute(Qt::WA_TranslucentBackground);

    // create a QOpenGLcontext
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setOption(QSurfaceFormat::DebugContext);
    QSurfaceFormat::setDefaultFormat(format);

    // add openGLwidget
    openGLWidget = new OpenGLWidget(ui->openGLWidget);
    openGLWidget->setFormat(format);
    openGLWidget->scene = new GLScene();
    openGLWidget->scene->mainWindow = this;
}

MainWindow::~MainWindow()
{
    delete openGLWidget;
    delete ui;
}

