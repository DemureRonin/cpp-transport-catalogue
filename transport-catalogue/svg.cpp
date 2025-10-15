#include "svg.h"
#include <cmath>
#include <sstream>

namespace svg {
    using namespace std::literals;


    inline std::string EscapeText(const std::string &data) {
        std::string result;
        for (char c: data) {
            switch (c) {
                case '"': result += "&quot;";
                    break;
                case '\'': result += "&apos;";
                    break;
                case '<': result += "&lt;";
                    break;
                case '>': result += "&gt;";
                    break;
                case '&': result += "&amp;";
                    break;
                default: result += c;
                    break;
            }
        }
        return result;
    }


    std::ostream &operator<<(std::ostream &out, StrokeLineCap line_cap) {
        switch (line_cap) {
            case StrokeLineCap::BUTT: return out << "butt";
            case StrokeLineCap::ROUND: return out << "round";
            case StrokeLineCap::SQUARE: return out << "square";
        }
        return out;
    }

    std::ostream &operator<<(std::ostream &out, StrokeLineJoin line_join) {
        switch (line_join) {
            case StrokeLineJoin::ARCS: return out << "arcs";
            case StrokeLineJoin::BEVEL: return out << "bevel";
            case StrokeLineJoin::MITER: return out << "miter";
            case StrokeLineJoin::MITER_CLIP: return out << "miter-clip";
            case StrokeLineJoin::ROUND: return out << "round";
        }
        return out;
    }


    void Circle::RenderObject(const RenderContext &context) const {
        auto &out = context.out;
        out << "<circle cx=\"" << center_.x << "\" cy=\"" << center_.y
                << "\" r=\"" << radius_ << "\"";
        this->RenderAttrs(out);
        out << "/>";
    }


    void Polyline::RenderObject(const RenderContext &context) const {
        auto &out = context.out;
        out << "<polyline points=\"";
        for (size_t i = 0; i < points_.size(); ++i) {
            if (i > 0) out << ' ';
            out << points_[i].x << ',' << points_[i].y;
        }
        out << "\"";
        this->RenderAttrs(out);
        out << " />";
    }


    void Text::RenderObject(const RenderContext &context) const {
        auto &out = context.out;
        out << "<text";
        this->RenderAttrs(out);
        out << " x=\"" << pos_.x << "\" y=\"" << pos_.y
                << "\" dx=\"" << offset_.x << "\" dy=\"" << offset_.y
                << "\" font-size=\"" << font_size_ << "\"";
        if (!font_family_.empty()) out << " font-family=\"" << font_family_ << "\"";
        if (!font_weight_.empty()) out << " font-weight=\"" << font_weight_ << "\"";
        out << ">" << EscapeText(data_) << "</text>";
    }


    void Document::Render(std::ostream &out) const {
        out << R"(<?xml version="1.0" encoding="UTF-8" ?>)"sv << std::endl;
        out << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1">)"sv << std::endl;
        RenderContext ctx(out, 2, 2);
        for (const auto &obj: objects_) {
            obj->Render(ctx);
        }
        out << "</svg>"sv;
    }
}
