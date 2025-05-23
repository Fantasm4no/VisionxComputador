#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QImage>
#include <QPixmap>

// ITK
#include "itkImageFileReader.h"
#include "itkExtractImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

// OpenCV
#include <opencv2/opencv.hpp>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), currentSlice(0) {
    ui->setupUi(this);

    filtersWindow = new FiltersWindow(this);

    connect(ui->checkBoxShowMessage, &QCheckBox::toggled, this, &MainWindow::on_checkBoxShowMessage_toggled);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_checkBoxShowMessage_toggled(bool checked) {
    if (!filtersWindow) return;

    if (checked) {
        if (imagen3D_1) {
            filtersWindow->setVolume(imagen3D_1);
            filtersWindow->show();
        } else {
            QMessageBox::warning(this, "Advertencia", "Primero carga una imagen.");
            ui->checkBoxShowMessage->setChecked(false);
        }
    } else {
        filtersWindow->close();
    }
}


void MainWindow::on_loadFile_clicked() {
    niftiPath1 = QFileDialog::getOpenFileName(this, "Cargar archivo NIfTI 1", "", "NIfTI files (*.nii *.nii.gz)");
    if (niftiPath1.isEmpty()) return;

    ui->labelRuta->setText("Imagen 1 cargada correctamente.");

    using ReaderType = itk::ImageFileReader<ImageType3D>;
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(niftiPath1.toStdString());

    try {
        reader->Update();
        imagen3D_1 = reader->GetOutput();
    } catch (itk::ExceptionObject &error) {
        QMessageBox::critical(this, "Error", "No se pudo leer el archivo NIfTI 1.");
        return;
    }

    actualizarTotalSlices();
    mostrarImagenProcesada(currentSlice);
}

void MainWindow::on_loadFile2_clicked() {
    niftiPath2 = QFileDialog::getOpenFileName(this, "Cargar archivo NIfTI 2", "", "NIfTI files (*.nii *.nii.gz)");
    if (niftiPath2.isEmpty()) return;

    ui->labelRuta2->setText("Imagen 2 cargada correctamente.");

    using ReaderType = itk::ImageFileReader<ImageType3D>;
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(niftiPath2.toStdString());

    try {
        reader->Update();
        imagen3D_2 = reader->GetOutput();
    } catch (itk::ExceptionObject &error) {
        QMessageBox::critical(this, "Error", "No se pudo leer el archivo NIfTI 2.");
        return;
    }

    actualizarTotalSlices();
    mostrarImagenProcesada(currentSlice);
}

void MainWindow::actualizarTotalSlices() {
    if (!imagen3D_1 || !imagen3D_2) return;

    int slices1 = imagen3D_1->GetLargestPossibleRegion().GetSize()[2];
    int slices2 = imagen3D_2->GetLargestPossibleRegion().GetSize()[2];

    totalSlices = std::min(slices1, slices2);
    ui->sliceSlider->setMaximum(totalSlices - 1);

    if (currentSlice >= totalSlices)
        currentSlice = totalSlices - 1;

    ui->sliceSlider->setValue(currentSlice);
}

void MainWindow::on_sliceSlider_valueChanged(int value) {
    currentSlice = value;
    mostrarImagenProcesada(value);
}

void MainWindow::mostrarImagenProcesada(int sliceIndex) {
    if (!imagen3D_1 || !imagen3D_2) return;

    QImage img1 = extraerSliceComoQImage(imagen3D_1, sliceIndex);
    QImage img2 = extraerSliceComoQImage(imagen3D_2, sliceIndex);

    ui->labelImagen->setPixmap(QPixmap::fromImage(img1).scaled(ui->labelImagen->size(), Qt::KeepAspectRatio));
    ui->labelImagen2->setPixmap(QPixmap::fromImage(img2).scaled(ui->labelImagen2->size(), Qt::KeepAspectRatio));
}

QImage MainWindow::extraerSliceComoQImage(ImageType3D::Pointer imagen, int sliceIndex) {
    if (!imagen) return QImage();

    using ExtractFilterType = itk::ExtractImageFilter<ImageType3D, ImageType2D>;
    using UCharImageType = itk::Image<unsigned char, 2>;
    using RescaleFilterType = itk::RescaleIntensityImageFilter<ImageType2D, UCharImageType>;

    ExtractFilterType::Pointer extractor = ExtractFilterType::New();

    ImageType3D::RegionType inputRegion = imagen->GetLargestPossibleRegion();
    ImageType3D::SizeType size = inputRegion.GetSize();
    ImageType3D::IndexType start = inputRegion.GetIndex();

    size[2] = 0;
    start[2] = sliceIndex;

    ImageType3D::RegionType desiredRegion;
    desiredRegion.SetSize(size);
    desiredRegion.SetIndex(start);

    extractor->SetExtractionRegion(desiredRegion);
    extractor->SetInput(imagen);
    extractor->SetDirectionCollapseToSubmatrix();

    try {
        extractor->Update();
    } catch (itk::ExceptionObject &) {
        return QImage();
    }

    ImageType2D::Pointer slice = extractor->GetOutput();

    auto rescaler = RescaleFilterType::New();
    rescaler->SetInput(slice);
    rescaler->SetOutputMinimum(0);
    rescaler->SetOutputMaximum(255);

    try {
        rescaler->Update();
    } catch (itk::ExceptionObject &) {
        return QImage();
    }

    UCharImageType::Pointer rescaledSlice = rescaler->GetOutput();

    auto region2D = rescaledSlice->GetLargestPossibleRegion();
    auto size2D = region2D.GetSize();

    int width = size2D[0];
    int height = size2D[1];

    QImage image(width, height, QImage::Format_Grayscale8);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            itk::Index<2> idx = {{x, y}};
            unsigned char val = rescaledSlice->GetPixel(idx);
            image.setPixel(x, y, qRgb(val, val, val));
        }
    }
    return image;
}
