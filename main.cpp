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

void getUserInput(bool &useWebCam, method &correlationMethod)
{
    std::string inString;
    std::cout << "use Webcam? (y/n)\n";
    std::cin >> inString;

    useWebCam = inString[0] == 'y' ? true : false;

    std::cout << "[1]: summed absolute Difference (SAD)\n"
              << "[2]: summed squared Diffenence (SSD)\n"
              << "[3]: normalized mean free crosscorrelation (KKFMF)\n"
              << "[4]: correlation of Moravec (MOR)\n";
    std::cin >> inString;
    correlationMethod;
    switch(inString[0])
    {
    case '1':   correlationMethod = SAD; break;
    case '2':   correlationMethod = SSD; break;
    case '3':   correlationMethod = KKFMF; break;
    case '4':   correlationMethod = MOR; break;
    default:    correlationMethod = KKFMF; break;
    }
}

bool programm()
{
    //diable printf() buffering
    setbuf(stdout, NULL);
    printf("press 'q' to quit, press 'r' to restart\n");

    bool useWebcam;
    method correlationMethod;
    getUserInput(useWebcam, correlationMethod);
    bool showResultOfMatching = true;

    VideoCapture videoHandle;

    if(useWebcam)
        {videoHandle = webcam();}
    else
    {
        string filename;
        std::cout << "filename?\n";
        getline(std::cin,filename);
        getline(std::cin,filename);

        std::string videoFileName = std::string("videos/") + filename + std::string(".mp4");
        try
        {
            videoHandle = videoFile(videoFileName);
        }
        catch(...)
        {
            videoFileName = "videos/trafficInChina.mp4";
            videoHandle = videoFile(videoFileName);
        }
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
        Point matchLocation;
        switch(correlationMethod)
        {
        case SAD:   matchLocation = matchSAD(outerFrameMatrix, innerFrameMatrix, result);   break;
        case SSD:   matchLocation = matchSSD(outerFrameMatrix, innerFrameMatrix, result);   break;
        case KKFMF: matchLocation = matchKKFMF(outerFrameMatrix, innerFrameMatrix, result); break;
        case MOR:   matchLocation = matchMOR(outerFrameMatrix, innerFrameMatrix, result);   break;
        default:    matchLocation = match(outerFrameMatrix, innerFrameMatrix, result);      break;
        }


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
        case '1':
            correlationMethod = SAD;
            break;
        case '2':
            correlationMethod = SSD;
            break;
        case '3':
            correlationMethod = KKFMF;
            break;
        case '4':
            correlationMethod = MOR;
            break;
        }

        innerFrameMatrix = Mat(frame,innerFrame);
    }

    if(!useWebcam)
        {delete[] bestMatchPositionsByFrame;}

    /* anyhow this doesn't work
    cvDestroyWindow(drawFrameWindowName.c_str());
    cvDestroyWindow(outerFrameWindowName.c_str());
    cvDestroyWindow(matchingWindowName.c_str());
    */

    // the camera will be deinitialized automatically in VideoCapture destructor
    return resetTheProgramm;
}

