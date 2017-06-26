#pragma once

#include "common.h"

#include <opencv2/imgproc.hpp>

// pHash from http://www.hackerfactor.com/blog/index.php?/archives/432-Looks-Like-It.html
static uint64_t image_hash(const cv::Mat& img)
{
    cv::Mat grey;
    cv::cvtColor(img, grey, CV_BGR2GRAY);    

    cv::Mat image;
    cv::resize(grey, image, cv::Size(kHashImageSize, kHashImageSize));

    cv::Mat input;
    image.convertTo(input, CV_32F, 1.0f/255.0f);

    assert(input.depth() == CV_32F);
    assert(input.type() == CV_32FC1);

    cv::Mat dct;
    cv::dct(input, dct);

    cv::Mat output(dct, cv::Rect(0, 0, kHashDctCount, kHashDctCount));
    output.at<float>(0, 0) = 0.f;

    float avg = cv::mean(output)[0];

    uint64_t hash = 0;
    for (size_t i=0; i<kHashDctCount; i++)
    {
        for (size_t j=0; j<kHashDctCount; j++)
        {
            hash |= (uint64_t)(output.at<float>(i, j) > avg) << (63 - (i*8+j));
        }
    }

    return hash;
}
