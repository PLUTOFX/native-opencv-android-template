#include "dehaze.h"

extern "C" {
void JNICALL
Java_com_example_nativeopencvandroidtemplate_MainActivity_dehaze(JNIEnv *env,jobject instance,jlong matAddr) {

    // get Mat from raw address
    Mat &dehazedMat = *(Mat *) matAddr;
    Mat mat = *(Mat *) matAddr;
    width = dehazedMat.cols;
    height = dehazedMat.rows;
    __android_log_print(ANDROID_LOG_INFO, "OpenCV", "Mat rows: %d, cols: %d", dehazedMat.rows, dehazedMat.cols);

    processImage(mat, dehazedMat);

    dehazedMat.convertTo(dehazedMat, CV_8UC3);
    __android_log_print(ANDROID_LOG_INFO, "OpenCV", "Converting image to CV_8UC3");
    __android_log_print(ANDROID_LOG_INFO, "OpenCV", "dehazedMat rows: %d, cols: %d", dehazedMat.rows, dehazedMat.cols);

//    if (mat.rows !=  dehazedMat.rows || dehazedImage.cols != image.cols) {
//        resize(dehazedImage, dehazedImage, Size(image.cols, image.rows));
//        __android_log_print(ANDROID_LOG_INFO, "OpenCV", "Resized dehazedImage to match original size");
//    }
    __android_log_print(ANDROID_LOG_INFO, "OpenCV", "Mat address: %p", matAddr);

}
}

// True method for processing
void processImage(Mat& image, Mat& dehazedImage) {
    int s = 24;
    // 压缩分辨率
    double rate = 1.2;
    double eeps = 0.002, omega = 0.9;

    image.convertTo(image, CV_64FC3);
    vector<Mat> channels(3);
    split(image, channels);        // separate color channels

    Mat R = channels[2];
    Mat G = channels[1];
    Mat B = channels[0];


    if (frameCnt > -1) {
        est_air(R, G, B, s, &air_r, &air_g, &air_b);
    }
    resize(image, image, Size(image.cols / rate, image.rows / rate));

    split(image, channels);        // separate color channels
    R = channels[2];
    G = channels[1];
    B = channels[0];

    if (frameCnt > -1) {
        t = est_trans_fast(R, G, B, s, eeps, omega, air_r, air_g, air_b);
    }

    //Mat dehazedimage;
    dehazedImage = rmv_haze(R, G, B, t, air_r, air_g, air_b);
    resize(dehazedImage, dehazedImage, Size(width, height));
    image.release();
}

//---------------------- GUIDED FILTER -------------------//
Mat GF_smooth(Mat& src, int s, double epsilon, int samplingRate)
{
    Mat srcResize;
    src.convertTo(src, CV_64FC1);
    resize(src, srcResize, Size(src.cols / samplingRate, src.rows / samplingRate));
    s /= samplingRate;

    /*srcResize.convertTo(srcResize, CV_64FC1);*/
    Mat mean_I;
    blur(srcResize, mean_I, Size(s, s), Point(-1, -1));

    Mat II = srcResize.mul(srcResize);
    Mat var_I;
    blur(II, var_I, Size(s, s), Point(-1, -1));
    var_I = var_I - mean_I.mul(mean_I);

    Mat a = var_I / ((var_I + epsilon));
    Mat b = mean_I - a.mul(mean_I);

    blur(a, a, Size(s, s), Point(-1, -1));
    blur(b, b, Size(s, s), Point(-1, -1));
    resize(a, a, Size(src.cols, src.rows));
    resize(b, b, Size(src.cols, src.rows));

    return a.mul(src) + b;
}


Mat staticMin(Mat& I, int s, double eps, double alpha, int samplingRate)
{
    Mat mean_I = GF_smooth(I, s, eps, samplingRate);

    Mat var_I;
    blur((I - mean_I).mul(I - mean_I), var_I, Size(s, s), Point(-1, -1));

    Mat mean_var_I;
    blur(var_I, mean_var_I, Size(s, s), Point(-1, -1));

    Mat z_I;
    sqrt(mean_var_I, z_I);

    return mean_I - alpha * z_I;
}


//---------------------- DEHAZING FUNCTIONS -------------------//
// 估计大气光图
int est_air(Mat& R,Mat& G,Mat& B, int s, double* A_r, double* A_g, double* A_b)
{
    // Based on DCP
    Mat Im = min(min(R, G), B);

    /// Estimate airlight
    Mat blur_Im;
    blur(Im, blur_Im, Size(s, s), Point(-1, -1));

    int maxIdx[2] = { 0, 0 };
    minMaxIdx(blur_Im, NULL, NULL, NULL, maxIdx);

    int width = R.cols;
    *A_r = ((double*)R.data)[maxIdx[0] * R.cols + maxIdx[1]];
    *A_g = ((double*)G.data)[maxIdx[0] * R.cols + maxIdx[1]];
    *A_b = ((double*)B.data)[maxIdx[0] * R.cols + maxIdx[1]];

    return 0;
}

// 估计透射率
Mat est_trans_fast(Mat& R, Mat& G, Mat& B, int s, double eeps, double k, double A_r, double A_g, double A_b)
{

    /// Estimate transmission
    Mat R_n = R / A_r;
    Mat G_n = G / A_g;
    Mat B_n = B / A_b;

    Mat Im = min(min(R_n, G_n), B_n);

    eeps = (3 * 255 / (A_r + A_g + A_b)) * (3 * 255 / (A_r + A_g + A_b)) * eeps;
    double alpha = 2;
    int samplingRate = 4;
    Mat z_Im = staticMin(Im, s, eeps, alpha, samplingRate);

    return min(max(0.001, 1 - k * z_Im), 1);
}

// Remove haze with t & A
Mat rmv_haze(Mat& R, Mat& G, Mat& B, Mat& t, double A_r, double A_g, double A_b)
{
    vector<Mat> channels(3);
    /// Remove haze
    channels[2] = (R - A_r) / t + A_r;
    channels[1] = (G - A_g) / t + A_g;
    channels[0] = (B - A_b) / t + A_b;

//    double thresholdPercentage = 0.005;

//    post_process(channels, thresholdPercentage);
    Mat dst;
    merge(channels, dst);

    dst.convertTo(dst, CV_8UC3);
    return dst;
}

//void post_process(vector<Mat>& channels, double thresholdPercentage) {
//    double minThreshold = 255, maxThreshold = 0;
//    for (int c = 0; c < 3; c++) {
//        vector<double> pixels;
//        channels[c].reshape(1, 1).copyTo(pixels);
//        size_t pixelCount = pixels.size();
//        size_t minIdx = pixelCount * thresholdPercentage;
//        size_t maxIdx = pixelCount * (1 - thresholdPercentage);
//
//        nth_element(pixels.begin(), pixels.begin() + minIdx, pixels.end());
//        nth_element(pixels.begin(), pixels.begin() + maxIdx, pixels.end());
//        minThreshold = min(minThreshold, pixels[minIdx]);
//        maxThreshold = max(maxThreshold, pixels[maxIdx]);
//    }
//    //printf("min and max threshold: %d, %d\n", minThreshold, maxThreshold);
//
//    for (int c = 0; c < 3; c++) {
//        Mat& channel = channels[c];
//        double scale = 255.0 / (maxThreshold - minThreshold);
//        channel.forEach<double>([minThreshold, maxThreshold, scale](double& pixel, const int* position) -> void {
//            if (pixel <= minThreshold) {
//                //printf("%d, %d\n", *position, pixel);
//                pixel = minThreshold;
//            }
//            else if (pixel >= maxThreshold) {
//                pixel = maxThreshold;
//            }
//            else {
//                //pixel = 0;
//                pixel = static_cast<double>((pixel - minThreshold) * scale);
//                //printf("(%d, %d) = %d \n", position[0], position[1], pixel);
//
//            }
//        });
//    }
//}