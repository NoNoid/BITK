#include <opencv2/opencv.hpp>

using namespace cv;

void clampRectangleToVideoDemensions(Rect &searchFrame,const Size &videoDimensions);

Rect createOuterFrameFromInnerFrame(const Rect &regionOfInterest,const Size &videoDimensions, int factor = 2);

void DrawPoint( Mat &img,const Point &center,const Scalar &Color);

void drawRectangle(const Rect &rectangleToDraw,Mat &matrixToDrawTheRectangleIn,const Scalar &color);

void createNewInnerFrameFromMatchLocation(const Point &matchLoc,Rect &regionOfInterest,const Size &videoDimensions);

void drawRectangle(const Rect &rectangleToDraw,Mat &matrixToDrawTheRectangleIn,const Scalar &color);

void mouseCallBack(int event, int x, int y, int flags, void* userdata);

VideoCapture webcam(const int cameraIndex = 0);

VideoCapture videoFile(const std::string &videoFileName);

struct mouseEventInformation
{
    mouseEventInformation(Rect* innerFrame, Size* videoDimensions, Rect* searchFrame)
        : innerFrame(innerFrame), videoDimensions(videoDimensions), searchFrame(searchFrame){}
    Rect* innerFrame;
    Size* videoDimensions;
    Rect* searchFrame;
};
