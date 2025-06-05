// statswindow.h (debes añadir esto en la sección de atributos de clase)
#ifndef STATSWINDOW_H
#define STATSWINDOW_H

#include <QWidget>
#include "itkImage.h"
#include "mainwindow.h"
#include "ui_statswindow.h"

using ImageType3D = itk::Image<float, 3>;
using ImageType2D = itk::Image<float, 2>;

namespace Ui {
class StatsWindow;
}

class StatsWindow : public QWidget {
    Q_OBJECT

public:
    explicit StatsWindow(QWidget *parent = nullptr);
    ~StatsWindow();
    void setImagenes(ImageType3D::Pointer img1, ImageType3D::Pointer img2, int slice);
    void actualizarTodo();
    void setSliceIndex(int index);
    void actualizarSegunContexto();

private slots:
    void calcularMedia();
    void calcularMediana();
    void calcularDesviacionEstandar();
    void calcularMinMax();
    void calcularArea();
    void detectarOutliers();
    void mostrarBoxplot();
    void guardarEstadisticas();

private:
    Ui::StatsWindow *ui;
    ImageType3D::Pointer imagen1;
    ImageType3D::Pointer imagen2;
    int sliceIndex;

    double ultimaMedia = 0;
    double ultimaMediana = 0;
    double ultimaDesviacion = 0;
    double ultimoMin = 0;
    double ultimoMax = 0;
    double ultimaArea = 0;
    int cantidadOutliers = 0;

    std::vector<float> extraerValoresDeInteres();
    void calcularTodasLasEstadisticas();

    enum TipoEstadistica {
        NINGUNA,
        MEDIA,
        MEDIANA,
        DESVIACION,
        MINMAX,
        AREA,
        OUTLIERS,
        BOXPLOT
    };

    TipoEstadistica ultimaEstadistica = NINGUNA;
};

#endif // STATSWINDOW_H
