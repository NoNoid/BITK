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

int main(int, char**)
{
    //diable printf() buffering
    setbuf(stdout, NULL);

    char cCurrentPath[FILENAME_MAX];

    if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
    {
        return -1;
    }

    cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */

    std::string videoFileName = "videos/Megamind.avi";
    VideoCapture videoHandle(videoFileName); // open the default camera
    if(!videoHandle.isOpened())  // check if we succeeded
    {
        printf ("Could not find File: %s/%s\n", cCurrentPath,videoFileName.c_str());
        return -1;
    }

    Size videoDimensions = Size((int) videoHandle.get(CV_CAP_PROP_FRAME_WIDTH),
                                (int) videoHandle.get(CV_CAP_PROP_FRAME_HEIGHT));
    printf("VideoWidth: %d,VideoHeight: %d\n",videoDimensions.width,videoDimensions.height);

    unsigned int numberOfVideoFrames = videoHandle.get(CV_CAP_PROP_FRAME_COUNT);
    printf("number of Frames: %d\n",numberOfVideoFrames);

    std::string drawFrameWindowName("DrawFrame"),ROIWindowName("ROIWindow"),matchingWindowName("ResultOfMatching");

    namedWindow(drawFrameWindowName,CV_WINDOW_AUTOSIZE);
    namedWindow(ROIWindowName,CV_WINDOW_AUTOSIZE);
    namedWindow(matchingWindowName,CV_WINDOW_AUTOSIZE);

    // define location of sub matrices in image
    Rect regionOfInterest( 100, 100, 100, 100 );
    cv::Point *bestMatchPositionsByFrame = new cv::Point[numberOfVideoFrames];

//    Mat edges;

    for(unsigned int i = 0; i < numberOfVideoFrames; ++i)
    {
        Mat frame;
        // get a new frame from camera
        videoHandle >> frame;

//        cvtColor(frame, edges, CV_BGR2GRAY);
//        GaussianBlur(edges, edges, Size(7,7), 1.5, 1.5);
//        Canny(edges, edges, 0, 30, 3);

        // define sub matrices in main matrix
        Mat RegionOfInterestMatrix(frame,regionOfInterest);

        int result_cols =  frame.cols - RegionOfInterestMatrix.cols + 1;
        int result_rows = frame.rows - RegionOfInterestMatrix.rows + 1;

        Mat result(result_cols,result_rows,CV_32FC1);

        /// Do the Matching and Normalize
        matchTemplate( frame, RegionOfInterestMatrix, result, CV_TM_CCOEFF_NORMED );
        normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat());

        double minVal, maxVal;
        Point minLoc, maxLoc, matchLoc;

        minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

        matchLoc = minLoc;
        bestMatchPositionsByFrame[i] = matchLoc;
        bestMatchPositionsByFrame[i].x += regionOfInterest.width/2;
        bestMatchPositionsByFrame[i].y += regionOfInterest.height/2;

        regionOfInterest.x = matchLoc.x - regionOfInterest.width/2;
        regionOfInterest.y = matchLoc.y - regionOfInterest.height/2;

        if(regionOfInterest.x <= 0)
            regionOfInterest.x = 0;

        if(regionOfInterest.x >= videoDimensions.width-regionOfInterest.width)
            regionOfInterest.x = videoDimensions.width-regionOfInterest.width;

        if(regionOfInterest.y <= 0)
            regionOfInterest.y = 0;

        if(regionOfInterest.y >= videoDimensions.height-regionOfInterest.height)
            regionOfInterest.y = videoDimensions.height-regionOfInterest.height;

        Mat drawFrame;
        frame.copyTo(drawFrame);
        if(i > 2)
        {
            for(int j = 2;j<=i;++j)
            {
                line(drawFrame,bestMatchPositionsByFrame[j-1],bestMatchPositionsByFrame[j],Scalar( 0, 255, 255 ),2,8);
            }
        }
//        if(i > 1)
//        line(drawFrame,bestMatchPositionsByFrame[i-1],bestMatchPositionsByFrame[i],Scalar( 255, 255, 255 ),2,8);

        printf("best Match Position: %d , %d\n",matchLoc.x,matchLoc.y);

        imshow(drawFrameWindowName, drawFrame);
        imshow(ROIWindowName, RegionOfInterestMatrix);
        imshow(matchingWindowName, result );

        if(waitKey(30) >= 0) break;
    }

    delete[] bestMatchPositionsByFrame;

    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}

