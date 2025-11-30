#include "request_handler.h"
#include <sstream>

namespace transport_catalogue::readers {
    RequestHandler::RequestHandler(TransportCatalogue &catalogue)
        : catalogue_(catalogue), reader_(catalogue) {
    }

    void RequestHandler::Load(std::istream &input) {
        reader_.Load(input);
    }

    void RequestHandler::ApplyCommands() const {
        reader_.ApplyCommands();
    }

    json::Array RequestHandler::ProcessRequests() const {
        const auto &settings = reader_.GetMapSettings();
        const renderer::MapRenderer renderer(catalogue_, settings);

        transport_router::TransportRouter router;
        router.SetRoutingSettings(reader_.GetRouteSettings());
        router.BuildGraph(catalogue_);


        json::Array result;
        for (const auto &req: reader_.GetStatRequests()) {
            const auto &m = req.AsDict();
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
                    for (const auto &bus: *buses_opt) {
                        bus_array.emplace_back(std::string(bus));
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
            } else if (type == "Route") {
                std::string from = m.at("from").AsString();
                std::string to = m.at("to").AsString();

                auto route_opt = router.BuildRoute(from, to);
                if (!route_opt) {
                    result.emplace_back(json::Dict{
                        {"request_id", json::Node(id)},
                        {"error_message", json::Node("not found")}
                    });
                } else {
                    const auto &route = *route_opt;
                    json::Array items_array;
                    for (const auto &item: route.items) {
                        if (item.type == transport_router::RouteItem::Type::Wait) {
                            items_array.emplace_back(json::Dict{
                                {"type", json::Node("Wait")},
                                {"stop_name", json::Node(item.stop_name)},
                                {"time", json::Node(item.time)}
                            });
                        } else {
                            items_array.emplace_back(json::Dict{
                                {"type", json::Node("Bus")},
                                {"bus", json::Node(item.bus)},
                                {"span_count", json::Node(item.span_count)},
                                {"time", json::Node(item.time)}
                            });
                        }
                    }

                    result.emplace_back(json::Dict{
                        {"request_id", json::Node(id)},
                        {"total_time", json::Node(route.total_time)},
                        {"items", json::Node(items_array)}
                    });
                }
            }
        }

        return result;
    }
}
