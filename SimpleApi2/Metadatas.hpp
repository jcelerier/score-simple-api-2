#pragma once

#include <boost/pfr.hpp>
#include <boost/mp11/algorithm.hpp>
#include <ossia/dataflow/safe_nodes/port.hpp>
#include <SimpleApi2/Concepts.hpp>
namespace SimpleApi2
{

template<typename>
struct get_type_list;

template<typename T>
struct get_control_value_type
{
  using type = typename decltype(std::remove_reference_t<T>::control())::type;

};
template<template<typename ...> typename Tuple, typename ... T>
struct get_type_list<Tuple<T...>>
{
  using type = Tuple<typename get_control_value_type<T>::type...>;
};
template<typename T>
struct get_ossia_port_type;
template<AudioInput T>
struct get_ossia_port_type<T> { using type = ossia::audio_inlet; };
template<AudioOutput T>
struct get_ossia_port_type<T> { using type = ossia::audio_outlet; };
template<ValueInput T>
struct get_ossia_port_type<T> { using type = ossia::value_inlet; };
template<ValueOutput T>
struct get_ossia_port_type<T> { using type = ossia::value_outlet; };
template<MidiInput T>
struct get_ossia_port_type<T> { using type = ossia::midi_inlet; };
template<MidiOutput T>
struct get_ossia_port_type<T> { using type = ossia::midi_outlet; };
template<ControlInput T>
struct get_ossia_port_type<T> { using type = ossia::value_inlet; };
template<ControlOutput T>
struct get_ossia_port_type<T> { using type = ossia::value_outlet; };

template<typename T>
using get_ossia_port_type_t = typename get_ossia_port_type<T>::type;

template<typename T>
struct is_control_input : std::false_type { };
template<ControlInput T>
struct is_control_input<T> : std::true_type { };
template<typename T>
struct is_control_output : std::false_type { };
template<ControlOutput T>
struct is_control_output<T> : std::true_type { };

template <typename Node_T>
struct info_functions_2
{
  // Anonymous struct:
  // struct {
  //   struct a: audio_in { } a;
  //   struct b: audio_out { } b;
  //   struct c: control_inlet { float value; } c;
  // };
  using inputs_type = decltype(Node_T::inputs);

  // tuple<a, b, c>
  using inputs_tuple = decltype(boost::pfr::structure_to_tuple(std::declval<inputs_type&>()));

  // mp_list<mp_size<0>, mp_size<1>, mp_size<2>>;
  using inputs_indices = boost::mp11::mp_iota_c<boost::mp11::mp_size<inputs_tuple>::value>;

  // tuple<ossia::audio_inlet, ossia::audio_outlet, ossia::value_inlet>
  using ossia_inputs_tuple = boost::mp11::mp_transform<get_ossia_port_type_t, inputs_tuple>;

  // Is the port at index N, a control input
  template<class N>
  using check_control_input = is_control_input<boost::mp11::mp_at_c<inputs_tuple, N::value>>;

  // mp_list<mp_size<2>>
  using control_input_indices = boost::mp11::mp_copy_if<inputs_indices, check_control_input>;

  // control_input_index<0>::value == 2
  template<std::size_t ControlN>
  using control_input_index = boost::mp11::mp_at_c<control_input_indices, ControlN>;

  // tuple<c>
  using control_input_tuple = boost::mp11::mp_copy_if<inputs_tuple, is_control_input>;

  // tuple<float>
  using control_input_values_type = typename SimpleApi2::get_type_list<control_input_tuple>::type;


  using outputs_type = decltype(Node_T::outputs);

  using outputs_tuple = decltype(boost::pfr::structure_to_tuple(std::declval<outputs_type&>()));

  using outputs_indices = boost::mp11::mp_iota_c<boost::mp11::mp_size<outputs_tuple>::value>;

  using ossia_outputs_tuple = boost::mp11::mp_transform<get_ossia_port_type_t, outputs_tuple>;

  template<class N>
  using check_control_output = is_control_output<boost::mp11::mp_at_c<outputs_tuple, N::value>>;

  using control_output_indices = boost::mp11::mp_copy_if<outputs_indices, check_control_output>;

  template<std::size_t ControlN>
  using control_output_index = boost::mp11::mp_at_c<control_output_indices, ControlN>;

  using control_output_tuple = boost::mp11::mp_copy_if<outputs_tuple, is_control_output>;

  using control_output_values_type = typename SimpleApi2::get_type_list<control_output_tuple>::type;


  static constexpr auto inlet_size = std::tuple_size_v<inputs_tuple>;
  static constexpr auto outlet_size = std::tuple_size_v<outputs_tuple>;

  static constexpr auto audio_in_count = boost::mp11::mp_count_if<inputs_tuple, IsAudioInput>::value;
  static constexpr auto audio_out_count = boost::mp11::mp_count_if<outputs_tuple, IsAudioOutput>::value;
  static constexpr auto value_in_count = boost::mp11::mp_count_if<inputs_tuple, IsAudioInput>::value;
  static constexpr auto value_out_count = boost::mp11::mp_count_if<outputs_tuple, IsAudioOutput>::value;
  static constexpr auto midi_in_count = boost::mp11::mp_count_if<inputs_tuple, IsMidiInput>::value;
  static constexpr auto midi_out_count = boost::mp11::mp_count_if<outputs_tuple, IsMidiOutput>::value;
  static constexpr auto control_in_count = boost::mp11::mp_count_if<inputs_tuple, IsControlInput>::value;
  static constexpr auto control_out_count = boost::mp11::mp_count_if<outputs_tuple, IsControlOutput>::value;
  /*
  using controls_type = decltype(Node_T::control_inputs);
  using controls_tuple = decltype(boost::pfr::structure_to_tuple(std::declval<controls_type&>()));
  using controls_values_type = typename SimpleApi2::get_type_list<controls_tuple>::type;

  using control_outs_type = decltype(Node_T::control_outputs);
  using control_outs_tuple = decltype(boost::pfr::structure_to_tuple(std::declval<control_outs_type&>()));
  using control_outs_values_type = typename SimpleApi2::get_type_list<control_outs_tuple>::type;

  static constexpr auto audio_in_count = [] {
    if constexpr(requires { Node_T::audio_inputs; })
      return boost::pfr::tuple_size_v<decltype(Node_T::audio_inputs)>;
    else
      return 0; }();
  static constexpr auto audio_out_count = [] {
    if constexpr(requires { Node_T::audio_outputs; })
      return boost::pfr::tuple_size_v<decltype(Node_T::audio_outputs)>;
    else
      return 0; }();

  static constexpr auto midi_in_count = [] {
    if constexpr(requires { Node_T::midi_inputs; })
      return boost::pfr::tuple_size_v<decltype(Node_T::midi_inputs)>;
    else
      return 0; }();
  static constexpr auto midi_out_count = [] {
    if constexpr(requires { Node_T::midi_outputs; })
      return boost::pfr::tuple_size_v<decltype(Node_T::midi_outputs)>;
    else
      return 0; }();

  static constexpr auto value_in_count = [] {
    if constexpr(requires { Node_T::value_inputs; })
      return boost::pfr::tuple_size_v<decltype(Node_T::value_inputs)>;
    else
      return 0; }();
  static constexpr auto value_out_count = [] {
    if constexpr(requires { Node_T::value_outputs; })
      return boost::pfr::tuple_size_v<decltype(Node_T::value_outputs)>;
    else
      return 0; }();

  static constexpr auto control_in_count = [] {
    if constexpr(requires { Node_T::control_inputs; })
      return boost::pfr::tuple_size_v<decltype(Node_T::control_inputs)>;
    else
      return 0; }();
  static constexpr auto control_out_count = [] {
    if constexpr(requires { Node_T::control_outputs; })
      return boost::pfr::tuple_size_v<decltype(Node_T::control_outputs)>;
    else
      return 0; }();

  static constexpr auto categorize_inlet(std::size_t i)
  {
    if (i < audio_in_count)
      return ossia::safe_nodes::inlet_kind::audio_in;
    else if (i < audio_in_count + midi_in_count)
      return ossia::safe_nodes::inlet_kind::midi_in;
    else if (i < audio_in_count + midi_in_count + value_in_count)
      return ossia::safe_nodes::inlet_kind::value_in;
    else if (i < audio_in_count + midi_in_count + value_in_count + control_in_count)
      return ossia::safe_nodes::inlet_kind::control_in;
    else
      throw std::runtime_error("Invalid input number");
  }

  static constexpr auto categorize_outlet(std::size_t i)
  {
    if (i < audio_out_count)
      return ossia::safe_nodes::outlet_kind::audio_out;
    else if (i < audio_out_count + midi_out_count)
      return ossia::safe_nodes::outlet_kind::midi_out;
    else if (i < audio_out_count + midi_out_count + value_out_count)
      return ossia::safe_nodes::outlet_kind::value_out;
    else if (i < audio_out_count + midi_out_count + value_out_count + control_out_count)
      return ossia::safe_nodes::outlet_kind::control_out;
    else
      throw std::runtime_error("Invalid output number");
  }

  static constexpr auto control_start
      = audio_in_count + midi_in_count + value_in_count;

  static constexpr auto control_out_start
      = audio_out_count + midi_out_count + value_out_count;

  static constexpr auto inlet_size = control_start + control_in_count;

  static constexpr auto outlet_size = control_out_start + control_out_count;
*/
};

}
