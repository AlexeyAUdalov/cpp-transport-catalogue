#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

namespace stat_reader {
    struct CommandId {
        // Определяет, задана ли команда (поле command непустое)
        explicit operator bool() const;

        bool operator!() const;

        std::string_view command;      // Название команды
        std::string_view id;           // id маршрута или остановки    
    };

    void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& tansport_catalogue, 
                           std::string_view request,
                           std::ostream& output);
}