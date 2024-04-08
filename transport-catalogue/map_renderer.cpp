#include "map_renderer.h"

#include <algorithm>
#include <memory>
#include <vector>
#include <unordered_set>

namespace renderer {

    void MapRenderer::SetCounter(size_t n) {
        counter_ = n;
    }

    size_t MapRenderer::GetCounter() {
       return counter_;
    }

    inline size_t palette_size = 0;

    inline const double EPSILON = 1e-6;

    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    class SphereProjector {
    public:
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding)
            : padding_(padding) //
        {
            // Если точки поверхности сферы не заданы, вычислять нечего
            if (points_begin == points_end) {
                return;
            }

            // Находим точки с минимальной и максимальной долготой
            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // Находим точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // Вычисляем коэффициент масштабирования вдоль координаты x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // Вычисляем коэффициент масштабирования вдоль координаты y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // Коэффициенты масштабирования по ширине и высоте ненулевые,
                // берём минимальный из них
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                // Коэффициент масштабирования по ширине ненулевой, используем его
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                // Коэффициент масштабирования по высоте ненулевой, используем его
                zoom_coeff_ = *height_zoom;
            }
        }

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(geo::Coordinates coords) const {
            return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    using namespace information_base;

    MapRenderer::MapRenderer(RenderSettings render_settings, const std::vector<std::pair<Bus, bool>>& buses)
        : render_settings_(std::move(render_settings))
    {
        palette_size = this->render_settings_.color_palette_.size();
        // Список всех остановок
        std::unordered_set<Stop, information_base::detail::StopsHasher> stops;
        for (const auto& [bus, is_round] : buses) {
            for (const Stop* stop : bus.stops) {
                stops.insert(*stop);
            }
        }

        // Точки, подлежащие проецированию
        std::vector<geo::Coordinates> geo_coords;
        geo_coords.reserve(stops.size());
        for (const Stop& stop : stops) {
            geo_coords.push_back(stop.coordinates);
        }

        // Создаём проектор сферических координат на карту
        SphereProjector sphere_projector{ geo_coords.begin(), geo_coords.end(), render_settings_.width_,
                                      render_settings_.height_, render_settings_.padding_ };
        for (const Stop& stop : stops) {
            stop_sphere_coordinates_[stop] = sphere_projector(stop.coordinates);
        }
    }

    svg::Polyline MapRenderer::RenderPolyline(const Bus& bus) {
        svg::Polyline route;
        route.SetStrokeColor(this->render_settings_.color_palette_[counter_++ % palette_size])
            .SetFillColor(svg::NoneColor)
            .SetStrokeWidth(this->render_settings_.line_width_)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        for (const auto& stop : bus.stops) {
            svg::Point point_on_sphere = stop_sphere_coordinates_.at(*stop);
            route.AddPoint(point_on_sphere);
        }
        return route;
    }

    svg::Text MapRenderer::RenderRouteUnderlayer(const Stop& stop, std::string route_name) {
        svg::Text underlayer;        
        return underlayer
            .SetPosition(this->stop_sphere_coordinates_.at(stop))
            .SetOffset(render_settings_.bus_label_offset_)
            .SetFontSize(render_settings_.bus_label_font_size_)
            .SetFontFamily("Verdana")
            .SetFontWeight("bold")
            .SetData(std::move(route_name))
            .SetFillColor(render_settings_.underlayer_color_)
            .SetStrokeColor(render_settings_.underlayer_color_)
            .SetStrokeWidth(render_settings_.underlayer_width_)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    }

    svg::Text MapRenderer::RenderRouteName(const Stop& stop, std::string name) {
        svg::Text route_name;
        return route_name
            .SetPosition(this->stop_sphere_coordinates_.at(stop))
            .SetOffset(render_settings_.bus_label_offset_)
            .SetFontSize(render_settings_.bus_label_font_size_)
            .SetFontFamily("Verdana")
            .SetFontWeight("bold")
            .SetData(std::move(name))
            .SetFillColor(render_settings_.color_palette_[counter_++ % palette_size]);

    }

    svg::Text MapRenderer::RenderStopUnderlayer(const Stop& stop) {
        svg::Text underlayer;
        return underlayer
            .SetPosition(stop_sphere_coordinates_.at(stop))
            .SetOffset(render_settings_.stop_label_offset_)
            .SetFontSize(render_settings_.stop_label_font_size_)
            .SetFontFamily("Verdana")           
            .SetData(stop.name)
            .SetFillColor(render_settings_.underlayer_color_)
            .SetStrokeColor(render_settings_.underlayer_color_)
            .SetStrokeWidth(render_settings_.underlayer_width_)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    }

    svg::Text MapRenderer::RenderStopName(const Stop& stop) {
        svg::Text route_name;
        return route_name
            .SetPosition(stop_sphere_coordinates_.at(stop))
            .SetOffset(render_settings_.stop_label_offset_)
            .SetFontSize(render_settings_.stop_label_font_size_)
            .SetFontFamily("Verdana")
            .SetData(stop.name)
            .SetFillColor("black");
    }

    const std::unordered_map<Stop, svg::Point, detail::StopsHasher>* MapRenderer::GetStops() {
        return &stop_sphere_coordinates_;
    }

    svg::Circle MapRenderer::RenderStopCircle(const Stop& stop) {
        svg::Circle dot;
        return dot
            .SetCenter(stop_sphere_coordinates_.at(stop))
            .SetRadius(render_settings_.stop_radius_)
            .SetFillColor("white");
    }
}