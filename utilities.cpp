#include "utilities.hpp"
#include <stdio.h>

#ifdef WINDOWS
    #include <direct.h>
    #define GetCurrentDir _getcwd
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
 #endif

using namespace cv;

void clampRectangleToVideoDemensions(Rect &searchFrame,const Size &videoDimensions)
{
    if(searchFrame.x <= 0)
        searchFrame.x = 0;

    if(searchFrame.x >= videoDimensions.width-searchFrame.width)
        searchFrame.x = videoDimensions.width-searchFrame.width;

    if(searchFrame.y <= 0)
        searchFrame.y = 0;

    if(searchFrame.y >= videoDimensions.height-searchFrame.height)
        searchFrame.y = videoDimensions.height-searchFrame.height;
}

Rect createOuterFrameFromInnerFrame(const Rect &regionOfInterest,const Size &videoDimensions, int factor )
{
    Rect searchFrame(
                regionOfInterest.x-((factor*regionOfInterest.width)/2-regionOfInterest.width/2),
                regionOfInterest.y-((factor*regionOfInterest.height)/2-regionOfInterest.height/2),
                factor*regionOfInterest.width,
                factor*regionOfInterest.height);
    clampRectangleToVideoDemensions(searchFrame, videoDimensions);

    return searchFrame;
}

void DrawPoint( Mat &img,const Point &center,const Scalar &Color)
{
 int thickness = -1;
 int lineType = 1;
 double radius = 0.5;

 circle( img,
         center,
         radius,
         Color,
         thickness,
         lineType );
}

void drawRectangle(const Rect &rectangleToDraw,Mat &matrixToDrawTheRectangleIn,const Scalar &color)
{
    line(matrixToDrawTheRectangleIn,
         Point(rectangleToDraw.x,rectangleToDraw.y),
         Point(rectangleToDraw.x+rectangleToDraw.width,rectangleToDraw.y),
         color,
         0,1);
    line(matrixToDrawTheRectangleIn,
         Point(rectangleToDraw.x+rectangleToDraw.width,rectangleToDraw.y),
         Point(rectangleToDraw.x+rectangleToDraw.width,rectangleToDraw.y+rectangleToDraw.height),
         color,
         0,1);
    line(matrixToDrawTheRectangleIn,
         Point(rectangleToDraw.x+rectangleToDraw.width,rectangleToDraw.y+rectangleToDraw.height),
         Point(rectangleToDraw.x,rectangleToDraw.y+rectangleToDraw.height),
         color,
         0,1);
    line(matrixToDrawTheRectangleIn,
         Point(rectangleToDraw.x,rectangleToDraw.y+rectangleToDraw.height),
         Point(rectangleToDraw.x,rectangleToDraw.y),
         color,
         0,1);
}


void createNewInnerFrameFromMatchLocation(const Point &matchLoc,Rect &regionOfInterest,const Size &videoDimensions)
{
    regionOfInterest = Rect(
                matchLoc.x-regionOfInterest.width/2,
                matchLoc.y-regionOfInterest.height/2,
                regionOfInterest.width,
                regionOfInterest.height);
    clampRectangleToVideoDemensions(regionOfInterest,videoDimensions);
}

Point buttonDownPosition(-1,-1);
Point p1, p2;
void mouseCallBack(int event, int x, int y, int flags, void* userdata)
{
    // change size of innerFrame
    if  ( event == EVENT_LBUTTONDOWN )
    {
        buttonDownPosition.x = x;
        buttonDownPosition.y = y;
        p1 = Point(x,y);
    }
    if(buttonDownPosition.x != -1 && buttonDownPosition.y != -1)
    {
        Rect* innerFrame = (Rect*)userdata;
        if(x >= buttonDownPosition.x)
        {
            innerFrame->x = buttonDownPosition.x;
            innerFrame->width = std::max(1, x - buttonDownPosition.x);
        }
        else //(x < buttonDownPosition.x)
        {
            innerFrame->x = x;
            innerFrame->width = buttonDownPosition.x - x;
        }
        if(y >= buttonDownPosition.y)
        {
            innerFrame->y = buttonDownPosition.y;
            innerFrame->height = std::max(1, y - buttonDownPosition.y);
        }
        else //(y < buttonDownPosition.y)
        {
            innerFrame->y = y;
            innerFrame->height = buttonDownPosition.y - y;
        }
    }
    if( event == EVENT_LBUTTONUP)
    {
        Rect* innerFrame = (Rect*)userdata;
        p2 = Point(x,y);
        buttonDownPosition.x = -1;
        buttonDownPosition.y = -1;
    }
}

VideoCapture webcam(const int cameraIndex)
{
    printf ("Successfully opened Camera with Index: %d\n",cameraIndex);
    return VideoCapture(cameraIndex);
}

VideoCapture videoFile(const std::string &videoFileName)
{
    char cCurrentPath[FILENAME_MAX];

    if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
    {
        exit(-1);
    }

    cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */

    VideoCapture videoHandle = VideoCapture(videoFileName); // open the default camera

    if(!videoHandle.isOpened())  // check if we succeeded
    {
        printf ("Could not find File: %s/%s\n", cCurrentPath,videoFileName.c_str());
        exit(-1);
    }else{
        printf ("Successfully loaded File: %s/%s\n", cCurrentPath,videoFileName.c_str());
    }

    return videoHandle;
}
