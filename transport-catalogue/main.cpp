#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"
#include "Test_module.cpp"
using namespace std;
using namespace transport_catalogue;
void TransportCatalogueTest() {
    TransportCatalogue catalogue;

    std::vector<std::string> abc{
        "Stop Tolstopaltsevo : 55.611087, 37.208290",
            "Stop Marushkino : 55.595884, 37.209755",
            "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye",
            "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka",
            "Stop Rasskazovka : 55.632761, 37.333324",
            "Stop Biryulyovo Zapadnoye : 55.574371, 37.651700",
            "Stop Biryusinka : 55.581065, 37.648390",
            "Stop Universam : 55.587655, 37.645687",
            "Stop Biryulyovo Tovarnaya : 55.592028, 37.653656",
            "Stop Biryulyovo Passazhirskaya : 55.580999, 37.659164"
    };

    InputReader reader;
    for (auto line : abc) {
        reader.ParseLine(line);
    }
    reader.ApplyCommands(catalogue);
    Stop str{ "Biryulyovo Tovarnaya", Coordinates{55.592028, 37.653656} };
    assert((catalogue.FindStop("Biryulyovo Tovarnaya")->name) == str.name);
    assert((catalogue.FindStop("Biryulyovo Tovarnaya")->coordinates) == str.coordinates);

    Stop str2{ "Tolstopaltsevo", Coordinates{55.611087, 37.208290} };
    assert((catalogue.FindStop("Tolstopaltsevo")->name) == str2.name);
    assert((catalogue.FindStop("Tolstopaltsevo")->coordinates) == str2.coordinates);

    assert(catalogue.FindBus("750")->name == "750");
    assert(catalogue.FindBus("750")->stops.size() == 5);
    std::vector<Stop> a{ {"Tolstopaltsevo", Coordinates{55.611087, 37.208290} }, {"Marushkino", Coordinates{55.595884, 37.209755}}, {"Rasskazovka", Coordinates{55.632761, 37.333324}} };
    for (auto i = 0; i < 3; ++i) {
        assert(catalogue.FindBus("750")->stops[i]->name == a[i].name);
        assert(catalogue.FindBus("750")->stops[i]->coordinates == a[i].coordinates);
    }

    //requests
    string line = "Bus 256"s;
    ParseAndPrintStat(catalogue, line, std::cout);
}

int main() {
    //TransportCatalogueTest();

    TransportCatalogue catalogue;

    int base_request_count;
    cin >> base_request_count >> ws;

    {
        InputReader reader;
        for (int i = 0; i < base_request_count; ++i) {
            string line;
            getline(cin, line);
            reader.ParseLine(line);
        }
        reader.ApplyCommands(catalogue);
    }

    int stat_request_count;
    cin >> stat_request_count >> ws;
    for (int i = 0; i < stat_request_count; ++i) {
        string line;
        getline(cin, line);
        ParseAndPrintStat(catalogue, line, cout);
    }
}
