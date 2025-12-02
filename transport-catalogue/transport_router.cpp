#include "transport_router.h"
#include <stdexcept>

using namespace transport_router;
using namespace transport_catalogue;

void TransportRouter::SetRoutingSettings(RoutingSettings settings) {
    routing_settings_ = settings;
}

graph::VertexId TransportRouter::GetWaitVertex(std::string_view stop_name) const {
    auto it = stop_wait_vertex_.find(std::string(stop_name));
    if (it == stop_wait_vertex_.end()) {
        throw std::out_of_range("Stop not found: " + std::string(stop_name));
    }
    return it->second;
}

graph::VertexId TransportRouter::GetBusVertex(std::string_view stop_name) const {
    auto it = stop_bus_vertex_.find(std::string(stop_name));
    if (it == stop_bus_vertex_.end()) {
        throw std::out_of_range("Stop not found: " + std::string(stop_name));
    }
    return it->second;
}

void TransportRouter::BuildGraph(const TransportCatalogue& tc) {
    stop_wait_vertex_.clear();
    stop_bus_vertex_.clear();
    vertex_to_stop_name_.clear();
    graph_ = graph::DirectedWeightedGraph<double>();

    const auto stops = tc.GetAllStops();
    graph_.Resize(stops.size() * 2);

    vertex_to_stop_name_.resize(stops.size() * 2);
    for (size_t i = 0; i < stops.size(); ++i) {
        const auto* stop = stops[i];
        std::string name = std::string(stop->name);

        graph::VertexId wait_v = 2 * i;
        graph::VertexId bus_v = 2 * i + 1;

        stop_wait_vertex_[name] = wait_v;
        stop_bus_vertex_[name] = bus_v;

        vertex_to_stop_name_[wait_v] = name;
        vertex_to_stop_name_[bus_v] = name;

        graph::Edge<double> wait_edge;
        wait_edge.from = wait_v;
        wait_edge.to = bus_v;
        wait_edge.weight = routing_settings_.bus_wait_time;
        graph_.AddEdge(wait_edge);
    }

    const auto buses = tc.GetAllBusNames();
    for (const auto& name : buses) {
        const auto* bus = tc.FindBus(name);
        if (!bus) continue;

        std::vector<const domain::Stop*> seq = bus->stops;
        if (!bus->is_roundtrip) {
            for (int i = seq.size() - 2; i >= 0; --i) {
                seq.push_back(seq[i]);
            }
        }

        for (size_t i = 0; i < seq.size(); ++i) {
            double dist = 0;
            for (size_t j = i + 1; j < seq.size(); ++j) {
                dist += tc.GetDistance(seq[j - 1], seq[j]);
                double t = (dist / 1000.0) / routing_settings_.bus_velocity * 60.0;

                graph::Edge<double> e;
                e.from = GetBusVertex(seq[i]->name);
                e.to = GetWaitVertex(seq[j]->name);
                e.weight = t;
                e.bus_name = name;
                e.span_count = j - i;
                graph_.AddEdge(e);
            }
        }
    }

    router_ = std::make_unique<graph::Router<double>>(graph_);
}

std::optional<Route> TransportRouter::BuildRoute(std::string_view from, std::string_view to) const {
    if (!router_) return std::nullopt;

    auto it_from = stop_wait_vertex_.find(std::string(from));
    auto it_to = stop_wait_vertex_.find(std::string(to));
    if (it_from == stop_wait_vertex_.end() || it_to == stop_wait_vertex_.end()) {
        return std::nullopt;
    }

    auto info = router_->BuildRoute(it_from->second, it_to->second);
    if (!info) return std::nullopt;

    Route route;
    route.total_time = info->weight;

    for (auto id : info->edges) {
        const auto& e = graph_.GetEdge(id);
        if (e.bus_name.empty()) {
            route.items.emplace_back(WaitItem{vertex_to_stop_name_[e.from], e.weight});
        } else {
            route.items.emplace_back(BusItem{e.bus_name, e.span_count, e.weight});
        }
    }

    std::vector<RouteItem> merged;
    for (const auto& item : route.items) {
        if (!merged.empty()) {
            if (auto* last_bus = std::get_if<BusItem>(&merged.back())) {
                if (auto* cur_bus = std::get_if<BusItem>(&item)) {
                    if (last_bus->bus == cur_bus->bus) {
                        last_bus->span_count += cur_bus->span_count;
                        last_bus->time += cur_bus->time;
                        continue;
                    }
                }
            }
        }
        merged.push_back(item);
    }

    route.items = std::move(merged);
    return route;
}
