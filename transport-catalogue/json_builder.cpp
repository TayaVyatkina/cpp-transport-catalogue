#include "json_builder.h"
#include <exception>

namespace json {  

    BaseContext::BaseContext(Builder& builder)
        : builder_(builder)
    {}

    KeyContext BaseContext::Key(std::string key) {
        return builder_.Key(std::move(key));
    }

    DictContext BaseContext::StartDict() {
        return DictContext(builder_.StartDict());
    }
    
    ArrayContext BaseContext::StartArray() {
        return ArrayContext(builder_.StartArray());
    }

    Builder& BaseContext::Value(Node::Value value) {
        return builder_.Value(std::move(value));
    }

    Builder& BaseContext::EndDict() {
        return builder_.EndDict();        
    }

    Builder& BaseContext::EndArray() {
        return builder_.EndArray();
    }



    KeyContext::KeyContext(Builder& builder) 
        : BaseContext(builder) 
    {}

    DictContext KeyContext::Value(Node::Value value) { 
        return BaseContext::Value(std::move(value)); 
    }


    DictContext::DictContext(Builder& builder) 
        : BaseContext(builder) 
    {}


    ArrayContext::ArrayContext(Builder& builder) 
        : BaseContext(builder) 
    {}

    ArrayContext ArrayContext::Value(Node::Value value) { 
        return BaseContext::Value(move(value)); 
    }


    Builder::Builder()
        : root_()
        , nodes_stack_{ &root_ }
    {}

    class LogicError : public std::logic_error {
    public:
        using logic_error::logic_error;
    };

    using namespace std::literals;

    Node Builder::Build() {
        // ������ ������� �������������� ���������
        if (!nodes_stack_.empty()) {
            throw LogicError("still unfinished Array/Dict"s);
        }
        return std::move(root_);
    }

    Builder& Builder::Value(Node::Value value) {
        AddObject(std::move(value), true);
        return *this;
    }

    KeyContext Builder::Key(std::string key) {
        Node::Value& host_value = GetCurrentValue();
        // ������ ������ ����� ��� �������
        if (!std::holds_alternative<Dict>(host_value)) {
            throw LogicError("Dict recording is not started"s);
        }

        nodes_stack_.push_back(
            &std::get<Dict>(host_value)[std::move(key)]
        );
        return *this;
    }

    DictContext Builder::StartDict() {
        AddObject(Dict{}, false);
        return *this;
    }

    ArrayContext Builder::StartArray() {
        AddObject(Array{}, false);
        return *this;
    }

    Builder& Builder::EndDict() {
        // ������ ���������� ������������ �������
        if (!std::holds_alternative<Dict>(GetCurrentValue())) {
            throw LogicError("Dict beginning is missing"s);
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Builder& Builder::EndArray() {
        // ������ ���������� ������������ �������
        if (!std::holds_alternative<Array>(GetCurrentValue())) {
            throw LogicError("Array beginning is missing"s);
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Node::Value& Builder::GetCurrentValue() {
        if (nodes_stack_.empty()) {
            // �� ���� ������ �� �������
            throw std::logic_error("nodes stack is empty"s);
        }
        return nodes_stack_.back()->GetValue();
    }

    void Builder::AddObject(Node::Value value, bool one_shot) {
        Node::Value& host_value = GetCurrentValue();
        if (std::holds_alternative<Array>(host_value)) {
            Node& node = std::get<Array>(host_value).emplace_back(std::move(value));
            if (!one_shot) {
                nodes_stack_.push_back(&node);
            }
        }
        else {
            if (!std::holds_alternative<std::nullptr_t>(GetCurrentValue())) {
                throw std::logic_error("wrong object type"s);
            }
            host_value = std::move(value);
            if (one_shot) {
                nodes_stack_.pop_back();
            }
        }
    }

}  // namespace json