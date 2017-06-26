#pragma once

#include "common.h"

#include <map>
#include <vector>
#include <string>

struct Database
{
	typedef std::vector<float> FaceId;
    typedef std::map<uint64_t, FaceId> PersonFaces;
    typedef std::map<std::string, PersonFaces> Persons;

    Persons persons;
    bool modified = false;

    Database();
    ~Database();

    Database(Database&) = delete;
    Database& operator = (const Database&) = delete;
};
