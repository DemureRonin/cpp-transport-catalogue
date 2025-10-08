#include "map_renderer.h"

#include <set>

namespace transport_catalogue::renderer {
    MapRenderer::MapRenderer(const TransportCatalogue &catalogue, const RenderSettings &settings)
        : catalogue_(catalogue), settings_(settings) {
    }

    svg::Document MapRenderer::Render() const {
        svg::Document doc;

        const auto unique_stops = CollectUniqueStops();

        const auto projector = CreateProjector(unique_stops);

        DrawBusLines(doc, projector);

        DrawBusLabels(doc, projector);

        DrawStopCircles(doc, unique_stops, projector);

        DrawStopLabels(doc, unique_stops, projector);

        return doc;
    }

    std::unordered_set<const domain::Stop *> MapRenderer::CollectUniqueStops() const {
        std::unordered_set<const domain::Stop *> unique_stops;

        for (const auto &bus_name: catalogue_.GetAllBusNames()) {
            if (const auto *bus = catalogue_.FindBus(bus_name)) {
                unique_stops.insert(bus->stops.begin(), bus->stops.end());
            }
        }
        return unique_stops;
    }


    SphereProjector MapRenderer::CreateProjector(const std::unordered_set<const domain::Stop *> &stops) const {
        std::vector<geo::Coordinates> coords;
        coords.reserve(stops.size());

        for (const auto *stop: stops) {
            coords.push_back(stop->coordinates);
        }

        return {
            coords.begin(), coords.end(),
            settings_.width, settings_.height, settings_.padding
        };
    }

    void MapRenderer::DrawBusLines(svg::Document &doc, const SphereProjector &projector) const {
        auto all_buses = catalogue_.GetAllBusNames();
        std::sort(all_buses.begin(), all_buses.end());

        size_t color_index = 0;
        for (const auto &bus_name: all_buses) {
            const auto *bus = catalogue_.FindBus(bus_name);
            if (!bus || bus->stops.empty()) continue;

            svg::Polyline line;
            line.SetStrokeColor(settings_.color_palette[color_index % settings_.color_palette.size()])
                    .SetStrokeWidth(settings_.line_width)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                    .SetFillColor(svg::NoneColor);

            for (const auto *stop: bus->stops) {
                line.AddPoint(projector(stop->coordinates));
            }

            doc.Add(line);
            ++color_index;
        }
    }

    void MapRenderer::DrawBusLabels(svg::Document &doc, const SphereProjector &projector) const {
        auto all_buses = catalogue_.GetAllBusNames();
        std::sort(all_buses.begin(), all_buses.end());

        size_t color_index = 0;
        for (const auto &bus_name: all_buses) {
            const auto *bus = catalogue_.FindBus(bus_name);
            if (!bus || bus->stops.empty()) continue;

            std::vector<const domain::Stop *> end_stops = {bus->stops.front()};

            if (!bus->is_roundtrip) {
                const size_t half = (bus->stops.size() + 1) / 2;
                const domain::Stop *last_direct_stop = bus->stops[half - 1];
                if (bus->stops.front()->name != last_direct_stop->name) {
                    end_stops.push_back(last_direct_stop);
                }
            }

            for (const auto *stop: end_stops) {
                AddBusLabel(doc, bus->name, projector(stop->coordinates), color_index);
            }

            ++color_index;
        }
    }

    void MapRenderer::DrawStopCircles(svg::Document &doc,
                                      const std::unordered_set<const domain::Stop *> &unique_stops,
                                      const SphereProjector &projector) const {
        std::set<std::string> stop_names;
        for (const auto *stop: unique_stops) {
            stop_names.insert(std::string(stop->name));
        }

        for (const auto &name: stop_names) {
            const domain::Stop *stop = catalogue_.FindStop(name);
            if (!stop) continue;

            svg::Circle circle;
            circle.SetCenter(projector(stop->coordinates))
                    .SetRadius(settings_.stop_radius)
                    .SetFillColor("white");
            doc.Add(circle);
        }
    }

    void MapRenderer::DrawStopLabels(svg::Document &doc,
                                     const std::unordered_set<const domain::Stop *> &unique_stops,
                                     const SphereProjector &projector) const {
        std::set<std::string> stop_names;
        for (const auto *stop: unique_stops) {
            stop_names.insert(std::string(stop->name));
        }

        for (const auto &name: stop_names) {
            const domain::Stop *stop = catalogue_.FindStop(name);
            if (!stop) continue;

            const svg::Point pos = projector(stop->coordinates);


            svg::Text underlayer;
            underlayer.SetPosition(pos)
                    .SetOffset(settings_.stop_label_offset)
                    .SetFontSize(settings_.stop_label_font_size)
                    .SetFontFamily("Verdana")
                    .SetFillColor(settings_.underlayer_color)
                    .SetStrokeColor(settings_.underlayer_color)
                    .SetStrokeWidth(settings_.underlayer_width)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                    .SetData(std::string(stop->name));
            doc.Add(underlayer);


            svg::Text text;
            text.SetPosition(pos)
                    .SetOffset(settings_.stop_label_offset)
                    .SetFontSize(settings_.stop_label_font_size)
                    .SetFontFamily("Verdana")
                    .SetFillColor("black")
                    .SetData(std::string(stop->name));
            doc.Add(text);
        }
    }


    void MapRenderer::AddBusLabel(svg::Document &doc, std::string_view bus_name, svg::Point pos, size_t color_index) const {
        svg::Text underlayer;
        underlayer.SetPosition(pos)
                .SetOffset(settings_.bus_label_offset)
                .SetFontSize(settings_.bus_label_font_size)
                .SetFontFamily("Verdana")
                .SetFontWeight("bold")
                .SetFillColor(settings_.underlayer_color)
                .SetStrokeColor(settings_.underlayer_color)
                .SetStrokeWidth(settings_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                .SetData(std::string(bus_name));
        doc.Add(underlayer);

        svg::Text text;
        text.SetPosition(pos)
                .SetOffset(settings_.bus_label_offset)
                .SetFontSize(settings_.bus_label_font_size)
                .SetFontFamily("Verdana")
                .SetFontWeight("bold")
                .SetFillColor(settings_.color_palette[color_index % settings_.color_palette.size()])
                .SetData(std::string(bus_name));
        doc.Add(text);
    }
}
