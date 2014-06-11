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
void mouseCallBack(int event, int x, int y, int flags, void* userdata)
{
    // change size of innerFrame
    if  ( event == EVENT_LBUTTONDOWN )
    {
        buttonDownPosition.x = x;
        buttonDownPosition.y = y;
    }
    if(buttonDownPosition.x != -1 && buttonDownPosition.y != -1)
    {
        mouseEventInformation* info = (mouseEventInformation*)userdata;
        if(x >= buttonDownPosition.x)
        {
            info->innerFrame->x = buttonDownPosition.x;
            info->innerFrame->width = (std::max(8, x - buttonDownPosition.x)) & ~1;
        }
        else //(x < buttonDownPosition.x)
        {
            info->innerFrame->x = x;
            info->innerFrame->width = (std::max(8,buttonDownPosition.x - x)) & ~1;
        }
        if(y >= buttonDownPosition.y)
        {
            info->innerFrame->y = buttonDownPosition.y;
            info->innerFrame->height = (std::max(8, y - buttonDownPosition.y)) & ~1;
        }
        else //(y < buttonDownPosition.y)
        {
            info->innerFrame->y = y;
            info->innerFrame->height = (std::max(8,buttonDownPosition.y - y)) & ~1;
        }

        *(info->searchFrame) = createOuterFrameFromInnerFrame(*(info->innerFrame), *(info->videoDimensions));
    }
    if( event == EVENT_LBUTTONUP)
    {
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
