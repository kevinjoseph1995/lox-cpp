//
// Created by kevin on 9/15/22.
//
#include "value.h"

auto Value::IsNil() const -> bool
{
    return std::holds_alternative<NilType>(*this);
}

auto Value::IsDouble() const -> bool
{
    return std::holds_alternative<double>(*this);
}

auto Value::IsBool() const -> bool
{
    return std::holds_alternative<bool>(*this);
}

auto Value::IsObject() const -> bool
{
    return std::holds_alternative<Object*>(*this);
}

auto Value::AsDouble() -> double&
{
    LOX_ASSERT(IsDouble());
    return *std::get_if<double>(this);
}

auto Value::AsBool() -> bool&
{
    LOX_ASSERT(IsBool());
    return *std::get_if<bool>(this);
}

auto Value::AsDouble() const -> double const&
{
    LOX_ASSERT(IsDouble());
    return *std::get_if<double>(this);
}

auto Value::AsObject() const -> Object const&
{
    LOX_ASSERT(IsObject());
    return *(*std::get_if<Object*>(this));
}

auto Value::AsObject() -> Object&
{
    LOX_ASSERT(IsObject());
    return *(*std::get_if<Object*>(this));
}

auto Value::AsObjectPtr() -> Object*
{
    LOX_ASSERT(IsObject());
    return (*std::get_if<Object*>(this));
}

auto Value::AsObjectPtr() const -> Object const*
{
    LOX_ASSERT(IsObject());
    return (*std::get_if<Object*>(this));
}

auto Value::AsBool() const -> bool const&
{
    LOX_ASSERT(IsBool());
    return *std::get_if<bool>(this);
}

auto Value::operator==(Value const& other) const -> bool
{
    if (this->index() != other.index()) {
        return false;
    }
    if (this->IsBool()) {
        return *std::get_if<bool>(this) == *std::get_if<bool>(&other);
    } else if (this->IsDouble()) {
        return std::abs(*std::get_if<double>(this) - *std::get_if<double>(&other)) < std::numeric_limits<double>::epsilon();
    } else if (this->IsObject()) {
        LOX_ASSERT(this->AsObject().GetType() == ObjectType::STRING);
        LOX_ASSERT(other.AsObject().GetType() == ObjectType::STRING);
        return static_cast<StringObject const*>(this->AsObjectPtr())->data == static_cast<StringObject const*>(other.AsObjectPtr())->data;
    } else {
        LOX_ASSERT(false, "Unsupported comparison");
    }
}
auto Value::operator!=(Value const& other) const -> bool
{
    return !(*this == other);
}
