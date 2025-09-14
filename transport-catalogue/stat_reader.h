#pragma once
#include "transport_catalogue.h"

namespace transport_catalogue{
	namespace readers{
		void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request,
		                       std::ostream& output);
	}
}
