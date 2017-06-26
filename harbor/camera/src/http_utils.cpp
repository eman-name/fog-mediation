#include "http_utils.h"

#include "control.grpc.pb.h"
#include <grpc++/grpc++.h>

bool http_post_faces(const char* control, const DetectedFaces& faces)
{
    auto channel = grpc::CreateChannel(control, grpc::InsecureChannelCredentials());
    auto stub = bitclave::ControlServer::NewStub(channel);

    bitclave::Data data;

    for (const auto& face : faces)
    {
        bitclave::Face* f = data.add_faces();

        bitclave::Box* b = f->mutable_box();
        b->set_left(face.left);
        b->set_top(face.top);
        b->set_width(face.width);
        b->set_height(face.height);

        bitclave::Markers* m = f->mutable_markers();
        for (const auto& marker : face.markers)
        {
            bitclave::Point* p = m->add_points();
            p->set_x(marker[0]);
            p->set_y(marker[1]);
        }

        for (const auto& name : face.names)
        {
            if (name.name)
            {
                bitclave::Name* n = f->add_names();
                n->set_name(name.name);
                n->set_quality(name.quality);
            }
        }
    }

    bitclave::Empty empty;

    grpc::ClientContext context;
    grpc::Status status = stub->PostData(&context, data, &empty);

    return status.ok();
}

bool http_post_image(const char* control, const std::vector<uint8_t>& image)
{
    auto channel = grpc::CreateChannel(control, grpc::InsecureChannelCredentials());
    auto stub = bitclave::ControlServer::NewStub(channel);

    bitclave::Frame frame;
    frame.set_image(image.data(), image.size());

    bitclave::Empty empty;

    grpc::ClientContext context;
    grpc::Status status = stub->PostFrame(&context, frame, &empty);

    return status.ok();
}
