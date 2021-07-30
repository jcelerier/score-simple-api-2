#pragma once
#include <Process/ProcessFlags.hpp>

#define meta_attribute_uuid(name, value) static uuid_constexpr auto uuid() { return_uuid(value); }

#define meta_attribute_name(name, value) static constexpr auto name() { return value; }
#define meta_attribute_pretty_name(name, value) static constexpr auto prettyName() { return value; }
#define meta_attribute_script_name(name, value) static constexpr auto objectKey() { { int value; (void) value; } return #value; }
#define meta_attribute_category(name, value) static constexpr auto category() { return #value; }
#define meta_attribute_author(name, value) static constexpr auto author() { return value; }
#define meta_attribute_kind(name, value) static constexpr auto kind() { return Process::ProcessCategory::value; }
#define meta_attribute_description(name, value) static constexpr auto description() { return value; }

#define meta_attribute(name, value) meta_attribute_ ## name(name, value)
#define meta_control(type, ...) static constexpr type control() { return {__VA_ARGS__}; }
#define meta_display(type, ...) static constexpr type display() { return {__VA_ARGS__}; }
