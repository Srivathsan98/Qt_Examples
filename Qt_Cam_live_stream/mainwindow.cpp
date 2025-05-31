#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QImage>
#include <QPixmap>
#include <QDir>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , timer(new QTimer(this))
{
    ui->setupUi(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateFrame);
    
}

MainWindow::~MainWindow() {
    if (cap.isOpened()) {
        cap.release();
    }
    delete ui;
}

void MainWindow::on_startButton_clicked() {
    if (!cap.isOpened()) {
        // cap.open(0);  // Open default camera
        cap.open("udpsrc port=5000 ! application/x-rtp, encoding-name=H264, payload=96 ! "
                 "rtph264depay ! avdec_h264 ! videoconvert ! appsink", cv::CAP_GSTREAMER);
        if (!cap.isOpened()) {
            qWarning("Failed to open camera");
            return;
        }
    }
    timer->start(30);  // ~33 FPS
}

void MainWindow::updateFrame() {
    cv::Mat frame;
    if (cap.read(frame)) {
        
        // Display frame
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
        QImage qimg(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
        ui->cameraLabel->setPixmap(QPixmap::fromImage(qimg).scaled(
            ui->cameraLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}
