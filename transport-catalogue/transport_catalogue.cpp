#include "transport_catalogue.h"
#include <algorithm>
#include <stdexcept>
#include <deque>

namespace transport_catalogue{
	void TransportCatalogue::AddStop(std::string_view name, const geo::Coordinates& coordinates) {
		auto& stored_name = stops_storage_.emplace_back(name);
		all_stops_[stored_name] = {.coordinates = coordinates};
	}

	void TransportCatalogue::AddBus(std::string_view bus_name, const std::vector<std::string_view>& stops_names) {
		Bus bus;
		bus.bus_stops.reserve(stops_names.size());

		auto& stored_bus_name = buses_storage_.emplace_back(bus_name);

		for (auto stop_name : stops_names) {
			auto it = all_stops_.find(stop_name);
			if (it == all_stops_.end()) {
				throw std::runtime_error("Stop not found: " + std::string(stop_name));
			}
			auto stop_ptr = &it->second;
			stops_to_buses_[stop_ptr].insert(stored_bus_name);
			bus.bus_stops.push_back(stop_ptr);
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

	bool TransportCatalogue::BusExists(std::string_view bus) const noexcept {
		return all_routes_.contains(bus);
	}

	size_t TransportCatalogue::GetStops(std::string_view bus) const noexcept {
		return all_routes_.at(bus).bus_stops.size();
	}

	size_t TransportCatalogue::GetUniqueStops(std::string_view bus) const noexcept {
		std::unordered_set<const Stop*> unique(
			all_routes_.at(bus).bus_stops.begin(),
			all_routes_.at(bus).bus_stops.end()
		);
		return unique.size();
	}

	double TransportCatalogue::GetRouteLength(std::string_view bus_name) const noexcept {
		const Bus& bus = all_routes_.at(bus_name);
		double route_length = 0;
		for (size_t i = 0; i + 1 < bus.bus_stops.size(); i++) {
			route_length += ComputeDistance(
				bus.bus_stops[i]->coordinates,
				bus.bus_stops[i + 1]->coordinates
			);
		}
		return route_length;
	}
}
