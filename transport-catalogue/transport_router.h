#pragma once

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

namespace transport_router {
    struct RoutingSettings {
        int bus_wait_time = 0;
        double bus_velocity = 0.0;
    };

    struct RouteItem {
        enum class Type { Wait, Bus } type;


        std::string stop_name;
        double time = 0.0;


        std::string bus;
        int span_count = 0;
    };

    struct Route {
        double total_time = 0.0;
        std::vector<RouteItem> items;
    };

    class TransportRouter {
    public:
        TransportRouter() = default;


        void SetRoutingSettings(RoutingSettings settings);


        void BuildGraph(const transport_catalogue::TransportCatalogue &tc);


        std::optional<Route> BuildRoute(std::string_view from, std::string_view to) const;


        const graph::DirectedWeightedGraph<double> &GetGraph() const { return graph_; }

    private:
        graph::VertexId GetWaitVertex(std::string_view stop_name) const;

        graph::VertexId GetBusVertex(std::string_view stop_name) const;

        RoutingSettings routing_settings_;

        graph::DirectedWeightedGraph<double> graph_;
        std::unique_ptr<graph::Router<double> > router_;

        std::unordered_map<std::string, graph::VertexId> stop_wait_vertex_;
        std::unordered_map<std::string, graph::VertexId> stop_bus_vertex_;

        std::vector<std::string> vertex_to_stop_name_;
    };
}
