#pragma once

#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "domain.h"

namespace transport_catalogue {

	using DistancesContainer = std::vector<std::pair<int, std::string_view>>;

	struct PairStopsHasher {
	public:
		size_t operator()(const std::pair<const domain::Stop*, const domain::Stop*> stops) const;
	private:
		std::hash<const void*> hasher_;
	};

	class TransportCatalogue {
	public:
		void AddStop(std::string stop_name, geo::Coordinates stop_coordinates);

		void AddBus(std::string bus_name, std::vector<std::string_view> const& bus_stops, bool is_roundtrip);

		const domain::Stop* FindStop(std::string_view stop_name) const;

		const domain::Bus* FindBus(std::string_view bus_name) const;

		domain::Bus_Information GetBusInformation(const domain::Bus* bus_iterator) const;

		std::set<std::string_view> GetBusesForStop(std::string_view stop_name) const;

		void AddDistanceBetweenStops(const std::string& stop, const DistancesContainer& distances_container);

		int GetDistanceBetweenStops(const domain::Stop* from_stop, const domain::Stop* to_stop) const;

		const std::unordered_map<std::string_view, const domain::Bus*>& GetBuses() const;

		const std::unordered_map<std::string_view, const domain::Stop*>& GetStops() const;

	private:
		std::deque<domain::Stop> stops_;
		std::unordered_map<std::string_view, const domain::Stop*> stopname_to_stop_;
		std::deque<domain::Bus> buses_;
		std::unordered_map<std::string_view, const domain::Bus*> busname_to_bus_;
		std::unordered_map<std::string_view, std::set<std::string_view>> buses_for_stopname_;
		std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, int, PairStopsHasher> distances_between_stops_;
	};
}