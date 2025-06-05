#include "filterswindow.h"
#include "ui_filterswindow.h"
#include <QMessageBox>

#include "itkExtractImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

#include <opencv2/opencv.hpp>

FiltersWindow::FiltersWindow(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::filterswindow)
{
    ui->setupUi(this);
    descWindow = new descriptionwindow(this);
    descWindow->setWindowModality(Qt::WindowModal);
    QString rutaImagen = "/home/henryg/Imágenes/Logo_Universidad_Politécnica_Salesiana_del_Ecuador.png"; // Cambia esta ruta
    QPixmap pixmap(rutaImagen);
    if (!pixmap.isNull()) {
        ui->labelUPS->setPixmap(pixmap.scaled(ui->labelUPS->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        QMessageBox::warning(this, "Error", "No se pudo cargar la imagen.");
    }
}

FiltersWindow::~FiltersWindow() {
    delete ui;
}

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
    // Convertir QImage a cv::Mat
    cv::Mat original = QImageToMat(input);

    // Crear kernel para morfología
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(15, 15));

    // Operaciones morfológicas Top Hat y Black Hat
    cv::Mat topHat, blackHat, morphResult;
    cv::morphologyEx(original, topHat, cv::MORPH_TOPHAT, kernel);
    cv::morphologyEx(original, blackHat, cv::MORPH_BLACKHAT, kernel);

    // Combinar resultados y sumar a original
    cv::Mat enhanced;
    cv::subtract(topHat, blackHat, morphResult);
    cv::add(original, morphResult, enhanced);

    // Normalizar entre 0 y 255
    cv::normalize(enhanced, enhanced, 0, 255, cv::NORM_MINMAX, CV_8UC1);

    // Crear y aplicar CLAHE
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
    cv::Mat claheResult;
    clahe->apply(enhanced, claheResult);

    // Convertir resultado a QImage y retornar
    return MatToQImage(claheResult);
}

QImage aplicarThreshold(const QImage &input, int umbral = 128) {
    cv::Mat img = QImageToMat(input);
    cv::Mat result;
    cv::threshold(img, result, umbral, 255, cv::THRESH_BINARY);
    return MatToQImage(result);
}

QImage aplicarBinarizacionColor(const QImage &input, int low=100, int high=255) {
    cv::Mat img = QImageToMat(input);
    cv::Mat result;
    cv::inRange(img, cv::Scalar(low), cv::Scalar(high), result);
    return MatToQImage(result);
}

void FiltersWindow::setVolume(ImageType3D::Pointer volumen) {
    volumen3D_1 = volumen;
    if (!volumen3D_1) return;

    totalSlices = volumen3D_1->GetLargestPossibleRegion().GetSize()[2];
    ui->sliceSlider->setMaximum(totalSlices - 1);
    ui->sliceSlider->setValue(0);

    mostrarSlice(0);
}

void FiltersWindow::on_sliceSlider_valueChanged(int value) {
    mostrarSlice(value);
}

void FiltersWindow::mostrarSlice(int sliceIndex) {
    if (!volumen3D_1) return;

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

    ui->imageLabelUB->setPixmap(QPixmap::fromImage(
                                    aplicarThreshold(original)).scaled(ui->imageLabelUB->size(), Qt::KeepAspectRatio));

    ui->imageLabelBR->setPixmap(QPixmap::fromImage(
                                    aplicarBinarizacionColor(original)).scaled(ui->imageLabelBR->size(), Qt::KeepAspectRatio));
}

QImage FiltersWindow::extraerSliceComoQImage(int sliceIndex) {
    if (!volumen3D_1) return QImage();

    using ExtractFilterType = itk::ExtractImageFilter<ImageType3D, ImageType2D>;
    using UCharImageType = itk::Image<unsigned char, 2>;
    using RescaleFilterType = itk::RescaleIntensityImageFilter<ImageType2D, UCharImageType>;

    ExtractFilterType::Pointer extractor = ExtractFilterType::New();

    ImageType3D::RegionType inputRegion = volumen3D_1->GetLargestPossibleRegion();
    ImageType3D::SizeType size = inputRegion.GetSize();
    ImageType3D::IndexType start = inputRegion.GetIndex();

    size[2] = 0;
    start[2] = sliceIndex;

    ImageType3D::RegionType desiredRegion;
    desiredRegion.SetSize(size);
    desiredRegion.SetIndex(start);

    extractor->SetExtractionRegion(desiredRegion);
    extractor->SetInput(volumen3D_1);
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

void FiltersWindow::on_DescHE_clicked()
{
    QString texto = "Histogram Equalization improves contrast using the gray level distribution.\n"
                    "In OpenCV, cv::equalizeHist is used to implement it.";
    descWindow->setDescription(texto);
    descWindow->show();
}

void FiltersWindow::on_DescIS_clicked()
{
    QString texto = "Image Smoothing reduces noise by smoothing the image.\n"
                    "In OpenCV, cv::GaussianBlur is used to apply a Gaussian filter.";
    descWindow->setDescription(texto);
    descWindow->show();
}


void FiltersWindow::on_DescMO_clicked()
{
    QString texto = "Morphological Operation applies shape-based transformations.\n"
                    "In OpenCV, functions like cv::morphologyEx are used for top hat, black hat, etc. operations.";
    descWindow->setDescription(texto);
    descWindow->show();
}


void FiltersWindow::on_DescED_clicked()
{
    QString texto = "Edge Detection detects edges by highlighting abrupt changes in intensity.\n"
                    "In OpenCV, cv::Canny is used for edge detection.";
    descWindow->setDescription(texto);
    descWindow->show();
}


void FiltersWindow::on_DescPM_clicked()
{
    QString texto = "Pixel Manipulation modifies pixel values ​​to improve contrast or brightness.\n"
                    "In OpenCV, cv::convertTo is used to adjust contrast and brightness.";
    descWindow->setDescription(texto);
    descWindow->show();
}


void FiltersWindow::on_DescBT_clicked()
{
    QString texto = "Basic Thresholding segments the image using a threshold to separate regions.\n"
                    "In OpenCV, cv::threshold is used to apply fixed or adaptive thresholding.";
    descWindow->setDescription(texto);
    descWindow->show();
}


void FiltersWindow::on_DescBC_clicked()
{
    QString texto = "Binarization By Color segments by color range to extract specific regions. \n"
                    "In OpenCV, cv::inRange is used for color binarization.";
    descWindow->setDescription(texto);
    descWindow->show();
}

