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
/*json_reader_(cin);// прочитали базу + запросы к базе + настройки карты
json_reader_.ConstructBase(catalogue); // сформировали бд
renderer::MapRenderer map_rend(json_reader_.LoadMapSettings(), json_reader_.GetRoutes(catalogue));// подгрузили из json настройки для карты + список остановок 
request_handler::RequestHandler request_handler(catalogue, map_rend); // агрегация бд и "чертёжника"
request_handler.AddRoutes(json_reader_.GetRoutes(catalogue));//  подгрузили список маршрутов
request_handler.RenderMap(cout);*/
    json_reader::JsonReader json_reader_(cin);// прочитали базу + запросы к базе + настройки карты
json_reader_.ConstructBase(catalogue); // сформировали бд



json_reader_.AskBase(catalogue, cout);

}