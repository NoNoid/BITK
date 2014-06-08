#include "opencv2/opencv.hpp"

#include <stdio.h>  /* defines FILENAME_MAX */

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

Rect crateSearchFrameFromRegionOfInterest(const Rect &regionOfInterest,const Size &videoDimensions, int factor = 2)
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

Point match(const Mat &SearchFrameMatrix,const Mat &RegionOfInterestMatrix, Mat &result)
{
    int match_method = CV_TM_CCOEFF;

    int resultCols =  SearchFrameMatrix.cols - RegionOfInterestMatrix.cols + 1;
    int resultRows = SearchFrameMatrix.rows - RegionOfInterestMatrix.rows + 1;

    result = Mat(resultCols,resultRows,CV_32FC1);

    /// Do the Matching and Normalize
    matchTemplate( SearchFrameMatrix, RegionOfInterestMatrix, result, match_method );
    normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat());

    double minVal, maxVal;
    Point minLoc, maxLoc, matchLoc;

    minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc);

    /// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
    if(  match_method == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED )
    {
        matchLoc = minLoc;
        rectangle( result, Point( matchLoc.x - result.cols/10 , matchLoc.y - result.rows/10 ), Point( matchLoc.x + result.cols/10 , matchLoc.y + result.rows/10 ), Scalar::all(255), 1, 1, 0 );
    }
    else
    {
        matchLoc = maxLoc;
        rectangle( result, Point( matchLoc.x - result.cols/10 , matchLoc.y - result.rows/10 ), Point( matchLoc.x + result.cols/10 , matchLoc.y + result.rows/10 ), Scalar::all(0), 1, 1, 0 );
    }
    //printf("matchLoc: = (%d,%d)",matchLoc.x,matchLoc.y);

    //printf("matchLoc: = (%f,%f)",double(SearchFrameMatrix.cols - result.cols)/2.0,double(SearchFrameMatrix.rows-result.rows)/2.0);
    Point diff((SearchFrameMatrix.cols - result.cols)/2 + 1 ,(SearchFrameMatrix.rows-result.rows)/2 + 1);
    Point retVal(matchLoc + diff);
    return retVal;
}

Point match_2(const Mat &OuterFrame,const Mat &InnerFrame, Mat &outResult)
{

    int outerX = OuterFrame.cols;
    int outerY = OuterFrame.rows;

    int innerX = InnerFrame.cols;
    int innerY = InnerFrame.rows;


    int resultCols =  OuterFrame.cols - InnerFrame.cols + 1;
    int resultRows = OuterFrame.rows - InnerFrame.rows + 1;
    float result[resultCols * resultRows];

    float minDifference = FLT_MAX;
    Point posMinDifferenceInOuterFrame;

    for(int leftUpperY = 0; leftUpperY < outerY - innerY; leftUpperY++)
    {
        for(int leftUpperX = 0; leftUpperX < outerX - innerX; leftUpperX++)
        {

            float difference = 0;
            for(int offsetX = 0; offsetX <= innerX; offsetX++)
            {
                for(int offsetY = 0; offsetY <= innerY; offsetY++)
                {
                    Point posInOuterFrame = Point(leftUpperX + offsetX, leftUpperY + offsetY);
                    Point posInInnerFrame = Point(offsetX, offsetY);
                    difference += OuterFrame.at<int>(posInOuterFrame) - InnerFrame.at<int>(posInInnerFrame);
                }
            }
            if(difference < minDifference)
            {
                minDifference = difference;
                // get the mid-Point of the innerFrame in outerFrame-Coordinates
                posMinDifferenceInOuterFrame = Point(leftUpperX + innerX/2, leftUpperY + innerY/2);
            }
            result[leftUpperX + leftUpperY * resultCols] = difference;

        }
    }

    //minDifference, result hold additional information
    outResult = Mat(resultCols, resultRows, CV_32FC1, result);
    return posMinDifferenceInOuterFrame;
}

void createNewRegionOfInterestFromMatchLocation(const Point &matchLoc,Rect &regionOfInterest,const Size &videoDimensions)
{
    regionOfInterest = Rect(
                matchLoc.x-regionOfInterest.width/2,
                matchLoc.y-regionOfInterest.height/2,
                regionOfInterest.width,
                regionOfInterest.height);
    clampRectangleToVideoDemensions(regionOfInterest,videoDimensions);
}

void mouseCallBack(int event, int x, int y, int flags, void* userdata)
{
    if  ( event == EVENT_LBUTTONDOWN )
    {
        Rect* innerFrame = (Rect*)userdata;
        innerFrame->x = x - innerFrame->width / 2;
        innerFrame->y = y - innerFrame->height / 2;
    }

}

void webcam()
{
    /*
    cv::VideoCapture cap(0);

    cv::Mat frame;
    cv::Mat eye_tpl;  // The eye template
    cv::Rect eye_bb;  // The eye bounding box

    while(cv::waitKey(15) != 'q')
    {
        cap >> frame;
        cv::Mat gray;
        cv::cvtColor(frame, gray, CV_BGR2GRAY);

        cv::imshow("video", frame);
    }*/

    VideoCapture Camcap(0); // open the default camera id == 0
    // Exit if fail to open a Webcam
    if( !Camcap.isOpened() ) {
        fprintf(stderr, " Fail to open a Camera\n" );
        exit(1);
    }

    // For storage the image from webcam
    Mat CamImage;
    // Create a window : "Camera Window" , 0 :allow user adjust the size
    namedWindow("Camera Window", 0 );

    while(cv::waitKey(15) != 'c') {
            // Retrieve the image from camera ID:0 then store in CamImage
            Camcap.retrieve( CamImage , 0 );
            // Displays the image in the specified window name
            imshow("Camera Window", CamImage );
    }

}

int main(int, char**)
{

    webcam();

    //diable printf() buffering
    setbuf(stdout, NULL);
    printf("press 'c' to close\n");

    char cCurrentPath[FILENAME_MAX];

    if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
    {
        return -1;
    }

    cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */

    std::string videoFileName = "videos/video_mitBahn.mp4";
    VideoCapture videoHandle(0); // open the default camera

    if(!videoHandle.isOpened())  // check if we succeeded
    {
        printf ("Could not find File: %s/%s\n", cCurrentPath,videoFileName.c_str());
        return -1;
    }else{
        printf ("Successfully loaded File: %s/%s\n", cCurrentPath,videoFileName.c_str());
    }

    Size videoDimensions = Size((int) videoHandle.get(CV_CAP_PROP_FRAME_WIDTH),
                                (int) videoHandle.get(CV_CAP_PROP_FRAME_HEIGHT));
    printf("VideoWidth: %d, VideoHeight: %d\n",videoDimensions.width,videoDimensions.height);

    unsigned int numberOfVideoFrames = videoHandle.get(CV_CAP_PROP_FRAME_COUNT);
    printf("number of Frames: %d\n",numberOfVideoFrames);

    std::string drawFrameWindowName("drawFrame"),outerFrameWindowName("outerFrame"),matchingWindowName("resultOfMatching");

    namedWindow(drawFrameWindowName,CV_WINDOW_AUTOSIZE);
    namedWindow(outerFrameWindowName,CV_WINDOW_AUTOSIZE);
    namedWindow(matchingWindowName,CV_WINDOW_AUTOSIZE);

    // define location of sub matrices in image
    Rect innerFrame( videoDimensions.width/2-100, videoDimensions.height/2-50, 16, 16 );
    Scalar outerFrame(255,0,0);
    Rect searchFrame;
    Scalar searchFrameColor(0,255,0);

    cv::Point *bestMatchPositionsByFrame = new cv::Point[numberOfVideoFrames];

    // Mouse Callback
    setMouseCallback(drawFrameWindowName, mouseCallBack, (void*)&innerFrame);

//    Mat edges;
    double oldMinVal, oldMaxVal;
    Point oldMinLoc, oldMaxLoc, oldMatchLocation;
    bool stopTheProgramm = false;


    for(unsigned int i = 0; i < numberOfVideoFrames; ++i)
    {
        Mat frame;
        // get a new frame from camera
        videoHandle >> frame;

        // create Matrix we can draw into without overwriting data of the original image
        Mat drawFrame;
        frame.copyTo(drawFrame);
        GaussianBlur(frame,frame,Size(3,3),0,0);

        DrawPoint(drawFrame,oldMatchLocation,Scalar(0,255,255));

        drawRectangle(innerFrame,drawFrame,outerFrame);
        Mat RegionOfInterestMatrix(frame,innerFrame);

        searchFrame = crateSearchFrameFromRegionOfInterest(innerFrame,videoDimensions);
        drawRectangle(searchFrame,drawFrame,searchFrameColor);
        Mat SearchFrameMatrix(frame,searchFrame);

        Mat result;
        Point matchLocation = match_2(SearchFrameMatrix,RegionOfInterestMatrix, result);

        // the returned matchLocation is in the wrong Coordinate System, we need to transform it back
        matchLocation.x += searchFrame.x;
        matchLocation.y += searchFrame.y;
        oldMatchLocation = matchLocation;

        DrawPoint(drawFrame,matchLocation,Scalar(0,0,255));


        //
        createNewRegionOfInterestFromMatchLocation(matchLocation, innerFrame, videoDimensions);


        Mat searchFrameZoomed(Point(256,256));
        resize(Mat(drawFrame,searchFrame),searchFrameZoomed,Size(256,256));
        imshow(outerFrameWindowName,searchFrameZoomed);

        Mat resultZoomed(Point(256,256));
        resize(result,resultZoomed,Size(256,256));
        imshow(matchingWindowName,resultZoomed);

        imshow(drawFrameWindowName,drawFrame);
        int key = waitKey(0);
        switch(key)
        {
        case ' ':
            continue;
            break;
        case 'c':
            stopTheProgramm = true;
            break;
        }
        if(stopTheProgramm) break;
    }

    delete[] bestMatchPositionsByFrame;

    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}

