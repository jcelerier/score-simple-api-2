#pragma once
#include <Process/ProcessMetadata.hpp>
#include <score/plugins/UuidKey.hpp>

#if defined(_MSC_VER)
#define meta_attribute_uuid(name, value) static inline auto uuid() { return_uuid(value); }
#else
#define meta_attribute_uuid(name, value) static constexpr auto uuid() { return_uuid(value); }
#endif

#define meta_attribute_name(name, value) static constexpr auto name() { return value; }
#define meta_attribute_pretty_name(name, value) static constexpr auto prettyName() { return value; }
#define meta_attribute_script_name(name, value) static constexpr auto objectKey() { { int value; (void) value; } return #value; }
#define meta_attribute_category(name, value) static constexpr auto category() { return #value; }
#define meta_attribute_author(name, value) static constexpr auto author() { return value; }
#define meta_attribute_kind(name, value) static constexpr auto kind() { return Process::ProcessCategory::value; }
#define meta_attribute_description(name, value) static constexpr auto description() { return value; }
#define meta_attribute_event(name, value) static constexpr auto is_event() { return value; }
#define meta_attribute_mimic_channels(name, value) static constexpr auto mimic_channels() { \
  static_assert(requires { &decltype(decltype(inputs)::value)::channels; }); \
  return &decltype(inputs)::value; \
}
#define meta_attribute_want_channels(name, value) static constexpr auto want_channels() { return value; }

#define meta_attribute(name, value) meta_attribute_ ## name(name, value)
#define meta_control(type, ...) static constexpr auto control() { return type{__VA_ARGS__}; }
#define meta_display(type, ...) static constexpr auto display() { return type{__VA_ARGS__}; }
