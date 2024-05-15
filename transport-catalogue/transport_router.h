#pragma once

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include "domain.h"
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

namespace graph {
    using namespace information_base;

    struct VertexIds {
        VertexId in{};
        VertexId out{};
    };

    class TransportRouter {
    public:
        TransportRouter(const transport_catalogue::TransportCatalogue& catalogue, const Settings& settings);

        std::optional<ReportRouter> GetReportRouter(const std::string_view& from, const std::string_view& to) const;
        
    private:
        const transport_catalogue::TransportCatalogue& catalogue_;
        Settings settings_;
        DirectedWeightedGraph<double> graph_;
        VertexId vertexes_count_ = 0;
        std::unordered_map<std::string, VertexIds> vertexes;
        std::unordered_map<EdgeId, RouteInfo::Wait> wait_edges_;
        std::unique_ptr<Router<double>> router_;
        std::unordered_map<EdgeId, RouteInfo::Bus> ride_edges_;

        size_t GetVertexesCount() const;
        void AddVertexes();
        void AddWaitEdges();
        void AddBusEdges();
        double CalcTimeBetweenStops(const Stop* from, const Stop* to) const;

        template<typename Iter>
        void AddEdgesGraph(Iter begin, Iter end, const std::string name) {
            for (auto from = begin; from != end; ++from) {
                double weight{};
                int span_count{};
                for (auto to = std::next(from); to != end; ++to) {
                    std::string departure_name = (*from)->name;
                    VertexId departure = vertexes.at(departure_name).out;
                    std::string arrival_name = (*to)->name;
                    VertexId arrival = vertexes.at(arrival_name).in;
                    weight += CalcTimeBetweenStops(*prev(to), *(to));
                    ++span_count;
                    auto bus_edge_id = graph_.AddEdge({ departure, arrival, weight });
                    ride_edges_[bus_edge_id] = { name , span_count, weight };
                }
            }
        }
    };
}