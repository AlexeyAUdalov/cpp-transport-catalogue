#include <iostream>
#include <string>
#include <iostream>

#include "request_handler.h"
#include "json_reader.h"
#include "map_renderer.h"

using namespace std;

int main() {
    transport_catalogue::TransportCatalogue catalogue;
    json_reader::JsonReader input_request(cin);

    input_request.AddStopsToTransportCatalogue(catalogue);
    input_request.AddBusesToTransportCatalogue(catalogue);    
    input_request.AddDistancesBetweenStopsToTransportCatalogue(catalogue);

    map_renderer::MapRender map_renderer(input_request.AddRenderingSettings());

    request_handler::RequestHandler request_handler(catalogue, map_renderer);

    input_request.PrintStatistics(request_handler, catalogue, cout);
 }