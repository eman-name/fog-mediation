#include "face_identify.h"
#include "database.h"

#include <svm.h>

static void svm_null_print(const char*)
{
}

void init_face_identify()
{  
    svm_set_print_string_function(&svm_null_print);
}

FaceIdentify::FaceIdentify(const Database& db)
{
    printf("Training face identification model\n");

    size_t count = 0;
    for (const auto& person : db.persons)
    {
        count += person.second.size();
    }

    if (count == 0)
    {
        model = NULL;
        return;
    }

    names.clear();
    names.reserve(db.persons.size());

    std::vector<double> labels;
    labels.reserve(count);

    std::vector<svm_node*> ptr;
    ptr.reserve(count);

    svm_node* nodes = (svm_node*)malloc(sizeof(*nodes) * count * (kFeatureCount + 1));
    svm_node* node = nodes;

    for (const auto& person : db.persons)
    {
        double label = double(names.size());
        names.push_back(person.first);
        for (const auto& face : person.second)
        {
            labels.push_back(label);
            ptr.push_back(node);
            const auto& features = face.second;
            for (size_t i=0; i<kFeatureCount; i++)
            {
                node->index = int(i);
                node->value = features[i];
                node++;
            }
            node->index = -1;
            node++;
        }
    }
    
    svm_problem prob =
    {
        int(count),
        labels.data(),
        ptr.data(),
    };

    svm_parameter param = {};
    param.svm_type = C_SVC;
    param.kernel_type = LINEAR;
    param.degree = 3.0;
    param.cache_size = 200.0;
    param.eps = 0.001;
    param.C = 1.0;
    param.nu = 0.5;
    param.p = 0.1;
    param.probability = 1;
    param.gamma = 1.0 / count;

    model = svm_train(&prob, &param);
}

FaceIdentify::~FaceIdentify()
{
    if (model)
    {
        svm_free_and_destroy_model(&model);
    }
}

FaceIdentify::Probabilities FaceIdentify::identify(const std::vector<float>& features) const
{
    if (model == NULL)
    {
        return Probabilities();
    }

    Probabilities result(names.size());

    svm_node nodes[kFeatureCount];
    for (size_t i=0; i<kFeatureCount; i++)
    {
        nodes[i].index = int(i);
        nodes[i].value = double(features[i]);
    }
    svm_predict_probability(model, nodes, result.data());
    return result;
}
