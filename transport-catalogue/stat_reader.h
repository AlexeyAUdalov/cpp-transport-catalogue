#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

namespace stat_reader {
    struct CommandId {
        // ����������, ������ �� ������� (���� command ��������)
        explicit operator bool() const;

        bool operator!() const;

        std::string_view command;      // �������� �������
        std::string_view id;           // id �������� ��� ���������    
    };

    void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& tansport_catalogue, 
                           std::string_view request,
                           std::ostream& output);
}