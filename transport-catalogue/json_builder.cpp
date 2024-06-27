#include "json_builder.h"

namespace json {

	using namespace std::literals;

	Builder::Builder()
		: node_(),
		nodes_{&node_}
	{
	}

	Node Builder::Build() {
		if (!nodes_.empty()) {
			throw std::logic_error("Failed to build not finalized JSON"s);
		}
		return std::move(node_);
	}

	Builder::KeyItemContext Builder::Key(std::string key) {
		Node::Value& host_value = GetCurrentValue();
		if (!std::holds_alternative<Dict>(host_value)) {
			throw std::logic_error("Key() outside a dict"s);
		}

		nodes_.push_back(&std::get<Dict>(host_value)[std::move(key)]);
		return BaseContext{ *this };
	}

	Builder::BaseContext Builder::Value(Node::Value value) {
		AddValue(std::move(value), /* one_shot */ true);
		return *this;
	}

	Builder::DictItemContext Builder::StartDict() {
		AddValue(Dict{}, /* one_shot */ false);
		return BaseContext{ *this };
	}

	Builder::ArrayItemContext Builder::StartArray() {
		AddValue(Array{}, /* one_shot */ false);
		return BaseContext{ *this };
	}

	Builder::BaseContext Builder::EndDict() {
		if (!std::holds_alternative<Dict>(GetCurrentValue())) {
			throw std::logic_error("EndDict() outside a dict"s);
		}
		nodes_.pop_back();
		return *this;
	}

	Builder::BaseContext Builder::EndArray() {
		if (!std::holds_alternative<Array>(GetCurrentValue())) {
			throw std::logic_error("EndDict() outside an array"s);
		}
		nodes_.pop_back();
		return *this;
	}

	// Current value can be:
	// * Dict, when .Key().Value() or EndDict() is expected
	// * Array, when .Value() or EndArray() is expected
	// * nullptr (default), when first call or dict Value() is expected

	Node::Value& Builder::GetCurrentValue() {
		if (nodes_.empty()) {
			throw std::logic_error("Attempt to change finalized JSON"s);
		}
		return nodes_.back()->GetValue();
	}

	const Node::Value& Builder::GetCurrentValue() const {
		return const_cast<Builder*>(this)->GetCurrentValue();
	}

	void Builder::CheckNewValue() const {	
		if (!std::holds_alternative<std::nullptr_t>(GetCurrentValue())) {
			throw std::logic_error("New value in wrong context"s);
		}
	}

	void Builder::AddValue(Node::Value value, bool one_shot) {
		Node::Value& host_value = GetCurrentValue();
		if (std::holds_alternative<Array>(host_value)) {
			Node& node
				= std::get<Array>(host_value).emplace_back(std::move(value));
			if (!one_shot) {
				nodes_.push_back(&node);
			}
		}
		else {
			CheckNewValue();
			host_value = std::move(value);
			if (one_shot) {
				nodes_.pop_back();
			}
		}
	}
}