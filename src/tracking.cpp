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


int initCamera(Settings& settings,  bool show)
{
    capture.open(0);
    capture >> cap_frame1;
    capture >> cap_frame;
    
    if( !capture.isOpened() )
    {
        std::cerr << "***Could not initialize capturing...***\n";
        std::cerr << "Current parameter's value: \n";
        return -1;
    }    

    if (show)  {
        //create GUI windows
        namedWindow("Frame");
        namedWindow("FG Mask MOG 2");
        showVideo = true;
    }

    //create Background Subtractor objects
    //MOG2 approach
    pMOG2 = createBackgroundSubtractorMOG2(settings.tracking_history,
                                           settings.tracking_threshold, true);

    return 0;
}

Tracks processCamera(Settings& settings) 
{
    if (1) {
        capture >> cap_frame1;
        blur(cap_frame1, cap_frame, Size(settings.tracking_blur,settings.tracking_blur));
    } else {
        capture >> cap_frame;
    }
    
    if (settings.tracking_flip_horizontal) {
        if (settings.tracking_flip_vertical) {
            cv::flip(cap_frame,cap_frame,-1);            
        } else {
            cv::flip(cap_frame,cap_frame,1);            
        }
    } else if (settings.tracking_flip_vertical) {
        cv::flip(cap_frame,cap_frame,0);
    }
    
    if (settings.tracking_scale_x != 1.0 || settings.tracking_scale_y != 1.0) {
        cv::Size s = cap_frame.size();
        cv::Rect roi(0, 0, s.width / settings.tracking_scale_x, s.height / settings.tracking_scale_y);
        
        cv::Mat cropped = cap_frame(roi);
        cap_frame = cropped;
        
        // cap_frame = cv::subImage(cap_frame,cvRect(0, 0, s.width * settings.tracking_scale_x, s.height * settings.tracking_scale_y ));
        
        // IplImage frm = cap_frame;
        // cvSetImageROI(&frm, cvRect(0, 0, s.width * settings.tracking_scale_x, s.height * settings.tracking_scale_y ));
        // cap_frame = cvarrToMat(&frm);
        // std::cout << "size = " << cap_frame.size() << " scale = " << settings.tracking_scale_x << " " << settings.tracking_scale_y << std::endl;
        // cv::resize(cap_frame, cap_frame, cap_frame.size(), settings.tracking_scale_x, settings.tracking_scale_y);
    }
    
    IplImage frm = cap_frame;
    IplImage *frame;

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

    cvFilterByArea(blobs, settings.tracking_min_blob_area, settings.tracking_max_blob_area);
    cvUpdateTracks(blobs, tracks, settings.tracking_distance, settings.tracking_inactive);

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
            y = (float) (frm.height - it->second->miny) / frm.height;
#if 0
            if (settings.tracking_scale_x != 1.0) {
                x *= settings.tracking_scale_x;
                if (x < 0.0 || x > 1.0) continue;
            }
            
            if (settings.tracking_scale_y != 1.0) {
                y *= settings.tracking_scale_y;
                if (y < 0.0 || y > 1.0) continue;
            }
#endif
            
            x -= 0.5;
            float y_floor = y * (settings.tracking_far - settings.tracking_near) + settings.tracking_near;
            x = x * y_floor / settings.tracking_far;
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

