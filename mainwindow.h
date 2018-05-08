#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <opencv/cv.hpp>
#include <opencv/highgui.h>
#include <math.h>


#include <QLabel>
#include <QMainWindow>
#include <QtCore>

using namespace cv;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    int findBestMatch(Mat& orig, Mat& other, int startX, int startY, int patchWidth, int patchHeight);
    int sumSquareDiff(Mat& orig, Mat& other, int origX, int origY, int otherX, int otherY, int patchWidth, int patchHeight, double origStdDev, double origAvg);

private slots:
    void displayFrame();

    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    cv::Mat left;
    cv::Mat right;
    cv::VideoCapture cam;
    cv::VideoCapture cam2;
    int rowDiff = 0;
    bool diffSet = false;

};

#endif // MAINWINDOW_H
