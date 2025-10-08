#pragma once

#include <sstream>
#include <algorithm>
#include <optional>
#include <unordered_set>
#include <vector>
#include <string>

#include "transport_catalogue.h"
#include "svg.h"

namespace transport_catalogue::renderer {
    inline constexpr double EPSILON = 1e-6;

    inline bool IsZero(const double value) {
        return std::abs(value) < EPSILON;
    }


    class SphereProjector {
    public:
        template<typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                        const double max_width, const double max_height, const double padding)
            : padding_(padding) {
            if (points_begin == points_end) return;

            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            } else if (width_zoom) {
                zoom_coeff_ = *width_zoom;
            } else if (height_zoom) {
                zoom_coeff_ = *height_zoom;
            }
        }

        svg::Point operator()(const geo::Coordinates coords) const {
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

    struct RenderSettings {
        double width = 600.0;
        double height = 400.0;
        double padding = 50.0;
        double stop_radius = 5.0;
        double line_width = 14.0;

        uint32_t bus_label_font_size = 20;
        svg::Point bus_label_offset{7, 15};

        uint32_t stop_label_font_size = 20;
        svg::Point stop_label_offset{7, -3};

        svg::Color underlayer_color = "white";
        double underlayer_width = 3.0;

        std::vector<std::string> color_palette{"green", "orange", "red"};
    };

    class MapRenderer {
    public:
        MapRenderer(const TransportCatalogue &catalogue, const RenderSettings &settings);

        [[nodiscard]] svg::Document Render() const;

    private:
        const TransportCatalogue &catalogue_;
        const RenderSettings &settings_;

        void AddBusLabel(svg::Document &doc, std::string_view bus_name, svg::Point pos, size_t color_index) const;

        [[nodiscard]] std::unordered_set<const  domain::Stop *> CollectUniqueStops() const;

        [[nodiscard]] SphereProjector CreateProjector(const std::unordered_set<const  domain::Stop *> &stops) const;

        void DrawBusLines(svg::Document &doc, const SphereProjector &projector) const;

        void DrawBusLabels(svg::Document &doc, const SphereProjector &projector) const;

        void DrawStopCircles(svg::Document &doc,
                             const std::unordered_set<const  domain::Stop *> &unique_stops,
                             const SphereProjector &projector) const;

        void DrawStopLabels(svg::Document &doc,
                            const std::unordered_set<const  domain::Stop *> &unique_stops,
                            const SphereProjector &projector) const;
    };
}
