#include "json.h"

namespace json {

	class Builder {
	private:
		class BaseContext;
		class KeyItemContext;
		class DictItemContext;
		class ArrayItemContext;

	public:
		Builder();
		Node Build();
		KeyItemContext Key(std::string key);
		BaseContext Value(Node::Value value);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		BaseContext EndDict();
		BaseContext EndArray();

	private:
		Node node_;
		std::vector<Node*> nodes_;

		Node::Value& GetCurrentValue();
		const Node::Value& GetCurrentValue() const;
		void CheckNewValue() const;
		void AddValue(Node::Value value, bool one_shot);

		class BaseContext {
		public:
			BaseContext(Builder& builder)
				:builder_(builder)
			{
			}
			
			Node Build() {
				return builder_.Build();
			}
			
			KeyItemContext Key(std::string key) {
				return builder_.Key(std::move(key));
			}
			
			BaseContext Value(Node::Value value) {
				return builder_.Value(std::move(value));
			}
			
			DictItemContext StartDict() {
				return builder_.StartDict();
			}
			
			ArrayItemContext StartArray() {
				return builder_.StartArray();
			}

			BaseContext EndDict() {
				return builder_.EndDict();
			}
			
			BaseContext EndArray() {
				return builder_.EndArray();
			}

		private:
			Builder& builder_;
		};

		class KeyItemContext : public BaseContext {
		public:
			KeyItemContext(BaseContext base)
				:BaseContext(base)
			{
			}

			Node Build() = delete;
			KeyItemContext Key(std::string key) = delete;
			BaseContext EndDict() = delete;
			BaseContext EndArray() = delete;

			DictItemContext Value(Node::Value value) {
				return BaseContext::Value(std::move(value));
			}
		};

		class DictItemContext : public BaseContext {
		public:
			DictItemContext(BaseContext base)
				:BaseContext(base)
			{
			}

			Node Build() = delete;
			BaseContext Value(Node::Value value) = delete;
			DictItemContext StartDict() = delete;
			ArrayItemContext StartArray() = delete;
			BaseContext EndArray() = delete;
		};

		class ArrayItemContext : public BaseContext {
		public:
			ArrayItemContext(BaseContext base)
				:BaseContext(base)
			{
			}

			Node Build() = delete;
			KeyItemContext Key(std::string key) = delete;
			BaseContext EndDict() = delete;

			ArrayItemContext Value(Node::Value value) {
				return BaseContext::Value(std::move(value));
			}
		};
	};
}