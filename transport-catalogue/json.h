#pragma once
#include <variant>
#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <iostream>
#include <type_traits>

namespace json {
    using namespace std;

    class Node;

    using Dict = map<string, Node>;
    using Array = vector<Node>;

    class Node {
    public:
        using Value = variant<nullptr_t, int, double, string, bool, Array, Dict>;

        Node() = default;


        template <typename T, typename = enable_if_t<
            is_same_v<decay_t<T>, nullptr_t> ||
            is_same_v<decay_t<T>, int> ||
            is_same_v<decay_t<T>, double> ||
            is_same_v<decay_t<T>, string> ||
            is_same_v<decay_t<T>, bool> ||
            is_same_v<decay_t<T>, Array> ||
            is_same_v<decay_t<T>, Dict>
        >>
        Node(T&& value) : value_(forward<T>(value)) {}

        Node(const char* value) : value_(string(value)) {}

        [[nodiscard]] bool IsInt() const { return holds_alternative<int>(value_); }
        [[nodiscard]] bool IsDouble() const { return IsInt() || holds_alternative<double>(value_); }
        [[nodiscard]] bool IsPureDouble() const { return holds_alternative<double>(value_); }
        [[nodiscard]] bool IsBool() const { return holds_alternative<bool>(value_); }
        [[nodiscard]] bool IsString() const { return holds_alternative<string>(value_); }
        [[nodiscard]] bool IsNull() const { return holds_alternative<nullptr_t>(value_); }
        [[nodiscard]] bool IsArray() const { return holds_alternative<Array>(value_); }
        [[nodiscard]] bool IsMap() const { return holds_alternative<Dict>(value_); }

        [[nodiscard]] int AsInt() const {
            if (!IsInt()) throw logic_error("Not an int");
            return get<int>(value_);
        }

        [[nodiscard]] double AsDouble() const {
            if (IsInt()) return static_cast<double>(get<int>(value_));
            if (IsPureDouble()) return get<double>(value_);
            throw logic_error("Not a double");
        }

        [[nodiscard]] bool AsBool() const {
            if (!IsBool()) throw logic_error("Not a bool");
            return get<bool>(value_);
        }

        [[nodiscard]] const string& AsString() const {
            if (!IsString()) throw logic_error("Not a string");
            return get<string>(value_);
        }

        [[nodiscard]] const Array& AsArray() const {
            if (!IsArray()) throw logic_error("Not an array");
            return get<Array>(value_);
        }

        [[nodiscard]] const Dict& AsMap() const {
            if (!IsMap()) throw logic_error("Not a map");
            return get<Dict>(value_);
        }

        [[nodiscard]] const Value& GetValue() const { return value_; }

        bool operator==(const Node& other) const { return value_ == other.value_; }
        bool operator!=(const Node& other) const { return !(*this == other); }

    private:
        Value value_;
    };

    class Document {
    public:
        explicit Document(Node root) : root_(std::move(root)) {}

        [[nodiscard]] const Node& GetRoot() const { return root_; }

    private:
        Node root_;
    };

    class ParsingError : public runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    Document Load(istream& input);

    void Print(const Document& doc, ostream& out);

    bool operator==(const Document& lhs, const Document& rhs);
    bool operator!=(const Document& lhs, const Document& rhs);
}
