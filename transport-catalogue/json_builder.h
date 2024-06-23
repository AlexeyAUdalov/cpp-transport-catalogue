#include "json.h"

namespace json {

	class KeyItemContext;
	class DictItemContext;
	class ArrayItemContext;

	class Builder {
	public:
		Builder() = default;
		Node Build();
		KeyItemContext Key(std::string);
		Builder& Value(Node value, bool = false);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		Builder& EndDict();
		Builder& EndArray();

	private:
		Node node_;
		std::vector<Node*> nodes_;
		bool is_empty_ = true;
	};

	class BaseContext {
	public:
		BaseContext(Builder& builder);
		Node Build();
		KeyItemContext Key(std::string);
		Builder& Value(Node value);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		Builder& EndDict();
		Builder& EndArray();

	private:
		Builder& builder_;
	};

	class KeyItemContext : public BaseContext {
	public:
		using BaseContext::BaseContext;
		Node Build() = delete;
		KeyItemContext Key(std::string) = delete;
		Builder& EndDict() = delete;
		Builder& EndArray() = delete;
		DictItemContext Value(Node value);
	};

	class DictItemContext : public BaseContext {
	public:
		using BaseContext::BaseContext;
		Node Build() = delete;
		Builder& Value(Node value) = delete;
		DictItemContext StartDict() = delete;
		ArrayItemContext StartArray() = delete;
		Builder& EndArray() = delete;
	};

	class ArrayItemContext : public BaseContext {
	public:
		using BaseContext::BaseContext;
		Node Build() = delete;
		KeyItemContext Key(std::string) = delete;
		Builder& EndDict() = delete;
		ArrayItemContext Value(Node value);
	};
}