#pragma once

#include <vector>
#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"
#include "json_reader.h"

namespace transport_catalogue::readers {
    class RequestHandler {
    public:
        explicit RequestHandler(TransportCatalogue &catalogue);

        void Load(std::istream &input);

        void ApplyCommands() const;

        [[nodiscard]] json::Array ProcessRequests() const;

    private:
        TransportCatalogue &catalogue_;
        JsonReader reader_;
    };
}
