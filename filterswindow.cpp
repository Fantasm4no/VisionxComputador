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
    QString rutaImagen = "/home/f4ntasmano/Downloads/ups.png";
    QPixmap pixmap(rutaImagen);
    if (!pixmap.isNull()) {
        ui->labelUPS->setPixmap(pixmap.scaled(ui->labelUPS->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        QMessageBox::warning(this, "Error", "The image could not be loaded.");
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

QImage aplicarOperacionNOT(const QImage &input) {
    cv::Mat img = QImageToMat(input);
    cv::Mat result;
    cv::bitwise_not(img, result);  // Invertir los valores de la imagen
    return MatToQImage(result);
}

QImage aplicarOperacionAND(const QImage &input1, const QImage &input2) {
    cv::Mat img1 = QImageToMat(input1);
    cv::Mat img2 = QImageToMat(input2);
    cv::Mat result;
    cv::bitwise_and(img1, img2, result);  // Operación AND sobre las dos imágenes
    return MatToQImage(result);
}

QImage aplicarOperacionOR(const QImage &input1, const QImage &input2) {
    cv::Mat img1 = QImageToMat(input1);
    cv::Mat img2 = QImageToMat(input2);
    cv::Mat result;
    cv::bitwise_or(img1, img2, result);  // Operación OR sobre las dos imágenes
    return MatToQImage(result);
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

QImage aplicarFiltroPrewitt(const QImage &input) {
    cv::Mat img = QImageToMat(input);
    cv::Mat resultX, resultY, result;

    // Filtro Prewitt en X y Y
    cv::Mat kernelX = (cv::Mat_<float>(3, 3) << -1, 0, 1, -1, 0, 1, -1, 0, 1);
    cv::Mat kernelY = (cv::Mat_<float>(3, 3) << -1, -1, -1, 0, 0, 0, 1, 1, 1);

    cv::filter2D(img, resultX, CV_32F, kernelX);
    cv::filter2D(img, resultY, CV_32F, kernelY);

    // Magnitud de los bordes
    cv::magnitude(resultX, resultY, result);

    // Normalizar entre 0 y 255
    result.convertTo(result, CV_8UC1);

    return MatToQImage(result);
}

void FiltersWindow::setVolumes(ImageType3D::Pointer volumen1, ImageType3D::Pointer volumen2) {
    volumen3D_1 = volumen1;  // Primer volumen
    volumen3D_2 = volumen2;  // Segundo volumen

    if (!volumen3D_1 || !volumen3D_2) return;

    totalSlices = volumen3D_1->GetLargestPossibleRegion().GetSize()[2];
    ui->sliceSlider->setMaximum(totalSlices - 1);
    ui->sliceSlider->setValue(0);

    // Mostrar el primer slice de ambos volúmenes
    mostrarSlice(0);
}


void FiltersWindow::on_sliceSlider_valueChanged(int value) {
    mostrarSlice(value);
}

void FiltersWindow::mostrarSlice(int sliceIndex) {
    if (!volumen3D_1 || !volumen3D_2) return;

    QImage original1 = extraerSliceComoQImage(sliceIndex, volumen3D_1);
    QImage original2 = extraerSliceComoQImage(sliceIndex, volumen3D_2);

    ui->imageLabelEH->setPixmap(QPixmap::fromImage(
                                    aplicarEcualizacionHistograma(original1)).scaled(ui->imageLabelEH->size(), Qt::KeepAspectRatio));

    ui->imageLabelSI->setPixmap(QPixmap::fromImage(
                                    aplicarSuavizadoGaussiano(original1)).scaled(ui->imageLabelSI->size(), Qt::KeepAspectRatio));

    ui->imageLabelDB->setPixmap(QPixmap::fromImage(
                                    aplicarDeteccionBordes(original1)).scaled(ui->imageLabelDB->size(), Qt::KeepAspectRatio));

    ui->imageLabelMP->setPixmap(QPixmap::fromImage(
                                    aplicarManipulacionPixeles(original1)).scaled(ui->imageLabelMP->size(), Qt::KeepAspectRatio));

    ui->imageLabelOM->setPixmap(QPixmap::fromImage(
                                    aplicarMorfologiaTopHat(original1)).scaled(ui->imageLabelOM->size(), Qt::KeepAspectRatio));

    ui->imageLabelUB->setPixmap(QPixmap::fromImage(
                                    aplicarThreshold(original2)).scaled(ui->imageLabelUB->size(), Qt::KeepAspectRatio));

    ui->imageLabelBR->setPixmap(QPixmap::fromImage(
                                    aplicarBinarizacionColor(original2)).scaled(ui->imageLabelBR->size(), Qt::KeepAspectRatio));

    ui->imageLabelON->setPixmap(QPixmap::fromImage(
                                    aplicarOperacionNOT(original2)).scaled(ui->imageLabelON->size(), Qt::KeepAspectRatio));

    ui->imageLabelOA->setPixmap(QPixmap::fromImage(
                                    aplicarOperacionAND(original1,original2)).scaled(ui->imageLabelOA->size(), Qt::KeepAspectRatio));

    ui->imageLabelOO->setPixmap(QPixmap::fromImage(
                                    aplicarOperacionOR(original1,original2)).scaled(ui->imageLabelOO->size(), Qt::KeepAspectRatio));

    ui->imageLabelPF->setPixmap(QPixmap::fromImage(
                                    aplicarFiltroPrewitt(original2)).scaled(ui->imageLabelPF->size(), Qt::KeepAspectRatio));
}

QImage FiltersWindow::extraerSliceComoQImage(int sliceIndex, ImageType3D::Pointer volumen) {
    if (!volumen) return QImage();

    using ExtractFilterType = itk::ExtractImageFilter<ImageType3D, ImageType2D>;
    using UCharImageType = itk::Image<unsigned char, 2>;
    using RescaleFilterType = itk::RescaleIntensityImageFilter<ImageType2D, UCharImageType>;

    ExtractFilterType::Pointer extractor = ExtractFilterType::New();

    ImageType3D::RegionType inputRegion = volumen->GetLargestPossibleRegion();
    ImageType3D::SizeType size = inputRegion.GetSize();
    ImageType3D::IndexType start = inputRegion.GetIndex();

    size[2] = 0;
    start[2] = sliceIndex;

    ImageType3D::RegionType desiredRegion;
    desiredRegion.SetSize(size);
    desiredRegion.SetIndex(start);

    extractor->SetExtractionRegion(desiredRegion);
    extractor->SetInput(volumen);
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


void FiltersWindow::on_DescON_clicked()
{
    QString texto = "NOT operation inverts the pixel values in an image. \n"
                    "In OpenCV, cv::bitwise_not is used to invert all bits, flipping the pixel values.";
    descWindow->setDescription(texto);
    descWindow->show();
}


void FiltersWindow::on_DescOA_clicked()
{
    QString texto = "AND operation is used to compare two binary images. \n"
                    "In OpenCV, cv::bitwise_and is used to apply the AND operation between two images, highlighting common regions.";
    descWindow->setDescription(texto);
    descWindow->show();
}


void FiltersWindow::on_DescOO_clicked()
{
    QString texto = "OR operation combines the features of two images. \n"
                    "In OpenCV, cv::bitwise_or is used to apply the OR operation between two images, emphasizing both areas of interest.";
    descWindow->setDescription(texto);
    descWindow->show();
}


void FiltersWindow::on_DescPF_clicked()
{
    QString texto = "Prewitt Filter is a classic edge detection technique used to highlight the boundaries in an image. \n"
                    "It uses convolution kernels in both horizontal and vertical directions to detect edges by emphasizing abrupt changes in intensity.";
    descWindow->setDescription(texto);
    descWindow->show();
}

