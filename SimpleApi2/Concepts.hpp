#pragma once

#include <Control/Widgets.hpp>
#include <cmath>
#include <ossia/dataflow/audio_port.hpp>
#include <ossia/dataflow/safe_nodes/tick_policies.hpp>
#include <Process/ProcessFlags.hpp>
#include <Process/ProcessMetadata.hpp>
#include <score/plugins/UuidKey.hpp>
namespace SimpleApi2
{

#define make_uuid(text) score::uuids::string_generator::compute((text))
#if defined(_MSC_VER)
#define uuid_constexpr inline
#else
#define uuid_constexpr constexpr
#endif


#define constant static inline constexpr auto

struct Meta_base
{
  static const constexpr double recommended_height{};
  static const constexpr Process::ProcessFlags flags
      = Process::ProcessFlags(Process::ProcessFlags::SupportsLasting | Process::ProcessFlags::ControlSurface);
};

template<typename T>
struct ControlMember {

  using value_type = typename T::type;

  const T spec;
  value_type value;

  operator value_type&() noexcept { return value; }
  operator const value_type&() const noexcept { return value; }
};

struct control_input
{

};

struct control_output
{

};

struct audio_input
{
  const ossia::audio_port* port{};
};
struct audio_output
{
  ossia::audio_port* port{};
};
struct value_input
{
  const ossia::value_port* port{};
};
struct value_output
{
  ossia::value_port* port{};
};
struct midi_input
{
  const ossia::midi_port* port{};
};
struct midi_output
{
  ossia::midi_port* port{};
};

template<typename T>
concept NamedPort = requires(T a) {
  { QString::fromUtf8(a.name()) } ;
};

template<typename T>
concept MultichannelAudioInput = requires (T a) {
  { a.samples.size() } -> std::convertible_to<std::size_t>;
  { a.samples[0].size() } -> std::convertible_to<std::size_t>;
  { a.samples[0][0] } -> std::same_as<const double&>;
};
template<typename T>
concept MultichannelAudioOutput = requires (T a) {
  { a.samples.size() } -> std::convertible_to<std::size_t>;
  { a.samples[0].size() } -> std::convertible_to<std::size_t>;
  { a.samples[0][0] } -> std::same_as<double&>;
};

template<typename T>
concept AudioEffectInput = requires (T a) {
  { a.samples } -> std::convertible_to<const double**>;
  { a.channels } -> std::convertible_to<std::size_t>;
};
template<typename T>
concept AudioEffectOutput = requires (T a) {
  { a.samples } -> std::convertible_to<double**>;
  { a.channels } -> std::convertible_to<std::size_t>;
};

template<typename T>
concept PortAudioInput = requires (T a) {
  { a.port } -> std::same_as<const ossia::audio_port*>;
};

template<typename T>
concept PortAudioOutput = requires (T a) {
  { a.port } -> std::same_as<ossia::audio_port*>;
};

template<typename T>
concept AudioInput = NamedPort<T> && (MultichannelAudioInput<T> || AudioEffectInput<T> || PortAudioInput<T>);
template<typename T>
concept AudioOutput = NamedPort<T> && (MultichannelAudioOutput<T> || AudioEffectOutput<T> || PortAudioOutput<T>);

template<typename T>
concept PortValueInput = requires (T a) {
  { a.port } -> std::same_as<const ossia::value_port*>;
};

template<typename T>
concept PortValueOutput = requires (T a) {
  { a.port } -> std::same_as<ossia::value_port*>;
};

template<typename T>
concept ValueInput = NamedPort<T> && requires (T a) {
  { a.port } -> std::same_as<const ossia::value_port*>;
};

template<typename T>
concept ValueOutput = NamedPort<T> && requires (T a) {
  { a.port } -> std::same_as<ossia::value_port*>;
};


template<typename T>
concept TimedValueInput = NamedPort<T> && requires (T a) {
  { a.values[0] } -> std::convertible_to<ossia::value>;
};

template<typename T>
concept TimedValueOutput = NamedPort<T> && requires (T a) {
  { a.values[0] } -> std::convertible_to<ossia::value>;
};

template<typename T>
concept MidiInput = NamedPort<T> && requires (T a) {
  { a.port } -> std::same_as<const ossia::midi_port*>;
};
template<typename T>
concept MidiOutput = NamedPort<T> && requires (T a) {
  { a.port } -> std::same_as<ossia::midi_port*>;
};

template<typename T>
concept ControlInput = requires (T a) {
  { a.control() };
};
template<typename T>
concept ControlOutput = requires (T a) {
  { a.display() };
};

template<typename T>
concept TimedVec = requires (T a) {
  { a.begin()->first } -> std::same_as<int64_t>;
  { a.begin()->second };
};


template<typename T>
using IsAudioInput = typename std::integral_constant< bool, AudioInput<T>>;
template<typename T>
using IsAudioOutput = typename std::integral_constant< bool, AudioOutput<T>>;
template<typename T>
using IsValueInput = typename std::integral_constant< bool, ValueInput<T>>;
template<typename T>
using IsValueOutput = typename std::integral_constant< bool, ValueOutput<T>>;
template<typename T>
using IsTimedValueInput = typename std::integral_constant< bool, TimedValueInput<T>>;
template<typename T>
using IsTimedValueOutput = typename std::integral_constant< bool, TimedValueOutput<T>>;
template<typename T>
using IsMidiInput = typename std::integral_constant< bool, MidiInput<T>>;
template<typename T>
using IsMidiOutput = typename std::integral_constant< bool, MidiOutput<T>>;
template<typename T>
using IsControlInput = typename std::integral_constant< bool, ControlInput<T>>;
template<typename T>
using IsControlOutput = typename std::integral_constant< bool, ControlOutput<T>>;

struct multichannel_audio_view {
  ossia::audio_vector* buffer{};
  const ossia::audio_channel& operator[](std::size_t i) const noexcept { return (*buffer)[i]; };
  std::size_t size() const noexcept { return buffer->size(); }
  void resize(std::size_t i) const noexcept { return buffer->resize(i); }
  void reserve(std::size_t channels, std::size_t bufferSize)
  {
    resize(channels);
    for(auto& vec : *buffer) vec.reserve(bufferSize);
  }
};

struct multichannel_audio {
  ossia::audio_vector* buffer{};
  ossia::audio_channel& operator[](int i) const noexcept { return (*buffer)[i];};
  std::size_t size() const noexcept { return buffer->size(); }
  void resize(std::size_t i) const noexcept { return buffer->resize(i); }
  void reserve(std::size_t channels, std::size_t bufferSize)
  {
    resize(channels);
    for(auto& vec : *buffer) vec.reserve(bufferSize);
  }
};
}



#include <boost/pfr.hpp>
namespace boost::pfr
{

template <class T, class F>
void for_each_field_ref(T&& value, F&& func) {
  constexpr std::size_t fields_count_val = boost::pfr::detail::fields_count<std::remove_reference_t<T>>();

  ::boost::pfr::detail::for_each_field_dispatcher(
      value,
      [&func](auto&& t) mutable {
        // MSVC related workaround. Its lambdas do not capture constexprs.
        constexpr std::size_t fields_count_val_in_lambda
            = boost::pfr::detail::fields_count<std::remove_reference_t<T>>();

        ::boost::pfr::detail::for_each_field_impl(
            t,
            std::forward<F>(func),
            detail::make_index_sequence<fields_count_val_in_lambda>{},
            std::is_rvalue_reference<T&&>{} );
      },
      detail::make_index_sequence<fields_count_val>{} );
}
}
