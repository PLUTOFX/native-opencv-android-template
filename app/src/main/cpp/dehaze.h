#ifndef NATIVE_OPENCV_ANDROID_TEMPLATE_DEHAZE_H
#define NATIVE_OPENCV_ANDROID_TEMPLATE_DEHAZE_H
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <jni.h>
#include <android/log.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#define TAG "DEHAZE"

using namespace std;
using namespace cv;

int width;
int height;

//---------------------- Guided Filter -------------------//

//---------------------- Dehaze Functions -------------------//
double air_r, air_g, air_b;
Mat t;
int frameCnt;

Mat GF_smooth(Mat& src, int s, double epsilon,int samplingRate);
Mat staticMin(Mat& I, int s, double eeps, double alpha, int samplingRate);
void est_air(Mat& R, Mat& G, Mat& B, int s, double* A_r, double* A_g, double* A_b);
Mat est_trans_fast(Mat& R, Mat& G, Mat& B, int s, double eeps, double k, double A_r, double A_g, double A_b);
Mat rmv_haze(Mat& R, Mat& G, Mat& B, Mat& t, double A_r, double A_g, double A_b);
//void post_process(vector<Mat>& channels, double thresholdPercentage);
void dehazeProcess(Mat& image, Mat& dehazedImage);

extern "C" {
void JNICALL
Java_com_example_nativeopencvandroidtemplate_MainActivity_dehaze(JNIEnv *env,jobject instance,jlong matAddr);}

#endif //NATIVE_OPENCV_ANDROID_TEMPLATE_DEHAZE_H
