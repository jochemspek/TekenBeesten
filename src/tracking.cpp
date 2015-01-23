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
#include "tracking.h"

using namespace cv;
using namespace cvb;
using namespace std;

VideoCapture capture;
CvTracks tracks;
Mat cap_frame; //current frame
Mat cap_frame1; //current frame
Mat fgMaskMOG2; //fg mask fg mask generated by MOG2 method
Ptr<BackgroundSubtractor> pMOG2; //MOG2 Background subtractor
int keyboard; //input from keyboard
bool showVideo = false;

int initCamera( bool show)
{
    if (show)  {
        //create GUI windows
        namedWindow("Frame");
        namedWindow("FG Mask MOG 2");
        showVideo = true;
    }

    //create Background Subtractor objects
    pMOG2 = createBackgroundSubtractorMOG2(TRACK_HISTORY, TRACK_THRESHOLD, true); //MOG2 approach

    capture.open(0);
    capture >> cap_frame1;
    capture >> cap_frame;
    return 0;
}

Tracks processCamera() 
{
    if (1) {
        capture >> cap_frame1;
        blur(cap_frame1, cap_frame, Size(TRACK_BLUR,TRACK_BLUR));
    } else {
        capture >> cap_frame;
    }
    
    IplImage frm = cap_frame;
    IplImage *frame = cvCreateImage(cvGetSize(&frm), frm.depth, frm.nChannels);

    //update the background model
    pMOG2->apply(cap_frame, fgMaskMOG2);
    dilate(fgMaskMOG2, fgMaskMOG2, Mat(), Point(-1, -1));

    CvBlobs blobs;
    IplImage img = fgMaskMOG2;
    frame = cvCloneImage(&img);
    cvThreshold(frame, frame, 150, 255, CV_THRESH_BINARY);

    // cvSetImageROI(frame, cvRect(0, 25, 900, 500));

    IplImage *chB=cvCreateImage(cvGetSize(frame),8,1);
    cvSplit(frame,chB,NULL,NULL,NULL);

    IplImage *labelImg = cvCreateImage(cvGetSize(frame), IPL_DEPTH_LABEL, 1);

    unsigned int result = cvLabel(chB, labelImg, blobs);

    cvFilterByArea(blobs, TRACK_MIN_BLOB_AREA, TRACK_MAX_BLOB_AREA);
    cvUpdateTracks(blobs, tracks, TRACK_DISTANCE, TRACK_INACTIVE);

    if (showVideo) {
        IplImage res = cap_frame;

        cvRenderBlobs(labelImg, blobs, &res, &res, CV_BLOB_RENDER_COLOR|CV_BLOB_RENDER_CENTROID|CV_BLOB_RENDER_BOUNDING_BOX);
        cvRenderTracks(tracks, &res, &res, CV_TRACK_RENDER_ID|CV_TRACK_RENDER_BOUNDING_BOX);
        Mat sh_img = cvarrToMat(&res);
        imshow("Frame", sh_img);
        imshow("FG Mask MOG 2", fgMaskMOG2);
        keyboard = waitKey( 30 );
    }
    cvReleaseBlobs(blobs);
    cvReleaseImage(&chB);
    cvReleaseImage(&frame);
    cvReleaseImage(&labelImg);

    Tracks trks;
    if (showVideo) cout << "FRAME +++++++++++++++++++++++++++ " << endl;
    for (CvTracks::const_iterator it=tracks.begin(); it != tracks.end(); ++it) {
        if (! it->second->inactive) {
            float x, y;
            x = (float) it->second->centroid.x / frm.width;
            y = (float) (frm.height - it->second->maxy) / frm.height;
            x -= 0.5;
            float y_floor = y * (TRACK_FAR - TRACK_NEAR) + TRACK_NEAR;
            x = x * y_floor / TRACK_FAR;
            y -= 0.5;

            Track t;
            t.id = it->second->id;
            t.x = x;
            t.y = y;

            trks.push_back(t);

            if (showVideo) {
                cout << "Track " << it->second->id << endl;
                cout << " - Lifetime " << it->second->lifetime << endl;
                cout << " - Active " << it->second->active << endl;
                cout << " - Bounding box: (" << it->second->minx << ", " << it->second->miny << ") - (" << it->second->maxx << ", " << it->second->maxy << ")" << endl;
                cout << " - Centroid: (" << it->second->centroid.x << ", " << it->second->centroid.y << ")" << endl;
                cout << " - track: " << t.id << " " << t.x << " " << t.y << endl;

                cout << endl;
            }
        }
    }

    return trks;
}

