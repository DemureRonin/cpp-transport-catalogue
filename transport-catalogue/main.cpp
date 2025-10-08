#include <iostream>
#include "transport_catalogue.h"
#include "request_handler.h"
#include "json.h"

using namespace transport_catalogue;
using namespace transport_catalogue::readers;

int main() {
    TransportCatalogue catalogue;
    RequestHandler handler(catalogue);

    handler.Load(std::cin);
    handler.ApplyCommands();

    json::Array result = handler.ProcessRequests();
    json::Print(json::Document{json::Node(result)}, std::cout);
}
