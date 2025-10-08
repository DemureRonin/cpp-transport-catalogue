#pragma once

#include <deque>
#include <optional>

#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geo.h"
#include "domain.h"

namespace transport_catalogue {
    class TransportCatalogue {
    public:
        void AddStop(std::string_view name, const geo::Coordinates &coordinates);

        void AddBus(std::string_view bus_name, const std::vector<std::string_view> &stops_names, bool is_roundtrip);

        std::vector<std::string_view> GetAllBusNames() const;

        std::vector<const domain::Stop *> GetAllStops() const;

        void SetDistance(const std::string &from, const std::string &to, int distance);

        int GetDistance(const domain::Stop *from, const domain::Stop *to) const;

        const domain::Stop *FindStop(std::string_view name) const noexcept;

        const domain::Bus *FindBus(std::string_view name) const noexcept;

        std::optional<std::vector<std::string_view> > GetBusesByStop(std::string_view stop_name) const;

        std::optional<domain::BusInfo> GetBusInfo(std::string_view bus_name) const noexcept;

    private:
        std::deque<std::string> stops_storage_;
        std::deque<std::string> buses_storage_;

        std::unordered_map<std::string_view, domain::Stop> all_stops_;
        std::unordered_map<std::string_view, domain::Bus> all_routes_;

        std::unordered_map<const domain::Stop *, std::unordered_set<std::string_view> > stops_to_buses_;

        struct StopPairHash {
            size_t operator()(const std::pair<const domain::Stop *, const domain::Stop *> &p) const noexcept {
                return std::hash<const void *>()(p.first) ^ (std::hash<const void *>()(p.second) << 1);
            }
        };

        std::unordered_map<std::pair<const  domain::Stop *, const  domain::Stop *>, int, StopPairHash> distances_;
    };
}
