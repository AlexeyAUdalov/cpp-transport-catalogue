#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"

#include<vector>

namespace json_reader {
	
	using namespace json;

	class JsonReader {
	public:
		JsonReader(std::istream& input)
			: input_json_(json::Load(input))
		{
		}

		void AddStopsToTransportCatalogue(transport_catalogue::TransportCatalogue& catalogue) const;

		void AddBusesToTransportCatalogue(transport_catalogue::TransportCatalogue& catalogue) const;
		
		void AddDistancesBetweenStopsToTransportCatalogue(transport_catalogue::TransportCatalogue& catalogue) const;
		
		map_renderer::RenderSettings AddRenderingSettings();
		
		void PrintStatistics(const request_handler::RequestHandler& request_handler,
			                 const transport_catalogue::TransportCatalogue& catalogue,
			                 std::ostream& output);

	private:
		Document input_json_;
	};
}