#pragma once

#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {
    class Node;
    // Сохраните объявления Dict и Array без изменения
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
        const Value& GetValue() const { return value_; }

        Node() :value_(nullptr) {}

        template<typename T>
        Node(T node) : value_(std::move(node)) {}

        bool operator==(const Node& other) const {
            return value_ == other.value_;
        }

        bool operator!=(const Node& other) const {
            return !(*this == other);
        }

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        bool AsBool() const;
        double AsDouble() const;// Возвращает значение типа double, если внутри хранится double либо int
        int AsInt() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

    private:
        Value value_;
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

        inline bool operator==(const Document& other) const {
            return root_ == other.root_;
        }

        inline bool operator!=(const Document& other) const {
            return !(*this == other);
        }


    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json