#include "interface.h"

#include <vector>
#include <iostream>

#include <android/log.h>

#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "libface", __VA_ARGS__))
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO , "libface", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN , "libface", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "libface", __VA_ARGS__))

MtcnnDetector g_mtcnnDetector;
Arcface g_arcFace;


//outputdat is x y x y
vector<FaceInfo> face_detect(ncnn::Mat ncnn_img)
{
    int st, et, cnt;
    double costtime;
    st = clock();
    vector<FaceInfo> results = g_mtcnnDetector.Detect(ncnn_img);
    et = clock();
    costtime = et - st;
    LOGD("detect face cost %fs\n", costtime / CLOCKS_PER_SEC);

    return results;
}

vector<float> face_exactfeature(ncnn::Mat img, FaceInfo info)
{
    int st, et, cnt;
    double costtime;
    st = clock();
    ncnn::Mat det = preprocess(img, info);
    et = clock();
    costtime = et - st;
    LOGD("face_exactfeature preprocess cost %fs\n", costtime / CLOCKS_PER_SEC);
    st = clock();
    vector<float>feature = g_arcFace.getFeature(det);
    et = clock();
    costtime = et - st;
    LOGD("face_exactfeature getFeature cost %fs\n", costtime / CLOCKS_PER_SEC);

    return feature;
}

float face_calcSimilar(std::vector<float> feature1, std::vector<float> feature2)
{
    double similar = 0;
    int st, et;
    double costtime;

    st = clock();
    similar = calcSimilar(feature1, feature2);
    et = clock();
    costtime = et - st;
    LOGD("calcSimilar cost %fs result %f\n", costtime / CLOCKS_PER_SEC, similar);
    return (float)similar;
}

float face_calcSimilar_ext(ncnn::Mat img1, ncnn::Mat img2, FaceInfo face1, FaceInfo face2)
{
    std::vector<float> feature1 = face_exactfeature(img1, face1);
    std::vector<float> feature2 = face_exactfeature(img2, face2);

    return calcSimilar(feature1, feature2);
}

//#include <opencv2/opencv.hpp>

//using namespace cv;
//using namespace std;

/*
cv::Mat ncnn2cv(ncnn::Mat img)
{
    unsigned char pix[img.h * img.w * 3];
    img.to_pixels(pix, ncnn::Mat::PIXEL_BGR);
    cv::Mat cv_img(img.h, img.w, CV_8UC3);
    for (int i = 0; i < cv_img.rows; i++)
    {
        for (int j = 0; j < cv_img.cols; j++)
        {
            cv_img.at<cv::Vec3b>(i,j)[0] = pix[3 * (i * cv_img.cols + j)];
            cv_img.at<cv::Vec3b>(i,j)[1] = pix[3 * (i * cv_img.cols + j) + 1];
            cv_img.at<cv::Vec3b>(i,j)[2] = pix[3 * (i * cv_img.cols + j) + 2];
        }
    }
    return cv_img;
}

int main(int argc, char* argv[])
{
    Mat img1;
    Mat img2;
    if (argc == 3)
    {
        img1 = imread(argv[1]);
        img2 = imread(argv[2]);
    }
    else{
        img1 = imread("../image/gyy1.jpeg");
        img2 = imread("../image/gyy2.jpeg");
    }
    ncnn::Mat ncnn_img1 = ncnn::Mat::from_pixels(img1.data, ncnn::Mat::PIXEL_BGR, img1.cols, img1.rows);
    ncnn::Mat ncnn_img2 = ncnn::Mat::from_pixels(img2.data, ncnn::Mat::PIXEL_BGR, img2.cols, img2.rows);

    MtcnnDetector detector("../models");

    double start = (double)getTickCount();
    vector<FaceInfo> results1 = detector.Detect(ncnn_img1);
    cout << "Detection Time: " << (getTickCount() - start) / getTickFrequency() << "s" << std::endl;

    start = (double)getTickCount();
    vector<FaceInfo> results2 = detector.Detect(ncnn_img2);
    cout << "Detection Time: " << (getTickCount() - start) / getTickFrequency() << "s" << std::endl;

    ncnn::Mat det1 = preprocess(ncnn_img1, results1[0]);
    ncnn::Mat det2 = preprocess(ncnn_img2, results2[0]);

    //for (auto it = results1.begin(); it != results1.end(); it++)
    //{
    //    rectangle(img1, cv::Point(it->x[0], it->y[0]), cv::Point(it->x[1], it->y[1]), cv::Scalar(0, 255, 0), 2);
    //    circle(img1, cv::Point(it->landmark[0], it->landmark[1]), 2, cv::Scalar(0, 255, 0), 2);
    //    circle(img1, cv::Point(it->landmark[2], it->landmark[3]), 2, cv::Scalar(0, 255, 0), 2);
    //    circle(img1, cv::Point(it->landmark[4], it->landmark[5]), 2, cv::Scalar(0, 255, 0), 2);
    //    circle(img1, cv::Point(it->landmark[6], it->landmark[7]), 2, cv::Scalar(0, 255, 0), 2);
    //    circle(img1, cv::Point(it->landmark[8], it->landmark[9]), 2, cv::Scalar(0, 255, 0), 2);
    //}

    //for (auto it = results2.begin(); it != results2.end(); it++)
    //{
    //    rectangle(img2, cv::Point(it->x[0], it->y[0]), cv::Point(it->x[1], it->y[1]), cv::Scalar(0, 255, 0), 2);
    //    circle(img2, cv::Point(it->landmark[0], it->landmark[1]), 2, cv::Scalar(0, 255, 0), 2);
    //    circle(img2, cv::Point(it->landmark[2], it->landmark[3]), 2, cv::Scalar(0, 255, 0), 2);
    //    circle(img2, cv::Point(it->landmark[4], it->landmark[5]), 2, cv::Scalar(0, 255, 0), 2);
    //    circle(img2, cv::Point(it->landmark[6], it->landmark[7]), 2, cv::Scalar(0, 255, 0), 2);
    //    circle(img2, cv::Point(it->landmark[8], it->landmark[9]), 2, cv::Scalar(0, 255, 0), 2);
    //}

    Arcface arc("../models");

    start = (double)getTickCount();
    vector<float> feature1 = arc.getFeature(det1);
    cout << "Extraction Time: " << (getTickCount() - start) / getTickFrequency() << "s" << std::endl;

    start = (double)getTickCount();
    vector<float> feature2 = arc.getFeature(det2);
    cout << "Extraction Time: " << (getTickCount() - start) / getTickFrequency() << "s" << std::endl;

    std::cout << "Similarity: " << calcSimilar(feature1, feature2) << std::endl;;

    //imshow("img1", img1);
    //imshow("img2", img2);

    imshow("det1", ncnn2cv(det1));
    imshow("det2", ncnn2cv(det2));

    waitKey(0);
    return 0;
}
*/