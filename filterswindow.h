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

    void setVolumes(ImageType3D::Pointer volumen, ImageType3D::Pointer volumen2);

private slots:
    void on_sliceSlider_valueChanged(int value);  // El nombre y firma deben coincidir
    void on_DescHE_clicked();

    void on_DescIS_clicked();

    void on_DescMO_clicked();

    void on_DescED_clicked();

    void on_DescPM_clicked();

    void on_DescBT_clicked();

    void on_DescBC_clicked();

    void on_DescON_clicked();

    void on_DescOA_clicked();

    void on_DescOO_clicked();

    void on_DescPF_clicked();

private:
    Ui::filterswindow *ui;
    descriptionwindow *descWindow;

    ImageType3D::Pointer volumen3D_1;
    ImageType3D::Pointer volumen3D_2;
    int totalSlices = 0;

    void mostrarSlice(int sliceIndex);
    QImage extraerSliceComoQImage(int sliceIndex,ImageType3D::Pointer volumen);
    void MatToQImage();
};

#endif // FILTERSWINDOW_H
