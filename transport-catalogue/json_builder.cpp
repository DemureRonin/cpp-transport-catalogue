#include "json_builder.h"

namespace json {
    Builder::Builder() : root_(nullptr) {
    }

    Node *Builder::GetCurrentContainer() const {
        if (nodes_stack_.empty()) {
            return nullptr;
        }
        return nodes_stack_.back();
    }

    Node &Builder::AddNode(Node node) {
        if (nodes_stack_.empty()) {
            if (!root_.IsNull()) throw std::logic_error("root already set");
            root_ = std::move(node);
            return root_;
        }
        Node *current = GetCurrentContainer();
        if (current->IsArray()) {
            auto &arr = const_cast<Array &>(current->AsArray());
            arr.push_back(std::move(node));
            return arr.back();
        } else if (current->IsDict()) {
            if (!pending_key_) throw std::logic_error("Key must be set before Value in dict");
            Dict &dict = const_cast<Dict &>(current->AsDict());
            auto &ref = dict[*pending_key_] = std::move(node);
            pending_key_.reset();
            return ref;
        }
        throw std::logic_error("invalid container type");
    }

    DictItemContext Builder::StartDict() {
        Node &node = AddNode(Node(Dict{}));
        nodes_stack_.push_back(&node);
        return DictItemContext(*this);
    }

    ArrayItemContext Builder::StartArray() {
        Node &node = AddNode(Node(Array{}));
        nodes_stack_.push_back(&node);
        return ArrayItemContext(*this);
    }


    Builder &Builder::EndDict() {
        if (nodes_stack_.empty() || !GetCurrentContainer()->IsDict()) {
            throw std::logic_error("Builder: EndDict() called without matching StartDict()");
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Builder &Builder::EndArray() {
        if (nodes_stack_.empty() || !GetCurrentContainer()->IsArray()) {
            throw std::logic_error("Builder: EndArray() called without matching StartArray()");
        }
        nodes_stack_.pop_back();
        return *this;
    }

    KeyItemContext Builder::Key(std::string key) {
        Node *current = GetCurrentContainer();
        if (!current || !current->IsDict()) {
            throw std::logic_error("Builder: Key() called outside of dict");
        }
        if (pending_key_) {
            throw std::logic_error("Builder: Key() called twice in a row");
        }
        pending_key_ = std::move(key);
        return KeyItemContext(*this);
    }

    Builder &Builder::Value(Node::Value value) {
        AddNode(Node(std::move(value)));
        return *this;
    }

    Node Builder::Build() {
        if (!nodes_stack_.empty()) {
            throw std::logic_error("Builder: Build() called with unclosed containers");
        }
        if (root_.IsNull()) {
            throw std::logic_error("Builder: nothing to build");
        }
        return std::move(root_);
    }


    DictItemContext KeyItemContext::Value(Node::Value value) const {
        builder_.Value(std::move(value));
        return DictItemContext(builder_);
    }

    DictItemContext KeyItemContext::StartDict() const {
        return builder_.StartDict();
    }

    ArrayItemContext KeyItemContext::StartArray() const {
        return builder_.StartArray();
    }


    KeyItemContext DictItemContext::Key(std::string key) const {
        return builder_.Key(std::move(key));
    }

    Builder &DictItemContext::EndDict() const {
        return builder_.EndDict();
    }

    ArrayItemContext ArrayItemContext::Value(Node::Value value) const {
        builder_.Value(std::move(value));
        return *this;
    }

    DictItemContext ArrayItemContext::StartDict() const {
        return builder_.StartDict();
    }

    ArrayItemContext ArrayItemContext::StartArray() const {
        return builder_.StartArray();
    }

    Builder &ArrayItemContext::EndArray() const {
        return builder_.EndArray();
    }
}
