#include "DicomViewer.h"

#include <vtkRenderWindow.h>
#include <vtkCamera.h>
#include <vtkImageData.h>
#include <vtkInteractorStyle.h>

#include <QDebug>
#include <QDir>
#include <QFileInfo>

#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcpixel.h>
#include <dcmtk/dcmdata/dcpxitem.h>
#include <dcmtk/dcmjpeg/djdecode.h>
#include <dcmtk/dcmimgle/dcmimage.h>
#include <dcmtk/dcmimage/diregist.h>

namespace viewer {

DicomViewer::DicomViewer(QWidget *parent)
    : QWidget(parent)
{
    DJDecoderRegistration::registerCodecs();
    setupLayout();
    setupVTK();
}

DicomViewer::~DicomViewer()
{
    DJDecoderRegistration::cleanup();
}

void DicomViewer::setupLayout()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_vtkWidget = new QVTKOpenGLNativeWidget(this);
    layout->addWidget(m_vtkWidget);

    setLayout(layout);
}

void DicomViewer::setupVTK()
{
    m_renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    m_vtkWidget->setRenderWindow(m_renderWindow);

    m_renderer = vtkSmartPointer<vtkRenderer>::New();
    m_renderer->SetBackground(0.04, 0.04, 0.04);
    m_renderWindow->AddRenderer(m_renderer);

    m_interactorStyle = vtkSmartPointer<vtkInteractorStyleImage>::New();
    m_vtkWidget->interactor()->SetInteractorStyle(m_interactorStyle);
}

void DicomViewer::configureImageViewer()
{
    if (!m_imageViewer) {
        m_imageViewer = vtkSmartPointer<vtkImageViewer2>::New();
    }

    m_renderWindow->RemoveRenderer(m_renderer);
    m_imageViewer->SetRenderWindow(m_renderWindow);

    vtkRenderWindowInteractor* interactor = m_vtkWidget->interactor();
    m_imageViewer->GetRenderer()->SetBackground(0.04, 0.04, 0.04);

    vtkSmartPointer<vtkInteractorStyle> style = vtkSmartPointer<vtkInteractorStyle>::New();
    interactor->SetInteractorStyle(style);

    m_renderer = m_imageViewer->GetRenderer();
}

// --- Helper DRY Template ---
template<typename T>
void DicomViewer::copyPixelData(void* dest, const T* source, size_t rows, size_t cols, size_t samplesPerPixel) {
    T* vtkData = static_cast<T*>(dest);
    const size_t rowBytes = cols * samplesPerPixel * sizeof(T);
    
    // Flip Y axis (DICOM Top-Left -> VTK Bottom-Left)
    for (size_t y = 0; y < rows; ++y) {
        const size_t srcRow = rows - 1 - y;
        memcpy(vtkData + y * cols * samplesPerPixel, 
               source + srcRow * cols * samplesPerPixel, 
               rowBytes);
    }
}

// --- SRP: Metadata Extraction ---
bool DicomViewer::extractMetadata(DcmDataset* dataset) {
    if (!dataset) return false;

    Uint16 rows = 0, cols = 0;
    dataset->findAndGetUint16(DCM_Rows, rows);
    dataset->findAndGetUint16(DCM_Columns, cols);

    if (rows == 0 || cols == 0) return false;

    Uint16 bitsAllocated = 16, bitsStored = 12, highBit = 11;
    Uint16 pixelRepresentation = 0;
    Uint16 samplesPerPixel = 1;

    dataset->findAndGetUint16(DCM_BitsAllocated, bitsAllocated);
    dataset->findAndGetUint16(DCM_BitsStored, bitsStored);
    dataset->findAndGetUint16(DCM_HighBit, highBit);
    dataset->findAndGetUint16(DCM_PixelRepresentation, pixelRepresentation);
    dataset->findAndGetUint16(DCM_SamplesPerPixel, samplesPerPixel);

    m_metadata.rows = rows;
    m_metadata.columns = cols;
    m_metadata.bitsAllocated = bitsAllocated;
    m_metadata.bitsStored = bitsStored;
    m_metadata.pixelRepresentation = pixelRepresentation;
    m_metadata.samplesPerPixel = samplesPerPixel;

    OFString strValue;
    if (dataset->findAndGetOFString(DCM_PatientName, strValue).good()) {
        m_metadata.patientName = QString::fromUtf8(strValue.c_str());
    }
    if (dataset->findAndGetOFString(DCM_PatientID, strValue).good()) {
        m_metadata.patientId = QString::fromUtf8(strValue.c_str());
    }
    if (dataset->findAndGetOFString(DCM_StudyDate, strValue).good()) {
        m_metadata.studyDate = QString::fromUtf8(strValue.c_str());
    }
    if (dataset->findAndGetOFString(DCM_Modality, strValue).good()) {
        m_metadata.modality = QString::fromUtf8(strValue.c_str());
    }
    if (dataset->findAndGetOFString(DCM_InstitutionName, strValue).good()) {
        m_metadata.institutionName = QString::fromUtf8(strValue.c_str());
    }

    Float64 windowCenter = 0.0, windowWidth = 0.0;
    if (dataset->findAndGetFloat64(DCM_WindowCenter, windowCenter).good() &&
        dataset->findAndGetFloat64(DCM_WindowWidth, windowWidth).good()) {
        m_metadata.windowCenter = windowCenter;
        m_metadata.windowWidth = windowWidth;
    }

    OFString pixelSpacing;
    if (dataset->findAndGetOFString(DCM_PixelSpacing, pixelSpacing, 0).good()) {
        m_metadata.pixelSpacingY = QString::fromStdString(pixelSpacing.c_str()).toDouble();
    }
    if (dataset->findAndGetOFString(DCM_PixelSpacing, pixelSpacing, 1).good()) {
        m_metadata.pixelSpacingX = QString::fromStdString(pixelSpacing.c_str()).toDouble();
    }

    Float64 rescaleSlope = 1.0, rescaleIntercept = 0.0;
    dataset->findAndGetFloat64(DCM_RescaleSlope, rescaleSlope);
    dataset->findAndGetFloat64(DCM_RescaleIntercept, rescaleIntercept);
    m_metadata.rescaleSlope = rescaleSlope;
    m_metadata.rescaleIntercept = rescaleIntercept;

    return true;
}

// --- SRP: Image Creation ---
vtkSmartPointer<vtkImageData> DicomViewer::createVtkImage(DcmDataset* dataset) {
    vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
    imageData->SetDimensions(m_metadata.columns, m_metadata.rows, 1);
    imageData->SetSpacing(m_metadata.pixelSpacingX, m_metadata.pixelSpacingY, 1.0);
    imageData->SetOrigin(0.0, 0.0, 0.0);

    // Color Images (RGB, YBR, etc.)
    if (m_metadata.samplesPerPixel > 1) {
        DicomImage dcmImage(dataset, dataset->getOriginalXfer());
        if (dcmImage.getStatus() != EIS_Normal) {
            qWarning() << "DCMTK: Error processing color image";
            return nullptr;
        }

        if (dcmImage.getOutputDataSize(8) == 0) return nullptr;

        imageData->AllocateScalars(VTK_UNSIGNED_CHAR, 3);
        const void* rawData = dcmImage.getOutputData(8); // Renders internal buffer
        
        copyPixelData(imageData->GetScalarPointer(), 
                      static_cast<const unsigned char*>(rawData), 
                      m_metadata.rows, m_metadata.columns, 3);

        if (m_metadata.windowWidth == 0.0) {
            m_metadata.windowWidth = 255.0;
            m_metadata.windowCenter = 127.5;
        }
    } 
    // Monochrome Images
    else {
        const Uint8* pixelData8 = nullptr;
        const Uint16* pixelData16 = nullptr;
        unsigned long pixelCount = 0;

        if (m_metadata.bitsAllocated <= 8) {
            if (dataset->findAndGetUint8Array(DCM_PixelData, pixelData8, &pixelCount).bad()) return nullptr;
            
            imageData->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
            copyPixelData(imageData->GetScalarPointer(), pixelData8, m_metadata.rows, m_metadata.columns, 1);
        } else {
            if (dataset->findAndGetUint16Array(DCM_PixelData, pixelData16, &pixelCount).bad()) return nullptr;

            int scalarType = (m_metadata.pixelRepresentation == 0) ? VTK_UNSIGNED_SHORT : VTK_SHORT;
            imageData->AllocateScalars(scalarType, 1);
            
            if (m_metadata.pixelRepresentation == 0) {
                 copyPixelData(imageData->GetScalarPointer(), pixelData16, m_metadata.rows, m_metadata.columns, 1);
            } else {
                 copyPixelData(imageData->GetScalarPointer(), reinterpret_cast<const short*>(pixelData16), m_metadata.rows, m_metadata.columns, 1);
            }
        }

        // Auto Window/Level if missing
        if (m_metadata.windowWidth == 0.0) {
            double range[2];
            imageData->GetScalarRange(range);
            m_metadata.windowWidth = range[1] - range[0];
            m_metadata.windowCenter = range[0] + m_metadata.windowWidth / 2.0;
        }
    }

    return imageData;
}

vtkSmartPointer<vtkImageData> DicomViewer::loadDicomWithDCMTK(const QString& filePath)
{
    DcmFileFormat fileFormat;
    OFCondition status = fileFormat.loadFile(filePath.toStdString().c_str());

    if (status.bad()) {
        qWarning() << "DCMTK: Failed to load file:" << filePath << "-" << status.text();
        return nullptr;
    }

    DcmDataset* dataset = fileFormat.getDataset();
    if (!dataset) {
        qWarning() << "DCMTK: Invalid dataset";
        return nullptr;
    }

    dataset->chooseRepresentation(EXS_LittleEndianExplicit, nullptr);

    if (!extractMetadata(dataset)) {
        qWarning() << "DCMTK: Failed to extract metadata or invalid dimensions";
        return nullptr;
    }

    return createVtkImage(dataset);
}

bool DicomViewer::loadFile(const QString& filePath)
{
    if (filePath.isEmpty()) {
        emit errorOccurred("Caminho do arquivo vazio");
        return false;
    }

    try {
        m_imageData = loadDicomWithDCMTK(filePath);

        if (!m_imageData || m_imageData->GetNumberOfPoints() == 0) {
            emit errorOccurred("Falha ao carregar imagem DICOM");
            return false;
        }

        configureImageViewer();
        m_imageViewer->SetInputData(m_imageData);
        m_imageViewer->SetColorWindow(m_metadata.windowWidth);
        m_imageViewer->SetColorLevel(m_metadata.windowCenter);

        m_imageViewer->Render();
        m_imageViewer->GetRenderer()->ResetCamera();
        m_imageViewer->Render();

        m_currentFilePath = filePath;
        m_hasImage = true;

        emit imageLoaded(filePath);
        return true;

    } catch (const std::exception& e) {
        emit errorOccurred(QString("Erro ao carregar DICOM: %1").arg(e.what()));
        return false;
    }
}

bool DicomViewer::loadDirectory(const QString& dirPath)
{
    if (dirPath.isEmpty()) {
        emit errorOccurred("Caminho do diretório vazio");
        return false;
    }

    try {
        QDir dir(dirPath);
        QStringList filters;
        filters << "*.dcm" << "*.DCM" << "*.dicom" << "*.DICOM";
        QStringList files = dir.entryList(filters, QDir::Files, QDir::Name);

        if (files.isEmpty()) {
            emit errorOccurred("Nenhum arquivo DICOM encontrado no diretório");
            return false;
        }

        QString firstFile = dir.absoluteFilePath(files.first());
        return loadFile(firstFile);

    } catch (const std::exception& e) {
        emit errorOccurred(QString("Erro ao carregar série DICOM: %1").arg(e.what()));
        return false;
    }
}

void DicomViewer::setWindowLevel(double window, double level)
{
    if (!m_hasImage || !m_imageViewer) return;

    m_imageViewer->SetColorWindow(window);
    m_imageViewer->SetColorLevel(level);
    m_imageViewer->Render();

    emit windowLevelChanged(window, level);
}

double DicomViewer::windowValue() const
{
    return (m_hasImage && m_imageViewer) ? m_imageViewer->GetColorWindow() : 0.0;
}

double DicomViewer::levelValue() const
{
    return (m_hasImage && m_imageViewer) ? m_imageViewer->GetColorLevel() : 0.0;
}

bool DicomViewer::hasImage() const
{
    return m_hasImage;
}

QString DicomViewer::currentFilePath() const
{
    return m_currentFilePath;
}

}
