#pragma once

#include <string>
#include <vector>
#include <variant>
#include <utility>
#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"
#include "transport_router.h"

namespace transport_catalogue::readers {
    struct StopCommand {
        std::string id;
        double latitude = 0.0;
        double longitude = 0.0;
        std::vector<std::pair<std::string, int> > distances;
    };

    struct BusCommand {
        std::string id;
        std::vector<std::string> stops;
        bool is_roundtrip = false;
    };

    using Command = std::variant<StopCommand, BusCommand>;

    class JsonReader {
    public:
        explicit JsonReader(TransportCatalogue &catalogue);

        void Load(std::istream &input);

        void ApplyCommands() const;

        [[nodiscard]] const std::vector<json::Node> &GetStatRequests() const;

        [[nodiscard]] const renderer::RenderSettings &GetMapSettings() const;

        [[nodiscard]] const transport_router::RoutingSettings &GetRouteSettings() const;

    private:
        TransportCatalogue &catalogue_;
        std::vector<Command> commands_;
        std::vector<json::Node> stat_requests_;
        renderer::RenderSettings map_settings_;
        transport_router::RoutingSettings route_settings_;

        void ParseBaseRequests(const json::Node &base_requests_node);

        void ParseRenderSettings(const json::Node &node);

        void ParseRoutingSettings(const json::Node &node);

        [[nodiscard]] static std::string NodeToColor(const json::Node &node);
    };
}