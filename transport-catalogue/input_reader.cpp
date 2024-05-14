#include "input_reader.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <iterator>
#include <map>
#include <set>
#include <stdexcept>

namespace input_reader {
    namespace detail {   
        /**
         * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
         */
        geo::Coordinates ParseCoordinates(std::string_view str) {
            static const double nan = std::nan("");

            auto not_space = str.find_first_not_of(' ');
            auto comma = str.find(',');

            if (comma == str.npos) {
                return { nan, nan };
            }

            auto not_space2 = str.find_first_not_of(' ', comma + 1);

            double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
            double lng = std::stod(std::string(str.substr(not_space2)));

            return { lat, lng };
        }

        /**
         * Удаляет пробелы в начале и конце строки
         */
        std::string_view Trim(std::string_view string) {
            const auto start = string.find_first_not_of(' ');
            if (start == string.npos) {
                return {};
            }
            return string.substr(start, string.find_last_not_of(' ') + 1 - start);
        }

        /**
         * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
         */
        std::vector<std::string_view> Split(std::string_view string, char delim) {
            std::vector<std::string_view> result;

            size_t pos = 0;
            while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
                auto delim_pos = string.find(delim, pos);
                if (delim_pos == string.npos) {
                    delim_pos = string.size();
                }
                if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
                    result.push_back(substr);
                }
                pos = delim_pos + 1;
            }
            return result;
        }

        /**
        * Парсит строку вида "7500m Universam" и возвращает пару (7500, Universam)
        */
        std::pair<int, std::string_view> DivideIntoDistanceAndStop(std::string_view str) {
            static const int min_distance = 0;
            static const int max_distance = 1000000;// задано условием задачи
            std::string_view distance_and_stop_devider = "m to ";
            auto devider_first_symbol = str.find(distance_and_stop_devider);

            if (devider_first_symbol != str.npos) {
                int distance = std::stoi(std::string(Trim(str.substr(0, devider_first_symbol))));
                if (distance < min_distance || distance > max_distance) {
                    return { 0, std::string_view{} };
                }
                std::string_view stop = Trim(str.substr(devider_first_symbol + distance_and_stop_devider.size()));

                return { distance, stop };
            }

            return { 0, std::string_view{} };
        }

        /**
         * Парсит маршрут.
         * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
         * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
         */
        std::vector<std::string_view> ParseRoute(std::string_view route) {
            if (route.find('>') != route.npos) {
                return Split(route, '>');
            }

            auto stops = Split(route, '-');
            std::vector<std::string_view> results(stops.begin(), stops.end());
            results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

            return results;
        }

        CommandDescription ParseCommandDescription(std::string_view line) {
            auto colon_pos = line.find(':');
            if (colon_pos == line.npos) {
                return {};
            }

            auto space_pos = line.find(' ');
            if (space_pos >= colon_pos) {
                return {};
            }

            auto not_space = line.find_first_not_of(' ', space_pos);
            if (not_space >= colon_pos) {
                return {};
            }

            return { std::string(line.substr(0, space_pos)),
                    std::string(line.substr(not_space, colon_pos - not_space)),
                    std::string(line.substr(colon_pos + 1)) };
        }
    }

    CommandDescription::operator bool() const {
        return !command.empty();
    }

    bool CommandDescription::operator!() const {
        return !operator bool();
    }

    void InputReader::ParseLine(std::string_view line) {
        auto command_description = detail::ParseCommandDescription(line);
        if (command_description) {
            commands_.push_back(std::move(command_description));
        }
    }    

    void InputReader::ApplyCommands([[maybe_unused]] transport_catalogue::TransportCatalogue& catalogue) const {
        std::array<std::string, 2> commands{ {std::string{"Stop"}, std::string{"Bus"} } };
        std::map<std::string, std::set<std::pair<int, std::string_view>>> distances_container;

        for (auto const& command : commands) {
            for (auto const& input_command : commands_) {
                if (input_command.command == commands[0] && command == commands[0]) {
                    std::vector<std::string_view> description_parts = detail::Split(input_command.description, ',');

                    if (description_parts.size() >= 2) {
                        double latitude = std::stod(std::string(description_parts[0]));
                        double longitude = std::stod(std::string(description_parts[1]));
                        if ((latitude >= -90 && latitude <= 90) && (longitude >= -180 && longitude <= 180)){
                            catalogue.AddStop(input_command.id, { latitude , longitude });
                        }
                        else {
                            throw std::out_of_range(std::string{ "Coordinates are out of range" });
                        }

                        for (size_t item = 2; item < description_parts.size(); ++item) {
                            std::pair<int, std::string_view> distance_and_stop = 
                                detail::DivideIntoDistanceAndStop(description_parts[item]);
                            if (distance_and_stop.first != 0 && distance_and_stop.second != std::string_view{}) {
                                distances_container[input_command.id].insert(distance_and_stop);
                            }
                            else {
                                throw std::invalid_argument(std::string{ "There are not stop and distance" });
                            }
                        }                        
                    }
                    else {
                        throw std::invalid_argument(std::string{ "There are not stop coordinates" });
                    }
                }
                else if (input_command.command == commands[1] && command == commands[1]) {
                    catalogue.AddBus(input_command.id, detail::ParseRoute(input_command.description));
                }
            }
        }

        catalogue.AddDistancesBetwinStops(distances_container);
    }
}