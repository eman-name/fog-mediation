#include "common.h"
#include "database.h"
#include "image_hash.h"
#include "face_detect.h"
#include "face_features.h"
#include "face_identify.h"
#include "http_utils.h"

#include <assert.h>
#include <string.h>

#include <signal.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <algorithm>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

static volatile sig_atomic_t stop = 0;

static void stop_signal(int num)
{
    stop = 1;
}

static cv::Mat get_aligned_face(const cv::Mat& image, const std::vector<cv::Point>& markers)
{
    cv::Point2f src[] =
    {
        markers[kDlibOuterEyeLeft],
        markers[kDlibOuterEyeRight],
        markers[kDlibOuterNose],
    };

    cv::Point2f dst[] =
    {
        kAlignedImageSize * get_face_marker(kDlibOuterEyeLeft),
        kAlignedImageSize * get_face_marker(kDlibOuterEyeRight),
        kAlignedImageSize * get_face_marker(kDlibOuterNose),
    };

    cv::Mat transform = cv::getAffineTransform(src, dst);
    
    cv::Mat aligned;
    cv::warpAffine(image, aligned, transform, cv::Size(kAlignedImageSize, kAlignedImageSize));
    return aligned;
}

static void stream(const char* src, const char* dst)
{
    FaceIdentify id{Database()};

    cv::VideoCapture cap;

    while (!stop)
    {
        if (!cap.isOpened())
        {
            printf("Connecting to camera...\n");
            if (!cap.open(src))
            {
                printf("ERROR: Cannot connect to camera!\n");
                usleep(1000 * 1000);
                continue;
            }

            printf("Connected to camera.\n");
        }

        printf("Reading a new frame from camera\n");

        cv::Mat image;
        if (!cap.read(image))
        {
            printf("ERROR: Cannot get frame from camera!\n");
            usleep(1000 * 1000);
            continue;
        }

        printf("Got a new frame!\n");

        std::vector<cv::Rect> faces = detect_faces(image);

        if (faces.empty())
        {
            printf("No face found in image\n");
            http_post_faces(dst, DetectedFaces());
        }
        else
        {
            printf("Found %zu face(s) in image\n", faces.size());

            DetectedFaces detected;
            detected.reserve(faces.size());

            size_t idx = 0;
            for (const auto& bb : faces)
            {
                // TODO: this can be used to save face cut-out from image to some database
                // cv::Mat cut(image, bb);
                // cv::imwrite("output_cut.jpg", cut);

                std::vector<cv::Point> markers = get_face_markers(image, bb);
                cv::Mat aligned = get_aligned_face(image, markers);

                std::vector<float> features = get_face_features(aligned);
                FaceIdentify::Probabilities values = id.identify(features);

                DetectedFace item = {};
                item.left = float(bb.x) / image.cols;
                item.top = float(bb.y) / image.rows;
                item.width = float(bb.width) / image.cols;
                item.height = float(bb.height) / image.rows;

                size_t mindex[] = {kDlibOuterEyeLeft, kDlibOuterEyeRight, kDlibOuterNose};
                for (size_t i=0; i<3; i++)
                {
                    cv::Point pt = markers[mindex[i]];
                    item.markers[i][0] = float(pt.x) / image.cols;
                    item.markers[i][1] = float(pt.y) / image.rows;
                }

                NameQuality nq[4];

                size_t count = 0;
                for (size_t i=0; i<values.size(); i++)
                {
                    nq[count].name = id.names[i].c_str();
                    nq[count].quality = values[i];

                    std::push_heap(nq, nq + count + 1);
                    count = std::min<size_t>(count+1, 3);
                }
                std::sort_heap(nq, nq + count);
                std::reverse_copy(nq, nq + count, item.names);
                
                printf(" ** detected %s with %.2f probability **\n", item.names[0].name, item.names[0].quality);
                detected.push_back(item);
            }

            http_post_faces(dst, detected);
        }

        std::vector<int> params { cv::IMWRITE_JPEG_QUALITY, 75 };
        std::vector<uint8_t> buffer;
        cv::imencode(".jpg", image, buffer, params);

        http_post_image(dst, buffer);
    }
}

static void load_image(Database& db, const char* name, const cv::Mat& image)
{
    std::vector<cv::Rect> faces = detect_faces(image);
    if (faces.empty())
    {
        printf("No face found in image\n");
        return;
    }

    printf("Face found in image\n");

    size_t largest = 0;
    int area = -1;
    for (size_t i=0; i<faces.size(); i++)
    {
        int tmp = faces[i].area();
        if (tmp > area)
        {
            largest = i;
            area = tmp;
        }
    }

    std::vector<cv::Point> markers = get_face_markers(image, faces[largest]);
    cv::Mat aligned = get_aligned_face(image, markers);

    uint64_t hash = image_hash(aligned);

    auto it = db.persons.find(name);
    if (it == db.persons.end())
    {
        it = db.persons.insert(std::make_pair(name, Database::PersonFaces())).first;
    }

    auto& person = it->second;

    if (person.find(hash) == person.end())
    {
        printf("Different face detected, adding to db\n");

        person.insert(std::make_pair(hash, get_face_features(aligned)));
        db.modified = true;
    }
    else
    {
        printf("Face is already present in db\n");
    }
}

static void load_folder(const char* name, const char* folder)
{
    Database db;

    DIR* dir = opendir(folder);
    if (!dir)
    {
        printf("ERROR: cannot open folder '%s'\n", folder);
        return;
    }

    struct dirent* d;
    while ((d = readdir(dir)) != NULL && !stop)
    {
        char filename[4096];
        snprintf(filename, sizeof(filename), "%s/%s", folder, d->d_name);

        if (d->d_type == DT_REG)
        {
            cv::Mat image = cv::imread(filename); 
            if (image.data)
            {
                printf("Loaded image from '%s'\n", filename);
                load_image(db, name, image);
            }
            else
            {
                printf("ERROR: Cannot load image from '%s'\n", filename);
            }
        }
    }
    closedir(dir);
}

static const char* get_camera_device()
{
    const char* camera = getenv("CAMERA");
    return camera ? camera : "/dev/video0";
}

static void load_camera(const char* name)
{
    Database db;

    const char* camera = get_camera_device();

    printf("Opening camera...\n");
    cv::VideoCapture cap(camera);
    if (!cap.isOpened())
    {
        printf("ERROR: failed to open '%s' device\n", camera);
        return;
    }

    printf("Capturing frames from camera, press Ctrl+C to stop...\n");
    while (!stop)
    {
        cv::Mat frame;
        if (cap.read(frame))
        {
            printf("Got a new frame from camera\n");

            cv::Mat image;
            cv::flip(frame, image, 1);

            load_image(db, name, image);
        }
        usleep(1000 * 1000);
    }
}

static void test(const char* filename)
{
    FaceIdentify id{Database()};

    cv::Mat image;
    if (filename)
    {
        image = cv::imread(filename); 
        if (!image.data)
        {
            printf("ERROR: failed to read '%s' image\n", filename);
            return;
        }
    }
    else
    {
        const char* camera = get_camera_device();

        printf("Opening camera...\n");
        cv::VideoCapture cap(camera);
        if (!cap.isOpened())
        {
            printf("ERROR: failed to open '%s' device\n", camera);
            return;
        }

        printf("Capturing frames from camera...\n");

        cv::Mat frame;
        if (!cap.read(frame))
        {
            printf("ERROR: cannot read image from camera\n");
            return;
        }
        printf("Got a new frame from camera\n");

        cv::flip(frame, image, 1);
        cv::imwrite("output_camera.jpg", image);
    }

    std::vector<cv::Rect> faces = detect_faces(image);
    if (faces.empty())
    {
        printf("No face found in image\n");
        return;
    }

    printf("Found %zu face(s) in image\n", faces.size());

    cv::Mat output;
    image.copyTo(output);

    size_t idx = 0;
    for (const auto& bb : faces)
    {
        std::vector<cv::Point> markers = get_face_markers(image, bb);
        cv::Mat aligned = get_aligned_face(image, markers);

        char fname[64];
        snprintf(fname, sizeof(fname), "output_aligned_%zu.jpg", idx++);

        cv::imwrite(fname, aligned);

        draw_face_markers(output, bb, markers);

        if (id.model)
        {
            std::vector<float> features = get_face_features(aligned);
            FaceIdentify::Probabilities values = id.identify(features);

            // for (int i=0; i<values.size(); i++)
            // {
            //     printf("%s %f\n", id.names[i].c_str(), values[i]);
            // }

            size_t max = std::max_element(values.begin(), values.end()) - values.begin();
            std::string name = id.names[max];
            double value = values[max];

            char text[128];
            snprintf(text, sizeof(text), "%s (%.2f)", name.c_str(), value);

            cv::putText(output, text, bb.tl() - cv::Point(0,3),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1, cv::LINE_AA);
            cv::putText(output, text, bb.tl() - cv::Point(1,4),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);

            printf(" ** detected %s with %.2f probability **\n", name.c_str(), value);
        }
    }
    cv::imwrite("output.jpg", output);
}

static bool init()
{
    if (!init_face_detect())
    {
        return false;
    }

    if (!init_face_features())
    {
        return false;
    }

    init_face_identify();

    return true;
}

static void help()
{
    printf("Usage: <command> [arguments] ...\n");
    printf("\n");
    printf("Where <command> is one of following:\n");
    printf("  stream <camera-url> <post-url> - starts analyzing viode from camera URL and posts JSON results to second URL\n");
    printf("  load <name> <folder>           - loads into database <name> person from images in <folder>\n");
    printf("  load <name>                    - loads into database <name> person captured from connected camera\n");
    printf("  test <file>                    - analyzes image and displays result\n");
    printf("  test                           - analyzes image captured from connected camera\n");
    exit(1);
}

int main(int argc, char* argv[])
{
    // for (int i=1; i<argc; i++)
    // {
    //     printf("%d='%s'\n", i, argv[i]);
    // }

    if (argc < 2)
    {
        help();
    }

    if (!init())
    {
        return 1;
    }

    signal(SIGINT, stop_signal); // Ctrl+C
    signal(SIGTERM, stop_signal); // docker stop

    if (strcmp(argv[1], "stream") == 0)
    {
        if (argc != 4)
        {
            help();
        }
        stream(argv[2], argv[3]);
    }
    else if (strcmp(argv[1], "load") == 0)
    {
        if (argc == 3)
        {
            load_camera(argv[2]);
        }
        else if (argc == 4)
        {
            load_folder(argv[2], argv[3]);
        }
        else
        {
            help();
        }
    }
    else if (strcmp(argv[1], "test") == 0)
    {
        if (argc != 2 && argc != 3)
        {
            help();
        }
        test(argc == 3 ? argv[2] : NULL);
    }
    else
    {
        help();
    }
}
