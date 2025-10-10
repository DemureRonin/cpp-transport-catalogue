#include "json_reader.h"
#include <sstream>

namespace transport_catalogue::readers {
    std::string JsonReader::NodeToColor(const json::Node &node) {
        if (node.IsString()) {
            return node.AsString();
        } else if (node.IsArray()) {
            const auto &arr = node.AsArray();
            std::ostringstream ss;
            if (arr.size() == 3) {
                ss << "rgb(" << arr[0].AsInt() << "," << arr[1].AsInt() << "," << arr[2].AsInt() << ")";
            } else if (arr.size() == 4) {
                ss << "rgba(" << arr[0].AsInt() << "," << arr[1].AsInt() << "," << arr[2].AsInt() << "," << arr[3].AsDouble() << ")";
            }
            return ss.str();
        }
        throw std::logic_error("Invalid color node");
    }

    JsonReader::JsonReader(TransportCatalogue &catalogue) : catalogue_(catalogue) {
    }

    void JsonReader::Load(std::istream &input) {
        const json::Document doc = json::Load(input);
        const auto &root = doc.GetRoot().AsMap();

        if (root.contains("base_requests")) {
            ParseBaseRequests(root.at("base_requests"));
        }
        if (root.contains("render_settings")) {
            ParseRenderSettings(root.at("render_settings"));
        }
        if (root.contains("stat_requests")) {
            stat_requests_ = root.at("stat_requests").AsArray();
        }
    }

    void JsonReader::ApplyCommands() const {

        for (const auto &cmd: commands_) {
            if (std::holds_alternative<StopCommand>(cmd)) {
                const auto &stop = std::get<StopCommand>(cmd);
                catalogue_.AddStop(stop.id, {stop.latitude, stop.longitude});
            }
        }

        for (const auto &cmd: commands_) {
            if (std::holds_alternative<StopCommand>(cmd)) {
                const auto &stop = std::get<StopCommand>(cmd);
                for (const auto &[other_stop, dist]: stop.distances) {
                    catalogue_.SetDistance(stop.id, other_stop, dist);
                }
            }
        }

        for (const auto &cmd: commands_) {
            if (std::holds_alternative<BusCommand>(cmd)) {
                const auto &bus = std::get<BusCommand>(cmd);
                std::vector<std::string_view> stops_view;
                stops_view.reserve(bus.stops.size());
                for (const auto &s: bus.stops) {
                    stops_view.push_back(s);
                }
                catalogue_.AddBus(bus.id, stops_view, bus.is_roundtrip);
            }
        }
    }


    const std::vector<json::Node> &JsonReader::GetStatRequests() const {
        return stat_requests_;
    }

    const renderer::RenderSettings &JsonReader::GetMapSettings() const {
        return map_settings_;
    }

    void JsonReader::ParseBaseRequests(const json::Node &base_requests_node) {
        for (const auto &item: base_requests_node.AsArray()) {
            const auto &m = item.AsMap();
            const std::string type = m.at("type").AsString();

            if (type == "Stop") {
                StopCommand cmd;
                cmd.id = m.at("name").AsString();
                cmd.latitude = m.at("latitude").AsDouble();
                cmd.longitude = m.at("longitude").AsDouble();
                if (m.contains("road_distances")) {
                    for (const auto &[stop_name, dist_node]: m.at("road_distances").AsMap()) {
                        cmd.distances.emplace_back(stop_name, dist_node.AsInt());
                    }
                }
                commands_.push_back(std::move(cmd));
            } else if (type == "Bus") {
                BusCommand cmd;
                cmd.id = m.at("name").AsString();
                const auto &stops_array = m.at("stops").AsArray();
                for (const auto &stop_node: stops_array) {
                    cmd.stops.push_back(stop_node.AsString());
                }
                cmd.is_roundtrip = m.at("is_roundtrip").AsBool();
                commands_.push_back(std::move(cmd));
            }
        }
    }

    void JsonReader::ParseRenderSettings(const json::Node &node) {
        const auto &m = node.AsMap();

        map_settings_.width = m.at("width").AsDouble();
        map_settings_.height = m.at("height").AsDouble();
        map_settings_.padding = m.at("padding").AsDouble();
        map_settings_.stop_radius = m.at("stop_radius").AsDouble();
        map_settings_.line_width = m.at("line_width").AsDouble();

        map_settings_.bus_label_font_size = m.at("bus_label_font_size").AsInt();
        const auto &bus_offset = m.at("bus_label_offset").AsArray();
        map_settings_.bus_label_offset = {bus_offset[0].AsDouble(), bus_offset[1].AsDouble()};

        map_settings_.stop_label_font_size = m.at("stop_label_font_size").AsInt();
        const auto &stop_offset = m.at("stop_label_offset").AsArray();
        map_settings_.stop_label_offset = {stop_offset[0].AsDouble(), stop_offset[1].AsDouble()};

        map_settings_.underlayer_color = NodeToColor(m.at("underlayer_color"));
        map_settings_.underlayer_width = m.at("underlayer_width").AsDouble();

        map_settings_.color_palette.clear();
        for (const auto &c: m.at("color_palette").AsArray()) {
            map_settings_.color_palette.push_back(NodeToColor(c));
        }
    }
}
