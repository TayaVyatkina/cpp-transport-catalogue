#pragma once
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <utility>

#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

namespace json_reader{

class JsonReader {
public:

    explicit JsonReader(std::istream& input) 
        : requests_(json::Load(input)) 
    {};

    renderer::RenderSettings LoadMapSettings()const;

    void ConstructBase(transport_catalogue::TransportCatalogue& catalogue) const;

    void AskBase(transport_catalogue::TransportCatalogue& catalogue, std::ostream& output) const;

private:
    json::Document requests_;

};
}//namespace json_reader