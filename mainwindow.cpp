#include "mainwindow.h"
#include "./ui_mainwindow.h"

//#include <QMessageBox>
#include <QFileDialog>

#include <cmath>

#include <QVTKOpenGLNativeWidget.h>
#include <vtkNew.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
//#include <vtkRenderWindowInteractor.h>
//#include <vtkNamedColors.h>
//#include <vtkConeSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkCamera.h>

#include <vtkSTLReader.h>
#include <vtkProperty.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Set slider ranges for full orbit control
    ui->horizontalSlider->setRange(0, 200);  // -π to π azimuth
    ui->zoomSlider->setRange(50, 150);       // 0.5x to 1.5x zoom
    ui->verticalSlider->setRange(0, 200);    // -π/2 to π/2 elevation

    int hMid = (ui->horizontalSlider->minimum() + ui->horizontalSlider->maximum()) / 2;
    ui->horizontalSlider->setValue(hMid);

    int zMid = (ui->zoomSlider->minimum() + ui->zoomSlider->maximum()) / 2;
    ui->zoomSlider->setValue(zMid);

    int vMid = (ui->verticalSlider->minimum() + ui->verticalSlider->maximum()) / 2;
    ui->verticalSlider->setValue(vMid);

    //connect released() signal of addbutton object to handleaddbutton slot
    connect(ui->selectButton, &QPushButton::released, this, &MainWindow::handleAddButton);
    //connect(ui->pushButton_3, &QPushButton::released, this, &MainWindow::handleTestButton);
    //connect(ui->actionAdd, &QAction::triggered, this, &MainWindow::handleAddButton);

    connect(ui->horizontalSlider, &QSlider::valueChanged, this, &MainWindow::updateCameraPan);
    connect(ui->zoomSlider, &QSlider::valueChanged, this, &MainWindow::updateCameraPan);
    connect(ui->verticalSlider, &QSlider::valueChanged, this, &MainWindow::updateCameraPan);

    connect(ui->resetButton, &QPushButton::released, this, &MainWindow::handleResetButton);

    //connect statusupdatemessage signal to showmessage slot of statusbar
    connect(this, &MainWindow::statusUpdateMessage, ui->statusbar, &QStatusBar::showMessage);

    // Example in constructor or setup code
    // Initialize VTK render window
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow =
        vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();

    ui->vtkWidget->setRenderWindow(renderWindow);  // vtkWidget was promoted to QVTKOpenGLNativeWidget

    // Create a renderer and attach to the render window
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow->AddRenderer(renderer);

    // Add actors here...

    renderer->SetBackground(0.1, 0.2, 0.4); // just a background color for testing
    renderer->ResetCamera();

    // Store them as member variables to reuse later:
    this->renderWindow = renderWindow;
    this->renderer = renderer;

    renderer->GetActiveCamera()->GetPosition(baseCameraPosition);
    renderer->GetActiveCamera()->GetFocalPoint(baseFocalPoint);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::handleAddButton() {

    QString defaultDir = "/Users/phillipwong/Documents";  // Set your desired default folder here

    QFileDialog dialog(this, tr("Open STL File"), defaultDir);
    dialog.setNameFilter(tr("STL Files (*.stl)"));
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setOption(QFileDialog::DontUseNativeDialog);  // Force Qt dialog (non-native)

    if (dialog.exec() != QDialog::Accepted)
        return;

    QStringList selectedFiles = dialog.selectedFiles();
    if (selectedFiles.isEmpty())
        return;

    QString filename = selectedFiles.first();

    vtkNew<vtkSTLReader> reader;
    reader->SetFileName(filename.toStdString().c_str());
    reader->Update();

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(reader->GetOutputPort());

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    actor->GetProperty()->SetDiffuse(0.8);
    actor->GetProperty()->SetDiffuseColor(0.69, 0.77, 0.87);
    actor->GetProperty()->SetSpecular(0.3);
    actor->GetProperty()->SetSpecularPower(60.0);

    this->renderer->RemoveAllViewProps();
    this->renderer->AddActor(actor);
    this->renderer->ResetCamera();
    this->renderWindow->Render();

    // Disable mouse interaction directly with the model
    this->renderWindow->GetInteractor()->Disable();

    renderer->GetActiveCamera()->GetPosition(baseCameraPosition);
    renderer->GetActiveCamera()->GetFocalPoint(baseFocalPoint);

    baseDistance = sqrt(
        pow(baseCameraPosition[0] - baseFocalPoint[0], 2) +
        pow(baseCameraPosition[1] - baseFocalPoint[1], 2) +
        pow(baseCameraPosition[2] - baseFocalPoint[2], 2)
        );

    // Reset sliders to default camera position
    ui->zoomSlider->setValue(100);         // Original zoom
    ui->horizontalSlider->setValue(100);   // No horizontal rotation
    ui->verticalSlider->setValue(100);     // No vertical rotation (if using one)

    emit statusUpdateMessage(tr("Loaded STL: %1").arg(filename), 5000);
}

void MainWindow::handleResetButton() {
    if (defaultCamera != nullptr) {
        renderer->GetActiveCamera()->DeepCopy(defaultCamera);
        renderer->ResetCameraClippingRange();
        renderWindow->Render();
    }

    ui->horizontalSlider->setValue(100);  // neutral (no rotation)
    ui->zoomSlider->setValue(100);    // original zoom (1.0x)
    ui->verticalSlider->setValue(100);
    emit statusUpdateMessage(QString("Reset clicked"), 0);
}

/*
void MainWindow::handleTestButton() {
    qDebug() << "Test button clicked!";

    emit statusUpdateMessage(QString("Test was clicked"), 0);
}*/

void MainWindow::updateCameraPan() {
    vtkCamera* camera = renderer->GetActiveCamera();

    int hVal = ui->horizontalSlider->value();  // 0–200
    int vVal = ui->verticalSlider->value();    // 0–200 (for tilt)
    int zVal = ui->zoomSlider->value();        // 50–150

    // Map sliders to spherical coordinates
    double azimuth = ((hVal - 100) / 100.0) * M_PI;         // -π to π
    double elevation = ((vVal - 100) / 100.0) * M_PI;       // -π to π (full 360 degrees)
    double zoomFactor = zVal / 100.0;                       // 0.5 to 1.5
    double radius = baseDistance * zoomFactor;

    // Spherical to Cartesian conversion
    double x = baseFocalPoint[0] + radius * cos(elevation) * cos(azimuth);
    double y = baseFocalPoint[1] + radius * cos(elevation) * sin(azimuth);
    double z = baseFocalPoint[2] + radius * sin(elevation);

    camera->SetPosition(x, y, z);
    camera->SetFocalPoint(baseFocalPoint);

    // Flip ViewUp when camera goes upside down
    // elevation between ±90° and ±270° => flip ViewUp
    double elevationDegrees = elevation * 180.0 / M_PI;

    if (elevationDegrees > 90.0 || elevationDegrees < -90.0) {
        // Flip ViewUp vector (pointing down)
        camera->SetViewUp(0, 0, -1);
    } else {
        // Normal ViewUp vector (pointing up)
        camera->SetViewUp(0, 0, 1);
    }

    renderer->ResetCameraClippingRange();
    renderWindow->Render();
}
