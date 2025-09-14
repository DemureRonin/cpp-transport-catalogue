#pragma once

#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <deque>
#include <optional>

#include "geo.h"

namespace transport_catalogue{
	class TransportCatalogue {
		struct Stop {
			geo::Coordinates coordinates;
		};

		struct Bus {
			std::vector<const Stop*> bus_stops;
		};

		std::unordered_map<std::string_view, Bus> all_routes_;
		std::unordered_map<std::string_view, Stop> all_stops_;
		std::unordered_map<const Stop*, std::set<std::string_view>> stops_to_buses_;

		std::deque<std::string> stops_storage_;
		std::deque<std::string> buses_storage_;

	public:
		void AddStop(std::string_view name, const geo::Coordinates& coordinates);

		void AddBus(std::string_view bus_name, const std::vector<std::string_view>& stops_names);

		std::optional<std::vector<std::string_view>>
		GetBusesByStop(std::string_view stop_name) const;

		bool BusExists(std::string_view bus) const noexcept;

		size_t GetStops(std::string_view bus) const noexcept;

		size_t GetUniqueStops(std::string_view bus) const noexcept;

		double GetRouteLength(std::string_view bus_name) const noexcept;
	};
}
