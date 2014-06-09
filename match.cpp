#include "match.hpp"
#include <stdio.h>

//using namespace cv;

Point match(const Mat &outerFrameMatrix,const Mat &innerFrameMatrix, Mat &result)
{
    int match_method = CV_TM_CCOEFF_NORMED;

    int resultCols =  outerFrameMatrix.cols - innerFrameMatrix.cols + 1;
    int resultRows = outerFrameMatrix.rows - innerFrameMatrix.rows + 1;

    result = Mat(resultCols,resultRows,CV_32FC1);

    /// Do the Matching and Normalize
    matchTemplate( outerFrameMatrix, innerFrameMatrix, result, match_method );
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
    Point diff((outerFrameMatrix.cols - result.cols)/2 + 1 ,(outerFrameMatrix.rows-result.rows)/2 + 1);
    Point retVal(matchLoc + diff);
    return retVal;
}

float Mor(const Mat& sm, const Mat& rm, int x,int y, float rMean_new, float sMean_old, int umax_width, int vmax_height)
{

    float sum_r_uv = 0;
    float sum_s_uv = 0;
    float sum_r_uv_sq = 0;
    float sum_s_uv_sq = 0;

    for(int u = -umax_width/2; u < umax_width/2; u++)
    {
        for(int v = -vmax_height/2; v < vmax_height/2; v++)
        {
            float r_val = float(rm.at<uchar>(u,v)) - rMean_new;
            float s_val = float(sm.at<uchar>(x+u,y+v)) - sMean_old;
            sum_r_uv += r_val;
            sum_s_uv += s_val;
            sum_r_uv_sq += r_val*r_val;
            sum_s_uv_sq += s_val*s_val;
        }
    }
    return (2*sum_r_uv*sum_s_uv)/(sum_r_uv_sq+sum_s_uv_sq);
}

Point matchMOR(const Mat &outerFrameMatrix,const Mat &innerFrameMatrix, Mat &outResult)
{
    if(outerFrameMatrix.type() != CV_8UC1 || innerFrameMatrix.type() != CV_8UC1)
        {printf("input Matrices dont have the correct type");}
    Scalar_<uchar> meanOfOuterFrameMatrixScalar = mean(outerFrameMatrix);
    uchar meanOfOuterFrameMatrix = meanOfOuterFrameMatrixScalar[0];
    Scalar_<uchar> meanOfInnerFrameMatrixScalar = mean(innerFrameMatrix);
    uchar meanOfInnerFrameMatrix = meanOfInnerFrameMatrixScalar[0];

    int resultCols =  outerFrameMatrix.cols - innerFrameMatrix.cols + 1;
    int resultRows = outerFrameMatrix.rows - innerFrameMatrix.rows + 1;

    outResult = Mat(resultCols,resultRows,CV_32FC1);
    Point offset((outerFrameMatrix.cols - outResult.cols)/2 + 1 ,(outerFrameMatrix.rows-outResult.rows)/2 + 1);

    for(int y = offset.y, totalRowsToTraverse = outerFrameMatrix.rows - offset.y +1; y < totalRowsToTraverse; ++y)
    {
        for(int x = offset.x, totalColumnsToTraverse = outerFrameMatrix.cols - offset.x + 1; x < totalColumnsToTraverse; ++x)
        {
            outResult.at<float>(x-offset.x,y-offset.y) = Mor(outerFrameMatrix,innerFrameMatrix,x,y,float(meanOfOuterFrameMatrix),float(meanOfInnerFrameMatrix),innerFrameMatrix.cols,innerFrameMatrix.rows);
        }
    }
    normalize( outResult, outResult, 0, 1, NORM_MINMAX, -1);

    double maxVal;
    Point maxLoc;
    minMaxLoc( outResult, NULL, &maxVal, NULL, &maxLoc);

    namedWindow("testMor",CV_WINDOW_AUTOSIZE);
    Mat outResultZoomed(Point(256,256));
    resize(outResult,outResultZoomed,Size(256,256));
    imshow("testMor",outResultZoomed);

    return maxLoc + offset;
}

Point matchSAD(const Mat &OuterFrameMatrix,const Mat &InnerFrameMatrix, Mat &outResult)
{
    int outerX = OuterFrameMatrix.cols;
    int outerY = OuterFrameMatrix.rows;

    int innerX = InnerFrameMatrix.cols;
    int innerY = InnerFrameMatrix.rows;


    int resultCols =  OuterFrameMatrix.cols - InnerFrameMatrix.cols + 1;
    int resultRows = OuterFrameMatrix.rows - InnerFrameMatrix.rows + 1;
    float result[resultCols * resultRows];

    float minDifference = FLT_MAX;
    Point posMinDifferenceInOuterFrame;

    outResult = Mat(resultCols, resultRows, CV_32FC1);

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
                    difference += OuterFrameMatrix.at<uchar>(posInOuterFrame) - InnerFrameMatrix.at<uchar>(posInInnerFrame);
                }
            }
            if(difference < minDifference)
            {
                minDifference = difference;
                // get the mid-Point of the innerFrame in outerFrame-Coordinates
                posMinDifferenceInOuterFrame = Point(leftUpperX + innerX/2, leftUpperY + innerY/2);
            }
            result[leftUpperX + leftUpperY * resultCols] = difference;
            //outResult.at<float>(leftUpperX,leftUpperY) = difference;
        }
    }

    //minDifference, result hold additional information
    outResult = Mat(resultCols, resultRows, CV_32FC1, result);
    normalize( outResult, outResult, 0, 1, NORM_MINMAX, -1, Mat());
    return posMinDifferenceInOuterFrame;
}

void determineValues(const Mat &Matrix, float &minValue, float &maxValue, float &meanValue)
{
    meanValue = 0;
    minValue = FLT_MAX;
    maxValue = FLT_MIN;
    for(int y = 0; y < Matrix.rows; y++)
    {
        for(int x = 0; x < Matrix.cols; x++)
        {
            Point pos = Point(x, y);
            int value = Matrix.at<uchar>(pos);
            meanValue += value;
            if( value < minValue )
                minValue = value;
            if( value > maxValue)
                maxValue = value;
        }
    }
    meanValue /= Matrix.rows * Matrix.cols;
}

Point matchKKFMF(const Mat &OuterFrameMatrix,const Mat &InnerFrameMatrix, Mat &outResult)
{
    int outerX = OuterFrameMatrix.cols;
    int outerY = OuterFrameMatrix.rows;
    float outerMinValue;
    float outerMaxValue;
    float outerMeanValue;
    determineValues(OuterFrameMatrix, outerMinValue, outerMaxValue, outerMeanValue);

    int innerX = InnerFrameMatrix.cols;
    int innerY = InnerFrameMatrix.rows;
    float innerMinValue;
    float innerMaxValue;
    float innerMeanValue;
    determineValues(InnerFrameMatrix, innerMinValue, innerMaxValue, innerMeanValue);


    int resultCols =  OuterFrameMatrix.cols - InnerFrameMatrix.cols + 1;
    int resultRows = OuterFrameMatrix.rows - InnerFrameMatrix.rows + 1;
    float result[resultCols * resultRows];

    float maxSimilarity = FLT_MIN;
    Point posMaxSimilarityInOuterFrame;

    outResult = Mat(resultCols, resultRows, CV_32FC1);

    for(int leftUpperY = 0; leftUpperY < outerY - innerY; leftUpperY++)
    {
        for(int leftUpperX = 0; leftUpperX < outerX - innerX; leftUpperX++)
        {

            float similarity = 0;
            for(int offsetX = 0; offsetX <= innerX; offsetX++)
            {
                for(int offsetY = 0; offsetY <= innerY; offsetY++)
                {
                    Point posInOuterFrame = Point(leftUpperX + offsetX, leftUpperY + offsetY);
                    Point posInInnerFrame = Point(offsetX, offsetY);
                    float outerValue = (OuterFrameMatrix.at<uchar>(posInOuterFrame) - outerMeanValue) / outerMeanValue;
                    float innerValue = (InnerFrameMatrix.at<uchar>(posInInnerFrame) - innerMeanValue) / outerMeanValue;
                    similarity += outerValue * innerValue;
                }
            }
            if(similarity > maxSimilarity)
            {
                maxSimilarity = similarity;
                // get the mid-Point of the innerFrame in outerFrame-Coordinates
                posMaxSimilarityInOuterFrame = Point(leftUpperX + innerX/2, leftUpperY + innerY/2);
            }
            result[leftUpperX + leftUpperY * resultCols] = similarity;
            //outResult.at<float>(leftUpperX,leftUpperY) = difference;
        }
    }

    //minDifference, result hold additional information
    outResult = Mat(resultCols, resultRows, CV_32FC1, result);
    normalize( outResult, outResult, 0, 1, NORM_MINMAX, -1, Mat());
    return posMaxSimilarityInOuterFrame;
}
