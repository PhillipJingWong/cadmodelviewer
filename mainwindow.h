#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
//#include <QMessageBox>

#include <vtkSmartPointer.h>
#include <vtkCamera.h>

class vtkGenericOpenGLRenderWindow;  // Forward declaration
class vtkRenderer;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void handleAddButton();
    void handleResetButton();
    //void handleTestButton();
    void updateCameraPan();

signals:
    void statusUpdateMessage(const QString &message, int timeout);

private:
    Ui::MainWindow *ui;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;
    vtkSmartPointer<vtkRenderer> renderer;
    vtkSmartPointer<vtkCamera> defaultCamera;
    double baseDistance;
    double baseCameraPosition[3];
    double baseFocalPoint[3];

};
#endif // MAINWINDOW_H
