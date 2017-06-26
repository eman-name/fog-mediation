#include "face_detect.h"

#include <opencv2/imgproc.hpp>

#include <dlib/opencv.h>
#include <dlib/image_processing/full_object_detection.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/shape_predictor.h>

static dlib::frontal_face_detector dlib_detector;
static dlib::shape_predictor dlib_predictor;

// face landmarks from
// https://openface-api.readthedocs.io/en/latest/_images/dlib-landmark-mean.png
// http://dlib.net/dlib/image_processing/render_face_detections.h.html

// normalized face landmark positions from
// https://github.com/cmusatyalab/openface/blob/master/openface/align_dlib.py#L22
static const cv::Point2f dlib_markers[68] =
{
    { 0.000000f, 0.178569f },
    { 0.004128f, 0.312592f },
    { 0.019679f, 0.447709f },
    { 0.048099f, 0.580073f },
    { 0.100283f, 0.703495f },
    { 0.179998f, 0.812087f },
    { 0.276273f, 0.904678f },
    { 0.384637f, 0.980063f },
    { 0.507356f, 1.000000f },
    { 0.630141f, 0.976112f },
    { 0.738678f, 0.899214f },
    { 0.835475f, 0.805133f },
    { 0.914345f, 0.694562f },
    { 0.964350f, 0.568262f },
    { 0.988706f, 0.432444f },
    { 0.999312f, 0.295293f },
    { 1.000000f, 0.159097f },
    { 0.094855f, 0.076033f },
    { 0.155349f, 0.024925f },
    { 0.237747f, 0.011391f },
    { 0.323134f, 0.024158f },
    { 0.403670f, 0.057801f },
    { 0.568647f, 0.052116f },
    { 0.651282f, 0.015440f },
    { 0.737961f, 0.000000f },
    { 0.822909f, 0.011915f },
    { 0.887398f, 0.060257f },
    { 0.488933f, 0.155132f },
    { 0.489915f, 0.243430f },
    { 0.490921f, 0.331765f },
    { 0.492094f, 0.422107f },
    { 0.397399f, 0.480047f },
    { 0.444263f, 0.499068f },
    { 0.494951f, 0.514441f },
    { 0.545583f, 0.496829f },
    { 0.591751f, 0.477226f },
    { 0.194157f, 0.169267f },
    { 0.246003f, 0.136930f },
    { 0.310005f, 0.137356f },
    { 0.363785f, 0.177947f },
    { 0.306370f, 0.190823f },
    { 0.243905f, 0.191382f },
    { 0.618963f, 0.172778f },
    { 0.672494f, 0.129881f },
    { 0.736286f, 0.127908f },
    { 0.788859f, 0.158171f },
    { 0.741151f, 0.181558f },
    { 0.679137f, 0.183704f },
    { 0.307110f, 0.641850f },
    { 0.375970f, 0.610959f },
    { 0.446703f, 0.597051f },
    { 0.497216f, 0.608726f },
    { 0.550020f, 0.595433f },
    { 0.623302f, 0.607091f },
    { 0.695414f, 0.634143f },
    { 0.628068f, 0.709068f },
    { 0.557395f, 0.743447f },
    { 0.500204f, 0.750584f },
    { 0.445287f, 0.745803f },
    { 0.375082f, 0.714543f },
    { 0.337288f, 0.646165f },
    { 0.447015f, 0.640647f },
    { 0.497952f, 0.644963f },
    { 0.551394f, 0.638594f },
    { 0.665023f, 0.639559f },
    { 0.553056f, 0.676479f },
    { 0.498648f, 0.684176f },
    { 0.446572f, 0.678605f },
};

bool init_face_detect()
{
    try
    {
        dlib_detector = dlib::get_frontal_face_detector();
        dlib::deserialize("/usr/local/share/shape_predictor_68_face_landmarks.dat") >> dlib_predictor;
    }
    catch (const dlib::serialization_error& e)
    {
        printf("ERROR: Cannot initialize dlib\n");
        return false;
    }

    return true;
}

cv::Point2f get_face_marker(size_t index)
{
    return dlib_markers[index];
}

std::vector<cv::Rect> detect_faces(const cv::Mat& img)
{
    dlib::cv_image<dlib::bgr_pixel> image(img);
    std::vector<dlib::rectangle> faces = dlib_detector(image);

    std::vector<cv::Rect> result;
    result.reserve(faces.size());

    for (const auto& rect : faces)
    {
        result.push_back(cv::Rect(rect.left(), rect.top(), rect.width(), rect.height()));
    }

    return result;
}

std::vector<cv::Point> get_face_markers(const cv::Mat& img, const cv::Rect& bb)
{
    dlib::cv_image<dlib::bgr_pixel> image(img);
    dlib::rectangle rect(bb.x, bb.y, bb.x + bb.width, bb.y + bb.height);
    dlib::full_object_detection markers = dlib_predictor(image, rect);

    std::vector<cv::Point> result;
    result.reserve(markers.num_parts());
    
    for (long i=0; i<markers.num_parts(); i++)
    {
        dlib::point p = markers.part(i);
        result.push_back(cv::Point(p.x(), p.y()));
    }

    return result;
}

void draw_face_markers(cv::Mat& image, const cv::Rect& bb, const std::vector<cv::Point>& markers)
{   
    assert(markers.size() == 68);

    const cv::Scalar kRed(0, 0, 255);
    const cv::Scalar kGreen(0, 255, 0);

    cv::rectangle(image, bb.tl(), bb.br(), kRed);

    // Around Chin. Ear to Ear
    for (unsigned long i = 1; i <= 16; ++i)
    {
        cv::line(image, markers[i], markers[i-1], kGreen, 1, cv::LINE_AA);
    }

    // Line on top of nose
    for (unsigned long i = 28; i <= 30; ++i)
    {
        cv::line(image, markers[i], markers[i-1], kGreen, 1, cv::LINE_AA);
    }
    
    // left eyebrow
    for (unsigned long i = 18; i <= 21; ++i)
    {
        cv::line(image, markers[i], markers[i-1], kGreen, 1, cv::LINE_AA);
    }

    // Right eyebrow
    for (unsigned long i = 23; i <= 26; ++i)
    {
        cv::line(image, markers[i], markers[i-1], kGreen, 1, cv::LINE_AA);
    }

    // Bottom part of the nose
    for (unsigned long i = 31; i <= 35; ++i)
    {
        cv::line(image, markers[i], markers[i-1], kGreen, 1, cv::LINE_AA);
    }

    // Line from the nose to the bottom part above
    cv::line(image, markers[30], markers[35], kGreen, 1, cv::LINE_AA);

    // Left eye
    for (unsigned long i = 37; i <= 41; ++i)
    {
        cv::line(image, markers[i], markers[i-1], kGreen, 1, cv::LINE_AA);
    }
    cv::line(image, markers[36], markers[41], kGreen, 1, cv::LINE_AA);

    // Right eye
    for (unsigned long i = 43; i <= 47; ++i)
    {
        cv::line(image, markers[i], markers[i-1], kGreen, 1, cv::LINE_AA);
    }
    cv::line(image, markers[42], markers[47], kGreen, 1, cv::LINE_AA);

    // Lips outer part
    for (unsigned long i = 49; i <= 59; ++i)
    {
        cv::line(image, markers[i], markers[i-1], kGreen, 1, cv::LINE_AA);
    }
    cv::line(image, markers[48], markers[59], kGreen, 1, cv::LINE_AA);

    // Lips inside part
    for (unsigned long i = 61; i <= 67; ++i)
    {
        cv::line(image, markers[i], markers[i-1], kGreen, 1, cv::LINE_AA);
    }
    cv::line(image, markers[60], markers[67], kGreen, 1, cv::LINE_AA);
}
