#include "request_handler.h"


#include <algorithm>
namespace request_handler {

RequestHandler::RequestHandler(transport_catalogue::TransportCatalogue& data_base, renderer::MapRenderer& renderer)
    : data_base_(data_base)
    , map_renderer_(renderer)
{}

std::optional<information_base::BusInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    return data_base_.GetBusInfo(bus_name);
}
   
std::set<const information_base::Bus*> RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
    std::set<const information_base::Bus*> result;
    for (const auto& route : data_base_.GetStopInfo(stop_name)) {
        result.insert(data_base_.FindBus(route));
    }
    return result;
}
    
void RequestHandler::AddRoutes(const std::vector<std::pair<information_base::Bus, bool>>& buses) {
    for (const auto& bus : buses) {
        buses_.insert(bus);
    }
}

void RequestHandler::RenderMap(std::ostream& out) const {
    svg::Document doc;
    // �������
    for (const auto& [bus, is_round] : buses_) {
        // ������� ������ ��������, ������� ���������
        if (GetBusStat(bus.name).value().stops_on_route > 0) {
            doc.Add(map_renderer_.RenderPolyline(bus));
        }
    }
    map_renderer_.SetCounter(0);
    // �������� + �������� ���������
    for (const auto& [bus, is_roundtrip] : buses_) {
        if (GetBusStat(bus.name).value().stops_on_route > 0) { 
            doc.Add(map_renderer_.RenderRouteUnderlayer(*bus.stops.front(), bus.name));
            doc.Add(map_renderer_.RenderRouteName(*bus.stops.front(), bus.name));     
            // �������������� ��������� ��� ������������ ��������
            // ���� �� ��� ����� ��������� ������ ���������
            size_t last_stop_pos = GetBusStat(bus.name).value().stops_on_route / 2;
            if (is_roundtrip == false && bus.stops.front() != bus.stops[last_stop_pos]) {
                map_renderer_.SetCounter(map_renderer_.GetCounter() - 1);
                doc.Add(map_renderer_.RenderRouteUnderlayer(*bus.stops[last_stop_pos], bus.name));
                doc.Add(map_renderer_.RenderRouteName(*bus.stops[last_stop_pos], bus.name));
            }
        }
    }

    // �������� ���������
    using namespace information_base;
    std::vector<const Stop*> stops;
    for (const auto& it : *map_renderer_.GetStops()) {
        stops.push_back(&it.first);
    }
    std::sort(stops.begin(), stops.end(),
        [](const Stop* lhs, const Stop* rhs) {
            return lhs->name < rhs->name;
        });
    for (const auto& stop : stops) {
        if (!GetBusesByStop(stop->name).empty()) {
            doc.Add(map_renderer_.RenderStopCircle(*stop));
        }
    }

    // �������� + �������� ���������
    for (const auto& stop : stops) {
        if (!GetBusesByStop(stop->name).empty()) {
            doc.Add(map_renderer_.RenderStopUnderlayer(*stop));
            doc.Add(map_renderer_.RenderStopName(*stop));
        }
    }
    doc.Render(out);
}

} // request_handler