#include "match.hpp"
#include "utilities.hpp"
#include <stdio.h>

//using namespace cv;

void drawBestMatchlocationInResult(const Point &maxLoc, Mat &outResult)
{
    cvtColor(outResult,outResult,CV_GRAY2BGR);
    DrawPoint(outResult,maxLoc,Scalar(0,0,255));
}

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

/*Point matchMOR(const Mat &outerFrameMatrix,const Mat &innerFrameMatrix, Mat &outResult)
{
    if(outerFrameMatrix.type() != CV_8UC1 || innerFrameMatrix.type() != CV_8UC1)
        {printf("input Matrices dont have the correct type\n");}
    if(innerFrameMatrix.cols%2 != 0 || innerFrameMatrix.rows%2 != 0 || outerFrameMatrix.cols%2 != 0 || outerFrameMatrix.rows%2 != 0)
        {printf("cols/rows are odd ic: %d, ir: %d, oc: %d or:%d\n",innerFrameMatrix.cols,innerFrameMatrix.rows,outerFrameMatrix.cols,outerFrameMatrix.rows); }
    Scalar_<uchar> meanOfOuterFrameMatrixScalar = mean(outerFrameMatrix);
    uchar meanOfOuterFrameMatrix = meanOfOuterFrameMatrixScalar[0];
    Scalar_<uchar> meanOfInnerFrameMatrixScalar = mean(innerFrameMatrix);
    uchar meanOfInnerFrameMatrix = meanOfInnerFrameMatrixScalar[0];

    int resultCols =  outerFrameMatrix.cols - innerFrameMatrix.cols + 1;
    int resultRows = outerFrameMatrix.rows - innerFrameMatrix.rows + 1;
    if(resultCols < 0 || resultRows < 0 )
    {
        printf("Inner greater than Outer\n");
        outResult = Mat(10,10,CV_32FC1);
        return Point(0,0);
    }

    outResult = Mat(resultRows,resultCols,CV_32FC1);
    Point offset(
                (outerFrameMatrix.cols - outResult.cols)/2 + 1,
                (outerFrameMatrix.rows - outResult.rows)/2 + 1
            );

    for(int y = offset.y, totalRows = outResult.rows+offset.y; y < totalRows; ++y)
    {
        for(int x = offset.x, totalCols = outResult.cols+offset.x; x < totalCols; ++x)
        {
            outResult.at<float>(y-offset.y,x-offset.x) = Mor(outerFrameMatrix,innerFrameMatrix,y,x,float(meanOfOuterFrameMatrix),float(meanOfInnerFrameMatrix),innerFrameMatrix.cols,innerFrameMatrix.rows);
        }
    }
    normalize( outResult, outResult, 0, 1, NORM_MINMAX, -1);

    double maxVal;
    Point maxLoc;
    minMaxLoc( outResult, NULL, &maxVal, NULL, &maxLoc);

    drawBestMatchlocationInResult(maxLoc, outResult);

    return maxLoc + offset;
}*/

//float SSD(const Mat& sm, const Mat& rm, int x,int y, float rMean_new, float sMean_old, int umax_width, int vmax_height)
//{
//    float sum_diff_r_s_sq = 0;
//
//    for(int u = -umax_width/2; u < umax_width/2; u++)
//    {
//        for(int v = -vmax_height/2; v < vmax_height/2; v++)
//        {
//            float r_val = float(rm.at<uchar>(u,v));
//            float s_val = float(sm.at<uchar>(x+u,y+v));
//            float diff_r_s = r_val - s_val;
//            float diff_r_s_sq = diff_r_s * diff_r_s;
//            sum_diff_r_s_sq += diff_r_s_sq;
//        }
//    }
//    return sum_diff_r_s_sq;
//}
//
//Point matchSSD(const Mat &outerFrameMatrix,const Mat &innerFrameMatrix, Mat &outResult)
//{
//    if(outerFrameMatrix.type() != CV_8UC1 || innerFrameMatrix.type() != CV_8UC1)
//        {printf("input Matrices dont have the correct type\n");}
//    if(innerFrameMatrix.cols%2 != 0 || innerFrameMatrix.rows%2 != 0 || outerFrameMatrix.cols%2 != 0 || outerFrameMatrix.rows%2 != 0)
//        {printf("cols/rows are odd ic: %d, ir: %d, oc: %d or:%d\n",innerFrameMatrix.cols,innerFrameMatrix.rows,outerFrameMatrix.cols,outerFrameMatrix.rows); }
//    Scalar_<uchar> meanOfOuterFrameMatrixScalar = mean(outerFrameMatrix);
//    uchar meanOfOuterFrameMatrix = meanOfOuterFrameMatrixScalar[0];
//    Scalar_<uchar> meanOfInnerFrameMatrixScalar = mean(innerFrameMatrix);
//    uchar meanOfInnerFrameMatrix = meanOfInnerFrameMatrixScalar[0];
//
//    int resultCols =  outerFrameMatrix.cols - innerFrameMatrix.cols + 1;
//    int resultRows = outerFrameMatrix.rows - innerFrameMatrix.rows + 1;
//    if(resultCols < 0 || resultRows < 0 )
//    {
//        printf("Inner greater than Outer\n");
//        outResult = Mat(10,10,CV_32FC1);
//        return Point(0,0);
//    }
//
//    outResult = Mat(resultRows,resultCols,CV_32FC1);
//    Point offset(
//                (outerFrameMatrix.cols - outResult.cols)/2 + 1,
//                (outerFrameMatrix.rows - outResult.rows)/2 + 1
//            );
//
//    float bestMatch = FLT_MAX;
//    Point minLoc;
//    for(int y = offset.y, totalRows = outResult.rows+offset.y; y < totalRows; ++y)
//    {
//        for(int x = offset.x, totalCols = outResult.cols+offset.x; x < totalCols; ++x)
//        {
//            float currentMatch = SSD(outerFrameMatrix,innerFrameMatrix,y,x,float(meanOfOuterFrameMatrix),float(meanOfInnerFrameMatrix),innerFrameMatrix.cols,innerFrameMatrix.rows);
//            outResult.at<float>(y-offset.y,x-offset.x) = currentMatch;
//            if(currentMatch < bestMatch)
//            {
//                bestMatch = currentMatch;
//                minLoc = Point(y-offset.y,x-offset.x);
//            }
//        }
//    }
//    double maxVal;
//    minMaxLoc( outResult, NULL, &maxVal, NULL , NULL);
//    cv::subtract(maxVal,outResult,outResult);
//    normalize( outResult, outResult,1,0,NORM_MINMAX);
//
//    Point minLoc;
//    minMaxLoc( outResult, NULL, NULL, &minLoc , NULL);
//
//    drawBestMatchlocationInResult(minLoc, outResult);
//
//    return minLoc + offset;
//}

float SSDMS(const Mat& sm, const Mat& rm, int x,int y, float rMean_new, float sMean_old, int umax_width, int vmax_height)
{
    float sum_diff_r_s_sq = 0;
    const float scale_val = float(sm.at<uchar>(x,y));

    for(int u = -umax_width/2; u < umax_width/2; u++)
    {
        for(int v = -vmax_height/2; v < vmax_height/2; v++)
        {

            float s_val = (rMean_new/scale_val) * float(sm.at<uchar>(x+u,y+v));
            float r_val = float(rm.at<uchar>(u,v));
            float diff_r_s = r_val - s_val;
            float diff_r_s_sq = diff_r_s * diff_r_s;
            sum_diff_r_s_sq += diff_r_s_sq;
        }
    }
    return sum_diff_r_s_sq;
}

Point matchSSDMS(const Mat &outerFrameMatrix,const Mat &innerFrameMatrix, Mat &outResult)
{
    if(outerFrameMatrix.type() != CV_8UC1 || innerFrameMatrix.type() != CV_8UC1)
        {printf("input Matrices dont have the correct type\n");}
    if(innerFrameMatrix.cols%2 != 0 || innerFrameMatrix.rows%2 != 0 || outerFrameMatrix.cols%2 != 0 || outerFrameMatrix.rows%2 != 0)
        {printf("cols/rows are odd ic: %d, ir: %d, oc: %d or:%d\n",innerFrameMatrix.cols,innerFrameMatrix.rows,outerFrameMatrix.cols,outerFrameMatrix.rows); }
    Scalar_<uchar> meanOfOuterFrameMatrixScalar = mean(outerFrameMatrix);
    uchar meanOfOuterFrameMatrix = meanOfOuterFrameMatrixScalar[0];
    Scalar_<uchar> meanOfInnerFrameMatrixScalar = mean(innerFrameMatrix);
    uchar meanOfInnerFrameMatrix = meanOfInnerFrameMatrixScalar[0];

    int resultCols =  outerFrameMatrix.cols - innerFrameMatrix.cols + 1;
    int resultRows = outerFrameMatrix.rows - innerFrameMatrix.rows + 1;
    if(resultCols < 0 || resultRows < 0 )
    {
        printf("Inner greater than Outer\n");
        outResult = Mat(10,10,CV_32FC1);
        return Point(0,0);
    }

    outResult = Mat(resultRows,resultCols,CV_32FC1);
    Point offset(
                (outerFrameMatrix.cols - outResult.cols)/2 + 1,
                (outerFrameMatrix.rows - outResult.rows)/2 + 1
            );

    for(int y = offset.y, totalRows = outResult.rows+offset.y; y < totalRows; ++y)
    {
        for(int x = offset.x, totalCols = outResult.cols+offset.x; x < totalCols; ++x)
        {
            outResult.at<float>(y-offset.y,x-offset.x) = SSDMS(outerFrameMatrix,innerFrameMatrix,y,x,float(meanOfOuterFrameMatrix),float(meanOfInnerFrameMatrix),innerFrameMatrix.cols,innerFrameMatrix.rows);
        }
    }
    normalize( outResult, outResult, 0, 1, NORM_MINMAX, -1);

    Point minLoc;
    minMaxLoc( outResult, NULL, NULL, &minLoc, NULL);

    drawBestMatchlocationInResult(minLoc, outResult);

    return minLoc + offset;
}

Point matchSAD(const Mat &OuterFrameMatrix,const Mat &InnerFrameMatrix, Mat &outResult)
{
    int outerX = OuterFrameMatrix.cols;
    int outerY = OuterFrameMatrix.rows;

    int innerX = InnerFrameMatrix.cols;
    int innerY = InnerFrameMatrix.rows;

    int resultCols =  outerX - innerX + 1;
    int resultRows = outerY - innerY + 1;
    outResult = Mat(resultRows, resultCols, CV_32FC1);

    float minDifference = FLT_MAX;
    Point posMinDifferenceInOuterFrame;

    for(int leftUpperY = 0; leftUpperY <= outerY - innerY; leftUpperY++)
    {
        for(int leftUpperX = 0; leftUpperX <= outerX - innerX; leftUpperX++)
        {

            float difference = 0;
            for(int offsetX = 0; offsetX <= innerX; offsetX++)
            {
                for(int offsetY = 0; offsetY <= innerY; offsetY++)
                {
                    Point posInOuterFrame = Point(leftUpperX + offsetX, leftUpperY + offsetY);
                    Point posInInnerFrame = Point(offsetX, offsetY);
                    float dif = OuterFrameMatrix.at<uchar>(posInOuterFrame) - InnerFrameMatrix.at<uchar>(posInInnerFrame);
                    difference += std::abs(dif);
                }
            }
            if(difference < minDifference)
            {
                minDifference = difference;
                // get the mid-Point of the innerFrame in outerFrame-Coordinates
                posMinDifferenceInOuterFrame = Point(leftUpperX + innerX/2, leftUpperY + innerY/2);
            }
            outResult.at<float>(leftUpperY,leftUpperX) = difference;
        }
    }

    //minDifference, result hold additional information
    normalize( outResult, outResult, 1, 0, NORM_MINMAX, -1, Mat());
    return posMinDifferenceInOuterFrame;
}

Point matchSSD(const Mat &OuterFrameMatrix,const Mat &InnerFrameMatrix, Mat &outResult)
{
    int outerX = OuterFrameMatrix.cols;
    int outerY = OuterFrameMatrix.rows;

    int innerX = InnerFrameMatrix.cols;
    int innerY = InnerFrameMatrix.rows;

    int resultCols =  outerX - innerX + 1;
    int resultRows = outerY - innerY + 1;
    outResult = Mat(resultRows, resultCols, CV_32FC1);

    float minDifference = FLT_MAX;
    Point posMinDifferenceInOuterFrame;

    for(int leftUpperY = 0; leftUpperY <= outerY - innerY; leftUpperY++)
    {
        for(int leftUpperX = 0; leftUpperX <= outerX - innerX; leftUpperX++)
        {

            float difference = 0;
            for(int offsetX = 0; offsetX <= innerX; offsetX++)
            {
                for(int offsetY = 0; offsetY <= innerY; offsetY++)
                {
                    Point posInOuterFrame = Point(leftUpperX + offsetX, leftUpperY + offsetY);
                    Point posInInnerFrame = Point(offsetX, offsetY);
                    float dif = OuterFrameMatrix.at<uchar>(posInOuterFrame) - InnerFrameMatrix.at<uchar>(posInInnerFrame);
                    difference += dif * dif;
                }
            }
            if(difference < minDifference)
            {
                minDifference = difference;
                // get the mid-Point of the innerFrame in outerFrame-Coordinates
                posMinDifferenceInOuterFrame = Point(leftUpperX + innerX/2, leftUpperY + innerY/2);
            }
            outResult.at<float>(leftUpperY,leftUpperX) = difference;
        }
    }

    //minDifference, result hold additional information
    normalize( outResult, outResult, 1, 0, NORM_MINMAX, -1, Mat());
    return posMinDifferenceInOuterFrame;
}

float meanValue(const Mat &Matrix)
{
    float mean = 0;
    for(int y = 0; y < Matrix.rows; y++)
    {
        for(int x = 0; x < Matrix.cols; x++)
        {
            int value = Matrix.at<uchar>(y,x);
            mean += value;
        }
    }
    return mean /= Matrix.rows * Matrix.cols;
}

Point matchKKFMF(const Mat &OuterFrameMatrix,const Mat &InnerFrameMatrix, Mat &outResult)
{
    int outerX = OuterFrameMatrix.cols;
    int outerY = OuterFrameMatrix.rows;
    float outerMeanValue = meanValue(OuterFrameMatrix);

    int innerX = InnerFrameMatrix.cols;
    int innerY = InnerFrameMatrix.rows;
    float innerMeanValue = meanValue(InnerFrameMatrix);

    int resultCols =  OuterFrameMatrix.cols - InnerFrameMatrix.cols + 1;
    int resultRows = OuterFrameMatrix.rows - InnerFrameMatrix.rows + 1;
    //float result[resultCols * resultRows];

    float maxSimilarity = FLT_MIN;
    Point posMaxSimilarityInOuterFrame;

    outResult = Mat(resultRows, resultCols, CV_32FC1);

    for(int leftUpperY = 0; leftUpperY <= outerY - innerY; leftUpperY++)
    {
        for(int leftUpperX = 0; leftUpperX <= outerX - innerX; leftUpperX++)
        {

            float similarity = 0;
            for(int offsetX = 0; offsetX <= innerX; offsetX++)
            {
                for(int offsetY = 0; offsetY <= innerY; offsetY++)
                {
                    Point posInOuterFrame = Point(leftUpperX + offsetX, leftUpperY + offsetY);
                    Point posInInnerFrame = Point(offsetX, offsetY);
                    float outerValue = (OuterFrameMatrix.at<uchar>(posInOuterFrame) - outerMeanValue) / outerMeanValue;
                    float innerValue = (InnerFrameMatrix.at<uchar>(posInInnerFrame) - innerMeanValue) / innerMeanValue;
                    similarity += outerValue * innerValue;
                }
            }
            if(similarity > maxSimilarity)
            {
                maxSimilarity = similarity;
                // get the mid-Point of the innerFrame in outerFrame-Coordinates
                posMaxSimilarityInOuterFrame = Point(leftUpperX + innerX/2, leftUpperY + innerY/2);
            }
            //result[leftUpperX + leftUpperY * resultCols] = similarity;
            outResult.at<float>(leftUpperY,leftUpperX) = similarity;
        }
    }

    //minDifference, result hold additional information
    //outResult = Mat(resultCols, resultRows, CV_32FC1, result);
    normalize( outResult, outResult, 0, 1, NORM_MINMAX, -1, Mat());
    return posMaxSimilarityInOuterFrame;
}


Point matchMOR(const Mat &OuterFrameMatrix,const Mat &InnerFrameMatrix, Mat &outResult)
{
    int outerX = OuterFrameMatrix.cols;
    int outerY = OuterFrameMatrix.rows;
    float outerMeanValue = meanValue(OuterFrameMatrix);

    int innerX = InnerFrameMatrix.cols;
    int innerY = InnerFrameMatrix.rows;
    float innerMeanValue = meanValue(InnerFrameMatrix);

    int resultCols =  OuterFrameMatrix.cols - InnerFrameMatrix.cols + 1;
    int resultRows = OuterFrameMatrix.rows - InnerFrameMatrix.rows + 1;
    //float result[resultCols * resultRows];

    float maxSimilarity = FLT_MIN;
    Point posMaxSimilarityInOuterFrame;

    outResult = Mat(resultRows, resultCols, CV_32FC1);

    for(int leftUpperY = 0; leftUpperY <= outerY - innerY; leftUpperY++)
    {
        for(int leftUpperX = 0; leftUpperX <= outerX - innerX; leftUpperX++)
        {

            float innerSum = 0;
            float outerSum = 0;
            float similarity = 0;
            for(int offsetX = 0; offsetX <= innerX; offsetX++)
            {
                for(int offsetY = 0; offsetY <= innerY; offsetY++)
                {
                    Point posInOuterFrame = Point(leftUpperX + offsetX, leftUpperY + offsetY);
                    Point posInInnerFrame = Point(offsetX, offsetY);
                    float outerValue = (OuterFrameMatrix.at<uchar>(posInOuterFrame) - outerMeanValue);
                    float innerValue = (InnerFrameMatrix.at<uchar>(posInInnerFrame) - innerMeanValue);
                    similarity += outerValue * innerValue;
                    innerSum += innerValue*innerValue;
                    outerSum += outerValue*outerValue;
                }
            }
            similarity = 2*similarity / (innerSum * outerSum);
            if(similarity > maxSimilarity)
            {
                maxSimilarity = similarity;
                // get the mid-Point of the innerFrame in outerFrame-Coordinates
                posMaxSimilarityInOuterFrame = Point(leftUpperX + innerX/2, leftUpperY + innerY/2);
            }
            //result[leftUpperX + leftUpperY * resultCols] = similarity;
            outResult.at<float>(leftUpperY,leftUpperX) = similarity;
        }
    }

    //minDifference, result hold additional information
    //outResult = Mat(resultCols, resultRows, CV_32FC1, result);
    normalize( outResult, outResult, 0, 1, NORM_MINMAX, -1, Mat());
    return posMaxSimilarityInOuterFrame;
}
