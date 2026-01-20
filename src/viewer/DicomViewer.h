#ifndef DICOMVIEWER_H
#define DICOMVIEWER_H

#include <QWidget>
#include <QVBoxLayout>
#include <QString>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkImageViewer2.h>
#include <vtkImageData.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleImage.h>
#include <QVTKOpenGLNativeWidget.h>

class DcmDataset;

namespace viewer {

struct DicomMetadata {
    QString patientName;
    QString patientId;
    QString studyDate;
    QString modality;
    QString institutionName;
    int rows = 0;
    int columns = 0;
    int bitsAllocated = 0;
    int bitsStored = 0;
    int pixelRepresentation = 0;
    int samplesPerPixel = 1;
    double windowCenter = 0.0;
    double windowWidth = 0.0;
    double pixelSpacingX = 1.0;
    double pixelSpacingY = 1.0;
    double rescaleSlope = 1.0;
    double rescaleIntercept = 0.0;
};

class DicomViewer : public QWidget
{
    Q_OBJECT

public:
    explicit DicomViewer(QWidget *parent = nullptr);
    ~DicomViewer() override;

    bool loadFile(const QString& filePath);
    bool loadDirectory(const QString& dirPath);

    void setWindowLevel(double window, double level);
    double windowValue() const;
    double levelValue() const;

    bool hasImage() const;
    QString currentFilePath() const;

    const DicomMetadata& metadata() const { return m_metadata; }

signals:
    void imageLoaded(const QString& filePath);
    void windowLevelChanged(double window, double level);
    void errorOccurred(const QString& error);

private:
    void setupLayout();
    void setupVTK();
    void configureImageViewer();
    vtkSmartPointer<vtkImageData> loadDicomWithDCMTK(const QString& filePath);

    // Helpers para SOLID (SRP) e DRY
    bool extractMetadata(DcmDataset* dataset);
    vtkSmartPointer<vtkImageData> createVtkImage(DcmDataset* dataset);
    
    template<typename T>
    void copyPixelData(void* dest, const T* source, size_t rows, size_t cols, size_t samplesPerPixel);

    QVTKOpenGLNativeWidget* m_vtkWidget = nullptr;

    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderWindow;
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkInteractorStyleImage> m_interactorStyle;
    vtkSmartPointer<vtkImageViewer2> m_imageViewer;
    vtkSmartPointer<vtkImageData> m_imageData;

    QString m_currentFilePath;
    bool m_hasImage = false;

    DicomMetadata m_metadata;
};

} // namespace viewer

#endif // DICOMVIEWER_H
