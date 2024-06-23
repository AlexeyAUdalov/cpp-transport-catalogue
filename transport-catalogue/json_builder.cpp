#include "json_builder.h"

namespace json {

	using namespace std::literals;

		Node Builder::Build() {
		if (!is_empty_ && nodes_.empty()) {
			return std::move(node_);
		}
		throw std::logic_error("Failed to Build"s);
	}

	KeyItemContext Builder::Key(std::string key) {
		if (!is_empty_ && !nodes_.empty() && nodes_.back()->IsMap()) {
			nodes_.emplace_back(&std::get<Dict>(nodes_.back()->GetValue())[std::move(key)]);
			return *this;
		}

		throw std::logic_error("Failed to add key"s);
	}

	Builder& Builder::Value(Node value, bool start) {

		if (is_empty_) {
			is_empty_ = false;
			node_ = std::move(value);
			if (start) {
				nodes_.push_back(&node_);
			}
			return *this;
		}

		if (!nodes_.empty()) {
			if (nodes_.back()->IsNull()) {
				*nodes_.back() = std::move(value);
				if (!start) {
					nodes_.pop_back();
				}
				return *this;
			}

			if (nodes_.back()->IsArray()) {
				auto& array = std::get<Array>(nodes_.back()->GetValue());
				array.push_back(value);
				if (start) {
					nodes_.push_back(&array.back());
				}
				return *this;
			}
		}

		throw std::logic_error("Failed to add value"s);
	}

	DictItemContext Builder::StartDict() {
		Value(Dict{}, true);
		return *this;
	}

	ArrayItemContext Builder::StartArray() {
		Value(Array{}, true);
		return *this;
	}

	Builder& Builder::EndDict() {
		if (!is_empty_ && !nodes_.empty() && nodes_.back()->IsMap()) {
			nodes_.pop_back();
			return *this;
		}
		throw std::logic_error("Failed to EndDict"s);
	}

	Builder& Builder::EndArray() {
		if (!is_empty_ && !nodes_.empty() && nodes_.back()->IsArray()) {
			nodes_.pop_back();
			return *this;
		}
		throw std::logic_error("Failed to EndArray"s);
	}

	BaseContext::BaseContext(Builder& builder)
		:builder_(builder)
	{
	}

	Node BaseContext::Build() {
		return builder_.Build();
	}

	KeyItemContext BaseContext::Key(std::string key) {
		return builder_.Key(std::move(key));
	}

	Builder& BaseContext::Value(Node value) {
		return builder_.Value(value);
	}

	DictItemContext BaseContext::StartDict() {
		return builder_.StartDict();
	}

	DictItemContext KeyItemContext::Value(Node value) {
		return BaseContext::Value(value);
	}

	ArrayItemContext BaseContext::StartArray() {
		return builder_.StartArray();
	}

	ArrayItemContext ArrayItemContext::Value(Node value) {
		return BaseContext::Value(value);
	}

	Builder& BaseContext::EndDict() {
		return builder_.EndDict();
	}

	Builder& BaseContext::EndArray() {
		return builder_.EndArray();
	}
}