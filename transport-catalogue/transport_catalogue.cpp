#include "transport_catalogue.h"
#include <algorithm>
#include <stdexcept>
#include <deque>
#include <unordered_set>

namespace transport_catalogue {

void TransportCatalogue::AddStop(std::string_view name, const geo::Coordinates& coordinates) {
    auto& stored_name = stops_storage_.emplace_back(name);
    all_stops_[stored_name] = Stop{
        .name = stored_name,
        .coordinates = coordinates
    };
}

void TransportCatalogue::AddBus(std::string_view bus_name, const std::vector<std::string_view>& stops_names) {
    Bus bus;
    auto& stored_bus_name = buses_storage_.emplace_back(bus_name);
    bus.name = stored_bus_name;
    bus.stops.reserve(stops_names.size());

    for (auto stop_name : stops_names) {
        auto it = all_stops_.find(stop_name);
        if (it == all_stops_.end()) {
            throw std::runtime_error("Stop not found: " + std::string(stop_name));
        }
        auto stop_ptr = &it->second;
        stops_to_buses_[stop_ptr].insert(stored_bus_name);
        bus.stops.push_back(stop_ptr);
    }

    all_routes_[stored_bus_name] = std::move(bus);
}

std::optional<std::vector<std::string_view>>
TransportCatalogue::GetBusesByStop(std::string_view stop_name) const {
    auto it = all_stops_.find(stop_name);
    if (it == all_stops_.end()) {
        return std::nullopt;
    }

    const Stop* stop_ptr = &it->second;
    auto buses_it = stops_to_buses_.find(stop_ptr);
    if (buses_it == stops_to_buses_.end() || buses_it->second.empty()) {
        return std::vector<std::string_view>{};
    }

    std::vector<std::string_view> buses(buses_it->second.begin(), buses_it->second.end());
    std::sort(buses.begin(), buses.end());
    return buses;
}

std::optional<TransportCatalogue::BusInfo>
TransportCatalogue::GetBusInfo(std::string_view bus_name) const noexcept {
    auto it = all_routes_.find(bus_name);
    if (it == all_routes_.end()) {
        return std::nullopt;
    }

    const Bus& bus = it->second;

    BusInfo info;
    info.name = bus.name;
    info.stop_count = bus.stops.size();

    std::unordered_set<const Stop*> unique(bus.stops.begin(), bus.stops.end());
    info.unique_stop_count = unique.size();

    double route_length = 0;
    for (size_t i = 0; i + 1 < bus.stops.size(); i++) {
        route_length += ComputeDistance(
            bus.stops[i]->coordinates,
            bus.stops[i + 1]->coordinates
        );
    }
    info.route_length = route_length;

    return info;
}

} 
