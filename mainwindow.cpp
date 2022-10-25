#include "mainwindow.h"
#include "./ui_mainwindow.h"

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

    // window meta information
    setWindowTitle(tr("Paperarium Design"));
    setObjectName("main_window");

    // window attributes
    setAttribute(Qt::WA_QuitOnClose);
    setAttribute(Qt::WA_TranslucentBackground);

    // window flags
//    setWindowFlags(Qt::FramelessWindowHint);
    // initial size of the window
    setFixedSize(1100, 650);
}

MainWindow::~MainWindow()
{
    delete ui;
}

