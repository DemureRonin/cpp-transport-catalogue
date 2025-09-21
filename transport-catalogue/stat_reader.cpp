#include "transport_catalogue.h"
#include <ostream>

namespace transport_catalogue {
	namespace readers {

		void ParseAndPrintStat(const TransportCatalogue& transport_catalogue,
							   std::string_view request,
							   std::ostream& output) {
			std::string request_type = std::string(request.substr(0, request.find(' ')));
			std::string request_name = std::string(request.substr(request.find(' ') + 1));

			if (request_type == "Stop") {
				auto buses_opt = transport_catalogue.GetBusesByStop(request_name);

				if (!buses_opt) {
					output << "Stop " << request_name << ": not found\n";
				}
				else if (buses_opt->empty()) {
					output << "Stop " << request_name << ": no buses\n";
				}
				else {
					output << "Stop " << request_name << ": buses";
					for (const auto& bus : *buses_opt) {
						output << " " << bus;
					}
					output << "\n";
				}
			}
			else if (request_type == "Bus") {
				auto bus_info = transport_catalogue.GetBusInfo(request_name);

				if (!bus_info) {
					output << "Bus " << request_name << ": not found\n";
				}
				else {
					output << "Bus " << bus_info->name << ": "
						   << bus_info->stop_count << " stops on route, "
						   << bus_info->unique_stop_count << " unique stops, "
						   << bus_info->route_length << " route length\n";
				}
			}
		}

	} 
} 
