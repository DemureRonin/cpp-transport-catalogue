#pragma once
#include <string_view>
#include <vector>
#include "geo.h"

namespace domain {
    struct Stop {
        std::string_view name;
        transport_catalogue::geo::Coordinates coordinates;
    };

    struct Bus {
        std::string_view name;
        std::vector<const Stop *> stops;
        bool is_roundtrip = false;
    };

    struct BusInfo {
        std::string_view name;
        size_t stop_count = 0;
        size_t unique_stop_count = 0;
        int route_length = 0;
        double curvature = 0.0;
    };
}
