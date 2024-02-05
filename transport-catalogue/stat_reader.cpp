#include <iomanip>
#include <iostream>
#include <tuple>

#include "stat_reader.h"

namespace transport_catalogue {

namespace output {
namespace detail {

output::CommandDescription ParseCommandDescription(std::string_view line) {
auto space_pos = line.find(' ');
if (space_pos == line.npos) {
    return {};
}

auto not_space = line.find_first_not_of(' ', space_pos);
if (not_space == line.npos) {
    return {};
}

return { std::string(line.substr(0, space_pos)),
        std::string(line.substr(not_space, line.length() - not_space))/*,
        std::string(line.substr(colon_pos + 1))*/ };
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
}// namespace detail
}// namespace output

void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
    std::ostream& output) {
    using namespace std::literals;
    using namespace output;
    CommandDescription new_request = std::move(detail::ParseCommandDescription(detail::Trim(request)));
    if (new_request.command == "Bus" && tansport_catalogue.FindBus(new_request.id)) {
        std::tuple<size_t, size_t, float> bus_info = std::move(tansport_catalogue.GetBusInfo(new_request.id));
        output << request << ": "s
            << std::get<0>(bus_info) << " stops on route, "s
            << std::get<1>(bus_info) << " unique stops, "s
            << std::setprecision(6) << std::get<2>(bus_info) << " route length\n"s;
    }
    else if (new_request.command == "Stop" && tansport_catalogue.FindStop(new_request.id)) {
        std::set<std::string_view> stop_info = std::move(tansport_catalogue.GetStopInfo(new_request.id));
        if (!stop_info.empty()) {
            output << request << ": buses"s;
            for (const auto& i : stop_info) {
                output << " "s << i;
            }
            output << '\n';
        }
        else {
            output << request << ": no buses\n"s;
        }
    }
    else {
        output << request << ": not found\n"s;
    }
}

}// namespace transport_catalogue