#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QImage>
#include <QPixmap>
#include "statswindow.h"

// ITK
#include "itkImageFileReader.h"
#include "itkExtractImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

// OpenCV
#include <opencv2/opencv.hpp>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), currentSlice(0), statsWindow(nullptr) {
    ui->setupUi(this);

    filtersWindow = new FiltersWindow(this);

    connect(ui->checkBoxShowMessage, &QCheckBox::toggled, this, &MainWindow::on_checkBoxShowMessage_toggled);
    QString rutaImagen = "/home/henryg/Imágenes/Logo_Universidad_Politécnica_Salesiana_del_Ecuador.png";
    QPixmap pixmap(rutaImagen);
    if (!pixmap.isNull()) {
        ui->labelUPS->setPixmap(pixmap.scaled(ui->labelUPS->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        QMessageBox::warning(this, "Error", "No se pudo cargar la imagen.");
    }
}

MainWindow::~MainWindow() {
    delete ui;
    delete statsWindow;
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
    mostrarSegmentacion(value);

    if (statsWindow && statsWindow->isVisible()) {
        statsWindow->setImagenes(imagen3D_1, imagen3D_2, currentSlice);
        statsWindow->actualizarSegunContexto();
    }
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

cv::Mat MainWindow::QImageToMat(const QImage &image) {
    // Suponemos que la imagen es de formato Grayscale8 (como en tu caso)
    return cv::Mat(image.height(), image.width(), CV_8UC1,
                   const_cast<uchar*>(image.bits()), image.bytesPerLine()).clone();
}

QImage MainWindow::MatToQImage(const cv::Mat &mat) {
    // Asegúrate que mat es tipo CV_8UC3 (BGR)
    if (mat.type() == CV_8UC3) {
        // Convertir de BGR a RGB
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        // Crear QImage desde datos rgb
        return QImage(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step), QImage::Format_RGB888).copy();
    } else if (mat.type() == CV_8UC1) {
        // Imagen en escala de grises
        return QImage(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_Grayscale8).copy();
    } else {
        // Otros formatos: no soportados aquí
        return QImage();
    }
}

void MainWindow::mostrarSegmentacion(int sliceIndex) {
    if (!imagen3D_1 || !imagen3D_2) return;

    // Extraer slice t1ce como QImage (grayscale)
    QImage qImgT1ce = extraerSliceComoQImage(imagen3D_1, sliceIndex);

    // Extraer slice segmentación como QImage (grayscale)
    QImage qImgSeg = extraerSliceComoQImage(imagen3D_2, sliceIndex);

    // Convertir QImage a cv::Mat (escala de grises)
    cv::Mat matT1ce = QImageToMat(qImgT1ce);
    cv::Mat matSeg = QImageToMat(qImgSeg);

    // Crear máscara binaria con un umbral
    cv::Mat maskBinaria;
    cv::threshold(matSeg, maskBinaria, 10, 255, cv::THRESH_BINARY);

    // Convertir t1ce a imagen color (BGR) para hacer overlay
    cv::Mat matT1ceColor;
    cv::cvtColor(matT1ce, matT1ceColor, cv::COLOR_GRAY2BGR);

    // Superponer color rojo semi-transparente donde la máscara es blanca
    for (int y = 0; y < matT1ceColor.rows; ++y) {
        for (int x = 0; x < matT1ceColor.cols; ++x) {
            if (maskBinaria.at<uchar>(y, x) > 0) {
                cv::Vec3b &pixel = matT1ceColor.at<cv::Vec3b>(y, x);
                pixel[0] = static_cast<uchar>(pixel[0] * 0.5);          // Azul
                pixel[1] = static_cast<uchar>(pixel[1] * 0.5);          // Verde
                pixel[2] = std::min(255, pixel[2] / 2 + 128);           // Rojo más intenso
            }
        }
    }

    // Convertir de nuevo a QImage
    QImage qImgOverlay = MatToQImage(matT1ceColor);

    // Mostrar las imágenes en cada label
    ui->labelImagen->setPixmap(QPixmap::fromImage(qImgT1ce).scaled(ui->labelImagen->size(), Qt::KeepAspectRatio));
    ui->labelImagen2->setPixmap(QPixmap::fromImage(qImgSeg).scaled(ui->labelImagen2->size(), Qt::KeepAspectRatio));
    ui->labelImagen3->setPixmap(QPixmap::fromImage(qImgOverlay).scaled(ui->labelImagen3->size(), Qt::KeepAspectRatio));
}

void MainWindow::on_btnEstadisticas_clicked() {
    if (!imagen3D_1 || !imagen3D_2) {
        QMessageBox::warning(this, "Advertencia", "Carga ambas imágenes antes de mostrar estadísticas.");
        return;
    }

    if (!statsWindow) {
        statsWindow = new StatsWindow(nullptr);
    }

    statsWindow->setImagenes(imagen3D_1, imagen3D_2, currentSlice);
    statsWindow->show();
}


