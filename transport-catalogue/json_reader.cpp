#include "json_reader.h"
#include "json_builder.h"
#include "request_handler.h"
#include "svg.h"
#include "sstream"

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
        // внесение обратного пути для некольцевого маршрута
        if (object.at("is_roundtrip").AsBool() == false) {
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
        catalogue.AddBus(Bus{ object.at("name").AsString(), std::move(obj_stops) });
    }
}
}

void JsonReader::AskBase(TransportCatalogue& catalogue, std::ostream& output) const {
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

        renderer::MapRenderer map_rend(LoadMapSettings(), buses);// подгрузили из json настройки для карты + список остановок 
        request_handler::RequestHandler request_handler(catalogue, map_rend); // агрегация бд и "чертёжника"
        request_handler.AddRoutes(buses);//  подгрузили список маршрутов
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