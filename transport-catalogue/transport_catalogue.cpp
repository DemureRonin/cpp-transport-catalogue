#include "transport_catalogue.h"
#include <algorithm>
#include <stdexcept>
#include <unordered_set>

namespace transport_catalogue {
    void TransportCatalogue::AddStop(std::string_view name, const geo::Coordinates &coordinates) {
        auto &stored_name = stops_storage_.emplace_back(name);
        all_stops_[stored_name] = domain::Stop{
            .name = stored_name,
            .coordinates = coordinates
        };
    }

    void TransportCatalogue::AddBus(std::string_view bus_name, const std::vector<std::string_view> &stops_names, bool is_roundtrip) {
        auto &stored_bus_name = buses_storage_.emplace_back(bus_name);

        domain::Bus bus;
        bus.name = stored_bus_name;
        bus.is_roundtrip = is_roundtrip;
        std::vector<const domain::Stop *> stops_ptrs;
        stops_ptrs.reserve(stops_names.size());

        for (auto stop_name: stops_names) {
            std::string stop_name_copy(stop_name);
            const domain::Stop *stop_ptr = FindStop(stop_name_copy);
            if (!stop_ptr) {
                throw std::runtime_error("Stop not found: " + stop_name_copy);
            }
            stops_to_buses_[stop_ptr].insert(stored_bus_name);
            stops_ptrs.push_back(stop_ptr);
        }

        if (!is_roundtrip && stops_ptrs.size() > 1) {
            std::vector<const domain::Stop *> back_path(stops_ptrs.begin(), stops_ptrs.end() - 1);
            std::reverse(back_path.begin(), back_path.end());
            stops_ptrs.insert(stops_ptrs.end(), back_path.begin(), back_path.end());
        }


        bus.stops = std::move(stops_ptrs);
        all_routes_[stored_bus_name] = std::move(bus);
    }


    std::vector<std::string_view> TransportCatalogue::GetAllBusNames() const {
        std::vector<std::string_view> names;
        names.reserve(all_routes_.size());
        for (const auto &[name, _]: all_routes_) {
            names.push_back(name);
        }
        std::sort(names.begin(), names.end());
        return names;
    }


    std::vector<const domain::Stop *> TransportCatalogue::GetAllStops() const {
        std::vector<const domain::Stop *> stops;
        stops.reserve(all_stops_.size());
        for (const auto &[name, stop]: all_stops_) {
            stops.push_back(&stop);
        }
        return stops;
    }

    void TransportCatalogue::SetDistance(const std::string &from, const std::string &to, int distance) {
        const domain::Stop *s_from = FindStop(from);
        const domain::Stop *s_to = FindStop(to);
        if (!s_from || !s_to) {
            throw std::invalid_argument("Unknown stop name in SetDistance");
        }
        distances_[{s_from, s_to}] = distance;
    }


    int TransportCatalogue::GetDistance(const domain::Stop *from, const domain::Stop *to) const {
        if (auto it = distances_.find({from, to}); it != distances_.end()) {
            return it->second;
        }
        if (auto it = distances_.find({to, from}); it != distances_.end()) {
            return it->second;
        }

        return static_cast<int>(geo::ComputeDistance(from->coordinates, to->coordinates));
    }


    const domain::Stop *TransportCatalogue::FindStop(std::string_view name) const noexcept {
        if (auto it = all_stops_.find(name); it != all_stops_.end()) {
            return &it->second;
        }
        return nullptr;
    }


    const domain::Bus *TransportCatalogue::FindBus(std::string_view name) const noexcept {
        if (auto it = all_routes_.find(name); it != all_routes_.end()) {
            return &it->second;
        }
        return nullptr;
    }

    std::optional<std::vector<std::string_view> >
    TransportCatalogue::GetBusesByStop(std::string_view stop_name) const {
        const domain::Stop *stop_ptr = FindStop(stop_name);
        if (!stop_ptr) {
            return std::nullopt;
        }

        auto buses_it = stops_to_buses_.find(stop_ptr);
        if (buses_it == stops_to_buses_.end() || buses_it->second.empty()) {
            return std::vector<std::string_view>{};
        }

        std::vector<std::string_view> buses(buses_it->second.begin(), buses_it->second.end());
        std::sort(buses.begin(), buses.end());
        return buses;
    }

    std::optional<domain::BusInfo> TransportCatalogue::GetBusInfo(std::string_view bus_name) const noexcept {
        const domain::Bus *bus = FindBus(bus_name);
        if (!bus) {
            return std::nullopt;
        }

        domain::BusInfo info;
        info.name = bus->name;
        info.stop_count = bus->stops.size();

        std::unordered_set<const domain::Stop *> unique(bus->stops.begin(), bus->stops.end());
        info.unique_stop_count = unique.size();

        double geo_length = 0;
        int road_length = 0;

        for (size_t i = 0; i + 1 < bus->stops.size(); ++i) {
            geo_length += geo::ComputeDistance(bus->stops[i]->coordinates,
                                               bus->stops[i + 1]->coordinates);
            road_length += GetDistance(bus->stops[i], bus->stops[i + 1]);
        }

        info.route_length = road_length;
        info.curvature = geo_length > 0 ? road_length / geo_length : 0.0;

        return info;
    }
}
