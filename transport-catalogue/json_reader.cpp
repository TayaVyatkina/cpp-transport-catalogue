#include "graph.h"
#include "json_reader.h"
#include "json_builder.h"
#include "router.h"
#include "svg.h"
#include "sstream"

#include "map_renderer.h"
#include "request_handler.h"
#include "transport_router.h"

namespace json_reader {
using namespace std::literals;
using namespace transport_catalogue;
using namespace json;
using namespace information_base;

svg::Color ReadColor(const json::Node& json) {
    if (json.IsArray()) {
        const auto& arr = json.AsArray();
        if (arr.size() == 3) {  // Rgb
            return svg::Rgb(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt());
        }
        else if (arr.size() == 4) {  // Rgba
            return svg::Rgba(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt(), arr[3].AsDouble());
        }
    }
    else if (json.IsString()) {
        return json.AsString();
    }
    else if (json.IsNull()) {
        return svg::NoneColor;
    }
    return {};
}

svg::Point ReadPoint(const json::Array& json) {
    return svg::Point{ json[0].AsDouble(), json[1].AsDouble() };
}

renderer::RenderSettings JsonReader::LoadMapSettings() const {
    const auto& settings = requests_.GetRoot().AsMap().at("render_settings").AsMap();
    std::vector<svg::Color> color_palette;
    for (const auto& color : settings.at("color_palette").AsArray()) {
        color_palette.push_back(ReadColor(color));
    }
    return renderer::RenderSettings(
        settings.at("width").AsDouble()
        , settings.at("height").AsDouble()
        , settings.at("padding").AsDouble()
        , settings.at("line_width").AsDouble()
        , settings.at("stop_radius").AsDouble()
        , settings.at("bus_label_font_size").AsInt()
        , ReadPoint(settings.at("bus_label_offset").AsArray())
        , settings.at("stop_label_font_size").AsInt()
        , ReadPoint(settings.at("stop_label_offset").AsArray())
        , ReadColor(settings.at("underlayer_color"))
        , settings.at("underlayer_width").AsDouble()
        , color_palette
    );
}

void JsonReader::ConstructBase(TransportCatalogue& catalogue) const {
const auto& base_request = requests_.GetRoot().AsMap().at("base_requests");
// формирование базы остановок 
for (const auto& request : base_request.AsArray()) {
    const auto& object = request.AsMap();
    if (object.at("type").AsString() == "Stop") {
        catalogue.AddStop(Stop{ object.at("name").AsString(),
                                object.at("latitude").AsDouble(),
                                object.at("longitude").AsDouble() });
    }
}
// внесение расстояний между остановками
for (const auto& request : base_request.AsArray()) {
    const auto& object = request.AsMap();
    if (object.at("type").AsString() == "Stop" && object.count("road_distances")) {
        for (const auto& [other_stop_name, dist] : object.at("road_distances").AsMap()) {
            catalogue.AddDistanceBetweenStops(object.at("name").AsString(), other_stop_name, dist.AsInt());
        }
    }
}
// формирование базы автобусов
for (auto& request : base_request.AsArray()) {
    auto& object = request.AsMap();
    if (object.at("type").AsString() == "Bus") {
        std::vector<const Stop*> obj_stops;
        for (const auto& stop : object.at("stops").AsArray()) {
            obj_stops.push_back(catalogue.FindStop(stop.AsString()));
        }
        std::vector<const Stop*> last_stops;
        // внесение обратного пути для некольцевого маршрута
        if (object.at("is_roundtrip").AsBool() == false) {
            last_stops.push_back(obj_stops[0]);
            last_stops.push_back(obj_stops[obj_stops.size() - 1]);
            if (last_stops.size() == 2 && last_stops[0] == last_stops[1]) {
                last_stops.pop_back();
            }
            // пропустить конечную 
            bool flag = false;
            for (auto rit = object.at("stops").AsArray().rbegin(); rit != object.at("stops").AsArray().rend(); ++rit) {
                if (flag) {
                    obj_stops.push_back(catalogue.FindStop((*rit).AsString()));
                }
                else {
                    flag = true;
                }
            }
        }
        else {
            last_stops.push_back(obj_stops[0]);
        }
        catalogue.AddBus(Bus{ object.at("name").AsString(), std::move(obj_stops), std::move(last_stops) });
    }
}
}

void JsonReader::AskBase(TransportCatalogue& catalogue, std::ostream& output) const {
request_handler::RequestHandler request_handler(&catalogue);
renderer::MapRenderer map_rend(LoadMapSettings());
request_handler.AddMapRenderer(&map_rend);

const auto& routing_settings = requests_.GetRoot().AsMap().at("routing_settings");
const graph::Settings settings(routing_settings.AsMap().at("bus_wait_time").AsInt(), routing_settings.AsMap().at("bus_velocity").AsDouble());
graph::TransportRouter transport_router(catalogue, settings);
request_handler.AddTransportRouter(&transport_router);

const auto& stat_request = requests_.GetRoot().AsMap().at("stat_requests");
// формирование ответов на запросы к базе данных
Array answers;
for (const auto& request : stat_request.AsArray()) {
    const auto& object = request.AsMap();
    // запрос информации об автобусе
    if (object.at("type").AsString() == "Bus" && catalogue.FindBus(object.at("name").AsString())) {
        std::string_view bus_name = object.at("name").AsString();
        std::optional<information_base::BusInfo> bus_info = catalogue.GetBusInfo(bus_name).has_value() ?
            catalogue.GetBusInfo(bus_name).value()
            : information_base::BusInfo{};
        answers.emplace_back(
            json::Builder{}
            .StartDict()
            .Key("curvature").Value(bus_info.value().curvature)
            .Key("request_id").Value(object.at("id").AsInt())
            .Key("route_length").Value(bus_info.value().route_length)
            .Key("stop_count").Value(static_cast<int>(bus_info.value().stops_on_route))
            .Key("unique_stop_count").Value(static_cast<int>(bus_info.value().unique_stops))
            .EndDict()
            .Build()
        );
    }
    // запрос информации об остановке
    else if (object.at("type").AsString() == "Stop" && catalogue.FindStop(object.at("name").AsString())) {
        std::string_view stop_name = object.at("name").AsString();
        std::set<std::string_view> stop_info = catalogue.GetStopInfo(stop_name);
        Builder builder;
            builder
                .StartDict()
                .Key("buses").StartArray();

            for (std::string_view stop : stop_info) {
                builder.Value(std::move(std::string{ stop.data(), stop.size() }));
            }
            builder
                .EndArray()
                .Key("request_id").Value(object.at("id").AsInt())
                .EndDict();
            
        answers.emplace_back(std::move(builder.Build()));

    }
    // запрос на построение карты
    else if (object.at("type").AsString() == "Map") {
        const auto& base_request = requests_.GetRoot().AsMap().at("base_requests");
        std::vector<std::pair<information_base::Bus, bool>>  buses;
        for (auto& request : base_request.AsArray()) {
            auto& object = request.AsMap();
            if (object.at("type").AsString() == "Bus") {
                buses.push_back({ *catalogue.FindBus(object.at("name").AsString())
                                 , object.at("is_roundtrip").AsBool() });
            }
        }

        map_rend.AddRoutes(buses);
        request_handler.AddRoutes(buses);
        std::ostringstream out;
        request_handler.RenderMap(out);

        answers.emplace_back(
            json::Builder{}
            .StartDict()
            .Key("map").Value(out.str())
            .Key("request_id").Value(object.at("id").AsInt())
            .EndDict()
            .Build()
        );
    }
    // запрос на поиск кратчайшего пути
    else if (object.at("type").AsString() == "Route" 
        && catalogue.FindStop(object.at("from").AsString()) 
        && catalogue.FindStop(object.at("to").AsString())) {
        Builder builder;
        const auto& report = request_handler.GetReportRouter(object.at("from").AsString(), object.at("to").AsString());
            if (report.has_value()) {
                builder
                    .StartDict()
                    .Key("request_id").Value(object.at("id").AsInt())
                    .Key("total_time"s).Value(report.value().total_minutes)
                    .Key("items"s).StartArray();

                for (const auto& info : report.value().information) {
                    if (!info.wait.stop_name.empty()) {
                        builder.StartDict()
                            .Key("type"s).Value("Wait"s)
                            .Key("time"s).Value(info.wait.minutes)
                            .Key("stop_name"s).Value(info.wait.stop_name)
                            .EndDict();
                    }
                    if (!info.bus.number.empty()) {
                        builder.StartDict()
                            .Key("type"s).Value("Bus"s)
                            .Key("time"s).Value(info.bus.minutes)
                            .Key("bus"s).Value(std::string(info.bus.number))
                            .Key("span_count"s).Value(info.bus.span_count)
                            .EndDict();
                    }
                }

                builder
                    .EndArray()
                    .EndDict();
                answers.emplace_back(std::move(builder.Build()));
            }
            else {
                answers.emplace_back(
                    json::Builder{}
                    .StartDict()
                    .Key("request_id").Value(object.at("id").AsInt())
                    .Key("error_message"s).Value("not found"s)
                    .EndDict()
                    .Build()
                );
            }
         }
    // прочие запросы
    else {
        answers.emplace_back(
            json::Builder{}
            .StartDict()
            .Key("request_id").Value(object.at("id").AsInt())
            .Key("error_message"s).Value("not found"s)
            .EndDict()
            .Build()
        );
    }
}

Print(Document{ answers }, output);
}

} // namespace json_reader