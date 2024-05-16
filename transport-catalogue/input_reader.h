#pragma once
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "geo.h"
#include "transport_catalogue.h"

namespace input_reader {
    using DistancesContainer = std::vector<std::pair<int, std::string_view>>;

    struct CommandDescription {
        // Определяет, задана ли команда (поле command непустое)
        explicit operator bool() const;

        bool operator!() const;

        std::string command;      // Название команды
        std::string id;           // id маршрута или остановки
        std::string description;  // Параметры команды
    };

    class InputReader {
    public:
        /**
         * Парсит строку в структуру CommandDescription и сохраняет результат в commands_
         */
        void ParseLine(std::string_view line);

        /**
         * Наполняет данными транспортный справочник, используя команды из commands_
         */
        void ApplyCommands(transport_catalogue::TransportCatalogue& catalogue) const;

    private:
        std::vector<CommandDescription> commands_;
    };
}