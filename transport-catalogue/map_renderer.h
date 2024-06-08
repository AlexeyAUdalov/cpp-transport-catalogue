#pragma once

#include "domain.h"
#include "svg.h"
#include "domain.h"
#include "geo.h"

#include <algorithm>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <map>
#include <optional>
#include <vector>

inline const double EPSILON = 1e-6;
bool IsZero(double value);

class SphereProjector {
public:
	// points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
	template <typename PointInputIt>
	SphereProjector(PointInputIt points_begin, PointInputIt points_end,
		double max_width, double max_height, double padding)
		: padding_(padding) //
	{
		// Если точки поверхности сферы не заданы, вычислять нечего
		if (points_begin == points_end) {
			return;
		}

		// Находим точки с минимальной и максимальной долготой
		const auto [left_it, right_it] = std::minmax_element(
			points_begin, points_end,
			[](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
		min_lon_ = left_it->lng;
		const double max_lon = right_it->lng;

		// Находим точки с минимальной и максимальной широтой
		const auto [bottom_it, top_it] = std::minmax_element(
			points_begin, points_end,
			[](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
		const double min_lat = bottom_it->lat;
		max_lat_ = top_it->lat;

		// Вычисляем коэффициент масштабирования вдоль координаты x
		std::optional<double> width_zoom;
		if (!IsZero(max_lon - min_lon_)) {
			width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
		}

		// Вычисляем коэффициент масштабирования вдоль координаты y
		std::optional<double> height_zoom;
		if (!IsZero(max_lat_ - min_lat)) {
			height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
		}

		if (width_zoom && height_zoom) {
			// Коэффициенты масштабирования по ширине и высоте ненулевые,
			// берём минимальный из них
			zoom_coeff_ = std::min(*width_zoom, *height_zoom);
		}
		else if (width_zoom) {
			// Коэффициент масштабирования по ширине ненулевой, используем его
			zoom_coeff_ = *width_zoom;
		}
		else if (height_zoom) {
			// Коэффициент масштабирования по высоте ненулевой, используем его
			zoom_coeff_ = *height_zoom;
		}
	}

	// Проецирует широту и долготу в координаты внутри SVG-изображения
	svg::Point operator()(geo::Coordinates coords) const {
		return {
			(coords.lng - min_lon_) * zoom_coeff_ + padding_,
			(max_lat_ - coords.lat) * zoom_coeff_ + padding_
		};
	}

private:
	double padding_;
	double min_lon_ = 0;
	double max_lat_ = 0;
	double zoom_coeff_ = 0;
};

namespace map_renderer {

	struct RenderSettings {
		svg::Point picture_size;// ширина и высота изображения в пикселях.
							// Вещественное число в диапазоне от 0 до 100000
		double	padding = 0.0;// отступ краёв карты от границ SVG - документа.
							  // Вещественное число не меньше 0 и меньше min(width, height) / 2
		double	line_width = 0.0;// толщина линий, которыми рисуются автобусные маршруты.
								 // Вещественное число в диапазоне от 0 до 100000
		double	stop_radius = 0.0;// радиус окружностей, которыми обозначаются остановки.
								  // Вещественное число в диапазоне от 0 до 100000
		int	bus_label_font_size = 0;// размер текста, которым написаны названия автобусных маршрутов.
									  // Целое число в диапазоне от 0 до 100000.
		svg::Point	bus_label_offset;// смещение надписи с названием маршрута относительно
									 // координат конечной остановки на карте.
									 // Массив из двух элементов типа double.
									 // Задаёт значения свойств dx и dy SVG - элемента <text>.
									 // Элементы массива — числа в диапазоне от –100000 до 100000
		int	stop_label_font_size = 0;// размер текста, которым отображаются названия остановок.
									 // Целое число в диапазоне от 0 до 100000
		svg::Point stop_label_offset;// смещение названия остановки относительно её координат на карте.
									 // Массив из двух элементов типа double.
									 // Задаёт значения свойств dx и dy SVG - элемента <text>.
									 // Числа в диапазоне от –100000 до 100000
		svg::Color underlayer_color;// цвет подложки под названиями остановок и маршрутов.Формат хранения цвета будет ниже.
		double underlayer_width = 0.0;// толщина подложки под названиями остановок и маршрутов.
									  // Задаёт значение атрибута stroke - width элемента <text>.
									  // Вещественное число в диапазоне от 0 до 100000
		std::vector<svg::Color> color_palette;// цветовая палитра.Непустой массив
	};

	class MapRender {
	public:
		explicit MapRender(const RenderSettings& render_settings)
			:render_settings_(render_settings)
		{
		}

		svg::Document CreateMap(const std::map<std::string_view, const domain::Bus*>& buses, const std::map<std::string_view, const domain::Stop*>& stops) const;

	private:
		RenderSettings render_settings_;

		//Выводим линии маршрутов
		void RenderBusesPolyline(svg::Document& output_map,
			const std::map<std::string_view, const domain::Bus*>& buses, const SphereProjector& sphere_projector) const;

		//Выводим названия маршрутов
		void RenderBusesNames(svg::Document& output_map,
			const std::map<std::string_view, const domain::Bus*>& buses, const SphereProjector& sphere_projector) const;

		//Выводим названия остановок
		void RenderStopsNames(svg::Document& output_map,
			const std::map<std::string_view, const domain::Stop*>& stops, const SphereProjector& sphere_projector) const;

		svg::Text RenderSvgBusText(const std::string& text,
			svg::Point text_coordinates,
			int color_counter,
			bool is_it_background) const;

		svg::Text RenderSvgStopText(const std::string& text,
			svg::Point text_coordinates,
			bool is_it_background) const;
	};

}