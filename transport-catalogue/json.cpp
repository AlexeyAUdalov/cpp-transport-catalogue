#include "json.h"

namespace json {

    namespace {
        using namespace std::literals;

        Node LoadNode(std::istream& input);

        std::string LoadLiteral(std::istream& input) {
            std::string s;
            while (std::isalpha(input.peek())) {
                s.push_back(static_cast<char>(input.get()));
            }
            return s;
        }

        Node LoadArray(std::istream& input) {
            Array result;
            char c;
            for (; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (c != ']') {
                throw ParsingError("Array parsing error"s);
            }
            return Node(move(result));
        }

        // Считывает содержимое строкового литерала JSON-документа
        // Функцию следует использовать после считывания открывающего символа ":
        std::string LoadString(std::istream& input) {
            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
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
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return s;
        }

        Node LoadDict(std::istream& input) {
            Dict result;
            char c;
            for (; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }
                std::string key = LoadString(input);
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }
            if (c != '}') {
                throw ParsingError("Dict parsing error"s);
            }
            return Node(move(result));
        }

        Node LoadBool(std::istream& input) {
            const auto s = LoadLiteral(input);
            if (s == "true"sv) {
                return Node{ true };
            }
            else if (s == "false"sv) {
                return Node{ false };
            }
            else {
                throw ParsingError("Failed to parse '"s + s + "' as bool"s);
            }
        }

        Node LoadNull(std::istream& input) {
            if (auto literal = LoadLiteral(input); literal == "null"sv) {
                return Node{ nullptr };
            }
            else {
                throw ParsingError("Failed to parse '"s + literal + "' as null"s);
            }
        }

        Node LoadNumber(std::istream& input) {
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
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadNode(std::istream& input) {
            char c;
            if (!(input >> c)) {
                throw ParsingError("Unexpected EOF"s);
            }
            switch (c) {
            case '[':
                return LoadArray(input);
            case '{':
                return LoadDict(input);
            case '"':
                return LoadString(input);
            case 't':
                // Атрибут [[fallthrough]] (провалиться) ничего не делает, и является
                // подсказкой компилятору и человеку, что здесь программист явно задумывал
                // разрешить переход к инструкции следующей ветки case, а не случайно забыл
                // написать break, return или throw.
                // В данном случае, встретив t или f, переходим к попытке парсинга
                // литералов true либо false
                [[fallthrough]];
            case 'f':
                input.putback(c);
                return LoadBool(input);
            case 'n':
                input.putback(c);
                return LoadNull(input);
            default:
                input.putback(c);
                return LoadNumber(input);
            }
        }

        struct PrintContext {
            std::ostream& out;
            int indent_step = 4;
            int indent = 0;

            void PrintIndent() const {
                for (int i = 0; i < indent; ++i) {
                    out.put(' ');
                }
            }

            // Возвращает новый контекст вывода с увеличенным смещением
            PrintContext Indented() const {
                return { out, indent_step, indent_step + indent };
            }
        };

        void PrintNode(const Node& node, const PrintContext& ctx);

        template <typename Number>
        void PrintValue(const Number& value, const PrintContext& ctx) {
            ctx.out << value;
        }

        void PrintValue(const std::string& str, const PrintContext& ctx) {
            std::ostream& out = ctx.out;
            out << "\""sv;
            std::string output_str;
            for (const auto& symbol : str) {
                switch (symbol) {
                case '\n':
                    output_str += "\\n";
                    break;
                case '\r':
                    output_str += "\\r";
                    break;
                case '\"':
                    output_str += "\\\"";
                    break;
                case '\t':
                    output_str += "\\t";
                    break;
                case '\\':
                    output_str += "\\\\";
                    break;
                default:
                    output_str += symbol;
                    break;
                }
            }
            out << output_str;
            out << "\""sv;
        }

        void PrintValue(const std::nullptr_t&, const PrintContext& ctx) {
            ctx.out << "null"sv;
        }

        // В специализации шаблона PrintValue для типа bool параметр value передаётся
        // по константной ссылке, как и в основном шаблоне.
        // В качестве альтернативы можно использовать перегрузку:
        // void PrintValue(bool value, const PrintContext& ctx);
        template <>
        void PrintValue<bool>(const bool& value, const PrintContext& ctx) {
            ctx.out << (value ? "true"sv : "false"sv);
        }

        void PrintValue(const Array& arr, const PrintContext& ctx) {
            std::ostream& out = ctx.out;
            bool first = true;
            out << "["sv << std::endl;
            PrintContext map_ctx{ ctx.Indented() };
            map_ctx.PrintIndent();
            for (const auto& value : arr) {
                if (!first) {
                    map_ctx.out << ", "sv << std::endl;
                    map_ctx.PrintIndent();
                }
                PrintNode(value, map_ctx);
                first = false;
            }
            out << std::endl;
            ctx.PrintIndent();
            out << "]"sv;
        }

        void PrintValue(const Dict& dict, const PrintContext& ctx) {
            std::ostream& out = ctx.out;
            bool first = true;
            out << "{"sv << std::endl;
            PrintContext map_ctx{ ctx.Indented() };
            map_ctx.PrintIndent();
            for (const auto& value : dict) {
                if (!first) {
                    map_ctx.out << ", "sv << std::endl;
                    map_ctx.PrintIndent();
                }
                PrintValue(value.first, map_ctx);
                map_ctx.out << ": "sv;
                PrintNode(value.second, map_ctx);
                first = false;
            }
            out << std::endl;
            ctx.PrintIndent();
            out << "}"sv;
        }

        void PrintNode(const Node& node, const PrintContext& ctx) {
            std::visit(
                [&ctx](const auto& value) {
                    PrintValue(value, { ctx });
                },
                node.GetValue());
        }

    }  // namespace

    Node::Node(Value value)
        :variant(std::move(value))
    {
    }

    bool Node::IsInt() const {
        return std::holds_alternative<int>(*this);
    }

    bool Node::IsDouble() const {
        return std::holds_alternative<int>(*this) || std::holds_alternative<double>(*this);
    }

    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(*this);
    }

    bool Node::IsBool() const {
        return std::holds_alternative<bool>(*this);
    }

    bool Node::IsString() const {
        return std::holds_alternative<std::string>(*this);
    }

    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(*this);
    }

    bool Node::IsArray() const {
        return std::holds_alternative<Array>(*this);
    }

    bool Node::IsMap() const {
        return std::holds_alternative<Dict>(*this);
    }

    int Node::AsInt() const {
        return IsInt() ? std::get<int>(*this) : throw std::logic_error("The Node is not int"s);
    }

    bool Node::AsBool() const {
        return IsBool() ? std::get<bool>(*this) : throw std::logic_error("The Node is not bool"s);
    }

    double Node::AsDouble() const {
        if (IsDouble()) {
            return IsPureDouble() ? std::get<double>(*this) : static_cast<double>(std::get<int>(*this));
        }
        else {
            throw std::logic_error("The Node is not double"s);
        }
    }

    const std::string& Node::AsString() const {
        return IsString() ? std::get<std::string>(*this) : throw std::logic_error("The Node is not string"s);
    }

    const Array& Node::AsArray() const {
        return IsArray() ? std::get<Array>(*this) : throw std::logic_error("The Node is not Array"s);
    }

    const Dict& Node::AsMap() const {
        return IsMap() ? std::get<Dict>(*this) : throw std::logic_error("The Node is not map"s);
    }

    const Node::variant& Node::GetValue() const {
        return *this;
    }

    Node::variant& Node::GetValue() {
        return *this;
    }

    Document::Document(Node root)
        : root_(std::move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(std::istream& input) {
        return Document{ LoadNode(input) };
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), PrintContext{ output });
    }

    bool operator==(const Node& left, const Node& right) {
        return left.GetValue() == right.GetValue();
    }

    bool operator!=(const Node& left, const Node& right) {
        return !(left == right);
    }

    bool operator==(const Document& left, const Document& right) {
        return left.GetRoot() == right.GetRoot();
    }

    bool operator!=(const Document& left, const Document& right) {
        return !(left == right);
    }

}  // namespace json