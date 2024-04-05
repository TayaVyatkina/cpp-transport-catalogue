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
using namespace transport_catalogue;

class JsonReader {
public:

    explicit JsonReader(std::istream& input) 
        : requests_(json::Load(input)) 
    {};

    renderer::RenderSettings LoadMapSettings()const;

    void ConstructBase(TransportCatalogue& catalogue) const;

    void AskBase(TransportCatalogue& catalogue, std::ostream& output) const;

private:
    json::Document requests_;

};
}//namespace json_reader