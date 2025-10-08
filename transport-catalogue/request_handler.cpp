#include "request_handler.h"
#include <sstream>

namespace transport_catalogue::readers {

RequestHandler::RequestHandler(TransportCatalogue &catalogue)
    : catalogue_(catalogue), reader_(catalogue) {}

void RequestHandler::Load(std::istream &input) {
    reader_.Load(input);
}

void RequestHandler::ApplyCommands() const {
    reader_.ApplyCommands();
}

json::Array RequestHandler::ProcessRequests() const {
    const auto &settings = reader_.GetMapSettings();
    const renderer::MapRenderer renderer(catalogue_, settings);

    json::Array result;
    for (const auto &req : reader_.GetStatRequests()) {
        const auto &m = req.AsMap();
        const std::string &type = m.at("type").AsString();
        int id = m.at("id").AsInt();

        if (type == "Map") {
            svg::Document doc = renderer.Render();
            std::ostringstream svg_out;
            doc.Render(svg_out);
            result.emplace_back(json::Dict{
                {"request_id", json::Node(id)},
                {"map", json::Node(svg_out.str())}
            });
        } else if (type == "Stop") {
            const std::string &stop_name = m.at("name").AsString();
            auto buses_opt = catalogue_.GetBusesByStop(stop_name);
            if (!buses_opt.has_value()) {
                result.emplace_back(json::Dict{
                    {"request_id", json::Node(id)},
                    {"error_message", json::Node("not found")}
                });
            } else {
                json::Array bus_array;
                for (const auto &bus : *buses_opt) {
                    bus_array.emplace_back(json::Node(std::string(bus)));
                }
                result.emplace_back(json::Dict{
                    {"request_id", json::Node(id)},
                    {"buses", json::Node(bus_array)}
                });
            }
        } else if (type == "Bus") {
            const std::string &bus_name = m.at("name").AsString();
            auto info_opt = catalogue_.GetBusInfo(bus_name);
            if (!info_opt.has_value()) {
                result.emplace_back(json::Dict{
                    {"request_id", json::Node(id)},
                    {"error_message", json::Node("not found")}
                });
            } else {
                const auto &info = *info_opt;
                result.emplace_back(json::Dict{
                    {"request_id", json::Node(id)},
                    {"route_length", json::Node(info.route_length)},
                    {"curvature", json::Node(info.curvature)},
                    {"stop_count", json::Node(static_cast<int>(info.stop_count))},
                    {"unique_stop_count", json::Node(static_cast<int>(info.unique_stop_count))}
                });
            }
        }
    }

    return result;
}

}
