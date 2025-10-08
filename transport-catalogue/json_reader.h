#pragma once

#include <string>
#include <vector>
#include <utility>
#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"

namespace transport_catalogue::readers {

    struct JsonCommand {
        std::string command;
        std::string id;
        std::vector<std::string> stops;
        std::vector<std::pair<std::string, int>> distances;
        double latitude = 0.0;
        double longitude = 0.0;
        bool is_roundtrip = false;
    };

    class JsonReader {
    public:
        explicit JsonReader(TransportCatalogue &catalogue);

        void Load(std::istream &input);
        void ApplyCommands() const;

        [[nodiscard]] const std::vector<json::Node> &GetStatRequests() const;
        [[nodiscard]] const renderer::RenderSettings &GetMapSettings() const;

    private:
        TransportCatalogue &catalogue_;
        std::vector<JsonCommand> commands_;
        std::vector<json::Node> stat_requests_;
        renderer::RenderSettings map_settings_;

        void ParseBaseRequests(const json::Node &base_requests_node);
        void ParseRenderSettings(const json::Node &node);
    };

}
