#include "transport_catalogue.h"
#include <ostream>

namespace transport_catalogue{
	class TransportCatalogue;
}

namespace transport_catalogue{
	namespace readers{
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
				if (transport_catalogue.BusExists(request_name)) {
					output << "Bus " << request_name << ": "
						<< transport_catalogue.GetStops(request_name)
						<< " stops on route, "
						<< transport_catalogue.GetUniqueStops(request_name)
						<< " unique stops, "
						<< transport_catalogue.GetRouteLength(request_name)
						<< " route length\n";
				}
				else {
					output << "Bus " << request_name << ": not found\n";
				}
			}
		}
	}
}
