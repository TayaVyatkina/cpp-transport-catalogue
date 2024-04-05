#include "json.h"

using namespace std;

namespace json {
    namespace {
        /*
            Удаляет пробелы в начале и конце строки
        */
        std::string_view Trim(std::string_view string) {
            const auto start = string.find_first_not_of(' ');
            if (start == string.npos) {
                return {};
            }
            return string.substr(start, string.find_last_not_of(' ') + 1 - start);
        }

        Node LoadNode(istream& input);

        Node LoadString(istream& input) {
            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end && s.back() != '"') {
                    throw ParsingError("String parsing error");
                }
                if (it == end) {
                    break;
                }
                const char ch = *it;
                if (ch == '"') {
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    ++it;
                    if (it == end) {
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    throw ParsingError("Unexpected end of line");
                }
                else {
                    s.push_back(ch);
                }
                ++it;
            }

            return Node(std::move(s));
        }

        Node LoadNull(istream& input) {
            string line;
            char c;
            for (; input >> c && c != ':' && c != ',' && c != ']' && c != '}';) {
                line += c;
            }
            Trim(line);
            if (line == "null") {
                return Node();
            }
            else {
                throw ParsingError("Incorrect input"s);
            }
        }

        Node LoadArray(istream& input) {
            Array result;
            char c;

            for (; input >> c && c != ']';) {
                if (c != ',' && c!=' ') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (c != ']') {
                throw ParsingError("Incorrect input"s);
            }

            return Node(move(result));
        }

        Node LoadDict(istream& input) {
            Dict result;
            char c;

            for (; input >> c && c != '}';) {               
                if (c == ',') {
                    input >> c;
                }
                else {
                    while (c == ' ') {
                        input >> c;
                    }
                }
                string key = LoadString(input).AsString();
                do {
                    input >> c;
                } while (c != ':');
                
                do {
                    input >> c;
                } while (c == ' ');
                input.putback(c);
                result.insert({ move(key), LoadNode(input) });
            }
            if (c != '}') {
                throw ParsingError("Incorrect input"s);
            }

            return Node(move(result));
        }

        Node LoadBool(istream& input) {
            string line;
            char c;
            for (; input >> c && c != ':' && c != ',' && c != '}' && c != ']' && c != '\t' && c != '\n' && c != '\r';) {
                line += c;
            }
            input.putback(c);
            Trim(line);
            if (line == "true"sv) {
                return Node(true);
            }
            else if (line == "false"sv) {
                return Node(false);
            }
            else {
                throw ParsingError("Incorrect input"s);
            }

        }

        std::variant<double, int> LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
                };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
                };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return std::stoi(parsed_num);
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadNode(istream& input) {
            char c;
            do {
                input >> c;
            } while (c == ' ');

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else if (c == 'n') {
                input.putback(c);
                return LoadNull(input);
            }
            else if ((c == 't') || (c == 'f')) {
                input.putback(c);
                return LoadBool(input);
            }
            else {
                input.putback(c);
                auto number = LoadNumber(input);
                if (holds_alternative<int>(number)) {
                    return Node(std::get<int>(number));
                }
                else return Node(std::get<double>(number));
            }

            throw ParsingError("Incorrect input"s);
        }

    }  // namespace

    bool Node::IsInt() const {
        return holds_alternative<int>(value_);
    }
    bool Node::IsDouble() const {
        return holds_alternative<double>(value_) || IsInt();
    }
    bool Node::IsPureDouble() const {
        return holds_alternative<double>(value_);
    }
    bool Node::IsBool() const {
        return holds_alternative<bool>(value_);
    }
    bool Node::IsString() const {
        return holds_alternative<std::string>(value_);
    }
    bool Node::IsNull() const {
        return holds_alternative<std::nullptr_t>(value_);
    }
    bool Node::IsArray() const {
        return holds_alternative<Array>(value_);
    }
    bool Node::IsMap() const {
        return holds_alternative<Dict>(value_);
    }

    bool Node::AsBool() const {
        if (IsBool()) {
            return get<bool>(value_);
        }
        throw std::logic_error("logic error");
    }
    int Node::AsInt() const {
        if (IsInt()) {
            return get<int>(value_);
        }
        throw std::logic_error("logic error");
    }
    double Node::AsDouble() const {
        if (IsInt()) {
            return AsInt() * 1.;
        }
        else if (IsDouble()) {
            return get<double>(value_);
        }
        throw std::logic_error("logic error");
    }
    const string& Node::AsString() const {
        if (IsString()) {
            return get<std::string>(value_);
        }
        throw std::logic_error("logic error");
    }
    const Array& Node::AsArray() const {
        if (IsArray()) {
            return get<Array>(value_);
        }
        throw std::logic_error("logic error");
    }
    const Dict& Node::AsMap() const {
        if (IsMap()) {
            return get<Dict>(value_);
        }
        throw std::logic_error("logic error");
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    //int, double
    template <typename Value>
    void PrintValue(const Value& value, std::ostream& out) {
        out << value;
    }

    //null
    void PrintValue(std::nullptr_t, std::ostream& out) {
        out << "null"sv;
    }

    //bool
    void PrintValue(const bool value, std::ostream& out) {
        out << boolalpha;
        out << value;
    }

    //string
    void PrintValue(const std::string& value, std::ostream& out) {
        out << "\""sv;
        for (const char c : value) {
            switch (c) {
            case '\r':
                out << "\\r"sv;
                break;
            case '\n':
                out << "\\n"sv;
                break;
            case '\t':
                out << "\\t"sv;
                break;
            case '\"':
                out << "\\\""sv;
                break;
            case '\\':
                out << "\\\\"sv;
                break;
            default:
                out.put(c);
                break;
            }
        }
        out << "\""sv;
    }

    void PrintNode(const Node& node, std::ostream& out);

    //array
    void PrintValue(const Array& arr, std::ostream& out) {
        bool flag = false;
        out << "["sv << endl;
        for (const auto& node : arr) {
            if (flag) {
                out << ", "sv;
            }
            else {
                flag = true;
            }
            out << '\t';
            PrintNode(node, out);
        }
        out << endl << "]"sv;
    }

    //dict
    void PrintValue(const Dict& dict, std::ostream& out) {
        bool flag = false;
        out << endl << "{"sv << endl;
        for (auto& [name, value] : dict) {
            if (flag) {
                out << ", "sv << endl;
            }
            else {
                flag = true;
            }
            out << '\t';
            PrintValue(name, out);
            out << ":"sv;
            PrintNode(value, out);
        }
        out << endl << "}"sv;
    }

    void PrintNode(const Node& node, std::ostream& out) {
        std::visit(
            [&out](const auto& value) { PrintValue(value, out); },
            node.GetValue()
        );
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), output);
    }

}  // namespace json