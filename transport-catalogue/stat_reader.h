#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

namespace transport_catalogue {
namespace output {
struct CommandDescription {
    // ����������, ������ �� ������� (���� command ��������)
    explicit operator bool() const {
        return !command.empty();
    }

    bool operator!() const {
        return !operator bool();
    }

    std::string command;      // �������� �������
    std::string id;           // id �������� ��� ���������
};
}// namespace output
void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
    std::ostream& output);
}// namespace transport_catalogue


