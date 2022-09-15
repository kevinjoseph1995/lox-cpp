//
// Created by kevin on 9/15/22.
//
#include "value.h"


bool Value::IsNil() const
{
    return std::holds_alternative<NilType>(*this);
}

bool Value::IsDouble() const
{
    return std::holds_alternative<double>(*this);
}

bool Value::IsBool() const
{
    return std::holds_alternative<bool>(*this);
}

bool Value::IsObject() const
{
    return std::holds_alternative<Object*>(*this);
}

double& Value::AsDouble()
{
    LOX_ASSERT(IsDouble());
    return *std::get_if<double>(this);
}

bool& Value::AsBool()
{
    LOX_ASSERT(IsBool());
    return *std::get_if<bool>(this);
}

double const& Value::AsDouble() const
{
    LOX_ASSERT(IsDouble());
    return *std::get_if<double>(this);
}

Object const& Value::AsObject() const
{
    LOX_ASSERT(IsObject());
    return *(*std::get_if<Object*>(this));
}

Object& Value::AsObject()
{
    LOX_ASSERT(IsObject());
    return *(*std::get_if<Object*>(this));
}

Object* Value::AsObjectPtr()
{
    LOX_ASSERT(IsObject());
    return (*std::get_if<Object*>(this));
}

Object const* Value::AsObjectPtr() const
{
    LOX_ASSERT(IsObject());
    return (*std::get_if<Object*>(this));
}

bool const& Value::AsBool() const
{
    LOX_ASSERT(IsBool());
    return *std::get_if<bool>(this);
}

bool Value::operator==(Value const& other) const
{
    if (this->index() != other.index()) {
        return false;
    }
    if (this->IsBool()) {
        return *std::get_if<bool>(this) == *std::get_if<bool>(&other);
    } else if (this->IsDouble()) {
        return std::abs(*std::get_if<double>(this) - *std::get_if<double>(&other)) < std::numeric_limits<double>::epsilon();
    } else if(this->IsObject()){
        LOX_ASSERT(this->AsObject().GetType() == ObjectType::STRING);
        LOX_ASSERT(other.AsObject().GetType() == ObjectType::STRING);
        return static_cast<StringObject const*>(this->AsObjectPtr())->data == static_cast<StringObject const*>(other.AsObjectPtr())->data;
    }else{
        LOX_ASSERT(false, "Unsupported comparison");
    }
}
bool Value::operator!=(Value const& other) const
{
    return !(*this == other);
}
