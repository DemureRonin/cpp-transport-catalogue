#pragma once

#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <deque>
#include <optional>

#include "geo.h"

namespace transport_catalogue {
	
	struct Stop {
		std::string_view name;
		geo::Coordinates coordinates;
	};

	struct Bus {
		std::string_view name;
		std::vector<const Stop*> stops;
	};

	struct BusInfo {
		std::string_view name;
		size_t stop_count = 0;
		size_t unique_stop_count = 0;
		double route_length = 0.0;
	};

	class TransportCatalogue {
	public:
		
		void AddStop(std::string_view name, const geo::Coordinates& coordinates);
		void AddBus(std::string_view bus_name, const std::vector<std::string_view>& stops_names);
		
		const Stop* FindStop(std::string_view name) const noexcept;
		const Bus* FindBus(std::string_view name) const noexcept;

		std::optional<std::vector<std::string_view>> GetBusesByStop(std::string_view stop_name) const;
		std::optional<BusInfo> GetBusInfo(std::string_view bus_name) const noexcept;

	private:
		std::unordered_map<std::string_view, Bus> all_routes_;
		std::unordered_map<std::string_view, Stop> all_stops_;
		std::unordered_map<const Stop*, std::set<std::string_view>> stops_to_buses_;

		std::deque<std::string> stops_storage_;
		std::deque<std::string> buses_storage_;
	};

} 
