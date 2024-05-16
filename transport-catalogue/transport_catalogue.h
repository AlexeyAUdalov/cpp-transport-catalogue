#pragma once
#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geo.h"

namespace transport_catalogue {
	using DistancesContainer = std::vector<std::pair<int, std::string_view>>;

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
		int bus_route_length;
		double geografical_bus_route_length;
	};

	struct PairStopsHasher {
	public:
		size_t operator()(const std::pair<const Stop*, const Stop*> stops) const;
	private:
		std::hash<const void*> hasher_;
	};

	class TransportCatalogue {
	public:
		void AddStop(std::string stop_name, geo::Coordinates stop_coordinates);

		void AddBus(std::string bus_name, std::vector<std::string_view> const& bus_stops);

		const Stop* FindStop(std::string_view stop_name) const;

		const Bus* FindBus(std::string_view bus_name) const;

		Bus_Information GetBusInformation(const Bus* bus_iterator) const;

		std::set<std::string_view> GetBusesForStop(std::string_view stop_name) const;

		void AddRealDistance(const std::string& stop, const DistancesContainer& distances_container);

		int GetRealDistance(const Stop* from_stop, const Stop* to_stop) const;

	private:
		std::deque<Stop> stops_;
		std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
		std::unordered_map<std::string_view, std::set<std::string_view>> buses_for_stopname_;
		std::unordered_map<std::pair<const Stop*, const Stop*>, int, PairStopsHasher> distances_betwin_stops_;
	};
}