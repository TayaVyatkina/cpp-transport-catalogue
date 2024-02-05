#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "geo.h"
#include "transport_catalogue.h"

namespace transport_catalogue {
namespace input {

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
    std::string description;  // ��������� �������
};

}// namespace input

class InputReader {
    public:
    /**
        * ������ ������ � ��������� CommandDescription � ��������� ��������� � commands_
        */
    void ParseLine(std::string_view line);

    /**
        * ��������� ������� ������������ ����������, ��������� ������� �� commands_
        */
    void ApplyCommands(TransportCatalogue& catalogue) const;

private:
    std::vector<input::CommandDescription> commands_;
};

}// namespace transport_catalogue