#pragma once

#include <unordered_map>

#include "domain.h"
#include "geo.h"
#include "svg.h"

namespace renderer {
    
    struct RenderSettings {
        RenderSettings() = default;
        RenderSettings(double width, double height, double padding, double line_width, double stop_radius
            , size_t bus_label_font_size, svg::Point bus_label_offset, size_t stop_label_font_size, svg::Point stop_label_offset
            , svg::Color underlayer_color, double underlayer_width, std::vector<svg::Color> color_palette)
            : width_(width)
            , height_(height)
            , padding_(padding)
            , line_width_(line_width)
            , stop_radius_(stop_radius)
            , bus_label_font_size_(bus_label_font_size)
            , bus_label_offset_(bus_label_offset)
            , stop_label_font_size_(stop_label_font_size)
            , stop_label_offset_(stop_label_offset)
            , underlayer_color_(underlayer_color)
            , underlayer_width_(underlayer_width)
            , color_palette_(color_palette)
        {}

        //ширина и высота изображения в пикселях.Вещественное число в диапазоне от 0 до 100000
        double width_ = 0.;
        double height_ = 0.;

        //отступ краёв карты от границ SVG-документа. Вещественное число не меньше 0 и меньше min(width, height)/2
        double padding_ = 0.;

        //толщина линий, которыми рисуются автобусные маршруты. Вещественное число в диапазоне от 0 до 100000
        double line_width_ = 0.;

        //радиус окружностей, которыми обозначаются остановки. Вещественное число в диапазоне от 0 до 100000
        double stop_radius_ = 0.;
        
        //размер текста, которым написаны названия автобусных маршрутов.Целое число в диапазоне от 0 до 100000.
        size_t bus_label_font_size_ = 0;

        // смещение надписи с названием маршрута относительно координат конечной остановки на карте
        // Массив из двух элементов типа double. Задаёт значения свойств dx и dy SVG-элемента <text>.
        // Элементы массива — числа в диапазоне от –100000 до 100000.
        svg::Point bus_label_offset_ = {0.,0.};

        //размер текста, которым написаны названия автобусных маршрутов. Целое число в диапазоне от 0 до 100000
        size_t stop_label_font_size_ = 0;

        // смещение названия остановки относительно её координат на карте.
        // Массив из двух элементов типа double.Задаёт значения свойств dx и dy SVG - элемента <text>.
        // Числа в диапазоне от –100000 до 100000.
        svg::Point stop_label_offset_ = { 0.,0. };

        //цвет подложки под названиями остановок и маршрутов.Формат хранения цвета будет ниже.
        svg::Color underlayer_color_;


        //толщина подложки под названиями остановок и маршрутов.
        // Задаёт значение атрибута stroke - width элемента <text>.
        // Вещественное число в диапазоне от 0 до 100000.
        double underlayer_width_ = 0.;

        //цветовая палитра.Непустой массив.
        std::vector<svg::Color> color_palette_;
    };

    using namespace information_base;

    class MapRenderer {
    public:
        MapRenderer() = default;

        MapRenderer(RenderSettings render_settings, const std::vector<std::pair<Bus, bool>>&  buses);

        void SetCounter(size_t n);

        size_t GetCounter();

        // сборка маршрута
        svg::Polyline RenderPolyline(const Bus& bus);

        svg::Text RenderRouteUnderlayer(const Stop& stop, std::string name);

        svg::Text RenderRouteName(const Stop& stop, std::string name);

        svg::Text RenderStopUnderlayer(const Stop& stop);

        svg::Text RenderStopName(const Stop& stop);

        svg::Circle RenderStopCircle(const Stop& stop);

        const std::unordered_map<Stop, svg::Point, detail::StopsHasher>* GetStops();

    private:
        RenderSettings render_settings_;

        std::unordered_map<Stop, svg::Point, detail::StopsHasher> stop_sphere_coordinates_;

        size_t counter_ = 0;
    };
}