#ifndef FILTERSWINDOW_H
#define FILTERSWINDOW_H

#include <QDialog>
#include <QImage>

#include "itkImage.h"

#include "ui_filterswindow.h"
#include "descriptionwindow.h"

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
    void on_DescHE_clicked();

    void on_DescIS_clicked();

    void on_DescMO_clicked();

    void on_DescED_clicked();

    void on_DescPM_clicked();

    void on_DescBT_clicked();

    void on_DescBC_clicked();

private:
    Ui::filterswindow *ui;
    descriptionwindow *descWindow;

    ImageType3D::Pointer volumen3D_1;
    int totalSlices = 0;

    void mostrarSlice(int sliceIndex);
    QImage extraerSliceComoQImage(int sliceIndex);
    void MatToQImage();
};

#endif // FILTERSWINDOW_H
