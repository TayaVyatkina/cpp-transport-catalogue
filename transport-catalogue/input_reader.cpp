#include <algorithm>
#include <iterator>
#include <utility>

#include "input_reader.h"

namespace transport_catalogue {
namespace detail {

/**
* Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
*/
Coordinates ParseCoordinates(std::string_view str) {
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

input::CommandDescription ParseCommandDescription(std::string_view line) {
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

}// namespace detail

void InputReader::ParseLine(std::string_view line) {
    auto command_description = detail::ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue) const {
    using namespace detail;
    std::vector<input::CommandDescription> commands_buses;
    for (const auto& obj : commands_) {
        if (obj.command == "Stop") {
            Coordinates coordinates = ParseCoordinates(Trim(obj.description));
            catalogue.AddStop(std::move(Stop{ static_cast<std::string>(Trim(obj.id)), coordinates }));
        }
        else {
            commands_buses.push_back(std::move(obj));
        }
    }
    for (const auto& obj : commands_buses) {
        if (obj.command == "Bus") {
            std::vector<Stop*> stops_str;
            for (const auto& stop : ParseRoute(Trim(obj.description))) {
                stops_str.push_back(std::move(catalogue.FindStop(stop)));
            }
            catalogue.AddBus(std::move(Bus{ static_cast<std::string>(Trim(obj.id)), stops_str }));
        }
    }
}

}// namespace transport_catalogue