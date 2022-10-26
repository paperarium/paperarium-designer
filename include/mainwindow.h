#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class OpenGLWidget;

/**
 * @brief The MainWindow class
 *
 * Holds the main layout for the PaperariumDesign application.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    OpenGLWidget* openGLWidget = nullptr;
};
#endif // MAINWINDOW_H
