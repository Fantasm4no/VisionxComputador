#include "statswindow.h"
#include <QMessageBox>
#include <QPixmap>
#include <fstream>
#include <numeric>
#include <algorithm>
#include <cmath>
#include "ui_statswindow.h"
#include "itkExtractImageFilter.h"
#include "itkImageRegionConstIterator.h"
#include "itkRescaleIntensityImageFilter.h"
#include "qcustomplot.h"

StatsWindow::StatsWindow(QWidget *parent)
    : QWidget(parent), ui(new Ui::StatsWindow){
    ui->setupUi(this);
    ultimaEstadistica = NINGUNA;

    connect(ui->btnMedia, &QPushButton::clicked, this, &StatsWindow::calcularMedia);
    connect(ui->btnMediana, &QPushButton::clicked, this, &StatsWindow::calcularMediana);
    connect(ui->btnDesviacion, &QPushButton::clicked, this, &StatsWindow::calcularDesviacionEstandar);
    connect(ui->btnMinMax, &QPushButton::clicked, this, &StatsWindow::calcularMinMax);
    connect(ui->btnArea, &QPushButton::clicked, this, &StatsWindow::calcularArea);
    connect(ui->btnOutliers, &QPushButton::clicked, this, &StatsWindow::detectarOutliers);
    connect(ui->btnBoxplot, &QPushButton::clicked, this, &StatsWindow::mostrarBoxplot);
    connect(ui->btnGuardarCSV, &QPushButton::clicked, this, &StatsWindow::guardarEstadisticas);
}

StatsWindow::~StatsWindow() {
    delete ui;
}
void StatsWindow::setSliceIndex(int index) {
    sliceIndex = index;
}

void StatsWindow::setImagenes(ImageType3D::Pointer img1, ImageType3D::Pointer img2, int slice) {
    imagen1 = img1;
    imagen2 = img2;
    sliceIndex = slice;
}

std::vector<float> StatsWindow::extraerValoresDeInteres() {
    std::vector<float> valores;

    using ExtractFilterType = itk::ExtractImageFilter<ImageType3D, ImageType2D>;
    ExtractFilterType::Pointer extractor1 = ExtractFilterType::New();
    ImageType3D::RegionType region1 = imagen1->GetLargestPossibleRegion();
    ImageType3D::SizeType size1 = region1.GetSize();
    ImageType3D::IndexType start1 = region1.GetIndex();
    size1[2] = 0;
    start1[2] = sliceIndex;
    ImageType3D::RegionType sliceRegion1;
    sliceRegion1.SetSize(size1);
    sliceRegion1.SetIndex(start1);
    extractor1->SetExtractionRegion(sliceRegion1);
    extractor1->SetInput(imagen1);
    extractor1->SetDirectionCollapseToSubmatrix();
    extractor1->Update();
    ImageType2D::Pointer sliceImg = extractor1->GetOutput();

    ExtractFilterType::Pointer extractor2 = ExtractFilterType::New();
    ImageType3D::RegionType region2 = imagen2->GetLargestPossibleRegion();
    ImageType3D::SizeType size2 = region2.GetSize();
    ImageType3D::IndexType start2 = region2.GetIndex();
    size2[2] = 0;
    start2[2] = sliceIndex;
    ImageType3D::RegionType sliceRegion2;
    sliceRegion2.SetSize(size2);
    sliceRegion2.SetIndex(start2);
    extractor2->SetExtractionRegion(sliceRegion2);
    extractor2->SetInput(imagen2);
    extractor2->SetDirectionCollapseToSubmatrix();
    extractor2->Update();
    ImageType2D::Pointer sliceMask = extractor2->GetOutput();

    itk::ImageRegionConstIterator<ImageType2D> itImg(sliceImg, sliceImg->GetLargestPossibleRegion());
    itk::ImageRegionConstIterator<ImageType2D> itMask(sliceMask, sliceMask->GetLargestPossibleRegion());

    for (itImg.GoToBegin(), itMask.GoToBegin(); !itImg.IsAtEnd(); ++itImg, ++itMask) {
        if (itMask.Get() > 0) {
            valores.push_back(itImg.Get());
        }
    }

    return valores;
}

void StatsWindow::calcularMedia() {
    ultimaEstadistica = MEDIA;
    std::vector<float> valores = extraerValoresDeInteres();
    if (valores.empty()) {
        ui->labelResultado->setText("No hay datos válidos.");
        return;
    }

    float suma = std::accumulate(valores.begin(), valores.end(), 0.0f);
    float media = suma / valores.size();
    ultimaMedia = media;
    ui->labelResultado->setText("Media: " + QString::number(media, 'f', 2));
}

void StatsWindow::calcularMediana() {
    ultimaEstadistica = MEDIANA;
    std::vector<float> valores = extraerValoresDeInteres();
    if (valores.empty()) {
        ui->labelResultado->setText("No hay datos válidos.");
        return;
    }

    std::sort(valores.begin(), valores.end());
    float mediana;
    size_t n = valores.size();

    if (n % 2 == 0) {
        mediana = (valores[n / 2 - 1] + valores[n / 2]) / 2.0f;
    } else {
        mediana = valores[n / 2];
    }
    ultimaMediana = mediana;
    ui->labelResultado->setText("Mediana: " + QString::number(mediana, 'f', 2));
}

void StatsWindow::calcularDesviacionEstandar() {
    ultimaEstadistica = DESVIACION;
    std::vector<float> valores = extraerValoresDeInteres();
    if (valores.empty()) {
        ui->labelResultado->setText("No hay datos válidos.");
        return;
    }

    float media = std::accumulate(valores.begin(), valores.end(), 0.0f) / valores.size();
    float suma = 0.0f;
    for (float v : valores) {
        suma += (v - media) * (v - media);
    }
    float desviacion = std::sqrt(suma / valores.size());
    ultimaDesviacion = desviacion;
    ui->labelResultado->setText("Desviación estándar: " + QString::number(desviacion, 'f', 2));
}

void StatsWindow::calcularMinMax() {
    ultimaEstadistica = MINMAX;
    std::vector<float> valores = extraerValoresDeInteres();
    if (valores.empty()) {
        ui->labelResultado->setText("No hay datos válidos.");
        return;
    }

    auto [minIt, maxIt] = std::minmax_element(valores.begin(), valores.end());
    double minimo = *minIt;
    double maximo = *maxIt;

    ultimoMin = minimo;   
    ultimoMax = maximo;

    ui->labelResultado->setText("Mínimo: " + QString::number(minimo, 'f', 2) +
                                "\nMáximo: " + QString::number(maximo, 'f', 2));
}


void StatsWindow::calcularArea() {
    std::vector<float> valores = extraerValoresDeInteres();
    if (valores.empty()) {
        ui->labelResultado->setText("Sin datos para calcular área.");
        return;
    }

    double area = static_cast<double>(valores.size());  // cantidad de píxeles
    ultimaArea = area;

    ui->labelResultado->setText("Área (número de valores): " + QString::number(area));
}


void StatsWindow::detectarOutliers() {
    ultimaEstadistica = OUTLIERS;
    std::vector<float> valores = extraerValoresDeInteres();
    if (valores.size() < 4) {
        cantidadOutliers = 0;
        ui->labelResultado->setText("No hay suficientes datos para detectar outliers.");
        return;
    }

    std::sort(valores.begin(), valores.end());
    size_t q1Index = valores.size() / 4;
    size_t q3Index = (3 * valores.size()) / 4;
    float q1 = valores[q1Index];
    float q3 = valores[q3Index];
    float iqr = q3 - q1;

    float lower = q1 - 1.5f * iqr;
    float upper = q3 + 1.5f * iqr;

    int count = std::count_if(valores.begin(), valores.end(), [=](float v) {
        return v < lower || v > upper;
    });

    cantidadOutliers = count;
    ui->labelResultado->setText("Outliers detectados: " + QString::number(count));
}


void StatsWindow::mostrarBoxplot() {
    ultimaEstadistica = BOXPLOT;

    std::vector<float> valores = extraerValoresDeInteres();
    ui->customPlot->clearItems();
    ui->customPlot->clearPlottables();

    if (valores.empty()) {
        ui->labelResultado->setText("No hay datos válidos para graficar.");
        ui->customPlot->replot();
        return;
    }

    std::sort(valores.begin(), valores.end());

    const int n = valores.size();
    double minVal = valores.front();
    double maxVal = valores.back();
    double q1 = valores[n * 25 / 100];
    double median = valores[n * 50 / 100];
    double q3 = valores[n * 75 / 100];
    double iqr = q3 - q1;
    double lowerFence = q1 - 1.5 * iqr;
    double upperFence = q3 + 1.5 * iqr;

    QVector<double> outlierY;
    for (float v : valores) {
        if (v < lowerFence || v > upperFence)
            outlierY.append(v);
    }

    double x = 1.0;
    double boxWidth = 0.4;

    // Caja
    auto *box = new QCPItemRect(ui->customPlot);
    box->topLeft->setCoords(x - boxWidth / 2.0, q3);
    box->bottomRight->setCoords(x + boxWidth / 2.0, q1);
    box->setBrush(QBrush(QColor(160, 200, 255, 120)));
    box->setPen(QPen(Qt::blue, 1.5));

    // Mediana
    auto *mediana = new QCPItemLine(ui->customPlot);
    mediana->start->setCoords(x - boxWidth / 2.0, median);
    mediana->end->setCoords(x + boxWidth / 2.0, median);
    mediana->setPen(QPen(Qt::darkRed, 2));

    // Bigotes
    auto *bigoteInf = new QCPItemLine(ui->customPlot);
    bigoteInf->start->setCoords(x, q1);
    bigoteInf->end->setCoords(x, minVal);
    bigoteInf->setPen(QPen(Qt::black));

    auto *bigoteSup = new QCPItemLine(ui->customPlot);
    bigoteSup->start->setCoords(x, q3);
    bigoteSup->end->setCoords(x, maxVal);
    bigoteSup->setPen(QPen(Qt::black));

    // Líneas horizontales min/max
    auto *minLine = new QCPItemLine(ui->customPlot);
    minLine->start->setCoords(x - boxWidth / 4.0, minVal);
    minLine->end->setCoords(x + boxWidth / 4.0, minVal);
    minLine->setPen(QPen(Qt::black));

    auto *maxLine = new QCPItemLine(ui->customPlot);
    maxLine->start->setCoords(x - boxWidth / 4.0, maxVal);
    maxLine->end->setCoords(x + boxWidth / 4.0, maxVal);
    maxLine->setPen(QPen(Qt::black));

    // Outliers: usar marcador discreto, no líneas
    for (double val : outlierY) {
        QCPItemTracer *outlier = new QCPItemTracer(ui->customPlot);
        outlier->position->setCoords(x, val);
        outlier->setStyle(QCPItemTracer::tsPlus);
        outlier->setPen(QPen(Qt::red, 1.5));
        outlier->setSize(7);
    }

    // Etiquetas
    auto *labelMed = new QCPItemText(ui->customPlot);
    labelMed->setText(QString("Mediana: %1").arg(median));
    labelMed->position->setCoords(x + 0.5, median);
    labelMed->setFont(QFont(font().family(), 9));
    labelMed->setColor(Qt::darkRed);

    auto *labelMin = new QCPItemText(ui->customPlot);
    labelMin->setText(QString("Min: %1").arg(minVal));
    labelMin->position->setCoords(x + 0.5, minVal);
    labelMin->setFont(QFont(font().family(), 9));

    auto *labelMax = new QCPItemText(ui->customPlot);
    labelMax->setText(QString("Max: %1").arg(maxVal));
    labelMax->position->setCoords(x + 0.5, maxVal);
    labelMax->setFont(QFont(font().family(), 9));

    // Ejes
    ui->customPlot->xAxis->setRange(0, 2);
    ui->customPlot->yAxis->setRange(minVal - 10, maxVal + 10);
    ui->customPlot->xAxis->setLabel("Distribución");
    ui->customPlot->yAxis->setLabel("Valor");

    ui->customPlot->replot();
}
void StatsWindow::guardarEstadisticas() {
    std::vector<float> valores = extraerValoresDeInteres();
    if (valores.empty()) {
        QMessageBox::warning(this, "Sin datos", "No hay datos válidos para guardar.");
        return;
    }

    // --- Calcular media ---
    double suma = std::accumulate(valores.begin(), valores.end(), 0.0);
    ultimaMedia = suma / valores.size();

    // --- Calcular mediana ---
    std::sort(valores.begin(), valores.end());
    size_t n = valores.size();
    if (n % 2 == 0) {
        ultimaMediana = (valores[n/2 - 1] + valores[n/2]) / 2.0;
    } else {
        ultimaMediana = valores[n/2];
    }

    // --- Calcular desviación estándar ---
    double media = ultimaMedia;
    double sumaCuadrados = 0.0;
    for (float v : valores) sumaCuadrados += std::pow(v - media, 2);
    ultimaDesviacion = std::sqrt(sumaCuadrados / valores.size());

    // --- Mínimo y máximo ---
    auto [minIt, maxIt] = std::minmax_element(valores.begin(), valores.end());
    ultimoMin = *minIt;
    ultimoMax = *maxIt;

    // --- Calcular área ---
    ultimaArea = static_cast<double>(valores.size());

    // --- Detectar outliers ---
    size_t q1Index = valores.size() / 4;
    size_t q3Index = (3 * valores.size()) / 4;
    double q1 = valores[q1Index];
    double q3 = valores[q3Index];
    double iqr = q3 - q1;
    double lower = q1 - 1.5 * iqr;
    double upper = q3 + 1.5 * iqr;
    cantidadOutliers = std::count_if(valores.begin(), valores.end(), [&](float v) {
        return v < lower || v > upper;
    });

    // --- Guardar ---
    QFile file("estadisticas.csv");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "Media,Mediana,Desviación,Mínimo,Máximo,Área,Outliers\n";
        out << ultimaMedia << "," << ultimaMediana << "," << ultimaDesviacion << ","
            << ultimoMin << "," << ultimoMax << "," << ultimaArea << "," << cantidadOutliers << "\n";
        file.close();
        QMessageBox::information(this, "Guardado", "Estadísticas calculadas y guardadas correctamente.");
    } else {
        QMessageBox::warning(this, "Error", "No se pudo guardar el archivo.");
    }
}


void StatsWindow::actualizarTodo() {
    calcularMedia();
    calcularMinMax();
    calcularArea();
    detectarOutliers();
    mostrarBoxplot();
}

void StatsWindow::actualizarSegunContexto() {
    switch (ultimaEstadistica) {
        case MEDIA:
            calcularMedia();
            break;
        case MINMAX:
            calcularMinMax();
            break;
        case AREA:
            calcularArea();
            break;
        case OUTLIERS:
            detectarOutliers();
            break;
        case BOXPLOT:
            mostrarBoxplot();
            break;
        case MEDIANA:
            calcularMediana();          
            break;
        case DESVIACION:
            calcularDesviacionEstandar(); 
            break;
        case NINGUNA:
        default:
            break;
    }
}


