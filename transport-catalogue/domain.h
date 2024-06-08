#pragma once

#include "geo.h"

#include <string>
#include <vector>

namespace domain {

	struct Stop {
		std::string stop_name;
		geo::Coordinates stop_coordinates;
	};

	struct Bus {
		std::string bus_name;
		std::vector<const Stop*> bus_stops;
		bool is_roundtrip;
	};

	struct Bus_Information {
		std::string bus_name;
		int stops_on_bus_route;
		int unique_bus_stops;
		int bus_route_length;
		double geografical_bus_route_length;
		double curvature;
	};


}