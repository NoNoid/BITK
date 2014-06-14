#include <opencv2/opencv.hpp>

using namespace cv;

enum method
{
    SAD,
    SSD,
    KKFMF,
    MOR
};

Point matchKKFMF(const Mat &OuterFrameMatrix,const Mat &InnerFrameMatrix, Mat &outResult);

Point match(const Mat &outerFrameMatrix,const Mat &innerFrameMatrix, Mat &result);

float Mor(const Mat& sm, const Mat& rm, int x,int y, float rMean_new, float sMean_old, int umax_width, int vmax_height);

Point matchMOR(const Mat &outerFrameMatrix,const Mat &innerFrameMatrix, Mat &outResult);

Point matchSAD(const Mat &OuterFrameMatrix,const Mat &InnerFrameMatrix, Mat &outResult);

Point matchSSD(const Mat &OuterFrameMatrix,const Mat &InnerFrameMatrix, Mat &outResult);

void determineValues(const Mat &Matrix, float &minValue, float &maxValue, float &meanValue);

Point matchKKFMF(const Mat &OuterFrameMatrix,const Mat &InnerFrameMatrix, Mat &outResult);

Point matchSSDMS(const Mat &,const Mat &, Mat &);
