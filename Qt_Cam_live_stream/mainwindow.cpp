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
    , isRecording(false)
    , framesRecorded(0)
{
    ui->setupUi(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateFrame);
    
    // Create recordings directory if it doesn't exist
    QDir().mkpath("recordings");
}

MainWindow::~MainWindow() {
    if (cap.isOpened()) {
        cap.release();
    }
    if (videoWriter.isOpened()) {
        videoWriter.release();
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

void MainWindow::on_recordButton_clicked() {
    if (!isRecording) {
        startRecording();
    }
}

void MainWindow::startRecording() {
    if (!cap.isOpened()) return;

    isRecording = true;
    framesRecorded = 0;
    
    // Get video properties
    int width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    double fps = 30.0;
    
    // Create video writer
    QString filename = generateTimestampFilename();
    videoWriter.open(filename.toStdString(), 
                    cv::VideoWriter::fourcc('m', 'p', '4', 'v'),
                    fps, cv::Size(width, height));
    
    if (!videoWriter.isOpened()) {
        qWarning("Failed to create video writer");
        isRecording = false;
        return;
    }
    
    // Save buffered frames (1 minute before)
    saveBufferedFrames();
}

void MainWindow::stopRecording() {
    if (videoWriter.isOpened()) {
        videoWriter.release();
    }
    isRecording = false;
    framesRecorded = 0;
}

QString MainWindow::generateTimestampFilename() {
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    return QString("recordings/recording_%1.mp4").arg(timestamp);
}

void MainWindow::saveBufferedFrames() {
    std::lock_guard<std::mutex> lock(bufferMutex);
    for (const auto& frame : frameBuffer) {
        videoWriter.write(frame);
    }
}

void MainWindow::updateFrame() {
    cv::Mat frame;
    if (cap.read(frame)) {
        // Store frame in buffer
        {
            std::lock_guard<std::mutex> lock(bufferMutex);
            frameBuffer.push_back(frame.clone());
            if (frameBuffer.size() > BUFFER_SIZE) {
                frameBuffer.pop_front();
            }
        }
        
        // If recording, write frame to video
        if (isRecording) {
            videoWriter.write(frame);
            framesRecorded++;
            
            // Stop recording after 1 minute (after button press)
            if (framesRecorded >= RECORD_DURATION) {
                stopRecording();
            }
        }
        
        // Display frame
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
        QImage qimg(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
        ui->cameraLabel->setPixmap(QPixmap::fromImage(qimg).scaled(
            ui->cameraLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}
