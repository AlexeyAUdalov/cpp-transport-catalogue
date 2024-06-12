#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace request_handler {

    class RequestHandler {
    public:
        
        RequestHandler(const transport_catalogue::TransportCatalogue& catalogue, 
                       const map_renderer::MapRender& map_renderer)
            :db_(catalogue), map_renderer_(map_renderer)
        {
        }
         
        // Возвращает svg-документ, для отображения карты маршрутов
        svg::Document RenderMap() const;

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        const transport_catalogue::TransportCatalogue& db_;
        const map_renderer::MapRender& map_renderer_;
    };
}