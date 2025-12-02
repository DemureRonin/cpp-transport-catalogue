#include "request_handler.h"
#include <sstream>

namespace transport_catalogue::readers {

    RequestHandler::RequestHandler(TransportCatalogue& catalogue)
        : catalogue_(catalogue)
        , reader_(catalogue) {
    }

    void RequestHandler::Load(std::istream& input) {
        reader_.Load(input);
    }

    void RequestHandler::ApplyCommands() const {
        reader_.ApplyCommands();
    }

    json::Array RequestHandler::ProcessRequests() const {
        const auto& settings = reader_.GetMapSettings();
        const renderer::MapRenderer renderer(catalogue_, settings);

        transport_router::TransportRouter router;
        router.SetRoutingSettings(reader_.GetRouteSettings());
        router.BuildGraph(catalogue_);

        json::Array result;
        for (const auto& req : reader_.GetStatRequests()) {
            const auto& m = req.AsDict();
            const std::string& type = m.at("type").AsString();
            int id = m.at("id").AsInt();

            if (type == "Map") {
                svg::Document doc = renderer.Render();
                std::ostringstream svg_out;
                doc.Render(svg_out);
                result.emplace_back(json::Dict{
                    {"request_id", id},
                    {"map", svg_out.str()}
                });
            } else if (type == "Stop") {
                const std::string& stop_name = m.at("name").AsString();
                auto buses_opt = catalogue_.GetBusesByStop(stop_name);
                if (!buses_opt) {
                    result.emplace_back(json::Dict{
                        {"request_id", id},
                        {"error_message", "not found"}
                    });
                } else {
                    json::Array arr;
                    for (const auto& bus : *buses_opt) {
                        arr.emplace_back(std::string(bus));
                    }
                    result.emplace_back(json::Dict{
                        {"request_id", id},
                        {"buses", arr}
                    });
                }
            } else if (type == "Bus") {
                const std::string& bus_name = m.at("name").AsString();
                auto info_opt = catalogue_.GetBusInfo(bus_name);
                if (!info_opt) {
                    result.emplace_back(json::Dict{
                        {"request_id", id},
                        {"error_message", "not found"}
                    });
                } else {
                    const auto& info = *info_opt;
                    result.emplace_back(json::Dict{
                        {"request_id", id},
                        {"route_length", info.route_length},
                        {"curvature", info.curvature},
                        {"stop_count", static_cast<int>(info.stop_count)},
                        {"unique_stop_count", static_cast<int>(info.unique_stop_count)}
                    });
                }
            } else if (type == "Route") {
                std::string from = m.at("from").AsString();
                std::string to = m.at("to").AsString();

                auto route_opt = router.BuildRoute(from, to);
                if (!route_opt) {
                    result.emplace_back(json::Dict{
                        {"request_id", id},
                        {"error_message", "not found"}
                    });
                } else {
                    const auto& route = *route_opt;
                    json::Array items_array;

                    for (const auto& item : route.items) {
                        std::visit([&](const auto& v) {
                            using T = std::decay_t<decltype(v)>;
                            if constexpr (std::is_same_v<T, transport_router::WaitItem>) {
                                items_array.emplace_back(json::Dict{
                                    {"type", "Wait"},
                                    {"stop_name", v.stop_name},
                                    {"time", v.time}
                                });
                            } else if constexpr (std::is_same_v<T, transport_router::BusItem>) {
                                items_array.emplace_back(json::Dict{
                                    {"type", "Bus"},
                                    {"bus", v.bus},
                                    {"span_count", v.span_count},
                                    {"time", v.time}
                                });
                            }
                        }, item);
                    }

                    result.emplace_back(json::Dict{
                        {"request_id", id},
                        {"total_time", route.total_time},
                        {"items", items_array}
                    });
                }
            }
        }

        return result;
    }
}
