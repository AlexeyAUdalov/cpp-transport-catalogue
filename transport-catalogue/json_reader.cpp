#include "json_reader.h"

#include <stdexcept>
#include <sstream>

namespace json_reader {
	using namespace std::literals;

	void JsonReader::AddStopsToTransportCatalogue(transport_catalogue::TransportCatalogue& catalogue) const {
		std::string base_requests = "base_requests"s;
		if (input_json_.GetRoot().AsMap().count(base_requests) > 0) {
			auto& input_data = input_json_.GetRoot().AsMap().at(base_requests).AsArray();
			for (auto& input_data_elemant : input_data) {
				auto& data = input_data_elemant.AsMap();
				if (data.count( "type"s )) {					
					if (data.at("type"s).AsString() == "Stop"s) {
						catalogue.AddStop(data.at("name"s).AsString(),
							{ data.at("latitude"s).AsDouble(), data.at("longitude"s).AsDouble() });
					}
				}
				else {
					throw std::invalid_argument(std::string{ "Unknown request type" });
				}
			}
		} 
	}

	void JsonReader::AddBusesToTransportCatalogue(transport_catalogue::TransportCatalogue& catalogue) const {
		std::string base_requests = "base_requests"s;
		if (input_json_.GetRoot().AsMap().count(base_requests) > 0) {
			auto& input_data = input_json_.GetRoot().AsMap().at(base_requests).AsArray();
			for (auto& input_data_elemant : input_data) {
				auto& data = input_data_elemant.AsMap();
				if (data.count("type"s)) {
					if (data.at("type"s).AsString() == "Bus"s) {
						std::vector<std::string_view> stops;
						for (auto& stop : data.at("stops"s).AsArray()) {
							stops.push_back(stop.AsString());
						}
						catalogue.AddBus(data.at("name"s).AsString(), stops, data.at("is_roundtrip"s).AsBool());
					}
				}
				else {
					throw std::invalid_argument(std::string{ "Unknown request type" });
				}
			}
		}
	}

	void JsonReader::AddDistancesBetweenStopsToTransportCatalogue(
		transport_catalogue::TransportCatalogue& catalogue) const {
		std::string base_requests = "base_requests"s;
		if (input_json_.GetRoot().AsMap().count(base_requests) > 0) {
			auto& input_data = input_json_.GetRoot().AsMap().at(base_requests).AsArray();
			for (auto& input_data_elemant : input_data) {
				auto& data = input_data_elemant.AsMap();
				if (data.count("type"s)) {
					if (data.at("type"s).AsString() == "Stop"s and data.count("road_distances"s) > 0) {
						auto& road_distances = data.at("road_distances"s).AsMap();
						transport_catalogue::DistancesContainer distances_container;
						for (auto& [stop, distance] : road_distances) {
							distances_container.emplace_back(distance.AsInt(), stop);							
						}
						catalogue.AddDistanceBetweenStops(data.at("name"s).AsString(), distances_container);
					}
				}
				else {
					throw std::invalid_argument(std::string{ "Unknown request type" });
				}
			}
		}
	}

	svg::Color ConvertColorToRgbOrRgbaFormat(std::vector<Node> color_array) {
		svg::Color output_color;
		if (color_array.size() == 3) {
			svg::Rgb rgb_color = { static_cast<uint8_t>(color_array.begin()->AsInt()),
						           static_cast<uint8_t>((color_array.begin() + 1)->AsInt()),
								   static_cast<uint8_t>((color_array.begin() + 2)->AsInt()) };
			output_color = rgb_color;
		} else if (color_array.size() == 4) {
			svg::Rgba rgba_color = { static_cast<uint8_t>(color_array.begin()->AsInt()),
									 static_cast<uint8_t>((color_array.begin() + 1)->AsInt()),
									 static_cast<uint8_t>((color_array.begin() + 2)->AsInt()),
			                         (color_array.begin() + 3)->AsDouble() };
			output_color = rgba_color;
		}
		return output_color;
	}

	map_renderer::RenderSettings JsonReader::AddRenderingSettings() {
		map_renderer::RenderSettings output_settings;
		std::string render_settings = "render_settings"s;
		if (input_json_.GetRoot().AsMap().count(render_settings) > 0) {
			auto& data = input_json_.GetRoot().AsMap().at(render_settings).AsMap();
			double width = data.at( "width"s ).AsDouble();
			double height = data.at( "height"s ).AsDouble();
			double padding = data.at( "padding"s ).AsDouble();
			double line_width = data.at( "line_width"s ).AsDouble();
			double stop_radius = data.at( "stop_radius"s ).AsDouble();
			int	bus_label_font_size = data.at( "bus_label_font_size"s ).AsInt();
			auto bus_label_offset_array = data.at( "bus_label_offset"s ).AsArray();
			svg::Point bus_label_offset = { bus_label_offset_array.front().AsDouble(),
			                                bus_label_offset_array.back().AsDouble() };
			int	stop_label_font_size = data.at( "stop_label_font_size"s ).AsInt();
			auto stop_label_offset_array = data.at( "stop_label_offset"s ).AsArray();
			svg::Point stop_label_offset = { stop_label_offset_array.front().AsDouble(),
				                             stop_label_offset_array.back().AsDouble() };
			svg::Color underlayer_color;
			if (data.at( "underlayer_color"s ).IsArray()) {
				underlayer_color = ConvertColorToRgbOrRgbaFormat(data.at( "underlayer_color"s ).AsArray());
			}
			else if (data.at( "underlayer_color"s ).IsString()) {
				underlayer_color = data.at( "underlayer_color"s ).AsString();
			}
			double underlayer_width = data.at( "underlayer_width"s ).AsDouble();
			std::vector<svg::Color> color_palette;
			auto color_palette_array = data.at( "color_palette"s ).AsArray();
			for (auto& value : color_palette_array) {
				if (value.IsString()) {
					color_palette.push_back(value.AsString());
				}
				else if (value.IsArray()) {
					color_palette.push_back(ConvertColorToRgbOrRgbaFormat(value.AsArray()));
				}
			}
			output_settings = { {width, height},
				                                      padding, 
				                                      line_width,
				                                      stop_radius,
				                                      bus_label_font_size,
				                                      bus_label_offset,
				                                      stop_label_font_size,
				                                      stop_label_offset,
				                                      underlayer_color,
				                                      underlayer_width,
				                                      color_palette };
		}
		return output_settings;
	}

	Node CreateStopNode(const Dict& data, const request_handler::RequestHandler& catalogue) {
		int request_id = data.at("id"s).AsInt();
		std::string stop_name = data.at("name"s).AsString();
		auto stop_is_exist = catalogue.StopIsExist(stop_name);
		if (!stop_is_exist) {
			Node stop{ Dict{ {"request_id"s, request_id}, {"error_message"s, "not found"s} } };
			return stop;
		}
		else {
			auto& buses_for_stop_name = catalogue.GetBusesByStop(stop_name);
			Array buses;
			for (auto& bus : buses_for_stop_name) {
				Node a{ std::string{bus} };
				buses.emplace_back(a);
			}
			Node stop{ Dict{{"buses"s, buses}, {"request_id"s, request_id}} };
			return stop;
		}
	}

	Node CreateBusNode(const Dict& data, const request_handler::RequestHandler& catalogue) {
		int request_id = data.at("id"s).AsInt();
		std::string bus_name = data.at("name"s).AsString();
		auto bus_information = catalogue.GetBusStat(bus_name);
		if (bus_information) {
			Node bus{ Dict{ {"curvature", bus_information->curvature},
							{"request_id", request_id},
							{"route_length", bus_information->bus_route_length},
							{"stop_count", bus_information->stops_on_bus_route},
							{"unique_stop_count", bus_information->unique_bus_stops}} };
			return bus;
		}
		else {
			Node bus{ Dict{ {"request_id"s, request_id}, {"error_message"s, "not found"s} } };
			return bus;
		}
	}

	Node CreateMapNode(const Dict& data, const request_handler::RequestHandler& catalogue) {
		int request_id = data.at("id"s).AsInt();
		std::ostringstream out;
		catalogue.RenderMap().Render(out);
		Node dict_node{ Dict{{"map"s, out.str()}, {"request_id", request_id}} };
		return dict_node;
	}

	void JsonReader::PrintStatistics(const request_handler::RequestHandler& catalogue, std::ostream& output) {
		std::string stat_requests = "stat_requests"s;
		json::Array output_array;
		if (input_json_.GetRoot().AsMap().count(stat_requests) > 0) {
			auto& input_data = input_json_.GetRoot().AsMap().at(stat_requests).AsArray();
			for (auto& input_data_elemant : input_data) {
				auto& data = input_data_elemant.AsMap();
				if (data.count("type"s)) {
					if (data.at("type"s).AsString() == "Stop"s) {
						Node stop = CreateStopNode(data, catalogue);
						output_array.push_back(stop);
					}
					else if (data.at("type"s).AsString() == "Bus"s) {
						Node bus = CreateBusNode(data, catalogue);
						output_array.push_back(bus);
					}
					else if (data.at("type"s).AsString() == "Map"s) {
						Node map = CreateMapNode(data, catalogue);
						output_array.push_back(map);
					}
				}
				else {
					throw std::invalid_argument(std::string{ "Unknown request type" });
				}
			}
			json::Print(Document{ output_array }, output);
		}
	}
}