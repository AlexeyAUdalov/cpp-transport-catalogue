#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node final
        : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>
    {
    public:
        // Делаем доступными все конструкторы родительского класса variant
        using variant::variant;
        using Value = variant;

        Node(Value value);

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;
        const variant& GetValue() const;
        variant& GetValue();
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    Document Load(std::istream& input);
    void Print(const Document& doc, std::ostream& output);
    bool operator==(const Node& left, const Node& right);
    bool operator!=(const Node& left, const Node& right);
    bool operator==(const Document& left, const Document& right);
    bool operator!=(const Document& left, const Document& right);
}  // namespace json