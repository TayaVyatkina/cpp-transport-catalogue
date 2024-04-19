#pragma once

#include "json.h"

namespace json {
    
    class Builder;
    class KeyContext;
    class DictContext;
    class ArrayContext;

    class BaseContext {
    public:
        BaseContext(Builder& builder);

        KeyContext Key(std::string key);

        DictContext StartDict();

        ArrayContext StartArray();

        Builder& Value(Node::Value value);

        Builder& EndDict();

        Builder& EndArray();
    private:
        Builder& builder_;
    };

    class  KeyContext : public BaseContext {
    public:
        KeyContext(Builder& builder);

        DictContext Value(Node::Value value);

        BaseContext Key(std::string key) = delete;

        BaseContext EndDict() = delete;

        BaseContext EndArray() = delete;     
    };

    class  DictContext : public BaseContext {
    public:
        DictContext(Builder& builder);

        DictContext StartDict() = delete;

        ArrayContext StartArray() = delete;

        Builder& Value(Node::Value value) = delete;

        Builder& EndArray() = delete;
    };

    class ArrayContext : public BaseContext {
    public:
        ArrayContext(Builder& builder);

        ArrayContext Value(Node::Value value);

        KeyContext Key(std::string key) = delete;

        Builder& EndDict() = delete;      
    };

    class Builder {
    public:
        Builder();

        Node Build();

        Builder& Value(Node::Value value);

        KeyContext Key(std::string key);

        DictContext StartDict();

        ArrayContext StartArray();

        Builder& EndDict();

        Builder& EndArray();

    private:
        Node root_;
        std::vector<Node*> nodes_stack_;

        Node::Value& GetCurrentValue();
        
        void AddObject(Node::Value value, bool one_shot);
    };

}  // namespace json