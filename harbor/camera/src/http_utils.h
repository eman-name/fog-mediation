#pragma once

#include "common.h"

#include <vector>
#include <string>

struct NameQuality
{
    const char* name;
    float quality;
};

inline bool operator < (const NameQuality& a, const NameQuality& b)
{
    return a.quality < b.quality;

}

struct DetectedFace
{
    float left;
    float top;
    float width;
    float height;

    float markers[3][2];

    NameQuality names[3];
};

typedef std::vector<DetectedFace> DetectedFaces;

bool http_post_faces(const char* control, const DetectedFaces& faces);
bool http_post_image(const char* control, const std::vector<uint8_t>& image);
