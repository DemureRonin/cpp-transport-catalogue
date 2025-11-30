#include "transport_router.h"

#include <cassert>
#include <cmath>
#include <limits>
#include <stdexcept>

using namespace transport_router;
using namespace transport_catalogue;


void TransportRouter::SetRoutingSettings(RoutingSettings settings) {
    routing_settings_ = settings;
}


graph::VertexId TransportRouter::GetWaitVertex(std::string_view stop_name) const {
    auto it = stop_wait_vertex_.find(std::string(stop_name));
    if (it == stop_wait_vertex_.end()) {
        throw std::out_of_range("Stop not found in router: " + std::string(stop_name));
    }
    return it->second;
}


graph::VertexId TransportRouter::GetBusVertex(std::string_view stop_name) const {
    auto it = stop_bus_vertex_.find(std::string(stop_name));
    if (it == stop_bus_vertex_.end()) {
        throw std::out_of_range("Stop not found in router: " + std::string(stop_name));
    }
    return it->second;
}

void TransportRouter::BuildGraph(const TransportCatalogue &tc) {

    stop_wait_vertex_.clear();
    stop_bus_vertex_.clear();
    vertex_to_stop_name_.clear();
    graph_ = graph::DirectedWeightedGraph<double>();


    const auto all_stops = tc.GetAllStops();

    graph_.Resize(all_stops.size() * 2);



    vertex_to_stop_name_.resize(all_stops.size() * 2);
    for (size_t i = 0; i < all_stops.size(); ++i) {
        const domain::Stop *stop = all_stops[i];
        std::string name = std::string(stop->name);

        graph::VertexId wait_v = 2 * i;
        graph::VertexId bus_v = 2 * i + 1;

        stop_wait_vertex_.emplace(name, wait_v);
        stop_bus_vertex_.emplace(name, bus_v);

        vertex_to_stop_name_[wait_v] = name;
        vertex_to_stop_name_[bus_v] = name;


        const double wait_time = routing_settings_.bus_wait_time;
        graph::Edge<double> wait_edge;
        wait_edge.from = wait_v;
        wait_edge.to = bus_v;
        wait_edge.weight = wait_time;

        graph_.AddEdge(wait_edge);
    }

    const auto all_buses = tc.GetAllBusNames();
    for (const auto &bus_name : all_buses) {
        const domain::Bus *b = tc.FindBus(bus_name);
        if (!b) continue;

        std::vector<const domain::Stop *> stops_seq = b->stops;
        if (!b->is_roundtrip) {
            if (!stops_seq.empty()) {
                for (int i = static_cast<int>(stops_seq.size()) - 2; i >= 0; --i) {
                    stops_seq.push_back(stops_seq[i]);
                }
            }
        }

        const size_t n = stops_seq.size();
        for (size_t i = 0; i < n; ++i) {
            double distance_m = 0.0;
            for (size_t j = i + 1; j < n; ++j) {

                const domain::Stop *from = stops_seq[j - 1];
                const domain::Stop *to = stops_seq[j];
                distance_m += static_cast<double>(tc.GetDistance(from, to));


                const double distance_km = distance_m / 1000.0;
                const double time_hours = distance_km / routing_settings_.bus_velocity;
                const double time_minutes = time_hours * 60.0;

                graph::Edge<double> bus_edge;
                bus_edge.from = GetBusVertex(stops_seq[i]->name);
                bus_edge.to = GetWaitVertex(stops_seq[j]->name);
                bus_edge.weight = time_minutes;
                bus_edge.bus_name = std::string(bus_name);
                bus_edge.span_count = static_cast<int>(j - i);
                graph_.AddEdge(bus_edge);
            }
        }
    }


    router_ = std::make_unique<graph::Router<double>>(graph_);
}


std::optional<Route> TransportRouter::BuildRoute(std::string_view from, std::string_view to) const {

    if (!router_) {
        return std::nullopt;
    }

    auto it_from = stop_wait_vertex_.find(std::string(from));
    auto it_to = stop_wait_vertex_.find(std::string(to));
    if (it_from == stop_wait_vertex_.end() || it_to == stop_wait_vertex_.end()) {
        return std::nullopt;
    }

    const graph::VertexId v_from = it_from->second;
    const graph::VertexId v_to = it_to->second;

    auto route_info_opt = router_->BuildRoute(v_from, v_to);
    if (!route_info_opt) {
        return std::nullopt;
    }
    const auto &route_info = *route_info_opt;

    Route result;
    result.total_time = route_info.weight;


    const auto &edges = route_info.edges;
    result.items.reserve(edges.size());
    for (size_t idx = 0; idx < edges.size(); ++idx) {
        const auto &edge = graph_.GetEdge(edges[idx]);
        if (edge.bus_name.empty()) {

            const std::string stop_name = vertex_to_stop_name_[edge.from];
            RouteItem item;
            item.type = RouteItem::Type::Wait;
            item.stop_name = stop_name;
            item.time = edge.weight;
            result.items.push_back(std::move(item));
        } else {

            RouteItem item;
            item.type = RouteItem::Type::Bus;
            item.bus = edge.bus_name;
            item.span_count = edge.span_count;
            item.time = edge.weight;
            result.items.push_back(std::move(item));
        }
    }

    std::vector<RouteItem> grouped;
    grouped.reserve(result.items.size());
    for (const auto &it : result.items) {
        if (!grouped.empty()) {
            RouteItem &last = grouped.back();
            if (last.type == RouteItem::Type::Bus && it.type == RouteItem::Type::Bus && last.bus == it.bus) {
                last.span_count += it.span_count;
                last.time += it.time;
                continue;
            }
        }
        grouped.push_back(it);
    }
    result.items = std::move(grouped);

    return result;
}
