#pragma once

#include "common.h"
#include <string>
#include <vector>

struct svm_model;
struct Database;

struct FaceIdentify
{
    typedef std::vector<double> Probabilities;

    svm_model* model;
    std::vector<std::string> names;

    Probabilities identify(const std::vector<float>& features) const;

    FaceIdentify(const Database& db);
    ~FaceIdentify();

    FaceIdentify(FaceIdentify&) = delete;
    FaceIdentify& operator = (const FaceIdentify&) = delete;
};

void init_face_identify();
