#include "stat_reader.h"

#include <iomanip>
#include <iostream>

namespace stat_reader {
    namespace detail {
        CommandId GetCommandAndId(std::string_view request) {
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

            if (id != std::string_view{}) {
                    return { command, id };
            }

            return { std::string_view{}, std::string_view{} };
        }

        void FindAndPrintBusesForStop(const transport_catalogue::TransportCatalogue& tansport_catalogue,
                                      const CommandId& command,
                                      std::ostream& output) {
            output << command.command << std::string{ " " }
            << command.id << std::string{ ": " };
            std::set<std::string_view> buses;
            buses = tansport_catalogue.GetBusesForStop(command.id);
            
            if (buses.size() > 0) {
                output << std::string{ "buses" };
                for (auto bus : buses) {
                    output << std::string{ " " } << bus;
                }
            }
            else if (tansport_catalogue.FindStop(command.id) != nullptr) {
                output << "no buses";
            }
            else {
                output << "not found";
            }
            output << std::endl;
        }

        void FindBusAndPrintBusInformation(const transport_catalogue::TransportCatalogue& tansport_catalogue,
            const CommandId& command,
            std::ostream& output) {
            const transport_catalogue::Bus* bus_iterator = tansport_catalogue.FindBus(command.id);
            if (bus_iterator != nullptr) {
                transport_catalogue::Bus_Information bus_information =
                    tansport_catalogue.GetBusInformation(bus_iterator);

                output << command.command << std::string{ " " }
                    << bus_information.bus_name << std::string{ ": " }
                    << bus_information.stops_on_bus_route << std::string{ " stops on route, " }
                    << bus_information.unique_bus_stops << std::string{ " unique stops, " }
                    << bus_information.bus_route_length                    
                    << std::string{ " route length, " }
                    << std::setprecision(6)
                    << static_cast<double>(bus_information.bus_route_length) /
                                           bus_information.geografical_bus_route_length
                    << std::string{ " curvature" } << std::endl;
            }
            else {
                output << command.command << std::string{ " " }
                << command.id << std::string{ ": not found" } << std::endl;
            }
        }
    }

    CommandId::operator bool() const {
        return !command.empty();
    }

    bool CommandId::operator!() const {
        return !operator bool();
    }

    void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& tansport_catalogue,
                           std::string_view request,
                           std::ostream& output) {
        const auto stop = std::string{ "Stop" };
        const auto bus = std::string{ "Bus" };
        
        CommandId in_command = detail::GetCommandAndId(request);

        if (in_command.command == stop) {
            detail::FindAndPrintBusesForStop(tansport_catalogue, in_command, output);
        }
        else if (in_command.command == bus) {
            detail::FindBusAndPrintBusInformation(tansport_catalogue, in_command, output);
        }
    }
}