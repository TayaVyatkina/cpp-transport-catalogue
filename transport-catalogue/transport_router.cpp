#include "transport_router.h"


namespace graph {

    TransportRouter::TransportRouter(const transport_catalogue::TransportCatalogue& catalogue, const Settings& settings) 
        : catalogue_(catalogue)
        , settings_(settings) {
        AddVertexes();
        graph_ = graph::DirectedWeightedGraph<double>(GetVertexesCount());
        AddWaitEdges();
        AddBusEdges();
        router_ = std::make_unique<Router<double>>(graph_);
    }

    size_t TransportRouter::GetVertexesCount() const{
        return vertexes_count_;
    }

    void TransportRouter::AddVertexes() {
        for (const auto& stop : catalogue_.GetStops()) {
            vertexes[stop.name].in = vertexes_count_++;
            vertexes[stop.name].out = vertexes_count_++;
        }    
    }
    
    void TransportRouter::AddWaitEdges() {
        for (const auto& stop : catalogue_.GetStops()) {
            wait_edges_.insert({
                graph_.AddEdge({
                    vertexes.at(stop.name).in,
                    vertexes.at(stop.name).out,
                    static_cast<double>(settings_.bus_wait_time)
                }),
                {
                    static_cast<double>(settings_.bus_wait_time),
                    stop.name
                }
                });
        }
    }

     void TransportRouter::AddBusEdges() {
        for (const auto& bus : catalogue_.GetBuses()) {
            AddEdgesGraph(bus.stops.begin(), bus.stops.end(), bus.name);

            if (!(bus.last_stops.size() == 1)) {
                AddEdgesGraph(bus.stops.rbegin(), bus.stops.rend(), bus.name);
            }
        }
    }

    double TransportRouter::CalcTimeBetweenStops(const Stop* from, const Stop* to) const {
        const double kilometer = 1000.;
        const double hour = 60.;
        return hour * catalogue_.GetDistanceBetweenStops(from, to) 
            / (kilometer * settings_.bus_velocity);
    }     

    std::optional<ReportRouter> TransportRouter::GetReportRouter(const std::string_view& from, const std::string_view& to) const {
        std::optional<Router<double>::RouteInfo> route_info = router_->BuildRoute(vertexes.at(std::string{ from }).in, vertexes.at(std::string{ to }).in);
        if (!route_info) {
            return {};
        }
        ReportRouter output;
        output.total_minutes = route_info->weight;
        auto& items = output.information;
        for (const auto& edge : route_info->edges) {
            RouteInfo info;
            if (ride_edges_.count(edge) > 0) {
                info.bus = ride_edges_.at(edge);
            }
            else if (wait_edges_.count(edge) > 0) {
                info.wait = wait_edges_.at(edge);
            }
            else {
                assert(false);
            }
            items.push_back(info);
        }
        return { std::move(output) };
    }

}