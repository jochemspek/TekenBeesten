/**
 * @file bg_sub.cpp
 * @brief Background subtraction tutorial sample code
 * @author Domenico D. Bloisi
 */

//opencv
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/photo.hpp>
//C
#include <stdio.h>
//C++
#include <iostream>
#include <sstream>
#include <cvblob.h>

using namespace cv;
using namespace cvb;
using namespace std;

// Global variables
Mat cap_frame; //current frame
Mat cap_frame1; //current frame
Mat fgMaskMOG2; //fg mask fg mask generated by MOG2 method
Ptr<BackgroundSubtractor> pMOG2; //MOG2 Background subtractor
int keyboard; //input from keyboard

/** Function Headers */
void help();
void processCamera();
void processVideo(char* videoFilename);
void processImages(char* firstFrameFilename);

void help()
{
    cout
    << "--------------------------------------------------------------------------" << endl
    << "This program shows how to use background subtraction methods provided by "  << endl
    << " OpenCV. You can process both videos (-vid) and images (-img)."             << endl
                                                                                    << endl
    << "Usage:"                                                                     << endl
    << "./bs {-vid <video filename>|-img <image filename>|-cam}"                    << endl
    << "for example: ./bs -vid video.avi"                                           << endl
    << "or: ./bs -img /data/images/1.png"                                           << endl
    << "--------------------------------------------------------------------------" << endl
    << endl;
}

/**
 * @function main
 */
int main(int argc, char* argv[])
{
    //print help information
    help();


    //create GUI windows
    namedWindow("Frame");
    namedWindow("FG Mask MOG 2");

    //create Background Subtractor objects
    pMOG2 = createBackgroundSubtractorMOG2(200, 10, true); //MOG2 approach

    if (argc > 1) {
        if(strcmp(argv[1], "-vid") == 0) {
            //input data coming from a video
            processVideo(argv[2]);
        } else if(strcmp(argv[1], "-cam") == 0) {
            //input data coming from a camera
            processCamera();
        } else if(strcmp(argv[1], "-img") == 0) {
            //input data coming from a sequence of images
            processImages(argv[2]);
        }
    } else {
        processCamera();
    }
    //destroy GUI windows
    destroyAllWindows();
    return EXIT_SUCCESS;
}

/**
 * @function processCamera
 */
void processCamera() {
    VideoCapture capture;
    CvTracks tracks;

    capture.open(0);
    capture >> cap_frame1;
    capture >> cap_frame;

    IplImage frm = cap_frame;
    IplImage *frame = cvCreateImage(cvGetSize(&frm), frm.depth, frm.nChannels);

    //read input data. ESC or 'q' for quitting
    while( (char)keyboard != 'q' && (char)keyboard != 27 ){
        if (1) {
            capture >> cap_frame1;
            // fastNlMeansDenoisingColored(frame1, frame);
            blur(cap_frame1, cap_frame, Size(7,7));
        } else {
            capture >> cap_frame;
        }

        //update the background model
        pMOG2->apply(cap_frame, fgMaskMOG2);
        dilate(fgMaskMOG2, fgMaskMOG2, Mat(), Point(-1, -1));

        if (1) {
            CvBlobs blobs;
            // cvResetImageROI(frame);
            // cvConvertScale(&img, frame, 1, 0);
            IplImage img = fgMaskMOG2;
            frame = cvCloneImage(&img);
            cvThreshold(frame, frame, 150, 255, CV_THRESH_BINARY);

            // cvSetImageROI(frame, cvRect(0, 25, 900, 500));

            IplImage *chB=cvCreateImage(cvGetSize(frame),8,1);
            cvSplit(frame,chB,NULL,NULL,NULL);

            IplImage *labelImg = cvCreateImage(cvGetSize(frame), IPL_DEPTH_LABEL, 1);

            unsigned int result = cvLabel(chB, labelImg, blobs);

            cvFilterByArea(blobs, 2000, 100000);

            cvUpdateTracks(blobs, tracks, 5., 10);
            // cvUpdateTracks(blobs, tracks, 10., 5);
            IplImage res = cap_frame;

            cvRenderBlobs(labelImg, blobs, &res, &res, CV_BLOB_RENDER_COLOR|CV_BLOB_RENDER_CENTROID|CV_BLOB_RENDER_BOUNDING_BOX);
            cvRenderTracks(tracks, &res, &res, CV_TRACK_RENDER_ID|CV_TRACK_RENDER_BOUNDING_BOX|CV_TRACK_RENDER_TO_LOG);
            cvReleaseBlobs(blobs);
            cvReleaseImage(&chB);
            cvReleaseImage(&frame);
            cvReleaseImage(&labelImg);

            Mat sh_img = cvarrToMat(&res);
            imshow("Frame", sh_img);
        } else {
            imshow("Frame", cap_frame);
        }  
        //show the current frame and the fg masks
        imshow("FG Mask MOG 2", fgMaskMOG2);
        //get the input from the keyboard
        keyboard = waitKey( 30 );
    }
}

/**
 * @function processVideo
 */
void processVideo(char* videoFilename) {
    //create the capture object
    VideoCapture capture(videoFilename);
    if(!capture.isOpened()){
        //error in opening the video input
        cerr << "Unable to open video file: " << videoFilename << endl;
        exit(EXIT_FAILURE);
    }
    //read input data. ESC or 'q' for quitting
    while( (char)keyboard != 'q' && (char)keyboard != 27 ){
        //read the current frame
        if(!capture.read(cap_frame)) {
            cerr << "Unable to read next frame." << endl;
            cerr << "Exiting..." << endl;
            exit(EXIT_FAILURE);
        }
        //update the background model
        pMOG2->apply(cap_frame, fgMaskMOG2);
        //get the frame number and write it on the current frame
        stringstream ss;
        rectangle(cap_frame, cv::Point(10, 2), cv::Point(100,20),
                  cv::Scalar(255,255,255), -1);
        ss << capture.get(CAP_PROP_POS_FRAMES);
        string frameNumberString = ss.str();
        putText(cap_frame, frameNumberString.c_str(), cv::Point(15, 15),
                FONT_HERSHEY_SIMPLEX, 0.5 , cv::Scalar(0,0,0));
        //show the current frame and the fg masks
        imshow("Frame", cap_frame);
        imshow("FG Mask MOG 2", fgMaskMOG2);
        //get the input from the keyboard
        keyboard = waitKey( 30 );
    }
    //delete capture object
    capture.release();
}

/**
 * @function processImages
 */
void processImages(char* fistFrameFilename) {
    //read the first file of the sequence
    cap_frame = imread(fistFrameFilename);
    if(cap_frame.empty()){
        //error in opening the first image
        cerr << "Unable to open first image frame: " << fistFrameFilename << endl;
        exit(EXIT_FAILURE);
    }
    //current image filename
    string fn(fistFrameFilename);
    //read input data. ESC or 'q' for quitting
    while( (char)keyboard != 'q' && (char)keyboard != 27 ){
        //update the background model
        pMOG2->apply(cap_frame, fgMaskMOG2);
        //get the frame number and write it on the current frame
        size_t index = fn.find_last_of("/");
        if(index == string::npos) {
            index = fn.find_last_of("\\");
        }
        size_t index2 = fn.find_last_of(".");
        string prefix = fn.substr(0,index+1);
        string suffix = fn.substr(index2);
        string frameNumberString = fn.substr(index+1, index2-index-1);
        istringstream iss(frameNumberString);
        int frameNumber = 0;
        iss >> frameNumber;
        rectangle(cap_frame, cv::Point(10, 2), cv::Point(100,20),
                  cv::Scalar(255,255,255), -1);
        putText(cap_frame, frameNumberString.c_str(), cv::Point(15, 15),
                FONT_HERSHEY_SIMPLEX, 0.5 , cv::Scalar(0,0,0));
        //show the current frame and the fg masks
        imshow("Frame", cap_frame);
        imshow("FG Mask MOG 2", fgMaskMOG2);
        //get the input from the keyboard
        keyboard = waitKey( 30 );
        //search for the next image in the sequence
        ostringstream oss;
        oss << (frameNumber + 1);
        string nextFrameNumberString = oss.str();
        string nextFrameFilename = prefix + nextFrameNumberString + suffix;
        //read the next frame
        cap_frame = imread(nextFrameFilename);
        if(cap_frame.empty()){
            //error in opening the next image in the sequence
            cerr << "Unable to open image frame: " << nextFrameFilename << endl;
            exit(EXIT_FAILURE);
        }
        //update the path of the current frame
        fn.assign(nextFrameFilename);
    }
}