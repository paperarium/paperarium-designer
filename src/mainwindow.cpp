#include "mainwindow.h"
#include "ui_mainwindow.h"

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
}

MainWindow::~MainWindow()
{
    delete ui;
}

