#include "stat_reader.h"
#include "input_reader.h" 

#include <ostream>

std::string_view Trim(std::string_view string) {
	const auto start = string.find_first_not_of(' ');
	if (start == string.npos) {
		return {};
	}
	return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

namespace transport_catalogue{
	namespace readers{
		void ParseAndPrintStat(const TransportCatalogue& catalogue,
		                       std::string_view request,
		                       std::ostream& output) {
			auto space_pos = request.find(' ');
			if (space_pos == request.npos) {
				return;
			}

			std::string_view request_type = request.substr(0, space_pos);
			std::string_view request_name = Trim(request.substr(space_pos + 1));

			if (request_type == "Stop") {
				auto buses_opt = catalogue.GetBusesByStop(request_name);

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
				auto bus_info = catalogue.GetBusInfo(request_name);

				if (!bus_info) {
					output << "Bus " << request_name << ": not found\n";
				}
				else {
					output << "Bus " << bus_info->name << ": "
						<< bus_info->stop_count << " stops on route, "
						<< bus_info->unique_stop_count << " unique stops, "
						<< bus_info->route_length << " route length, "
						<< bus_info->curvature << " curvature\n";
				}
			}
		}
	} 
} 
