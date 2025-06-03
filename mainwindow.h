#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include "filterswindow.h"  // Asegúrate de incluir la clase FiltersWindow
#include <opencv2/opencv.hpp>

// Incluir ITK
#include "itkImage.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
using namespace cv;
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // Funciones de conversión (pueden estar públicas o privadas)
    Mat QImageToMat(const QImage &image);
    QImage MatToQImage(const Mat &mat);

private slots:
    void on_loadFile_clicked();    // Cargar archivo 1
    void on_loadFile2_clicked();   // Cargar archivo 2
    void on_sliceSlider_valueChanged(int value);  // Slider común
    void on_checkBoxShowMessage_toggled(bool checked);

private:
    FiltersWindow *filtersWindow;  // Puntero a FiltersWindow
    Ui::MainWindow *ui;

    // Rutas archivos
    QString niftiPath1;
    QString niftiPath2;

    int currentSlice = 0;
    int totalSlices = 0;  // mínimo entre las dos imágenes

    // Tipos ITK
    using ImageType3D = itk::Image<float, 3>;
    using ImageType2D = itk::Image<float, 2>;

    // Punteros a las imágenes cargadas
    ImageType3D::Pointer imagen3D_1;
    ImageType3D::Pointer imagen3D_2;

    // Funciones auxiliares
    void mostrarImagenProcesada(int sliceIndex);
    ImageType3D::Pointer cargarImagenITK(const QString &path);
    QImage extraerSliceComoQImage(ImageType3D::Pointer imagen, int sliceIndex);
    void actualizarTotalSlices();
    void mostrarSegmentacion(int sliceIndex);
};

#endif // MAINWINDOW_H
