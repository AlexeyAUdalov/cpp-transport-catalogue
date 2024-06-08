#include "map_renderer.h"

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

namespace map_renderer {

    svg::Document MapRender::CreateMap(const std::map<std::string_view, const domain::Bus*>& buses, const std::map<std::string_view, const domain::Stop*>& stops) const {
        svg::Document output_map;

        std::deque<geo::Coordinates> stops_geo_coords;
        for (const auto& [bus_name, bus_detail] : buses) {
            if (!bus_detail->bus_stops.empty()) {
                for (const auto& stop : bus_detail->bus_stops) {
                    stops_geo_coords.push_back(stop->stop_coordinates);
                }
            }
        }

        // Создаём проектор сферических координат на карту
        const SphereProjector sphere_projector{ stops_geo_coords.begin(),
                                                stops_geo_coords.end(),
                                                render_settings_.picture_size.x,
                                                render_settings_.picture_size.y,
                                                render_settings_.padding };

        RenderBusesPolyline(output_map, buses, sphere_projector);
        RenderBusesNames(output_map, buses, sphere_projector);
        RenderStopsNames(output_map, stops, sphere_projector);
               
        return output_map;
    }

    void MapRender::RenderBusesPolyline(svg::Document& output_map,
        const std::map<std::string_view, const domain::Bus*>& buses,
        const SphereProjector& sphere_projector) const {
        size_t color_counter = 0;
        for (const auto& [bus_name, bus_detail] : buses) {
            if (!bus_detail->bus_stops.empty()) {
                svg::Polyline bus_svg;
                for (const auto& stop : bus_detail->bus_stops) {
                    bus_svg.AddPoint(sphere_projector(stop->stop_coordinates));
                }
                if (!bus_detail->is_roundtrip) {
                    for (auto it = bus_detail->bus_stops.rbegin() + 1; it != bus_detail->bus_stops.rend(); it++) {
                        bus_svg.AddPoint(sphere_projector((*it)->stop_coordinates));
                    }
                }
                bus_svg.SetStrokeColor(render_settings_.color_palette[color_counter]);
                ++color_counter;
                if (color_counter == render_settings_.color_palette.size()) {
                    color_counter = 0;
                }
                bus_svg.SetFillColor(std::string{ "none" });
                bus_svg.SetStrokeWidth(render_settings_.line_width);
                bus_svg.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
                bus_svg.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                output_map.Add(bus_svg);
            }
        }
    }

    void MapRender::RenderBusesNames(svg::Document& output_map,
        const std::map<std::string_view, const domain::Bus*>& buses, const SphereProjector& sphere_projector) const {
        size_t color_counter = 0;
        for (const auto& [bus_name, bus_detail] : buses) {
            if (!bus_detail->bus_stops.empty()) {
                const domain::Stop* the_firs_stop = bus_detail->bus_stops.front();
                svg::Point the_first_stop_coordinates = sphere_projector(the_firs_stop->stop_coordinates);

                svg::Text the_first_stop_svg = RenderSvgBusText(bus_detail->bus_name,
                    the_first_stop_coordinates,
                    static_cast<int>(color_counter),
                    false);

                svg::Text the_first_stop_background_svg = RenderSvgBusText(bus_detail->bus_name,
                    the_first_stop_coordinates,
                    static_cast<int>(color_counter),
                    true);

                output_map.Add(the_first_stop_background_svg);
                output_map.Add(the_first_stop_svg);

                if (!bus_detail->is_roundtrip && bus_detail->bus_stops.front() != bus_detail->bus_stops.back()) {
                    const domain::Stop* the_last_stop = bus_detail->bus_stops.back();
                    svg::Point the_last_stop_coordinates = sphere_projector(the_last_stop->stop_coordinates);

                    svg::Text the_last_stop_svg = RenderSvgBusText(bus_detail->bus_name,
                        the_last_stop_coordinates,
                        static_cast<int>(color_counter),
                        false);

                    svg::Text the_last_stop_background_svg = RenderSvgBusText(bus_detail->bus_name,
                        the_last_stop_coordinates,
                        static_cast<int>(color_counter),
                        true);

                    output_map.Add(the_last_stop_background_svg);
                    output_map.Add(the_last_stop_svg);
                }

                ++color_counter;
                if (color_counter == render_settings_.color_palette.size()) {
                    color_counter = 0;
                }
            }
        }
    }

    void MapRender::RenderStopsNames(svg::Document& output_map,
        const std::map<std::string_view, const domain::Stop*>& stops, const SphereProjector& sphere_projector) const {
        for (const auto& [stop_name, stop_detail] : stops) {
            svg::Circle stop_circle_svg;
            stop_circle_svg.SetCenter(sphere_projector(stop_detail->stop_coordinates));
            stop_circle_svg.SetRadius(render_settings_.stop_radius);
            stop_circle_svg.SetFillColor(std::string{ "white" });
            output_map.Add(stop_circle_svg);
        }

        for (const auto& [stop_name, stop_detail] : stops) {
            svg::Point stop_svg_coordinates = sphere_projector(stop_detail->stop_coordinates);

            svg::Text stop_svg = RenderSvgStopText(stop_detail->stop_name, stop_svg_coordinates, false);
            svg::Text stop_svg_background = RenderSvgStopText(stop_detail->stop_name, stop_svg_coordinates, true);

            output_map.Add(stop_svg_background);
            output_map.Add(stop_svg);
        }
    }

    svg::Text MapRender::RenderSvgBusText(const std::string& text,
        svg::Point text_coordinates,
        int color_counter,
        bool is_it_background) const {
        svg::Text svg_data;

        svg_data.SetData(text);
        svg_data.SetPosition(text_coordinates);
        svg_data.SetOffset(render_settings_.bus_label_offset);
        svg_data.SetFontSize(render_settings_.bus_label_font_size);
        svg_data.SetFontFamily(std::string{ "Verdana" });
        svg_data.SetFontWeight(std::string{ "bold" });

        if (is_it_background) {
            svg_data.SetFillColor(render_settings_.underlayer_color);
            svg_data.SetStrokeColor(render_settings_.underlayer_color);
            svg_data.SetStrokeWidth(render_settings_.underlayer_width);
            svg_data.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            svg_data.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        }
        else {
            svg_data.SetFillColor(render_settings_.color_palette[color_counter]);
        }

        return svg_data;
    }

    svg::Text MapRender::RenderSvgStopText(const std::string& text,
                                           svg::Point text_coordinates,
                                           bool is_it_background) const {
        svg::Text svg_data;

        svg_data.SetData(text);
        svg_data.SetPosition(text_coordinates);
        svg_data.SetOffset(render_settings_.stop_label_offset);
        svg_data.SetFontSize(render_settings_.stop_label_font_size);
        svg_data.SetFontFamily(std::string{ "Verdana" });

        if (is_it_background) {
            svg_data.SetFillColor(render_settings_.underlayer_color);
            svg_data.SetStrokeColor(render_settings_.underlayer_color);
            svg_data.SetStrokeWidth(render_settings_.underlayer_width);
            svg_data.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            svg_data.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        }
        else {
            svg_data.SetFillColor("black");
        }

        return svg_data;
    }

}