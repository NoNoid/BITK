#include "opencv2/opencv.hpp"

#include <stdio.h>  /* defines FILENAME_MAX */

#include "match.hpp"
#include "utilities.hpp"

using namespace cv;

bool programm();

int main(int, char**)
{
    // programm returns true, if it wants to be restarted
    while(programm());
    return 0;
}

bool programm()
{
    //diable printf() buffering
    setbuf(stdout, NULL);
    printf("press 'q' to quit\n");

    bool useWebcam = false;
    bool showResultOfMatching = true;
    VideoCapture videoHandle;

    if(useWebcam)
        {videoHandle = webcam();}
    else
    {
        std::string videoFileName = "videos/video_ampel1.mp4";
        videoHandle = videoFile(videoFileName);
    }

    Size videoDimensions = Size((int) videoHandle.get(CV_CAP_PROP_FRAME_WIDTH),
                                (int) videoHandle.get(CV_CAP_PROP_FRAME_HEIGHT));
    printf("VideoWidth: %d, VideoHeight: %d\n",videoDimensions.width,videoDimensions.height);

    int numberOfVideoFrames;
    if(!useWebcam)
    {
        numberOfVideoFrames = videoHandle.get(CV_CAP_PROP_FRAME_COUNT);
        printf("number of Frames: %d\n",numberOfVideoFrames);
    }

    std::string drawFrameWindowName("drawFrame"),outerFrameWindowName("outerFrame"),matchingWindowName("resultOfMatching");

    namedWindow(drawFrameWindowName,CV_WINDOW_AUTOSIZE);
    cvMoveWindow(drawFrameWindowName.c_str(),0,0);
    namedWindow(outerFrameWindowName,CV_WINDOW_AUTOSIZE);
    cvMoveWindow(outerFrameWindowName.c_str(), videoDimensions.width+55, 50);
    if(showResultOfMatching)
    {
        namedWindow(matchingWindowName,CV_WINDOW_AUTOSIZE);
        cvMoveWindow(matchingWindowName.c_str(), videoDimensions.width+55, 100 + 256);
    }

    // define location of sub matrices in image
    Rect innerFrame( videoDimensions.width/2-100, videoDimensions.height/2-50, 16, 16 );
    Scalar innerFrameColor(255,0,0);
    Rect searchFrame;
    Scalar searchFrameColor(0,255,0);

    cv::Point *bestMatchPositionsByFrame;
    if(!useWebcam)
        {bestMatchPositionsByFrame = new cv::Point[numberOfVideoFrames];}

    // Mouse Callback
    mouseEventInformation mouseInfo(&innerFrame, &videoDimensions, &searchFrame);
    setMouseCallback(drawFrameWindowName, mouseCallBack, (void*)&mouseInfo);

//    Mat edges;
    double oldMinVal, oldMaxVal;
    Point oldMinLoc, oldMaxLoc, oldMatchLocation;
    bool stopTheProgramm = false;
    bool resetTheProgramm = false;
    Mat frame;
    videoHandle >> frame;
    cv::cvtColor(frame,frame,CV_BGR2GRAY);
    Mat innerFrameMatrix(frame,innerFrame);

    for(unsigned int i = 1; (useWebcam ? true :i < numberOfVideoFrames) && !stopTheProgramm && !resetTheProgramm; ++i)
    {
        // get a new frame from camera
        videoHandle >> frame;

        // create Matrix we can draw into without overwriting data of the original image
        Mat drawFrame;
        frame.copyTo(drawFrame);
        cv::cvtColor(frame,frame,CV_BGR2GRAY);
        GaussianBlur(frame,frame,Size(3,3),0,0);

        DrawPoint(drawFrame,oldMatchLocation,Scalar(0,255,255));

        drawRectangle(innerFrame,drawFrame,innerFrameColor);


        searchFrame = createOuterFrameFromInnerFrame(innerFrame,videoDimensions);
        drawRectangle(searchFrame,drawFrame,searchFrameColor);
        Mat outerFrameMatrix(frame,searchFrame);

        Mat result;
        Point matchLocation = match(outerFrameMatrix, innerFrameMatrix, result);

        // the returned matchLocation is in the wrong Coordinate System, we need to transform it back
        matchLocation.x += searchFrame.x;
        matchLocation.y += searchFrame.y;
        oldMatchLocation = matchLocation;

        DrawPoint(drawFrame,matchLocation,Scalar(0,0,255));

        createNewInnerFrameFromMatchLocation(matchLocation, innerFrame, videoDimensions);

        Mat outerFrameZoomed(Point(256,256));
        resize(Mat(drawFrame,searchFrame),outerFrameZoomed,Size(256,256));
        imshow(outerFrameWindowName,outerFrameZoomed);

        if(showResultOfMatching)
        {
            Mat resultZoomed(Point(256,256));
            resize(result,resultZoomed,Size(256,256));
            imshow(matchingWindowName,resultZoomed);
        }

        imshow(drawFrameWindowName,drawFrame);
        // let the user interact with the programm
        char key;
        if(!useWebcam)
            key = waitKey(0);
        else
            key = waitKey(1);

        switch(key)
        {
        case 'q':
            stopTheProgramm = true;
            break;
        case 'r':
            resetTheProgramm = true;
            break;
        }

        innerFrameMatrix = Mat(frame,innerFrame);
    }

    if(!useWebcam)
        {delete[] bestMatchPositionsByFrame;}

    // the camera will be deinitialized automatically in VideoCapture destructor
    return resetTheProgramm;
}

