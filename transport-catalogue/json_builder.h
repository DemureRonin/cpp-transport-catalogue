#pragma once
#include "json.h"
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <stdexcept>

namespace json {

    class Builder;

    class BaseContext {
    public:
        Builder &builder_;
        explicit BaseContext(Builder &builder) : builder_(builder) {}
    };

    class KeyItemContext;
    class DictItemContext;
    class ArrayItemContext;

    class Builder {
    public:
        Builder();

        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder &EndDict();
        Builder &EndArray();
        KeyItemContext Key(std::string key);
        Builder &Value(Node::Value value);

        Node Build();

    private:
        Node root_;
        std::vector<Node*> nodes_stack_;
        std::optional<std::string> pending_key_;

        Node* GetCurrentContainer() const;
        Node & AddNode(Node node);
    };

    class KeyItemContext : public BaseContext {
    public:
        explicit KeyItemContext(Builder &builder) : BaseContext(builder) {}

        DictItemContext Value(Node::Value value) const;
        DictItemContext StartDict() const;
        ArrayItemContext StartArray() const;
    };

    class DictItemContext : public BaseContext {
    public:
        explicit DictItemContext(Builder &builder) : BaseContext(builder) {}

        KeyItemContext Key(std::string key) const;
        Builder &EndDict() const;
    };

    class ArrayItemContext : public BaseContext {
    public:
        explicit ArrayItemContext(Builder &builder) : BaseContext(builder) {}

        ArrayItemContext Value(Node::Value value) const;
        DictItemContext StartDict() const;
        ArrayItemContext StartArray() const;
        Builder &EndArray() const;
    };

}