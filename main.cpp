// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html

#include <opencv2/objdetect.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>
#include <iomanip>

using namespace cv;
using namespace std;

int status = 1;
Mat frame;
Mat black;



class Detector
{
    enum Mode { Default, Daimler } m;
    HOGDescriptor hog, hog_d;
public:
    Detector() : m(Default), hog(), hog_d(Size(48, 96), Size(16, 16), Size(8, 8), Size(8, 8), 9)
    {
        hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());
        hog_d.setSVMDetector(HOGDescriptor::getDaimlerPeopleDetector());
    }
    void toggleMode() { m = (m == Default ? Daimler : Default); }
    string modeName() const { return (m == Default ? "Default" : "Daimler"); }
    vector<Rect> detect(InputArray img)
    {
        // Run the detector with default parameters. to get a higher hit-rate
        // (and more false alarms, respectively), decrease the hitThreshold and
        // groupThreshold (set groupThreshold to 0 to turn off the grouping completely).
        vector<Rect> found;
        if (m == Default)
            hog.detectMultiScale(img, found, 0, Size(8,8), Size(32,32), 1.05, 2, false);
        else if (m == Daimler)
            hog_d.detectMultiScale(img, found, 0.5, Size(8,8), Size(32,32), 1.05, 2, true);
        return found;
    }
    void adjustRect(Rect & r) const
    {
        // The HOG detector returns slightly larger rectangles than the real objects,
        // so we slightly shrink the rectangles to get a nicer output.
        r.x += cvRound(r.width*0.1);
        r.width = cvRound(r.width*0.8);
        r.y += cvRound(r.height*0.07);
        r.height = cvRound(r.height*0.8);
    }

    void adjustRectBackSub(Rect & r) const
    {
        // The HOG detector returns slightly larger rectangles than the real objects,
        // so we slightly shrink the rectangles to get a nicer output.

//        r.x += cvRound(r.width*0.1);
//        r.width = cvRound(r.width*0.8);
//        r.y += cvRound(r.height*0.07);
//        r.height = cvRound(r.height*0.8);

//        r.x += cvRound(r.width);
//        r.width = cvRound(r.width);
//        r.y += cvRound(r.height);
//        r.height = cvRound(r.height);

        for (int row = r.y; row < r.height; row++) {
            for (int col = r.x; col < r.width; col++) {

                Vec3b blackPixel = black.at<Vec3b>(Point(col, row));
                Vec3b colorPixel = frame.at<Vec3b>(Point(col, row));

                blackPixel[0] = colorPixel[0];
                blackPixel[1] = colorPixel[1];
                blackPixel[2] = colorPixel[2];

//                blackPixel[0] = 0;
//                blackPixel[1] = 0;
//                blackPixel[2] = 255;

                black.at<Vec3b>(Point(col,row)) = blackPixel;

            }
        }
    }
};

static const string keys = "{ help h   |   | print help message }"
                           "{ camera c | 0 | capture video from camera (device index starting from 0) }"
                           "{ video v  |   | use video as input }";



void on_trackbar( int, void* ){
    if (status == 1){
        status = 0;
        //black = Mat(frame.rows,frame.cols,frame.type());
        //black.setTo(Scalar(0,0,0));
        //destroyWindow("Pedestrian Detection");
        imshow("Background Subtracted", black);
    }else {
        status = 1;
        frame = frame.clone();
    }
    //cout << "Status of bg was changed to: " << status << endl;
}

int main(int argc, char** argv)
{
    //namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"

    //Create trackbars in "Control" window
    //createButton(b1, background(b1), b1, CV_PUSH_BUTTON, 0);
    //int low = 0;
    //int status = 0;
    //createTrackbar("changeBG", "Control", &low, 1, on_trackbar); //Amount of background visible (0 - 100)

    CommandLineParser parser(argc, argv, keys);
    parser.about("This sample demonstrates the use ot the HoG descriptor.");
    if (parser.has("help"))
    {
        parser.printMessage();
        return 0;
    }
    int camera = 1; //parser.get<int>("camera");
    string file = parser.get<string>("video");
    if (!parser.check())
    {
        parser.printErrors();
        return 1;
    }

    VideoCapture cap;
    if (file.empty())
        cap.open(camera);
    else
        cap.open(file.c_str());
    if (!cap.isOpened())
    {
        cout << "Can not open video stream: '" << (file.empty() ? "<camera>" : file) << "'" << endl;
        return 2;
    }

    cout << "Press 'q' or <ESC> to quit." << endl;
    cout << "Press <space> to toggle between Default and Daimler detector" << endl;
    Detector detector;
    //Mat frame;

    while (true){

        cap >> frame;
        cap >> black;
        black.setTo(Scalar(0,0,0));

        if (frame.empty())
        {
            cout << "Finished reading: empty frame" << endl;
            break;
        }
        int64 t = getTickCount();
        vector<Rect> found = detector.detect(frame);
        vector<Rect> found2 = detector.detect(black);

        t = getTickCount() - t;

//        // show the window
//        {
//            ostringstream buf;
//            buf << "Mode: " << detector.modeName() << " ||| "
//                << "FPS: " << fixed << setprecision(1) << (getTickFrequency() / (double)t);
//            putText(frame, buf.str(), Point(10, 30), FONT_HERSHEY_PLAIN, 2.0, Scalar(0, 0, 255), 2, LINE_AA);
//        }


        for (vector<Rect>::iterator i = found.begin(); i != found.end(); ++i){

            Rect &r = *i;
            //detector.adjustRect(r);
            rectangle(frame, r.tl(), r.br(), cv::Scalar(0, 255, 0), 2);
        }

        for (vector<Rect>::iterator j = found.begin(); j != found.end(); ++j){
            Rect &r = *j;
            detector.adjustRectBackSub(r);

            rectangle(black, r.tl(), r.br(), cv::Scalar(0, 255, 0), 2);
        }

        imshow("Pedestrian Detection", frame);
        imshow("Background Subtracted", black);


        // Use can end program with 'Q' button
        const char key = (char)waitKey(30);
        if (key == 27 || key == 'q') // ESC
        {
            cout << "Exit requested" << endl;
            break;
        }
        else if (key == ' ')
        {
            detector.toggleMode();
        }
    }
    return 0;
}
