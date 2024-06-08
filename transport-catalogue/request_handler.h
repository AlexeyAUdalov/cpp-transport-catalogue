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
         
        // Возвращает информацию о маршруте (запрос Bus)
        std::optional<domain::Bus_Information> GetBusStat(const std::string_view& bus_name) const;

        // Возвращает маршруты, проходящие через
        const std::set<std::string_view> GetBusesByStop(const std::string_view& stop_name) const;

        // Возвращает true, если остановка есть в базе "Транспортный Справочник"
        bool StopIsExist(const std::string_view& stop_name) const;

        // Этот метод будет нужен в следующей части итогового проекта
        svg::Document RenderMap() const;

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        const transport_catalogue::TransportCatalogue& db_;
        const map_renderer::MapRender& map_renderer_;
    };
}