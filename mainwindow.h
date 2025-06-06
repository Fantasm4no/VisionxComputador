#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include "filterswindow.h"
#include <opencv2/opencv.hpp>

#include "itkImage.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
using namespace cv;
QT_END_NAMESPACE

class StatsWindow;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    Mat QImageToMat(const QImage &image);
    QImage MatToQImage(const Mat &mat);
    QImage aplicarFiltroPrewitt(const QImage &input);

private slots:
    void on_loadFile_clicked();
    void on_loadFile2_clicked();
    void on_sliceSlider_valueChanged(int value);
    void on_checkBoxShowMessage_toggled(bool checked);
    void on_btnEstadisticas_clicked();
    void generarVideo();

private:
    FiltersWindow *filtersWindow;
    StatsWindow *statsWindow;
    Ui::MainWindow *ui;

    QString niftiPath1;
    QString niftiPath2;
    QString imagenBaseName;

    int lastSliceIndex = -1;
    int currentSlice = 0;
    int totalSlices = 0;

    using ImageType3D = itk::Image<float, 3>;
    using ImageType2D = itk::Image<float, 2>;

    ImageType3D::Pointer imagen3D_1;
    ImageType3D::Pointer imagen3D_2;

    void mostrarImagenProcesada(int sliceIndex);
    ImageType3D::Pointer cargarImagenITK(const QString &path);
    QImage extraerSliceComoQImage(ImageType3D::Pointer imagen, int sliceIndex);
    void actualizarTotalSlices();
    void mostrarSegmentacion(int sliceIndex);
};

#endif // MAINWINDOW_H
