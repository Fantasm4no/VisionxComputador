#include "filterswindow.h"
#include "ui_filterswindow.h"

#include "itkExtractImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"


FiltersWindow::FiltersWindow(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::filterswindow)  // Debe coincidir con el nombre generado por Qt Designer
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

    QImage img = extraerSliceComoQImage(sliceIndex);
    ui->imageLabelDB->setPixmap(QPixmap::fromImage(img).scaled(ui->imageLabelDB->size(), Qt::KeepAspectRatio));
    ui->imageLabelEH->setPixmap(QPixmap::fromImage(img).scaled(ui->imageLabelEH->size(), Qt::KeepAspectRatio));
    ui->imageLabelMP->setPixmap(QPixmap::fromImage(img).scaled(ui->imageLabelMP->size(), Qt::KeepAspectRatio));
    ui->imageLabelOM->setPixmap(QPixmap::fromImage(img).scaled(ui->imageLabelOM->size(), Qt::KeepAspectRatio));
    ui->imageLabelSI->setPixmap(QPixmap::fromImage(img).scaled(ui->imageLabelSI->size(), Qt::KeepAspectRatio));
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

    size[2] = 0;               // Extraer solo un slice (en Z)
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

