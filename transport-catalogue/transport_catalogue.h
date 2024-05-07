#pragma once
#include <deque>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geo.h"

namespace transport_catalogue {
	struct Stop {
		std::string stop_name;
		geo::Coordinates stop_coordinates;
	};

	struct Bus {
		std::string bus_name;
		std::vector<const Stop*> bus_stops;
	};

	struct Bus_Information {
		std::string bus_name;
		int stops_on_bus_route;
		int unique_bus_stops;
		double bus_route_length;
	};

	class TransportCatalogue {
	public:
		void AddStop(std::string stop_name, geo::Coordinates stop_coordinates);

		void AddBus(std::string bus_name, std::vector<std::string_view> const& bus_stops);

		const Stop* FindStop(std::string_view stop_name) const;

		const Bus* FindBus(std::string_view bus_name) const;

		Bus_Information GetBusInformation(const Bus* bus_iterator) const;

		const std::set<std::string_view> GetBusesForStop(std::string_view stop_name) const;

	private:
		std::deque<Stop> stops_;
		std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
		std::unordered_map<std::string_view, std::set<std::string_view>> buses_for_stopname_;
	};
}