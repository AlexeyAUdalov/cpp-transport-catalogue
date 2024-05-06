#include "stat_reader.h"

#include <array>
#include <iomanip>
#include <iostream>

namespace stat_reader {
    CommandId GetCommandAndId(const std::array<std::string, 2>& commands, std::string_view request) {
        std::string_view command;
        std::string_view id;

        auto space_pos = request.find(' ');
        if (space_pos == std::string_view::npos) {
            command = std::string_view{};
        }
        else {
            command = std::string_view(request.substr(0, space_pos));
        }

        auto not_space = request.find_first_not_of(' ', space_pos);
        if (not_space == std::string_view::npos) {
            id = std::string_view{};
        }
        else {
            auto last_pos = request.find_last_not_of(' ');
            id = std::string_view(request.substr(not_space, last_pos - space_pos));
        }

        for (const auto& command_from_commands : commands) {
            if (command_from_commands == command && id != std::string_view{}) {
                return { command, id };
            }
        }

        return { std::string_view{}, std::string_view{} };
    }

    void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& tansport_catalogue,
                           std::string_view request,
                           std::ostream& output) {
        std::array<std::string, 2> commands{ {std::string{"Stop"}, std::string{"Bus"} } };

        CommandId input_command = GetCommandAndId(commands, request);

        for (const auto& command_from_commands : commands) {
            if (input_command.command == commands[0] && command_from_commands == commands[0]) {
                output << input_command.command << std::string{ " " }
                << input_command.id << std::string{ ": " };
                std::set<std::string_view> buses;
                buses = tansport_catalogue.GetBussesForStop(input_command.id);
                if (buses.size() > 0) {
                    output << std::string{ "buses" };
                    for (auto bus : buses) {
                        output << std::string{ " " } << bus;
                    }
                }
                else if (tansport_catalogue.FindStop(input_command.id) != nullptr) {
                    output << "no buses";
                }
                else {
                    output << "not found";
                }
                output << std::endl;
            }
            else if (input_command.command == commands[1] && command_from_commands == commands[1]) {
                const transport_catalogue::Bus* bus_iterator = tansport_catalogue.FindBus(input_command.id);
                if (bus_iterator != nullptr) {
                    transport_catalogue::Bus_Information bus_information = 
                        tansport_catalogue.GetBusInformation(bus_iterator);

                    output << input_command.command << std::string{ " " }
                        << bus_information.bus_name << std::string{ ": " }
                        << bus_information.stops_on_bus_route << std::string{ " stops on route, " }
                        << bus_information.unique_bus_stops << std::string{ " unique stops, " }
                        << std::setprecision(6) << bus_information.bus_route_length
                        << std::string{ " route length" } << std::endl;
                }
                else {
                    output << input_command.command << std::string{ " " }
                    << input_command.id << std::string{ ": not found" } << std::endl;
                }
            }
        }
    }
}