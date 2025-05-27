#include "filterswindow.h"
#include "ui_filterswindow.h"

#include "itkExtractImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

#include <opencv2/opencv.hpp>

// QImage → cv::Mat
cv::Mat QImageToMat(const QImage &image) {
    return cv::Mat(image.height(), image.width(), CV_8UC1,
                   const_cast<uchar*>(image.bits()), image.bytesPerLine()).clone();
}

// cv::Mat → QImage
QImage MatToQImage(const cv::Mat &mat) {
    QImage image(mat.cols, mat.rows, QImage::Format_Grayscale8);
    for (int y = 0; y < mat.rows; ++y)
        for (int x = 0; x < mat.cols; ++x)
            image.setPixel(x, y, qRgb(mat.at<uchar>(y, x), mat.at<uchar>(y, x), mat.at<uchar>(y, x)));
    return image;
}

// Filtros aplicados

QImage aplicarEcualizacionHistograma(const QImage &input) {
    cv::Mat img = QImageToMat(input);
    cv::Mat result;
    cv::equalizeHist(img, result);
    return MatToQImage(result);
}

QImage aplicarSuavizadoGaussiano(const QImage &input) {
    cv::Mat img = QImageToMat(input);
    cv::Mat result;
    cv::GaussianBlur(img, result, cv::Size(5, 5), 1.5);
    return MatToQImage(result);
}

QImage aplicarDeteccionBordes(const QImage &input) {
    cv::Mat img = QImageToMat(input);
    cv::Mat result;
    cv::Canny(img, result, 100, 200);
    return MatToQImage(result);
}

QImage aplicarManipulacionPixeles(const QImage &input) {
    cv::Mat img = QImageToMat(input);
    cv::Mat stretched;
    img.convertTo(stretched, -1, 1.5, 20);  // contraste + brillo
    return MatToQImage(stretched);
}

QImage aplicarMorfologiaTopHat(const QImage &input) {
    cv::Mat img = QImageToMat(input);
    cv::Mat result;
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(11, 11));
    cv::morphologyEx(img, result, cv::MORPH_TOPHAT, kernel);
    return MatToQImage(result);
}

FiltersWindow::FiltersWindow(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::filterswindow)
{
    ui->setupUi(this);
}

FiltersWindow::~FiltersWindow() {
    delete ui;
}

void FiltersWindow::setVolume(ImageType3D::Pointer volumen) {
    volumen3D = volumen;
    if (!volumen3D) return;

    totalSlices = volumen3D->GetLargestPossibleRegion().GetSize()[2];
    ui->sliceSlider->setMaximum(totalSlices - 1);
    ui->sliceSlider->setValue(0);

    mostrarSlice(0);
}

void FiltersWindow::on_sliceSlider_valueChanged(int value) {
    mostrarSlice(value);
}

void FiltersWindow::mostrarSlice(int sliceIndex) {
    if (!volumen3D) return;

    QImage original = extraerSliceComoQImage(sliceIndex);

    ui->imageLabelEH->setPixmap(QPixmap::fromImage(
                                    aplicarEcualizacionHistograma(original)).scaled(ui->imageLabelEH->size(), Qt::KeepAspectRatio));

    ui->imageLabelSI->setPixmap(QPixmap::fromImage(
                                    aplicarSuavizadoGaussiano(original)).scaled(ui->imageLabelSI->size(), Qt::KeepAspectRatio));

    ui->imageLabelDB->setPixmap(QPixmap::fromImage(
                                    aplicarDeteccionBordes(original)).scaled(ui->imageLabelDB->size(), Qt::KeepAspectRatio));

    ui->imageLabelMP->setPixmap(QPixmap::fromImage(
                                    aplicarManipulacionPixeles(original)).scaled(ui->imageLabelMP->size(), Qt::KeepAspectRatio));

    ui->imageLabelOM->setPixmap(QPixmap::fromImage(
                                    aplicarMorfologiaTopHat(original)).scaled(ui->imageLabelOM->size(), Qt::KeepAspectRatio));
}

QImage FiltersWindow::extraerSliceComoQImage(int sliceIndex) {
    if (!volumen3D) return QImage();

    using ExtractFilterType = itk::ExtractImageFilter<ImageType3D, ImageType2D>;
    using UCharImageType = itk::Image<unsigned char, 2>;
    using RescaleFilterType = itk::RescaleIntensityImageFilter<ImageType2D, UCharImageType>;

    ExtractFilterType::Pointer extractor = ExtractFilterType::New();

    ImageType3D::RegionType inputRegion = volumen3D->GetLargestPossibleRegion();
    ImageType3D::SizeType size = inputRegion.GetSize();
    ImageType3D::IndexType start = inputRegion.GetIndex();

    size[2] = 0;
    start[2] = sliceIndex;

    ImageType3D::RegionType desiredRegion;
    desiredRegion.SetSize(size);
    desiredRegion.SetIndex(start);

    extractor->SetExtractionRegion(desiredRegion);
    extractor->SetInput(volumen3D);
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
