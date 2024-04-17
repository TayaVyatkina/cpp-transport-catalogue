#include <iostream>
#include <string>
#include <fstream>

#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"


using namespace std;
using namespace transport_catalogue;


int main() {
    TransportCatalogue catalogue;

    json_reader::JsonReader json_reader_(cin);// прочитали базу + запросы к базе + настройки карты
json_reader_.ConstructBase(catalogue); // сформировали бд



json_reader_.AskBase(catalogue, cout);

}