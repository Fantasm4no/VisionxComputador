#ifndef FILTERSWINDOW_H
#define FILTERSWINDOW_H

#include <QDialog>
#include <QImage>

#include "itkImage.h"

#include "ui_filterswindow.h"

using ImageType3D = itk::Image<float, 3>;
using ImageType2D = itk::Image<float, 2>;

class FiltersWindow : public QDialog {
    Q_OBJECT

public:
    explicit FiltersWindow(QWidget *parent = nullptr);
    ~FiltersWindow();

    void setVolume(ImageType3D::Pointer volumen);

private slots:
    void on_sliceSlider_valueChanged(int value);  // El nombre y firma deben coincidir

private:
    Ui::filterswindow *ui;

    ImageType3D::Pointer volumen3D;
    int totalSlices = 0;

    void mostrarSlice(int sliceIndex);
    QImage extraerSliceComoQImage(int sliceIndex);
};

#endif // FILTERSWINDOW_H
