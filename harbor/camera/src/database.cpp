#include "database.h"

#include <dlib/serialize.h>

static const char* kDatabase = "faces.db";

Database::Database()
{
    printf("Loading face database\n");

    try
    {
        dlib::deserialize(kDatabase) >> persons;
    }
    catch (const dlib::serialization_error&)
    {
        printf("ERROR: failed to load face database\n");
    }

    printf("Loaded %ju person(s)\n", persons.size());
}

Database::~Database()
{
    if (!modified)
    {
        return;
    }

    printf("Saving face database\n");

    try
    {
        dlib::serialize(kDatabase) << persons;
    }
    catch (const dlib::serialization_error&)
    {
        printf("ERROR: failed to save face database\n");
    }
}
