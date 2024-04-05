#pragma once
#include "domain.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

#include <set>


// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
namespace request_handler {

class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(transport_catalogue::TransportCatalogue& data_base, renderer::MapRenderer& renderer);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<information_base::BusInfo> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через остановку 
    std::set<const information_base::Bus*> GetBusesByStop(const std::string_view& stop_name) const;

    // Добавляет маршруты и их типы (кольцевой/некольцевой)
    void AddRoutes(const std::vector<std::pair<information_base::Bus, bool>>& buses);

    // Этот метод будет нужен в следующей части итогового проекта
    void RenderMap(std::ostream& out) const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    transport_catalogue::TransportCatalogue& data_base_;
    renderer::MapRenderer& map_renderer_;
    std::set<std::pair<information_base::Bus, bool>, information_base::detail::PairsHasher> buses_;
};
} //namespace request_handler
