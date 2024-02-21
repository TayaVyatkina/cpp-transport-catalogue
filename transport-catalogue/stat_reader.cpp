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
    CommandDescription new_request = output::detail::ParseCommandDescription(output::detail::Trim(request));
    if (new_request.command == "Bus" && tansport_catalogue.FindBus(new_request.id)) {
        std::optional<BusInfo> bus_info = tansport_catalogue.GetBusInfo(new_request.id).has_value() ?
            tansport_catalogue.GetBusInfo(new_request.id).value()
            : BusInfo{};
        output << request << ": "s
            << bus_info.value().stops_on_route << " stops on route, "s
            << bus_info.value().unique_stops << " unique stops, "s
            << bus_info.value().route_length << " route length, "s
            << std::setprecision(6) << bus_info.value().curvature << " curvature\n"s;
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