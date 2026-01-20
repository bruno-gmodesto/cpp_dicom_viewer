#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "styles/StyleManager.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QSlider>

#include <QVTKOpenGLNativeWidget.h>
#include <vtkAutoInit.h>

VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    resize(1024, 800);
    setupViewer();
    setupConnections();
    applyStyles();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupViewer()
{
    m_viewer = new viewer::DicomViewer(this);

    QLayout* layout = ui->viewerArea->layout();
    if (layout) {
        QLayoutItem* item;
        while ((item = layout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        layout->addWidget(m_viewer);
    } else {
        QVBoxLayout* newLayout = new QVBoxLayout(ui->viewerArea);
        newLayout->setContentsMargins(0, 0, 0, 0);
        newLayout->addWidget(m_viewer);
    }
}

void MainWindow::setupConnections()
{
    connect(ui->pushLerDicomButton, &QPushButton::clicked,
            this, &MainWindow::onOpenFileClicked);
    connect(ui->pushSairButton, &QPushButton::clicked,
            this, &MainWindow::onExitClicked);

    connect(m_viewer, &viewer::DicomViewer::imageLoaded,
            this, &MainWindow::onImageLoaded);
    connect(m_viewer, &viewer::DicomViewer::errorOccurred,
            this, &MainWindow::onViewerError);

    connect(ui->windowWidthSlider, &QSlider::valueChanged,
            this, &MainWindow::onWindowWidthChanged);
    connect(ui->windowLevelSlider, &QSlider::valueChanged,
            this, &MainWindow::onWindowLevelChanged);
}

void MainWindow::applyStyles()
{
    const auto& style = ui::StyleManager::instance();

    setStyleSheet(style.mainWindowStyle());
    ui->sidePanel->setStyleSheet(style.sidePanelStyle());
    ui->pushLerDicomButton->setStyleSheet(style.primaryButtonStyle());
    ui->pushSairButton->setStyleSheet(style.secondaryButtonStyle());
    ui->logoLabel->setStyleSheet(style.labelTitleStyle());
    ui->subtitleLabel->setStyleSheet(style.labelSubtitleStyle());
    ui->versionLabel->setStyleSheet(style.labelMutedStyle());
    ui->viewerArea->setStyleSheet(style.viewerAreaStyle());
}

void MainWindow::onOpenFileClicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Abrir arquivo DICOM",
        QString(),
        "Arquivos DICOM (*.dcm *.dicom);;Todos os arquivos (*)"
    );

    if (filePath.isEmpty()) {
        return;
    }

    // Usa o viewer VTK para carregar
    m_viewer->loadFile(filePath);
}

void MainWindow::onExitClicked()
{
    close();
}

void MainWindow::onImageLoaded(const QString& filePath)
{
    setWindowTitle(QString("DICOM Viewer - %1").arg(filePath));

    const auto& metadata = m_viewer->metadata();

    ui->ajustesWidget->setVisible(true);
    ui->metadataWidget->setVisible(true);

    ui->windowWidthSlider->blockSignals(true);
    ui->windowLevelSlider->blockSignals(true);

    int maxWidth = qMax(10000, static_cast<int>(metadata.windowWidth * 2));
    ui->windowWidthSlider->setMaximum(maxWidth);
    ui->windowWidthSlider->setValue(static_cast<int>(metadata.windowWidth));

    int minLevel = qMin(-5000, static_cast<int>(metadata.windowCenter - metadata.windowWidth));
    int maxLevel = qMax(5000, static_cast<int>(metadata.windowCenter + metadata.windowWidth));
    ui->windowLevelSlider->setMinimum(minLevel);
    ui->windowLevelSlider->setMaximum(maxLevel);
    ui->windowLevelSlider->setValue(static_cast<int>(metadata.windowCenter));

    ui->windowWidthSlider->blockSignals(false);
    ui->windowLevelSlider->blockSignals(false);

    QString patientDisplay = metadata.patientName.isEmpty() ? "--" : metadata.patientName;
    ui->patientNameLabel->setText(QString("Patient: %1").arg(patientDisplay));

    QString dateDisplay = metadata.studyDate;
    if (dateDisplay.length() == 8) {
        dateDisplay = QString("%1/%2/%3").arg(dateDisplay.mid(6, 2),
                                               dateDisplay.mid(4, 2),
                                               dateDisplay.mid(0, 4));
    }
    ui->studyDateLabel->setText(QString("Date: %1").arg(dateDisplay.isEmpty() ? "--" : dateDisplay));

    QString modalityDisplay = metadata.modality.isEmpty() ? "--" : metadata.modality;
    ui->modalityLabel->setText(QString("Modality: %1").arg(modalityDisplay));

    ui->dimensionsLabel->setText(QString("Size: %1 x %2").arg(metadata.columns).arg(metadata.rows));

    ui->bitsLabel->setText(QString("Bits: %1 (%2 stored)").arg(metadata.bitsAllocated).arg(metadata.bitsStored));
}

void MainWindow::onViewerError(const QString& error)
{
    QMessageBox::warning(this, "Erro", error);
}

void MainWindow::onWindowWidthChanged(int value)
{
    if (m_viewer && m_viewer->hasImage()) {
        m_viewer->setWindowLevel(value, m_viewer->levelValue());
    }
}

void MainWindow::onWindowLevelChanged(int value)
{
    if (m_viewer && m_viewer->hasImage()) {
        m_viewer->setWindowLevel(m_viewer->windowValue(), value);
    }
}
