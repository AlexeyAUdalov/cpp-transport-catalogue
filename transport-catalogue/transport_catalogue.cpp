#include "transport_catalogue.h"

#include <stdexcept>

namespace transport_catalogue {
	size_t PairStopsHasher::operator()(const std::pair<const Stop*, const Stop*> stops) const {
		return hasher_(stops.first) + 53 * hasher_(stops.second);
	}

	void TransportCatalogue::AddStop(std::string stop_name, geo::Coordinates stop_coordinates) {
		Stop temp_stop_object = { std::move(stop_name), std::move(stop_coordinates) };
		Stop const& stop = stops_.emplace_back(std::move(temp_stop_object));
		stopname_to_stop_[static_cast<std::string_view>(stops_.back().stop_name)] = &stop;
	}

	void TransportCatalogue::AddBus(std::string bus_name, std::vector<std::string_view> const& bus_stops) {
		std::vector<const Stop*> input_stops;

		for (auto const& bus_stop : bus_stops) {
			if (stopname_to_stop_.find(bus_stop) != stopname_to_stop_.end()) {
				auto const& stop = stopname_to_stop_.at(bus_stop);
				input_stops.push_back(stop);
			}
			else {
				throw std::out_of_range(std::string{ "There is not stop in data base" });
			}
		}

		Bus temp_bus_object = { std::move(bus_name),
								std::move(input_stops) };
		Bus const& bus = buses_.emplace_back(std::move(temp_bus_object));
		busname_to_bus_[static_cast<std::string_view>(buses_.back().bus_name)] = &bus;
		for (auto const& stop : buses_.back().bus_stops) {
			buses_for_stopname_[stop->stop_name].insert(buses_.back().bus_name);
		}
	}

	const Stop* TransportCatalogue::FindStop(std::string_view stop_name) const {
		if (stopname_to_stop_.find(stop_name) != stopname_to_stop_.end()) {
			return stopname_to_stop_.at(stop_name);
		}
		else {
			return nullptr;
		}
	}

	const Bus* TransportCatalogue::FindBus(std::string_view bus_name) const {
		if (busname_to_bus_.find(bus_name) != busname_to_bus_.end()) {
			return busname_to_bus_.at(bus_name);
		}
		else {
			return nullptr;
		}
	}

	Bus_Information TransportCatalogue::GetBusInformation(const Bus* bus_iterator) const {
		std::unordered_set<std::string_view> unique_bus_stops;
		int bus_route_length = 0;
		double geografical_bus_route_length = 0;
		const Stop* from_stop = bus_iterator->bus_stops[0];
		bool the_fist_stop = true;

		for (auto const& bus_stop : bus_iterator->bus_stops) {
			unique_bus_stops.insert(bus_stop->stop_name);
			if (the_fist_stop) {
				the_fist_stop = false;
			}
			else {
				bus_route_length += GetRealDistance(from_stop, bus_stop);								
				geografical_bus_route_length += geo::ComputeDistance(from_stop->stop_coordinates, 
					                                                 bus_stop->stop_coordinates);
				from_stop = bus_stop;
			}
		}

		return { bus_iterator->bus_name,
				 static_cast<int>(bus_iterator->bus_stops.size()),
				 static_cast<int>(unique_bus_stops.size()),
				 bus_route_length,
				 geografical_bus_route_length };
	}

	std::set<std::string_view> TransportCatalogue::GetBusesForStop(std::string_view stop_name) const {
		if (buses_for_stopname_.find(stop_name) != buses_for_stopname_.end()) {
			return buses_for_stopname_.at(stop_name);
		}
		else {
			return {};
		}
	}

	void TransportCatalogue::AddRealDistance(const std::string& stop, const DistancesContainer& distances_container) {
		if (distances_container.size() == 0) {
			return;
		}

		const Stop* from_stop = FindStop(stop);

		for (const auto& [real_distance, destination] : distances_container) {
			const Stop* to_stop = FindStop(destination);		
			distances_betwin_stops_[{from_stop, to_stop}] = real_distance;
		}
	}

	int TransportCatalogue::GetRealDistance(const Stop* from_stop, const Stop* to_stop) const {
		std::pair<const Stop*, const Stop*> stops = { from_stop, to_stop };
		std::pair<const Stop*, const Stop*> inverse_stops = { to_stop, from_stop };
		return distances_betwin_stops_.count(stops) ? distances_betwin_stops_.at(stops) : 
			                                          distances_betwin_stops_.at(inverse_stops);
	}
}