#include "json.h"
#include <limits>
#include <cctype>
#include <sstream>

using namespace std;

namespace json {
    namespace {
        Node LoadNode(istream &input);

        Node LoadString(std::istream &input) {
            std::string s;
            char c;
            while (input.get(c)) {
                if (c == '"') return Node(std::move(s));
                if (c == '\\') {
                    char esc;
                    if (!input.get(esc)) throw ParsingError("Unexpected end of input in string");
                    switch (esc) {
                        case 'n': s += '\n';
                            break;
                        case 't': s += '\t';
                            break;
                        case 'r': s += '\r';
                            break;
                        case '"': s += '"';
                            break;
                        case '\\': s += '\\';
                            break;
                        default: s += esc;
                            break;
                    }
                } else {
                    s += c;
                }
            }
            throw ParsingError("Unexpected end of input in string");
        }


        Node LoadNumber(istream &input) {
            string num_str;
            while (input.peek() != EOF) {
                char c = static_cast<char>(input.peek());
                if (isdigit(c) || c == '-' || c == '+' || c == '.' || c == 'e' || c == 'E') {
                    num_str += c;
                    input.get();
                } else {
                    break;
                }
            }

            stringstream ss(num_str);
            double d;
            ss >> d;

            if (ss.fail()) throw ParsingError("Failed to parse number: " + num_str);

            if (num_str.find('.') == string::npos && num_str.find('e') == string::npos &&
                num_str.find('E') == string::npos && d >= numeric_limits<int>::min() &&
                d <= numeric_limits<int>::max()) {
                return Node(static_cast<int>(d));
            }
            return Node(d);
        }

        Node LoadLiteral(istream &input) {
            string literal;
            while (isalpha(input.peek())) {
                literal += static_cast<char>(input.get());
            }
            if (literal == "true") return Node(true);
            if (literal == "false") return Node(false);
            if (literal == "null") return Node(nullptr);
            throw ParsingError("Unknown literal: " + literal);
        }

        void skip_ws(std::istream &input) {
            char c;
            while (input.get(c)) {
                if (!isspace(c)) {
                    input.putback(c);
                    break;
                }
            }
        }

        Node LoadArray(std::istream &input) {
            using namespace std::literals;

            Array result;
            char c;


            skip_ws(input);
            if (!(input >> c)) {
                throw ParsingError("Unexpected end of input inside array"s);
            }
            if (c == ']') return Node(result);

            input.putback(c);

            for (;;) {
                result.push_back(LoadNode(input));

                skip_ws(input);
                if (!(input >> c)) {
                    throw ParsingError("Unexpected end of input inside array"s);
                }
                if (c == ']') break;
                if (c != ',') {
                    throw ParsingError("Expected ',' or ']' in array"s);
                }
            }

            return Node(std::move(result));
        }


        Node LoadMap(std::istream &input) {
            Dict dict;
            char c;

            skip_ws(input);
            if (input.peek() == '}') {
                input.get();
                return Node(dict);
            }

            for (;;) {
                skip_ws(input);
                if (!(input >> c) || c != '"') {
                    throw ParsingError("Expected string key");
                }
                Node key = LoadString(input);

                skip_ws(input);
                if (!(input >> c) || c != ':') {
                    throw ParsingError("Expected ':' after key");
                }

                Node value = LoadNode(input);
                dict.emplace(key.AsString(), value);

                skip_ws(input);
                if (!(input >> c)) {
                    throw ParsingError("Unexpected end of input inside map");
                }
                if (c == '}') break;
                if (c != ',') {
                    throw ParsingError("Expected ',' or '}' in map");
                }
            }

            return Node(dict);
        }


        Node LoadNode(istream &input) {
            char c;
            do {
                if (!input.get(c)) throw ParsingError("Unexpected end of input");
            } while (isspace(c));

            if (c == '{') return LoadMap(input);
            if (c == '[') return LoadArray(input);
            if (c == '"') return LoadString(input);
            if (c == '-' || isdigit(c)) {
                input.putback(c);
                return LoadNumber(input);
            }

            input.putback(c);
            return LoadLiteral(input);
        }

        void PrintValue(const Node &node, ostream &out);

        void PrintString(const string &s, ostream &out) {
            out << '"';
            for (char c: s) {
                switch (c) {
                    case '\n': out << "\\n";
                        break;
                    case '\t': out << "\\t";
                        break;
                    case '\r': out << "\\r";
                        break;
                    case '"': out << "\\\"";
                        break;
                    case '\\': out << "\\\\";
                        break;
                    default: out << c;
                        break;
                }
            }
            out << '"';
        }

        void PrintArray(const Array &arr, ostream &out) {
            out << '[';
            bool first = true;
            for (const auto &item: arr) {
                if (!first) out << ',';
                first = false;
                PrintValue(item, out);
            }
            out << ']';
        }

        void PrintMap(const Dict &dict, ostream &out) {
            out << '{';
            bool first = true;
            for (const auto &[key, value]: dict) {
                if (!first) out << ',';
                first = false;
                PrintString(key, out);
                out << ':';
                PrintValue(value, out);
            }
            out << '}';
        }

        void PrintValue(const Node &node, ostream &out) {
            const auto &val = node.GetValue();
            if (holds_alternative<nullptr_t>(val)) out << "null";
            else if (holds_alternative<int>(val)) out << get<int>(val);
            else if (holds_alternative<double>(val)) out << get<double>(val);
            else if (holds_alternative<bool>(val)) out << (get<bool>(val) ? "true" : "false");
            else if (holds_alternative<string>(val)) PrintString(get<string>(val), out);
            else if (holds_alternative<Array>(val)) PrintArray(get<Array>(val), out);
            else if (holds_alternative<Dict>(val)) PrintMap(get<Dict>(val), out);
        }
    }

    Document Load(istream &input) {
        return Document(LoadNode(input));
    }

    void Print(const Document &doc, ostream &out) {
        PrintValue(doc.GetRoot(), out);
    }

bool operator==(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}

bool operator!=(const Document& lhs, const Document& rhs) {
    return !(lhs == rhs);
}

}