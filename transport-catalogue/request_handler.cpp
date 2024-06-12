#include "request_handler.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <stdexcept>
#include <iomanip>
#include <iostream>

namespace request_handler {

    svg::Document RequestHandler::RenderMap() const {        
        std::map<std::string_view, const domain::Bus*> buses;
        for (const auto& [bus_name, bus_detail] : db_.GetBuses()) {
            buses.try_emplace(bus_name, bus_detail);
        }

        std::map<std::string_view, const domain::Stop*> stops;
        for (const auto& [stop_name, stop_detail] : db_.GetStops()) {
            if (!db_.GetBusesForStop(stop_name).empty()) {
                stops.try_emplace(stop_name, stop_detail);
            }
        }

        svg::Document output_map = map_renderer_.CreateMap(buses, stops);

        return output_map;
    }
}