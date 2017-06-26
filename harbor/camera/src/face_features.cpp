#include "face_features.h"

#include <fstream>

#include <opencv2/core.hpp>
#include <cpptorch/cpptorch.h>

static std::shared_ptr<cpptorch::nn::Layer<float>> torch_net;

bool init_face_features()
{
    std::ifstream inf("/usr/local/share/nn4.small2.v1.t7");
    if (!inf)
    {
        printf("ERROR: Cannot open torch nn file\n");
        return false;
    }

    torch_net = cpptorch::read_net<float>(cpptorch::load(inf).get());
    if (!torch_net)
    {
        printf("ERROR: Cannot load torch nn file\n");
        return false;
    }

    return true;
}

std::vector<float> get_face_features(const cv::Mat& image)
{
    assert(image.rows == kAlignedImageSize);
    assert(image.cols == kAlignedImageSize);
    assert(image.depth() == CV_8U);
    assert(image.type() == CV_8UC3);

    cpptorch::Tensor<float> input;
    input.create();
    input.resize({1, 3, kAlignedImageSize, kAlignedImageSize});

    const uint8_t* img = image.ptr(0);
    for (size_t c=0; c<3; c++)
    {
        for (size_t p=0; p<kAlignedImageSize * kAlignedImageSize; p++)
        {
            input.data()[c * kAlignedImageSize * kAlignedImageSize + p] = (float)img[p * 3 + 2 - c] / 255.0f;
        }
    }

    cpptorch::Tensor<float> output = torch_net->forward(input);

    return std::vector<float>(output.data(), output.data() + kFeatureCount);
}
