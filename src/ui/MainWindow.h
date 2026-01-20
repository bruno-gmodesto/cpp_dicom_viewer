#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <memory>
#include "../viewer/DicomViewer.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onOpenFileClicked();
    void onExitClicked();
    void onImageLoaded(const QString& filePath);
    void onViewerError(const QString& error);
    void onWindowWidthChanged(int value);
    void onWindowLevelChanged(int value);

private:
    void setupUi();
    void setupConnections();
    void applyStyles();
    void setupViewer();

    Ui::MainWindow *ui;
    viewer::DicomViewer* m_viewer = nullptr;
};

#endif // MAINWINDOW_H
