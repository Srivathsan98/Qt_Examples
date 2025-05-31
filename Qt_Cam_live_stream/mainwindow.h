#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QDateTime>
#include <opencv2/opencv.hpp>
#include <deque>
#include <mutex>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_startButton_clicked();
    void on_recordButton_clicked();
    void updateFrame();

private:
    void startRecording();
    void stopRecording();
    QString generateTimestampFilename();
    void saveBufferedFrames();

    Ui::MainWindow *ui;
    QTimer *timer;
    cv::VideoCapture cap;
    
    // Recording related members
    std::deque<cv::Mat> frameBuffer;
    cv::VideoWriter videoWriter;
    bool isRecording;
    std::mutex bufferMutex;
    static const int BUFFER_SIZE = 1800; // 1 minute at 30 FPS
    static const int RECORD_DURATION = 1800; // 1 minute at 30 FPS
    int framesRecorded;
};

#endif // MAINWINDOW_H
