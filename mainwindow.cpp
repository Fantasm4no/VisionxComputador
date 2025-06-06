#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QImage>
#include <QPixmap>
#include <QDir>
#include <QFileInfoList>
#include <iostream>
#include <algorithm>
#include <regex>

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
    connect(ui->genVideo, &QPushButton::clicked, this, &MainWindow::generarVideo);
    QString rutaImagen = "/home/f4ntasmano/Downloads/ups.png";
    QPixmap pixmap(rutaImagen);
    if (!pixmap.isNull()) {
        ui->labelUPS->setPixmap(pixmap.scaled(ui->labelUPS->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        QMessageBox::warning(this, "Error", "The image could not be loaded.");
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
            filtersWindow->setVolumes(imagen3D_1, imagen3D_2);
            filtersWindow->show();
        } else {
            QMessageBox::warning(this, "Warning", "First upload an image.");
            ui->checkBoxShowMessage->setChecked(false);
        }
    } else {
        filtersWindow->close();
    }
}

void MainWindow::on_loadFile_clicked() {
    niftiPath1 = QFileDialog::getOpenFileName(this, "Upload NIfTI 1 file", "", "NIfTI files (*.nii *.nii.gz)");
    if (niftiPath1.isEmpty()) return;

    ui->labelRuta->setText("Image 1 loaded successfully.");

    // Obtener el nombre base del archivo sin la extensión
    QFileInfo fileInfo(niftiPath1);
    QString baseName = fileInfo.baseName();

    using ReaderType = itk::ImageFileReader<ImageType3D>;
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(niftiPath1.toStdString());

    try {
        reader->Update();
        imagen3D_1 = reader->GetOutput();
    } catch (itk::ExceptionObject &error) {
        QMessageBox::critical(this, "Error", "Could not read NIfTI file 1.");
        return;
    }

    // Asignar el nombre base para usarlo en las imágenes guardadas
    imagenBaseName = baseName;  // Guardar el nombre base de la imagen

    actualizarTotalSlices();
    mostrarImagenProcesada(currentSlice);
}

void MainWindow::on_loadFile2_clicked() {
    niftiPath2 = QFileDialog::getOpenFileName(this, "Upload NIfTI 2 file", "", "NIfTI files (*.nii *.nii.gz)");
    if (niftiPath2.isEmpty()) return;

    ui->labelRuta2->setText("Image 2 loaded successfully.");

    using ReaderType = itk::ImageFileReader<ImageType3D>;
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(niftiPath2.toStdString());

    try {
        reader->Update();
        imagen3D_2 = reader->GetOutput();
    } catch (itk::ExceptionObject &error) {
        QMessageBox::critical(this, "Error", "Could not read NIfTI file 2.");
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

    // Solo guardar la imagen si el slider ha avanzado
    if (currentSlice > lastSliceIndex) {
        mostrarSegmentacion(currentSlice);  // Guardar la imagen solo si el slider avanza
    }

    mostrarImagenProcesada(value);
    mostrarSegmentacion(value);

    // Actualizar el índice anterior
    lastSliceIndex = currentSlice;

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

QImage MainWindow::aplicarFiltroPrewitt(const QImage &input) {
    // Convertir la imagen QImage a cv::Mat (escala de grises)
    cv::Mat img = QImageToMat(input);
    cv::Mat resultX, resultY, result;

    // Filtro Prewitt en X y Y
    cv::Mat kernelX = (cv::Mat_<float>(3, 3) << -1, 0, 1, -1, 0, 1, -1, 0, 1);
    cv::Mat kernelY = (cv::Mat_<float>(3, 3) << -1, -1, -1, 0, 0, 0, 1, 1, 1);

    // Aplicar filtro Prewitt en ambas direcciones
    cv::filter2D(img, resultX, CV_32F, kernelX);  // Detectar bordes en dirección X
    cv::filter2D(img, resultY, CV_32F, kernelY);  // Detectar bordes en dirección Y

    // Magnitud de los bordes
    cv::magnitude(resultX, resultY, result);

    // Normalizar la imagen para tener valores entre 0 y 255
    result.convertTo(result, CV_8UC1);  // Convertir a tipo 8 bits por canal

    return MatToQImage(result);  // Convertir el resultado de nuevo a QImage
}

void MainWindow::mostrarSegmentacion(int sliceIndex) {
    if (!imagen3D_1 || !imagen3D_2) return;

    // Extraer slices
    QImage qImgT1ce = extraerSliceComoQImage(imagen3D_1, sliceIndex);  // Imagen original (densidades)
    QImage qImgSeg  = extraerSliceComoQImage(imagen3D_2, sliceIndex);  // Segmentación

    // Convertir imágenes a cv::Mat
    cv::Mat matT1ce = QImageToMat(qImgT1ce);  // Imagen original escala de grises (valores HU)
    cv::Mat matSeg  = QImageToMat(qImgSeg);   // Segmentación

    // Convertir la imagen original a color para pintar
    cv::Mat matColor;
    cv::cvtColor(matT1ce, matColor, cv::COLOR_GRAY2BGR);

    // Parámetros del rango de densidad (ajusta según tus datos reales)
    float valorMin = 0.0f;    // Mínimo valor HU en ROI
    float valorMax = 255.0f;  // Máximo valor HU en ROI

    // Aplicar esquema de color basado en V a regiones segmentadas
    for (int y = 0; y < matColor.rows; ++y) {
        for (int x = 0; x < matColor.cols; ++x) {
            uchar segVal = matSeg.at<uchar>(y, x);
            if (segVal > 0) {
                float valorHU = static_cast<float>(matT1ce.at<uchar>(y, x));

                // Normalizar y limitar V
                float V = (valorHU - valorMin) / (valorMax - valorMin);
                V = std::clamp(V, 0.0f, 1.0f);

                cv::Vec3b &pix = matColor.at<cv::Vec3b>(y, x);

                if (V <= 0.5f) {
                    pix[2] = static_cast<uchar>(255 * (1 - 2 * V));  // Rojo
                    pix[1] = static_cast<uchar>(255 * (2 * V));      // Verde
                    pix[0] = 0;                                       // Azul
                } else {
                    pix[2] = 0;                                       // Rojo
                    pix[1] = static_cast<uchar>(255 * (2 - 2 * V));  // Verde
                    pix[0] = static_cast<uchar>(255 * (2 * V - 1));  // Azul
                }
            }
        }
    }

    // Opcional: Aplicar filtro Prewitt para obtener bordes (en escala de grises)
    QImage qImgPrewitt = aplicarFiltroPrewitt(qImgT1ce);
    cv::Mat matPrewitt = QImageToMat(qImgPrewitt);

    // Superponer bordes (blanco) sobre la imagen coloreada para resaltarlos
    // Superponer bordes con mezcla suave (blending)
    for (int y = 0; y < matPrewitt.rows; ++y) {
        for (int x = 0; x < matPrewitt.cols; ++x) {
            uchar bordeVal = matPrewitt.at<uchar>(y, x);
            if (bordeVal > 50) {  // Umbral para borde
                cv::Vec3b &pix = matColor.at<cv::Vec3b>(y, x);

                // Mezcla simple: 70% color original + 30% blanco para suavizar el borde
                pix[0] = static_cast<uchar>(pix[0] * 0.7 + 255 * 0.3);  // Azul
                pix[1] = static_cast<uchar>(pix[1] * 0.7 + 255 * 0.3);  // Verde
                pix[2] = static_cast<uchar>(pix[2] * 0.7 + 255 * 0.3);  // Rojo
            }
        }
    }

    // Mostrar imágenes en UI
    QImage qImgOverlay = MatToQImage(matColor);
    ui->labelImagen->setPixmap(QPixmap::fromImage(qImgT1ce).scaled(ui->labelImagen->size(), Qt::KeepAspectRatio));
    ui->labelImagen2->setPixmap(QPixmap::fromImage(qImgSeg).scaled(ui->labelImagen2->size(), Qt::KeepAspectRatio));
    ui->labelImagen3->setPixmap(QPixmap::fromImage(qImgOverlay).scaled(ui->labelImagen3->size(), Qt::KeepAspectRatio));

    // Guardar imagen coloreada
    QString folderPath = "/home/f4ntasmano/Downloads/" + imagenBaseName;
    QDir dir(folderPath);
    if (!dir.exists()) dir.mkpath(".");

    std::string filename = folderPath.toStdString() + "/slice_" + std::to_string(sliceIndex) + "_resaltado.png";
    cv::imwrite(filename, matColor);
}

void MainWindow::generarVideo() {
    // Definir la carpeta donde se encuentran las imágenes
    QString folderPath = "/home/f4ntasmano/Downloads/" + imagenBaseName;
    QDir dir(folderPath);

    // Obtener todas las imágenes en la carpeta (ordenadas por nombre)
    QFileInfoList files = dir.entryInfoList(QStringList() << "*.png", QDir::Files, QDir::Name);

    if (files.isEmpty()) {
        std::cerr << "No se encontraron imágenes en la carpeta." << std::endl;
        return;
    }

    // Crear una lista con los nombres de las imágenes ordenadas por el índice del slice
    std::vector<QFileInfo> sortedFiles;
    for (const QFileInfo &fileInfo : files) {
        sortedFiles.push_back(fileInfo);
    }

    // Ordenar las imágenes numéricamente utilizando una expresión regular para extraer el número de cada archivo
    std::sort(sortedFiles.begin(), sortedFiles.end(), [](const QFileInfo &a, const QFileInfo &b) {
        // Extraer el número del slice utilizando una expresión regular
        std::regex re(R"(\d+)"); // Expresión regular para encontrar el número en el nombre del archivo
        std::smatch matchA, matchB;

        std::string nameA = a.baseName().toStdString();
        std::string nameB = b.baseName().toStdString();

        // Buscar el número en el nombre del archivo
        std::regex_search(nameA, matchA, re);
        std::regex_search(nameB, matchB, re);

        // Comparar los números extraídos de los nombres de archivo
        int sliceIndexA = std::stoi(matchA.str());
        int sliceIndexB = std::stoi(matchB.str());

        return sliceIndexA < sliceIndexB;  // Ordenar numéricamente
    });

    // Leer la primera imagen para obtener el tamaño del video
    cv::Mat firstImage = cv::imread(sortedFiles[0].absoluteFilePath().toStdString());
    if (firstImage.empty()) {
        std::cerr << "No se pudo leer la primera imagen." << std::endl;
        return;
    }

    // Configurar el VideoWriter (nombre del archivo de salida, códec, fps, tamaño)
    std::string videoFilename = folderPath.toStdString() + "/video_salida.avi";
    cv::VideoWriter videoWriter;
    int fourcc = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');  // Códec MJPEG
    double fps = 10.0;  // Frecuencia de cuadros (puedes ajustarlo según tus necesidades)
    videoWriter.open(videoFilename, fourcc, fps, firstImage.size(), true);

    if (!videoWriter.isOpened()) {
        std::cerr << "No se pudo abrir el archivo de video para escritura." << std::endl;
        return;
    }

    // Escribir las imágenes en el video
    for (const QFileInfo &fileInfo : sortedFiles) {
        cv::Mat img = cv::imread(fileInfo.absoluteFilePath().toStdString());
        if (img.empty()) {
            std::cerr << "No se pudo leer la imagen: " << fileInfo.absoluteFilePath().toStdString() << std::endl;
            continue;
        }

        // Escribir el frame al video
        videoWriter.write(img);
    }

    std::cout << "Video guardado en: " << videoFilename << std::endl;
    videoWriter.release();  // Liberar el VideoWriter cuando haya terminado
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


